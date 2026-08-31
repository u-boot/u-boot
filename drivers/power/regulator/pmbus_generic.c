// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * Generic PMBus 1.x compatible voltage regulator driver.
 *
 * Catch all driver bound to compatible = "pmbus" for chips that have
 * no per chip driver under drivers/power/regulator/. The probe path
 * detects the VOUT numeric format from VOUT_MODE bits[7:5]:
 *
 *   - 0  LINEAR16 with the exponent supplied via VOUT_MODE bits[4:0]
 *   - 1  VID; mapped to pmbus_fmt_vid (decoder returns 0 today; per
 *        chip driver still required to plug a VID table)
 *   - 2  DIRECT; default coefficients m=1, b=0, R=0 (per chip
 *        coefficients arrive via PMBUS_QUERY / PMBUS_COEFFICIENTS,
 *        not yet consumed by U-Boot; values may need a per chip
 *        driver if telemetry numbers are wrong)
 *   - 3  IEEE754; mapped to pmbus_fmt_ieee754 (decoder returns 0
 *        today; per chip driver required)
 *
 * Other sensor classes (VIN, IIN, IOUT, TEMPERATURE) default to
 * LINEAR which is the spec baseline for compliant chips. If an
 * operator sees wrong telemetry numbers on this driver, the answer
 * is to write a per chip driver with the correct format[] / m / b / R.
 *
 * Adapted in spirit from linux/drivers/hwmon/pmbus/pmbus.c (the
 * kernel's generic probe driver). The U-Boot version drops the
 * page count auto detection (most generic compliant parts are
 * single rail; multi rail chips are quirky enough to need a per
 * chip driver) and the kernel hwmon publication layers.
 */

#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <power/regulator.h>

#include "pmbus_helper.h"

struct pmbus_generic_priv {
	struct pmbus_regulator_priv base;	/* must be first */
	struct pmbus_driver_info info;
};

static int pmbus_generic_probe(struct udevice *dev)
{
	struct pmbus_generic_priv *gpriv = dev_get_priv(dev);
	struct pmbus_driver_info *info = &gpriv->info;
	enum pmbus_sensor_classes c;
	int ret;

	info->pages = 1;
	for (c = 0; c < PSC_NUM_CLASSES; c++) {
		info->format[c] = pmbus_fmt_linear;
		info->m[c] = 0;
		info->b[c] = 0;
		info->R[c] = 0;
	}

	ret = pmbus_regulator_probe_common(dev, info, 0);
	if (ret)
		return ret;

	/*
	 * Avoid reading non supported pages to avoid device's sticky
	 * status.
	 */
	info->pages = dev_read_u32_default(dev, "pmbus,num-pages", 1);
	if (info->pages < 1)
		info->pages = 1;

	pmbus_regulator_identify_vout(gpriv->base.i2c_dev, info);

	return 0;
}

static const struct udevice_id pmbus_generic_ids[] = {
	{ .compatible = "pmbus" },
	{ }
};

U_BOOT_DRIVER(pmbus_generic_regulator) = {
	.name      = "pmbus_generic_regulator",
	.id        = UCLASS_REGULATOR,
	.of_match  = pmbus_generic_ids,
	.probe     = pmbus_generic_probe,
	.ops       = &pmbus_regulator_ops,
	.priv_auto = sizeof(struct pmbus_generic_priv),
};
