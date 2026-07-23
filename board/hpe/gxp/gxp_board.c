// SPDX-License-Identifier: GPL-2.0+
/*
 * GXP board support
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 *
 * Copyright (C) 2026 9elements GmbH
 */

#include <linux/bitfield.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass.h>
#include <fdt_support.h>
#include <init.h>
#include <sysinfo.h>

DECLARE_GLOBAL_DATA_PTR;

#define ECHI_CMD 0xcefe0010

#define GXP_XREG_BASE		0xd1000000
#define GXP_XREG_SERVER_ID	GENMASK(23, 8)

static u16 gxp_get_server_id(void)
{
	return FIELD_GET(GXP_XREG_SERVER_ID, readl(GXP_XREG_BASE));
}

int checkboard(void)
{
	struct udevice *dev;
	char str[64];
	u16 server_id;
	int ret;

	server_id = gxp_get_server_id();
	printf("Server ID: 0x%04x\n", server_id);

	ret = sysinfo_get(&dev);
	if (ret)
		return 0;

	ret = sysinfo_detect(dev);
	if (ret)
		return 0;

	ret = sysinfo_get_str(dev, SYSID_SM_SYSTEM_SERIAL, sizeof(str), str);
	if (!ret)
		printf("Serial Number: %s\n", str);

	ret = sysinfo_get_str(dev, SYSID_SM_BASEBOARD_PRODUCT, sizeof(str), str);
	if (!ret)
		printf("Part Number: %s\n", str);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	char expected[16];
	u16 server_id;

	server_id = gxp_get_server_id();
	snprintf(expected, sizeof(expected), "gxp-0x%04x", server_id);

	if (strstr(name, expected))
		return 0;

	return -1;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct udevice *dev;
	char str[64];
	int ret;

	ret = sysinfo_get(&dev);
	if (ret)
		return 0;

	ret = sysinfo_detect(dev);
	if (ret)
		return 0;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MANUFACTURER, sizeof(str), str);
	if (!ret)
		fdt_setprop_string(blob, 0, "manufacturer", str);

	ret = sysinfo_get_str(dev, SYSID_SM_SYSTEM_SERIAL, sizeof(str), str);
	if (!ret)
		fdt_setprop_string(blob, 0, "serial-number", str);

	ret = sysinfo_get_str(dev, SYSID_SM_BASEBOARD_PRODUCT, sizeof(str), str);
	if (!ret)
		fdt_setprop_string(blob, 0, "part-number", str);

	return 0;
}

int board_init(void)
{
	writel(0x00080002, ECHI_CMD);

	return 0;
}

int dram_init(void)
{
	if (IS_ENABLED(CONFIG_TARGET_GXP)) {
		if (IS_ENABLED(CONFIG_GXP_ECC)) {
			/* 0x0f800000 */
			gd->ram_size = SZ_128M + SZ_64M + SZ_32M + SZ_16M + SZ_8M;
		} else {
			/* 0x1f000000 */
			gd->ram_size = SZ_256M + SZ_128M + SZ_64M + SZ_32M + SZ_16M;
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_64MB)) {
			if (IS_ENABLED(CONFIG_GXP_ECC)) {
				/* 0x0c000000 */
				gd->ram_size = SZ_128M + SZ_64M;
			} else {
				/* 0x18000000 */
				gd->ram_size = SZ_256M + SZ_128M;
			}
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_32MB)) {
			if (IS_ENABLED(CONFIG_GXP_ECC)) {
				/* 0x0e000000 */
				gd->ram_size = SZ_128M + SZ_64M + SZ_32M;
			} else {
				/* 0x1c000000 */
				gd->ram_size = SZ_256M + SZ_128M + SZ_64M;
			}
		}
	}

	if (IS_ENABLED(CONFIG_TARGET_GXP2)) {
		/* 0x1b200000 */
		gd->ram_size = SZ_256M + SZ_128M + SZ_32M + SZ_16M + SZ_2M;
		if (IS_ENABLED(CONFIG_GXP_VROM_64MB)) {
			/* 0x14000000 */
			gd->ram_size = SZ_256M + SZ_64M;
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_32MB)) {
			/* 0x18000000 */
			gd->ram_size = SZ_256M + SZ_128M;
		}
	}

	return 0;
}
