/*
 * Copyright 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
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
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>
#include "rot_ex1.h"
#define FB_BUFS 3

#define PAGE_ALIGN(x) (((x) + 4095) & ~4095)

int ctrl_c_rev = 0;

void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
	ctrl_c_rev = 1;
}

int fd_fb_alloc = 0;

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


int main(int argc, char *argv[])
{

	ipu_test_handle_t test_handle;
	struct ipu_task *t = &test_handle.task;
	int ret = 0, done_cnt = 0, i = 0;
	struct sigaction act;
	int file_in;
	int fd_ipu = 0, fd_fb = 0;
	int isize = 0;
	void *inbuf = NULL;
	dma_addr_t outpaddr[FB_BUFS];
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	int blank;
	size_t filesize;
	void * raw_image, * buf;

	// Open the raw image
	if ((file_in = open("../../../images/freescale_1024x768_rgb565.raw", O_RDWR, 0)) < 0) {
		printf("Unable to open freescale_1024x768_rgb565.raw\n");
		ret = -1;
		goto done;
	}
	filesize = lseek(file_in, 0, SEEK_END);
	raw_image = mmap(0, filesize, PROT_READ, MAP_SHARED, file_in, 0);

	// Cleaning the test_handle struct
	memset(&test_handle, 0, sizeof(ipu_test_handle_t));

	// Default Settings
	t->priority = 0;
	t->task_id = 0;
	t->timeout = 1000;
	test_handle.fcount = 50;
	test_handle.loop_cnt = 1;
	t->input.width = 1024;
	t->input.height = 768;
	t->input.format =  v4l2_fourcc('R','G','B','P');
	t->input.crop.pos.x = 0;
	t->input.crop.pos.y = 0;
	t->input.crop.w = 0;
	t->input.crop.h = 0;
	t->input.deinterlace.enable = 0;
	t->input.deinterlace.motion = 0;

	t->output.width = 1024;
	t->output.height = 768;
	t->output.format = v4l2_fourcc('R','G','B','P');
	t->output.rotate = 0;
	t->output.crop.pos.x = 0;
	t->output.crop.pos.y = 0;
	t->output.crop.w = 0;
	t->output.crop.h = 0;

	test_handle.show_to_fb = 1;

	fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
	if (fd_ipu < 0) {
		printf("open ipu dev fail\n");
		ret = -1;
		goto done;
	}

	if (IPU_PIX_FMT_TILED_NV12F == t->input.format) {
		isize = PAGE_ALIGN(t->input.width * t->input.height/2) +
			PAGE_ALIGN(t->input.width * t->input.height/4);
		isize = t->input.paddr = isize * 2;
	} else
		isize = t->input.paddr =
			t->input.width * t->input.height
			* fmt_to_bpp(t->input.format)/8;
	ret = ioctl(fd_ipu, IPU_ALLOC, &t->input.paddr);
	
	if (ret < 0) {
		printf("ioctl IPU_ALLOC fail\n");
		goto done;
	}

	// Map the IPU input buffer
	inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd_ipu, t->input.paddr);
	if (!inbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}

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

	if ( ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
		printf("Get FB var info failed!\n");
		ret = -1;
		goto done;
	}

	ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);

	for (i=0; i<FB_BUFS; i++)
		outpaddr[i] = fb_fix.smem_start +
			i * fb_var.yres * fb_fix.line_length;

	// Unblank the display
	blank = FB_BLANK_UNBLANK;
	ioctl(fd_fb, FBIOBLANK, blank);

	printf("isize value %d\n", isize);

	buf = mmap(NULL, isize, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_fb, 0);
	if (buf == MAP_FAILED) {
		printf("mmap failed!\n");
		ret = -1;
		goto done;
	}

	t->output.paddr = outpaddr[done_cnt % FB_BUFS];

	//Copy the raw image to the IPU buffer
	memcpy(inbuf, raw_image, isize);

	/* If ctrl-c is pressed, go to done */
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = ctrl_c_handler;

	if((ret = sigaction(SIGINT, &act, NULL)) < 0) {
		printf("install sigal error\n");
		goto done;
	}

	for (i=1; i < 8 ; i++) {
		t->output.rotate = i;
		ret = ioctl(fd_ipu, IPU_QUEUE_TASK, t);
		if (ret < 0) {
			printf("ioct IPU_QUEUE_TASK fail\n");
			goto done;
		}
		sleep(2); // Show each image for 2 seconds
	}

done:

	if (fd_fb)
		close(fd_fb);
	if (t->output.paddr)
		ioctl(fd_ipu, IPU_FREE, &t->output.paddr);
	if (inbuf)
		munmap(inbuf, isize);
	if (t->input.paddr)
		ioctl(fd_ipu, IPU_FREE, &t->input.paddr);
	if (fd_ipu)
		close(fd_ipu);
	if (file_in)
		close(file_in);

	return 0;
}

