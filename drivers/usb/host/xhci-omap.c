/*
 * OMAP USB HOST xHCI Controller
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm-generic/errno.h>
#include <asm/omap_common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>

#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include <linux/usb/xhci-omap.h>

#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

static struct omap_xhci omap;

struct usb_dpll_params {
	u16	m;
	u8	n;
	u8	freq:3;
	u8	sd;
	u32	mf;
};

#define	NUM_USB_CLKS		6

static struct usb_dpll_params omap_usb3_dpll_params[NUM_USB_CLKS] = {
	{1250, 5, 4, 20, 0},		/* 12 MHz */
	{3125, 20, 4, 20, 0},		/* 16.8 MHz */
	{1172, 8, 4, 20, 65537},	/* 19.2 MHz */
	{1250, 12, 4, 20, 0},		/* 26 MHz */
	{3125, 47, 4, 20, 92843},	/* 38.4 MHz */
	{1000, 7, 4, 10, 0},        /* 20 MHz */
};

static void omap_usb_dpll_relock(struct omap_usb3_phy *phy_regs)
{
	u32 val;

	writel(SET_PLL_GO, &phy_regs->pll_go);
	do {
		val = readl(&phy_regs->pll_status);
			if (val & PLL_LOCK)
				break;
	} while (1);
}

static void omap_usb_dpll_lock(struct omap_usb3_phy *phy_regs)
{
	u32 clk_index = get_sys_clk_index();
	u32 val;

	val = readl(&phy_regs->pll_config_1);
	val &= ~PLL_REGN_MASK;
	val |= omap_usb3_dpll_params[clk_index].n << PLL_REGN_SHIFT;
	writel(val, &phy_regs->pll_config_1);

	val = readl(&phy_regs->pll_config_2);
	val &= ~PLL_SELFREQDCO_MASK;
	val |= omap_usb3_dpll_params[clk_index].freq << PLL_SELFREQDCO_SHIFT;
	writel(val, &phy_regs->pll_config_2);

	val = readl(&phy_regs->pll_config_1);
	val &= ~PLL_REGM_MASK;
	val |= omap_usb3_dpll_params[clk_index].m << PLL_REGM_SHIFT;
	writel(val, &phy_regs->pll_config_1);

	val = readl(&phy_regs->pll_config_4);
	val &= ~PLL_REGM_F_MASK;
	val |= omap_usb3_dpll_params[clk_index].mf << PLL_REGM_F_SHIFT;
	writel(val, &phy_regs->pll_config_4);

	val = readl(&phy_regs->pll_config_3);
	val &= ~PLL_SD_MASK;
	val |= omap_usb3_dpll_params[clk_index].sd << PLL_SD_SHIFT;
	writel(val, &phy_regs->pll_config_3);

	omap_usb_dpll_relock(phy_regs);
}

static void usb3_phy_partial_powerup(struct omap_usb3_phy *phy_regs)
{
	u32 rate = get_sys_clk_freq()/1000000;
	u32 val;

	val = readl((*ctrl)->control_phy_power_usb);
	val &= ~(USB3_PWRCTL_CLK_CMD_MASK | USB3_PWRCTL_CLK_FREQ_MASK);
	val |= (USB3_PHY_PARTIAL_RX_POWERON | USB3_PHY_TX_RX_POWERON);
	val |= rate << USB3_PWRCTL_CLK_FREQ_SHIFT;

	writel(val, (*ctrl)->control_phy_power_usb);
}

static void usb3_phy_power(int on)
{
	u32 val;

	val = readl((*ctrl)->control_phy_power_usb);
	if (on) {
		val &= ~USB3_PWRCTL_CLK_CMD_MASK;
		val |= USB3_PHY_TX_RX_POWERON;
	} else {
		val &= (~USB3_PWRCTL_CLK_CMD_MASK & ~USB3_PHY_TX_RX_POWERON);
	}

	writel(val, (*ctrl)->control_phy_power_usb);
}

static void dwc_usb3_phy_init(struct omap_usb3_phy *phy_regs)
{
	omap_usb_dpll_lock(phy_regs);

	usb3_phy_partial_powerup(phy_regs);
	/*
	 * Give enough time for the PHY to partially power-up before
	 * powering it up completely. delay value suggested by the HW
	 * team.
	 */
	mdelay(100);
	usb3_phy_power(1);
}

