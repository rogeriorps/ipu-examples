/*
 * Copyright 2013 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

/*
 * basic_ex1.c: This example application changes the display processor
 * gamma corection parameters. The input and output parameters are
 * stored on gin and gout variables. Constk and  slopek values are
 * calculated according Processor's Reference Manual.
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
#include "framebuffer_ex1.h"

int main(int argc, char *argv[])
{
	int fd_fb = 0;
	struct mxcfb_gamma fb_gamma;
	int blank, i, ret;
	int constk[16], slopek[16];

	int gin[16] = {0,2,4,8,16,32,64,96,128,160,192,224,256,288,320,352};

	int gout[16] = {0,2,4,8,16,32,64,96,128,160,192,224,255,255,255,255};

	if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = -1;
		goto done;
	}

	// Unblank the display
	blank = FB_BLANK_UNBLANK;
	ioctl(fd_fb, FBIOBLANK, blank);

	// Calculation according reference manual
	constk[0] = gout[0];
	constk[1] = 2*gout[1]-gout[2];
	constk[2] = 2*gout[2]-gout[3];
	constk[3] = 2*gout[3]-gout[4];
	constk[4] = 2*gout[4]-gout[5];
	constk[5] = gout[5];
	constk[6] = gout[6];
	constk[7] = gout[7];
	constk[8] = gout[8];
	constk[9] = gout[9];
	constk[10] = gout[10];
	constk[11] = gout[11];
	constk[12] = gout[12];
	constk[13] = gout[13];
	constk[14] = gout[14];
	constk[15] = gout[15];

	slopek[0] = 16*(gout[1]-gout[0]);
	slopek[1] = 16*(gout[2]-gout[1]);
	slopek[2] = 8*(gout[3]-gout[2]);
	slopek[3] = 4*(gout[4]-gout[3]);
	slopek[4] = 2*(gout[5]-gout[4]);
	slopek[5] = gout[6]-gout[5];
	slopek[6] = gout[7]-gout[6];
	slopek[7] = gout[8]-gout[7];
	slopek[8] = gout[9]-gout[8];
	slopek[9] = gout[10]-gout[9];
	slopek[10] = gout[11]-gout[10];
	slopek[11] = gout[12]-gout[11];
	slopek[12] = gout[13]-gout[12];
	slopek[13] = gout[14]-gout[13];
	slopek[14] = gout[15]-gout[14];
	slopek[15] = 255-gout[15];

	fb_gamma.enable = 1;

	/* initialize elements of array n to 0 */         
   	for ( i = 0; i < 16; i++ ) {
      		fb_gamma.constk[i] = constk[i];
		printf("constk %d = %d\n", i, constk[i]);
		fb_gamma.slopek[i] = slopek[i];
		printf("slopek %d = %d\n", i, slopek[i]);
  	}	

	if ( ioctl(fd_fb, MXCFB_SET_GAMMA, &fb_gamma) < 0) {
		printf("Wrong gamma seting!\n");
		ret = -1;
		goto done;
	}

done:
	if (fd_fb)
		close(fd_fb);
	return 0;
}
