/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Based on Rockchip's drivers/power/pmic/pmic_rk808.c:
 * Copyright (C) 2012 rockchips
 * zyw <zyw@rock-chips.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/rk808_pmic.h>
#include <power/pmic.h>
#include <power/regulator.h>

#ifndef CONFIG_SPL_BUILD
#define ENABLE_DRIVER
#endif

struct rk808_reg_info {
	uint min_uv;
	uint step_uv;
	s8 vsel_reg;
	u8 vsel_bits;
};

static const struct rk808_reg_info rk808_buck[] = {
	{ 712500, 12500, REG_BUCK1_ON_VSEL, 6, },
	{ 712500, 12500, REG_BUCK2_ON_VSEL, 6, },
	{ 712500, 12500, -1, 6, },
	{ 1800000, 100000, REG_BUCK4_ON_VSEL, 4, },
};

static const struct rk808_reg_info rk808_ldo[] = {
	{ 1800000, 100000, LDO1_ON_VSEL, 5, },
	{ 1800000, 100000, LDO2_ON_VSEL, 5, },
	{ 800000, 100000, LDO3_ON_VSEL, 4, },
	{ 1800000, 100000, LDO4_ON_VSEL, 5, },
	{ 1800000, 100000, LDO5_ON_VSEL, 5, },
	{ 800000, 100000, LDO6_ON_VSEL, 5, },
	{ 800000, 100000, LDO7_ON_VSEL, 5, },
	{ 1800000, 100000, LDO8_ON_VSEL, 5, },
};


static int _buck_set_value(struct udevice *pmic, int buck, int uvolt)
{
	const struct rk808_reg_info *info = &rk808_buck[buck - 1];
	int mask = (1 << info->vsel_bits) - 1;
	int val;

	if (info->vsel_reg == -1)
		return -ENOSYS;
	val = (uvolt - info->min_uv) / info->step_uv;
	debug("%s: reg=%x, mask=%x, val=%x\n", __func__, info->vsel_reg, mask,
	      val);

	return pmic_clrsetbits(pmic, info->vsel_reg, mask, val);
}

static int _buck_set_enable(struct udevice *pmic, int buck, bool enable)
{
	uint mask;
	int ret;

	buck--;
	mask = 1 << buck;
	if (enable) {
		ret = pmic_clrsetbits(pmic, DCDC_ILMAX, 0, 3 << (buck * 2));
		if (ret)
			return ret;
		ret = pmic_clrsetbits(pmic, REG_DCDC_UV_ACT, 1 << buck, 0);
		if (ret)
			return ret;
	}

	return pmic_clrsetbits(pmic, REG_DCDC_EN, mask, enable ? mask : 0);
}

#ifdef ENABLE_DRIVER
static int buck_get_value(struct udevice *dev)
{
	int buck = dev->driver_data - 1;
	const struct rk808_reg_info *info = &rk808_buck[buck];
	int mask = (1 << info->vsel_bits) - 1;
	int ret, val;

	if (info->vsel_reg == -1)
		return -ENOSYS;
	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;
	val = ret & mask;

	return info->min_uv + val * info->step_uv;
}

static int buck_set_value(struct udevice *dev, int uvolt)
{
	int buck = dev->driver_data;

	return _buck_set_value(dev->parent, buck, uvolt);
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	int buck = dev->driver_data;

	return _buck_set_enable(dev->parent, buck, enable);
}

static bool buck_get_enable(struct udevice *dev)
{
	int buck = dev->driver_data - 1;
	int ret;
	uint mask;

	mask = 1 << buck;

	ret = pmic_reg_read(dev->parent, REG_DCDC_EN);
	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int ldo_get_value(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;
	const struct rk808_reg_info *info = &rk808_ldo[ldo];
	int mask = (1 << info->vsel_bits) - 1;
	int ret, val;

	if (info->vsel_reg == -1)
		return -ENOSYS;
	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;
	val = ret & mask;

	return info->min_uv + val * info->step_uv;
}

static int ldo_set_value(struct udevice *dev, int uvolt)
{
	int ldo = dev->driver_data - 1;
	const struct rk808_reg_info *info = &rk808_ldo[ldo];
	int mask = (1 << info->vsel_bits) - 1;
	int val;

	if (info->vsel_reg == -1)
		return -ENOSYS;
	val = (uvolt - info->min_uv) / info->step_uv;
	debug("%s: reg=%x, mask=%x, val=%x\n", __func__, info->vsel_reg, mask,
	      val);

	return pmic_clrsetbits(dev->parent, info->vsel_reg, mask, val);
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	int ldo = dev->driver_data - 1;
	uint mask;

	mask = 1 << ldo;

	return pmic_clrsetbits(dev->parent, REG_LDO_EN, mask,
			       enable ? mask : 0);
}

static bool ldo_get_enable(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;
	int ret;
	uint mask;

	mask = 1 << ldo;

	ret = pmic_reg_read(dev->parent, REG_LDO_EN);
	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int switch_set_enable(struct udevice *dev, bool enable)
{
	int sw = dev->driver_data - 1;
	uint mask;

	mask = 1 << (sw + 5);

	return pmic_clrsetbits(dev->parent, REG_DCDC_EN, mask,
			       enable ? mask : 0);
}

static bool switch_get_enable(struct udevice *dev)
{
	int sw = dev->driver_data - 1;
	int ret;
	uint mask;

	mask = 1 << (sw + 5);

	ret = pmic_reg_read(dev->parent, REG_DCDC_EN);
	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int rk808_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = 0;

	return 0;
}

static int rk808_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}

static int rk808_switch_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops rk808_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

static const struct dm_regulator_ops rk808_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

static const struct dm_regulator_ops rk808_switch_ops = {
	.get_enable = switch_get_enable,
	.set_enable = switch_set_enable,
};

U_BOOT_DRIVER(rk808_buck) = {
	.name = "rk808_buck",
	.id = UCLASS_REGULATOR,
	.ops = &rk808_buck_ops,
	.probe = rk808_buck_probe,
};

U_BOOT_DRIVER(rk808_ldo) = {
	.name = "rk808_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &rk808_ldo_ops,
	.probe = rk808_ldo_probe,
};

U_BOOT_DRIVER(rk808_switch) = {
	.name = "rk808_switch",
	.id = UCLASS_REGULATOR,
	.ops = &rk808_switch_ops,
	.probe = rk808_switch_probe,
};
#endif

int rk808_spl_configure_buck(struct udevice *pmic, int buck, int uvolt)
{
	int ret;

	ret = _buck_set_value(pmic, buck, uvolt);
	if (ret)
		return ret;

	return _buck_set_enable(pmic, buck, true);
}
