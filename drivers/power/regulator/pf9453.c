// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NXP PF9453 regulator driver
 * Copyright 2024 NXP
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <linux/bitops.h>
#include <power/pf9453.h>
#include <power/pmic.h>
#include <power/regulator.h>

/**
 * struct pf9453_vrange - describe linear range of voltages
 *
 * @min_volt:	smallest voltage in range
 * @step:	how much voltage changes at each selector step
 * @min_sel:	smallest selector in the range
 * @max_sel:	maximum selector in the range
 */
struct pf9453_vrange {
	unsigned int	min_volt;
	unsigned int	step;
	u8		min_sel;
	u8		max_sel;
};

/**
 * struct pf9453_plat - describe regulator control registers
 *
 * @name:	name of the regulator. Used for matching the dt-entry
 * @enable_reg:	register address used to enable/disable regulator
 * @enablemask:	register mask used to enable/disable regulator
 * @volt_reg:	register address used to configure regulator voltage
 * @volt_mask:	register mask used to configure regulator voltage
 * @ranges:	pointer to ranges of regulator voltages and matching register
 *		values
 * @numranges:	number of voltage ranges pointed by ranges
 */
struct pf9453_plat {
	const char		*name;
	u8			enable_reg;
	u8			enablemask;
	u8			volt_reg;
	u8			volt_mask;
	struct pf9453_vrange	*ranges;
	unsigned int		numranges;
};

#define PCA_RANGE(_min, _vstep, _sel_low, _sel_hi) \
{ \
	.min_volt = (_min), .step = (_vstep), \
	.min_sel = (_sel_low), .max_sel = (_sel_hi), \
}

#define PCA_DATA(_name, enreg, enmask, vreg, vmask, _range, _numranges) \
{ \
	.name = (_name), .enable_reg = (enreg), .enablemask = (enmask), \
	.volt_reg = (vreg), .volt_mask = (vmask), .ranges = (_range), \
	.numranges = _numranges \
}

static struct pf9453_vrange pf9453_buck134_vranges[] = {
	PCA_RANGE(600000, 25000, 0, 0x7f),
};

static struct pf9453_vrange pf9453_buck2_vranges[] = {
	PCA_RANGE(600000, 12500, 0, 0x7f),
};

static struct pf9453_vrange pf9453_ldo1_vranges[] = {
	PCA_RANGE(800000, 25000, 0x0, 0x64),
};

static struct pf9453_vrange pf9453_ldo2_vranges[] = {
	PCA_RANGE(500000, 25000, 0x0, 0x3a),
};

static struct pf9453_vrange pf9453_ldosnvs_vranges[] = {
	PCA_RANGE(800000, 25000, 0x0, 0x58),
};

static struct pf9453_plat pf9453_reg_data[] = {
	PCA_DATA("BUCK1", PF9453_BUCK1CTRL, PF9453_EN_MODE_MASK,
		 PF9453_BUCK1OUT, PF9453_BUCK_RUN_MASK,
		 pf9453_buck134_vranges, ARRAY_SIZE(pf9453_buck134_vranges)),
	PCA_DATA("BUCK2", PF9453_BUCK2CTRL, PF9453_EN_MODE_MASK,
		 PF9453_BUCK2OUT, PF9453_BUCK_RUN_MASK,
		 pf9453_buck2_vranges, ARRAY_SIZE(pf9453_buck2_vranges)),
	PCA_DATA("BUCK3", PF9453_BUCK3CTRL, PF9453_EN_MODE_MASK,
		 PF9453_BUCK3OUT, PF9453_BUCK_RUN_MASK,
		 pf9453_buck134_vranges, ARRAY_SIZE(pf9453_buck134_vranges)),
	PCA_DATA("BUCK4", PF9453_BUCK4CTRL, PF9453_EN_MODE_MASK,
		 PF9453_BUCK4OUT, PF9453_BUCK_RUN_MASK,
		 pf9453_buck134_vranges, ARRAY_SIZE(pf9453_buck134_vranges)),
	/* LDOs */
	PCA_DATA("LDO1", PF9453_LDO1CFG, PF9453_EN_MODE_MASK,
		 PF9453_LDO1OUT_H, PF9453_LDO1_MASK,
		 pf9453_ldo1_vranges, ARRAY_SIZE(pf9453_ldo1_vranges)),
	PCA_DATA("LDO2", PF9453_LDO2CFG, PF9453_EN_MODE_MASK,
		 PF9453_LDO2OUT, PF9453_LDO2_MASK,
		 pf9453_ldo2_vranges, ARRAY_SIZE(pf9453_ldo2_vranges)),
	PCA_DATA("LDO-SNVS", PF9453_LDOSNVS_CFG2, PF9453_EN_MODE_MASK,
		 PF9453_LDOSNVS_CFG1, PF9453_LDOSNVS_MASK,
		 pf9453_ldosnvs_vranges, ARRAY_SIZE(pf9453_ldosnvs_vranges)),
};

