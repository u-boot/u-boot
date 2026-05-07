// SPDX-License-Identifier: GPL-2.0
/*
 * core.c - DesignWare USB3 DRD Controller Core file
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - https://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 */

#include <clk.h>
#include <cpu_func.h>
#include <malloc.h>
#include <dwc3-uboot.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/bitfield.h>
#include <linux/math64.h>
#include <linux/time.h>

#include <version.h>

#include "core.h"
#include "gadget.h"
#include "glue.h"
#include "io.h"

#include "../host/xhci-ext-caps.h"

#define msleep(a) udelay(a * 1000)

#define DWC3_DEFAULT_AUTOSUSPEND_DELAY	5000 /* ms */

static LIST_HEAD(dwc3_list);

void dwc3_enable_susphy(struct dwc3 *dwc, bool enable)
{
	u32 reg;
	int i;

	for (i = 0; i < dwc->num_usb3_ports; i++) {
		reg = dwc3_readl(dwc->regs, DWC3_GUSB3PIPECTL(i));
		if (enable && !dwc->dis_u3_susphy_quirk)
			reg |= DWC3_GUSB3PIPECTL_SUSPHY;
		else
			reg &= ~DWC3_GUSB3PIPECTL_SUSPHY;

		dwc3_writel(dwc->regs, DWC3_GUSB3PIPECTL(i), reg);
	}

	for (i = 0; i < dwc->num_usb2_ports; i++) {
		reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(i));
		if (enable && !dwc->dis_u2_susphy_quirk)
			reg |= DWC3_GUSB2PHYCFG_SUSPHY;
		else
			reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;

		dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(i), reg);
	}
}

void dwc3_set_prtcap(struct dwc3 *dwc, u32 mode, bool ignore_susphy)
{
	unsigned int hw_mode;
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GCTL);

	 /*
	  * For DRD controllers, GUSB3PIPECTL.SUSPENDENABLE and
	  * GUSB2PHYCFG.SUSPHY should be cleared during mode switching,
	  * and they can be set after core initialization.
	  */
	hw_mode = DWC3_GHWPARAMS0_MODE(dwc->hwparams.hwparams0);
	if (hw_mode == DWC3_GHWPARAMS0_MODE_DRD && !ignore_susphy) {
		if (DWC3_GCTL_PRTCAP(reg) != mode)
			dwc3_enable_susphy(dwc, false);
	}

	reg &= ~(DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG));
	reg |= DWC3_GCTL_PRTCAPDIR(mode);
	dwc3_writel(dwc->regs, DWC3_GCTL, reg);

	dwc->current_dr_role = mode;
}

u32 dwc3_core_fifo_space(struct dwc3_ep *dep, u8 type)
{
	struct dwc3		*dwc = dep->dwc;
	u32			reg;

	dwc3_writel(dwc->regs, DWC3_GDBGFIFOSPACE,
			DWC3_GDBGFIFOSPACE_NUM(dep->number) |
			DWC3_GDBGFIFOSPACE_TYPE(type));

	reg = dwc3_readl(dwc->regs, DWC3_GDBGFIFOSPACE);

	return DWC3_GDBGFIFOSPACE_SPACE_AVAILABLE(reg);
}

/**
 * dwc3_core_soft_reset - Issues core soft reset and PHY reset
 * @dwc: pointer to our context structure
 */
int dwc3_core_soft_reset(struct dwc3 *dwc)
{
	u32		reg;
	int		retries = 1000;

	/*
	 * We're resetting only the device side because, if we're in host mode,
	 * XHCI driver will reset the host block. If dwc3 was configured for
	 * host-only mode, then we can return early.
	 */
	if (dwc->current_dr_role == DWC3_GCTL_PRTCAP_HOST)
		return 0;

	reg = dwc3_readl(dwc->regs, DWC3_DCTL);
	reg |= DWC3_DCTL_CSFTRST;
	reg &= ~DWC3_DCTL_RUN_STOP;
	dwc3_gadget_dctl_write_safe(dwc, reg);

	/*
	 * For DWC_usb31 controller 1.90a and later, the DCTL.CSFRST bit
	 * is cleared only after all the clocks are synchronized. This can
	 * take a little more than 50ms. Set the polling rate at 20ms
	 * for 10 times instead.
	 */
	if (DWC3_VER_IS_WITHIN(DWC31, 190A, ANY) || DWC3_IP_IS(DWC32))
		retries = 10;

	do {
		reg = dwc3_readl(dwc->regs, DWC3_DCTL);
		if (!(reg & DWC3_DCTL_CSFTRST))
			goto done;

		if (DWC3_VER_IS_WITHIN(DWC31, 190A, ANY) || DWC3_IP_IS(DWC32))
			msleep(20);
		else
			udelay(1);
	} while (--retries);

	dev_warn(dwc->dev, "DWC3 controller soft reset failed.\n");
	return -ETIMEDOUT;

done:
	/*
	 * For DWC_usb31 controller 1.80a and prior, once DCTL.CSFRST bit
	 * is cleared, we must wait at least 50ms before accessing the PHY
	 * domain (synchronization delay).
	 */
	if (DWC3_VER_IS_WITHIN(DWC31, ANY, 180A))
		msleep(50);

	return 0;
}

/*
 * dwc3_frame_length_adjustment - Adjusts frame length if required
 * @dwc3: Pointer to our controller context structure
 */
static void dwc3_frame_length_adjustment(struct dwc3 *dwc)
{
	u32 reg;
	u32 dft;

	if (DWC3_VER_IS_PRIOR(DWC3, 250A))
		return;

	if (dwc->fladj == 0)
		return;

	reg = dwc3_readl(dwc->regs, DWC3_GFLADJ);
	dft = reg & DWC3_GFLADJ_30MHZ_MASK;
	if (dft != dwc->fladj) {
		reg &= ~DWC3_GFLADJ_30MHZ_MASK;
		reg |= DWC3_GFLADJ_30MHZ_SDBND_SEL | dwc->fladj;
		dwc3_writel(dwc->regs, DWC3_GFLADJ, reg);
	}
}

/**
 * dwc3_ref_clk_period - Reference clock period configuration
 *		Default reference clock period depends on hardware
 *		configuration. For systems with reference clock that differs
 *		from the default, this will set clock period in DWC3_GUCTL
 *		register.
 * @dwc: Pointer to our controller context structure
 */
static void dwc3_ref_clk_period(struct dwc3 *dwc)
{
	unsigned long period;
	unsigned long fladj;
	unsigned long decr;
	unsigned long rate;
	u32 reg;

	if (dwc->ref_clk) {
		rate = clk_get_rate(dwc->ref_clk);
		if (!rate)
			return;
		period = NSEC_PER_SEC / rate;
	} else if (dwc->ref_clk_per) {
		period = dwc->ref_clk_per;
		rate = NSEC_PER_SEC / period;
	} else {
		return;
	}

	reg = dwc3_readl(dwc->regs, DWC3_GUCTL);
	reg &= ~DWC3_GUCTL_REFCLKPER_MASK;
	reg |=  FIELD_PREP(DWC3_GUCTL_REFCLKPER_MASK, period);
	dwc3_writel(dwc->regs, DWC3_GUCTL, reg);

	if (DWC3_VER_IS_PRIOR(DWC3, 250A))
		return;

	/*
	 * The calculation below is
	 *
	 * 125000 * (NSEC_PER_SEC / (rate * period) - 1)
	 *
	 * but rearranged for fixed-point arithmetic. The division must be
	 * 64-bit because 125000 * NSEC_PER_SEC doesn't fit in 32 bits (and
	 * neither does rate * period).
	 *
	 * Note that rate * period ~= NSEC_PER_SECOND, minus the number of
	 * nanoseconds of error caused by the truncation which happened during
	 * the division when calculating rate or period (whichever one was
	 * derived from the other). We first calculate the relative error, then
	 * scale it to units of 8 ppm.
	 */
	fladj = div64_u64(125000ULL * NSEC_PER_SEC, (u64)rate * period);
	fladj -= 125000;

	/*
	 * The documented 240MHz constant is scaled by 2 to get PLS1 as well.
	 */
	decr = 480000000 / rate;

	reg = dwc3_readl(dwc->regs, DWC3_GFLADJ);
	reg &= ~DWC3_GFLADJ_REFCLK_FLADJ_MASK
	    &  ~DWC3_GFLADJ_240MHZDECR
	    &  ~DWC3_GFLADJ_240MHZDECR_PLS1;
	reg |= FIELD_PREP(DWC3_GFLADJ_REFCLK_FLADJ_MASK, fladj)
	    |  FIELD_PREP(DWC3_GFLADJ_240MHZDECR, decr >> 1)
	    |  FIELD_PREP(DWC3_GFLADJ_240MHZDECR_PLS1, decr & 1);

	if (dwc->gfladj_refclk_lpm_sel)
		reg |=  DWC3_GFLADJ_REFCLK_LPM_SEL;

	dwc3_writel(dwc->regs, DWC3_GFLADJ, reg);
}

