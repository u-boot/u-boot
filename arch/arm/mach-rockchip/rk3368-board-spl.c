// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch-rockchip/cru_rk3368.h>
#include <asm/arch-rockchip/grf_rk3368.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/timer.h>
#include <dm/pinctrl.h>

void board_debug_uart_init(void)
{
}

void board_init_f(ulong dummy)
{
	struct udevice *pinctrl;
	struct udevice *dev;
	int ret;

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	/* Set up our preloader console */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		pr_err("%s: pinctrl init failed: %d\n", __func__, ret);
		hang();
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_UART0);
	if (ret) {
		pr_err("%s: failed to set up console UART\n", __func__);
		hang();
	}

	preloader_console_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
