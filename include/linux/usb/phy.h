/* SPDX-License-Identifier: GPL-2.0 */
/*
 * USB PHY defines
 *
 * These APIs may be used between USB controllers.  USB device drivers
 * (for either host or peripheral roles) don't use these calls; they
 * continue to use just usb_device and usb_gadget.
 */

#ifndef __LINUX_USB_PHY_H
#define __LINUX_USB_PHY_H

#include <dm/ofnode.h>

enum usb_phy_interface {
	USBPHY_INTERFACE_MODE_UNKNOWN,
	USBPHY_INTERFACE_MODE_UTMI,
	USBPHY_INTERFACE_MODE_UTMIW,
	USBPHY_INTERFACE_MODE_ULPI,
	USBPHY_INTERFACE_MODE_SERIAL,
	USBPHY_INTERFACE_MODE_HSIC,
};

/* associate a type with PHY */
enum usb_phy_type {
	USB_PHY_TYPE_UNDEFINED,
	USB_PHY_TYPE_USB2,
	USB_PHY_TYPE_USB3,
};

#if CONFIG_IS_ENABLED(DM_USB)
/**
 * usb_get_phy_mode - Get phy mode for given device_node
 * @np:	Pointer to the given device_node
 *
 * The function gets phy interface string from property 'phy_type',
 * and returns the corresponding enum usb_phy_interface
 */
enum usb_phy_interface usb_get_phy_mode(ofnode node);
#else
static inline enum usb_phy_interface usb_get_phy_mode(ofnode node)
{
	return USBPHY_INTERFACE_MODE_UNKNOWN;
}
#endif

struct usb_phy {
	struct device		*dev;

	/* initialize/shutdown the phy */
	int     (*init)(struct usb_phy *x);
	void    (*shutdown)(struct usb_phy *x);

	/* enable/disable VBUS */
	int     (*set_vbus)(struct usb_phy *x, int on);

	/* effective for B devices, ignored for A-peripheral */
	int     (*set_power)(struct usb_phy *x,
			     unsigned mA);
};

static inline int
usb_phy_init(struct usb_phy *x)
{
        if (x && x->init)
                return x->init(x);

        return 0;
}

static inline void
usb_phy_shutdown(struct usb_phy *x)
{
	if (x && x->shutdown)
		x->shutdown(x);
}

static inline int
usb_phy_set_power(struct usb_phy *x, unsigned mA)
{
	if (!x)
		return 0;

	/* TODO usb_phy_set_charger_current(x, mA); */

	if (x->set_power)
		return x->set_power(x, mA);
	return 0;
}

static inline int
usb_phy_set_suspend(struct usb_phy *x, int suspend)
{
	return 0;
}
#endif /* __LINUX_USB_PHY_H */
