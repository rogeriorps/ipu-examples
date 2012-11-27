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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "rot_ex1.h"

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>

#define BUF_CNT		5

int ctrl_c_rev = 0;

void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
	ctrl_c_rev = 1;
}

int fd_fb_alloc = 0;

int main(int argc, char *argv[])
{

	int ret = 0, next_update_idx = 0, done_cnt = 0, first_time = 1;
	int done_loop = 0, total_cnt = 0;
	ipu_lib_handle_t ipu_handle;
	ipu_test_handle_t test_handle;
	int file_in;
	struct sigaction act;
	struct timeval begin, end;
	int sec, usec, run_time = 0;
	size_t filesize;

	int x = 0, y = 0, fd_fb = 0;
	int lcd_w, lcd_h;
	int blank;
	char random_color;
	int start_angle, screen_size;
	void * buf, * raw_image;
	struct mxcfb_pos pos;
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;

	if ((argc != 2) || (atoi(argv[1]) < 0) || (atoi(argv[1]) > 7)) {
		printf("\nMXC IPU device Test\n\n" \
			"Usage: ./rot_ex1 <rotation number>\n");
		printf("\nWhere <rotation number>:\n\n" \
		"0 = IPU_ROTATE_NONE\n" \
		"1 = IPU_ROTATE_VERT_FLIP\n" \
		"2 = IPU_ROTATE_HORIZ_FLIP\n" \
		"3 = IPU_ROTATE_180\n" \
		"4 = IPU_ROTATE_90_RIGHT\n" \
		"5 = IPU_ROTATE_90_RIGHT_VFLIP\n" \
		"6 = IPU_ROTATE_90_RIGHT_HFLIP\n" \
		"7 = IPU_ROTATE_90_LEFT\n\n");
		goto done;
	}

	/* Open the raw image */
	if ((file_in = open("../../../images/raw_img_800x480.raw", O_RDWR, 0)) < 0) {
		printf("Unable to open raw_img_800x480.raw\n");
		ret = -1;
		goto done;
	}
	filesize = lseek(file_in, 0, SEEK_END);
	raw_image = mmap(0, filesize, PROT_READ, MAP_SHARED, file_in, 0);

	if (raw_image == MAP_FAILED) {
		printf("mmap failed file in!\n");
		ret = -1;
		goto done;
	}

	memset(&ipu_handle, 0, sizeof(ipu_lib_handle_t));    //initialize (clean) ipu_handle
	memset(&test_handle, 0, sizeof(ipu_test_handle_t));  //initialize (clean) test_handle
	test_handle.ipu_handle = &ipu_handle;

	/* clear background fb, get the lcd frame info */
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

	screen_size = fb_var.yres_virtual * fb_fix.line_length;

	buf = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_fb, 0);
	if (buf == MAP_FAILED) {
		printf("mmap failed!\n");
		ret = -1;
		goto done;
	}
	memset(buf, 0x00, screen_size);

	blank = FB_BLANK_UNBLANK;
	if ( ioctl(fd_fb, FBIOBLANK, blank) < 0) {
		printf("UNBLANK FB failed!\n");
		ret = -1;
		goto done;
	}

	test_handle.mode = OP_STREAM_MODE;
	test_handle.input.width = fb_var.xres;
	test_handle.input.height = fb_var.yres;
	test_handle.input.fmt = v4l2_fourcc('R', 'G', 'B', 'P');
	test_handle.output.width = fb_var.xres;
	test_handle.output.height = fb_var.yres;
	if (fb_var.bits_per_pixel == 24)
		test_handle.output.fmt = v4l2_fourcc('B', 'G', 'R', '3');
	else
		test_handle.output.fmt = v4l2_fourcc('R', 'G', 'B', 'P');

	test_handle.output.rot = atoi(argv[1]);	
	
	printf ("\n rotation=%d\n",atoi(argv[1]));

	screen_size = fb_var.yres * fb_fix.line_length;
	test_handle.output.user_def_paddr[0] = fb_fix.smem_start + screen_size;
	test_handle.output.user_def_paddr[1] = fb_fix.smem_start;
	test_handle.output.user_def_paddr[2] = fb_fix.smem_start + 2*screen_size;

	ret = mxc_ipu_lib_task_init(&(test_handle.input), NULL, &(test_handle.output),
			test_handle.mode, test_handle.ipu_handle);

	if (ret < 0) {
		printf("mxc_ipu_lib_task_init failed!\n");
		goto done;
	}

	srand((unsigned int)time(0));
	random_color = (char)(rand()%255);

	/* for stream mode, fill two input frame to prepare */
	memset(test_handle.ipu_handle->inbuf_start[0], random_color, test_handle.ipu_handle->ifr_size);
	memset(test_handle.ipu_handle->inbuf_start[0], 0x44, test_handle.ipu_handle->ifr_size / 4);
	memset(test_handle.ipu_handle->inbuf_start[1], random_color, test_handle.ipu_handle->ifr_size);
	memset(test_handle.ipu_handle->inbuf_start[1], 0x44, test_handle.ipu_handle->ifr_size / 4);

	memcpy(test_handle.ipu_handle->inbuf_start[1], raw_image, test_handle.ipu_handle->ifr_size);

	/* start first frame */
	if((next_update_idx = mxc_ipu_lib_task_buf_update(test_handle.ipu_handle, 0, 0, 0, NULL, &fd_fb)) < 0) {
		mxc_ipu_lib_task_uninit(test_handle.ipu_handle);
		goto done;
	}

	/* If ctrl-c is pressed, go to done */
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = ctrl_c_handler;

	if((ret = sigaction(SIGINT, &act, NULL)) < 0) {
		printf("install sigal error\n");
		goto done;
	}

	sleep(5); // Show the image for 5 seconds

done:

	if (fd_fb)
		close(fd_fb);
	if (ret < 0)
		printf("Error return !\n");
	if (ret == 0)
		printf("Return OK !\n");

	return 0;
}

