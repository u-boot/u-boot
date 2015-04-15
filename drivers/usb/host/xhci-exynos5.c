/*
 * SAMSUNG EXYNOS5 USB HOST XHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
 *	Vikas Sajjan <vikas.sajjan@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file is a conglomeration for DWC3-init sequence and further
 * exynos5 specific PHY-init sequence.
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <usb.h>
#include <watchdog.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <asm/arch/xhci-exynos.h>
#include <asm/gpio.h>
#include <asm-generic/errno.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>

#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct exynos_xhci {
	struct exynos_usb3_phy *usb3_phy;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
	struct gpio_desc vbus_gpio;
};

static struct exynos_xhci exynos;

#ifdef CONFIG_OF_CONTROL
static int exynos_usb3_parse_dt(const void *blob, struct exynos_xhci *exynos)
{
	fdt_addr_t addr;
	unsigned int node;
	int depth;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS5_XHCI);
	if (node <= 0) {
		debug("XHCI: Can't get device node for xhci\n");
		return -ENODEV;
	}

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	addr = fdtdec_get_addr(blob, node, "reg");
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}
	exynos->hcd = (struct xhci_hccr *)addr;

	/* Vbus gpio */
	gpio_request_by_name_nodev(blob, node, "samsung,vbus-gpio", 0,
				   &exynos->vbus_gpio, GPIOD_IS_OUT);

	depth = 0;
	node = fdtdec_next_compatible_subnode(blob, node,
				COMPAT_SAMSUNG_EXYNOS5_USB3_PHY, &depth);
	if (node <= 0) {
		debug("XHCI: Can't get device node for usb3-phy controller\n");
		return -ENODEV;
	}

	/*
	 * Get the base address for usbphy from the device node
	 */
	exynos->usb3_phy = (struct exynos_usb3_phy *)fdtdec_get_addr(blob, node,
								"reg");
	if (exynos->usb3_phy == NULL) {
		debug("Can't get the usbphy register address\n");
		return -ENXIO;
	}

	return 0;
}
#endif

static void exynos5_usb3_phy_init(struct exynos_usb3_phy *phy)
{
	u32 reg;

	/* enabling usb_drd phy */
	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_EN);

	/* Reset USB 3.0 PHY */
	writel(0x0, &phy->phy_reg0);

	clrbits_le32(&phy->phy_param0,
			/* Select PHY CLK source */
			PHYPARAM0_REF_USE_PAD |
			/* Set Loss-of-Signal Detector sensitivity */
			PHYPARAM0_REF_LOSLEVEL_MASK);
	setbits_le32(&phy->phy_param0, PHYPARAM0_REF_LOSLEVEL);

	writel(0x0, &phy->phy_resume);

	/*
	 * Setting the Frame length Adj value[6:1] to default 0x20
	 * See xHCI 1.0 spec, 5.2.4
	 */
	setbits_le32(&phy->link_system,
			LINKSYSTEM_XHCI_VERSION_CONTROL |
			LINKSYSTEM_FLADJ(0x20));

	/* Set Tx De-Emphasis level */
	clrbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH_MASK);
	setbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH);

	setbits_le32(&phy->phy_batchg, PHYBATCHG_UTMI_CLKSEL);

	/* PHYTEST POWERDOWN Control */
	clrbits_le32(&phy->phy_test,
			PHYTEST_POWERDOWN_SSP |
			PHYTEST_POWERDOWN_HSP);

	/* UTMI Power Control */
	writel(PHYUTMI_OTGDISABLE, &phy->phy_utmi);

		/* Use core clock from main PLL */
	reg = PHYCLKRST_REFCLKSEL_EXT_REFCLK |
		/* Default 24Mhz crystal clock */
		PHYCLKRST_FSEL(FSEL_CLKSEL_24M) |
		PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF |
		PHYCLKRST_SSC_REFCLKSEL(0x88) |
		/* Force PortReset of PHY */
		PHYCLKRST_PORTRESET |
		/* Digital power supply in normal operating mode */
		PHYCLKRST_RETENABLEN |
		/* Enable ref clock for SS function */
		PHYCLKRST_REF_SSP_EN |
		/* Enable spread spectrum */
		PHYCLKRST_SSC_EN |
		/* Power down HS Bias and PLL blocks in suspend mode */
		PHYCLKRST_COMMONONN;

	writel(reg, &phy->phy_clk_rst);

	/* giving time to Phy clock to settle before resetting */
	udelay(10);

	reg &= ~PHYCLKRST_PORTRESET;
	writel(reg, &phy->phy_clk_rst);
}

