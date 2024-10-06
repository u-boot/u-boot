// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <power/pmic.h>
#include <power/max8907.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = MAX8907_LDO_DRIVER },
	{ .prefix = "sd", .driver = MAX8907_SD_DRIVER },
	{ },
};

static int max8907_write(struct udevice *dev, uint reg, const uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret) {
		log_debug("%s: write error to device: %p register: %#x!\n",
			  __func__, dev, reg);
		return ret;
	}

	return 0;
}

static int max8907_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		log_debug("%s: read error from device: %p register: %#x!\n",
			  __func__, dev, reg);
		return ret;
	}

	return 0;
}

static int max8907_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children, ret;

	if (IS_ENABLED(CONFIG_SYSRESET_MAX8907) &&
	    dev_read_bool(dev, "maxim,system-power-controller")) {
		ret = device_bind_driver_to_node(dev, MAX8907_RST_DRIVER,
						 "sysreset", dev_ofnode(dev),
						 NULL);
		if (ret) {
			log_debug("%s: cannot bind SYSRESET (ret = %d)\n",
				  __func__, ret);
			return ret;
		}
	}

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		log_err("%s regulators subnode not found!\n", dev->name);
		return -ENXIO;
	}

	log_debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		log_err("%s - no child found\n", dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops max8907_ops = {
	.read = max8907_read,
	.write = max8907_write,
};

static const struct udevice_id max8907_ids[] = {
	{ .compatible = "maxim,max8907" },
	{ }
};

U_BOOT_DRIVER(pmic_max8907) = {
	.name = "max8907_pmic",
	.id = UCLASS_PMIC,
	.of_match = max8907_ids,
	.bind = max8907_bind,
	.ops = &max8907_ops,
};