/**
 * dwc3_free_one_event_buffer - Frees one event buffer
 * @dwc: Pointer to our controller context structure
 * @evt: Pointer to event buffer to be freed
 */
static void dwc3_free_one_event_buffer(struct dwc3 *dwc,
		struct dwc3_event_buffer *evt)
{
	dma_free_coherent(evt->buf);
}

/**
 * dwc3_alloc_one_event_buffer - Allocates one event buffer structure
 * @dwc: Pointer to our controller context structure
 * @length: size of the event buffer
 *
 * Returns a pointer to the allocated event buffer structure on success
 * otherwise ERR_PTR(errno).
 */
static struct dwc3_event_buffer *dwc3_alloc_one_event_buffer(struct dwc3 *dwc,
		unsigned int length)
{
	struct dwc3_event_buffer	*evt;

	evt = devm_kzalloc((struct udevice*)dwc->dev, sizeof(*evt), GFP_KERNEL);
	if (!evt)
		return ERR_PTR(-ENOMEM);

	evt->dwc	= dwc;
	evt->length	= length;
	evt->cache	= devm_kzalloc((struct udevice *)dwc->dev, length,
				       GFP_KERNEL);
	if (!evt->cache)
		return ERR_PTR(-ENOMEM);

	evt->buf	= dma_alloc_coherent(length,
					     (unsigned long *)&evt->dma);
	if (!evt->buf)
		return ERR_PTR(-ENOMEM);

	return evt;
}

/**
 * dwc3_free_event_buffers - frees all allocated event buffers
 * @dwc: Pointer to our controller context structure
 */
static void dwc3_free_event_buffers(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;

	evt = dwc->ev_buf;
	if (evt)
		dwc3_free_one_event_buffer(dwc, evt);
}

/**
 * dwc3_alloc_event_buffers - Allocates @num event buffers of size @length
 * @dwc: pointer to our controller context structure
 * @length: size of event buffer
 *
 * Returns 0 on success otherwise negative errno. In the error case, dwc
 * may contain some buffers allocated but not all which were requested.
 */
static int dwc3_alloc_event_buffers(struct dwc3 *dwc, unsigned int length)
{
	struct dwc3_event_buffer *evt;
	unsigned int hw_mode;

	hw_mode = DWC3_GHWPARAMS0_MODE(dwc->hwparams.hwparams0);
	if (hw_mode == DWC3_GHWPARAMS0_MODE_HOST) {
		dwc->ev_buf = NULL;
		return 0;
	}

	evt = dwc3_alloc_one_event_buffer(dwc, length);
	if (IS_ERR(evt)) {
		dev_err(dwc->dev, "can't allocate event buffer\n");
		return PTR_ERR(evt);
	}
	dwc->ev_buf = evt;

	return 0;
}

/**
 * dwc3_event_buffers_setup - setup our allocated event buffers
 * @dwc: pointer to our controller context structure
 *
 * Returns 0 on success otherwise negative errno.
 */
int dwc3_event_buffers_setup(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;
	u32				reg;

	if (!dwc->ev_buf)
		return 0;

	evt = dwc->ev_buf;
	evt->lpos = 0;
	dwc3_writel(dwc->regs, DWC3_GEVNTADRLO(0),
			lower_32_bits(evt->dma));
	dwc3_writel(dwc->regs, DWC3_GEVNTADRHI(0),
			upper_32_bits(evt->dma));
	dwc3_writel(dwc->regs, DWC3_GEVNTSIZ(0),
			DWC3_GEVNTSIZ_SIZE(evt->length));

	/* Clear any stale event */
	reg = dwc3_readl(dwc->regs, DWC3_GEVNTCOUNT(0));
	dwc3_writel(dwc->regs, DWC3_GEVNTCOUNT(0), reg);
	return 0;
}

void dwc3_event_buffers_cleanup(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;
	u32				reg;

	if (!dwc->ev_buf)
		return;
	/*
	 * Exynos platforms may not be able to access event buffer if the
	 * controller failed to halt on dwc3_core_exit().
	 */
	reg = dwc3_readl(dwc->regs, DWC3_DSTS);
	if (!(reg & DWC3_DSTS_DEVCTRLHLT))
		return;

	evt = dwc->ev_buf;

	evt->lpos = 0;

	dwc3_writel(dwc->regs, DWC3_GEVNTADRLO(0), 0);
	dwc3_writel(dwc->regs, DWC3_GEVNTADRHI(0), 0);
	dwc3_writel(dwc->regs, DWC3_GEVNTSIZ(0), DWC3_GEVNTSIZ_INTMASK
			| DWC3_GEVNTSIZ_SIZE(0));

	/* Clear any stale event */
	reg = dwc3_readl(dwc->regs, DWC3_GEVNTCOUNT(0));
	dwc3_writel(dwc->regs, DWC3_GEVNTCOUNT(0), reg);
}

static void dwc3_core_num_eps(struct dwc3 *dwc)
{
	struct dwc3_hwparams	*parms = &dwc->hwparams;

	dwc->num_eps = DWC3_NUM_EPS(parms);
}

static void dwc3_cache_hwparams(struct dwc3 *dwc)
{
	struct dwc3_hwparams	*parms = &dwc->hwparams;

	parms->hwparams0 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS0);
	parms->hwparams1 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS1);
	parms->hwparams2 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS2);
	parms->hwparams3 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS3);
	parms->hwparams4 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS4);
	parms->hwparams5 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS5);
	parms->hwparams6 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS6);
	parms->hwparams7 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS7);
	parms->hwparams8 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS8);

	if (DWC3_IP_IS(DWC32))
		parms->hwparams9 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS9);
}

static void dwc3_config_soc_bus(struct dwc3 *dwc)
{
	if (dwc->gsbuscfg0_reqinfo != DWC3_GSBUSCFG0_REQINFO_UNSPECIFIED) {
		u32 reg;

		reg = dwc3_readl(dwc->regs, DWC3_GSBUSCFG0);
		reg &= ~DWC3_GSBUSCFG0_REQINFO(~0);
		reg |= DWC3_GSBUSCFG0_REQINFO(dwc->gsbuscfg0_reqinfo);
		dwc3_writel(dwc->regs, DWC3_GSBUSCFG0, reg);
	}
}

static int dwc3_core_ulpi_init(struct dwc3 *dwc)
{
	int intf;
	int ret = 0;

	intf = DWC3_GHWPARAMS3_HSPHY_IFC(dwc->hwparams.hwparams3);

	if (intf == DWC3_GHWPARAMS3_HSPHY_IFC_ULPI ||
	    (intf == DWC3_GHWPARAMS3_HSPHY_IFC_UTMI_ULPI &&
	     dwc->hsphy_interface &&
	     !strncmp(dwc->hsphy_interface, "ulpi", 4)))
		ret = dwc3_ulpi_init(dwc);

	return ret;
}

