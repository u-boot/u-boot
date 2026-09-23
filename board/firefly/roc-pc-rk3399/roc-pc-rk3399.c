// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <i2c.h>
#include <image.h>
#include <log.h>
#include <spl_gpio.h>
#include <asm/global_data.h>
#include <asm/io.h>

#include <asm/arch-rockchip/cru.h>
#include <asm/arch-rockchip/gpio.h>
#include <asm/arch-rockchip/grf_rk3399.h>

#define ROC_PC_MP8859_BUS	"i2c@ff160000"
#define ROC_PC_MP8859_ADDR	0x66
#define ROC_PC_PLUS_FDTFILE	"rockchip/rk3399-roc-pc-plus.dtb"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_XPL_BUILD

#define PMUGRF_BASE	0xff320000
#define GPIO0_BASE	0xff720000

/**
 * LED setup for roc-rk3399-pc
 *
 * 1. Set the low power leds (only during POR, pwr_key env is 'y')
 *    glow yellow LED, termed as low power
 *    poll for on board power key press
 *    once powe key pressed, turn off yellow
 * 2. Turn on red LED, indicating full power mode
 */
void led_setup(void)
{
	struct rockchip_gpio_regs * const gpio0 = (void *)GPIO0_BASE;
	struct rk3399_pmugrf_regs * const pmugrf = (void *)PMUGRF_BASE;
	bool press_pwr_key = false;

	if (IS_ENABLED(CONFIG_SPL_ENV_SUPPORT)) {
		env_init();
		env_load();
		if (env_get_yesno("pwr_key") == 1)
			press_pwr_key = true;
	}

	if (press_pwr_key && !strcmp(get_reset_cause(), "POR")) {
		spl_gpio_output(gpio0, GPIO(BANK_A, 2), 1);

		spl_gpio_set_pull(&pmugrf->gpio0_p, GPIO(BANK_A, 5),
				  GPIO_PULL_NORMAL);
		while (readl(&gpio0->ext_port) & 0x20)
			;

		spl_gpio_output(gpio0, GPIO(BANK_A, 2), 0);
	}

	spl_gpio_output(gpio0, GPIO(BANK_B, 5), 1);
}

static bool is_roc_pc_plus(void)
{
	struct udevice *bus, *dev;

	if (!(CONFIG_IS_ENABLED(I2C) && CONFIG_IS_ENABLED(DM_I2C)))
		return false;

	if (uclass_get_device_by_name(UCLASS_I2C, ROC_PC_MP8859_BUS, &bus))
		return false;

	return dm_i2c_probe(bus, ROC_PC_MP8859_ADDR, 0, &dev);
}

int board_fit_config_name_match(const char *name)
{
	if (is_roc_pc_plus())
		return strcmp(name, ROC_PC_PLUS_FDTFILE);

	return strcmp(name, CONFIG_DEFAULT_FDT_FILE);
}

#endif

int rk_board_late_init(void)
{
	if (!fdt_node_check_compatible(gd->fdt_blob, 0,
				       "firefly,roc-rk3399-pc-plus"))
		env_set("fdtfile", ROC_PC_PLUS_FDTFILE);
	else
		env_set("fdtfile", CONFIG_DEFAULT_FDT_FILE);

	return 0;
}
