/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ENV_IS_IN_MMC
int board_mmc_get_env_dev(int devno)
{
	return devno - 1;
}
#endif

#ifdef CONFIG_SPL_BUILD
#include <spl.h>

#ifdef CONFIG_ENV_IS_IN_MMC
void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 boot_dev = BOOT_DEVICE_MMC1;

	switch ((bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC1 */
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC */
		boot_dev = BOOT_DEVICE_MMC2;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		printf("Wrong board boot order\n");
		break;
	}

	spl_boot_list[0] = boot_dev;
}
#endif
#endif /* CONFIG_SPL_BUILD */