static int dwc3_ss_phy_setup(struct dwc3 *dwc, int index)
{
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GUSB3PIPECTL(index));

	/*
	 * Make sure UX_EXIT_PX is cleared as that causes issues with some
	 * PHYs. Also, this bit is not supposed to be used in normal operation.
	 */
	reg &= ~DWC3_GUSB3PIPECTL_UX_EXIT_PX;

	/* Ensure the GUSB3PIPECTL.SUSPENDENABLE is cleared prior to phy init. */
	reg &= ~DWC3_GUSB3PIPECTL_SUSPHY;

	if (dwc->u2ss_inp3_quirk)
		reg |= DWC3_GUSB3PIPECTL_U2SSINP3OK;

	if (dwc->dis_rxdet_inp3_quirk)
		reg |= DWC3_GUSB3PIPECTL_DISRXDETINP3;

	if (dwc->req_p1p2p3_quirk)
		reg |= DWC3_GUSB3PIPECTL_REQP1P2P3;

	if (dwc->del_p1p2p3_quirk)
		reg |= DWC3_GUSB3PIPECTL_DEP1P2P3_EN;

	if (dwc->del_phy_power_chg_quirk)
		reg |= DWC3_GUSB3PIPECTL_DEPOCHANGE;

	if (dwc->lfps_filter_quirk)
		reg |= DWC3_GUSB3PIPECTL_LFPSFILT;

	if (dwc->rx_detect_poll_quirk)
		reg |= DWC3_GUSB3PIPECTL_RX_DETOPOLL;

	if (dwc->tx_de_emphasis_quirk)
		reg |= DWC3_GUSB3PIPECTL_TX_DEEPH(dwc->tx_de_emphasis);

	if (dwc->dis_del_phy_power_chg_quirk)
		reg &= ~DWC3_GUSB3PIPECTL_DEPOCHANGE;

	dwc3_writel(dwc->regs, DWC3_GUSB3PIPECTL(index), reg);

	return 0;
}

static int dwc3_hs_phy_setup(struct dwc3 *dwc, int index)
{
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(index));

	/* Select the HS PHY interface */
	switch (DWC3_GHWPARAMS3_HSPHY_IFC(dwc->hwparams.hwparams3)) {
	case DWC3_GHWPARAMS3_HSPHY_IFC_UTMI_ULPI:
		if (dwc->hsphy_interface &&
				!strncmp(dwc->hsphy_interface, "utmi", 4)) {
			reg &= ~DWC3_GUSB2PHYCFG_ULPI_UTMI;
			break;
		} else if (dwc->hsphy_interface &&
				!strncmp(dwc->hsphy_interface, "ulpi", 4)) {
			reg |= DWC3_GUSB2PHYCFG_ULPI_UTMI;
			dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(index), reg);
		} else {
			/* Relying on default value. */
			if (!(reg & DWC3_GUSB2PHYCFG_ULPI_UTMI))
				break;
		}
		fallthrough;
	case DWC3_GHWPARAMS3_HSPHY_IFC_ULPI:
	default:
		break;
	}

	switch (dwc->hsphy_mode) {
	case USBPHY_INTERFACE_MODE_UTMI:
		reg &= ~(DWC3_GUSB2PHYCFG_PHYIF_MASK |
		       DWC3_GUSB2PHYCFG_USBTRDTIM_MASK);
		reg |= DWC3_GUSB2PHYCFG_PHYIF(UTMI_PHYIF_8_BIT) |
		       DWC3_GUSB2PHYCFG_USBTRDTIM(USBTRDTIM_UTMI_8_BIT);
		break;
	case USBPHY_INTERFACE_MODE_UTMIW:
		reg &= ~(DWC3_GUSB2PHYCFG_PHYIF_MASK |
		       DWC3_GUSB2PHYCFG_USBTRDTIM_MASK);
		reg |= DWC3_GUSB2PHYCFG_PHYIF(UTMI_PHYIF_16_BIT) |
		       DWC3_GUSB2PHYCFG_USBTRDTIM(USBTRDTIM_UTMI_16_BIT);
		break;
	default:
		break;
	}

	/* Ensure the GUSB2PHYCFG.SUSPHY is cleared prior to phy init. */
	reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;

	if (dwc->dis_enblslpm_quirk)
		reg &= ~DWC3_GUSB2PHYCFG_ENBLSLPM;
	else
		reg |= DWC3_GUSB2PHYCFG_ENBLSLPM;

	if (dwc->dis_u2_freeclk_exists_quirk || dwc->gfladj_refclk_lpm_sel)
		reg &= ~DWC3_GUSB2PHYCFG_U2_FREECLK_EXISTS;

	/*
	 * Some ULPI USB PHY does not support internal VBUS supply, to drive
	 * the CPEN pin requires the configuration of the ULPI DRVVBUSEXTERNAL
	 * bit of OTG_CTRL register. Controller configures the USB2 PHY
	 * ULPIEXTVBUSDRV bit[17] of the GUSB2PHYCFG register to drive vBus
	 * with an external supply.
	 */
	if (dwc->ulpi_ext_vbus_drv)
		reg |= DWC3_GUSB2PHYCFG_ULPIEXTVBUSDRV;

	dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(index), reg);

	return 0;
}

/**
 * dwc3_phy_setup - Configure USB PHY Interface of DWC3 Core
 * @dwc: Pointer to our controller context structure
 *
 * Returns 0 on success. The USB PHY interfaces are configured but not
 * initialized. The PHY interfaces and the PHYs get initialized together with
 * the core in dwc3_core_init.
 */
static int dwc3_phy_setup(struct dwc3 *dwc)
{
	int i;
	int ret;

	for (i = 0; i < dwc->num_usb3_ports; i++) {
		ret = dwc3_ss_phy_setup(dwc, i);
		if (ret)
			return ret;
	}

	for (i = 0; i < dwc->num_usb2_ports; i++) {
		ret = dwc3_hs_phy_setup(dwc, i);
		if (ret)
			return ret;
	}

	return 0;
}

static int dwc3_phy_init(struct dwc3 *dwc)
{
	int ret;
	int i;
	int j;

	usb_phy_init(dwc->usb2_phy);
	usb_phy_init(dwc->usb3_phy);

	for (i = 0; i < dwc->num_usb2_ports; i++) {
		ret = generic_phy_init(dwc->usb2_generic_phy[i]);
		if (ret < 0)
			goto err_exit_usb2_phy;
	}

	for (j = 0; j < dwc->num_usb3_ports; j++) {
		ret = generic_phy_init(dwc->usb3_generic_phy[j]);
		if (ret < 0)
			goto err_exit_usb3_phy;
	}

	/*
	 * Above DWC_usb3.0 1.94a, it is recommended to set
	 * DWC3_GUSB3PIPECTL_SUSPHY and DWC3_GUSB2PHYCFG_SUSPHY to '0' during
	 * coreConsultant configuration. So default value will be '0' when the
	 * core is reset. Application needs to set it to '1' after the core
	 * initialization is completed.
	 *
	 * Certain phy requires to be in P0 power state during initialization.
	 * Make sure GUSB3PIPECTL.SUSPENDENABLE and GUSB2PHYCFG.SUSPHY are clear
	 * prior to phy init to maintain in the P0 state.
	 *
	 * After phy initialization, some phy operations can only be executed
	 * while in lower P states. Ensure GUSB3PIPECTL.SUSPENDENABLE and
	 * GUSB2PHYCFG.SUSPHY are set soon after initialization to avoid
	 * blocking phy ops.
	 */
	if (!DWC3_VER_IS_WITHIN(DWC3, ANY, 194A))
		dwc3_enable_susphy(dwc, true);

	return 0;

err_exit_usb3_phy:
	while (--j >= 0)
		generic_phy_exit(dwc->usb3_generic_phy[j]);

err_exit_usb2_phy:
	while (--i >= 0)
		generic_phy_exit(dwc->usb2_generic_phy[i]);

	usb_phy_shutdown(dwc->usb3_phy);
	usb_phy_shutdown(dwc->usb2_phy);

	return ret;
}

static void dwc3_phy_exit(struct dwc3 *dwc)
{
	int i;

	for (i = 0; i < dwc->num_usb3_ports; i++)
		generic_phy_exit(dwc->usb3_generic_phy[i]);

	for (i = 0; i < dwc->num_usb2_ports; i++)
		generic_phy_exit(dwc->usb2_generic_phy[i]);

	usb_phy_shutdown(dwc->usb3_phy);
	usb_phy_shutdown(dwc->usb2_phy);
}

