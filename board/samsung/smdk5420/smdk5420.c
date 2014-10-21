/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <i2c.h>
#include <lcd.h>
#include <spi.h>
#include <errno.h>
#include <asm/arch/board.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/system.h>
#include <asm/arch/dp_info.h>
#include <power/tps65090_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USB_EHCI_EXYNOS
static int board_usb_vbus_init(void)
{
	/* Enable VBUS power switch */
	gpio_direction_output(EXYNOS5420_GPIO_X26, 1);

	/* VBUS turn ON time */
	mdelay(3);

	return 0;
}
#endif

int exynos_init(void)
{
#ifdef CONFIG_USB_EHCI_EXYNOS
	board_usb_vbus_init();
#endif
	return 0;
}

#ifdef CONFIG_LCD
static int has_edp_bridge(void)
{
	int node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_PARADE_PS8625);

	/* No node for bridge in device tree. */
	if (node <= 0)
		return 0;

	/* Default is with bridge ic */
	return 1;
}

void exynos_lcd_power_on(void)
{
	int ret;

#ifdef CONFIG_POWER_TPS65090
	ret = tps65090_init();
	if (ret < 0) {
		printf("%s: tps65090_init() failed\n", __func__);
		return;
	}

	tps65090_fet_enable(6);
#endif

	mdelay(5);

	/* TODO(ajaykumar.rs@samsung.com): Use device tree */
	gpio_direction_output(EXYNOS5420_GPIO_X35, 1);	/* EDP_SLP# */
	mdelay(10);
	gpio_direction_output(EXYNOS5420_GPIO_Y77, 1);	/* EDP_RST# */
	gpio_direction_input(EXYNOS5420_GPIO_X26);	/* EDP_HPD */
	gpio_set_pull(EXYNOS5420_GPIO_X26, S5P_GPIO_PULL_NONE);

	if (has_edp_bridge())
		if (parade_init(gd->fdt_blob))
			printf("%s: ps8625_init() failed\n", __func__);
}

void exynos_backlight_on(unsigned int onoff)
{
	/* For PWM */
	gpio_cfg_pin(EXYNOS5420_GPIO_B20, S5P_GPIO_FUNC(0x1));
	gpio_set_value(EXYNOS5420_GPIO_B20, 1);

#ifdef CONFIG_POWER_TPS65090
	tps65090_fet_enable(1);
#endif
}
#endif

int board_get_revision(void)
{
	return 0;
}
