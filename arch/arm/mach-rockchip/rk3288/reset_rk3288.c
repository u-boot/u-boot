/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/hardware.h>
#include <linux/err.h>

int rk3288_reset_request(struct udevice *dev, enum reset_t type)
{
	struct rk3288_cru *cru = rockchip_get_cru();

	if (IS_ERR(cru))
		return PTR_ERR(cru);
	switch (type) {
	case RESET_WARM:
		rk_clrreg(&cru->cru_mode_con, 0xffff);
		writel(0xeca8, &cru->cru_glb_srst_snd_value);
		break;
	case RESET_COLD:
		rk_clrreg(&cru->cru_mode_con, 0xffff);
		writel(0xfdb9, &cru->cru_glb_srst_fst_value);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct reset_ops rk3288_reset = {
	.request	= rk3288_reset_request,
};

U_BOOT_DRIVER(reset_rk3288) = {
	.name	= "rk3288_reset",
	.id	= UCLASS_RESET,
	.ops	= &rk3288_reset,
};