static int dwc3_phy_power_on(struct dwc3 *dwc)
{
	int ret;
	int i;
	int j;

	usb_phy_set_suspend(dwc->usb2_phy, 0);
	usb_phy_set_suspend(dwc->usb3_phy, 0);

	for (i = 0; i < dwc->num_usb2_ports; i++) {
		ret = generic_phy_power_on(dwc->usb2_generic_phy[i]);
		if (ret < 0)
			goto err_power_off_usb2_phy;
	}

	for (j = 0; j < dwc->num_usb3_ports; j++) {
		ret = generic_phy_power_on(dwc->usb3_generic_phy[j]);
		if (ret < 0)
			goto err_power_off_usb3_phy;
	}

	return 0;

err_power_off_usb3_phy:
	while (--j >= 0)
		generic_phy_power_off(dwc->usb3_generic_phy[j]);

err_power_off_usb2_phy:
	while (--i >= 0)
		generic_phy_power_off(dwc->usb2_generic_phy[i]);

	usb_phy_set_suspend(dwc->usb3_phy, 1);
	usb_phy_set_suspend(dwc->usb2_phy, 1);

	return ret;
}

static void dwc3_phy_power_off(struct dwc3 *dwc)
{
	int i;

	for (i = 0; i < dwc->num_usb3_ports; i++)
		generic_phy_power_off(dwc->usb3_generic_phy[i]);

	for (i = 0; i < dwc->num_usb2_ports; i++)
		generic_phy_power_off(dwc->usb2_generic_phy[i]);

	usb_phy_set_suspend(dwc->usb3_phy, 1);
	usb_phy_set_suspend(dwc->usb2_phy, 1);
}

static void dwc3_clk_disable(struct dwc3 *dwc)
{
	clk_disable_unprepare(dwc->pipe_clk);
	clk_disable_unprepare(dwc->utmi_clk);
	clk_disable_unprepare(dwc->susp_clk);
	clk_disable_unprepare(dwc->ref_clk);
	clk_disable_unprepare(dwc->bus_clk);
}

static void dwc3_core_exit(struct dwc3 *dwc)
{
	dwc3_event_buffers_cleanup(dwc);
	dwc3_phy_power_off(dwc);
	dwc3_phy_exit(dwc);
	dwc3_clk_disable(dwc);
	reset_control_assert(dwc->reset);
}

static void dwc3_core_setup_global_control(struct dwc3 *dwc)
{
	unsigned int power_opt;
	unsigned int hw_mode;
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;
	hw_mode = DWC3_GHWPARAMS0_MODE(dwc->hwparams.hwparams0);
	power_opt = DWC3_GHWPARAMS1_EN_PWROPT(dwc->hwparams.hwparams1);

	switch (power_opt) {
	case DWC3_GHWPARAMS1_EN_PWROPT_CLK:
		/**
		 * WORKAROUND: DWC3 revisions between 2.10a and 2.50a have an
		 * issue which would cause xHCI compliance tests to fail.
		 *
		 * Because of that we cannot enable clock gating on such
		 * configurations.
		 *
		 * Refers to:
		 *
		 * STAR#9000588375: Clock Gating, SOF Issues when ref_clk-Based
		 * SOF/ITP Mode Used
		 */
		if ((dwc->dr_mode == USB_DR_MODE_HOST ||
				dwc->dr_mode == USB_DR_MODE_OTG) &&
				DWC3_VER_IS_WITHIN(DWC3, 210A, 250A))
			reg |= DWC3_GCTL_DSBLCLKGTNG | DWC3_GCTL_SOFITPSYNC;
		else
			reg &= ~DWC3_GCTL_DSBLCLKGTNG;
		break;
	case DWC3_GHWPARAMS1_EN_PWROPT_HIB:
		/*
		 * REVISIT Enabling this bit so that host-mode hibernation
		 * will work. Device-mode hibernation is not yet implemented.
		 */
		reg |= DWC3_GCTL_GBLHIBERNATIONEN;
		break;
	default:
		/* nothing */
		break;
	}

	/*
	 * This is a workaround for STAR#4846132, which only affects
	 * DWC_usb31 version2.00a operating in host mode.
	 *
	 * There is a problem in DWC_usb31 version 2.00a operating
	 * in host mode that would cause a CSR read timeout When CSR
	 * read coincides with RAM Clock Gating Entry. By disable
	 * Clock Gating, sacrificing power consumption for normal
	 * operation.
	 */
	if (power_opt != DWC3_GHWPARAMS1_EN_PWROPT_NO &&
	    hw_mode != DWC3_GHWPARAMS0_MODE_GADGET && DWC3_VER_IS(DWC31, 200A))
		reg |= DWC3_GCTL_DSBLCLKGTNG;

	/* check if current dwc3 is on simulation board */
	if (dwc->hwparams.hwparams6 & DWC3_GHWPARAMS6_EN_FPGA) {
		dev_info(dwc->dev, "Running with FPGA optimizations\n");
		dwc->is_fpga = true;
	}

	WARN_ONCE(dwc->disable_scramble_quirk && !dwc->is_fpga,
			"disable_scramble cannot be used on non-FPGA builds\n");

	if (dwc->disable_scramble_quirk && dwc->is_fpga)
		reg |= DWC3_GCTL_DISSCRAMBLE;
	else
		reg &= ~DWC3_GCTL_DISSCRAMBLE;

	if (dwc->u2exit_lfps_quirk)
		reg |= DWC3_GCTL_U2EXIT_LFPS;

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * where the device can fail to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter a Connect/Disconnect loop
	 */
	if (DWC3_VER_IS_PRIOR(DWC3, 190A))
		reg |= DWC3_GCTL_U2RSTECN;

	dwc3_writel(dwc->regs, DWC3_GCTL, reg);
}

static int dwc3_core_ulpi_init(struct dwc3 *dwc);

/* set global incr burst type configuration registers */
static void dwc3_set_incr_burst_type(struct dwc3 *dwc)
{
	struct udevice *dev = dwc->dev;
	/* incrx_mode : for INCR burst type. */
	bool incrx_mode;
	/* incrx_size : for size of INCRX burst. */
	u32 incrx_size;
	u32 *vals;
	u32 cfg;
	int ntype;
	int ret;
	int i;

	cfg = dwc3_readl(dwc->regs, DWC3_GSBUSCFG0);

	/*
	 * Handle property "snps,incr-burst-type-adjustment".
	 * Get the number of value from this property:
	 * result <= 0, means this property is not supported.
	 * result = 1, means INCRx burst mode supported.
	 * result > 1, means undefined length burst mode supported.
	 */
	ntype = device_property_count_u32(dev, "snps,incr-burst-type-adjustment");
	if (ntype <= 0)
		return;

	vals = kcalloc(ntype, sizeof(u32), GFP_KERNEL);
	if (!vals)
		return;

	/* Get INCR burst type, and parse it */
	ret = device_property_read_u32_array(dev,
			"snps,incr-burst-type-adjustment", vals, ntype);
	if (ret) {
		kfree(vals);
		dev_err(dev, "Error to get property\n");
		return;
	}

	incrx_size = *vals;

	if (ntype > 1) {
		/* INCRX (undefined length) burst mode */
		incrx_mode = INCRX_UNDEF_LENGTH_BURST_MODE;
		for (i = 1; i < ntype; i++) {
			if (vals[i] > incrx_size)
				incrx_size = vals[i];
		}
	} else {
		/* INCRX burst mode */
		incrx_mode = INCRX_BURST_MODE;
	}

	kfree(vals);

	/* Enable Undefined Length INCR Burst and Enable INCRx Burst */
	cfg &= ~DWC3_GSBUSCFG0_INCRBRST_MASK;
	if (incrx_mode)
		cfg |= DWC3_GSBUSCFG0_INCRBRSTENA;
	switch (incrx_size) {
	case 256:
		cfg |= DWC3_GSBUSCFG0_INCR256BRSTENA;
		break;
	case 128:
		cfg |= DWC3_GSBUSCFG0_INCR128BRSTENA;
		break;
	case 64:
		cfg |= DWC3_GSBUSCFG0_INCR64BRSTENA;
		break;
	case 32:
		cfg |= DWC3_GSBUSCFG0_INCR32BRSTENA;
		break;
	case 16:
		cfg |= DWC3_GSBUSCFG0_INCR16BRSTENA;
		break;
	case 8:
		cfg |= DWC3_GSBUSCFG0_INCR8BRSTENA;
		break;
	case 4:
		cfg |= DWC3_GSBUSCFG0_INCR4BRSTENA;
		break;
	case 1:
		break;
	default:
		dev_err(dev, "Invalid property\n");
		break;
	}

	dwc3_writel(dwc->regs, DWC3_GSBUSCFG0, cfg);
}

