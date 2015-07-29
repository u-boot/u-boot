/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include "max77620_init.h"

/* MAX77620-PMIC-specific early init code - get CPU rails up, etc */

void tegra_i2c_ll_write_addr(uint addr, uint config)
{
	struct i2c_ctlr *reg = (struct i2c_ctlr *)TEGRA_DVC_BASE;

	writel(addr, &reg->cmd_addr0);
	writel(config, &reg->cnfg);
}

void tegra_i2c_ll_write_data(uint data, uint config)
{
	struct i2c_ctlr *reg = (struct i2c_ctlr *)TEGRA_DVC_BASE;

	writel(data, &reg->cmd_data1);
	writel(config, &reg->cnfg);
}

void pmic_enable_cpu_vdd(void)
{
	uint reg;
	debug("%s entry\n", __func__);

	/* Setup/Enable GPIO5 - VDD_CPU_REG_EN */
	debug("%s: Setting GPIO5 to enable CPU regulator\n", __func__);
	/* B3=1=logic high,B2=dontcare,B1=0=output,B0=1=push-pull */
	reg = 0x0900 | MAX77620_GPIO5_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	/* Setup/Enable GPIO1 - VDD_HDMI_5V0_BST_EN */
	debug("%s: Setting GPIO1 to enable HDMI\n", __func__);
	reg = 0x0900 | MAX77620_GPIO1_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	/* GPIO 0,1,5,6,7 = GPIO, 2,3,4 = alt mode */
	reg = 0x1C00 | MAX77620_AME_GPIO;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	/* Disable SD1 Remote Sense, Set SD1 for LPDDR4 to 1.125v */
	debug("%s: Set SD1 for LPDDR4, disable SD1RS, voltage to 1.125v\n",
	      __func__);
	/* bit1=0, SD1 remote sense disabled */
	reg = 0x0400 | MAX77620_CNFG2SD_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	/* SD1 output = 1.125V */
	reg = 0x2A00 | MAX77620_SD1_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	debug("%s: Set LDO2 for VDDIO_SDMMC_AP power to 3.3V\n", __func__);
	/* 0xF2 for 3.3v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	reg = 0xF200 | MAX77620_CNFG1_L2_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);

	debug("%s: Set LDO1 for USB3 phy power to 1.05V??\n", __func__);
	/* 0xCA for 105v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	reg = 0xCA00 | MAX77620_CNFG1_L1_REG;
	tegra_i2c_ll_write_addr(MAX77620_I2C_ADDR, 2);
	tegra_i2c_ll_write_data(reg, I2C_SEND_2_BYTES);
	udelay(10 * 1000);
}
