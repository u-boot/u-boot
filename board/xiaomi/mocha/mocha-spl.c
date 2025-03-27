// SPDX-License-Identifier: GPL-2.0+
/*
 *  Mocha SPL stage configuration
 *
 *  (C) Copyright 2024
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <asm/arch/tegra.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <linux/delay.h>

#define TPS65913_I2C_ADDR		(0x58 << 1)

#define TPS65913_SMPS12_CTRL		0x20
#define TPS65913_SMPS12_VOLTAGE		0x23
#define TPS65913_SMPS45_CTRL		0x28
#define TPS65913_SMPS45_VOLTAGE		0x2B
#define TPS65913_SMPS7_CTRL		0x30
#define TPS65913_SMPS7_VOLTAGE		0x33

#define TPS65913_SMPS12_CTRL_DATA	(0x5100 | TPS65913_SMPS12_CTRL)
#define TPS65913_SMPS12_VOLTAGE_DATA	(0x3800 | TPS65913_SMPS12_VOLTAGE)
#define TPS65913_SMPS45_CTRL_DATA	(0x5100 | TPS65913_SMPS45_CTRL)
#define TPS65913_SMPS45_VOLTAGE_DATA	(0x3800 | TPS65913_SMPS45_VOLTAGE)
#define TPS65913_SMPS7_CTRL_DATA	(0x5100 | TPS65913_SMPS7_CTRL)
#define TPS65913_SMPS7_VOLTAGE_DATA	(0x4700 | TPS65913_SMPS7_VOLTAGE)

void pmic_enable_cpu_vdd(void)
{
	/* Set CORE VDD to 1.150V. */
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS7_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS7_CTRL_DATA);

	udelay(1000);

	/* Set CPU VDD to 1.0V. */
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS12_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS12_CTRL_DATA);
	udelay(10 * 1000);

	/* Set GPU VDD to 1.0V. */
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS45_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65913_I2C_ADDR, TPS65913_SMPS45_CTRL_DATA);
	udelay(10 * 1000);
}
