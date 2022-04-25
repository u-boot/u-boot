// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdevice for system, used for bootmeths not tied to any partition device
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <command.h>
#include <distro.h>
#include <dm.h>
#include <log.h>
#include <net.h>

static int system_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			       struct bootflow *bflow)
{
	int ret;

	/* Must be an bootstd device */
	ret = bootflow_iter_uses_system(iter);
	if (ret)
		return log_msg_ret("net", ret);

	ret = bootmeth_check(bflow->method, iter);
	if (ret)
		return log_msg_ret("check", ret);

	ret = bootmeth_read_bootflow(bflow->method, bflow);
	if (ret)
		return log_msg_ret("method", ret);

	return 0;
}

static int system_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_6_SYSTEM;

	return 0;
}

struct bootdev_ops system_bootdev_ops = {
	.get_bootflow	= system_get_bootflow,
};

static const struct udevice_id system_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-system" },
	{ }
};

U_BOOT_DRIVER(system_bootdev) = {
	.name		= "system_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &system_bootdev_ops,
	.bind		= system_bootdev_bind,
	.of_match	= system_bootdev_ids,
};
