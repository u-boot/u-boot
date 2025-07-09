// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/k3-common-fdt.h>
#include <asm/hardware.h>
#include <fdt_support.h>

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_reserved(blob, "tfa", CONFIG_K3_ATF_LOAD_ADDR, 0x80000);
	fdt_fixup_reserved(blob, "optee", CONFIG_K3_OPTEE_LOAD_ADDR, 0x1800000);

	return 0;
}
