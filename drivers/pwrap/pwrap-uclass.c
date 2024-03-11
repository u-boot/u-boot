// SPDX-License-Identifier: GPL-2.0-only

#define LOG_CATEGORY UCLASS_PWRAP

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pwrap/pwrap.h>
#include <linux/ctype.h>

int pwrap_reg_read(struct udevice *dev, u32 adr, u32 *rdata)
{
	const struct dm_pwrap_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->read)
		return -ENOSYS;

	return ops->read(dev, adr, rdata);
}

int pwrap_reg_write(struct udevice *dev, u32 adr, u32 wdata)
{
	const struct dm_pwrap_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->write)
		return -ENOSYS;

	return ops->write(dev, adr, wdata);
}

UCLASS_DRIVER(pwrap) = {
	.id		= UCLASS_PWRAP,
	.name		= "pwrap",
	.post_bind	= dm_scan_fdt_dev,
};
