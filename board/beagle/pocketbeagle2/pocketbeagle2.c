// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM625 PocketBeagle 2
 * https://www.beagleboard.org/boards/pocketbeagle-2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2025 Robert Nelson, BeagleBoard.org Foundation
 */

#include <env.h>
#include <fdt_support.h>
#include <spl.h>

#include <asm/arch/hardware.h>
#include <asm/arch/k3-ddr.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/*
 * The U-Boot stack can collide with some dt reservations in the 512MB
 * configuration. Because of this, we need to relocate to just below our dt
 * reservations. The magic number below corresponds to the lowest value from
 * the block of conflicting reservations in the current device tree.
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	if (gd->ram_size == SZ_512M)
		return 0x9c800000;

	return gd->ram_top;
}

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	char fdtfile[50];

	snprintf(fdtfile, sizeof(fdtfile), "%s.dtb",
		 CONFIG_DEFAULT_DEVICE_TREE);

	env_set("fdtfile", fdtfile);

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */
