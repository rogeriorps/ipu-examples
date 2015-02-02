/*
 * Copyright 2012 Freescale Semiconductor, Inc. All Rights Reserved.
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

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>

#include "basic_ex1.h"

void print_info(int fb_id, struct fb_var_screeninfo *fb_var, struct fb_fix_screeninfo *fb_fix)
{

	printf ("xres  		- visible x resolution (pixels): %d\n", fb_var->xres);
	printf ("yres		- visible y resolution: %d\n", fb_var->yres);
	printf ("xres_virtual	- virtual x resolution: %d\n", fb_var->xres_virtual);
	printf ("yres_virtual	- virtual y resolution: %d\n", fb_var->yres_virtual);
	printf ("xoffset	- x offset from virtual to visible resolution: %d\n", fb_var->xoffset);
	printf ("yoffset	- y offset from virtual to visible resolution: %d\n", fb_var->yoffset);
	printf ("bits_per_pixel	- bits per pixel: %d\n", fb_var->bits_per_pixel);
	printf ("grayscale	- != 0 Graylevels instead of colors:  %d\n", fb_var->grayscale);
	printf ("nonstd		- != 0 Non standard pixel format: %d\n", fb_var->nonstd);
	printf ("activate	- Framebuffer active: %d\n", fb_var->activate);
	printf ("height		- height of picture in mm: %d\n", fb_var->height);
	printf ("width		- width of picture in mm: %d\n", fb_var->width);
	printf ("\nTiming: All values in pixclocks, except pixclock\n\n");
	printf ("pixclock	- pixel clock in ps (pico seconds): %d\n", fb_var->pixclock);
	printf ("left_margin	- time from sync to picture: %d\n", fb_var->left_margin);
	printf ("right_margin	- time from picture to sync: %d\n", fb_var->right_margin);
	printf ("upper_margin	- time from sync to picture: %d\n", fb_var->upper_margin);
	printf ("lower_margin	- : %d\n", fb_var->lower_margin);
	printf ("hsync_len	- length of horizontal sync: %d\n", fb_var->hsync_len);
	printf ("vsync_len	- length of vertical sync: %d\n", fb_var->vsync_len);
	printf ("sync		- see FB_SYNC_*: %d\n", fb_var->sync);
	printf ("vmode		- see FB_VMODE_*: %d\n", fb_var->vmode);
	printf ("rotate		- angle we rotate counter clockwise: %d\n", fb_var->rotate);
	printf ("id		- identification: \"" "%c%c%c%c" "%c%c%c%c" "%c%c%c%c" "%c%c%c%c" "\"\n",
	fb_fix->id[0],  fb_fix->id[1],  fb_fix->id[2],  fb_fix->id[3],
	fb_fix->id[4],  fb_fix->id[5],  fb_fix->id[6],  fb_fix->id[7],
	fb_fix->id[8],  fb_fix->id[9],  fb_fix->id[10], fb_fix->id[11],
	fb_fix->id[12], fb_fix->id[13], fb_fix->id[14], fb_fix->id[15] );
	printf ("smem_start	- Start of frame buffer mem (physical address): 0x%08x\n", (unsigned int)fb_fix->smem_start);
	printf ("smem_len	- length of frame buffer mem: %u\n", fb_fix->smem_len);
	printf ("type		- see FB_TYPE_: %d\n", fb_fix->type);
	printf ("type_aux	- Interleave for interleaved Planes: %d\n", fb_fix->type_aux);
	printf ("visual		- see FB_VISUAL_*: %d\n", fb_fix->visual);
	printf ("xpanstep	- zero if no hardware panning: %u\n", fb_fix->xpanstep);
	printf ("ypanstep	- zero if no hardware panning: %u\n", fb_fix->ypanstep);
	printf ("ywrapstep	- zero if no hardware ywrap: %u\n", fb_fix->ywrapstep);
	printf ("line_length	- length of a line in bytes: %d\n", fb_fix->line_length);
	printf ("mmio_start	- Start of Memory Mapped I/O (physical address): 0x%08x\n", (unsigned int)fb_fix->mmio_start);
	printf ("mmio_len	- length of Memory Mapped I/O: %d\n", fb_fix->mmio_len);
	printf ("accel		- indicate to driver which specific chip/card we have: %d\n", fb_fix->accel);
}

int main(int argc, char *argv[])
{

	int ret = 0, fb_id = 0;
	int fd_fb;
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;

	char * framebuffer[] = {
	    "/dev/fb0",
	    "/dev/fb1",
	    "/dev/fb2",
	    "/dev/fb3",
	    "/dev/fb4",
	    "/dev/fb5"
	};

	for (fb_id = 0; fb_id < 6 ; fb_id++) {

		printf ("\n\nOpening %s\n\n", framebuffer[fb_id]);

		/* Open framebuffer device */
		if ((fd_fb = open(framebuffer[fb_id], O_RDWR, 0)) < 0) {
			printf("Unable to open %s\n", framebuffer[fb_id]);
			ret = -1;
			goto done;
		}

		/* Get fix info from framebuffer */
		if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
			printf("Get FB fix info failed!\n");
			ret = -1;
			goto done;
		}

		/* Get var info from framebuffer */
		if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
			printf("Get FB var info failed!\n");
			ret = -1;
			goto done;
		}

		/* Print all information */
		print_info(fb_id, &fb_var, &fb_fix);
		
		/* Close the frambuffer device */
		if (fd_fb)
			close(fd_fb);
	}

done:

	if (fd_fb)
		close(fd_fb);

	return 0;
}

