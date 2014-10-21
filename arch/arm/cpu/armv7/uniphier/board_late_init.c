/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <nand.h>
#include <asm/io.h>
#include <../drivers/mtd/nand/denali.h>

static void nand_denali_wp_disable(void)
{
#ifdef CONFIG_NAND_DENALI
	/*
	 * Since the boot rom enables the write protection for NAND boot mode,
	 * it must be disabled somewhere for "nand write", "nand erase", etc.
	 * The workaround is here to not disturb the Denali NAND controller
	 * driver just for a really SoC-specific thing.
	 */
	void __iomem *denali_reg = (void __iomem *)CONFIG_SYS_NAND_REGS_BASE;

	writel(WRITE_PROTECT__FLAG, denali_reg + WRITE_PROTECT);
#endif
}

static void nand_denali_fixup(void)
{
#if defined(CONFIG_NAND_DENALI) && \
	(defined(CONFIG_MACH_PH1_SLD8) || defined(CONFIG_MACH_PH1_PRO4))
	/*
	 * The Denali NAND controller on some of UniPhier SoCs does not
	 * automatically query the device parameters.  For those SoCs,
	 * some registers must be set after the device is probed.
	 */
	void __iomem *denali_reg = (void __iomem *)CONFIG_SYS_NAND_REGS_BASE;
	struct mtd_info *mtd;
	struct nand_chip *chip;

	if (nand_curr_device < 0 ||
	    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE) {
		/* NAND was not detected. Just return. */
		return;
	}

	mtd = &nand_info[nand_curr_device];
	chip = mtd->priv;

	writel(mtd->erasesize / mtd->writesize, denali_reg + PAGES_PER_BLOCK);
	writel(0, denali_reg + DEVICE_WIDTH);
	writel(mtd->writesize, denali_reg + DEVICE_MAIN_AREA_SIZE);
	writel(mtd->oobsize, denali_reg + DEVICE_SPARE_AREA_SIZE);
	writel(1, denali_reg + DEVICES_CONNECTED);

	/*
	 * chip->scan_bbt in nand_scan_tail() has been skipped.
	 * It should be done in here.
	 */
	chip->scan_bbt(mtd);
#endif
}

int board_late_init(void)
{
	puts("MODE:  ");

	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
		printf("eMMC Boot\n");
		setenv("bootmode", "emmcboot");
		nand_denali_fixup();
		break;
	case BOOT_DEVICE_NAND:
		printf("NAND Boot\n");
		setenv("bootmode", "nandboot");
		nand_denali_wp_disable();
		break;
	case BOOT_DEVICE_NOR:
		printf("NOR Boot\n");
		setenv("bootmode", "norboot");
		nand_denali_fixup();
		break;
	default:
		printf("Unsupported Boot Mode\n");
		return -1;
	}

	return 0;
}
