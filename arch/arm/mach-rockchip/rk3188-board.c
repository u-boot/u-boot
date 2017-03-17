/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/periph.h>
#include <asm/arch/pmu_rk3288.h>
#include <asm/arch/boot_mode.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
#if defined(CONFIG_ROCKCHIP_SPL_BACK_TO_BROM)
	struct udevice *pinctrl;
	int ret;

	/*
	 * We need to implement sdcard iomux here for the further
	 * initialization, otherwise, it'll hit sdcard command sending
	 * timeout exception.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_SDCARD);
	if (ret) {
		debug("%s: Failed to set up SD card\n", __func__);
		goto err;
	}

	return 0;
err:
	printf("board_init: Error %d\n", ret);

	/* No way to report error here */
	hang();

	return -1;
#else
	return 0;
#endif
}

int dram_init(void)
{
	/* FIXME: read back ram size from sys_reg2 */
	gd->ram_size = 0x40000000;

	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
