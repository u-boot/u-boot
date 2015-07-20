/*
 * LG Optimus Black (P970) codename sniper board
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <linux/ctype.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <ns16550.h>
#include <twl4030.h>
#include "sniper.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	.mtype = DDR_STACKED,
	.board_string = "Sniper",
	.nand_string = "MMC"
};

static const struct ns16550_platdata serial_omap_platdata = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK
};

U_BOOT_DEVICE(sniper_serial) = {
	.name = "serial_omap",
	.platdata = &serial_omap_platdata
};

#ifdef CONFIG_SPL_BUILD
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mcfg = HYNIX_V_MCFG_200(256 << 20);
	timings->ctrla = HYNIX_V_ACTIMA_200;
	timings->ctrlb = HYNIX_V_ACTIMB_200;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
	timings->mr = MICRON_V_MR_165;
}
#endif

u32 get_board_rev(void)
{
	/* Sold devices are expected to be at least revision F. */
	return 6;
}

int board_init(void)
{
	/* GPMC init */
	gpmc_init();

	/* MACH number */
	gd->bd->bi_arch_number = 3000;

	/* ATAGs location */
	gd->bd->bi_boot_params = OMAP34XX_SDRC_CS0 + 0x100;

	return 0;
}

int misc_init_r(void)
{
	char reboot_mode[2] = { 0 };

	/* Reboot mode */

	reboot_mode[0] = omap_reboot_mode();
	if (reboot_mode[0] > 0 && isascii(reboot_mode[0])) {
		if (!getenv("reboot-mode"))
			setenv("reboot-mode", (char *)reboot_mode);

		omap_reboot_mode_clear();
	}

	return 0;
}

void set_muxconf_regs(void)
{
	MUX_SNIPER();
}

#ifndef CONFIG_SPL_BUILD
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(1);
}
