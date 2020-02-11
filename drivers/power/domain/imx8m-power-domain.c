// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 NXP
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <asm/io.h>
#include <asm/arch/power-domain.h>
#include <asm/mach-imx/sys_proto.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <imx_sip.h>

DECLARE_GLOBAL_DATA_PTR;

static int imx8m_power_domain_request(struct power_domain *power_domain)
{
	return 0;
}

static int imx8m_power_domain_free(struct power_domain *power_domain)
{
	return 0;
}

static int imx8m_power_domain_on(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8m_power_domain_platdata *pdata;
	pdata = dev_get_platdata(dev);

	if (pdata->resource_id < 0)
		return -EINVAL;

	if (pdata->has_pd)
		power_domain_on(&pdata->pd);

	call_imx_sip(IMX_SIP_GPC, IMX_SIP_GPC_PM_DOMAIN,
		     pdata->resource_id, 1, 0);

	return 0;
}

static int imx8m_power_domain_off(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8m_power_domain_platdata *pdata;
	pdata = dev_get_platdata(dev);

	if (pdata->resource_id < 0)
		return -EINVAL;

	call_imx_sip(IMX_SIP_GPC, IMX_SIP_GPC_PM_DOMAIN,
		     pdata->resource_id, 0, 0);

	if (pdata->has_pd)
		power_domain_off(&pdata->pd);

	return 0;
}

static int imx8m_power_domain_of_xlate(struct power_domain *power_domain,
				      struct ofnode_phandle_args *args)
{
	return 0;
}

static int imx8m_power_domain_bind(struct udevice *dev)
{
	int offset;
	const char *name;
	int ret = 0;

	offset = dev_of_offset(dev);
	for (offset = fdt_first_subnode(gd->fdt_blob, offset); offset > 0;
	     offset = fdt_next_subnode(gd->fdt_blob, offset)) {
		/* Bind the subnode to this driver */
		name = fdt_get_name(gd->fdt_blob, offset, NULL);

		ret = device_bind_with_driver_data(dev, dev->driver, name,
						   dev->driver_data,
						   offset_to_ofnode(offset),
						   NULL);

		if (ret == -ENODEV)
			printf("Driver '%s' refuses to bind\n",
			       dev->driver->name);

		if (ret)
			printf("Error binding driver '%s': %d\n",
			       dev->driver->name, ret);
	}

	return 0;
}

static int imx8m_power_domain_probe(struct udevice *dev)
{
	return 0;
}

static int imx8m_power_domain_ofdata_to_platdata(struct udevice *dev)
{
	struct imx8m_power_domain_platdata *pdata = dev_get_platdata(dev);

	pdata->resource_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					    "reg", -1);

	if (!power_domain_get(dev, &pdata->pd))
		pdata->has_pd = 1;

	return 0;
}

static const struct udevice_id imx8m_power_domain_ids[] = {
	{ .compatible = "fsl,imx8mq-gpc" },
	{ }
};

struct power_domain_ops imx8m_power_domain_ops = {
	.request = imx8m_power_domain_request,
	.rfree = imx8m_power_domain_free,
	.on = imx8m_power_domain_on,
	.off = imx8m_power_domain_off,
	.of_xlate = imx8m_power_domain_of_xlate,
};

U_BOOT_DRIVER(imx8m_power_domain) = {
	.name = "imx8m_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = imx8m_power_domain_ids,
	.bind = imx8m_power_domain_bind,
	.probe = imx8m_power_domain_probe,
	.ofdata_to_platdata = imx8m_power_domain_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct imx8m_power_domain_platdata),
	.ops = &imx8m_power_domain_ops,
};
