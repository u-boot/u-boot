/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <libfdt.h>
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

struct uniphier_fdt_file {
	const char *compatible;
	const char *file_name;
};

static const struct uniphier_fdt_file uniphier_fdt_files[] = {
	{ "socionext,ph1-ld4-ref", "uniphier-ph1-ld4-ref.dtb", },
	{ "socionext,ph1-ld6b-ref", "uniphier-ph1-ld6b-ref.dtb", },
	{ "socionext,ph1-ld10-ref", "uniphier-ph1-ld10-ref.dtb", },
	{ "socionext,ph1-pro4-ref", "uniphier-ph1-pro4-ref.dtb", },
	{ "socionext,ph1-pro5-4kbox", "uniphier-ph1-pro5-4kbox.dtb", },
	{ "socionext,ph1-sld3-ref", "uniphier-ph1-sld3-ref.dtb", },
	{ "socionext,ph1-sld8-ref", "uniphier-ph1-sld8-ref.dtb", },
	{ "socionext,proxstream2-gentil", "uniphier-proxstream2-gentil.dtb", },
	{ "socionext,proxstream2-vodka", "uniphier-proxstream2-vodka.dtb", },
};

static void uniphier_set_fdt_file(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int i;

	/* lookup DTB file name based on the compatible string */
	for (i = 0; i < ARRAY_SIZE(uniphier_fdt_files); i++) {
		if (!fdt_node_check_compatible(gd->fdt_blob, 0,
					uniphier_fdt_files[i].compatible)) {
			setenv("fdt_file", uniphier_fdt_files[i].file_name);
			return;
		}
	}
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

	uniphier_set_fdt_file();

	return 0;
}
