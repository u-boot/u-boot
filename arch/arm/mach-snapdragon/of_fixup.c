// SPDX-License-Identifier: GPL-2.0+
/*
 * OF_LIVE devicetree fixup.
 *
 * This file implements runtime fixups for Qualcomm DT to improve
 * compatibility with U-Boot. This includes adjusting the USB nodes
 * to only use USB high-speed, as well as remapping volume buttons
 * to behave as up/down for navigating U-Boot.
 *
 * We use OF_LIVE for this rather than early FDT fixup for a couple
 * of reasons: it has a much nicer API, is most likely more efficient,
 * and our changes are only applied to U-Boot. This allows us to use a
 * DT designed for Linux, run U-Boot with a modified version, and then
 * boot Linux with the original FDT.
 *
 * Copyright (c) 2024 Linaro Ltd.
 *   Author: Caleb Connolly <caleb.connolly@linaro.org>
 */

#define pr_fmt(fmt) "of_fixup: " fmt

#include <dt-bindings/input/linux-event-codes.h>
#include <dm/of_access.h>
#include <dm/of.h>
#include <fdt_support.h>
#include <linux/errno.h>
#include <stdlib.h>
#include <time.h>

/* U-Boot only supports USB high-speed mode on Qualcomm platforms with DWC3
 * USB controllers. Rather than requiring source level DT changes, we fix up
 * DT here. This improves compatibility with upstream DT and simplifies the
 * porting process for new devices.
 */
static int fixup_qcom_dwc3(struct device_node *glue_np)
{
	struct device_node *dwc3;
	int ret, len, hsphy_idx = 1;
	const __be32 *phandles;
	const char *second_phy_name;

	debug("Fixing up %s\n", glue_np->name);

	/* Tell the glue driver to configure the wrapper for high-speed only operation */
	ret = of_write_prop(glue_np, "qcom,select-utmi-as-pipe-clk", 0, NULL);
	if (ret) {
		log_err("Failed to add property 'qcom,select-utmi-as-pipe-clk': %d\n", ret);
		return ret;
	}

	/* Find the DWC3 node itself */
	dwc3 = of_find_compatible_node(glue_np, NULL, "snps,dwc3");
	if (!dwc3) {
		log_err("Failed to find dwc3 node\n");
		return -ENOENT;
	}

	phandles = of_get_property(dwc3, "phys", &len);
	len /= sizeof(*phandles);
	if (len == 1) {
		log_debug("Only one phy, not a superspeed controller\n");
		return 0;
	}

	/* Figure out if the superspeed phy is present and if so then which phy is it? */
	ret = of_property_read_string_index(dwc3, "phy-names", 1, &second_phy_name);
	if (ret == -ENODATA) {
		log_debug("Only one phy, not a super-speed controller\n");
		return 0;
	} else if (ret) {
		log_err("Failed to read second phy name: %d\n", ret);
		return ret;
	}

	if (!strncmp("usb3-phy", second_phy_name, strlen("usb3-phy"))) {
		log_debug("Second phy isn't superspeed (is '%s') assuming first phy is SS\n",
			  second_phy_name);
		hsphy_idx = 0;
	}

	/* Overwrite the "phys" property to only contain the high-speed phy */
	ret = of_write_prop(dwc3, "phys", sizeof(*phandles), phandles + hsphy_idx);
	if (ret) {
		log_err("Failed to overwrite 'phys' property: %d\n", ret);
		return ret;
	}

	/* Overwrite "phy-names" to only contain a single entry */
	ret = of_write_prop(dwc3, "phy-names", strlen("usb2-phy") + 1, "usb2-phy");
	if (ret) {
		log_err("Failed to overwrite 'phy-names' property: %d\n", ret);
		return ret;
	}

	ret = of_write_prop(dwc3, "maximum-speed", strlen("high-speed") + 1, "high-speed");
	if (ret) {
		log_err("Failed to set 'maximum-speed' property: %d\n", ret);
		return ret;
	}

	return 0;
}

static void fixup_usb_nodes(void)
{
	struct device_node *glue_np = NULL;
	int ret;

	while ((glue_np = of_find_compatible_node(glue_np, NULL, "qcom,dwc3"))) {
		ret = fixup_qcom_dwc3(glue_np);
		if (ret)
			log_warning("Failed to fixup node %s: %d\n", glue_np->name, ret);
	}
}

/* Remove all references to the rpmhpd device */
static void fixup_power_domains(void)
{
	struct device_node *pd = NULL, *np = NULL;
	struct property *prop;
	const __be32 *val;

	/* All Qualcomm platforms name the rpm(h)pd "power-controller" */
	for_each_of_allnodes(pd) {
		if (pd->name && !strcmp("power-controller", pd->name))
			break;
	}

	/* Sanity check that this is indeed a power domain controller */
	if (!of_find_property(pd, "#power-domain-cells", NULL)) {
		log_err("Found power-controller but it doesn't have #power-domain-cells\n");
		return;
	}

	/* Remove all references to the power domain controller */
	for_each_of_allnodes(np) {
		if (!(prop = of_find_property(np, "power-domains", NULL)))
			continue;

		val = prop->value;
		if (val[0] == cpu_to_fdt32(pd->phandle))
			of_remove_property(np, prop);
	}
}

#define time_call(func, ...) \
	do { \
		u64 start = timer_get_us(); \
		func(__VA_ARGS__); \
		debug(#func " took %lluus\n", timer_get_us() - start); \
	} while (0)

void qcom_of_fixup_nodes(void)
{
	time_call(fixup_usb_nodes);
	time_call(fixup_power_domains);
}

int ft_board_setup(void *blob, struct bd_info __maybe_unused *bd)
{
	struct fdt_header *fdt = blob;
	int node;

	/* On RB1/2 we need to fix-up the dr_mode */
	if (!fdt_node_check_compatible(fdt, 0, "qcom,qrb4210-rb2") ||
	    !fdt_node_check_compatible(fdt, 0, "qcom,qrb2210-rb1")) {
		fdt_for_each_node_by_compatible(node, blob, 0, "snps,dwc3") {
			log_debug("%s: Setting 'dr_mode' to OTG\n", fdt_get_name(blob, node, NULL));
			fdt_setprop_string(fdt, node, "dr_mode", "otg");
			break;
		}
	}

	return 0;
}
