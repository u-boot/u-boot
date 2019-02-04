// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author: Christophe Kerello <christophe.kerello@st.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/stpmu1.h>

struct stpmu1_range {
	int min_uv;
	int min_sel;
	int max_sel;
	int step;
};

struct stpmu1_output_range {
	const struct stpmu1_range *ranges;
	int nbranges;
};

#define STPMU1_MODE(_id, _val, _name) { \
	.id = _id,			\
	.register_value = _val,		\
	.name = _name,			\
}

#define STPMU1_RANGE(_min_uv, _min_sel, _max_sel, _step) { \
	.min_uv = _min_uv,		\
	.min_sel = _min_sel,		\
	.max_sel = _max_sel,		\
	.step = _step,			\
}

#define STPMU1_OUTPUT_RANGE(_ranges, _nbranges) { \
	.ranges = _ranges,		\
	.nbranges = _nbranges,		\
}

static int stpmu1_output_find_uv(int sel,
				 const struct stpmu1_output_range *output_range)
{
	const struct stpmu1_range *range;
	int i;

	for (i = 0, range = output_range->ranges;
	     i < output_range->nbranges; i++, range++) {
		if (sel >= range->min_sel && sel <= range->max_sel)
			return range->min_uv +
			       (sel - range->min_sel) * range->step;
	}

	return -EINVAL;
}

static int stpmu1_output_find_sel(int uv,
				  const struct stpmu1_output_range *output_range)
{
	const struct stpmu1_range *range;
	int i;

	for (i = 0, range = output_range->ranges;
	     i < output_range->nbranges; i++, range++) {
		if (uv == range->min_uv && !range->step)
			return range->min_sel;

		if (uv >= range->min_uv &&
		    uv <= range->min_uv +
			  (range->max_sel - range->min_sel) * range->step)
			return range->min_sel +
			       (uv - range->min_uv) / range->step;
	}

	return -EINVAL;
}

/*
 * BUCK regulators
 */

static const struct stpmu1_range buck1_ranges[] = {
	STPMU1_RANGE(725000, 0, 4, 0),
	STPMU1_RANGE(725000, 5, 36, 25000),
	STPMU1_RANGE(1500000, 37, 63, 0),
};

static const struct stpmu1_range buck2_ranges[] = {
	STPMU1_RANGE(1000000, 0, 17, 0),
	STPMU1_RANGE(1050000, 18, 19, 0),
	STPMU1_RANGE(1100000, 20, 21, 0),
	STPMU1_RANGE(1150000, 22, 23, 0),
	STPMU1_RANGE(1200000, 24, 25, 0),
	STPMU1_RANGE(1250000, 26, 27, 0),
	STPMU1_RANGE(1300000, 28, 29, 0),
	STPMU1_RANGE(1350000, 30, 31, 0),
	STPMU1_RANGE(1400000, 32, 33, 0),
	STPMU1_RANGE(1450000, 34, 35, 0),
	STPMU1_RANGE(1500000, 36, 63, 0),
};

static const struct stpmu1_range buck3_ranges[] = {
	STPMU1_RANGE(1000000, 0, 19, 0),
	STPMU1_RANGE(1100000, 20, 23, 0),
	STPMU1_RANGE(1200000, 24, 27, 0),
	STPMU1_RANGE(1300000, 28, 31, 0),
	STPMU1_RANGE(1400000, 32, 35, 0),
	STPMU1_RANGE(1500000, 36, 55, 100000),
	STPMU1_RANGE(3400000, 56, 63, 0),
};

static const struct stpmu1_range buck4_ranges[] = {
	STPMU1_RANGE(600000, 0, 27, 25000),
	STPMU1_RANGE(1300000, 28, 29, 0),
	STPMU1_RANGE(1350000, 30, 31, 0),
	STPMU1_RANGE(1400000, 32, 33, 0),
	STPMU1_RANGE(1450000, 34, 35, 0),
	STPMU1_RANGE(1500000, 36, 60, 100000),
	STPMU1_RANGE(3900000, 61, 63, 0),
};

/* BUCK: 1,2,3,4 - voltage ranges */
static const struct stpmu1_output_range buck_voltage_range[] = {
	STPMU1_OUTPUT_RANGE(buck1_ranges, ARRAY_SIZE(buck1_ranges)),
	STPMU1_OUTPUT_RANGE(buck2_ranges, ARRAY_SIZE(buck2_ranges)),
	STPMU1_OUTPUT_RANGE(buck3_ranges, ARRAY_SIZE(buck3_ranges)),
	STPMU1_OUTPUT_RANGE(buck4_ranges, ARRAY_SIZE(buck4_ranges)),
};