static void dwc3_set_power_down_clk_scale(struct dwc3 *dwc)
{
	u32 scale;
	u32 reg;

	if (!dwc->susp_clk)
		return;

	/*
	 * The power down scale field specifies how many suspend_clk
	 * periods fit into a 16KHz clock period. When performing
	 * the division, round up the remainder.
	 *
	 * The power down scale value is calculated using the fastest
	 * frequency of the suspend_clk. If it isn't fixed (but within
	 * the accuracy requirement), the driver may not know the max
	 * rate of the suspend_clk, so only update the power down scale
	 * if the default is less than the calculated value from
	 * clk_get_rate() or if the default is questionably high
	 * (3x or more) to be within the requirement.
	 */
	scale = DIV_ROUND_UP(clk_get_rate(dwc->susp_clk), 16000);
	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	if ((reg & DWC3_GCTL_PWRDNSCALE_MASK) < DWC3_GCTL_PWRDNSCALE(scale) ||
	    (reg & DWC3_GCTL_PWRDNSCALE_MASK) > DWC3_GCTL_PWRDNSCALE(scale*3)) {
		reg &= ~(DWC3_GCTL_PWRDNSCALE_MASK);
		reg |= DWC3_GCTL_PWRDNSCALE(scale);
		dwc3_writel(dwc->regs, DWC3_GCTL, reg);
	}
}

static void dwc3_config_threshold(struct dwc3 *dwc)
{
	u32 reg;
	u8 rx_thr_num;
	u8 rx_maxburst;
	u8 tx_thr_num;
	u8 tx_maxburst;

	/*
	 * Must config both number of packets and max burst settings to enable
	 * RX and/or TX threshold.
	 */
	if (!DWC3_IP_IS(DWC3) && dwc->dr_mode == USB_DR_MODE_HOST) {
		rx_thr_num = dwc->rx_thr_num_pkt_prd;
		rx_maxburst = dwc->rx_max_burst_prd;
		tx_thr_num = dwc->tx_thr_num_pkt_prd;
		tx_maxburst = dwc->tx_max_burst_prd;

		if (rx_thr_num && rx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GRXTHRCFG);
			reg |= DWC31_RXTHRNUMPKTSEL_PRD;

			reg &= ~DWC31_RXTHRNUMPKT_PRD(~0);
			reg |= DWC31_RXTHRNUMPKT_PRD(rx_thr_num);

			reg &= ~DWC31_MAXRXBURSTSIZE_PRD(~0);
			reg |= DWC31_MAXRXBURSTSIZE_PRD(rx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GRXTHRCFG, reg);
		}

		if (tx_thr_num && tx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GTXTHRCFG);
			reg |= DWC31_TXTHRNUMPKTSEL_PRD;

			reg &= ~DWC31_TXTHRNUMPKT_PRD(~0);
			reg |= DWC31_TXTHRNUMPKT_PRD(tx_thr_num);

			reg &= ~DWC31_MAXTXBURSTSIZE_PRD(~0);
			reg |= DWC31_MAXTXBURSTSIZE_PRD(tx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GTXTHRCFG, reg);
		}
	}

	rx_thr_num = dwc->rx_thr_num_pkt;
	rx_maxburst = dwc->rx_max_burst;
	tx_thr_num = dwc->tx_thr_num_pkt;
	tx_maxburst = dwc->tx_max_burst;

	if (DWC3_IP_IS(DWC3)) {
		if (rx_thr_num && rx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GRXTHRCFG);
			reg |= DWC3_GRXTHRCFG_PKTCNTSEL;

			reg &= ~DWC3_GRXTHRCFG_RXPKTCNT(~0);
			reg |= DWC3_GRXTHRCFG_RXPKTCNT(rx_thr_num);

			reg &= ~DWC3_GRXTHRCFG_MAXRXBURSTSIZE(~0);
			reg |= DWC3_GRXTHRCFG_MAXRXBURSTSIZE(rx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GRXTHRCFG, reg);
		}

		if (tx_thr_num && tx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GTXTHRCFG);
			reg |= DWC3_GTXTHRCFG_PKTCNTSEL;

			reg &= ~DWC3_GTXTHRCFG_TXPKTCNT(~0);
			reg |= DWC3_GTXTHRCFG_TXPKTCNT(tx_thr_num);

			reg &= ~DWC3_GTXTHRCFG_MAXTXBURSTSIZE(~0);
			reg |= DWC3_GTXTHRCFG_MAXTXBURSTSIZE(tx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GTXTHRCFG, reg);
		}
	} else {
		if (rx_thr_num && rx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GRXTHRCFG);
			reg |= DWC31_GRXTHRCFG_PKTCNTSEL;

			reg &= ~DWC31_GRXTHRCFG_RXPKTCNT(~0);
			reg |= DWC31_GRXTHRCFG_RXPKTCNT(rx_thr_num);

			reg &= ~DWC31_GRXTHRCFG_MAXRXBURSTSIZE(~0);
			reg |= DWC31_GRXTHRCFG_MAXRXBURSTSIZE(rx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GRXTHRCFG, reg);
		}

		if (tx_thr_num && tx_maxburst) {
			reg = dwc3_readl(dwc->regs, DWC3_GTXTHRCFG);
			reg |= DWC31_GTXTHRCFG_PKTCNTSEL;

			reg &= ~DWC31_GTXTHRCFG_TXPKTCNT(~0);
			reg |= DWC31_GTXTHRCFG_TXPKTCNT(tx_thr_num);

			reg &= ~DWC31_GTXTHRCFG_MAXTXBURSTSIZE(~0);
			reg |= DWC31_GTXTHRCFG_MAXTXBURSTSIZE(tx_maxburst);

			dwc3_writel(dwc->regs, DWC3_GTXTHRCFG, reg);
		}
	}
}

/**
 * dwc3_core_init - Low-level initialization of DWC3 Core
 * @dwc: Pointer to our controller context structure
 *
 * Returns 0 on success otherwise negative errno.
 */
