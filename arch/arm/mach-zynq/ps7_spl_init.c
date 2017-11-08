/*
 * (c) Copyright 2010-2017 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/ps7_init_gpl.h>

__weak int ps7_init(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynq/(platform)/ps7_init_gpl.c, if it exists.
	 */
	return 0;
}

__weak int ps7_post_config(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynq/(platform)/ps7_init_gpl.c, if it exists.
	 */
	return 0;
}
