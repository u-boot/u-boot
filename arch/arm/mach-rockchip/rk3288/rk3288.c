// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */
#include <asm/io.h>
#include <asm/arch/hardware.h>

#define GRF_SOC_CON2 0xff77024c

int arch_cpu_init(void)
{
	/* We do some SoC one time setting here. */

	/* Use rkpwm by default */
	rk_setreg(GRF_SOC_CON2, 1 << 0);

	return 0;
}
