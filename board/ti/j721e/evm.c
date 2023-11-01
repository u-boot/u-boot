// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for J721E EVM
 *
 * Copyright (C) 2018-2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>

#include "evm.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	if (IS_ENABLED(CONFIG_PHYS_64BIT))
		gd->ram_size = 0x100000000;
	else
		gd->ram_size = 0x80000000;

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/* Limit RAM used by U-Boot to the DDR low region */
	if (IS_ENABLED(CONFIG_PHYS_64BIT) && gd->ram_top > 0x100000000)
		return 0x100000000;

	return gd->ram_top;
}

int dram_init_banksize(void)
{
	/* Bank 0 declares the memory available in the DDR low region */
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = 0x80000000;
	gd->ram_size = 0x80000000;

	if (IS_ENABLED(CONFIG_PHYS_64BIT)) {
		/* Bank 1 declares the memory available in the DDR high region */
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE1;
		gd->bd->bi_dram[1].size = 0x80000000;
		gd->ram_size = 0x100000000;
	}

	return 0;
}

void configure_serdes_torrent(void)
{
	struct udevice *dev;
	struct phy serdes;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PHY,
					  DM_DRIVER_GET(torrent_phy_provider),
					  &dev);
	if (ret) {
		printf("Torrent init failed:%d\n", ret);
		return;
	}

	serdes.dev = dev;
	serdes.id = 0;

	ret = generic_phy_init(&serdes);
	if (ret) {
		printf("phy_init failed!!: %d\n", ret);
		return;
	}

	ret = generic_phy_power_on(&serdes);
	if (ret) {
		printf("phy_power_on failed!!: %d\n", ret);
		return;
	}
}

void configure_serdes_sierra(void)
{
	struct udevice *dev, *link_dev;
	struct phy link;
	int ret, count, i;
	int link_count = 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(sierra_phy_provider),
					  &dev);
	if (ret) {
		printf("Sierra init failed:%d\n", ret);
		return;
	}

	count = device_get_child_count(dev);
	for (i = 0; i < count; i++) {
		ret = device_get_child(dev, i, &link_dev);
		if (ret) {
			printf("probe of sierra child node %d failed: %d\n", i, ret);
			return;
		}
		if (link_dev->driver->id == UCLASS_PHY) {
			link.dev = link_dev;
			link.id = link_count++;

			ret = generic_phy_power_on(&link);
			if (ret) {
				printf("phy_power_on failed!!: %d\n", ret);
				return;
			}
		}
	}
}

int board_serdes_late_init(void)
{
	if (IS_ENABLED(CONFIG_PHY_CADENCE_TORRENT))
		configure_serdes_torrent();

	if (IS_ENABLED(CONFIG_PHY_CADENCE_SIERRA))
		configure_serdes_sierra();

	return 0;
}

int board_late_init(void)
{
	int ret;

	ret = variant_board_late_init();
	if (ret)
		return ret;

	board_serdes_late_init();

	return 0;
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_ESM_K3)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(k3_esm), &dev);
		if (ret)
			printf("ESM init failed: %d\n", ret);
	}

	if (IS_ENABLED(CONFIG_ESM_PMIC)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(pmic_esm),
						  &dev);
		if (ret)
			printf("ESM PMIC init failed: %d\n", ret);
	}

	variant_spl_board_init();
}