/* BUCK modes */
static const struct dm_regulator_mode buck_modes[] = {
	STPMU1_MODE(STPMU1_BUCK_MODE_HP, STPMU1_BUCK_MODE_HP, "HP"),
	STPMU1_MODE(STPMU1_BUCK_MODE_LP, STPMU1_BUCK_MODE_LP, "LP"),
};

static int stpmu1_buck_get_uv(struct udevice *dev, int buck)
{
	int sel;

	sel = pmic_reg_read(dev, STPMU1_BUCKX_CTRL_REG(buck));
	if (sel < 0)
		return sel;

	sel &= STPMU1_BUCK_OUTPUT_MASK;
	sel >>= STPMU1_BUCK_OUTPUT_SHIFT;

	return stpmu1_output_find_uv(sel, &buck_voltage_range[buck]);
}

static int stpmu1_buck_get_value(struct udevice *dev)
{
	return stpmu1_buck_get_uv(dev->parent, dev->driver_data - 1);
}

static int stpmu1_buck_set_value(struct udevice *dev, int uv)
{
	int sel, buck = dev->driver_data - 1;

	sel = stpmu1_output_find_sel(uv, &buck_voltage_range[buck]);
	if (sel < 0)
		return sel;

	return pmic_clrsetbits(dev->parent,
			       STPMU1_BUCKX_CTRL_REG(buck),
			       STPMU1_BUCK_OUTPUT_MASK,
			       sel << STPMU1_BUCK_OUTPUT_SHIFT);
}

static int stpmu1_buck_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMU1_BUCKX_CTRL_REG(dev->driver_data - 1));
	if (ret < 0)
		return false;

	return ret & STPMU1_BUCK_EN ? true : false;
}

static int stpmu1_buck_set_enable(struct udevice *dev, bool enable)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int delay = enable ? STPMU1_DEFAULT_START_UP_DELAY_MS :
			     STPMU1_DEFAULT_STOP_DELAY_MS;
	int ret, uv;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmu1_buck_get_enable(dev) == enable)
		return 0;

	if (enable) {
		uc_pdata = dev_get_uclass_platdata(dev);
		uv = stpmu1_buck_get_value(dev);
		if ((uv < uc_pdata->min_uV) || (uv > uc_pdata->max_uV))
			stpmu1_buck_set_value(dev, uc_pdata->min_uV);
	}

	ret = pmic_clrsetbits(dev->parent,
			      STPMU1_BUCKX_CTRL_REG(dev->driver_data - 1),
			      STPMU1_BUCK_EN, enable ? STPMU1_BUCK_EN : 0);
	mdelay(delay);

	return ret;
}

static int stpmu1_buck_get_mode(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMU1_BUCKX_CTRL_REG(dev->driver_data - 1));
	if (ret < 0)
		return ret;

	return ret & STPMU1_BUCK_MODE ? STPMU1_BUCK_MODE_LP :
					 STPMU1_BUCK_MODE_HP;
}

static int stpmu1_buck_set_mode(struct udevice *dev, int mode)
{
	return pmic_clrsetbits(dev->parent,
			       STPMU1_BUCKX_CTRL_REG(dev->driver_data - 1),
			       STPMU1_BUCK_MODE,
			       mode ? STPMU1_BUCK_MODE : 0);
}

static int stpmu1_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMU1_MAX_BUCK)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = (struct dm_regulator_mode *)buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(buck_modes);

	return 0;
}

static const struct dm_regulator_ops stpmu1_buck_ops = {
	.get_value  = stpmu1_buck_get_value,
	.set_value  = stpmu1_buck_set_value,
	.get_enable = stpmu1_buck_get_enable,
	.set_enable = stpmu1_buck_set_enable,
	.get_mode   = stpmu1_buck_get_mode,
	.set_mode   = stpmu1_buck_set_mode,
};

U_BOOT_DRIVER(stpmu1_buck) = {
	.name = "stpmu1_buck",
	.id = UCLASS_REGULATOR,
	.ops = &stpmu1_buck_ops,
	.probe = stpmu1_buck_probe,
};

/*
 * LDO regulators
 */

static const struct stpmu1_range ldo12_ranges[] = {
	STPMU1_RANGE(1700000, 0, 7, 0),
	STPMU1_RANGE(1700000, 8, 24, 100000),
	STPMU1_RANGE(3300000, 25, 31, 0),
};

