/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#include <common.h>
#include <asm/arch/fsp/fsp_support.h>

void update_fsp_upd(struct upd_region *fsp_upd)
{
	/* Override any UPD setting if required */

	/* Uncomment the line below to enable DEBUG message */
	/* fsp_upd->serial_dbgport_type = 1; */

	/* Examples on how to initialize the pointers in UPD region */
	/* fsp_upd->pcd_example = (EXAMPLE_DATA *)&example; */
}
