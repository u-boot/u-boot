// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
 * Keerthy <j-keerthy@ti.com>
 */

#include <dm.h>
#include <i2c.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

#define TPS6287X_REG_VSET			0x0
#define TPS6287X_REG_CONTROL1			0x1
#define TPS6287X_REG_CONTROL2			0x2
#define TPS6287X_REG_CONTROL3			0x3
#define TPS6287X_REG_STATUS			0x4
#define TPS6287X_REG_VSET_VSET_MASK		0xff
#define TPS6287X_REG_CONTROL2_VRANGE_MASK	0xc

struct tps6287x_regulator_config {
	u32 vmin;
	u32 vmax;
};

struct tps6287x_regulator_pdata {
	u8 vsel_offset;
	struct udevice *i2c;
	struct tps6287x_regulator_config *config;
};

static struct tps6287x_regulator_config tps6287x_data = {
	.vmin = 400000,
	.vmax = 3350000,
};

static int tps6287x_regulator_set_value(struct udevice *dev, int uV)
{
	struct tps6287x_regulator_pdata *pdata = dev_get_plat(dev);
	u8 regval, vset;
	int ret;

	if (uV < pdata->config->vmin || uV > pdata->config->vmax)
		return -EINVAL;
	/*
	 * Based on the value of VRANGE bit field of CONTROL2 reg the range
	 * varies.
	 */
	ret = dm_i2c_read(pdata->i2c, TPS6287X_REG_CONTROL2, &regval, 1);
	if (ret) {
		dev_err(dev, "CTRL2 reg read failed: %d\n", ret);
		return ret;
	}

	regval &= TPS6287X_REG_CONTROL2_VRANGE_MASK;
	regval >>= ffs(TPS6287X_REG_CONTROL2_VRANGE_MASK) - 1;

	/*
	 * VRANGE = 0. Increment step 1250 uV starting with 0 --> 400000 uV
	 * VRANGE = 1. Increment step 2500 uV starting with 0 --> 400000 uV
	 * VRANGE = 2. Increment step 5000 uV starting with 0 --> 400000 uV
	 * VRANGE = 3. Increment step 10000 uV starting with 0 --> 800000 uV
	 */
	switch (regval) {
	case 0:
		vset = (uV - 400000) / 1250;
		break;
	case 1:
		vset = (uV - 400000) / 2500;
		break;
	case 2:
		vset = (uV - 400000) / 5000;
		break;
	case 3:
		vset = (uV - 800000) / 10000;
		break;
	default:
		pr_err("%s: invalid regval %d\n", dev->name, regval);
		return -EINVAL;
	}

	return dm_i2c_write(pdata->i2c, TPS6287X_REG_VSET, &vset, 1);
}

static int tps6287x_regulator_get_value(struct udevice *dev)
{
	u8 regval, vset;
	int uV;
	int ret;
	struct tps6287x_regulator_pdata *pdata = dev_get_plat(dev);

	/*
	 * Based on the value of VRANGE bit field of CONTROL2 reg the range
	 * varies.
	 */
	ret = dm_i2c_read(pdata->i2c, TPS6287X_REG_CONTROL2, &regval, 1);
	if (ret) {
		dev_err(dev, "i2c read failed: %d\n", ret);
		return ret;
	}

	regval &= TPS6287X_REG_CONTROL2_VRANGE_MASK;
	regval >>= ffs(TPS6287X_REG_CONTROL2_VRANGE_MASK) - 1;

	ret = dm_i2c_read(pdata->i2c, TPS6287X_REG_VSET, &vset, 1);
	if (ret) {
		dev_err(dev, "i2c VSET read failed: %d\n", ret);
		return ret;
	}

	/*
	 * VRANGE = 0. Increment step 1250 uV starting with 0 --> 400000 uV
	 * VRANGE = 1. Increment step 2500 uV starting with 0 --> 400000 uV
	 * VRANGE = 2. Increment step 5000 uV starting with 0 --> 400000 uV
	 * VRANGE = 3. Increment step 10000 uV starting with 0 --> 800000 uV
	 */
	switch (regval) {
	case 0:
		uV = 400000 + vset * 1250;
		break;
	case 1:
		uV = 400000 + vset * 2500;
		break;
	case 2:
		uV = 400000 + vset * 5000;
		break;
	case 3:
		uV = 800000 + vset * 10000;
		break;
	default:
		pr_err("%s: invalid regval %d\n", dev->name, regval);
		return -EINVAL;
	}

	return uV;
}

static int tps6287x_regulator_probe(struct udevice *dev)
{
	struct tps6287x_regulator_pdata *pdata = dev_get_plat(dev);
	int ret, slave_id;

	pdata->config = (void *)dev_get_driver_data(dev);

	slave_id = devfdt_get_addr_index(dev, 0);

	ret = i2c_get_chip(dev->parent, slave_id, 1, &pdata->i2c);
	if (ret) {
		dev_err(dev, "i2c dev get failed.\n");
		return ret;
	}

	return 0;
}

static const struct dm_regulator_ops tps6287x_regulator_ops = {
	.get_value  = tps6287x_regulator_get_value,
	.set_value  = tps6287x_regulator_set_value,
};

static const struct udevice_id tps6287x_regulator_ids[] = {
	{ .compatible = "ti,tps62873", .data = (ulong)&tps6287x_data },
	{ },
};

U_BOOT_DRIVER(tps6287x_regulator) = {
	.name = "tps6287x_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &tps6287x_regulator_ops,
	.of_match = tps6287x_regulator_ids,
	.plat_auto	= sizeof(struct tps6287x_regulator_pdata),
	.probe = tps6287x_regulator_probe,
};