static const struct stpmu1_range ldo3_ranges[] = {
	STPMU1_RANGE(1700000, 0, 7, 0),
	STPMU1_RANGE(1700000, 8, 24, 100000),
	STPMU1_RANGE(3300000, 25, 30, 0),
	/* Sel 31 is special case when LDO3 is in mode sync_source (BUCK2/2) */
};

static const struct stpmu1_range ldo5_ranges[] = {
	STPMU1_RANGE(1700000, 0, 7, 0),
	STPMU1_RANGE(1700000, 8, 30, 100000),
	STPMU1_RANGE(3900000, 31, 31, 0),
};

static const struct stpmu1_range ldo6_ranges[] = {
	STPMU1_RANGE(900000, 0, 24, 100000),
	STPMU1_RANGE(3300000, 25, 31, 0),
};

/* LDO: 1,2,3,4,5,6 - voltage ranges */
static const struct stpmu1_output_range ldo_voltage_range[] = {
	STPMU1_OUTPUT_RANGE(ldo12_ranges, ARRAY_SIZE(ldo12_ranges)),
	STPMU1_OUTPUT_RANGE(ldo12_ranges, ARRAY_SIZE(ldo12_ranges)),
	STPMU1_OUTPUT_RANGE(ldo3_ranges, ARRAY_SIZE(ldo3_ranges)),
	STPMU1_OUTPUT_RANGE(NULL, 0),
	STPMU1_OUTPUT_RANGE(ldo5_ranges, ARRAY_SIZE(ldo5_ranges)),
	STPMU1_OUTPUT_RANGE(ldo6_ranges, ARRAY_SIZE(ldo6_ranges)),
};

/* LDO modes */
static const struct dm_regulator_mode ldo_modes[] = {
	STPMU1_MODE(STPMU1_LDO_MODE_NORMAL,
		    STPMU1_LDO_MODE_NORMAL, "NORMAL"),
	STPMU1_MODE(STPMU1_LDO_MODE_BYPASS,
		    STPMU1_LDO_MODE_BYPASS, "BYPASS"),
	STPMU1_MODE(STPMU1_LDO_MODE_SINK_SOURCE,
		    STPMU1_LDO_MODE_SINK_SOURCE, "SINK SOURCE"),
};

static int stpmu1_ldo_get_value(struct udevice *dev)
{
	int sel, ldo = dev->driver_data - 1;

	sel = pmic_reg_read(dev->parent, STPMU1_LDOX_CTRL_REG(ldo));
	if (sel < 0)
		return sel;

	/* ldo4 => 3,3V */
	if (ldo == STPMU1_LDO4)
		return STPMU1_LDO4_UV;

	sel &= STPMU1_LDO12356_OUTPUT_MASK;
	sel >>= STPMU1_LDO12356_OUTPUT_SHIFT;

	/* ldo3, sel = 31 => BUCK2/2 */
	if (ldo == STPMU1_LDO3 && sel == STPMU1_LDO3_DDR_SEL)
		return stpmu1_buck_get_uv(dev->parent, STPMU1_BUCK2) / 2;

	return stpmu1_output_find_uv(sel, &ldo_voltage_range[ldo]);
}

static int stpmu1_ldo_set_value(struct udevice *dev, int uv)
{
	int sel, ldo = dev->driver_data - 1;

	/* ldo4 => not possible */
	if (ldo == STPMU1_LDO4)
		return -EINVAL;

	sel = stpmu1_output_find_sel(uv, &ldo_voltage_range[ldo]);
	if (sel < 0)
		return sel;

	return pmic_clrsetbits(dev->parent,
			       STPMU1_LDOX_CTRL_REG(ldo),
			       STPMU1_LDO12356_OUTPUT_MASK,
			       sel << STPMU1_LDO12356_OUTPUT_SHIFT);
}

static int stpmu1_ldo_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent,
			    STPMU1_LDOX_CTRL_REG(dev->driver_data - 1));
	if (ret < 0)
		return false;

	return ret & STPMU1_LDO_EN ? true : false;
}

