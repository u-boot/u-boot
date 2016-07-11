/*
 * Qualcomm APQ8016 reset controller driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static int msm_reset_request(struct udevice *dev, enum reset_t type)
{
	phys_addr_t addr = dev_get_addr(dev);
	if (!addr)
		return -EINVAL;
	writel(0, addr);
	return -EINPROGRESS;
}

static struct reset_ops msm_reset_ops = {
	.request	= msm_reset_request,
};

static const struct udevice_id msm_reset_ids[] = {
	{ .compatible = "qcom,pshold" },
	{ }
};

U_BOOT_DRIVER(msm_reset) = {
	.name		= "msm_reset",
	.id		= UCLASS_RESET,
	.of_match	= msm_reset_ids,
	.ops		= &msm_reset_ops,
};
