/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * Shared UCLASS_REGULATOR ops for PMBus 1.x voltage regulator chips.
 *
 * Per chip drivers under drivers/power/regulator/<chip>.c bind a
 * vendor,chip compatible from DT and call pmbus_regulator_probe_common()
 * in their .probe. They install pmbus_regulator_ops as the .ops vector;
 * the helper handles VOUT_MODE / READ_VOUT / VOUT_COMMAND / OPERATION
 * via the tree level <pmbus.h> framework.
 *
 * Per chip drivers retain control of identify hooks (VOUT_MODE based
 * format selection), chip specific quirks (vendor registers, ADDR pin
 * auto promotion), and DT property handling (e.g. MPS
 * mps,vout-fb-divider-ratio-permille).
 */

#ifndef _DRIVERS_POWER_REGULATOR_PMBUS_HELPER_H_
#define _DRIVERS_POWER_REGULATOR_PMBUS_HELPER_H_

#include <linux/types.h>
#include <pmbus.h>

struct udevice;
struct dm_regulator_ops;

/*
 * Per chip private state. The first field of every per chip driver's
 * priv_auto must be (or contain at offset 0) a struct
 * pmbus_regulator_priv so the shared ops vector can recover it via
 * dev_get_priv(dev).
 *
 *   i2c_dev   chip handle obtained from dev->parent at probe time
 *             (the parent must be a UCLASS_I2C bus).
 *   page      PMBUS_PAGE selector for multi rail chips. Single rail
 *             chips set page = 0; the helper writes PMBUS_PAGE only
 *             when page > 0 to avoid wasted bus traffic on single
 *             rail parts.
 *   info      pointer to the chip's pmbus_driver_info; consumed by
 *             pmbus_reg2data() / pmbus_data2reg_linear16() to pick
 *             the right format[] / m / b / R coefficients.
 */
struct pmbus_regulator_priv {
	struct udevice *i2c_dev;
	int page;
	const struct pmbus_driver_info *info;
};

extern const struct dm_regulator_ops pmbus_regulator_ops;

/*
 * Per chip probe glue. Reads `reg` from DT, gets the I2C chip handle
 * from dev->parent, populates priv->i2c_dev / page / info, and writes
 * PMBUS_PAGE if page > 0. Per chip drivers call this in their .probe
 * before any chip specific identification.
 */
int pmbus_regulator_probe_common(struct udevice *dev,
				 const struct pmbus_driver_info *info,
				 int page);

/*
 * Optional helper for per chip drivers that honour an external
 * feedback divider DT property (e.g. MPS mps,vout-fb-divider-ratio-
 * permille). Writes the supplied ratio to PMBUS_VOUT_SCALE_LOOP at
 * probe time. fb_divider_permille == 0 leaves the chip default.
 */
int pmbus_regulator_apply_voltage_scale(struct udevice *dev,
					u32 fb_divider_permille);

/*
 * Read PMBUS_VOUT_MODE and set info->format[PSC_VOLTAGE_OUT] from its
 * mode selector bits[7:5] per PMBus 1.3 Part II sec 8.3:
 *   LINEAR  -> pmbus_fmt_linear
 *   VID     -> pmbus_fmt_vid
 *   DIRECT  -> pmbus_fmt_direct (default coefficients m=1, b=0, R=0)
 *   IEEE754 -> pmbus_fmt_ieee754
 *
 * The single place that knows the VOUT_MODE bit layout; both the
 * generic regulator and per chip drivers call it so they never
 * re-implement the switch. Returns the selected format so a chip
 * driver can post-adjust a quirk (e.g. MPS encodes VOUT in DIRECT
 * with m=64 R=1 even when VOUT_MODE reports VID). On a VOUT_MODE read
 * failure the format is left unchanged and the prior value is returned.
 */
enum pmbus_data_format
pmbus_regulator_identify_vout(struct udevice *i2c_dev,
			      struct pmbus_driver_info *info);

#endif /* _DRIVERS_POWER_REGULATOR_PMBUS_HELPER_H_ */
