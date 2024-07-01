// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#include <fdt_support.h>

#include "../common_fdt.h"

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return fdt_fixup_msmc_ram_k3(blob);
}
