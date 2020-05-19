// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * (C) Copyright 2019 Amarula Solutions(India)
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/err.h>

static char *get_reset_cause(void)
{
	struct rockchip_cru *cru = rockchip_get_cru();
	char *cause = NULL;

	if (IS_ERR(cru))
		return cause;

	switch (cru->glb_rst_st) {
	case GLB_POR_RST:
		cause = "POR";
		break;
	case FST_GLB_RST_ST:
	case SND_GLB_RST_ST:
		cause = "RST";
		break;
	case FST_GLB_TSADC_RST_ST:
	case SND_GLB_TSADC_RST_ST:
		cause = "THERMAL";
		break;
	case FST_GLB_WDT_RST_ST:
	case SND_GLB_WDT_RST_ST:
		cause = "WDOG";
		break;
	default:
		cause = "unknown reset";
	}

	/**
	 * reset_reason env is used by rk3288, due to special use case
	 * to figure it the boot behavior. so keep this as it is.
	 */
	env_set("reset_reason", cause);

	/*
	 * Clear glb_rst_st, so we can determine the last reset cause
	 * for following resets.
	 */
	rk_clrreg(&cru->glb_rst_st, GLB_RST_ST_MASK);

	return cause;
}

int print_cpuinfo(void)
{
	printf("SoC: Rockchip %s\n", CONFIG_SYS_SOC);
	printf("Reset cause: %s\n", get_reset_cause());

	/* TODO print operating temparature and clock */

	return 0;
}