static int vrange_find_value(struct pf9453_vrange *r, unsigned int sel,
			     unsigned int *val)
{
	if (!val || sel < r->min_sel || sel > r->max_sel)
		return -EINVAL;

	*val = r->min_volt + r->step * (sel - r->min_sel);
	return 0;
}

static int vrange_find_selector(struct pf9453_vrange *r, int val,
				unsigned int *sel)
{
	int ret = -EINVAL;
	int num_vals = r->max_sel - r->min_sel + 1;

	if (val >= r->min_volt &&
	    val <= r->min_volt + r->step * (num_vals - 1)) {
		if (r->step) {
			*sel = r->min_sel + ((val - r->min_volt) / r->step);
			ret = 0;
		} else {
			*sel = r->min_sel;
			ret = 0;
		}
	}
	return ret;
}

static int pf9453_get_enable(struct udevice *dev)
{
	struct pf9453_plat *plat = dev_get_plat(dev);
	int val;

	val = pmic_reg_read(dev->parent, plat->enable_reg);
	if (val < 0)
		return val;

	return (val & plat->enablemask);
}

static int pf9453_set_enable(struct udevice *dev, bool enable)
{
	int val = 0;
	struct pf9453_plat *plat = dev_get_plat(dev);

	if (enable)
		val = plat->enablemask;

	return pmic_clrsetbits(dev->parent, plat->enable_reg, plat->enablemask,
			       val);
}

static int pf9453_get_value(struct udevice *dev)
{
	struct pf9453_plat *plat = dev_get_plat(dev);
	unsigned int reg, tmp;
	int i, ret;

	ret = pmic_reg_read(dev->parent, plat->volt_reg);
	if (ret < 0)
		return ret;

	reg = ret;
	reg &= plat->volt_mask;

	for (i = 0; i < plat->numranges; i++) {
		struct pf9453_vrange *r = &plat->ranges[i];

		if (!vrange_find_value(r, reg, &tmp))
			return tmp;
	}

	pr_err("Unknown voltage value read from pmic\n");

	return -EINVAL;
}

static int pf9453_set_value(struct udevice *dev, int uvolt)
{
	struct pf9453_plat *plat = dev_get_plat(dev);
	unsigned int sel;
	int i, found = 0;

	for (i = 0; i < plat->numranges; i++) {
		struct pf9453_vrange *r = &plat->ranges[i];

		found = !vrange_find_selector(r, uvolt, &sel);
		if (found) {
			unsigned int tmp;

			/*
			 * We require exactly the requested value to be
			 * supported - this can be changed later if needed
			 */
			found = !vrange_find_value(r, sel, &tmp);
			if (found && tmp == uvolt)
				break;
			found = 0;
		}
	}

	if (!found)
		return -EINVAL;

	return pmic_clrsetbits(dev->parent, plat->volt_reg,
			       plat->volt_mask, sel);
}

static int pf9453_regulator_probe(struct udevice *dev)
{
	struct pf9453_plat *plat = dev_get_plat(dev);
	int i, type;

	type = dev_get_driver_data(dev_get_parent(dev));

	if (type != NXP_CHIP_TYPE_PF9453) {
		debug("Unknown PMIC type\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(pf9453_reg_data); i++) {
		if (strcmp(dev->name, pf9453_reg_data[i].name))
			continue;
		*plat = pf9453_reg_data[i];
		return 0;
	}

	pr_err("Unknown regulator '%s'\n", dev->name);

	return -ENOENT;
}

static const struct dm_regulator_ops pf9453_regulator_ops = {
	.get_value	= pf9453_get_value,
	.set_value	= pf9453_set_value,
	.get_enable	= pf9453_get_enable,
	.set_enable	= pf9453_set_enable,
};

U_BOOT_DRIVER(pf9453_regulator) = {
	.name		= PF9453_REGULATOR_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &pf9453_regulator_ops,
	.probe		= pf9453_regulator_probe,
	.plat_auto	= sizeof(struct pf9453_plat),
};
