/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/mbus.h>

DECLARE_GLOBAL_DATA_PTR;

/* Base addresses for the external device chip selects */
#define DEV_CS0_BASE		0xe0000000
#define DEV_CS1_BASE		0xe1000000
#define DEV_CS2_BASE		0xe2000000
#define DEV_CS3_BASE		0xe3000000

/* Needed for dynamic (board-specific) mbus configuration */
extern struct mvebu_mbus_state mbus_state;

int board_early_init_f(void)
{
	/*
	 * Don't configure MPP (pin multiplexing) and GPIO here,
	 * its already done in bin_hdr
	 */

	/*
	 * Setup some board specific mbus address windows
	 */
	mbus_dt_setup_win(&mbus_state, DEV_CS0_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS0);
	mbus_dt_setup_win(&mbus_state, DEV_CS1_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS1);
	mbus_dt_setup_win(&mbus_state, DEV_CS2_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS2);
	mbus_dt_setup_win(&mbus_state, DEV_CS3_BASE, 16 << 20,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_DEV_CS3);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: maxBCM\n");

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
/* Configure and enable MV88E6185 switch */
void reset_phy(void)
{
	u16 devadr = CONFIG_PHY_BASE_ADDR;
	char *name = "neta0";
	u16 reg;

	if (miiphy_set_current_dev(name))
		return;

	/* todo: fill this with the real setup / config code */

	printf("88E6185 Initialized on %s\n", name);
}
#endif /* CONFIG_RESET_PHY_R */
