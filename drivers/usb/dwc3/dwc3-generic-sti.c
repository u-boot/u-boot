// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * STi specific glue layer for DWC3
 *
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_NOP

#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <linux/usb/otg.h>
#include "dwc3-generic.h"

/* glue registers */
#define CLKRST_CTRL		0x00
#define AUX_CLK_EN		BIT(0)
#define SW_PIPEW_RESET_N	BIT(4)
#define EXT_CFG_RESET_N		BIT(8)

#define XHCI_REVISION		BIT(12)

#define USB2_VBUS_MNGMNT_SEL1	0x2C
#define USB2_VBUS_UTMIOTG	0x1

#define SEL_OVERRIDE_VBUSVALID(n)	((n) << 0)
#define SEL_OVERRIDE_POWERPRESENT(n)	((n) << 4)
#define SEL_OVERRIDE_BVALID(n)		((n) << 8)

/* Static DRD configuration */
#define USB3_CONTROL_MASK		0xf77

#define USB3_DEVICE_NOT_HOST		BIT(0)
#define USB3_FORCE_VBUSVALID		BIT(1)
#define USB3_DELAY_VBUSVALID		BIT(2)
#define USB3_SEL_FORCE_OPMODE		BIT(4)
#define USB3_FORCE_OPMODE(n)		((n) << 5)
#define USB3_SEL_FORCE_DPPULLDOWN2	BIT(8)
#define USB3_FORCE_DPPULLDOWN2		BIT(9)
#define USB3_SEL_FORCE_DMPULLDOWN2	BIT(10)
#define USB3_FORCE_DMPULLDOWN2		BIT(11)

static void dwc3_stih407_glue_configure(struct udevice *dev, int index,
					enum usb_dr_mode mode)
{
	struct dwc3_glue_data *glue = dev_get_plat(dev);
	struct regmap *regmap;
	ulong syscfg_base;
	ulong syscfg_offset;
	ulong glue_base;
	int ret;

	/* deassert both powerdown and softreset */
	ret = reset_deassert_bulk(&glue->resets);
	if (ret) {
		dev_err(dev, "reset_deassert_bulk error: %d\n", ret);
		return;
	}

	regmap = syscon_regmap_lookup_by_phandle(dev, "st,syscfg");
	if (IS_ERR(regmap)) {
		dev_err(dev, "unable to get st,syscfg, dev %s\n", dev->name);
		return;
	}

	syscfg_base = regmap->ranges[0].start;
	glue_base = dev_read_addr_index(dev, 0);
	syscfg_offset = dev_read_addr_index(dev, 1);

	clrbits_le32(syscfg_base + syscfg_offset, USB3_CONTROL_MASK);

	/* glue drd init */
	switch (mode) {
	case USB_DR_MODE_PERIPHERAL:
		clrbits_le32(syscfg_base + syscfg_offset,
			     USB3_DELAY_VBUSVALID | USB3_SEL_FORCE_OPMODE |
			     USB3_FORCE_OPMODE(0x3) | USB3_SEL_FORCE_DPPULLDOWN2 |
			     USB3_FORCE_DPPULLDOWN2 | USB3_SEL_FORCE_DMPULLDOWN2 |
			     USB3_FORCE_DMPULLDOWN2);

		setbits_le32(syscfg_base + syscfg_offset,
			     USB3_DEVICE_NOT_HOST | USB3_FORCE_VBUSVALID);
		break;

	case USB_DR_MODE_HOST:
		clrbits_le32(syscfg_base + syscfg_offset,
			     USB3_DEVICE_NOT_HOST | USB3_FORCE_VBUSVALID |
			     USB3_SEL_FORCE_OPMODE | USB3_FORCE_OPMODE(0x3) |
			     USB3_SEL_FORCE_DPPULLDOWN2 | USB3_FORCE_DPPULLDOWN2 |
			     USB3_SEL_FORCE_DMPULLDOWN2 | USB3_FORCE_DMPULLDOWN2);

		setbits_le32(syscfg_base + syscfg_offset, USB3_DELAY_VBUSVALID);
		break;

	default:
		dev_err(dev, "Unsupported mode of operation %d\n", mode);
		return;
	}

	/* glue init */
	setbits_le32(glue_base + CLKRST_CTRL, AUX_CLK_EN | EXT_CFG_RESET_N | XHCI_REVISION);
	clrbits_le32(glue_base + CLKRST_CTRL, SW_PIPEW_RESET_N);

	/* configure mux for vbus, powerpresent and bvalid signals */
	setbits_le32(glue_base + USB2_VBUS_MNGMNT_SEL1,
		     SEL_OVERRIDE_VBUSVALID(USB2_VBUS_UTMIOTG) |
		     SEL_OVERRIDE_POWERPRESENT(USB2_VBUS_UTMIOTG) |
		     SEL_OVERRIDE_BVALID(USB2_VBUS_UTMIOTG));
	setbits_le32(glue_base + CLKRST_CTRL, SW_PIPEW_RESET_N);
};

struct dwc3_glue_ops stih407_ops = {
	.glue_configure = dwc3_stih407_glue_configure,
};

static const struct udevice_id dwc3_sti_match[] = {
	{ .compatible = "st,stih407-dwc3", .data = (ulong)&stih407_ops},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(dwc3_sti_wrapper) = {
	.name = "dwc3-sti",
	.id = UCLASS_NOP,
	.of_match = dwc3_sti_match,
	.bind = dwc3_glue_bind,
	.probe = dwc3_glue_probe,
	.remove = dwc3_glue_remove,
	.plat_auto = sizeof(struct dwc3_glue_data),
};
