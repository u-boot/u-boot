// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2021
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

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
#include "pinmux-config-endeavoru.h"

#define TPS80032_CTL1_I2C_ADDR		0x48
#define TPS80032_PHOENIX_DEV_ON		0x25
#define   DEVOFF			BIT(0)
#define TPS80032_LDO1_CFG_STATE		0x9E
#define TPS80032_LDO1_CFG_VOLTAGE	0x9F

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp, int flag,
		int argc, char *const argv[])
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS80032_CTL1_I2C_ADDR, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return 0;
	}

	ret = dm_i2c_reg_write(dev, TPS80032_PHOENIX_DEV_ON, DEVOFF);
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
	pinmux_config_pingrp_table(endeavoru_pinmux_common,
		ARRAY_SIZE(endeavoru_pinmux_common));
}

#ifdef CONFIG_MMC_SDHCI_TEGRA
static void tps80032_voltage_init(void)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS80032_CTL1_I2C_ADDR, 1, &dev);
	if (ret)
		log_debug("cannot find PMIC I2C chip\n");

	/* TPS80032: LDO1_REG = 1.2v to DSI */
	ret = dm_i2c_reg_write(dev, TPS80032_LDO1_CFG_VOLTAGE, 0x03);
	if (ret)
		log_debug("avdd_dsi_csi voltage set failed: %d\n", ret);

	/* TPS80032: LDO1_REG enable */
	ret = dm_i2c_reg_write(dev, TPS80032_LDO1_CFG_STATE, 0x01);
	if (ret)
		log_debug("avdd_dsi_csi enable failed: %d\n", ret);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the MMC muxes, power rails, etc.
 */
void pin_mux_mmc(void)
{
	/* Bring up DSI power */
	tps80032_voltage_init();
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
