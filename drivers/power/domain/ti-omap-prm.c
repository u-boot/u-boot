// SPDX-License-Identifier: GPL-2.0+
/*
 * TI OMAP PRM (Power and Reset Manager) power domain driver
 *
 * Stub driver to provide power domain support.
 */

#include <dm.h>
#include <power-domain-uclass.h>

static int ti_omap_prm_xlate(struct power_domain *power_domain,
			     struct ofnode_phandle_args *args)
{
	if (args->args_count != 0)
		return -EINVAL;

	return 0;
}

static const struct udevice_id ti_omap_prm_ids[] = {
	{ .compatible = "ti,am3-prm-inst" },
	{ .compatible = "ti,am4-prm-inst" },
	{ .compatible = "ti,omap-prm-inst" },
	{ }
};

static struct power_domain_ops ti_omap_prm_ops = {
	.of_xlate = ti_omap_prm_xlate,
};

U_BOOT_DRIVER(ti_omap_prm) = {
	.name = "ti-omap-prm",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = ti_omap_prm_ids,
	.ops = &ti_omap_prm_ops,
};
