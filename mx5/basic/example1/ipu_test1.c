/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc. All Rights Reserved.
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

/*!
 * @file mxc_ipudev_test.c
 *
 * @brief IPU device lib test implementation
 *
 * @ingroup IPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "ipu_test1.h"

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>

/*
       Y = R *  .299 + G *  .587 + B *  .114;
       U = R * -.169 + G * -.332 + B *  .500 + 128.;
       V = R *  .500 + G * -.419 + B * -.0813 + 128.;*/

#define red(x) (((x & 0xE0) >> 5) * 0x24)
#define green(x) (((x & 0x1C) >> 2) * 0x24)
#define blue(x) ((x & 0x3) * 0x55)
#define y(rgb) ((red(rgb)*299L + green(rgb)*587L + blue(rgb)*114L) / 1000)
#define u(rgb) ((((blue(rgb)*500L) - (red(rgb)*169L) - (green(rgb)*332L)) / 1000))
#define v(rgb) (((red(rgb)*500L - green(rgb)*419L - blue(rgb)*81L) / 1000))
#define BUF_CNT		5

int fd_fb_alloc = 0;

int dma_memory_alloc(int size, int cnt, dma_addr_t paddr[], void * vaddr[])
{
	int i, ret = 0;

	if ((fd_fb_alloc = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = -1;
		goto done;
	}

	for (i=0;i<cnt;i++) {
		//alloc mem from DMA zone
		//input as request mem size
		// Before ioctl, paddr[i] contains the requesting buffer size,
		// after ioctl, paddr[i] contains the address of requested buffer.
		// vaddr[i] contains the mapped address.
		paddr[i] = size;
		if ( ioctl(fd_fb_alloc, FBIO_ALLOC, &(paddr[i])) < 0) {
			printf("Unable alloc mem from /dev/fb0\n");
			close(fd_fb_alloc);
			if ((fd_fb_alloc = open("/dev/fb1", O_RDWR, 0)) < 0) {
				printf("Unable to open /dev/fb1\n");
				if ((fd_fb_alloc = open("/dev/fb2", O_RDWR, 0)) < 0) {
					printf("Unable to open /dev/fb2\n");
					ret = -1;
					goto done;
				} else if ( ioctl(fd_fb_alloc, FBIO_ALLOC, &(paddr[i])) < 0) {
					printf("Unable alloc mem from /dev/fb2\n");
					ret = -1;
					goto done;
				}
			} else if ( ioctl(fd_fb_alloc, FBIO_ALLOC, &(paddr[i])) < 0) {
				printf("Unable alloc mem from /dev/fb1\n");
				close(fd_fb_alloc);
				if ((fd_fb_alloc = open("/dev/fb2", O_RDWR, 0)) < 0) {
					printf("Unable to open /dev/fb2\n");
					ret = -1;
					goto done;
				} else if ( ioctl(fd_fb_alloc, FBIO_ALLOC, &(paddr[i])) < 0) {
					printf("Unable alloc mem from /dev/fb2\n");
					ret = -1;
					goto done;
				}
			}
		}

		vaddr[i] = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
				fd_fb_alloc, paddr[i]);
		if (vaddr[i] == MAP_FAILED) {
			printf("mmap failed!\n");
			ret = -1;
			goto done;
		}
	} 
done: 
	return ret;
}

void dma_memory_free(int size, int cnt, dma_addr_t paddr[], void * vaddr[])
{
	int i;

	for (i=0;i<cnt;i++) {
		if (vaddr[i])
			munmap(vaddr[i], size);
		if (paddr[i])
			ioctl(fd_fb_alloc, FBIO_FREE, &(paddr[i]));
	}
}

void gen_fill_pattern(char * buf, int in_width, int in_height)
{
	int y_size = in_width * in_height;
	int h_step = in_height / 255;
	int w_step = in_width / 255;
	int h, w;
	uint32_t y_color = 0;
	int32_t u_color = 0;
	int32_t v_color = 0;
	uint32_t rgb = 0x0;
	static int32_t alpha = 0;
	static int inc_alpha = 1;

	int32_t rgb_temp, rgb_temp2= rgb;

	for (h = 0; h < (in_height); h++) {
		

		for (w = 0; w < (in_width); w++) {
		
			y_color = y(rgb_temp);
			u_color = u(rgb_temp);
			v_color = v(rgb_temp);
			
			if (!(w_step)) {
				w_step = in_width / 255;
				rgb_temp ++;
				if (rgb_temp > 255)
					rgb_temp = 0;
			} 
			w_step--;

//			buf[(h*in_width) + w] = 0x3f;//(int) y_color;
//			buf[(h*in_width) + w + 1] = 0x00;//(int) u_color;
//			buf[(h*in_width) + w + 2] = 0x00;//(int) v_color;

			buf[(h*in_width) + w] = rgb_temp;


//			buf[(h*in_width) + w] = y_color;
			if (!(h & 0x1) && !(w & 0x1)) {
			buf[y_size + (((h*in_width)/4) + (w/2)) ] = 0x3f;//u_color;
			buf[y_size + y_size/4 + (((h*in_width)/4) + (w/2))] = 0x3f;//v_color;

	//		rgb_temp++;

			}
		}
	}

}

