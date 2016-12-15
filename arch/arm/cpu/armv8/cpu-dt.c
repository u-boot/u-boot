/*
 * Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/psci.h>
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#include <asm/armv8/sec_firmware.h>
#endif

int psci_update_dt(void *fdt)
{
#ifdef CONFIG_MP
#if defined(CONFIG_ARMV8_PSCI)
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
	/*
	 * If the PSCI in SEC Firmware didn't work, avoid to update the
	 * device node of PSCI. But still return 0 instead of an error
	 * number to support detecting PSCI dynamically and then switching
	 * the SMP boot method between PSCI and spin-table.
	 */
	if (sec_firmware_support_psci_version() == 0xffffffff)
		return 0;
#endif
	fdt_psci(fdt);
#endif
#endif
	return 0;
}
