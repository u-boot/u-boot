// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/k3-common-fdt.h>
#include <asm/hardware.h>
#include <fdt_support.h>

static int fdt_fixup_a53_nodes(void *dt)
{
	int nr = k3_get_core_nr();
	char path[32];

	/* remove the fused out cpus */
	for (; nr < 4; nr++) {
		snprintf(path, sizeof(path), "/cpus/cpu@%d", nr);
		fdt_del_node_path(dt, path);

		/* remove the cpu from the cluster */
		snprintf(path, sizeof(path), "/cpus/cpu-map/cluster0/core%d", nr);
		fdt_del_node_path(dt, path);

		/* remove the watchdog for the cpu */
		snprintf(path, sizeof(path), "/bus@f0000/watchdog@e0%d0000", nr);
		fdt_del_node_path(dt, path);
	}

	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_a53_nodes(blob);
	return fdt_fixup_reserved(blob);
}
