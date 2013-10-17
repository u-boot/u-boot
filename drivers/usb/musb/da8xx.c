/*
 * da8xx.c - TI's DA8xx platform specific usb wrapper functions.
 *
 * Author: Ajay Kumar Gupta <ajay.gupta@ti.com>
 *
 * Based on drivers/usb/musb/davinci.c
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include "musb_core.h"
#include <asm/arch/da8xx-usb.h>

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	.regs		= (struct musb_regs *)DA8XX_USB_OTG_CORE_BASE,
	.timeout	= DA8XX_USB_OTG_TIMEOUT,
	.musb_speed	= 0,
};

/*
 * This function enables VBUS by driving the GPIO Bank4 Pin 15 high.
 */
static void enable_vbus(void)
{
	u32 value;

	/* configure GPIO bank4 pin 15 in output direction */
	value = readl(&davinci_gpio_bank45->dir);
	writel((value & (~DA8XX_USB_VBUS_GPIO)), &davinci_gpio_bank45->dir);

	/* set GPIO bank4 pin 15 high to drive VBUS */
	value = readl(&davinci_gpio_bank45->set_data);
	writel((value | DA8XX_USB_VBUS_GPIO), &davinci_gpio_bank45->set_data);
}

/*
 * Enable the usb0 phy. This initialization procedure is explained in
 * the DA8xx USB user guide document.
 */
static u8 phy_on(void)
{
	u32 timeout;
	u32 cfgchip2;

	cfgchip2 = readl(&davinci_syscfg_regs->cfgchip2);

	cfgchip2 &= ~(CFGCHIP2_RESET | CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN |
		      CFGCHIP2_OTGMODE | CFGCHIP2_REFFREQ);
	cfgchip2 |= CFGCHIP2_SESENDEN | CFGCHIP2_VBDTCTEN | CFGCHIP2_PHY_PLLON |
		    CFGCHIP2_REFFREQ_24MHZ;

	writel(cfgchip2, &davinci_syscfg_regs->cfgchip2);

	/* wait until the usb phy pll locks */
	timeout = musb_cfg.timeout;
	while (timeout--)
		if (readl(&davinci_syscfg_regs->cfgchip2) & CFGCHIP2_PHYCLKGD)
			return 1;

	/* USB phy was not turned on */
	return 0;
}

/*
 * Disable the usb phy
 */
static void phy_off(void)
{
	u32 cfgchip2;

	/*
	 * Power down the on-chip PHY.
	 */
	cfgchip2 = readl(&davinci_syscfg_regs->cfgchip2);
	cfgchip2 &= ~CFGCHIP2_PHY_PLLON;
	cfgchip2 |= CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN;
	writel(cfgchip2, &davinci_syscfg_regs->cfgchip2);
}

/*
 * This function performs DA8xx platform specific initialization for usb0.
 */
int musb_platform_init(void)
{
	u32  revision;

	/* enable psc for usb2.0 */
	lpsc_on(33);

	/* enable usb vbus */
	enable_vbus();

	/* reset the controller */
	writel(0x1, &da8xx_usb_regs->control);
	udelay(5000);

	/* start the on-chip usb phy and its pll */
	if (phy_on() == 0)
		return -1;

	/* Returns zero if e.g. not clocked */
	revision = readl(&da8xx_usb_regs->revision);
	if (revision == 0)
		return -1;

	/* Disable all interrupts */
	writel((DA8XX_USB_USBINT_MASK | DA8XX_USB_TXINT_MASK |
		DA8XX_USB_RXINT_MASK), &da8xx_usb_regs->intmsk_set);
	return 0;
}

/*
 * This function performs DA8xx platform specific deinitialization for usb0.
 */
void musb_platform_deinit(void)
{
	/* Turn of the phy */
	phy_off();

	/* flush any interrupts */
	writel((DA8XX_USB_USBINT_MASK | DA8XX_USB_TXINT_MASK |
		DA8XX_USB_RXINT_MASK), &da8xx_usb_regs->intmsk_clr);
	writel(0, &da8xx_usb_regs->eoi);
}
