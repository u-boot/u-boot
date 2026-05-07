// SPDX-License-Identifier: GPL-2.0
/*
 * TI AM62 specific glue layer for DWC3
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>

#include "dwc3-generic.h"

#define USBSS_MODE_CONTROL		0x1c
#define USBSS_PHY_CONFIG		0x8
#define USBSS_PHY_VBUS_SEL_MASK		GENMASK(2, 1)
#define USBSS_PHY_VBUS_SEL_SHIFT	1
#define USBSS_MODE_VALID	BIT(0)
#define PHY_PLL_REFCLK_MASK	GENMASK(3, 0)
static const int dwc3_ti_am62_rate_table[] = {	/* in KHZ */
	9600,
	10000,
	12000,
	19200,
	20000,
	24000,
	25000,
	26000,
	38400,
	40000,
	58000,
	50000,
	52000,
};

static void dwc3_ti_am62_glue_configure(struct udevice *dev, int index,
					enum usb_dr_mode mode)
{
	struct clk usb2_refclk;
	int rate_code, i, ret;
	unsigned long rate;
	u32 reg;
	void *usbss;
	bool vbus_divider;
	struct regmap *syscon;
	struct ofnode_phandle_args args;

	usbss = dev_remap_addr_index(dev, 0);
	if (IS_ERR(usbss)) {
		dev_err(dev, "can't map IOMEM resource\n");
		return;
	}

	ret = clk_get_by_name(dev, "ref", &usb2_refclk);
	if (ret) {
		dev_err(dev, "can't get usb2_refclk\n");
		return;
	}

	/* Calculate the rate code */
	rate = clk_get_rate(&usb2_refclk);
	rate /= 1000;	/* To KHz */
	for (i = 0; i < ARRAY_SIZE(dwc3_ti_am62_rate_table); i++) {
		if (dwc3_ti_am62_rate_table[i] == rate)
			break;
	}

	if (i == ARRAY_SIZE(dwc3_ti_am62_rate_table)) {
		dev_err(dev, "unsupported usb2_refclk rate: %lu KHz\n", rate);
		return;
	}

	rate_code = i;

	/* Read the syscon property */
	syscon = syscon_regmap_lookup_by_phandle(dev, "ti,syscon-phy-pll-refclk");
	if (IS_ERR(syscon)) {
		dev_err(dev, "unable to get ti,syscon-phy-pll-refclk regmap\n");
		return;
	}

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "ti,syscon-phy-pll-refclk", NULL, 1,
					     0, &args);
	if (ret)
		return;

	/* Program PHY PLL refclk by reading syscon property */
	ret = regmap_update_bits(syscon, args.args[0], PHY_PLL_REFCLK_MASK, rate_code);
	if (ret) {
		dev_err(dev, "failed to set phy pll reference clock rate\n");
		return;
	}

	/* VBUS divider select */
	reg = readl(usbss + USBSS_PHY_CONFIG);
	vbus_divider = dev_read_bool(dev, "ti,vbus-divider");
	if (vbus_divider)
		reg |= 1 << USBSS_PHY_VBUS_SEL_SHIFT;

	writel(reg, usbss + USBSS_PHY_CONFIG);

	/* Set mode valid */
	reg = readl(usbss + USBSS_MODE_CONTROL);
	reg |= USBSS_MODE_VALID;
	writel(reg, usbss + USBSS_MODE_CONTROL);
}

struct dwc3_glue_ops ti_am62_ops = {
	.glue_configure = dwc3_ti_am62_glue_configure,
};

static const struct udevice_id dwc3_am62_match[] = {
	{ .compatible = "ti,am62-usb", .data = (ulong)&ti_am62_ops },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(dwc3_am62_wrapper) = {
	.name	= "dwc3-am62",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = dwc3_am62_match,
	.bind = dwc3_glue_bind,
	.probe = dwc3_glue_probe,
	.remove = dwc3_glue_remove,
	.plat_auto	= sizeof(struct dwc3_glue_data),
};
