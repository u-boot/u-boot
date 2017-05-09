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
#include <asm/arch/grf_rk3188.h>
#include <asm/arch/periph.h>
#include <asm/arch/pmu_rk3288.h>
#include <asm/arch/boot_mode.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

int board_late_init(void)
{
	struct rk3188_grf *grf;

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(grf)) {
		error("grf syscon returned %ld\n", PTR_ERR(grf));
	} else {
		/* enable noc remap to mimic legacy loaders */
		rk_clrsetreg(&grf->soc_con0,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT);
	}

	return 0;
}

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
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("Cannot get DRAM size: %d\n", ret);
		return ret;
	}
	debug("SDRAM base=%lx, size=%x\n", ram.base, ram.size);
	gd->ram_size = ram.size;

	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
