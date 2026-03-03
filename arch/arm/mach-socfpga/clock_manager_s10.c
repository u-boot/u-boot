// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 */

#include <clk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <dt-bindings/clock/stratix10-clock.h>

static ulong cm_get_rate_dm(u32 id)
{
	struct udevice *dev;
	struct clk clk;
	ulong rate;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(socfpga_s10_clk),
					  &dev);
	if (ret)
		return 0;

	clk.id = id;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return 0;

	rate = clk_get_rate(&clk);

	if ((rate == (unsigned long)-ENOSYS) ||
	    (rate == (unsigned long)-ENXIO) ||
	    (rate == (unsigned long)-EIO)) {
		debug("%s id %u: clk_get_rate err: %ld\n",
		      __func__, id, rate);
		return 0;
	}

	return rate;
}

static u32 cm_get_rate_dm_khz(u32 id)
{
	return cm_get_rate_dm(id) / 1000;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	return cm_get_rate_dm(STRATIX10_MPU_CLK);
}

unsigned int cm_get_l4_sys_free_clk_hz(void)
{
	return cm_get_rate_dm(STRATIX10_L4_SYS_FREE_CLK);
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU         %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_MPU_CLK));
	printf("L3 main     %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_NOC_CLK));
	printf("Main VCO    %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_MAIN_PLL_CLK));
	printf("Per VCO     %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_PERIPH_PLL_CLK));
	printf("EOSC1       %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_OSC1));
	printf("HPS MMC     %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_SDMMC_CLK));
	printf("UART        %d kHz\n",
	       cm_get_rate_dm_khz(STRATIX10_L4_SP_CLK));
}
