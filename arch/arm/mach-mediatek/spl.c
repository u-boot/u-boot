// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <hang.h>
#include <init.h>
#include <spl.h>

__weak int mtk_soc_early_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		hang();

	/* enable console uart printing */
	preloader_console_init();

	/* soc early initialization */
	ret = mtk_soc_early_init();
	if (ret)
		hang();
}

u32 spl_boot_device(void)
{
	if (CONFIG_IS_ENABLED(SPI))
		return BOOT_DEVICE_SPI;
	if (CONFIG_IS_ENABLED(MMC))
		return BOOT_DEVICE_MMC1;
	if (CONFIG_IS_ENABLED(NAND_SUPPORT))
		return BOOT_DEVICE_NAND;
	if (CONFIG_IS_ENABLED(NOR_SUPPORT))
		return BOOT_DEVICE_NOR;

	return BOOT_DEVICE_NONE;
}
