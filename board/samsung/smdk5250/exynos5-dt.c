/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>
#include <netdev.h>
#include <spi.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/tps65090_pmic.h>
#include <tmu.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SOUND_MAX98095
static void board_enable_audio_codec(void)
{
	/* Enable MAX98095 Codec */
	gpio_direction_output(EXYNOS5_GPIO_X17, 1);
	gpio_set_pull(EXYNOS5_GPIO_X17, S5P_GPIO_PULL_NONE);
}
#endif

int exynos_init(void)
{
#ifdef CONFIG_SOUND_MAX98095
	board_enable_audio_codec();
#endif
	return 0;
}

#if defined(CONFIG_POWER)
#ifdef CONFIG_POWER_MAX77686
static int pmic_reg_update(struct pmic *p, int reg, uint regval)
{
	u32 val;
	int ret = 0;

	ret = pmic_reg_read(p, reg, &val);
	if (ret) {
		debug("%s: PMIC %d register read failed\n", __func__, reg);
		return -1;
	}
	val |= regval;
	ret = pmic_reg_write(p, reg, val);
	if (ret) {
		debug("%s: PMIC %d register write failed\n", __func__, reg);
		return -1;
	}
	return 0;
}

static int max77686_init(void)
{
	struct pmic *p;

	if (pmic_init(I2C_PMIC))
		return -1;

	p = pmic_get("MAX77686_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (pmic_reg_update(p, MAX77686_REG_PMIC_32KHZ, MAX77686_32KHCP_EN))
		return -1;

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BBAT,
			    MAX77686_BBCHOSTEN | MAX77686_BBCVS_3_5V))
		return -1;

	/* VDD_MIF */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK1OUT,
			   MAX77686_BUCK1OUT_1V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK1OUT);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK1CRTL,
			    MAX77686_BUCK1CTRL_EN))
		return -1;

	/* VDD_ARM */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK2DVS1,
			   MAX77686_BUCK2DVS1_1_3V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK2DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK2CTRL1,
			    MAX77686_BUCK2CTRL_ON))
		return -1;

	/* VDD_INT */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK3DVS1,
			   MAX77686_BUCK3DVS1_1_0125V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK3DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK3CTRL,
			    MAX77686_BUCK3CTRL_ON))
		return -1;

	/* VDD_G3D */
	if (pmic_reg_write(p, MAX77686_REG_PMIC_BUCK4DVS1,
			   MAX77686_BUCK4DVS1_1_2V)) {
		debug("%s: PMIC %d register write failed\n", __func__,
		      MAX77686_REG_PMIC_BUCK4DVS1);
		return -1;
	}

	if (pmic_reg_update(p, MAX77686_REG_PMIC_BUCK4CTRL1,
			    MAX77686_BUCK3CTRL_ON))
		return -1;

	/* VDD_LDO2 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO2CTRL1,
			    MAX77686_LD02CTRL1_1_5V | EN_LDO))
		return -1;

	/* VDD_LDO3 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO3CTRL1,
			    MAX77686_LD03CTRL1_1_8V | EN_LDO))
		return -1;

	/* VDD_LDO5 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO5CTRL1,
			    MAX77686_LD05CTRL1_1_8V | EN_LDO))
		return -1;

	/* VDD_LDO10 */
	if (pmic_reg_update(p, MAX77686_REG_PMIC_LDO10CTRL1,
			    MAX77686_LD10CTRL1_1_8V | EN_LDO))
		return -1;

	return 0;
}
#endif	/* CONFIG_POWER_MAX77686 */

int exynos_power_init(void)
{
	int ret = 0;

#ifdef CONFIG_POWER_MAX77686
	ret = max77686_init();
	if (ret)
		return ret;
#endif
#ifdef CONFIG_POWER_TPS65090
	/*
	 * The TPS65090 may not be in the device tree. If so, it is not
	 * an error.
	 */
	ret = tps65090_init();
	if (ret == 0 || ret == -ENODEV)
		return 0;
#endif

	return ret;
}
#endif	/* CONFIG_POWER */

#ifdef CONFIG_LCD
void exynos_cfg_lcd_gpio(void)
{
	/* For Backlight */
	gpio_cfg_pin(EXYNOS5_GPIO_B20, S5P_GPIO_OUTPUT);
	gpio_set_value(EXYNOS5_GPIO_B20, 1);

	/* LCD power on */
	gpio_cfg_pin(EXYNOS5_GPIO_X15, S5P_GPIO_OUTPUT);
	gpio_set_value(EXYNOS5_GPIO_X15, 1);

	/* Set Hotplug detect for DP */
	gpio_cfg_pin(EXYNOS5_GPIO_X07, S5P_GPIO_FUNC(0x3));
}

void exynos_set_dp_phy(unsigned int onoff)
{
	set_dp_phy_ctrl(onoff);
}
#endif
