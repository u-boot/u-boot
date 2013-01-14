/*
 * SAMSUNG EXYNOS USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <usb.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ehci.h>
#include <asm/arch/system.h>
#include <asm/arch/power.h>
#include <asm-generic/errno.h>
#include <linux/compat.h>
#include "ehci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct exynos_ehci {
	struct exynos_usb_phy *usb;
	unsigned int *hcd;
};

static int exynos_usb_parse_dt(const void *blob, struct exynos_ehci *exynos)
{
	unsigned int node;
	int depth;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS_EHCI);
	if (node <= 0) {
		debug("EHCI: Can't get device node for ehci\n");
		return -ENODEV;
	}

	/*
	 * Get the base address for EHCI controller from the device node
	 */
	exynos->hcd = (unsigned int *)fdtdec_get_addr(blob, node, "reg");
	if (exynos->hcd == NULL) {
		debug("Can't get the EHCI register address\n");
		return -ENXIO;
	}

	depth = 0;
	node = fdtdec_next_compatible_subnode(blob, node,
					COMPAT_SAMSUNG_EXYNOS_USB_PHY, &depth);
	if (node <= 0) {
		debug("EHCI: Can't get device node for usb-phy controller\n");
		return -ENODEV;
	}

	/*
	 * Get the base address for usbphy from the device node
	 */
	exynos->usb = (struct exynos_usb_phy *)fdtdec_get_addr(blob, node,
								"reg");
	if (exynos->usb == NULL) {
		debug("Can't get the usbphy register address\n");
		return -ENXIO;
	}

	return 0;
}

/* Setup the EHCI host controller. */
static void setup_usb_phy(struct exynos_usb_phy *usb)
{
	set_usbhost_mode(USB20_PHY_CFG_HOST_LINK_EN);

	set_usbhost_phy_ctrl(POWER_USB_HOST_PHY_CTRL_EN);

	clrbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_FSEL_MASK |
			HOST_CTRL0_COMMONON_N |
			/* HOST Phy setting */
			HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ |
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP);

	setbits_le32(&usb->usbphyctrl0,
			/* Setting up the ref freq */
			(CLK_24MHZ << 16) |
			/* HOST Phy setting */
			HOST_CTRL0_LINKSWRST |
			HOST_CTRL0_UTMISWRST);
	udelay(10);
	clrbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_LINKSWRST |
			HOST_CTRL0_UTMISWRST);
	udelay(20);

	/* EHCI Ctrl setting */
	setbits_le32(&usb->ehcictrl,
			EHCICTRL_ENAINCRXALIGN |
			EHCICTRL_ENAINCR4 |
			EHCICTRL_ENAINCR8 |
			EHCICTRL_ENAINCR16);
}

/* Reset the EHCI host controller. */
static void reset_usb_phy(struct exynos_usb_phy *usb)
{
	/* HOST_PHY reset */
	setbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ |
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP);

	set_usbhost_phy_ctrl(POWER_USB_HOST_PHY_CTRL_DISABLE);
}

/*
 * EHCI-initialization
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct exynos_ehci *exynos = NULL;

	exynos = (struct exynos_ehci *)
			kzalloc(sizeof(struct exynos_ehci), GFP_KERNEL);
	if (!exynos) {
		debug("failed to allocate exynos ehci context\n");
		return -ENOMEM;
	}

	exynos_usb_parse_dt(gd->fdt_blob, exynos);

	setup_usb_phy(exynos->usb);

	*hccr = (struct ehci_hccr *)(exynos->hcd);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("Exynos5-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	kfree(exynos);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	struct exynos_ehci *exynos = NULL;

	exynos = (struct exynos_ehci *)
			kzalloc(sizeof(struct exynos_ehci), GFP_KERNEL);
	if (!exynos) {
		debug("failed to allocate exynos ehci context\n");
		return -ENOMEM;
	}

	exynos_usb_parse_dt(gd->fdt_blob, exynos);

	reset_usb_phy(exynos->usb);

	kfree(exynos);

	return 0;
}
