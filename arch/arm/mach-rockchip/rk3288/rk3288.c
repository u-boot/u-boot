// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */
#include <asm/io.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3288.h>

#define GRF_BASE	0xff770000

int arch_cpu_init(void)
{
	/* We do some SoC one time setting here. */
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	/* Use rkpwm by default */
	rk_setreg(&grf->soc_con2, 1 << 0);

	return 0;
}
