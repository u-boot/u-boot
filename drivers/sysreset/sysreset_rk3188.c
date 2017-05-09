/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3188.h>
#include <asm/arch/grf_rk3188.h>
#include <asm/arch/hardware.h>
#include <linux/err.h>

int rk3188_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct rk3188_cru *cru = rockchip_get_cru();
	struct rk3188_grf *grf;

	if (IS_ERR(cru))
		return PTR_ERR(cru);
	switch (type) {
	case SYSRESET_WARM:
		grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
		if (IS_ERR(grf))
			return -EPROTONOSUPPORT;

		/*
		 * warm-reset keeps the remap value,
		 * so make sure it's disabled.
		 */
		rk_clrsetreg(&grf->soc_con0,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT,
			0 << NOC_REMAP_SHIFT);

		rk_clrreg(&cru->cru_mode_con, 0xffff);
		writel(0xeca8, &cru->cru_glb_srst_snd_value);
		break;
	case SYSRESET_COLD:
		rk_clrreg(&cru->cru_mode_con, 0xffff);
		writel(0xfdb9, &cru->cru_glb_srst_fst_value);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops rk3188_sysreset = {
	.request	= rk3188_sysreset_request,
};

U_BOOT_DRIVER(sysreset_rk3188) = {
	.name	= "rk3188_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &rk3188_sysreset,
};
