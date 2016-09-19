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

/*!
 * @file basic_ex1.h
 *
 * @brief IPU rotation example for i.MX6
 *
 */
#ifndef __MXC_IPUDEV_TEST_H__
#define __MXC_IPUDEV_TEST_H__

#include <linux/ipu.h>

typedef struct {
	struct ipu_task task;
	int fcount;
	int loop_cnt;
	int show_to_fb;
	char outfile[128];
} ipu_test_handle_t;

#endif