static void omap_enable_phy_clocks(struct omap_xhci *omap)
{
	u32	val;

	/* Setting OCP2SCP1 register */
	setbits_le32((*prcm)->cm_l3init_ocp2scp1_clkctrl,
		     OCP2SCP1_CLKCTRL_MODULEMODE_HW);

	/* Turn on 32K AON clk */
	setbits_le32((*prcm)->cm_coreaon_usb_phy_core_clkctrl,
		     USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

	/* Setting CM_L3INIT_CLKSTCTRL to 0x0 i.e NO sleep */
	writel(0x0, (*prcm)->cm_l3init_clkstctrl);

	val = (USBOTGSS_DMADISABLE |
			USBOTGSS_STANDBYMODE_SMRT_WKUP |
			USBOTGSS_IDLEMODE_NOIDLE);
	writel(val, &omap->otg_wrapper->sysconfig);

	/* Clear the utmi OTG status */
	val = readl(&omap->otg_wrapper->utmi_otg_status);
	writel(val, &omap->otg_wrapper->utmi_otg_status);

	/* Enable interrupts */
	writel(USBOTGSS_COREIRQ_EN, &omap->otg_wrapper->irqenable_set_0);
	val = (USBOTGSS_IRQ_SET_1_IDPULLUP_FALL_EN |
			USBOTGSS_IRQ_SET_1_DISCHRGVBUS_FALL_EN |
			USBOTGSS_IRQ_SET_1_CHRGVBUS_FALL_EN	|
			USBOTGSS_IRQ_SET_1_DRVVBUS_FALL_EN	|
			USBOTGSS_IRQ_SET_1_IDPULLUP_RISE_EN	|
			USBOTGSS_IRQ_SET_1_DISCHRGVBUS_RISE_EN	|
			USBOTGSS_IRQ_SET_1_CHRGVBUS_RISE_EN |
			USBOTGSS_IRQ_SET_1_DRVVBUS_RISE_EN |
			USBOTGSS_IRQ_SET_1_OEVT_EN);
	writel(val, &omap->otg_wrapper->irqenable_set_1);

	/* Clear the IRQ status */
	val = readl(&omap->otg_wrapper->irqstatus_1);
	writel(val, &omap->otg_wrapper->irqstatus_1);
	val = readl(&omap->otg_wrapper->irqstatus_0);
	writel(val, &omap->otg_wrapper->irqstatus_0);

	/* Enable the USB OTG Super speed clocks */
	val = (OPTFCLKEN_REFCLK960M | OTG_SS_CLKCTRL_MODULEMODE_HW);
	setbits_le32((*prcm)->cm_l3init_usb_otg_ss_clkctrl, val);

};

inline int __board_usb_init(void)
{
	return 0;
}
int board_usb_init(void) __attribute__((weak, alias("__board_usb_init")));

static void dwc3_set_mode(struct dwc3 *dwc3_reg, u32 mode)
{
	clrsetbits_le32(&dwc3_reg->g_ctl,
			DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG),
			DWC3_GCTL_PRTCAPDIR(mode));
}

static void dwc3_core_soft_reset(struct dwc3 *dwc3_reg)
{
	/* Before Resetting PHY, put Core in Reset */
	setbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);

	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(100);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

	/* After PHYs are stable we can take Core out of reset state */
	clrbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);
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
		return -1;
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

static int omap_xhci_core_init(struct omap_xhci *omap)
{
	int ret = 0;

	omap_enable_phy_clocks(omap);

	dwc_usb3_phy_init(omap->usb3_phy);

	ret = dwc3_core_init(omap->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(omap->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return ret;
}

static void omap_xhci_core_exit(struct omap_xhci *omap)
{
	usb3_phy_power(0);
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct omap_xhci *ctx = &omap;
	int ret = 0;

	ctx->hcd = (struct xhci_hccr *)OMAP_XHCI_BASE;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);
	ctx->usb3_phy = (struct omap_usb3_phy *)OMAP_OCP1_SCP_BASE;
	ctx->otg_wrapper = (struct omap_dwc_wrapper *)OMAP_OTG_WRAPPER_BASE;

	ret = board_usb_init();
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	ret = omap_xhci_core_init(ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)(OMAP_XHCI_BASE);
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("omap-xhci: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	struct omap_xhci *ctx = &omap;

	omap_xhci_core_exit(ctx);
}
