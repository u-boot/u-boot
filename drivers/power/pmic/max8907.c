// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <power/pmic.h>

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
	.ops = &max8907_ops,
};