int color_bar(int overlay, ipu_test_handle_t * test_handle)
//int color_bar(ipu_test_handle_t * test_handle)

{
	int ret = 0, fd_fb = 0, size = 0, i, k = 0, done_cnt = 0, fcount = 0;
	void * buf[BUF_CNT] = {0}, * fb[3];
	void * ov_fake_fb = 0, * ov_alpha_fake_fb = 0;
	int ov_fake_fb_paddr = 0, ov_alpha_fake_fb_paddr = 0;
	int paddr[BUF_CNT] = {0};
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	struct mxcfb_gbl_alpha g_alpha;
	unsigned int system_rev = 0, ipu_version;
	ipu_lib_overlay_param_t ov;
	int screen_size, ov_fake_fb_size = 0, ov_alpha_fake_fb_size = 0;

	if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = -1;
		goto done;
	}

//	g_alpha.alpha = 128;
//	g_alpha.enable = 1;
//	if (ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &g_alpha) < 0) {
//		printf("Set global alpha failed\n");
//		ret = -1;
//		goto done;
//	}

	if ( ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
		printf("Get FB var info failed!\n");
		ret = -1;
		goto done;
	}
	if ( ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		printf("Get FB fix info failed!\n");
		ret = -1;
		goto done;
	}

	if(fb_var.yres_virtual != 3*fb_var.yres)
	{
		fb_var.yres_virtual = 3*fb_var.yres;
		if ( ioctl(fd_fb, FBIOPUT_VSCREENINFO, &fb_var) < 0) {
			printf("Get FB var info failed!\n");
			ret = -1;
			goto done;
		}
	}

	/* map the framebuffer, now fb[0] contains the mapped address of /dev/fb0 */
	screen_size = fb_var.yres * fb_fix.line_length;
	printf("fb_var.yres: %d\n ", fb_var.yres);
	printf("fb_var.xres: %d\n ", fb_var.xres);
	printf("fb_fix.line_length: %d\n", fb_fix.line_length);
	printf("fb_var.bits_per_pixel: %d\n", fb_var.bits_per_pixel);
	fb[0] = mmap(NULL, 3 * screen_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_fb, 0);
	if (fb[0] == MAP_FAILED) {
		printf("fb buf0 mmap failed, errno %d!\n", errno);
		ret = -1;
		goto done;
	}

	/* ipu use fb base+screen_size as buf0 */
	fb[1] = (void *)((char *)fb[0]);
	fb[0] = (void *)((char *)fb[1] + screen_size);
	fb[2] = (void *)((char *)fb[1] + 2*screen_size);

	/* use I420 input format as fix*/
	test_handle->mode = OP_NORMAL_MODE; //OP_STREAM_MODE;
	test_handle->fcount = fcount = 500;
	test_handle->input.width = 800;
	test_handle->input.height = 480;
	test_handle->input.fmt = v4l2_fourcc('I', '4', '2', '0');
//	test_handle->input.fmt = IPU_PIX_FMT_YUYV;
//	test_handle->output.fmt = v4l2_fourcc('I', '4', '2', '0');
//	test_handle->output.fmt = IPU_PIX_FMT_YUYV;
	test_handle->output.show_to_fb = 1;
	test_handle->output.fb_disp.fb_num = 0;
	
//	test_handle->input.fmt = v4l2_fourcc('R', 'G', 'B', 'P');
	if (fb_var.bits_per_pixel == 24)
		test_handle->output.fmt = v4l2_fourcc('B', 'G', 'R', '3');
	else
		test_handle->output.fmt = v4l2_fourcc('R', 'G', 'B', 'P');

	test_handle->output.rot = 0;

	/* one output case -- full screen */
	test_handle->output.width = 799;//fb_var.xres;
	test_handle->output.height = 480;//fb_var.yres;

	/*allocate dma buffers from fb dev. For I420, each pixel = 3/2*bytes */
	size = test_handle->input.width * test_handle->input.height * 3/2;
	ret = dma_memory_alloc(size, BUF_CNT, paddr, buf);
	if ( ret < 0) {
		printf("dma_memory_alloc failed\n");
		goto done;
	}

	test_handle->input.user_def_paddr[0] = paddr[0];
	gen_fill_pattern(buf[0], test_handle->input.width, test_handle->input.height);

	i = 0;

	ret = mxc_ipu_lib_task_init(&(test_handle->input), NULL, &(test_handle->output),
		test_handle->mode, test_handle->ipu_handle);

	if (ret < 0) {
		printf("mxc_ipu_lib_task_init failed!\n");
		goto done;
	}

	printf("mxc_ipu_lib_task_init completed!\n");
	memset(test_handle->ipu_handle->inbuf_start[0], 0x44, test_handle->ipu_handle->ifr_size);   //test
	mxc_ipu_lib_task_buf_update(test_handle->ipu_handle, 0, 0, 0, NULL, &fd_fb);   //test
	

