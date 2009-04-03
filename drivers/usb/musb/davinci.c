/*
 * TI's Davinci platform specific USB wrapper functions.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#include <common.h>
#include <asm/io.h>
#include "davinci.h"

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	(struct	musb_regs *)MENTOR_USB0_BASE,
	DAVINCI_USB_TIMEOUT,
	0
};

/* MUSB module register overlay */
struct davinci_usb_regs *dregs;

/*
 * Enable the USB phy
 */
static u8 phy_on(void)
{
	u32 timeout;

	/* Wait until the USB phy is turned on */
	writel(USBPHY_SESNDEN | USBPHY_VBDTCTEN, USBPHY_CTL_PADDR);
	timeout = musb_cfg.timeout;
	while (timeout--)
		if (readl(USBPHY_CTL_PADDR) & USBPHY_PHYCLKGD)
			return 1;

	/* USB phy was not turned on */
	return 0;
}

/*
 * Disable the USB phy
 */
static void phy_off(void)
{
	/* powerdown the on-chip PHY and its oscillator */
	writel(USBPHY_OSCPDWN | USBPHY_PHYPDWN, USBPHY_CTL_PADDR);
}

/*
 * This function performs Davinci platform specific initialization for usb0.
 */
int musb_platform_init(void)
{
	u32  revision;

	/* enable USB VBUS */
	enable_vbus();

	/* start the on-chip USB phy and its pll */
	if (!phy_on())
		return -1;

	/* reset the controller */
	dregs = (struct davinci_usb_regs *)DAVINCI_USB0_BASE;
	writel(1, &dregs->ctrlr);
	udelay(5000);

	/* Returns zero if e.g. not clocked */
	revision = readl(&dregs->version);
	if (!revision)
		return -1;

	/* Disable all interrupts */
	writel(DAVINCI_USB_USBINT_MASK | DAVINCI_USB_RXINT_MASK |
			DAVINCI_USB_TXINT_MASK , &dregs->intmsksetr);
	return 0;
}

/*
 * This function performs Davinci platform specific deinitialization for usb0.
 */
void musb_platform_deinit(void)
{
	/* Turn of the phy */
	phy_off();

	/* flush any interrupts */
	writel(DAVINCI_USB_USBINT_MASK | DAVINCI_USB_TXINT_MASK |
			DAVINCI_USB_RXINT_MASK , &dregs->intclrr);
}