static int dwc3_core_init(struct dwc3 *dwc)
{
	unsigned int		hw_mode;
	u32			reg;
	int			ret;

	hw_mode = DWC3_GHWPARAMS0_MODE(dwc->hwparams.hwparams0);

	/*
	 * Write Linux Version Code to our GUID register so it's easy to figure
	 * out which U-Boot version a bug was found.
	 */
	dwc3_writel(dwc->regs, DWC3_GUID, U_BOOT_VERSION_NUM);

	ret = dwc3_phy_setup(dwc);
	if (ret)
		return ret;

	if (!dwc->ulpi_ready) {
		ret = dwc3_core_ulpi_init(dwc);
		if (ret) {
			if (ret == -ETIMEDOUT) {
				dwc3_core_soft_reset(dwc);
				ret = -EPROBE_DEFER;
			}
			return ret;
		}
		dwc->ulpi_ready = true;
	}

	ret = dwc3_phy_init(dwc);
	if (ret)
		goto err_exit_ulpi;

	ret = dwc3_core_soft_reset(dwc);
	if (ret)
		goto err_exit_phy;

	dwc3_core_setup_global_control(dwc);
	dwc3_core_num_eps(dwc);

	/* Set power down scale of suspend_clk */
	dwc3_set_power_down_clk_scale(dwc);

	/* Adjust Frame Length */
	dwc3_frame_length_adjustment(dwc);

	/* Adjust Reference Clock Period */
	dwc3_ref_clk_period(dwc);

	dwc3_set_incr_burst_type(dwc);

	dwc3_config_soc_bus(dwc);

	ret = dwc3_phy_power_on(dwc);
	if (ret)
		goto err_exit_phy;

	ret = dwc3_event_buffers_setup(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to setup event buffers\n");
		goto err_power_off_phy;
	}

	/*
	 * ENDXFER polling is available on version 3.10a and later of
	 * the DWC_usb3 controller. It is NOT available in the
	 * DWC_usb31 controller.
	 */
	if (DWC3_VER_IS_WITHIN(DWC3, 310A, ANY)) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL2);
		reg |= DWC3_GUCTL2_RST_ACTBITLATER;
		dwc3_writel(dwc->regs, DWC3_GUCTL2, reg);
	}

	/*
	 * STAR 9001285599: This issue affects DWC_usb3 version 3.20a
	 * only. If the PM TIMER ECM is enabled through GUCTL2[19], the
	 * link compliance test (TD7.21) may fail. If the ECN is not
	 * enabled (GUCTL2[19] = 0), the controller will use the old timer
	 * value (5us), which is still acceptable for the link compliance
	 * test. Therefore, do not enable PM TIMER ECM in 3.20a by
	 * setting GUCTL2[19] by default; instead, use GUCTL2[19] = 0.
	 */
	if (DWC3_VER_IS(DWC3, 320A)) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL2);
		reg &= ~DWC3_GUCTL2_LC_TIMER;
		dwc3_writel(dwc->regs, DWC3_GUCTL2, reg);
	}

	/*
	 * When configured in HOST mode, after issuing U3/L2 exit controller
	 * fails to send proper CRC checksum in CRC5 field. Because of this
	 * behaviour Transaction Error is generated, resulting in reset and
	 * re-enumeration of usb device attached. All the termsel, xcvrsel,
	 * opmode becomes 0 during end of resume. Enabling bit 10 of GUCTL1
	 * will correct this problem. This option is to support certain
	 * legacy ULPI PHYs.
	 */
	if (dwc->resume_hs_terminations) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);
		reg |= DWC3_GUCTL1_RESUME_OPMODE_HS_HOST;
		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}

	if (!DWC3_VER_IS_PRIOR(DWC3, 250A)) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);

		/*
		 * Enable hardware control of sending remote wakeup
		 * in HS when the device is in the L1 state.
		 */
		if (!DWC3_VER_IS_PRIOR(DWC3, 290A))
			reg |= DWC3_GUCTL1_DEV_L1_EXIT_BY_HW;

		/*
		 * Decouple USB 2.0 L1 & L2 events which will allow for
		 * gadget driver to only receive U3/L2 suspend & wakeup
		 * events and prevent the more frequent L1 LPM transitions
		 * from interrupting the driver.
		 */
		if (!DWC3_VER_IS_PRIOR(DWC3, 300A))
			reg |= DWC3_GUCTL1_DEV_DECOUPLE_L1L2_EVT;

		if (dwc->dis_tx_ipgap_linecheck_quirk)
			reg |= DWC3_GUCTL1_TX_IPGAP_LINECHECK_DIS;

		if (dwc->parkmode_disable_ss_quirk)
			reg |= DWC3_GUCTL1_PARKMODE_DISABLE_SS;

		if (dwc->parkmode_disable_hs_quirk)
			reg |= DWC3_GUCTL1_PARKMODE_DISABLE_HS;

		if (DWC3_VER_IS_WITHIN(DWC3, 290A, ANY)) {
			if (dwc->maximum_speed == USB_SPEED_FULL ||
			    dwc->maximum_speed == USB_SPEED_HIGH)
				reg |= DWC3_GUCTL1_DEV_FORCE_20_CLK_FOR_30_CLK;
			else
				reg &= ~DWC3_GUCTL1_DEV_FORCE_20_CLK_FOR_30_CLK;
		}

		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}

	dwc3_config_threshold(dwc);

	/*
	 * Modify this for all supported Super Speed ports when
	 * multiport support is added.
	 */
	if (hw_mode != DWC3_GHWPARAMS0_MODE_GADGET &&
	    (DWC3_IP_IS(DWC31)) &&
	    dwc->maximum_speed == USB_SPEED_SUPER) {
		int i;

		for (i = 0; i < dwc->num_usb3_ports; i++) {
			reg = dwc3_readl(dwc->regs, DWC3_LLUCTL(i));
			reg |= DWC3_LLUCTL_FORCE_GEN1;
			dwc3_writel(dwc->regs, DWC3_LLUCTL(i), reg);
		}
	}

	/*
	 * STAR 9001346572: This issue affects DWC_usb31 versions 1.80a and
	 * prior. When an active endpoint not currently cached in the host
	 * controller is chosen to be cached to the same index as an endpoint
	 * receiving NAKs, the endpoint receiving NAKs enters continuous
	 * retry mode. This prevents it from being evicted from the host
	 * controller cache, blocking the new endpoint from being cached and
	 * serviced.
	 *
	 * To resolve this, for controller versions 1.70a and 1.80a, set the
	 * GUCTL3 bit[16] (USB2.0 Internal Retry Disable) to 1. This bit
	 * disables the USB2.0 internal retry feature. The GUCTL3[16] register
	 * function is available only from version 1.70a.
	 */
	if (DWC3_VER_IS_WITHIN(DWC31, 170A, 180A)) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL3);
		reg |= DWC3_GUCTL3_USB20_RETRY_DISABLE;
		dwc3_writel(dwc->regs, DWC3_GUCTL3, reg);
	}

	return 0;

err_power_off_phy:
	dwc3_phy_power_off(dwc);
err_exit_phy:
	dwc3_phy_exit(dwc);
err_exit_ulpi:
	dwc3_ulpi_exit(dwc);

	return ret;
}

int phy_set_mode_ext(struct phy *phy, enum phy_mode mode, int submode)
{
	/* TODO */
	return -ENOSYS;
}
#define phy_set_mode(phy, mode) \
	phy_set_mode_ext(phy, mode, 0)

static int dwc3_core_init_mode(struct dwc3 *dwc)
{
	struct udevice *dev = dwc->dev;
	int ret;
	int i;

	switch (dwc->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		dwc3_set_prtcap(dwc, DWC3_GCTL_PRTCAP_DEVICE, false);

		phy_set_mode(dwc->usb2_generic_phy[0], PHY_MODE_USB_DEVICE);
		phy_set_mode(dwc->usb3_generic_phy[0], PHY_MODE_USB_DEVICE);

		ret = dwc3_gadget_init(dwc);
		if (ret)
			return dev_err_probe(dev, ret, "failed to initialize gadget\n");
		break;
	case USB_DR_MODE_HOST:
		dwc3_set_prtcap(dwc, DWC3_GCTL_PRTCAP_HOST, false);

		for (i = 0; i < dwc->num_usb2_ports; i++)
			phy_set_mode(dwc->usb2_generic_phy[i], PHY_MODE_USB_HOST);
		for (i = 0; i < dwc->num_usb3_ports; i++)
			phy_set_mode(dwc->usb3_generic_phy[i], PHY_MODE_USB_HOST);

		ret = dwc3_host_init(dwc);
		if (ret)
			return dev_err_probe(dev, ret, "failed to initialize host\n");
		break;
	case USB_DR_MODE_OTG:
		INIT_WORK(&dwc->drd_work, __dwc3_set_mode);
		ret = dwc3_drd_init(dwc);
		if (ret)
			return dev_err_probe(dev, ret, "failed to initialize dual-role\n");
		break;
	default:
		dev_err(dev, "Unsupported mode of operation %d\n", dwc->dr_mode);
		return -EINVAL;
	}

	return 0;
}

static void dwc3_core_stop(struct dwc3 *dwc)
{
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_DCTL);
	dwc3_writel(dwc->regs, DWC3_DCTL, reg & ~(DWC3_DCTL_RUN_STOP));
}

static void dwc3_core_exit_mode(struct dwc3 *dwc)
{
	switch (dwc->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		dwc3_gadget_exit(dwc);
		break;
	case USB_DR_MODE_HOST:
		dwc3_host_exit(dwc);
		break;
	case USB_DR_MODE_OTG:
		dwc3_drd_exit(dwc);
		break;
	default:
		/* do nothing */
		break;
	}

	/* de-assert DRVVBUS for HOST and OTG mode */
	dwc3_set_prtcap(dwc, DWC3_GCTL_PRTCAP_DEVICE, true);
}

#define DWC3_ALIGN_MASK		(16 - 1)


/* check whether the core supports IMOD */
bool dwc3_has_imod(struct dwc3 *dwc)
{
	return DWC3_VER_IS_WITHIN(DWC3, 300A, ANY) ||
		DWC3_VER_IS_WITHIN(DWC31, 120A, ANY) ||
		DWC3_IP_IS(DWC32);
}

