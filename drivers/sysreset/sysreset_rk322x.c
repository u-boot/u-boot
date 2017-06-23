/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk322x.h>
#include <asm/arch/hardware.h>
#include <linux/err.h>

int rk322x_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct rk322x_cru *cru = rockchip_get_cru();

	if (IS_ERR(cru))
		return PTR_ERR(cru);
	switch (type) {
	case SYSRESET_WARM:
		writel(0xeca8, &cru->cru_glb_srst_snd_value);
		break;
	case SYSRESET_COLD:
		writel(0xfdb9, &cru->cru_glb_srst_fst_value);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops rk322x_sysreset = {
	.request	= rk322x_sysreset_request,
};

U_BOOT_DRIVER(sysreset_rk322x) = {
	.name	= "rk322x_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &rk322x_sysreset,
};
