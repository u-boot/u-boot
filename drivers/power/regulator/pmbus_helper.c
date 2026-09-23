// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * Shared UCLASS_REGULATOR operations over the PMBus 1.x framework.
 * See pmbus_helper.h for the API surface and doc/develop/pmbus.rst
 * for the porting guide.
 *
 * No code in this file may reference a specific chip family or
 * board. Chip specific quirks (vendor registers, VID coercion,
 * ADDR pin auto promotion, byte reversed MFR strings, etc.) belong
 * in the per chip driver under drivers/power/regulator/<chip>.c.
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <vsprintf.h>
#include <linux/types.h>
#include <power/regulator.h>

#include "pmbus_helper.h"

static int pmbus_regulator_select_page(struct pmbus_regulator_priv *priv)
{
	u8 p;

	if (priv->page <= 0)
		return 0;
	p = (u8)priv->page;
	return dm_i2c_write(priv->i2c_dev, PMBUS_PAGE, &p, 1);
}

static int pmbus_regulator_get_value(struct udevice *dev)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	u8 vout_mode = 0;
	u16 raw = 0;
	s64 uv;
	int ret;

	ret = pmbus_regulator_select_page(priv);
	if (ret)
		return ret;
	pmbus_read_byte(priv->i2c_dev, PMBUS_VOUT_MODE, &vout_mode);
	if (pmbus_read_word(priv->i2c_dev, PMBUS_READ_VOUT, &raw))
		return -EIO;

	if (priv->info)
		uv = pmbus_reg2data(priv->info, PSC_VOLTAGE_OUT, raw, vout_mode);
	else
		uv = pmbus_reg2data_linear16(raw, vout_mode);

	if (uv > INT_MAX)
		uv = INT_MAX;
	if (uv < INT_MIN)
		uv = INT_MIN;
	return (int)uv;
}

static int pmbus_regulator_set_value(struct udevice *dev, int uV)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	u8 vout_mode = 0;
	u8 buf[2];
	u16 raw;
	int ret;

	ret = pmbus_regulator_select_page(priv);
	if (ret)
		return ret;
	if (pmbus_read_byte(priv->i2c_dev, PMBUS_VOUT_MODE, &vout_mode))
		return -EIO;

	/*
	 * Dispatch on the chip's VOUT_MODE selector. LINEAR16 and DIRECT
	 * are wired today; VID and IEEE754 return -ENOSYS until their
	 * encoders land. For DIRECT, the m / b / R triple comes from the
	 * chip's pmbus_driver_info[PSC_VOLTAGE_OUT]; if the per chip
	 * driver did not populate them, the encoder cannot run.
	 */
	switch (vout_mode & PB_VOUT_MODE_MODE_MASK) {
	case PB_VOUT_MODE_LINEAR:
		raw = pmbus_data2reg_linear16((s64)uV, vout_mode);
		break;
	case PB_VOUT_MODE_DIRECT:
		if (!priv->info ||
		    priv->info->format[PSC_VOLTAGE_OUT] != pmbus_fmt_direct)
			return -ENODATA;
		raw = pmbus_data2reg_direct((s64)uV,
					    priv->info->m[PSC_VOLTAGE_OUT],
					    priv->info->b[PSC_VOLTAGE_OUT],
					    priv->info->R[PSC_VOLTAGE_OUT]);
		break;
	default:
		return -ENOSYS;
	}

	buf[0] = (u8)(raw & 0xff);
	buf[1] = (u8)((raw >> 8) & 0xff);
	return dm_i2c_write(priv->i2c_dev, PMBUS_VOUT_COMMAND, buf, 2);
}

static int pmbus_regulator_get_enable(struct udevice *dev)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	u8 op = 0;
	int ret;

	ret = pmbus_regulator_select_page(priv);
	if (ret)
		return ret;
	if (pmbus_read_byte(priv->i2c_dev, PMBUS_OPERATION, &op))
		return -EIO;
	return (op & PB_OPERATION_ON) ? 1 : 0;
}

static int pmbus_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	u8 op = 0;
	int ret;

	ret = pmbus_regulator_select_page(priv);
	if (ret)
		return ret;
	if (pmbus_read_byte(priv->i2c_dev, PMBUS_OPERATION, &op))
		return -EIO;

	if (enable)
		op |= PB_OPERATION_ON;
	else
		op &= (u8)~PB_OPERATION_ON;
	return dm_i2c_write(priv->i2c_dev, PMBUS_OPERATION, &op, 1);
}

const struct dm_regulator_ops pmbus_regulator_ops = {
	.get_value  = pmbus_regulator_get_value,
	.set_value  = pmbus_regulator_set_value,
	.get_enable = pmbus_regulator_get_enable,
	.set_enable = pmbus_regulator_set_enable,
};

