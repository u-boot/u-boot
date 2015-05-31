/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <nand.h>
#include <linux/io.h>
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

int board_late_init(void)
{
	puts("MODE:  ");

	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
		printf("eMMC Boot\n");
		setenv("bootmode", "emmcboot");
		break;
	case BOOT_DEVICE_NAND:
		printf("NAND Boot\n");
		setenv("bootmode", "nandboot");
		nand_denali_wp_disable();
		break;
	case BOOT_DEVICE_NOR:
		printf("NOR Boot\n");
		setenv("bootmode", "norboot");
		break;
	default:
		printf("Unsupported Boot Mode\n");
		return -1;
	}

	return 0;
}
