/*
 * USB 3.0 DRD Controller
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <usb.h>
#include <asm/arch/psc_defs.h>
#include <asm/io.h>
#include <linux/usb/dwc3.h>
#include <asm/arch/xhci-keystone.h>
#include <asm-generic/errno.h>
#include <linux/list.h>
#include "xhci.h"

struct kdwc3_irq_regs {
	u32 revision;	/* 0x000 */
	u32 rsvd0[3];
	u32 sysconfig;	/* 0x010 */
	u32 rsvd1[1];
	u32 irq_eoi;
	u32 rsvd2[1];
	struct {
		u32 raw_status;
		u32 status;
		u32 enable_set;
		u32 enable_clr;
	} irqs[16];
};

struct keystone_xhci {
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
	struct xhci_hcor *hcor;
	struct kdwc3_irq_regs *usbss;
	struct keystone_xhci_phy *phy;
};

struct keystone_xhci keystone;

static void keystone_xhci_phy_set(struct keystone_xhci_phy *phy)
{
	u32 val;

	/*
	 * VBUSVLDEXTSEL has a default value of 1 in BootCfg but shouldn't.
	 * It should always be cleared because our USB PHY has an onchip VBUS
	 * analog comparator.
	 */
	val = readl(&phy->phy_clock);
	/* quit selecting the vbusvldextsel by default! */
	val &= ~USB3_PHY_OTG_VBUSVLDECTSEL;
	writel(val, &phy->phy_clock);
}

static void keystone_xhci_phy_unset(struct keystone_xhci_phy *phy)
{
	u32 val;

	/* Disable the PHY REFCLK clock gate */
	val = readl(&phy->phy_clock);
	val &= ~USB3_PHY_REF_SSP_EN;
	writel(val, &phy->phy_clock);
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
	setbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);

	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg[0], DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(100);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg[0], DWC3_GUSB2PHYCFG_PHYSOFTRST);

	/* After PHYs are stable we can take Core out of reset state */
	clrbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);
}

static int dwc3_core_init(struct dwc3 *dwc3_reg)
{
	u32 revision, val;
	unsigned long t_rst;
	unsigned int dwc3_hwparams1;

	revision = readl(&dwc3_reg->g_snpsid);
	/* This should read as U3 followed by revision number */
	if ((revision & DWC3_GSNPSID_MASK) != 0x55330000) {
		puts("this is not a DesignWare USB3 DRD Core\n");
		return -EINVAL;
	}

	/* issue device SoftReset too */
	writel(DWC3_DCTL_CSFTRST, &dwc3_reg->d_ctl);

	t_rst = get_timer(0);
	do {
		val = readl(&dwc3_reg->d_ctl);
		if (!(val & DWC3_DCTL_CSFTRST))
			break;
		WATCHDOG_RESET();
	} while (get_timer(t_rst) < 500);

	if (val & DWC3_DCTL_CSFTRST) {
		debug("Reset timed out\n");
		return -2;
	}

	dwc3_core_soft_reset(dwc3_reg);

	dwc3_hwparams1 = readl(&dwc3_reg->g_hwparams1);

	val = readl(&dwc3_reg->g_ctl);
	val &= ~DWC3_GCTL_SCALEDOWN_MASK;
	val &= ~DWC3_GCTL_DISSCRAMBLE;
	switch (DWC3_GHWPARAMS1_EN_PWROPT(dwc3_hwparams1)) {
	case DWC3_GHWPARAMS1_EN_PWROPT_CLK:
		val &= ~DWC3_GCTL_DSBLCLKGTNG;
		break;
	default:
		printf("No power optimization available\n");
	}

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * where the device can fail to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter a Connect/Disconnect loop
	 */
	if ((revision & DWC3_REVISION_MASK) < 0x190a)
		val |= DWC3_GCTL_U2RSTECN;

	writel(val, &dwc3_reg->g_ctl);

	return 0;
}

