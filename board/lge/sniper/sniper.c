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
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
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

static struct musb_hdrc_config musb_config = {
	.multipoint = 1,
	.dyn_fifo = 1,
	.num_eps = 16,
	.ram_bits = 12
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type	= MUSB_INTERFACE_ULPI,
};

static struct musb_hdrc_platform_data musb_platform_data = {
	.mode = MUSB_PERIPHERAL,
	.config = &musb_config,
	.power = 100,
	.platform_ops = &omap2430_ops,
	.board_data = &musb_board_data,
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
	char serial_string[17] = { 0 };
	char reboot_mode[2] = { 0 };
	u32 dieid[4] = { 0 };

	/* Power button reset init */

	twl4030_power_reset_init();

	/* Reboot mode */

	reboot_mode[0] = omap_reboot_mode();
	if (reboot_mode[0] > 0 && isascii(reboot_mode[0])) {
		if (!getenv("reboot-mode"))
			setenv("reboot-mode", (char *)reboot_mode);

		omap_reboot_mode_clear();
	}

	/* Serial number */

	get_dieid((u32 *)&dieid);

	if (!getenv("serial#")) {
		snprintf(serial_string, sizeof(serial_string),
			"%08x%08x", dieid[0], dieid[3]);

		setenv("serial#", serial_string);
	}

	/* MUSB */

	musb_register(&musb_platform_data, &musb_board_data, (void *)MUSB_BASE);

	return 0;
}

void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	unsigned long long serial;

	serial_string = getenv("serial#");

	if (serial_string) {
		serial = simple_strtoull(serial_string, NULL, 16);

		serialnr->high = (unsigned int) (serial >> 32);
		serialnr->low = (unsigned int) (serial & 0xffffffff);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}

int fb_set_reboot_flag(void)
{
	return omap_reboot_mode_store('b');
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
