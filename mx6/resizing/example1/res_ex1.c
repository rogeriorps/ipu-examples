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
	
	FILE * file_in = NULL;
        FILE * file_out = NULL;

	struct timeval begin, end;
	int sec, usec, run_time = 0;
 
	int fd_ipu = 0;   // IPU file descriptor
	int isize = 0;    // input size
	int osize = 0;    // output size
 
	void *inbuf = NULL;
	void *outbuf = NULL;
 
	int ret = 0;
 
	memset(&task, 0, sizeof(task));
 
	// Input image size and format
	task.input.width    = 1024;
	task.input.height   = 768;
	task.input.format   = v4l2_fourcc('R', 'G', 'B', 'P');
 
	// Output image size and format
	task.output.width   = 800;
	task.output.height  = 480;
	task.output.format  = v4l2_fourcc('R', 'G', 'B', '3');
 
 	// Open the raw image
	if ((file_in = fopen("../../../images/freescale_1024x768_rgb565.raw", "rb")) < 0) {
		printf("Unable to open freescale_1024x768.raw\n");
		ret = -1;
		goto done;
	}

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
  
	// Allocate memory for output image
	osize = task.output.paddr =
		task.output.width * task.output.height
		* fmt_to_bpp(task.output.format)/8;
	ret = ioctl(fd_ipu, IPU_ALLOC, &task.output.paddr);
	if (ret < 0) {
		printf("ioctl IPU_ALLOC fail\n");
		goto done;
	}
 
	// Create memory map for output image
	outbuf = mmap(0, osize, PROT_READ | PROT_WRITE,
	MAP_SHARED, fd_ipu, task.output.paddr);
	if (!outbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}
 
	// Open output file for writing
	if ((file_out = fopen("output_file.raw", "wb")) < 0) {
		printf("Cannot open output file");
		ret = -1;
		goto done;
	}
 
	// Read input image
	ret = fread(inbuf, 1, isize, file_in);
	if (ret < isize) {
		ret = 0;
		printf("Cannot read enough data from input file\n");
		goto done;
	}

	gettimeofday(&begin, NULL);

	// Perform the rotation 
	ret = ioctl(fd_ipu, IPU_QUEUE_TASK, &task);
	if (ret < 0) {
		printf("ioct IPU_QUEUE_TASK fail\n");
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

	// Write resized image to output file
	ret = fwrite(outbuf, 1, osize, file_out);
	if (ret < osize) {
		ret = -1;
		printf("Cannot write enough data into output file\n");
		goto done;
	}

  	printf("Resize time: %d usecs\n", run_time);
 
done:
	if (file_out)
		fclose(file_out);
	if (outbuf)
		munmap(outbuf, osize);
	if (task.output.paddr)
		ioctl(fd_ipu, IPU_FREE, &task.output.paddr);
	if (inbuf)
		munmap(inbuf, isize);
	if (task.input.paddr)
		ioctl(fd_ipu, IPU_FREE, &task.input.paddr);
	if (fd_ipu)
		close(fd_ipu);
	if (file_in)
		fclose(file_in);

	return ret;
}
