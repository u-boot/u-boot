/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rv1108.h>
#include <asm/arch/hardware.h>
#include <linux/err.h>

int rv1108_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct rv1108_cru *cru = rockchip_get_cru();

	if (IS_ERR(cru))
		return PTR_ERR(cru);

	switch (type) {
	case SYSRESET_WARM:
		writel(0xeca8, &cru->glb_srst_snd_val);
		break;
	case SYSRESET_COLD:
		writel(0xfdb9, &cru->glb_srst_fst_val);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops rv1108_sysreset = {
	.request	= rv1108_sysreset_request,
};

U_BOOT_DRIVER(sysreset_rv1108) = {
	.name	= "rv1108_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &rv1108_sysreset,
};
