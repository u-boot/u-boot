// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021, Xilinx. Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <misc.h>
#include <power-domain-uclass.h>
#include <linux/bitops.h>

#include <zynqmp_firmware.h>

#define NODE_ID_LOCATION	5

static unsigned int xpm_configobject[] = {
	/* HEADER */
	2,	/* Number of remaining words in the header */
	1,	/* Number of sections included in config object */
	PM_CONFIG_OBJECT_TYPE_OVERLAY,	/* Type of Config object as overlay */
	/* SLAVE SECTION */

	PM_CONFIG_SLAVE_SECTION_ID,	/* Section ID */
	1,				/* Number of slaves */

	0, /* Node ID which will be changed below */
	PM_SLAVE_FLAG_IS_SHAREABLE,
	PM_CONFIG_IPI_PSU_CORTEXA53_0_MASK |
	PM_CONFIG_IPI_PSU_CORTEXR5_0_MASK |
	PM_CONFIG_IPI_PSU_CORTEXR5_1_MASK, /* IPI Mask */
};

static int zynqmp_pm_request_node(const u32 node, const u32 capabilities,
				  const u32 qos, const enum zynqmp_pm_request_ack ack)
{
	return xilinx_pm_request(PM_REQUEST_NODE, node, capabilities,
				   qos, ack, NULL);
}

static int zynqmp_power_domain_request(struct power_domain *power_domain)
{
	/* Record power domain id */
	xpm_configobject[NODE_ID_LOCATION] = power_domain->id;

	zynqmp_pmufw_load_config_object(xpm_configobject, sizeof(xpm_configobject));

	return 0;
}

static int zynqmp_power_domain_free(struct power_domain *power_domain)
{
	/* nop now */
	return 0;
}

static int zynqmp_power_domain_on(struct power_domain *power_domain)
{
	return zynqmp_pm_request_node(power_domain->id,
				      ZYNQMP_PM_CAPABILITY_ACCESS,
				      ZYNQMP_PM_MAX_QOS,
				      ZYNQMP_PM_REQUEST_ACK_BLOCKING);
}

static int zynqmp_power_domain_off(struct power_domain *power_domain)
{
	/* nop now */
	return 0;
}

struct power_domain_ops zynqmp_power_domain_ops = {
	.request = zynqmp_power_domain_request,
	.rfree = zynqmp_power_domain_free,
	.on = zynqmp_power_domain_on,
	.off = zynqmp_power_domain_off,
};

static int zynqmp_power_domain_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(zynqmp_power_domain) = {
	.name = "zynqmp_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.probe = zynqmp_power_domain_probe,
	.ops = &zynqmp_power_domain_ops,
};
