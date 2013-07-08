/*
 * TI's Davinci platform specific USB wrapper functions.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#include <common.h>
#include <asm/io.h>
#include "davinci.h"
#include <asm/arch/hardware.h>

#if !defined(CONFIG_DV_USBPHY_CTL)
#define CONFIG_DV_USBPHY_CTL (USBPHY_SESNDEN | USBPHY_VBDTCTEN)
#endif

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	.regs		= (struct musb_regs *)MENTOR_USB0_BASE,
	.timeout	= DAVINCI_USB_TIMEOUT,
	.musb_speed	= 0,
};

/* MUSB module register overlay */
struct davinci_usb_regs *dregs;

/*
 * Enable the USB phy
 */
static u8 phy_on(void)
{
	u32 timeout;
#ifdef DAVINCI_DM365EVM
	u32 val;
#endif
	/* Wait until the USB phy is turned on */
#ifdef DAVINCI_DM365EVM
	writel(USBPHY_PHY24MHZ | USBPHY_SESNDEN |
			USBPHY_VBDTCTEN, USBPHY_CTL_PADDR);
#else
	writel(CONFIG_DV_USBPHY_CTL, USBPHY_CTL_PADDR);
#endif
	timeout = musb_cfg.timeout;

#ifdef DAVINCI_DM365EVM
	/* Set the ownership of GIO33 to USB */
	val = readl(PINMUX4);
	val &= ~(PINMUX4_USBDRVBUS_BITCLEAR);
	val |= PINMUX4_USBDRVBUS_BITSET;
	writel(val, PINMUX4);
#endif
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

void __enable_vbus(void)
{
	/*
	 *  nothing to do, vbus is handled through the cpu.
	 *  Define this function in board code, if it is
	 *  different on your board.
	 */
}
void  enable_vbus(void)
	__attribute__((weak, alias("__enable_vbus")));

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
