/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/s2mps11.h>

DECLARE_GLOBAL_DATA_PTR;

static int s2mps11_reg_count(struct udevice *dev)
{
	return S2MPS11_REG_COUNT;
}

static int s2mps11_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		error("write error to device: %p register: %#x!", dev, reg);

	return ret;
}

static int s2mps11_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		error("read error from device: %p register: %#x!", dev, reg);

	return ret;
}

static struct dm_pmic_ops s2mps11_ops = {
	.reg_count = s2mps11_reg_count,
	.read = s2mps11_read,
	.write = s2mps11_write,
};

static const struct udevice_id s2mps11_ids[] = {
	{ .compatible = "samsung,s2mps11-pmic" },
	{ }
};

U_BOOT_DRIVER(pmic_s2mps11) = {
	.name = "s2mps11_pmic",
	.id = UCLASS_PMIC,
	.of_match = s2mps11_ids,
	.ops = &s2mps11_ops,
};