static void exynos5_usb3_phy_exit(struct exynos_usb3_phy *phy)
{
	setbits_le32(&phy->phy_utmi,
			PHYUTMI_OTGDISABLE |
			PHYUTMI_FORCESUSPEND |
			PHYUTMI_FORCESLEEP);

	clrbits_le32(&phy->phy_clk_rst,
			PHYCLKRST_REF_SSP_EN |
			PHYCLKRST_SSC_EN |
			PHYCLKRST_COMMONONN);

	/* PHYTEST POWERDOWN Control to remove leakage current */
	setbits_le32(&phy->phy_test,
			PHYTEST_POWERDOWN_SSP |
			PHYTEST_POWERDOWN_HSP);

	/* disabling usb_drd phy */
	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_DISABLE);
}

static void dwc3_set_mode(struct dwc3 *dwc3_reg, u32 mode)
{
	clrsetbits_le32(&dwc3_reg->g_ctl,
			DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG),
			DWC3_GCTL_PRTCAPDIR(mode));
}

static void dwc3_core_soft_reset(struct dwc3 *dwc3_reg)
{
	/* Before Resetting PHY, put Core in Reset */
	setbits_le32(&dwc3_reg->g_ctl,
			DWC3_GCTL_CORESOFTRESET);

	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0],
			DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg,
			DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(100);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0],
			DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg,
			DWC3_GUSB2PHYCFG_PHYSOFTRST);

	/* After PHYs are stable we can take Core out of reset state */
	clrbits_le32(&dwc3_reg->g_ctl,
			DWC3_GCTL_CORESOFTRESET);
}

static int dwc3_core_init(struct dwc3 *dwc3_reg)
{
	u32 reg;
	u32 revision;
	unsigned int dwc3_hwparams1;

	revision = readl(&dwc3_reg->g_snpsid);
	/* This should read as U3 followed by revision number */
	if ((revision & DWC3_GSNPSID_MASK) != 0x55330000) {
		puts("this is not a DesignWare USB3 DRD Core\n");
		return -EINVAL;
	}

	dwc3_core_soft_reset(dwc3_reg);

	dwc3_hwparams1 = readl(&dwc3_reg->g_hwparams1);

	reg = readl(&dwc3_reg->g_ctl);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;
	reg &= ~DWC3_GCTL_DISSCRAMBLE;
	switch (DWC3_GHWPARAMS1_EN_PWROPT(dwc3_hwparams1)) {
	case DWC3_GHWPARAMS1_EN_PWROPT_CLK:
		reg &= ~DWC3_GCTL_DSBLCLKGTNG;
		break;
	default:
		debug("No power optimization available\n");
	}

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * where the device can fail to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter a Connect/Disconnect loop
	 */
	if ((revision & DWC3_REVISION_MASK) < 0x190a)
		reg |= DWC3_GCTL_U2RSTECN;

	writel(reg, &dwc3_reg->g_ctl);

	return 0;
}

static int exynos_xhci_core_init(struct exynos_xhci *exynos)
{
	int ret;

	exynos5_usb3_phy_init(exynos->usb3_phy);

	ret = dwc3_core_init(exynos->dwc3_reg);
	if (ret) {
		debug("failed to initialize core\n");
		return -EINVAL;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(exynos->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return 0;
}

static void exynos_xhci_core_exit(struct exynos_xhci *exynos)
{
	exynos5_usb3_phy_exit(exynos->usb3_phy);
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct exynos_xhci *ctx = &exynos;
	int ret;

#ifdef CONFIG_OF_CONTROL
	exynos_usb3_parse_dt(gd->fdt_blob, ctx);
#else
	ctx->usb3_phy = (struct exynos_usb3_phy *)samsung_get_base_usb3_phy();
	ctx->hcd = (struct xhci_hccr *)samsung_get_base_usb_xhci();
#endif

	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);

#ifdef CONFIG_OF_CONTROL
	/* setup the Vbus gpio here */
	if (dm_gpio_is_valid(&ctx->vbus_gpio))
		dm_gpio_set_value(&ctx->vbus_gpio, 1);
#endif

	ret = exynos_xhci_core_init(ctx);
	if (ret) {
		puts("XHCI: failed to initialize controller\n");
		return -EINVAL;
	}

	*hccr = (ctx->hcd);
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("Exynos5-xhci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void xhci_hcd_stop(int index)
{
	struct exynos_xhci *ctx = &exynos;

	exynos_xhci_core_exit(ctx);
}
