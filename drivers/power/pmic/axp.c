// SPDX-License-Identifier: GPL-2.0+

#include <axp_pmic.h>
#include <dm.h>
#include <dm/lists.h>
#include <i2c.h>
#include <power/pmic.h>
#include <sysreset.h>

#if CONFIG_IS_ENABLED(SYSRESET)
static int axp_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int ret;

	if (type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	ret = pmic_clrsetbits(dev->parent, AXP152_SHUTDOWN, 0, AXP152_POWEROFF);
	if (ret < 0)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops axp_sysreset_ops = {
	.request	= axp_sysreset_request,
};

U_BOOT_DRIVER(axp_sysreset) = {
	.name		= "axp_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &axp_sysreset_ops,
};
#endif

static int axp_pmic_reg_count(struct udevice *dev)
{
	/* TODO: Get the specific value from driver data. */
	return 0x100;
}

static struct dm_pmic_ops axp_pmic_ops = {
	.reg_count	= axp_pmic_reg_count,
	.read		= dm_i2c_read,
	.write		= dm_i2c_write,
};

static int axp_pmic_bind(struct udevice *dev)
{
	int ret;

	ret = dm_scan_fdt_dev(dev);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(SYSRESET)) {
		ret = device_bind_driver_to_node(dev, "axp_sysreset", "axp_sysreset",
						 dev_ofnode(dev), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id axp_pmic_ids[] = {
	{ .compatible = "x-powers,axp152" },
	{ .compatible = "x-powers,axp202" },
	{ .compatible = "x-powers,axp209" },
	{ .compatible = "x-powers,axp221" },
	{ .compatible = "x-powers,axp223" },
	{ .compatible = "x-powers,axp803" },
	{ .compatible = "x-powers,axp806" },
	{ .compatible = "x-powers,axp809" },
	{ .compatible = "x-powers,axp813" },
	{ }
};

U_BOOT_DRIVER(axp_pmic) = {
	.name		= "axp_pmic",
	.id		= UCLASS_PMIC,
	.of_match	= axp_pmic_ids,
	.bind		= axp_pmic_bind,
	.ops		= &axp_pmic_ops,
};
