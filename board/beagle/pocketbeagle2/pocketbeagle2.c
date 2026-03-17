// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM625 PocketBeagle 2
 * https://www.beagleboard.org/boards/pocketbeagle-2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2025 Robert Nelson, BeagleBoard.org Foundation
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

/**
 * board_setup_dest_addr - Adjust the relocation address for this device
 *
 * The u-boot stack can collide with some dt reservations in the 512MB
 * configuration. Because of this, we need to relocate u-boot to just below our
 * dt reservations. This is the lowest remoteproc related reservation in dt
 * currently.
 */
int board_setup_dest_addr(void)
{
	gd->relocaddr = 0x9c800000;
	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
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