//	if (mxc_ipu_lib_task_buf_update(test_handle->ipu_handle, paddr[i], 0, 0, NULL, NULL) < 0)
		printf("mxc_ipu_lib_task_buf_update Error\n") ;

	printf("mxc_ipu_lib_task_buf_update1 completed!\n");

	gen_fill_pattern(buf[i], test_handle->input.width, test_handle->input.height);

	sleep(3);

	test_handle->output.rot = 2;

	if (mxc_ipu_lib_task_buf_update(test_handle->ipu_handle, paddr[i], 0, 0, NULL, NULL) < 0)
		printf("mxc_ipu_lib_task_buf_update Error\n") ;

	printf("mxc_ipu_lib_task_buf_update2 completed!\n");

	sleep(3);

	mxc_ipu_lib_task_uninit(test_handle->ipu_handle);

	printf("mxc_ipu_lib_task_uninit completed!\n");

done:
	dma_memory_free(size, BUF_CNT, paddr, buf);
	printf("colorbar done!\n");
	if (fd_fb)
		close(fd_fb);

	return ret;
}

//rog int run_test_pattern(int pattern, ipu_test_handle_t * test_handle)
int run_test_pattern(ipu_test_handle_t * test_handle)

{
//	printf("Color bar test with full-screen:\n");
//	return color_bar(NO_OV, test_handle);
	return hop_block(test_handle);
}


/*
 * This call-back function provide one method to update
 * framebuffer by pan_display.
 */
void hop_block_output_cb(void * arg, int index)
{
	int fd_fb = *((int *)arg);
	struct fb_var_screeninfo fb_var;

	ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
	/* for buf index 0, its phyaddr is fb buf1*/
	/* for buf index 1, its phyaddr is fb buf0*/
	/* for buf index 2, its phyaddr is fb buf2*/
	if (index == 0)
		fb_var.yoffset = fb_var.yres;
	else if (index == 1)
		fb_var.yoffset = 0;
	else
		fb_var.yoffset = 2 * fb_var.yres;

	ioctl(fd_fb, FBIOPAN_DISPLAY, &fb_var);
}

int hop_block(ipu_test_handle_t * test_handle)
{
	int ret = 0, x = 0, y = 0, fd_fb = 0, next_update_idx = 0;
	int lcd_w, lcd_h;
	int blank;
	char random_color;
	int start_angle, screen_size;
	void * buf;
	struct mxcfb_pos pos;
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;

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
	lcd_w = fb_var.xres;
	lcd_h = fb_var.yres;
	screen_size = fb_var.yres_virtual * fb_fix.line_length;

	buf = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_fb, 0);
	if (buf == MAP_FAILED) {
		printf("mmap failed!\n");
		ret = -1;
		goto done;
	}
	memset(buf, 0, screen_size);
	close(fd_fb);

	/* display hop block to overlay */
//	if (foreground_fb() == 2) {
		if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
			printf("Unable to open /dev/fb2\n");
			ret = -1;
			goto done;
		}
//	} else {
//		if ((fd_fb = open("/dev/fb1", O_RDWR, 0)) < 0) {
//			printf("Unable to open /dev/fb1\n");
//			ret = -1;
//			goto done;
//		}
//	}

	fb_var.xres = test_handle->block_width
			- test_handle->block_width%8;
	fb_var.xres_virtual = fb_var.xres;
	fb_var.yres = test_handle->block_width;
	fb_var.yres_virtual = fb_var.yres * 3;
//	if ( ioctl(fd_fb, FBIOPUT_VSCREENINFO, &fb_var) < 0) {
//		printf("Set FB var info failed!\n");
//		ret = -1;
//		goto done;
//	}

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
	blank = FB_BLANK_UNBLANK;
	if ( ioctl(fd_fb, FBIOBLANK, blank) < 0) {
		printf("UNBLANK FB failed!\n");
		ret = -1;
		goto done;
	}

	test_handle->mode = OP_STREAM_MODE;
	test_handle->input.width = 800;
	test_handle->input.height = 480;
	test_handle->input.fmt = v4l2_fourcc('R', 'G', 'B', 'P');
	test_handle->output.width = 800;//test_handle->block_width
					//- test_handle->block_width%8;
	test_handle->output.height = 400; //test_handle->block_width;
	if (fb_var.bits_per_pixel == 24)
		test_handle->output.fmt = v4l2_fourcc('B', 'G', 'R', '3');
	else
		test_handle->output.fmt = v4l2_fourcc('R', 'G', 'B', 'P');
	test_handle->output.show_to_fb = 0;
	test_handle->output.rot = 5;

	screen_size = fb_var.yres * fb_fix.line_length;
	test_handle->output.user_def_paddr[0] = fb_fix.smem_start + screen_size;
	test_handle->output.user_def_paddr[1] = fb_fix.smem_start;
	test_handle->output.user_def_paddr[2] = fb_fix.smem_start + 2*screen_size;

	ret = mxc_ipu_lib_task_init(&(test_handle->input), NULL, &(test_handle->output),
			test_handle->mode, test_handle->ipu_handle);
	if (ret < 0) {
		printf("mxc_ipu_lib_task_init failed!\n");
		goto done;
	}

	srand((unsigned int)time(0));
	random_color = (char)(rand()%255);
	/* for stream mode, fill two input frame to prepare */
	memset(test_handle->ipu_handle->inbuf_start[0], random_color, test_handle->ipu_handle->ifr_size);
	memset(test_handle->ipu_handle->inbuf_start[1], random_color, test_handle->ipu_handle->ifr_size);
	//memset(test_handle->ipu_handle->inbuf_start[0]+0xfff, 0x44, 0xff);
	memset(test_handle->ipu_handle->inbuf_start[1]+0xfff, 0x44, 0xff);
	start_angle = rand()%90;
	if (start_angle == 90) start_angle = 89;
	if (start_angle == 0) start_angle = 1;
	printf("Start angle is %d\n", start_angle);

	/* start first frame */
	if((next_update_idx = mxc_ipu_lib_task_buf_update(test_handle->ipu_handle, 0, 0, 0, NULL, &fd_fb)) < 0)
		goto err;

//	while(ctrl_c_rev == 0) {
		usleep(100000);
		/* update frame if only hop block hit the LCD frame */
//		if(update_block_pos(&x, &y, start_angle, test_handle->block_width, lcd_w, lcd_h, fd_fb)) {
//			random_color = (char)(rand()%255);
//			memset(test_handle->ipu_handle->inbuf_start[next_update_idx], random_color,
//					test_handle->ipu_handle->ifr_size);
//			if((next_update_idx = mxc_ipu_lib_task_buf_update(test_handle->ipu_handle, 0, 0, 0, hop_block_output_cb, &fd_fb)) < 0)
//				break;
//		}
//	}

	/* ipu need reset position to 0,0 */
//	pos.x = 0;
//	pos.y = 0;
//	ioctl(fd_fb, MXCFB_SET_OVERLAY_POS, &pos);

//	blank = FB_BLANK_POWERDOWN;
//	if ( ioctl(fd_fb, FBIOBLANK, blank) < 0) {
//		printf("POWERDOWN FB failed!\n");
//		ret = -1;
//		goto done;
//	}
err:
	mxc_ipu_lib_task_uninit(test_handle->ipu_handle);

done:
	if (fd_fb)
		close(fd_fb);
	return ret;
}


int main(int argc, char *argv[])
{

	int ret = 0, next_update_idx = 0, done_cnt = 0, first_time = 1;
	int done_loop = 0, total_cnt = 0;
	ipu_lib_handle_t ipu_handle;
	ipu_test_handle_t test_handle;
	FILE * file_in = NULL;
	struct sigaction act;
	struct timeval begin, end;
	int sec, usec, run_time = 0;

	memset(&ipu_handle, 0, sizeof(ipu_lib_handle_t));    //initialize (clean) ipu_handle
	memset(&test_handle, 0, sizeof(ipu_test_handle_t));  //initialize (clean) test_handle
	test_handle.ipu_handle = &ipu_handle;
	test_handle.mode = OP_NORMAL_MODE;
	test_handle.block_width = 80;

	/*for ctrl-c*/
//	sigemptyset(&act.sa_mask);
//	act.sa_flags = SA_SIGINFO;
//	act.sa_sigaction = ctrl_c_handler;

//	if((ret = sigaction(SIGINT, &act, NULL)) < 0) {
//		printf("install sigal error\n");
//		goto done;
//	}

	system("echo 0,0 > /sys/class/graphics/fb0/pan");

done:
//	fclose(file_in);
//	if (test_handle.file_out)
//		fclose(test_handle.file_out);

	system("echo 0,0 > /sys/class/graphics/fb0/pan");

//rog	ret = run_test_pattern(test_handle.test_pattern, &test_handle);
	ret = run_test_pattern(&test_handle);
	if (ret < 0)
		printf("Error return !\n");
	if (ret == 0)
		printf("Return OK !\n");

	return 0;
}

