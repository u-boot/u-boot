// SPDX-License-Identifier: GPL-2.0-only OR MIT
/*
 * Copyright (C) 2025 PHYTEC Messtechnik GmbH
 * Author: Dominik Haller <d.haller@phytec.de>
 *
 * https://www.phytec.eu/en/produkte/system-on-modules/phycore-am68x-tda4x/
 */

#include <env.h>
#include <fdt_support.h>
#include <generic-phy.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <spl.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/root.h>
#include <asm/arch/k3-ddr.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif

	return gd->ram_top;
}

int dram_init(void)
{
	s32 ret;

	ret = fdtdec_setup_mem_size_base_lowest();
	if (ret)
		printf("Error setting up mem size and base. %d\n", ret);

	return ret;
}

int dram_init_banksize(void)
{
	s32 ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		printf("Error setting up memory banksize. %d\n", ret);

	return ret;
}

#if defined(CONFIG_XPL_BUILD)
void spl_perform_board_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS)) {
		if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
			fixup_ddr_driver_for_ecc(spl_image);
	} else {
		fixup_memory_node(spl_image);
	}
}
#endif

#define TPS6594_I2C_WD_ADDR	0x12
#define WD_THR_CFG_REG		0x09
#define WD_EN			BIT(6)

int tps6594_disable_watchdog(void)
{
	struct udevice *i2c_bus, *i2c_dev;
	int ret;
	u8 val;

	ret = uclass_get_device_by_name(UCLASS_I2C, "i2c@40b00000", &i2c_bus);
	if (ret)
		return ret;

	ret = dm_i2c_probe(i2c_bus, TPS6594_I2C_WD_ADDR, 0, &i2c_dev);
	if (ret)
		return ret;

	ret = dm_i2c_reg_read(i2c_dev, WD_THR_CFG_REG);
	if (ret < 0)
		return ret;

	val = (u8)ret;
	val &= ~WD_EN;
	ret = dm_i2c_reg_write(i2c_dev, WD_THR_CFG_REG, val);

	return ret;
}

int board_init(void)
{
	int ret;

	ret = tps6594_disable_watchdog();
	if (ret)
		printf("disabling TPS6594 watchdog failed: %d\n", ret);

	return 0;
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_ESM_K3)) {
		const char * const esms[] = {"esm@700000", "esm@40800000", "esm@42080000"};

		for (int i = 0; i < ARRAY_SIZE(esms); ++i) {
			ret = uclass_get_device_by_name(UCLASS_MISC, esms[i],
							&dev);
			if (ret) {
				printf("MISC init for %s failed: %d\n", esms[i], ret);
				break;
			}
		}
	}

	if (IS_ENABLED(CONFIG_ESM_PMIC) && ret == 0) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(pmic_esm),
						  &dev);
		if (ret)
			printf("ESM PMIC init failed: %d\n", ret);
	}
}
