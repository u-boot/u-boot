// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * RISC-V-specific handling of firmware FDT
 */

#include <asm/global_data.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int board_fdt_blob_setup(void **fdtp)
{
	if (!gd->arch.firmware_fdt_addr)
		return -EEXIST;

	*fdtp = (ulong *)(uintptr_t)gd->arch.firmware_fdt_addr;

	return 0;
}
