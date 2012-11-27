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
 * @file mxc_ipudev_test.h
 *
 * @brief IPU device lib test implementation
 *
 * @ingroup IPU
 */
#ifndef __MXC_IPUDEV_TEST_H__
#define __MXC_IPUDEV_TEST_H__

#include <ipu.h>
#include <linux/videodev.h>
#include "mxc_ipu_hl_lib.h"

typedef struct {
	ipu_lib_handle_t * ipu_handle;
	int fcount;
	int loop_cnt;
	int mode;
	int test_pattern;
	int block_width;
	int query_task;
	int kill_task;
	int kill_task_idx;
	char outfile[128];
	FILE * file_out;
	ipu_lib_input_param_t input;
	ipu_lib_output_param_t output;
} ipu_test_handle_t;

enum {
	NO_OV = 0x00,
	IC_GLB_ALP_OV = 0x01,
	IC_LOC_SEP_ALP_OV = 0x02,
	IC_LOC_PIX_ALP_OV = 0x04,
	DP_LOC_SEP_ALP_OV = 0x08,
	COPY_TV = 0x10,
};

extern int parse_config_file(char *file_name, ipu_test_handle_t *test_handle);
//rog int run_test_pattern(int pattern, ipu_test_handle_t * test_handle);
//int run_test_pattern(void);

#endif
