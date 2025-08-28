// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <i2c.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/pca9450.h>
#include <sysreset.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	/* buck */
	{ .prefix = "b", .driver = PCA9450_REGULATOR_DRIVER},
	{ .prefix = "B", .driver = PCA9450_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = PCA9450_REGULATOR_DRIVER},
	{ .prefix = "L", .driver = PCA9450_REGULATOR_DRIVER},
	{ },
};

static int pca9450_reg_count(struct udevice *dev)
{
	return PCA9450_REG_NUM;
}

static int pca9450_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_read(struct udevice *dev, uint reg, uint8_t *buff,
			int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node,
				      pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static int pca9450_probe(struct udevice *dev)
{
	unsigned int reset_ctrl;
	int ret;

	if (CONFIG_IS_ENABLED(SYSRESET)) {
		ret = device_bind_driver_to_node(dev, "pca9450_sysreset",
						 "pca9450_sysreset",
						 dev_ofnode(dev), NULL);
		if (ret)
			return ret;
	}

	if (ofnode_read_bool(dev_ofnode(dev), "nxp,wdog_b-warm-reset"))
		reset_ctrl = PCA9450_PMIC_RESET_WDOG_B_CFG_WARM;
	else
		reset_ctrl = PCA9450_PMIC_RESET_WDOG_B_CFG_COLD_LDO12;

	return pmic_clrsetbits(dev, PCA9450_RESET_CTRL,
			       PCA9450_PMIC_RESET_WDOG_B_CFG_MASK, reset_ctrl);
}

static struct dm_pmic_ops pca9450_ops = {
	.reg_count = pca9450_reg_count,
	.read = pca9450_read,
	.write = pca9450_write,
};

static const struct udevice_id pca9450_ids[] = {
	{ .compatible = "nxp,pca9450a", .data = NXP_CHIP_TYPE_PCA9450A, },
	{ .compatible = "nxp,pca9450b", .data = NXP_CHIP_TYPE_PCA9450BC, },
	{ .compatible = "nxp,pca9450c", .data = NXP_CHIP_TYPE_PCA9450BC, },
	{ .compatible = "nxp,pca9451a", .data = NXP_CHIP_TYPE_PCA9451A, },
	{ .compatible = "nxp,pca9452",  .data = NXP_CHIP_TYPE_PCA9452, },
	{ }
};

U_BOOT_DRIVER(pmic_pca9450) = {
	.name = "pca9450 pmic",
	.id = UCLASS_PMIC,
	.of_match = pca9450_ids,
	.bind = pca9450_bind,
	.probe = pca9450_probe,
	.ops = &pca9450_ops,
};

#ifdef CONFIG_SYSRESET
static int pca9450_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	u8 cmd = PCA9450_SW_RST_COLD_RST;

	if (type != SYSRESET_COLD)
		return -EPROTONOSUPPORT;

	if (pmic_write(dev->parent, PCA9450_SW_RST, &cmd, 1)) {
		dev_err(dev, "reset command failed\n");
	} else {
		/* tRESTART is 250ms, delay 300ms just to be sure */
		mdelay(300);
		/* Should not get here, warn if we do */
		dev_warn(dev, "didn't respond to reset command\n");
	}

	return -EINPROGRESS;
}

int pca9450_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	const char *reason;
	int ret;
	u8 reg;

	ret = pmic_read(dev->parent, PCA9450_PWRON_STAT, &reg, 1);
	if (ret)
		return ret;

	switch (reg) {
	case PCA9450_PWRON_STAT_PWRON_MASK:
		reason = "PWRON";
		break;
	case PCA9450_PWRON_STAT_WDOG_MASK:
		reason = "WDOGB";
		break;
	case PCA9450_PWRON_STAT_SW_RST_MASK:
		reason = "SW_RST";
		break;
	case PCA9450_PWRON_STAT_PMIC_RST_MASK:
		reason = "PMIC_RST";
		break;
	default:
		reason = "UNKNOWN";
		break;
	}

	ret = snprintf(buf, size, "Reset Status: %s\n", reason);
	if (ret < 0) {
		dev_err(dev, "Write reset status error (err = %d)\n", ret);
		return -EIO;
	}

	return 0;
}

static struct sysreset_ops pca9450_sysreset_ops = {
	.request	= pca9450_sysreset_request,
	.get_status	= pca9450_sysreset_get_status,
};

U_BOOT_DRIVER(pca9450_sysreset) = {
	.name		= "pca9450_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &pca9450_sysreset_ops,
};
#endif /* CONFIG_SYSRESET */