static int dwc3_get_num_ports(struct dwc3 *dwc)
{
	void __iomem *base;
	u8 major_revision;
	u32 offset;
	u32 val;

	/*
	 * Remap xHCI address space to access XHCI ext cap regs since it is
	 * needed to get information on number of ports present.
	 */
	base = ioremap(dwc->xhci_resources[0].start,
		       resource_size(&dwc->xhci_resources[0]));
	if (!base)
		return -ENOMEM;

	offset = 0;
	do {
		offset = xhci_find_next_ext_cap(base, offset,
						XHCI_EXT_CAPS_PROTOCOL);
		if (!offset)
			break;

		val = readl(base + offset);
		major_revision = XHCI_EXT_PORT_MAJOR(val);

		val = readl(base + offset + 0x08);
		if (major_revision == 0x03) {
			dwc->num_usb3_ports += XHCI_EXT_PORT_COUNT(val);
		} else if (major_revision <= 0x02) {
			dwc->num_usb2_ports += XHCI_EXT_PORT_COUNT(val);
		} else {
			dev_warn(dwc->dev, "unrecognized port major revision %d\n",
				 major_revision);
		}
	} while (1);

	dev_dbg(dwc->dev, "hs-ports: %u ss-ports: %u\n",
		dwc->num_usb2_ports, dwc->num_usb3_ports);

	iounmap(base);

	if (dwc->num_usb2_ports > DWC3_USB2_MAX_PORTS ||
	    dwc->num_usb3_ports > DWC3_USB3_MAX_PORTS)
		return -EINVAL;

	return 0;
}

#if CONFIG_IS_ENABLED(PHY) && CONFIG_IS_ENABLED(DM_USB)
int dwc3_setup_phy(struct udevice *dev, struct phy_bulk *phys)
{
	int ret;

	ret = generic_phy_get_bulk(dev, phys);
	if (ret)
		return ret;

	ret = generic_phy_init_bulk(phys);
	if (ret)
		return ret;

	ret = generic_phy_power_on_bulk(phys);
	if (ret)
		generic_phy_exit_bulk(phys);

	return ret;
}

int dwc3_shutdown_phy(struct udevice *dev, struct phy_bulk *phys)
{
	int ret;

	ret = generic_phy_power_off_bulk(phys);
	ret |= generic_phy_exit_bulk(phys);
	return ret;
}
#endif

#if CONFIG_IS_ENABLED(DM_USB)
void dwc3_of_parse(struct dwc3 *dwc)
{
	const u8 *tmp;
	struct udevice *dev = dwc->dev;
	u8 lpm_nyet_threshold;
	u8 tx_de_emphasis;
	u8 hird_threshold;
	u32 val;
	int i;

	/* default to highest possible threshold */
	lpm_nyet_threshold = 0xff;

	/* default to -3.5dB de-emphasis */
	tx_de_emphasis = 1;

	/*
	 * default to assert utmi_sleep_n and use maximum allowed HIRD
	 * threshold value of 0b1100
	 */
	hird_threshold = 12;

	dwc->hsphy_mode = usb_get_phy_mode(dev_ofnode(dev));

	dwc->has_lpm_erratum = dev_read_bool(dev,
				"snps,has-lpm-erratum");
	tmp = dev_read_u8_array_ptr(dev, "snps,lpm-nyet-threshold", 1);
	if (tmp)
		lpm_nyet_threshold = *tmp;

	dwc->is_utmi_l1_suspend = dev_read_bool(dev,
				"snps,is-utmi-l1-suspend");
	tmp = dev_read_u8_array_ptr(dev, "snps,hird-threshold", 1);
	if (tmp)
		hird_threshold = *tmp;

	dwc->disable_scramble_quirk = dev_read_bool(dev,
				"snps,disable_scramble_quirk");
	dwc->u2exit_lfps_quirk = dev_read_bool(dev,
				"snps,u2exit_lfps_quirk");
	dwc->u2ss_inp3_quirk = dev_read_bool(dev,
				"snps,u2ss_inp3_quirk");
	dwc->req_p1p2p3_quirk = dev_read_bool(dev,
				"snps,req_p1p2p3_quirk");
	dwc->del_p1p2p3_quirk = dev_read_bool(dev,
				"snps,del_p1p2p3_quirk");
	dwc->del_phy_power_chg_quirk = dev_read_bool(dev,
				"snps,del_phy_power_chg_quirk");
	dwc->lfps_filter_quirk = dev_read_bool(dev,
				"snps,lfps_filter_quirk");
	dwc->rx_detect_poll_quirk = dev_read_bool(dev,
				"snps,rx_detect_poll_quirk");
	dwc->dis_u3_susphy_quirk = dev_read_bool(dev,
				"snps,dis_u3_susphy_quirk");
	dwc->dis_u2_susphy_quirk = dev_read_bool(dev,
				"snps,dis_u2_susphy_quirk");
	dwc->dis_del_phy_power_chg_quirk = dev_read_bool(dev,
				"snps,dis-del-phy-power-chg-quirk");
	dwc->dis_tx_ipgap_linecheck_quirk = dev_read_bool(dev,
				"snps,dis-tx-ipgap-linecheck-quirk");
	dwc->dis_enblslpm_quirk = dev_read_bool(dev,
				"snps,dis_enblslpm_quirk");
	dwc->dis_u2_freeclk_exists_quirk = dev_read_bool(dev,
				"snps,dis-u2-freeclk-exists-quirk");
	dwc->tx_de_emphasis_quirk = dev_read_bool(dev,
				"snps,tx_de_emphasis_quirk");
	tmp = dev_read_u8_array_ptr(dev, "snps,tx_de_emphasis", 1);
	if (tmp)
		tx_de_emphasis = *tmp;

	dwc->lpm_nyet_threshold = lpm_nyet_threshold;
	dwc->tx_de_emphasis = tx_de_emphasis;

	dwc->hird_threshold = hird_threshold
		| (dwc->is_utmi_l1_suspend << 4);

	dev_read_u32(dev, "snps,quirk-frame-length-adjustment", &dwc->fladj);

	/*
	 * Handle property "snps,incr-burst-type-adjustment".
	 * Get the number of value from this property:
	 * result <= 0, means this property is not supported.
	 * result = 1, means INCRx burst mode supported.
	 * result > 1, means undefined length burst mode supported.
	 */
	dwc->incrx_mode = INCRX_BURST_MODE;
	dwc->incrx_size = 0;
	for (i = 0; i < 8; i++) {
		if (dev_read_u32_index(dev, "snps,incr-burst-type-adjustment",
				       i, &val))
			break;

		dwc->incrx_mode = INCRX_UNDEF_LENGTH_BURST_MODE;
		dwc->incrx_size = max(dwc->incrx_size, val);
	}
}

int dwc3_init(struct dwc3 *dwc)
{
	unsigned int hw_mode;
	int ret;
	u32 reg;

	dwc3_cache_hwparams(dwc);

	/*
	 * Currently only DWC3 controllers that are host-only capable
	 * can have more than one port.
	 */
	hw_mode = DWC3_GHWPARAMS0_MODE(dwc->hwparams.hwparams0);
	if (hw_mode == DWC3_GHWPARAMS0_MODE_HOST) {
		ret = dwc3_get_num_ports(dwc);
		if (ret)
			return ret;
	} else {
		dwc->num_usb2_ports = 1;
		dwc->num_usb3_ports = 1;
	}

	spin_lock_init(&dwc->lock);
	mutex_init(&dwc->mutex);

	ret = dwc3_alloc_event_buffers(dwc, DWC3_EVENT_BUFFERS_SIZE);
	if (ret) {
		dev_err(dwc->dev, "failed to allocate event buffers\n");
		return -ENOMEM;
	}

	ret = dwc3_core_init(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to initialize core\n");
		goto core_fail;
	}

	ret = dwc3_event_buffers_setup(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to setup event buffers\n");
		goto event_fail;
	}

	if (dwc->revision >= DWC3_REVISION_250A) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL1);

		/*
		 * Enable hardware control of sending remote wakeup
		 * in HS when the device is in the L1 state.
		 */
		if (dwc->revision >= DWC3_REVISION_290A)
			reg |= DWC3_GUCTL1_DEV_L1_EXIT_BY_HW;

		if (dwc->dis_tx_ipgap_linecheck_quirk)
			reg |= DWC3_GUCTL1_TX_IPGAP_LINECHECK_DIS;

		dwc3_writel(dwc->regs, DWC3_GUCTL1, reg);
	}

	if (dwc->dr_mode == USB_DR_MODE_HOST ||
	    dwc->dr_mode == USB_DR_MODE_OTG) {
		reg = dwc3_readl(dwc->regs, DWC3_GUCTL);

		reg |= DWC3_GUCTL_HSTINAUTORETRY;

		dwc3_writel(dwc->regs, DWC3_GUCTL, reg);
	}

	ret = dwc3_core_init_mode(dwc);
	if (ret)
		goto mode_fail;

	return 0;

