// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025-2026 RISCStar Ltd.
 */

#include <dm.h>
#include <dm/lists.h>
#include <errno.h>
#include <log.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/spacemit_p1.h>

struct p1_reg_info {
	uint min_uv;
	uint step_uv;
	u8 vsel_reg;
	u8 vsel_sleep_reg;
	u8 config_reg;
	u8 vsel_mask;
	u8 min_sel;
	u8 max_sel;
};

static const struct p1_reg_info p1_bucks[] = {
	/* BUCK 1 */
	{ 500000,  5000, P1_BUCK_VSEL(1), P1_BUCK_SVSEL(1), P1_BUCK_CTRL(1),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(1), P1_BUCK_SVSEL(1), P1_BUCK_CTRL(1),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
	/* BUCK 2 */
	{ 500000,  5000, P1_BUCK_VSEL(2), P1_BUCK_SVSEL(2), P1_BUCK_CTRL(2),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(2), P1_BUCK_SVSEL(2), P1_BUCK_CTRL(2),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
	/* BUCK 3 */
	{ 500000,  5000, P1_BUCK_VSEL(3), P1_BUCK_SVSEL(3), P1_BUCK_CTRL(3),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(3), P1_BUCK_SVSEL(3), P1_BUCK_CTRL(3),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
	/* BUCK 4 */
	{ 500000,  5000, P1_BUCK_VSEL(4), P1_BUCK_SVSEL(4), P1_BUCK_CTRL(4),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(4), P1_BUCK_SVSEL(4), P1_BUCK_CTRL(4),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
	/* BUCK 5 */
	{ 500000,  5000, P1_BUCK_VSEL(5), P1_BUCK_SVSEL(5), P1_BUCK_CTRL(5),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(5), P1_BUCK_SVSEL(5), P1_BUCK_CTRL(5),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
	/* BUCK 6 */
	{ 500000,  5000, P1_BUCK_VSEL(6), P1_BUCK_SVSEL(6), P1_BUCK_CTRL(6),
	  BUCK_VSEL_MASK, 0x00, 0xaa },
	{ 1375000, 25000, P1_BUCK_VSEL(6), P1_BUCK_SVSEL(6), P1_BUCK_CTRL(6),
	  BUCK_VSEL_MASK, 0xab, 0xfe },
};

static const struct p1_reg_info p1_aldos[] = {
	/* ALDO 1 */
	{ 500000, 25000, P1_ALDO_VOLT(1), P1_ALDO_SVOLT(1), P1_ALDO_CTRL(1),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* ALDO 2 */
	{ 500000, 25000, P1_ALDO_VOLT(2), P1_ALDO_SVOLT(2), P1_ALDO_CTRL(2),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* ALDO 3 */
	{ 500000, 25000, P1_ALDO_VOLT(3), P1_ALDO_SVOLT(3), P1_ALDO_CTRL(3),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* ALDO 4 */
	{ 500000, 25000, P1_ALDO_VOLT(4), P1_ALDO_SVOLT(4), P1_ALDO_CTRL(4),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
};

static const struct p1_reg_info p1_dldos[] = {
	/* DLDO 1 */
	{ 500000, 25000, P1_DLDO_VOLT(1), P1_DLDO_SVOLT(1), P1_DLDO_CTRL(1),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 2 */
	{ 500000, 25000, P1_DLDO_VOLT(2), P1_DLDO_SVOLT(2), P1_DLDO_CTRL(2),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 3 */
	{ 500000, 25000, P1_DLDO_VOLT(3), P1_DLDO_SVOLT(3), P1_DLDO_CTRL(3),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 4 */
	{ 500000, 25000, P1_DLDO_VOLT(4), P1_DLDO_SVOLT(4), P1_DLDO_CTRL(4),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 5 */
	{ 500000, 25000, P1_DLDO_VOLT(5), P1_DLDO_SVOLT(5), P1_DLDO_CTRL(5),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 6 */
	{ 500000, 25000, P1_DLDO_VOLT(6), P1_DLDO_SVOLT(6), P1_DLDO_CTRL(6),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
	/* DLDO 7 */
	{ 500000, 25000, P1_DLDO_VOLT(7), P1_DLDO_SVOLT(7), P1_DLDO_CTRL(7),
	  ALDO_VSEL_MASK, 0x0b, 0x7f },
};

static const struct p1_reg_info *get_buck_reg(struct udevice *pmic,
					      int idx, int uvolt)
{
	if (idx < 0)
		return NULL;
	if (uvolt < 1375000)
		return &p1_bucks[(idx - 1) * 2 + 0];
	return &p1_bucks[(idx - 1) * 2 + 1];
}

static const struct p1_reg_info *get_aldo_reg(struct udevice *pmic,
					      int idx, int uvolt)
{
	return &p1_aldos[idx];
}

static const struct p1_reg_info *get_dldo_reg(struct udevice *pmic,
					      int idx, int uvolt)
{
	return &p1_dldos[idx];
}

static int buck_get_value(struct udevice *dev)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->read)
		return -ENOSYS;

	info = get_buck_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;
	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;
	val = ret & info->vsel_mask;
	while (val > info->max_sel)
		info++;

	return info->min_uv + (val - info->min_sel) * info->step_uv;
}

static int buck_set_value(struct udevice *dev, int uvolt)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->write)
		return -ENOSYS;

	info = get_buck_reg(dev->parent, dev->driver_data, uvolt);
	if (!info)
		return -ENOENT;
	val = (uvolt - info->min_uv);
	val = val / info->step_uv;
	val += info->min_sel;
	ret = pmic_reg_write(dev->parent, info->vsel_reg, val);
	if (ret < 0)
		return ret;
	return 0;
}

static int buck_get_enable(struct udevice *dev)
{
	const struct p1_reg_info *info;
	int ret;

	info = get_buck_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	return ret & BUCK_EN_MASK;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	const struct p1_reg_info *info;
	uint val;
	int ret;

	info = get_buck_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	val = (unsigned int)ret;
	val &= BUCK_EN_MASK;

	if (enable == val)
		return 0;

	val = enable;
	ret = pmic_clrsetbits(dev->parent, info->config_reg, BUCK_EN_MASK, val);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct dm_regulator_ops p1_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

static int p1_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = 0;

	return 0;
}

U_BOOT_DRIVER(p1_buck) = {
	.name		= P1_BUCK_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &p1_buck_ops,
	.probe		= p1_buck_probe,
};

static int aldo_get_value(struct udevice *dev)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->read)
		return -ENOSYS;

	info = get_aldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;

	val = ret & info->vsel_mask;
	while (val > info->max_sel)
		info++;

	return info->min_uv + (val - info->min_sel) * info->step_uv;
}

static int aldo_set_value(struct udevice *dev, int uvolt)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->write)
		return -ENOSYS;

	info = get_aldo_reg(dev->parent, dev->driver_data, uvolt);
	if (!info)
		return -ENOENT;
	val = (uvolt - info->min_uv);
	val = val / info->step_uv;
	val += info->min_sel;
	ret = pmic_reg_write(dev->parent, info->vsel_reg, val);
	if (ret < 0)
		return ret;
	return 0;
}

static int aldo_get_enable(struct udevice *dev)
{
	const struct p1_reg_info *info;
	int ret;

	info = get_aldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	return ret & ALDO_EN_MASK;
}

static int aldo_set_enable(struct udevice *dev, bool enable)
{
	const struct p1_reg_info *info;
	uint val;
	int ret;

	info = get_aldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	val = (unsigned int)ret;
	val &= ALDO_EN_MASK;

	if (enable == val)
		return 0;

	val = enable;
	ret = pmic_clrsetbits(dev->parent, info->config_reg, ALDO_EN_MASK, val);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct dm_regulator_ops p1_aldo_ops = {
	.get_value	= aldo_get_value,
	.set_value	= aldo_set_value,
	.get_enable	= aldo_get_enable,
	.set_enable	= aldo_set_enable,
};

static int p1_aldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}

U_BOOT_DRIVER(p1_aldo) = {
	.name		= P1_ALDO_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &p1_aldo_ops,
	.probe		= p1_aldo_probe,
};

static int dldo_get_value(struct udevice *dev)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->read)
		return -ENOSYS;

	info = get_dldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;

	val = ret & info->vsel_mask;
	while (val > info->max_sel)
		info++;

	return info->min_uv + (val - info->min_sel) * info->step_uv;
}

static int dldo_set_value(struct udevice *dev, int uvolt)
{
	const struct dm_pmic_ops *ops = device_get_ops(dev->parent);
	const struct p1_reg_info *info;
	uint val;
	int ret;

	if (!ops || !ops->write)
		return -ENOSYS;

	info = get_dldo_reg(dev->parent, dev->driver_data, uvolt);
	if (!info)
		return -ENOENT;
	val = (uvolt - info->min_uv);
	val = val / info->step_uv;
	val += info->min_sel;
	ret = pmic_reg_write(dev->parent, info->vsel_reg, val);
	if (ret < 0)
		return ret;
	return 0;
}

static int dldo_get_enable(struct udevice *dev)
{
	const struct p1_reg_info *info;
	int ret;

	info = get_dldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	return ret & DLDO_EN_MASK;
}

static int dldo_set_enable(struct udevice *dev, bool enable)
{
	const struct p1_reg_info *info;
	uint val;
	int ret;

	info = get_dldo_reg(dev->parent, dev->driver_data, 0);
	if (!info)
		return -ENOENT;

	ret = pmic_reg_read(dev->parent, info->config_reg);
	if (ret < 0)
		return ret;
	val = (unsigned int)ret;
	val &= DLDO_EN_MASK;

	if (enable == val)
		return 0;

	val = enable;
	ret = pmic_clrsetbits(dev->parent, info->config_reg, DLDO_EN_MASK, val);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct dm_regulator_ops p1_dldo_ops = {
	.get_value	= dldo_get_value,
	.set_value	= dldo_set_value,
	.get_enable	= dldo_get_enable,
	.set_enable	= dldo_set_enable,
};

static int p1_dldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}

U_BOOT_DRIVER(p1_dldo) = {
	.name		= P1_DLDO_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &p1_dldo_ops,
	.probe		= p1_dldo_probe,
};
