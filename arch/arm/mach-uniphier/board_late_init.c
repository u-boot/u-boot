/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <libfdt.h>
#include <nand.h>
#include <linux/io.h>
#include <../drivers/mtd/nand/denali.h>

#include "boot-mode/boot-device.h"

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

static int uniphier_set_fdt_file(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const char *compat;
	char dtb_name[256];
	int buf_len = sizeof(dtb_name);

	if (getenv("fdt_file"))
		return 0;	/* do nothing if it is already set */

	compat = fdt_stringlist_get(gd->fdt_blob, 0, "compatible", 0, NULL);
	if (!compat)
		return -EINVAL;

	/* rip off the vendor prefix "socionext,"  */
	compat = strchr(compat, ',');
	if (!compat)
		return -EINVAL;
	compat++;

	strncpy(dtb_name, compat, buf_len);
	buf_len -= strlen(compat);

	strncat(dtb_name, ".dtb", buf_len);

	return setenv("fdt_file", dtb_name);
}

int board_late_init(void)
{
	puts("MODE:  ");

	switch (spl_boot_device_raw()) {
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
	case BOOT_DEVICE_USB:
		printf("USB Boot\n");
		setenv("bootmode", "usbboot");
		break;
	default:
		printf("Unknown\n");
		break;
	}

	if (uniphier_set_fdt_file())
		printf("fdt_file environment was not set correctly\n");

	return 0;
}
