// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2021 Intel Corporation <www.intel.com>
 *
 */

#include <asm/arch/clock_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dt-bindings/clock/n5x-clock.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong cm_get_rate_dm(u32 id)
{
	struct udevice *dev;
	struct clk clk;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(socfpga_n5x_clk),
					  &dev);
	if (ret)
		return 0;

	clk.id = id;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return 0;

	return clk_get_rate(&clk);
}

static u32 cm_get_rate_dm_khz(u32 id)
{
	return cm_get_rate_dm(id) / 1000;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	return cm_get_rate_dm(N5X_MPU_CLK);
}

unsigned int cm_get_l4_sys_free_clk_hz(void)
{
	return cm_get_rate_dm(N5X_L4_SYS_FREE_CLK);
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU       %10d kHz\n",
	       cm_get_rate_dm_khz(N5X_MPU_CLK));
	printf("L4 Main	    %8d kHz\n",
	       cm_get_rate_dm_khz(N5X_L4_MAIN_CLK));
	printf("L4 sys free %8d kHz\n",
	       cm_get_rate_dm_khz(N5X_L4_SYS_FREE_CLK));
	printf("L4 MP       %8d kHz\n",
	       cm_get_rate_dm_khz(N5X_L4_MP_CLK));
	printf("L4 SP       %8d kHz\n",
	       cm_get_rate_dm_khz(N5X_L4_SP_CLK));
	printf("SDMMC       %8d kHz\n",
	       cm_get_rate_dm_khz(N5X_SDMMC_CLK));
}
