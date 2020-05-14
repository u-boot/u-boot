// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <asm/sections.h>
#include <asm/system.h>

/*
 * Force data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
static unsigned long nt_fw_dtb __section(".data");

/*
 * Save the FDT address provided by TF-A in r2 at boot time
 * This function is called from start.S
 */
void save_boot_params(unsigned long r0, unsigned long r1, unsigned long r2,
		      unsigned long r3)
{
	nt_fw_dtb = r2;

	save_boot_params_ret();
}

/*
 * Use the saved FDT address provided by TF-A at boot time (NT_FW_CONFIG =
 * Non Trusted Firmware configuration file) when the pointer is valid
 */
void *board_fdt_blob_setup(void)
{
	debug("%s: nt_fw_dtb=%lx\n", __func__, nt_fw_dtb);

	/* use external device tree only if address is valid */
	if (nt_fw_dtb >= STM32_DDR_BASE) {
		if (fdt_magic(nt_fw_dtb) == FDT_MAGIC)
			return (void *)nt_fw_dtb;
		debug("%s: DTB not found.\n", __func__);
	}
	debug("%s: fall back to builtin DTB, %p\n", __func__, &_end);

	return (void *)&_end;
}
