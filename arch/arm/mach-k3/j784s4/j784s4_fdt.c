// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * J784S4: SoC specific initialization
 *
 * Copyright (C) 2023-2024 Texas Instruments Incorporated - https://www.ti.com/
 *	Apurva Nandan <a-nandan@ti.com>
 */

#include <asm/arch/k3-common-fdt.h>
#include <fdt_support.h>

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return fdt_fixup_msmc_ram_k3(blob);
}
