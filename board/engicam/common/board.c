/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <asm/arch/sys_proto.h>

#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ENV_IS_IN_MMC
static void mmc_late_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	env_set_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw", dev_no);
	env_set("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
#endif

int board_late_init(void)
{
	switch ((imx6_src_get_boot_mode() & IMX6_BMODE_MASK) >>
			IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
#ifdef CONFIG_ENV_IS_IN_MMC
		mmc_late_init();
#endif
		env_set("modeboot", "mmcboot");
		break;
	case IMX6_BMODE_NAND:
		env_set("modeboot", "nandboot");
		break;
	default:
		env_set("modeboot", "");
		break;
	}

	if (is_mx6ul())
		env_set("console", "ttymxc0");
	else
		env_set("console", "ttymxc3");

	setenv_fdt_file();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#ifdef CONFIG_VIDEO_IPUV3
	setup_display();
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}
