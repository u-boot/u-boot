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
#include <parade.h>
#include <spi.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/arch/board.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/system.h>
#include <asm/arch/dp_info.h>
#include <power/tps65090_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

int exynos_init(void)
{
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

	if (has_edp_bridge())
		if (parade_init(gd->fdt_blob))
			printf("%s: ps8625_init() failed\n", __func__);
}

void exynos_backlight_on(unsigned int onoff)
{
#ifdef CONFIG_POWER_TPS65090
	tps65090_fet_enable(1);
#endif
}
#endif

int board_get_revision(void)
{
	return 0;
}
