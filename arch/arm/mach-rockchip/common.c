/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/err.h>

void *rockchip_get_cru(void)
{
	struct udevice *dev;
	fdt_addr_t addr;
	int ret;

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret)
		return ERR_PTR(ret);

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return ERR_PTR(-EINVAL);

	return (void *)addr;
}
