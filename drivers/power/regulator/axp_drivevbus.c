// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define AXP_VBUS_IPSOUT			0x30
#define AXP_VBUS_IPSOUT_DRIVEBUS		BIT(2)
#define AXP_MISC_CTRL			0x8f
#define AXP_MISC_CTRL_N_VBUSEN_FUNC		BIT(4)

static int axp_drivevbus_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, AXP_VBUS_IPSOUT);
	if (ret < 0)
		return ret;

	return !!(ret & AXP_VBUS_IPSOUT_DRIVEBUS);
}

static int axp_drivevbus_set_enable(struct udevice *dev, bool enable)
{
	return pmic_clrsetbits(dev->parent, AXP_VBUS_IPSOUT,
			       AXP_VBUS_IPSOUT_DRIVEBUS,
			       enable ? AXP_VBUS_IPSOUT_DRIVEBUS : 0);
}

static const struct dm_regulator_ops axp_drivevbus_ops = {
	.get_enable		= axp_drivevbus_get_enable,
	.set_enable		= axp_drivevbus_set_enable,
};

static int axp_drivevbus_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_plat = dev_get_uclass_plat(dev);
	int ret;

	uc_plat->type = REGULATOR_TYPE_FIXED;

	if (dev_read_bool(dev->parent, "x-powers,drive-vbus-en")) {
		ret = pmic_clrsetbits(dev->parent, AXP_MISC_CTRL,
				      AXP_MISC_CTRL_N_VBUSEN_FUNC, 0);
		if (ret)
			return ret;
	}

	return 0;
}

U_BOOT_DRIVER(axp_drivevbus) = {
	.name		= "axp_drivevbus",
	.id		= UCLASS_REGULATOR,
	.probe		= axp_drivevbus_probe,
	.ops		= &axp_drivevbus_ops,
};
