/*
 *  (C) Copyright 2014
 *  Marcel Ziswiler <marcel@ziswiler.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/tegra.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <i2c.h>
#include <netdev.h>

#include "pinmux-config-apalis_t30.h"

#define PMU_I2C_ADDRESS		0x2D
#define MAX_I2C_RETRY		3

int arch_misc_init(void)
{
	if (readl(NV_PA_BASE_SRAM + NVBOOTINFOTABLE_BOOTTYPE) ==
	    NVBOOTTYPE_RECOVERY)
		printf("USB recovery mode\n");

	return 0;
}

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(tegra3_pinmux_common,
				   ARRAY_SIZE(tegra3_pinmux_common));

	pinmux_config_pingrp_table(unused_pins_lowpower,
				   ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(apalis_t30_padctrl,
				   ARRAY_SIZE(apalis_t30_padctrl));
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	u8 addr, data[1];
	int err;

	err = i2c_get_chip_for_busnum(0, PMU_I2C_ADDRESS, 1, &dev);
	if (err) {
		debug("%s: Cannot find PMIC I2C chip\n", __func__);
		return err;
	}

	/* TPS659110: VDD2_OP_REG = 1.05V */
	data[0] = 0x27;
	addr = 0x25;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to set VDD supply\n");
		return err;
	}

	/* TPS659110: VDD2_REG 7.5 mV/us, ACTIVE */
	data[0] = 0x0D;
	addr = 0x24;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to enable VDD supply\n");
		return err;
	}

	/* TPS659110: LDO6_REG = 1.1V, ACTIVE */
	data[0] = 0x0D;
	addr = 0x35;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to set AVDD supply\n");
		return err;
	}

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
#endif /* CONFIG_PCI_TEGRA */
