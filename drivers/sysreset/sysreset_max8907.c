// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <errno.h>
#include <sysreset.h>
#include <power/pmic.h>
#include <power/max8907.h>

static int max8907_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	switch (type) {
	case SYSRESET_POWER:
	case SYSRESET_POWER_OFF:
		/* MAX8907: PWR_OFF > RESET_CNFG */
		pmic_clrsetbits(dev->parent, MAX8907_REG_RESET_CNFG,
				MASK_POWER_OFF, MASK_POWER_OFF);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops max8907_sysreset = {
	.request = max8907_sysreset_request,
};

U_BOOT_DRIVER(sysreset_max8907) = {
	.id	= UCLASS_SYSRESET,
	.name	= MAX8907_RST_DRIVER,
	.ops	= &max8907_sysreset,
};