mode_fail:
	dwc3_event_buffers_cleanup(dwc);

event_fail:
	dwc3_core_exit(dwc);

core_fail:
	dwc3_free_event_buffers(dwc);

	return ret;
}

void dwc3_remove(struct dwc3 *dwc)
{
	dwc3_core_exit_mode(dwc);
	dwc3_event_buffers_cleanup(dwc);
	dwc3_free_event_buffers(dwc);
	dwc3_core_stop(dwc);
	dwc3_core_exit(dwc);
}
#endif

/**
 * dwc3_uboot_init - dwc3 core uboot initialization code
 * @dwc3_dev: struct dwc3_device containing initialization data
 *
 * Entry point for dwc3 driver (equivalent to dwc3_probe in linux
 * kernel driver). Pointer to dwc3_device should be passed containing
 * base address and other initialization data. Returns '0' on success and
 * a negative value on failure.
 *
 * Generally called from board_usb_init() implemented in board file.
 */
int dwc3_uboot_init(struct dwc3_device *dwc3_dev)
{
	struct dwc3		*dwc;
	struct device		*dev = NULL;
	u8			lpm_nyet_threshold;
	u8			tx_de_emphasis;
	u8			hird_threshold;

	int			ret;

	void			*mem;

	mem = devm_kzalloc((struct udevice *)dev,
			   sizeof(*dwc) + DWC3_ALIGN_MASK, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;

	dwc = PTR_ALIGN(mem, DWC3_ALIGN_MASK + 1);
	dwc->mem = mem;

	dwc->regs = (void *)(uintptr_t)(dwc3_dev->base +
					DWC3_GLOBALS_REGS_START);

	/* default to highest possible threshold */
	lpm_nyet_threshold = 0xff;

	/* default to -3.5dB de-emphasis */
	tx_de_emphasis = 1;

	/*
	 * default to assert utmi_sleep_n and use maximum allowed HIRD
	 * threshold value of 0b1100
	 */
	hird_threshold = 12;

	dwc->maximum_speed = dwc3_dev->maximum_speed;
	dwc->has_lpm_erratum = dwc3_dev->has_lpm_erratum;
	if (dwc3_dev->lpm_nyet_threshold)
		lpm_nyet_threshold = dwc3_dev->lpm_nyet_threshold;
	dwc->is_utmi_l1_suspend = dwc3_dev->is_utmi_l1_suspend;
	if (dwc3_dev->hird_threshold)
		hird_threshold = dwc3_dev->hird_threshold;

	dwc->do_fifo_resize = dwc3_dev->tx_fifo_resize;
	dwc->dr_mode = dwc3_dev->dr_mode;

	dwc->disable_scramble_quirk = dwc3_dev->disable_scramble_quirk;
	dwc->u2exit_lfps_quirk = dwc3_dev->u2exit_lfps_quirk;
	dwc->u2ss_inp3_quirk = dwc3_dev->u2ss_inp3_quirk;
	dwc->req_p1p2p3_quirk = dwc3_dev->req_p1p2p3_quirk;
	dwc->del_p1p2p3_quirk = dwc3_dev->del_p1p2p3_quirk;
	dwc->del_phy_power_chg_quirk = dwc3_dev->del_phy_power_chg_quirk;
	dwc->lfps_filter_quirk = dwc3_dev->lfps_filter_quirk;
	dwc->rx_detect_poll_quirk = dwc3_dev->rx_detect_poll_quirk;
	dwc->dis_u3_susphy_quirk = dwc3_dev->dis_u3_susphy_quirk;
	dwc->dis_u2_susphy_quirk = dwc3_dev->dis_u2_susphy_quirk;
	dwc->dis_del_phy_power_chg_quirk = dwc3_dev->dis_del_phy_power_chg_quirk;
	dwc->dis_tx_ipgap_linecheck_quirk = dwc3_dev->dis_tx_ipgap_linecheck_quirk;
	dwc->dis_enblslpm_quirk = dwc3_dev->dis_enblslpm_quirk;
	dwc->dis_u2_freeclk_exists_quirk = dwc3_dev->dis_u2_freeclk_exists_quirk;

	dwc->tx_de_emphasis_quirk = dwc3_dev->tx_de_emphasis_quirk;
	if (dwc3_dev->tx_de_emphasis)
		tx_de_emphasis = dwc3_dev->tx_de_emphasis;

	/* default to superspeed if no maximum_speed passed */
	if (dwc->maximum_speed == USB_SPEED_UNKNOWN)
		dwc->maximum_speed = USB_SPEED_SUPER;

	dwc->lpm_nyet_threshold = lpm_nyet_threshold;
	dwc->tx_de_emphasis = tx_de_emphasis;

	dwc->hird_threshold = hird_threshold
		| (dwc->is_utmi_l1_suspend << 4);

	dwc->hsphy_mode = dwc3_dev->hsphy_mode;

	dwc->index = dwc3_dev->index;

	dwc3_cache_hwparams(dwc);

	ret = dwc3_alloc_event_buffers(dwc, DWC3_EVENT_BUFFERS_SIZE);
	if (ret) {
		dev_err(dwc->dev, "failed to allocate event buffers\n");
		return -ENOMEM;
	}

	if (!IS_ENABLED(CONFIG_USB_DWC3_GADGET))
		dwc->dr_mode = USB_DR_MODE_HOST;
	else if (!IS_ENABLED(CONFIG_USB_HOST))
		dwc->dr_mode = USB_DR_MODE_PERIPHERAL;

	if (dwc->dr_mode == USB_DR_MODE_UNKNOWN)
		dwc->dr_mode = USB_DR_MODE_OTG;

	ret = dwc3_core_init(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to initialize core\n");
		goto err0;
	}

	ret = dwc3_event_buffers_setup(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to setup event buffers\n");
		goto err1;
	}

	ret = dwc3_core_init_mode(dwc);
	if (ret)
		goto err2;

	list_add_tail(&dwc->list, &dwc3_list);

	return 0;

err2:
	dwc3_event_buffers_cleanup(dwc);

err1:
	dwc3_core_exit(dwc);

err0:
	dwc3_free_event_buffers(dwc);

	return ret;
}

/**
 * dwc3_uboot_exit - dwc3 core uboot cleanup code
 * @index: index of this controller
 *
 * Performs cleanup of memory allocated in dwc3_uboot_init and other misc
 * cleanups (equivalent to dwc3_remove in linux). index of _this_ controller
 * should be passed and should match with the index passed in
 * dwc3_device during init.
 *
 * Generally called from board file.
 */
void dwc3_uboot_exit(int index)
{
	struct dwc3 *dwc;

	list_for_each_entry(dwc, &dwc3_list, list) {
		if (dwc->index != index)
			continue;

		dwc3_core_exit_mode(dwc);
		dwc3_event_buffers_cleanup(dwc);
		dwc3_free_event_buffers(dwc);
		dwc3_core_exit(dwc);
		list_del(&dwc->list);
		kfree(dwc->mem);
		break;
	}
}


#if !CONFIG_IS_ENABLED(DM_USB_GADGET)
__weak int dwc3_uboot_interrupt_status(struct udevice *dev)
{
	return 1;
}

/**
 * dm_usb_gadget_handle_interrupts - handle dwc3 core interrupt
 * @dev: device of this controller
 *
 * Invokes dwc3 gadget interrupts.
 *
 * Generally called from board file.
 */
int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct dwc3 *dwc = NULL;

	if (!dwc3_uboot_interrupt_status(dev))
		return 0;

	list_for_each_entry(dwc, &dwc3_list, list) {
		if (dwc->dev != dev)
			continue;

		dwc3_gadget_uboot_handle_interrupt(dwc);
		break;
	}

	return 0;
}
#endif

