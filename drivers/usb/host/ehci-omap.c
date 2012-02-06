/*
 * (C) Copyright 2011 Ilya Yanok, Emcraft Systems
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Derived from Beagle Board code by
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */
#include <common.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clocks.h>
#include <asm/arch/clocks_omap3.h>
#include <asm/arch/ehci_omap3.h>
#include <asm/arch/sys_proto.h>
#include "ehci-core.h"

inline int __board_usb_init(void)
{
	return 0;
}
int board_usb_init(void) __attribute__((weak, alias("__board_usb_init")));

#if defined(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO) || \
	defined(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO)
/* controls PHY(s) reset signal(s) */
static inline void omap_ehci_phy_reset(int on, int delay)
{
	/*
	 * Refer ISSUE1:
	 * Hold the PHY in RESET for enough time till
	 * PHY is settled and ready
	 */
	if (delay && !on)
		udelay(delay);
#ifdef CONFIG_OMAP_EHCI_PHY1_RESET_GPIO
	gpio_request(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, "USB PHY1 reset");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, !on);
#endif
#ifdef CONFIG_OMAP_EHCI_PHY2_RESET_GPIO
	gpio_request(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO, "USB PHY2 reset");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO, !on);
#endif

	/* Hold the PHY in RESET for enough time till DIR is high */
	/* Refer: ISSUE1 */
	if (delay && on)
		udelay(delay);
}
#else
#define omap_ehci_phy_reset(on, delay)	do {} while (0)
#endif

/* Reset is needed otherwise the kernel-driver will throw an error. */
int ehci_hcd_stop(void)
{
	debug("Resetting OMAP3 EHCI\n");
	omap_ehci_phy_reset(1, 0);
	writel(OMAP_UHH_SYSCONFIG_SOFTRESET,
			OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);
	/* disable USB clocks */
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	sr32(&prcm_base->iclken_usbhost, 0, 1, 0);
	sr32(&prcm_base->fclken_usbhost, 0, 2, 0);
	sr32(&prcm_base->iclken3_core, 2, 1, 0);
	sr32(&prcm_base->fclken3_core, 2, 1, 0);
	return 0;
}

/*
 * Initialize the OMAP3 EHCI controller and PHY.
 * Based on "drivers/usb/host/ehci-omap.c" from Linux 2.6.37.
 * See there for additional Copyrights.
 */
int ehci_hcd_init(void)
{
	int ret;

	debug("Initializing OMAP3 EHCI\n");

	ret = board_usb_init();
	if (ret < 0)
		return ret;

	/* Put the PHY in RESET */
	omap_ehci_phy_reset(1, 10);

	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	/* Enable USBHOST_L3_ICLK (USBHOST_MICLK) */
	sr32(&prcm_base->iclken_usbhost, 0, 1, 1);
	/*
	 * Enable USBHOST_48M_FCLK (USBHOST_FCLK1)
	 * and USBHOST_120M_FCLK (USBHOST_FCLK2)
	 */
	sr32(&prcm_base->fclken_usbhost, 0, 2, 3);
	/* Enable USBTTL_ICLK */
	sr32(&prcm_base->iclken3_core, 2, 1, 1);
	/* Enable USBTTL_FCLK */
	sr32(&prcm_base->fclken3_core, 2, 1, 1);
	debug("USB clocks enabled\n");

	/* perform TLL soft reset, and wait until reset is complete */
	writel(OMAP_USBTLL_SYSCONFIG_SOFTRESET,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);
	/* Wait for TLL reset to complete */
	while (!(readl(OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSSTATUS)
			& OMAP_USBTLL_SYSSTATUS_RESETDONE))
		;
	debug("TLL reset done\n");

	writel(OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
		OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
		OMAP_USBTLL_SYSCONFIG_CACTIVITY,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);

	/* Put UHH in NoIdle/NoStandby mode */
	writel(OMAP_UHH_SYSCONFIG_ENAWAKEUP
		| OMAP_UHH_SYSCONFIG_SIDLEMODE
		| OMAP_UHH_SYSCONFIG_CACTIVITY
		| OMAP_UHH_SYSCONFIG_MIDLEMODE,
		OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);

	/* setup burst configurations */
	writel(OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN,
		OMAP3_UHH_BASE + OMAP_UHH_HOSTCONFIG);

	omap_ehci_phy_reset(0, 10);

	hccr = (struct ehci_hccr *)(OMAP3_EHCI_BASE);
	hcor = (struct ehci_hcor *)(OMAP3_EHCI_BASE + 0x10);

	debug("OMAP3 EHCI init done\n");
	return 0;
}
