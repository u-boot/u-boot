// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Jingbao Qiu <qiujingbao.dlmu@gmail.com>
 * Copyright (C) 2026 Zhuxu Ran <zhuxu_ran@zohomail.com>
 *
 * Based on the Linux driver by Jingbao Qiu <qiujingbao.dlmu@gmail.com>.
 */

#include <clk.h>
#include <dm.h>
#include <dm/lists.h>
#include <regmap.h>
#include <rtc.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/err.h>

#define SEC_PULSE_GEN		0x1004
#define SET_SEC_CNTR_VAL	0x1010
#define SET_SEC_CNTR_TRIG	0x1014
#define SEC_CNTR_VAL		0x1018
#define MACRO_RG_SET_T		0x1498

#define SEL_SEC_PULSE		BIT(31)

struct cv1800b_rtc_priv {
	struct regmap *regmap;
};

static int cv1800b_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct cv1800b_rtc_priv *priv = dev_get_priv(dev);
	u32 value = 0;

	regmap_read(priv->regmap, SEC_PULSE_GEN, &value);
	if (value & SEL_SEC_PULSE)
		return -EINVAL;

	regmap_read(priv->regmap, SEC_CNTR_VAL, &value);

	rtc_to_tm(value, time);

	return 0;
}

static int cv1800b_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	struct cv1800b_rtc_priv *priv = dev_get_priv(dev);
	time64_t seconds;

	if (time->tm_year < 1970)
		return -EINVAL;

	seconds = rtc_mktime(time);
	if (seconds < 0 || seconds > U32_MAX)
		return -ERANGE;

	regmap_write(priv->regmap, SET_SEC_CNTR_VAL, seconds);
	regmap_write(priv->regmap, SET_SEC_CNTR_TRIG, 1);
	regmap_write(priv->regmap, MACRO_RG_SET_T, seconds);
	regmap_update_bits(priv->regmap, SEC_PULSE_GEN, SEL_SEC_PULSE, 0);

	return 0;
}

static int cv1800b_rtc_bind(struct udevice *dev)
{
	if (!CONFIG_IS_ENABLED(SYSRESET_CV1800B))
		return 0;

	return device_bind_driver_to_node(dev, "cv1800b_sysreset",
					  "sysreset", dev_ofnode(dev), NULL);
}

static int cv1800b_rtc_probe(struct udevice *dev)
{
	struct cv1800b_rtc_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->regmap = devm_regmap_init(dev, NULL, NULL, NULL);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	ret = clk_get_by_name(dev, "rtc", &clk);
	if (ret)
		return ret;

	return clk_enable(&clk);
}

static const struct rtc_ops cv1800b_rtc_ops = {
	.get = cv1800b_rtc_get,
	.set = cv1800b_rtc_set,
};

static const struct udevice_id cv1800b_rtc_ids[] = {
	{ .compatible = "sophgo,cv1800b-rtc" },
	{ }
};

U_BOOT_DRIVER(cv1800b_rtc) = {
	.name = "cv1800b_rtc",
	.id = UCLASS_RTC,
	.of_match = cv1800b_rtc_ids,
	.bind = cv1800b_rtc_bind,
	.probe = cv1800b_rtc_probe,
	.ops = &cv1800b_rtc_ops,
	.priv_auto = sizeof(struct cv1800b_rtc_priv),
};
