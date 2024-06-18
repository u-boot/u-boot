/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

/*
 * Name:     ddr3_tip_init_silicon
 * Desc:     initiate silicon parameters
 * Args:
 * Notes:
 * Returns:  required value
 */
int ddr3_silicon_init(void)
{
	int status;
	static int init_done;

	if (init_done == 1)
		return MV_OK;

	status = ddr3_tip_init_a38x(0, 0);
	if (MV_OK != status) {
		printf("DDR3 A38x silicon init - FAILED 0x%x\n", status);
		return status;
	}

	init_done = 1;

	return MV_OK;
}