int pmbus_regulator_read_temp(struct udevice *reg_dev, int *temp_mc)
{
	struct pmbus_regulator_priv *priv;
	u16 raw = 0;
	s64 udeg;
	int ret;

	if (!reg_dev || !temp_mc)
		return -EINVAL;
	priv = dev_get_priv(reg_dev);
	if (!priv || !priv->i2c_dev)
		return -ENODEV;

	ret = pmbus_regulator_select_page(priv);
	if (ret)
		return ret;
	if (pmbus_read_word(priv->i2c_dev, PMBUS_READ_TEMPERATURE_1, &raw))
		return -EIO;

	/*
	 * vout_mode is meaningless for the temperature class. With a
	 * chip info record the dispatcher honours its per-class format
	 * (DIRECT m/b/R for MPS, LINEAR11 for spec-compliant parts);
	 * without one, fall back to the PMBus 1.x standard LINEAR11.
	 */
	if (priv->info)
		udeg = pmbus_reg2data(priv->info, PSC_TEMPERATURE, raw, 0);
	else
		udeg = pmbus_reg2data_linear11(raw);

	*temp_mc = (int)(udeg / 1000);
	return 0;
}

enum pmbus_data_format
pmbus_regulator_identify_vout(struct udevice *i2c_dev,
			      struct pmbus_driver_info *info)
{
	u8 vout_mode = 0;

	if (pmbus_read_byte(i2c_dev, PMBUS_VOUT_MODE, &vout_mode))
		return info->format[PSC_VOLTAGE_OUT];

	switch (vout_mode & PB_VOUT_MODE_MODE_MASK) {
	case PB_VOUT_MODE_LINEAR:
		info->format[PSC_VOLTAGE_OUT] = pmbus_fmt_linear;
		break;
	case PB_VOUT_MODE_VID:
		info->format[PSC_VOLTAGE_OUT] = pmbus_fmt_vid;
		break;
	case PB_VOUT_MODE_DIRECT:
		info->format[PSC_VOLTAGE_OUT] = pmbus_fmt_direct;
		info->m[PSC_VOLTAGE_OUT] = 1;
		info->b[PSC_VOLTAGE_OUT] = 0;
		info->R[PSC_VOLTAGE_OUT] = 0;
		break;
	case PB_VOUT_MODE_IEEE754:
		info->format[PSC_VOLTAGE_OUT] = pmbus_fmt_ieee754;
		break;
	default:
		break;
	}
	return info->format[PSC_VOLTAGE_OUT];
}

const struct pmbus_driver_info *pmbus_regulator_info_by_addr(int bus_seq,
							     u8 addr)
{
	struct uclass *uc;
	struct udevice *r;

	if (uclass_get(UCLASS_REGULATOR, &uc))
		return NULL;

	uclass_foreach_dev(r, uc) {
		struct udevice *parent = dev_get_parent(r);
		struct pmbus_regulator_priv *priv;
		int ra;

		if (!parent || device_get_uclass_id(parent) != UCLASS_I2C)
			continue;
		if (dev_seq(parent) != bus_seq)
			continue;
		ra = dev_read_addr(r);
		if (ra < 0 || (u8)ra != addr)
			continue;

		/*
		 * Address matches. Only chips driven through this helper
		 * carry a pmbus_regulator_priv at the head of their priv;
		 * identify them by their shared ops vector so we never
		 * misread a foreign regulator's private layout.
		 */
		if (!r->driver || r->driver->ops != &pmbus_regulator_ops)
			return NULL;
		if (device_probe(r))
			return NULL;
		priv = dev_get_priv(r);
		return priv ? priv->info : NULL;
	}
	return NULL;
}

/*
 * Spawn the generic UCLASS_THERMAL companion (drivers/thermal/
 * pmbus_thermal.c) as a child of this regulator so READ_TEMPERATURE_1
 * is reachable through the standard `temperature list` / `temperature
 * get` interface. Named "<regulator-name>-temp" so several PMBus rails
 * on one board produce distinct, descriptive device names. Failure is
 * non-fatal: the chip still works as a UCLASS_REGULATOR.
 */
static void pmbus_regulator_bind_thermal(struct udevice *dev)
{
	struct udevice *therm;
	const char *rname;
	char name[48];

	if (!IS_ENABLED(CONFIG_PMBUS_THERMAL))
		return;
	if (device_bind_driver(dev, "pmbus_thermal", "pmbus-temp", &therm))
		return;
	rname = dev_read_string(dev, "regulator-name");
	snprintf(name, sizeof(name), "%s-temp", rname ? rname : dev->name);
	device_set_name(therm, name);
}

int pmbus_regulator_probe_common(struct udevice *dev,
				 const struct pmbus_driver_info *info,
				 int page)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	int chip_addr;
	int ret;

	chip_addr = dev_read_addr(dev);
	if (chip_addr < 0)
		return -EINVAL;

	ret = i2c_get_chip(dev_get_parent(dev), (u32)chip_addr, 1, &priv->i2c_dev);
	if (ret)
		return ret;

	priv->info = info;
	priv->page = page;

	if (page > 0) {
		u8 p = (u8)page;

		ret = dm_i2c_write(priv->i2c_dev, PMBUS_PAGE, &p, 1);
		if (ret)
			return ret;
	}

	pmbus_regulator_bind_thermal(dev);
	return 0;
}

int pmbus_regulator_apply_voltage_scale(struct udevice *dev,
					u32 fb_divider_permille)
{
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	u8 buf[2];

	if (fb_divider_permille == 0)
		return 0;
	buf[0] = (u8)(fb_divider_permille & 0xff);
	buf[1] = (u8)((fb_divider_permille >> 8) & 0xff);
	return dm_i2c_write(priv->i2c_dev, PMBUS_VOUT_SCALE_LOOP, buf, 2);
}
