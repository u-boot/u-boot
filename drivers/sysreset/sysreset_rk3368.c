/*
 * (C) Copyright Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3368.h>
#include <asm/arch/hardware.h>
#include <linux/err.h>

static void rk3368_pll_enter_slow_mode(struct rk3368_cru *cru)
{
	struct rk3368_pll *pll;
	int i;

	for (i = 0; i < 6; i++) {
		pll = &cru->pll[i];
		rk_clrreg(&pll->con3, PLL_MODE_MASK);
	}
}

static int rk3368_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct rk3368_cru *cru = rockchip_get_cru();

	if (IS_ERR(cru))
		return PTR_ERR(cru);
	switch (type) {
	case SYSRESET_WARM:
		rk3368_pll_enter_slow_mode(cru);
		rk_clrsetreg(&cru->glb_rst_con, PMU_GLB_SRST_CTRL_MASK,
			     PMU_RST_BY_SND_GLB_SRST << PMU_GLB_SRST_CTRL_SHIFT);
		writel(0xeca8, &cru->glb_srst_snd_val);
		break;
	case SYSRESET_COLD:
		rk3368_pll_enter_slow_mode(cru);
		rk_clrsetreg(&cru->glb_rst_con, PMU_GLB_SRST_CTRL_MASK,
			     PMU_RST_BY_FST_GLB_SRST << PMU_GLB_SRST_CTRL_SHIFT);
		writel(0xfdb9, &cru->glb_srst_fst_val);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops rk3368_sysreset = {
	.request	= rk3368_sysreset_request,
};

U_BOOT_DRIVER(sysreset_rk3368) = {
	.name	= "rk3368_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &rk3368_sysreset,
};
