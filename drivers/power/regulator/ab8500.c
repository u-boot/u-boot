// SPDX-License-Identifier: GPL-2.0+
/* ST-Ericsson AB8500/AB8505 LDO AUX3 regulator */

#include <dm.h>
#include <dm/lists.h>
#include <linux/errno.h>
#include <power/ab8500.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define AB8500_VAUX3_REGU		AB8500_REGU_CTRL2(0x0a)
#define AB8500_VAUX3_REGU_EN_MASK	GENMASK(1, 0)
#define AB8500_VAUX3_REGU_EN		BIT(0)

#define AB8500_VAUX3_SEL	AB8500_REGU_CTRL2(0x21)
#define AB8500_VAUX3_SEL_MASK	GENMASK(2, 0)

#define AB8505_VAUX3_SEL3	AB8500_REGU_CTRL2(0x01)

#define AB8505_VAUX3_SEL3_MASK	BIT(4)
#define AB8505_VAUX3_SEL3_UV	3050000

static const int ab8500_vaux3_voltages[] = {
	1200000, 1500000, 1800000, 2100000,
	2500000, 2750000, 2790000, 2910000,
};

static struct udevice *ab8500_regulator_pmic(struct udevice *dev)
{
	return dev->parent->parent;
}

static int ab8500_regulator_get_value(struct udevice *dev)
{
	struct udevice *pmic = ab8500_regulator_pmic(dev);
	int ret;

	if (device_is_compatible(pmic, "stericsson,ab8505")) {
		ret = pmic_reg_read(pmic, AB8505_VAUX3_SEL3);
		if (ret < 0)
			return ret;
		if (ret & AB8505_VAUX3_SEL3_MASK)
			return AB8505_VAUX3_SEL3_UV;
	}

	ret = pmic_reg_read(pmic, AB8500_VAUX3_SEL);
	if (ret < 0)
		return ret;
	ret &= AB8500_VAUX3_SEL_MASK;

	return ab8500_vaux3_voltages[ret];
}

static int ab8500_regulator_set_value(struct udevice *dev, int uV)
{
	struct udevice *pmic = ab8500_regulator_pmic(dev);
	bool is_ab8505 = device_is_compatible(pmic, "stericsson,ab8505");
	int ret;
	int i;

	if (uV == AB8505_VAUX3_SEL3_UV) {
		if (!is_ab8505)
			return -EINVAL;

		return pmic_clrsetbits(pmic, AB8505_VAUX3_SEL3,
				       AB8505_VAUX3_SEL3_MASK,
				       AB8505_VAUX3_SEL3_MASK);
	}

	for (i = 0; i < ARRAY_SIZE(ab8500_vaux3_voltages); i++) {
		if (ab8500_vaux3_voltages[i] != uV)
			continue;

		ret = pmic_clrsetbits(pmic, AB8500_VAUX3_SEL,
				      AB8500_VAUX3_SEL_MASK, i);
		if (ret || !is_ab8505)
			return ret;

		return pmic_clrsetbits(pmic, AB8505_VAUX3_SEL3,
				       AB8505_VAUX3_SEL3_MASK, 0);
	}

	return -EINVAL;
}

static int ab8500_regulator_set_value_clamp(struct udevice *dev, int min_uV,
					    int target_uV, int max_uV)
{
	struct udevice *pmic = ab8500_regulator_pmic(dev);
	int best_uV = -EINVAL;
	int i;

	for (i = 0; i < ARRAY_SIZE(ab8500_vaux3_voltages); i++) {
		int uV = ab8500_vaux3_voltages[i];

		if (uV < min_uV || uV > max_uV)
			continue;
		if (best_uV < 0 || abs(uV - target_uV) <
				   abs(best_uV - target_uV))
			best_uV = uV;
	}

	if (device_is_compatible(pmic, "stericsson,ab8505") &&
	    min_uV <= AB8505_VAUX3_SEL3_UV &&
	    max_uV >= AB8505_VAUX3_SEL3_UV &&
	    (best_uV < 0 || abs(AB8505_VAUX3_SEL3_UV - target_uV) <
	     abs(best_uV - target_uV)))
		best_uV = AB8505_VAUX3_SEL3_UV;

	if (best_uV < 0)
		return best_uV;

	return ab8500_regulator_set_value(dev, best_uV);
}

static int ab8500_regulator_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(ab8500_regulator_pmic(dev), AB8500_VAUX3_REGU);
	if (ret < 0)
		return ret;

	return !!(ret & AB8500_VAUX3_REGU_EN_MASK);
}

static int ab8500_regulator_set_enable(struct udevice *dev, bool enable)
{
	return pmic_clrsetbits(ab8500_regulator_pmic(dev), AB8500_VAUX3_REGU,
			       AB8500_VAUX3_REGU_EN_MASK,
			       enable ? AB8500_VAUX3_REGU_EN : 0);
}

static const struct dm_regulator_ops ab8500_regulator_ops = {
	.get_value = ab8500_regulator_get_value,
	.set_value = ab8500_regulator_set_value,
	.set_value_clamp = ab8500_regulator_set_value_clamp,
	.get_enable = ab8500_regulator_get_enable,
	.set_enable = ab8500_regulator_set_enable,
};

U_BOOT_DRIVER(ab8500_ldo_aux3) = {
	.name = "ab8500_ldo_aux3",
	.id = UCLASS_REGULATOR,
	.ops = &ab8500_regulator_ops,
};

static int ab8500_regulators_bind(struct udevice *dev)
{
	ofnode node;

	node = dev_read_subnode(dev, "ab8500_ldo_aux3");
	if (!ofnode_valid(node))
		return -ENODEV;

	return device_bind_driver_to_node(dev, "ab8500_ldo_aux3",
					  ofnode_get_name(node), node, NULL);
}

static const struct udevice_id ab8500_regulator_ids[] = {
	{ .compatible = "stericsson,ab8500-regulator" },
	{ .compatible = "stericsson,ab8505-regulator" },
	{ }
};

U_BOOT_DRIVER(ab8500_regulators) = {
	.name = "ab8500_regulators",
	.id = UCLASS_NOP,
	.of_match = ab8500_regulator_ids,
	.bind = ab8500_regulators_bind,
};