static int stpmu1_ldo_set_enable(struct udevice *dev, bool enable)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int delay = enable ? STPMU1_DEFAULT_START_UP_DELAY_MS :
			     STPMU1_DEFAULT_STOP_DELAY_MS;
	int ret, uv;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmu1_ldo_get_enable(dev) == enable)
		return 0;

	if (enable) {
		uc_pdata = dev_get_uclass_platdata(dev);
		uv = stpmu1_ldo_get_value(dev);
		if ((uv < uc_pdata->min_uV) || (uv > uc_pdata->max_uV))
			stpmu1_ldo_set_value(dev, uc_pdata->min_uV);
	}

	ret = pmic_clrsetbits(dev->parent,
			      STPMU1_LDOX_CTRL_REG(dev->driver_data - 1),
			      STPMU1_LDO_EN, enable ? STPMU1_LDO_EN : 0);
	mdelay(delay);

	return ret;
}

static int stpmu1_ldo_get_mode(struct udevice *dev)
{
	int ret, ldo = dev->driver_data - 1;

	if (ldo != STPMU1_LDO3)
		return -EINVAL;

	ret = pmic_reg_read(dev->parent, STPMU1_LDOX_CTRL_REG(ldo));
	if (ret < 0)
		return ret;

	if (ret & STPMU1_LDO3_MODE)
		return STPMU1_LDO_MODE_BYPASS;

	ret &= STPMU1_LDO12356_OUTPUT_MASK;
	ret >>= STPMU1_LDO12356_OUTPUT_SHIFT;

	return ret == STPMU1_LDO3_DDR_SEL ? STPMU1_LDO_MODE_SINK_SOURCE :
					     STPMU1_LDO_MODE_NORMAL;
}

static int stpmu1_ldo_set_mode(struct udevice *dev, int mode)
{
	int ret, ldo = dev->driver_data - 1;

	if (ldo != STPMU1_LDO3)
		return -EINVAL;

	ret = pmic_reg_read(dev->parent, STPMU1_LDOX_CTRL_REG(ldo));
	if (ret < 0)
		return ret;

	switch (mode) {
	case STPMU1_LDO_MODE_SINK_SOURCE:
		ret &= ~STPMU1_LDO12356_OUTPUT_MASK;
		ret |= STPMU1_LDO3_DDR_SEL << STPMU1_LDO12356_OUTPUT_SHIFT;
	case STPMU1_LDO_MODE_NORMAL:
		ret &= ~STPMU1_LDO3_MODE;
		break;
	case STPMU1_LDO_MODE_BYPASS:
		ret |= STPMU1_LDO3_MODE;
		break;
	}

	return pmic_reg_write(dev->parent, STPMU1_LDOX_CTRL_REG(ldo), ret);
}

static int stpmu1_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMU1_MAX_LDO)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	if (dev->driver_data - 1 == STPMU1_LDO3) {
		uc_pdata->mode = (struct dm_regulator_mode *)ldo_modes;
		uc_pdata->mode_count = ARRAY_SIZE(ldo_modes);
	} else {
		uc_pdata->mode_count = 0;
	}

	return 0;
}

static const struct dm_regulator_ops stpmu1_ldo_ops = {
	.get_value  = stpmu1_ldo_get_value,
	.set_value  = stpmu1_ldo_set_value,
	.get_enable = stpmu1_ldo_get_enable,
	.set_enable = stpmu1_ldo_set_enable,
	.get_mode   = stpmu1_ldo_get_mode,
	.set_mode   = stpmu1_ldo_set_mode,
};

U_BOOT_DRIVER(stpmu1_ldo) = {
	.name = "stpmu1_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &stpmu1_ldo_ops,
	.probe = stpmu1_ldo_probe,
};

/*
 * VREF DDR regulator
 */

static int stpmu1_vref_ddr_get_value(struct udevice *dev)
{
	/* BUCK2/2 */
	return stpmu1_buck_get_uv(dev->parent, STPMU1_BUCK2) / 2;
}

static int stpmu1_vref_ddr_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMU1_VREF_CTRL_REG);
	if (ret < 0)
		return false;

	return ret & STPMU1_VREF_EN ? true : false;
}

static int stpmu1_vref_ddr_set_enable(struct udevice *dev, bool enable)
{
	int delay = enable ? STPMU1_DEFAULT_START_UP_DELAY_MS :
			     STPMU1_DEFAULT_STOP_DELAY_MS;
	int ret;

	/* if regulator is already in the wanted state, nothing to do */
	if (stpmu1_vref_ddr_get_enable(dev) == enable)
		return 0;

	ret = pmic_clrsetbits(dev->parent, STPMU1_VREF_CTRL_REG,
			      STPMU1_VREF_EN, enable ? STPMU1_VREF_EN : 0);
	mdelay(delay);

	return ret;
}

