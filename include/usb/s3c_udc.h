/*
 * drivers/usb/gadget/s3c_udc.h
 * Designware DWC2 on-chip full/high speed USB device controllers
 * Copyright (C) 2005 for Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __S3C_USB_GADGET
#define __S3C_USB_GADGET

#define PHY0_SLEEP              (1 << 5)

struct dwc2_plat_otg_data {
	int		(*phy_control)(int on);
	unsigned int	regs_phy;
	unsigned int	regs_otg;
	unsigned int    usb_phy_ctrl;
	unsigned int    usb_flags;
	unsigned int	usb_gusbcfg;
};

int s3c_udc_probe(struct dwc2_plat_otg_data *pdata);

#endif