static int keystone_xhci_core_init(struct dwc3 *dwc3_reg)
{
	int ret;

	ret = dwc3_core_init(dwc3_reg);
	if (ret) {
		debug("failed to initialize core\n");
		return -EINVAL;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return 0;
}

int xhci_hcd_init(int index,
		  struct xhci_hccr **ret_hccr, struct xhci_hcor **ret_hcor)
{
	u32 val;
	int ret;
	struct xhci_hccr *hcd;
	struct xhci_hcor *hcor;
	struct kdwc3_irq_regs *usbss;
	struct keystone_xhci_phy *phy;

	usbss = (struct kdwc3_irq_regs *)CONFIG_USB_SS_BASE;
	phy = (struct keystone_xhci_phy *)CONFIG_DEV_USB_PHY_BASE;

	/* Enable the PHY REFCLK clock gate with phy_ref_ssp_en = 1 */
	val = readl(&(phy->phy_clock));
	val |= USB3_PHY_REF_SSP_EN;
	writel(val, &phy->phy_clock);

	mdelay(100);

	/* Release USB from reset */
	ret = psc_enable_module(KS2_LPSC_USB);
	if (ret) {
		puts("Cannot enable USB module");
		return -1;
	}

	mdelay(100);

	/* Initialize usb phy */
	keystone_xhci_phy_set(phy);

	/* soft reset usbss */
	writel(1, &usbss->sysconfig);
	while (readl(&usbss->sysconfig) & 1)
		;

	val = readl(&usbss->revision);
	debug("usbss revision %x\n", val);

	/* Initialize usb core */
	hcd = (struct xhci_hccr *)CONFIG_USB_HOST_XHCI_BASE;
	keystone.dwc3_reg = (struct dwc3 *)(CONFIG_USB_HOST_XHCI_BASE +
					    DWC3_REG_OFFSET);

	keystone_xhci_core_init(keystone.dwc3_reg);

	/* set register addresses */
	hcor = (struct xhci_hcor *)((uint32_t)hcd +
		HC_LENGTH(readl(&hcd->cr_capbase)));

	debug("Keystone2-xhci: init hccr %08x and hcor %08x hc_length %d\n",
	      (u32)hcd, (u32)hcor,
	      (u32)HC_LENGTH(xhci_readl(&hcd->cr_capbase)));

	keystone.usbss = usbss;
	keystone.phy = phy;
	keystone.hcd = hcd;
	keystone.hcor = hcor;

	*ret_hccr = hcd;
	*ret_hcor = hcor;

	return 0;
}

static int keystone_xhci_phy_suspend(void)
{
	int loop_cnt = 0;
	struct xhci_hcor *hcor;
	uint32_t *portsc_1 = NULL;
	uint32_t *portsc_2 = NULL;
	u32 val, usb2_pls, usb3_pls, event_q;
	struct dwc3 *dwc3_reg = keystone.dwc3_reg;

	/* set register addresses */
	hcor = keystone.hcor;

	/* Bypass Scrambling and Set Shorter Training sequence for simulation */
	val = DWC3_GCTL_PWRDNSCALE(0x4b0) | DWC3_GCTL_PRTCAPDIR(0x2);
	writel(val, &dwc3_reg->g_ctl);

	/* GUSB2PHYCFG */
	val = readl(&dwc3_reg->g_usb2phycfg[0]);

	/* assert bit 6 (SusPhy) */
	val |= DWC3_GUSB2PHYCFG_SUSPHY;
	writel(val, &dwc3_reg->g_usb2phycfg[0]);

	/* GUSB3PIPECTL */
	val = readl(&dwc3_reg->g_usb3pipectl[0]);

	/*
	 * assert bit 29 to allow PHY to go to suspend when idle
	 * and cause the USB3 SS PHY to enter suspend mode
	 */
	val |= (BIT(29) | DWC3_GUSB3PIPECTL_SUSPHY);
	writel(val, &dwc3_reg->g_usb3pipectl[0]);

	/*
	 * Steps necessary to allow controller to suspend even when
	 * VBUS is HIGH:
	 * - Init DCFG[2:0] (DevSpd) to: 1=FS
	 * - Init GEVNTADR0 to point to an eventQ
	 * - Init GEVNTSIZ0 to 0x0100 to specify the size of the eventQ
	 * - Init DCTL::Run_nStop = 1
	 */
	writel(0x00020001, &dwc3_reg->d_cfg);
	/* TODO: local2global( (Uint32) eventQ )? */
	writel((u32)&event_q, &dwc3_reg->g_evnt_buf[0].g_evntadrlo);
	writel(0, &dwc3_reg->g_evnt_buf[0].g_evntadrhi);
	writel(0x4, &dwc3_reg->g_evnt_buf[0].g_evntsiz);
	/* Run */
	writel(DWC3_DCTL_RUN_STOP, &dwc3_reg->d_ctl);

	mdelay(100);

	/* Wait for USB2 & USB3 PORTSC::PortLinkState to indicate suspend */
	portsc_1 = (uint32_t *)(&hcor->portregs[0].or_portsc);
	portsc_2 = (uint32_t *)(&hcor->portregs[1].or_portsc);
	usb2_pls = 0;
	usb3_pls = 0;
	do {
		++loop_cnt;
		usb2_pls = (readl(portsc_1) & PORT_PLS_MASK) >> 5;
		usb3_pls = (readl(portsc_2) & PORT_PLS_MASK) >> 5;
	} while (((usb2_pls != 0x4) || (usb3_pls != 0x4)) && loop_cnt < 1000);

	if (usb2_pls != 0x4 || usb3_pls != 0x4) {
		debug("USB suspend failed - PLS USB2=%02x, USB3=%02x\n",
		      usb2_pls, usb3_pls);
		return -1;
	}

	debug("USB2 and USB3 PLS - Disabled, loop_cnt=%d\n", loop_cnt);
	return 0;
}

void xhci_hcd_stop(int index)
{
	/* Disable USB */
	if (keystone_xhci_phy_suspend())
		return;

	if (psc_disable_module(KS2_LPSC_USB)) {
		debug("PSC disable module USB failed!\n");
		return;
	}

	/* Disable PHY */
	keystone_xhci_phy_unset(keystone.phy);

/*	memset(&keystone, 0, sizeof(struct keystone_xhci)); */
	debug("xhci_hcd_stop OK.\n");
}
