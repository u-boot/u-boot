// SPDX-License-Identifier: GPL-2.0+
/*
 * Common SPL code for Qualcomm Snapdragon boards.
 *
 * Copyright (c) 2026 Michael Srba <Michael.Srba@seznam.cz>
 */

#include <hang.h>
#include <spl.h>

/* in SPL, we always use internal DT */
int board_fdt_blob_setup(void **fdtp)
{
	return -EEXIST;
}

int board_init(void)
{
	return 0;
}

__weak void reset_cpu(void)
{
	/* This should currently not get called in non-error paths, so just hang */
	printf("reset_cpu called, going to hang()\n");
	hang();
}

u32 spl_boot_device(void)
{
	/* TODO: check boot reason to support UFS and sdcard */
	u32 boot_device = BOOT_DEVICE_DFU;

	return boot_device;
}
