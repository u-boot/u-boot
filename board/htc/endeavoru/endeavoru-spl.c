// SPDX-License-Identifier: GPL-2.0+
/*
 *  T30 HTC Endeavoru SPL stage configuration
 *
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <linux/delay.h>

/*
 * Endeavoru uses TPS80032 PMIC with SMPS1 and SMPS2 in strandard
 * mode with zero offset.
 */

#define TPS80032_DVS_I2C_ADDR			(0x12 << 1)
#define TPS80032_SMPS1_CFG_VOLTAGE_REG		0x56
#define TPS80032_SMPS2_CFG_VOLTAGE_REG		0x5C
#define TPS80032_SMPS1_CFG_VOLTAGE_DATA		(0x2100 | TPS80032_SMPS1_CFG_VOLTAGE_REG)
#define TPS80032_SMPS2_CFG_VOLTAGE_DATA		(0x3000 | TPS80032_SMPS2_CFG_VOLTAGE_REG)

#define TPS80032_CTL1_I2C_ADDR			(0x48 << 1)
#define TPS80032_SMPS1_CFG_STATE_REG		0x54
#define TPS80032_SMPS2_CFG_STATE_REG		0x5A
#define TPS80032_SMPS1_CFG_STATE_DATA		(0x0100 | TPS80032_SMPS1_CFG_STATE_REG)
#define TPS80032_SMPS2_CFG_STATE_DATA		(0x0100 | TPS80032_SMPS2_CFG_STATE_REG)

void pmic_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(TPS80032_DVS_I2C_ADDR, TPS80032_SMPS2_CFG_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS80032_CTL1_I2C_ADDR, TPS80032_SMPS2_CFG_STATE_DATA);

	udelay(1000);

	/* Bring up VDD_CPU to 1.0125V. */
	tegra_i2c_ll_write(TPS80032_DVS_I2C_ADDR, TPS80032_SMPS1_CFG_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS80032_CTL1_I2C_ADDR, TPS80032_SMPS1_CFG_STATE_DATA);
	udelay(10 * 1000);
}
