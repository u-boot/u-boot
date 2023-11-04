// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2021
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

/* T30 Transformers derive from Cardhu board */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <i2c.h>
#include <log.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include "pinmux-config-transformer.h"

#define TPS65911_I2C_ADDRESS		0x2D

#define TPS65911_VDD1			0x21
#define TPS65911_VDD1_OP		0x22
#define TPS65911_LDO1			0x30
#define TPS65911_LDO2			0x31
#define TPS65911_LDO3			0x37
#define TPS65911_LDO5			0x32
#define TPS65911_LDO6			0x35

#define TPS65911_DEVCTRL		0x3F
#define   DEVCTRL_PWR_OFF_MASK		BIT(7)
#define   DEVCTRL_DEV_ON_MASK		BIT(2)
#define   DEVCTRL_DEV_OFF_MASK		BIT(0)

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp, int flag,
		int argc, char *const argv[])
{
	struct udevice *dev;
	uchar data_buffer[1];
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS65911_I2C_ADDRESS, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return 0;
	}

	ret = dm_i2c_read(dev, TPS65911_DEVCTRL, data_buffer, 1);
	if (ret)
		return ret;

	data_buffer[0] |= DEVCTRL_PWR_OFF_MASK;

	ret = dm_i2c_write(dev, TPS65911_DEVCTRL, data_buffer, 1);
	if (ret)
		return ret;

	data_buffer[0] |= DEVCTRL_DEV_OFF_MASK;
	data_buffer[0] &= ~DEVCTRL_DEV_ON_MASK;

	ret = dm_i2c_write(dev, TPS65911_DEVCTRL, data_buffer, 1);
	if (ret)
		return ret;

	// wait some time and then print error
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
	pinmux_config_pingrp_table(transformer_pinmux_common,
		ARRAY_SIZE(transformer_pinmux_common));

	pinmux_config_drvgrp_table(transformer_padctrl,
		ARRAY_SIZE(transformer_padctrl));

	if (of_machine_is_compatible("asus,tf700t")) {
		pinmux_config_pingrp_table(tf700t_mipi_pinmux,
			ARRAY_SIZE(tf700t_mipi_pinmux));
	}
}

#ifdef CONFIG_MMC_SDHCI_TEGRA
static void tps65911_voltage_init(void)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS65911_I2C_ADDRESS, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return;
	}

	/* TPS659110: LDO1_REG = 3.3v, ACTIVE to SDMMC4 */
	ret = dm_i2c_reg_write(dev, TPS65911_LDO1, 0xc9);
	if (ret)
		log_debug("vcore_emmc set failed: %d\n", ret);

	if (of_machine_is_compatible("asus,tf600t")) {
		/* TPS659110: VDD1_REG = 1.2v, ACTIVE to backlight */
		ret = dm_i2c_reg_write(dev, TPS65911_VDD1_OP, 0x33);
		if (ret)
			log_debug("vdd_bl set failed: %d\n", ret);

		ret = dm_i2c_reg_write(dev, TPS65911_VDD1, 0x0d);
		if (ret)
			log_debug("vdd_bl enable failed: %d\n", ret);

		/* TPS659110: LDO5_REG = 3.3v, ACTIVE to SDMMC1 VIO */
		ret = dm_i2c_reg_write(dev, TPS65911_LDO5, 0x65);
		if (ret)
			log_debug("vdd_usd set failed: %d\n", ret);

		/* TPS659110: LDO6_REG = 1.2v, ACTIVE to MIPI */
		ret = dm_i2c_reg_write(dev, TPS65911_LDO6, 0x11);
		if (ret)
			log_debug("vdd_mipi set failed: %d\n", ret);
	} else {
		/* TPS659110: LDO2_REG = 3.1v, ACTIVE to SDMMC1 */
		ret = dm_i2c_reg_write(dev, TPS65911_LDO2, 0xb9);
		if (ret)
			log_debug("vdd_usd set failed: %d\n", ret);

		/* TPS659110: LDO3_REG = 3.1v, ACTIVE to SDMMC1 VIO */
		ret = dm_i2c_reg_write(dev, TPS65911_LDO3, 0x5d);
		if (ret)
			log_debug("vddio_usd set failed: %d\n", ret);
	}
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the MMC muxes, power rails, etc.
 */
void pin_mux_mmc(void)
{
	/* Bring up uSD and eMMC power */
	tps65911_voltage_init();
}
#endif	/* MMC */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Remove TrustZone nodes */
	fdt_del_node_and_alias(blob, "/firmware");
	fdt_del_node_and_alias(blob, "/reserved-memory/trustzone@bfe00000");

	return 0;
}
#endif
