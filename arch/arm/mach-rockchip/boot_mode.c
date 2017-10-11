/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/boot_mode.h>

int setup_boot_mode(void)
{
	void *reg = (void *)CONFIG_ROCKCHIP_BOOT_MODE_REG;
	int boot_mode = readl(reg);

	debug("boot mode %x.\n", boot_mode);

	/* Clear boot mode */
	writel(BOOT_NORMAL, reg);

	switch (boot_mode) {
	case BOOT_FASTBOOT:
		printf("enter fastboot!\n");
		env_set("preboot", "setenv preboot; fastboot usb0");
		break;
	case BOOT_UMS:
		printf("enter UMS!\n");
		env_set("preboot", "setenv preboot; ums mmc 0");
		break;
	}

	return 0;
}