static int stpmu1_vref_ddr_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmu1_vref_ddr_ops = {
	.get_value  = stpmu1_vref_ddr_get_value,
	.get_enable = stpmu1_vref_ddr_get_enable,
	.set_enable = stpmu1_vref_ddr_set_enable,
};

U_BOOT_DRIVER(stpmu1_vref_ddr) = {
	.name = "stpmu1_vref_ddr",
	.id = UCLASS_REGULATOR,
	.ops = &stpmu1_vref_ddr_ops,
	.probe = stpmu1_vref_ddr_probe,
};

/*
 * BOOST regulator
 */

static int stpmu1_boost_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMU1_USB_CTRL_REG);
	if (ret < 0)
		return false;

	return ret & STPMU1_USB_BOOST_EN ? true : false;
}

static int stpmu1_boost_set_enable(struct udevice *dev, bool enable)
{
	int ret;

	ret = pmic_reg_read(dev->parent, STPMU1_USB_CTRL_REG);
	if (ret < 0)
		return ret;

	if (!enable && ret & STPMU1_USB_PWR_SW_EN)
		return -EINVAL;

	/* if regulator is already in the wanted state, nothing to do */
	if (!!(ret & STPMU1_USB_BOOST_EN) == enable)
		return 0;

	ret = pmic_clrsetbits(dev->parent, STPMU1_USB_CTRL_REG,
			      STPMU1_USB_BOOST_EN,
			      enable ? STPMU1_USB_BOOST_EN : 0);
	if (enable)
		mdelay(STPMU1_USB_BOOST_START_UP_DELAY_MS);

	return ret;
}

static int stpmu1_boost_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmu1_boost_ops = {
	.get_enable = stpmu1_boost_get_enable,
	.set_enable = stpmu1_boost_set_enable,
};

U_BOOT_DRIVER(stpmu1_boost) = {
	.name = "stpmu1_boost",
	.id = UCLASS_REGULATOR,
	.ops = &stpmu1_boost_ops,
	.probe = stpmu1_boost_probe,
};

/*
 * USB power switch
 */

static int stpmu1_pwr_sw_get_enable(struct udevice *dev)
{
	uint mask = 1 << dev->driver_data;
	int ret;

	ret = pmic_reg_read(dev->parent, STPMU1_USB_CTRL_REG);
	if (ret < 0)
		return false;

	return ret & mask ? true : false;
}

static int stpmu1_pwr_sw_set_enable(struct udevice *dev, bool enable)
{
	uint mask = 1 << dev->driver_data;
	int delay = enable ? STPMU1_DEFAULT_START_UP_DELAY_MS :
			     STPMU1_DEFAULT_STOP_DELAY_MS;
	int ret;

	ret = pmic_reg_read(dev->parent, STPMU1_USB_CTRL_REG);
	if (ret < 0)
		return ret;

	/* if regulator is already in the wanted state, nothing to do */
	if (!!(ret & mask) == enable)
		return 0;

	/* Boost management */
	if (enable && !(ret & STPMU1_USB_BOOST_EN)) {
		pmic_clrsetbits(dev->parent, STPMU1_USB_CTRL_REG,
				STPMU1_USB_BOOST_EN, STPMU1_USB_BOOST_EN);
		mdelay(STPMU1_USB_BOOST_START_UP_DELAY_MS);
	} else if (!enable && ret & STPMU1_USB_BOOST_EN &&
		   (ret & STPMU1_USB_PWR_SW_EN) != STPMU1_USB_PWR_SW_EN) {
		pmic_clrsetbits(dev->parent, STPMU1_USB_CTRL_REG,
				STPMU1_USB_BOOST_EN, 0);
	}

	ret = pmic_clrsetbits(dev->parent, STPMU1_USB_CTRL_REG,
			      mask, enable ? mask : 0);
	mdelay(delay);

	return ret;
}

static int stpmu1_pwr_sw_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	if (!dev->driver_data || dev->driver_data > STPMU1_MAX_PWR_SW)
		return -EINVAL;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops stpmu1_pwr_sw_ops = {
	.get_enable = stpmu1_pwr_sw_get_enable,
	.set_enable = stpmu1_pwr_sw_set_enable,
};

U_BOOT_DRIVER(stpmu1_pwr_sw) = {
	.name = "stpmu1_pwr_sw",
	.id = UCLASS_REGULATOR,
	.ops = &stpmu1_pwr_sw_ops,
	.probe = stpmu1_pwr_sw_probe,
};
