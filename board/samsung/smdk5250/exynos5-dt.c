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
