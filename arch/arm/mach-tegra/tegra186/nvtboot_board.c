/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/arch/tegra.h>

extern unsigned long nvtboot_boot_x0;

/*
 * Attempt to use /chosen/nvidia,ether-mac in the nvtboot DTB to U-Boot's
 * ethaddr environment variable if possible.
 */
static int set_ethaddr_from_nvtboot(void)
{
	const void *nvtboot_blob = (void *)nvtboot_boot_x0;
	int ret, node, len;
	const u32 *prop;

	/* Already a valid address in the environment? If so, keep it */
	if (getenv("ethaddr"))
		return 0;

	node = fdt_path_offset(nvtboot_blob, "/chosen");
	if (node < 0) {
		printf("Can't find /chosen node in nvtboot DTB\n");
		return node;
	}
	prop = fdt_getprop(nvtboot_blob, node, "nvidia,ether-mac", &len);
	if (!prop) {
		printf("Can't find nvidia,ether-mac property in nvtboot DTB\n");
		return -ENOENT;
	}

	ret = setenv("ethaddr", (void *)prop);
	if (ret) {
		printf("Failed to set ethaddr from nvtboot DTB: %d\n", ret);
		return ret;
	}

	return 0;
}

int tegra_soc_board_init_late(void)
{
	/* Ignore errors here; not all cases care about Ethernet addresses */
	set_ethaddr_from_nvtboot();

	return 0;
}
