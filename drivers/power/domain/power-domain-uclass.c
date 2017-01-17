/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <power-domain.h>
#include <power-domain-uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static inline struct power_domain_ops *power_domain_dev_ops(struct udevice *dev)
{
	return (struct power_domain_ops *)dev->driver->ops;
}

static int power_domain_of_xlate_default(struct power_domain *power_domain,
				       struct fdtdec_phandle_args *args)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	power_domain->id = args->args[0];

	return 0;
}

int power_domain_get(struct udevice *dev, struct power_domain *power_domain)
{
	struct fdtdec_phandle_args args;
	int ret;
	struct udevice *dev_power_domain;
	struct power_domain_ops *ops;

	debug("%s(dev=%p, power_domain=%p)\n", __func__, dev, power_domain);

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev_of_offset(dev),
					     "power-domains",
					     "#power-domain-cells", 0, 0,
					     &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: %d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_POWER_DOMAIN, args.node,
					     &dev_power_domain);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: %d\n",
		      __func__, ret);
		return ret;
	}
	ops = power_domain_dev_ops(dev_power_domain);

	power_domain->dev = dev_power_domain;
	if (ops->of_xlate)
		ret = ops->of_xlate(power_domain, &args);
	else
		ret = power_domain_of_xlate_default(power_domain, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	ret = ops->request(power_domain);
	if (ret) {
		debug("ops->request() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

int power_domain_free(struct power_domain *power_domain)
{
	struct power_domain_ops *ops = power_domain_dev_ops(power_domain->dev);

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return ops->free(power_domain);
}

int power_domain_on(struct power_domain *power_domain)
{
	struct power_domain_ops *ops = power_domain_dev_ops(power_domain->dev);

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return ops->on(power_domain);
}

int power_domain_off(struct power_domain *power_domain)
{
	struct power_domain_ops *ops = power_domain_dev_ops(power_domain->dev);

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return ops->off(power_domain);
}

UCLASS_DRIVER(power_domain) = {
	.id		= UCLASS_POWER_DOMAIN,
	.name		= "power_domain",
};
