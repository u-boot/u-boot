/*
 * OMAP USB PHY Support
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

#include "../host/xhci.h"

#ifdef CONFIG_OMAP_USB3PHY1_HOST
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

void usb_phy_power(int on)
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

void omap_usb3_phy_init(struct omap_usb3_phy *phy_regs)
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

static void omap_enable_usb3_phy(struct omap_xhci *omap)
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
#endif /* CONFIG_OMAP_USB3PHY1_HOST */

#ifdef CONFIG_OMAP_USB2PHY2_HOST
static void omap_enable_usb2_phy2(struct omap_xhci *omap)
{
	u32 reg, val;

	val = (~USB2PHY_AUTORESUME_EN & USB2PHY_DISCHGDET);
	writel(val, (*ctrl)->control_srcomp_north_side);

	setbits_le32((*prcm)->cm_coreaon_usb_phy2_core_clkctrl,
			USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K);

	setbits_le32((*prcm)->cm_l3init_hsusbhost_clkctrl,
					(USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K |
					 OTG_SS_CLKCTRL_MODULEMODE_HW));

	/* This is an undocumented Reserved register */
	reg = 0x4a0086c0;
	val = readl(reg);
	val |= 0x100;
	setbits_le32(reg, val);
}

void usb_phy_power(int on)
{
	return;
}
#endif /* CONFIG_OMAP_USB2PHY2_HOST */

#ifdef CONFIG_AM437X_USB2PHY2_HOST
static void am437x_enable_usb2_phy2(struct omap_xhci *omap)
{
	const u32 usb_otg_ss_clk_val = (USBOTGSSX_CLKCTRL_MODULE_EN |
				USBOTGSSX_CLKCTRL_OPTFCLKEN_REFCLK960);

	writel(usb_otg_ss_clk_val, PRM_PER_USB_OTG_SS0_CLKCTRL);
	writel(usb_otg_ss_clk_val, PRM_PER_USB_OTG_SS1_CLKCTRL);

	writel(USBPHYOCPSCP_MODULE_EN, PRM_PER_USBPHYOCP2SCP0_CLKCTRL);
	writel(USBPHYOCPSCP_MODULE_EN, PRM_PER_USBPHYOCP2SCP1_CLKCTRL);
}

void usb_phy_power(int on)
{
	return;
}
#endif /* CONFIG_AM437X_USB2PHY2_HOST */

void omap_reset_usb_phy(struct dwc3 *dwc3_reg)
{
	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(100);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

}

void omap_enable_phy(struct omap_xhci *omap)
{
#ifdef CONFIG_OMAP_USB2PHY2_HOST
	omap_enable_usb2_phy2(omap);
#endif

#ifdef CONFIG_AM437X_USB2PHY2_HOST
	am437x_enable_usb2_phy2(omap);
#endif

#ifdef CONFIG_OMAP_USB3PHY1_HOST
	omap_enable_usb3_phy(omap);
	omap_usb3_phy_init(omap->usb3_phy);
#endif
}
