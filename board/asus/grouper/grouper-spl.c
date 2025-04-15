// SPDX-License-Identifier: GPL-2.0+
/*
 *  T30 Grouper SPL stage configuration
 *
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <spl_gpio.h>
#include <linux/delay.h>

#define MAX77663_I2C_ADDR		(0x3C << 1)

#define MAX77663_REG_SD0		0x16
#define MAX77663_REG_SD0_DATA		(0x2100 | MAX77663_REG_SD0)
#define MAX77663_REG_SD1		0x17
#define MAX77663_REG_SD1_DATA		(0x3000 | MAX77663_REG_SD1)
#define MAX77663_REG_LDO4		0x2B
#define MAX77663_REG_LDO4_DATA		(0xE000 | MAX77663_REG_LDO4)

#define MAX77663_REG_GPIO4		0x3A
#define MAX77663_REG_GPIO4_DATA		(0x0100 | MAX77663_REG_GPIO4)

#define TPS65911_I2C_ADDR		(0x2D << 1)

#define TPS65911_VDDCTRL_OP_REG		0x28
#define TPS65911_VDDCTRL_SR_REG		0x27
#define TPS65911_VDDCTRL_OP_DATA	(0x2400 | TPS65911_VDDCTRL_OP_REG)
#define TPS65911_VDDCTRL_SR_DATA	(0x0100 | TPS65911_VDDCTRL_SR_REG)

#define TPS62361B_I2C_ADDR		(0x60 << 1)

#define TPS62361B_SET3_REG		0x03
#define TPS62361B_SET3_DATA		(0x4600 | TPS62361B_SET3_REG)

/*
 *	PCB_ID[8] is GMI_CS2_N_PK3
 *
 *	    PMIC module detection
 *	==============================
 *	PCB_ID[8]	0	1
 *	PMIC		Maxim	TI
 */
static bool ti_pmic_detected(void)
{
	/* Configure pinmux */
	pinmux_set_func(PMUX_PINGRP_GMI_CS2_N_PK3, PMUX_FUNC_GMI);
	pinmux_set_pullupdown(PMUX_PINGRP_GMI_CS2_N_PK3, PMUX_PULL_DOWN);
	pinmux_tristate_enable(PMUX_PINGRP_GMI_CS2_N_PK3);
	pinmux_set_io(PMUX_PINGRP_GMI_CS2_N_PK3, PMUX_PIN_INPUT);

	spl_gpio_input(NULL, TEGRA_GPIO(K, 3));
	return spl_gpio_get_value(NULL, TEGRA_GPIO(K, 3));
}

static void max_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_SD1_DATA);

	udelay(1000);

	/* Bring up VDD_CPU to 1.0125V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_SD0_DATA);
	udelay(1000);

	/* Bring up VDD_RTC to 1.200V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_LDO4_DATA);
	udelay(10 * 1000);

	/* Set 32k-out gpio state */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_GPIO4_DATA);
}

static void ti_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(TPS62361B_I2C_ADDR, TPS62361B_SET3_DATA);

	udelay(1000);

	/*
	 * Bring up CPU VDD via the TPS65911x PMIC on the DVC I2C bus.
	 * First set VDD to 1.0125V, then enable the VDD regulator.
	 */
	tegra_i2c_ll_write(TPS65911_I2C_ADDR, TPS65911_VDDCTRL_OP_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65911_I2C_ADDR, TPS65911_VDDCTRL_SR_DATA);
	udelay(10 * 1000);
}

void pmic_enable_cpu_vdd(void)
{
	if (ti_pmic_detected())
		ti_enable_cpu_vdd();
	else
		max_enable_cpu_vdd();
}
