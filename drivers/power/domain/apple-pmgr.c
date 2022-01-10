// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/err.h>
#include <linux/bitfield.h>
#include <power-domain-uclass.h>
#include <regmap.h>
#include <syscon.h>

#define APPLE_PMGR_PS_TARGET	GENMASK(3, 0)
#define APPLE_PMGR_PS_ACTUAL	GENMASK(7, 4)

#define APPLE_PMGR_PS_ACTIVE	0xf
#define APPLE_PMGR_PS_PWRGATE	0x0

#define APPLE_PMGR_PS_SET_TIMEOUT_US	100

struct apple_pmgr_priv {
	struct regmap *regmap;
	u32 offset;		/* offset within regmap for this domain */
};

static int apple_pmgr_request(struct power_domain *power_domain)
{
	return 0;
}

static int apple_pmgr_rfree(struct power_domain *power_domain)
{
	return 0;
}

static int apple_pmgr_ps_set(struct power_domain *power_domain, u32 pstate)
{
	struct apple_pmgr_priv *priv = dev_get_priv(power_domain->dev);
	uint reg;

	regmap_update_bits(priv->regmap, priv->offset, APPLE_PMGR_PS_TARGET,
			   FIELD_PREP(APPLE_PMGR_PS_TARGET, pstate));

	return regmap_read_poll_timeout(
		priv->regmap, priv->offset, reg,
		(FIELD_GET(APPLE_PMGR_PS_ACTUAL, reg) == pstate), 1,
		APPLE_PMGR_PS_SET_TIMEOUT_US);
}

static int apple_pmgr_on(struct power_domain *power_domain)
{
	return apple_pmgr_ps_set(power_domain, APPLE_PMGR_PS_ACTIVE);
}

static int apple_pmgr_off(struct power_domain *power_domain)
{
	return 0;
}

static int apple_pmgr_of_xlate(struct power_domain *power_domain,
			       struct ofnode_phandle_args *args)
{
	if (args->args_count != 0) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id apple_pmgr_ids[] = {
	{ .compatible = "apple,pmgr-pwrstate" },
	{ /* sentinel */ }
};

static int apple_pmgr_probe(struct udevice *dev)
{
	struct apple_pmgr_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dev_power_domain_on(dev);
	if (ret)
		return ret;

	priv->regmap = syscon_get_regmap(dev->parent);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	ret = dev_read_u32(dev, "reg", &priv->offset);
	if (ret < 0)
		return ret;

	return 0;
}

struct power_domain_ops apple_pmgr_ops = {
	.request = apple_pmgr_request,
	.rfree = apple_pmgr_rfree,
	.on = apple_pmgr_on,
	.off = apple_pmgr_off,
	.of_xlate = apple_pmgr_of_xlate,
};

U_BOOT_DRIVER(apple_pmgr) = {
	.name = "apple_pmgr",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = apple_pmgr_ids,
	.ops = &apple_pmgr_ops,
	.probe = apple_pmgr_probe,
	.priv_auto = sizeof(struct apple_pmgr_priv),
};
