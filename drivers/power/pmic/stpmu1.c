// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/stpmu1.h>

#define STMPU1_NUM_OF_REGS 0x100

static int stpmu1_reg_count(struct udevice *dev)
{
	return STMPU1_NUM_OF_REGS;
}

static int stpmu1_write(struct udevice *dev, uint reg, const uint8_t *buff,
			int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to write register %#x :%d",
			__func__, reg, ret);

	return ret;
}

static int stpmu1_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to read register %#x : %d",
			__func__, reg, ret);

	return ret;
}

static struct dm_pmic_ops stpmu1_ops = {
	.reg_count = stpmu1_reg_count,
	.read = stpmu1_read,
	.write = stpmu1_write,
};

static const struct udevice_id stpmu1_ids[] = {
	{ .compatible = "st,stpmu1" },
	{ }
};

U_BOOT_DRIVER(pmic_stpmu1) = {
	.name = "stpmu1_pmic",
	.id = UCLASS_PMIC,
	.of_match = stpmu1_ids,
	.ops = &stpmu1_ops,
};
