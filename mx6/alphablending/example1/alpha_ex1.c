/*
* Copyright 2013 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* The code contained herein is licensed under the GNU Lesser General
* Public License. You may obtain a copy of the GNU Lesser General
* Public License Version 2.1 or later at the following locations:
*
* http://www.opensource.org/licenses/lgpl-license.html
* http://www.gnu.org/copyleft/lgpl.html
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>
#include <linux/ipu.h>

static unsigned int fmt_to_bpp(unsigned int pixelformat)
{
	unsigned int bpp;
 
        switch (pixelformat)
        {
                case IPU_PIX_FMT_RGB565:
               /*interleaved 422*/
                case IPU_PIX_FMT_YUYV:
                case IPU_PIX_FMT_UYVY:
                /*non-interleaved 422*/
                case IPU_PIX_FMT_YUV422P:
                case IPU_PIX_FMT_YVU422P:
                        bpp = 16;
                        break;
                case IPU_PIX_FMT_BGR24:
                case IPU_PIX_FMT_RGB24:
                case IPU_PIX_FMT_YUV444:
                case IPU_PIX_FMT_YUV444P:
                        bpp = 24;
                        break;
                case IPU_PIX_FMT_BGR32:
                case IPU_PIX_FMT_BGRA32:
                case IPU_PIX_FMT_RGB32:
                case IPU_PIX_FMT_RGBA32:
                case IPU_PIX_FMT_ABGR32:
                        bpp = 32;
                        break;
                /*non-interleaved 420*/
                case IPU_PIX_FMT_YUV420P:
                case IPU_PIX_FMT_YVU420P:
                case IPU_PIX_FMT_YUV420P2:
                case IPU_PIX_FMT_NV12:
    case IPU_PIX_FMT_TILED_NV12:
                        bpp = 12;
                        break;
                default:
                        bpp = 8;
                        break;
        }
        return bpp;
}

int main (int argc, char *argv[])
{
	struct ipu_task task;
	struct timeval begin, end;
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	dma_addr_t outpaddr;

	int sec, usec, run_time;
	int fd_ipu, fd_fb, isize, ovsize, osize;
	int ret, i;
 
	void *inbuf = NULL;
	void *ovbuf = NULL;
	void *outbuf = NULL;

	// Clear &task	
	memset(&task, 0, sizeof(task));
 
	// Input image size and format
	task.input.width    = 1024;
	task.input.height   = 768;
	task.input.format   = v4l2_fourcc('R', 'G', 'B', 'P');
 
	// Overlay image size and format
	task.overlay.width    = 1024;
	task.overlay.height   = 768;
	task.overlay.format   = v4l2_fourcc('R', 'G', 'B', 'P');
	task.overlay_en = 1;
	task.overlay.alpha.mode = 0; //0 - Global alpha blending
	task.overlay.alpha.gvalue = 0;

	// Output image size and format
	task.output.width   = 1024;
	task.output.height  = 768;
	task.output.format  = v4l2_fourcc('R', 'G', 'B', 'P');
 
	// Open IPU device
	fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
	if (fd_ipu < 0) {
		printf("open ipu dev fail\n");
		ret = -1;
		goto done;
	}

	// Calculate input size from image dimensions and bits-per-pixel
	// according to format
	isize = task.input.paddr =
		task.input.width * task.input.height
		* fmt_to_bpp(task.input.format)/8;

	// Allocate contingous physical memory for input image
	// input.paddr contains the amount needed
	// this value will be replaced with physical address on success
	ret = ioctl(fd_ipu, IPU_ALLOC, &task.input.paddr);
	if (ret < 0) {
		printf("ioctl IPU_ALLOC fail: (errno = %d)\n", errno);
		goto done;
	}

	// Create memory map and obtain the allocated memory virtual address
	inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd_ipu, task.input.paddr);
	if (!inbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}

	// Fill input buffer with white color
	memset(inbuf, 0xff, isize);

	// Calculate the overlay size 
	ovsize = task.overlay.paddr =
		task.overlay.width * task.overlay.height
		* fmt_to_bpp(task.overlay.format)/8;

	// Allocate contiguous  physical memory for overlay buffer
	ret = ioctl(fd_ipu, IPU_ALLOC, &task.overlay.paddr);
	if (ret < 0) {
		printf("ioctl IPU_ALLOC fail\n");
		goto done;
	}

	// Map the overlay buffer
	ovbuf = mmap(0, ovsize, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_ipu, task.overlay.paddr);
	if (!ovbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}

	// Fill overlay buffer with 4 color strips
	memset(ovbuf, 0x2222, ovsize/4);
	memset(ovbuf+ovsize/4, 0x6666, ovsize/4);
	memset(ovbuf+ovsize/2, 0xaaaa, ovsize/4);
	memset(ovbuf+ovsize*3/4, 0xdddd, ovsize/4);

	// Open Framebuffer and gets its address
	if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = -1;
		goto done;
	}

	if ( ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		printf("Get FB fix info failed!\n");
		ret = -1;
		goto done;
	}

	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);

	// Set IPU output address as framebuffer address
	outpaddr = fb_fix.smem_start;	
	task.output.paddr = outpaddr;

	// Create memory map for output image
	outbuf = mmap(0, osize, PROT_READ | PROT_WRITE,
	MAP_SHARED, fd_ipu, task.output.paddr);
	if (!outbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}

	for (i=0; i < 256 ; i++) {

		task.overlay.alpha.gvalue = i;
		printf("Global Alpha Value = %d\n", i);

		gettimeofday(&begin, NULL);
	
		// Perform combining 
		ret = ioctl(fd_ipu, IPU_QUEUE_TASK, &task);
		if (ret < 0) {
			printf("ioct IPU_QUEUE_TASK fail %x\n", ret);
			goto done;
		}
	 
		gettimeofday(&end, NULL);
		sec = end.tv_sec - begin.tv_sec;
		usec = end.tv_usec - begin.tv_usec;
		if (usec < 0) {
			sec--;
			usec = usec + 1000000;
		}
		run_time = (sec * 1000000) + usec;
	
	  	printf("Resize time: %d usecs\n", run_time);
		usleep(100000);
	}
 
done:
	if (outbuf)
		munmap(outbuf, osize);
	if (task.output.paddr)
		ioctl(fd_ipu, IPU_FREE, &task.output.paddr);
	if (inbuf)
		munmap(inbuf, isize);
	if (ovbuf)
		munmap(ovbuf, isize);
	if (task.input.paddr)
		ioctl(fd_ipu, IPU_FREE, &task.input.paddr);
	if (fd_ipu)
		close(fd_ipu);
	if (fd_fb)
		close(fd_fb);

	return ret;
}

