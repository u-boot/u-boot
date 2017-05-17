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
#include <sysreset.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static int msm_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	phys_addr_t addr = devfdt_get_addr(dev);
	if (!addr)
		return -EINVAL;
	writel(0, addr);
	return -EINPROGRESS;
}

static struct sysreset_ops msm_sysreset_ops = {
	.request	= msm_sysreset_request,
};

static const struct udevice_id msm_sysreset_ids[] = {
	{ .compatible = "qcom,pshold" },
	{ }
};

U_BOOT_DRIVER(msm_reset) = {
	.name		= "msm_sysreset",
	.id		= UCLASS_SYSRESET,
	.of_match	= msm_sysreset_ids,
	.ops		= &msm_sysreset_ops,
};
