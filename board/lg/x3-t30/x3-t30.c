// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <i2c.h>
#include <log.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/fuse.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include "pinmux-config-x3.h"

#define MAX77663_I2C_ADDR		0x1C

#define MAX77663_REG_SD2		0x18
#define MAX77663_REG_LDO2		0x27
#define MAX77663_REG_LDO3		0x29
#define MAX77663_REG_LDO5		0x2D
#define MAX77663_REG_ONOFF_CFG1		0x41
#define   ONOFF_PWR_OFF			BIT(1)

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp, int flag,
		int argc, char *const argv[])
{
	struct udevice *dev;
	uchar data_buffer[1];
	int ret;

	ret = i2c_get_chip_for_busnum(0, MAX77663_I2C_ADDR, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return 0;
	}

	ret = dm_i2c_read(dev, MAX77663_REG_ONOFF_CFG1, data_buffer, 1);
	if (ret)
		return ret;

	data_buffer[0] |= ONOFF_PWR_OFF;

	ret = dm_i2c_write(dev, MAX77663_REG_ONOFF_CFG1, data_buffer, 1);
	if (ret)
		return ret;

	/* wait some time and then print error */
	mdelay(5000);

	printf("Failed to power off!!!\n");
	return 1;
}
#endif

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(tegra3_x3_pinmux_common,
		ARRAY_SIZE(tegra3_x3_pinmux_common));

#ifdef CONFIG_DEVICE_P880
	pinmux_config_pingrp_table(tegra3_p880_pinmux,
		ARRAY_SIZE(tegra3_p880_pinmux));
#endif

#ifdef CONFIG_DEVICE_P895
	pinmux_config_pingrp_table(tegra3_p895_pinmux,
		ARRAY_SIZE(tegra3_p895_pinmux));
#endif
}

#ifdef CONFIG_MMC_SDHCI_TEGRA
static void max77663_voltage_init(void)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, MAX77663_I2C_ADDR, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return;
	}

	/* 0x60 for 1.8v, bit7:0 = voltage */
	ret = dm_i2c_reg_write(dev, MAX77663_REG_SD2, 0x60);
	if (ret)
		log_debug("vdd_1v8_vio set failed: %d\n", ret);

	/* 0xF2 for 3.30v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	ret = dm_i2c_reg_write(dev, MAX77663_REG_LDO2, 0xF2);
	if (ret)
		log_debug("avdd_usb set failed: %d\n", ret);

	/* 0xEC for 3.00v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	ret = dm_i2c_reg_write(dev, MAX77663_REG_LDO3, 0xEC);
	if (ret)
		log_debug("vdd_usd set failed: %d\n", ret);

	/* 0xE9 for 2.85v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	ret = dm_i2c_reg_write(dev, MAX77663_REG_LDO5, 0xE9);
	if (ret)
		log_debug("vcore_emmc set failed: %d\n", ret);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the MMC muxes, power rails, etc.
 */
void pin_mux_mmc(void)
{
	/* Bring up uSD and eMMC power */
	max77663_voltage_init();
}
#endif	/* MMC */

int nvidia_board_init(void)
{
	/* Set up panel bridge clocks */
	clock_start_periph_pll(PERIPH_ID_EXTPERIPH3, CLOCK_ID_PERIPH,
			       24 * 1000000);
	clock_external_output(3);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* First 3 bytes refer to LG vendor */
	u8 btmacaddr[6] = { 0x00, 0x00, 0x00, 0xD0, 0xC9, 0x88 };

	/* Generate device 3 bytes based on chip sd */
	u64 bt_device = tegra_chip_uid() >> 24ull;

	btmacaddr[0] =  (bt_device >> 1 & 0x0F) |
			(bt_device >> 5 & 0xF0);
	btmacaddr[1] =  (bt_device >> 11 & 0x0F) |
			(bt_device >> 17 & 0xF0);
	btmacaddr[2] =  (bt_device >> 23 & 0x0F) |
			(bt_device >> 29 & 0xF0);

	/* Set BT MAC address */
	fdt_find_and_setprop(blob, "/serial@70006200/bluetooth",
			     "local-bd-address", btmacaddr, 6, 1);

	/* Remove TrustZone nodes */
	fdt_del_node_and_alias(blob, "/firmware");
	fdt_del_node_and_alias(blob, "/reserved-memory/trustzone@bfe00000");

	return 0;
}
#endif
