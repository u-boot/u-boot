/*
 * SAMSUNG EXYNOS USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <asm/gpio.h>
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
	struct ehci_hccr *hcd;
	struct fdt_gpio_state vbus_gpio;
};

static struct exynos_ehci exynos;

#ifdef CONFIG_OF_CONTROL
static int exynos_usb_parse_dt(const void *blob, struct exynos_ehci *exynos)
{
	fdt_addr_t addr;
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
	addr = fdtdec_get_addr(blob, node, "reg");
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get the EHCI register address\n");
		return -ENXIO;
	}

	exynos->hcd = (struct ehci_hccr *)addr;

	/* Vbus gpio */
	fdtdec_decode_gpio(blob, node, "samsung,vbus-gpio", &exynos->vbus_gpio);

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
#endif

/* Setup the EHCI host controller. */
static void setup_usb_phy(struct exynos_usb_phy *usb)
{
	u32 hsic_ctrl;

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

	/* HSIC Phy Setting */
	hsic_ctrl = (HSIC_CTRL_FORCESUSPEND |
			HSIC_CTRL_FORCESLEEP |
			HSIC_CTRL_SIDDQ);

	clrbits_le32(&usb->hsicphyctrl1, hsic_ctrl);
	clrbits_le32(&usb->hsicphyctrl2, hsic_ctrl);

	hsic_ctrl = (((HSIC_CTRL_REFCLKDIV_12 & HSIC_CTRL_REFCLKDIV_MASK)
				<< HSIC_CTRL_REFCLKDIV_SHIFT)
			| ((HSIC_CTRL_REFCLKSEL & HSIC_CTRL_REFCLKSEL_MASK)
				<< HSIC_CTRL_REFCLKSEL_SHIFT)
			| HSIC_CTRL_UTMISWRST);

	setbits_le32(&usb->hsicphyctrl1, hsic_ctrl);
	setbits_le32(&usb->hsicphyctrl2, hsic_ctrl);

	udelay(10);

	clrbits_le32(&usb->hsicphyctrl1, HSIC_CTRL_PHYSWRST |
					HSIC_CTRL_UTMISWRST);

	clrbits_le32(&usb->hsicphyctrl2, HSIC_CTRL_PHYSWRST |
					HSIC_CTRL_UTMISWRST);

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
	u32 hsic_ctrl;

	/* HOST_PHY reset */
	setbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ |
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP);

	/* HSIC Phy reset */
	hsic_ctrl = (HSIC_CTRL_FORCESUSPEND |
			HSIC_CTRL_FORCESLEEP |
			HSIC_CTRL_SIDDQ |
			HSIC_CTRL_PHYSWRST);

	setbits_le32(&usb->hsicphyctrl1, hsic_ctrl);
	setbits_le32(&usb->hsicphyctrl2, hsic_ctrl);

	set_usbhost_phy_ctrl(POWER_USB_HOST_PHY_CTRL_DISABLE);
}

/*
 * EHCI-initialization
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct exynos_ehci *ctx = &exynos;

#ifdef CONFIG_OF_CONTROL
	if (exynos_usb_parse_dt(gd->fdt_blob, ctx)) {
		debug("Unable to parse device tree for ehci-exynos\n");
		return -ENODEV;
	}
#else
	ctx->usb = (struct exynos_usb_phy *)samsung_get_base_usb_phy();
	ctx->hcd = (struct ehci_hccr *)samsung_get_base_usb_ehci();
#endif

#ifdef CONFIG_OF_CONTROL
	/* setup the Vbus gpio here */
	if (fdt_gpio_isvalid(&ctx->vbus_gpio) &&
	    !fdtdec_setup_gpio(&ctx->vbus_gpio))
		gpio_direction_output(ctx->vbus_gpio.gpio, 1);
#endif

	setup_usb_phy(ctx->usb);

	board_usb_init(index, init);

	*hccr = ctx->hcd;
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("Exynos5-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	struct exynos_ehci *ctx = &exynos;

	reset_usb_phy(ctx->usb);

	return 0;
}
