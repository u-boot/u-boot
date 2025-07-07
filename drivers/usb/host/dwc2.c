// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Oleksandr Tymoshenko <gonzo@freebsd.org>
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <phys2bus.h>
#include <usb.h>
#include <usbroothubdes.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/usb/otg.h>
#include <power/regulator.h>
#include <reset.h>

#include "../common/dwc2_core.h"
#include "dwc2.h"

/* Use only HC channel 0. */
#define DWC2_HC_CHANNEL			0

#define DWC2_STATUS_BUF_SIZE		64
#define DWC2_DATA_BUF_SIZE		(CONFIG_USB_DWC2_BUFFER_SIZE * 1024)

#define MAX_DEVICE			16
#define MAX_ENDPOINT			16

struct dwc2_priv {
#if CONFIG_IS_ENABLED(DM_USB)
	u8 aligned_buffer[DWC2_DATA_BUF_SIZE] __aligned(ARCH_DMA_MINALIGN);
	u8 status_buffer[DWC2_STATUS_BUF_SIZE] __aligned(ARCH_DMA_MINALIGN);
#ifdef CONFIG_DM_REGULATOR
	struct udevice *vbus_supply;
#endif
	struct phy phy;
	struct clk_bulk clks;
#else
	u8 *aligned_buffer;
	u8 *status_buffer;
#endif
	u8 in_data_toggle[MAX_DEVICE][MAX_ENDPOINT];
	u8 out_data_toggle[MAX_DEVICE][MAX_ENDPOINT];
	struct dwc2_core_regs *regs;
	int root_hub_devnum;
	bool ext_vbus;
	/*
	 * The hnp/srp capability must be disabled if the platform
	 * does't support hnp/srp. Otherwise the force mode can't work.
	 */
	bool hnp_srp_disable;
	bool oc_disable;

	struct reset_ctl_bulk	resets;
};

#if !CONFIG_IS_ENABLED(DM_USB)
/* We need cacheline-aligned buffers for DMA transfers and dcache support */
DEFINE_ALIGN_BUFFER(u8, aligned_buffer_addr, DWC2_DATA_BUF_SIZE,
		    ARCH_DMA_MINALIGN);
DEFINE_ALIGN_BUFFER(u8, status_buffer_addr, DWC2_STATUS_BUF_SIZE,
		    ARCH_DMA_MINALIGN);

static struct dwc2_priv local;
#endif

/*
 * DWC2 IP interface
 */

/*
 * Initializes the FSLSPClkSel field of the HCFG register
 * depending on the PHY type.
 */
static void init_fslspclksel(struct dwc2_core_regs *regs)
{
	u32 phyclk;

#if (DWC2_PHY_TYPE == DWC2_PHY_TYPE_FS)
	phyclk = HCFG_FSLSPCLKSEL_48_MHZ;	/* Full speed PHY */
#else
	/* High speed PHY running at full speed or high speed */
	phyclk = HCFG_FSLSPCLKSEL_30_60_MHZ;
#endif

#ifdef DWC2_ULPI_FS_LS
	u32 hwcfg2 = readl(&regs->global_regs.ghwcfg2);
	u32 hval = FIELD_GET(GHWCFG2_HS_PHY_TYPE_MASK, ghwcfg2);
	u32 fval = FIELD_GET(GHWCFG2_FS_PHY_TYPE_MASK, ghwcfg2);

	if (hval == GHWCFG2_FS_PHY_TYPE_SHARED_UTMI && fval == GHWCFG2_HS_PHY_TYPE_UTMI)
		phyclk = HCFG_FSLSPCLKSEL_48_MHZ;	/* Full speed PHY */
#endif

	clrsetbits_le32(&regs->host_regs.hcfg,
			HCFG_FSLSPCLKSEL_MASK,
			FIELD_PREP(HCFG_FSLSPCLKSEL_MASK, phyclk));
}

#if CONFIG_IS_ENABLED(DM_USB) && defined(CONFIG_DM_REGULATOR)
static int dwc_vbus_supply_init(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret) {
		debug("%s: No vbus supply\n", dev->name);
		return 0;
	}

	ret = regulator_set_enable_if_allowed(priv->vbus_supply, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "Error enabling vbus supply\n");
		return ret;
	}

	return 0;
}

static int dwc_vbus_supply_exit(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vbus_supply, false);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "Error disabling vbus supply\n");
		return ret;
	}

	return 0;
}
#else
static int dwc_vbus_supply_init(struct udevice *dev)
{
	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB)
static int dwc_vbus_supply_exit(struct udevice *dev)
{
	return 0;
}
#endif
#endif

/*
 * This function initializes the DWC_otg controller registers for
 * host mode.
 *
 * This function flushes the Tx and Rx FIFOs and it flushes any entries in the
 * request queues. Host channels are reset to ensure that they are ready for
 * performing transfers.
 *
 * @param dev USB Device (NULL if driver model is not being used)
 * @param regs Programming view of DWC_otg controller
 *
 */
static void dwc_otg_core_host_init(struct udevice *dev,
				   struct dwc2_core_regs *regs)
{
	u32 nptxfifosize = 0;
	u32 ptxfifosize = 0;
	u32 hprt0 = 0;
	int i, ret, num_channels;

	/* Restart the Phy Clock */
	writel(0, &regs->pcgcctl);

	/* Initialize Host Configuration Register */
	init_fslspclksel(regs);
#ifdef DWC2_DFLT_SPEED_FULL
	setbits_le32(&regs->host_regs.hcfg, HCFG_FSLSSUPP);
#endif

	/* Configure data FIFO sizes */
#ifdef DWC2_ENABLE_DYNAMIC_FIFO
	if (readl(&regs->global_regs.ghwcfg2) & GHWCFG2_DYNAMIC_FIFO) {
		/* Rx FIFO */
		writel(DWC2_HOST_RX_FIFO_SIZE, &regs->global_regs.grxfsiz);

		/* Non-periodic Tx FIFO */
		nptxfifosize |= FIELD_PREP(FIFOSIZE_DEPTH_MASK, DWC2_HOST_NPERIO_TX_FIFO_SIZE);
		nptxfifosize |= FIELD_PREP(FIFOSIZE_STARTADDR_MASK, DWC2_HOST_RX_FIFO_SIZE);
		writel(nptxfifosize, &regs->global_regs.gnptxfsiz);

		/* Periodic Tx FIFO */
		ptxfifosize |= FIELD_PREP(FIFOSIZE_DEPTH_MASK, DWC2_HOST_PERIO_TX_FIFO_SIZE);
		ptxfifosize |= FIELD_PREP(FIFOSIZE_STARTADDR_MASK, DWC2_HOST_RX_FIFO_SIZE +
					  DWC2_HOST_NPERIO_TX_FIFO_SIZE);
		writel(ptxfifosize, &regs->global_regs.hptxfsiz);
	}
#endif

	/* Clear Host Set HNP Enable in the OTG Control Register */
	clrbits_le32(&regs->global_regs.gotgctl, GOTGCTL_HSTSETHNPEN);

	/* Make sure the FIFOs are flushed. */
	dwc2_flush_tx_fifo(regs, GRSTCTL_TXFNUM_ALL);	/* All Tx FIFOs */
	dwc2_flush_rx_fifo(regs);

	/* Flush out any leftover queued requests. */
	num_channels = FIELD_GET(GHWCFG2_NUM_HOST_CHAN_MASK, readl(&regs->global_regs.ghwcfg2)) + 1;

	for (i = 0; i < num_channels; i++)
		clrsetbits_le32(&regs->host_regs.hc[i].hcchar, HCCHAR_CHENA | HCCHAR_EPDIR,
				HCCHAR_CHDIS);

	/* Halt all channels to put them into a known state. */
	for (i = 0; i < num_channels; i++) {
		clrsetbits_le32(&regs->host_regs.hc[i].hcchar,
				HCCHAR_EPDIR,
				HCCHAR_CHENA | HCCHAR_CHDIS);
		ret = wait_for_bit_le32(&regs->host_regs.hc[i].hcchar,
					HCCHAR_CHENA, false, 1000, false);
		if (ret)
			dev_info(dev, "%s: Timeout!\n", __func__);
	}

	/* Turn on the vbus power. */
	if (readl(&regs->global_regs.gintsts) & GINTSTS_CURMODE_HOST) {
		hprt0 = readl(&regs->host_regs.hprt0) & ~HPRT0_W1C_MASK;
		if (!(hprt0 & HPRT0_PWR)) {
			hprt0 |= HPRT0_PWR;
			writel(hprt0, &regs->host_regs.hprt0);
		}
	}

	if (dev)
		dwc_vbus_supply_init(dev);
}

/*
 * This function initializes the DWC_otg controller registers and
 * prepares the core for device mode or host mode operation.
 *
 * @param regs Programming view of the DWC_otg controller
 */
static void dwc_otg_core_init(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	struct dwc2_core_regs *regs = priv->regs;
	u32 ahbcfg = 0;
	u32 usbcfg = 0;
	u8 brst_sz = DWC2_DMA_BURST_SIZE;

	/* Common Initialization */
	usbcfg = readl(&regs->global_regs.gusbcfg);

	/* Program the ULPI External VBUS bit if needed */
	if (priv->ext_vbus) {
		usbcfg |= GUSBCFG_ULPI_EXT_VBUS_DRV;
		if (!priv->oc_disable) {
			usbcfg |= GUSBCFG_ULPI_INT_VBUS_IND |
				  GUSBCFG_INDICATORPASSTHROUGH;
		}
	} else {
		usbcfg &= ~GUSBCFG_ULPI_EXT_VBUS_DRV;
	}

	/* Set external TS Dline pulsing */
#ifdef DWC2_TS_DLINE
	usbcfg |= GUSBCFG_TERMSELDLPULSE;
#else
	usbcfg &= ~GUSBCFG_TERMSELDLPULSE;
#endif
	writel(usbcfg, &regs->global_regs.gusbcfg);

	/* Reset the Controller */
	dwc2_core_reset(regs);

	/*
	 * This programming sequence needs to happen in FS mode before
	 * any other programming occurs
	 */
#if defined(DWC2_DFLT_SPEED_FULL) && \
	(DWC2_PHY_TYPE == DWC2_PHY_TYPE_FS)
	/* If FS mode with FS PHY */
	setbits_le32(&regs->global_regs.gusbcfg, GUSBCFG_PHYSEL);

	/* Reset after a PHY select */
	dwc2_core_reset(regs);

	/*
	 * Program DCFG.DevSpd or HCFG.FSLSPclkSel to 48Mhz in FS.
	 * Also do this on HNP Dev/Host mode switches (done in dev_init
	 * and host_init).
	 */
	if (readl(&regs->global_regs.gintsts) & GINTSTS_CURMODE_HOST)
		init_fslspclksel(regs);

#ifdef DWC2_I2C_ENABLE
	/* Program GUSBCFG.OtgUtmifsSel to I2C */
	setbits_le32(&regs->global_regs.gusbcfg, GUSBCFG_OTG_UTMI_FS_SEL);

	/* Program GI2CCTL.I2CEn */
	clrsetbits_le32(&regs->global_regs.gi2cctl, GI2CCTL_I2CEN |
			GI2CCTL_I2CDEVADDR_MASK,
			FIELD_PREP(GI2CCTL_I2CDEVADDR_MASK, 1));
	setbits_le32(&regs->global_regs.gi2cctl, GI2CCTL_I2CEN);
#endif

#else
	/* High speed PHY. */

	/*
	 * HS PHY parameters. These parameters are preserved during
	 * soft reset so only program the first time. Do a soft reset
	 * immediately after setting phyif.
	 */
#if (DWC2_PHY_TYPE == DWC2_PHY_TYPE_ULPI)
	usbcfg |= GUSBCFG_ULPI_UTMI_SEL;
	usbcfg &= ~GUSBCFG_PHYIF16;
#ifdef DWC2_PHY_ULPI_DDR
	usbcfg |= GUSBCFG_DDRSEL;
#else
	usbcfg &= ~GUSBCFG_DDRSEL;
#endif /* DWC2_PHY_ULPI_DDR */
#elif (DWC2_PHY_TYPE == DWC2_PHY_TYPE_UTMI)
	usbcfg &= ~GUSBCFG_ULPI_UTMI_SEL;
#if (DWC2_UTMI_WIDTH == 16)
	usbcfg |= GUSBCFG_PHYIF16;
#else
	usbcfg &= ~GUSBCFG_PHYIF16;
#endif /* DWC2_UTMI_WIDTH */
#endif /* DWC2_PHY_TYPE */

	writel(usbcfg, &regs->global_regs.gusbcfg);

	/* Reset after setting the PHY parameters */
	dwc2_core_reset(regs);
#endif

	usbcfg = readl(&regs->global_regs.gusbcfg);
	usbcfg &= ~(GUSBCFG_ULPI_FS_LS | GUSBCFG_ULPI_CLK_SUSP_M);
#ifdef DWC2_ULPI_FS_LS
	u32 hwcfg2 = readl(&regs->global_regs.ghwcfg2);
	u32 hval = FIELD_GET(GHWCFG2_HS_PHY_TYPE_MASK, ghwcfg2);
	u32 fval = FIELD_GET(GHWCFG2_FS_PHY_TYPE_MASK, ghwcfg2);

	if (hval == GHWCFG2_FS_PHY_TYPE_SHARED_UTMI && fval == GHWCFG2_HS_PHY_TYPE_UTMI) {
		usbcfg |= GUSBCFG_ULPI_FS_LS;
		usbcfg |= GUSBCFG_ULPI_CLK_SUSP_M;
	}
#endif
	if (priv->hnp_srp_disable)
		usbcfg |= GUSBCFG_FORCEHOSTMODE;

	writel(usbcfg, &regs->global_regs.gusbcfg);

	/* Program the GAHBCFG Register. */
	switch (FIELD_GET(GHWCFG2_ARCHITECTURE_MASK, readl(&regs->global_regs.ghwcfg2))) {
	case GHWCFG2_SLAVE_ONLY_ARCH:
		break;
	case GHWCFG2_EXT_DMA_ARCH:
		ahbcfg |= FIELD_PREP(GAHBCFG_HBSTLEN_MASK, LOG2(brst_sz >> 1));
#ifdef DWC2_DMA_ENABLE
		ahbcfg |= GAHBCFG_DMA_EN;
#endif
		break;
	case GHWCFG2_INT_DMA_ARCH:
		ahbcfg |= FIELD_PREP(GAHBCFG_HBSTLEN_MASK, GAHBCFG_HBSTLEN_INCR4);
#ifdef DWC2_DMA_ENABLE
		ahbcfg |= GAHBCFG_DMA_EN;
#endif
		break;
	}

	writel(ahbcfg, &regs->global_regs.gahbcfg);

	/* Program the capabilities in GUSBCFG Register */
	usbcfg = 0;

	if (!priv->hnp_srp_disable)
		usbcfg |= GUSBCFG_HNPCAP | GUSBCFG_SRPCAP;
#ifdef DWC2_IC_USB_CAP
	usbcfg |= GUSBCFG_ICUSBCAP;
#endif

	setbits_le32(&regs->global_regs.gusbcfg, usbcfg);
}

/*
 * Prepares a host channel for transferring packets to/from a specific
 * endpoint. The HCCHARn register is set up with the characteristics specified
 * in _hc. Host channel interrupts that may need to be serviced while this
 * transfer is in progress are enabled.
 *
 * @param regs Programming view of DWC_otg controller
 * @param hc Information needed to initialize the host channel
 */
static void dwc_otg_hc_init(struct dwc2_core_regs *regs, u8 hc_num,
			    struct usb_device *dev, u8 dev_addr, u8 ep_num,
			    u8 ep_is_in, u8 ep_type, u16 max_packet)
{
	struct dwc2_hc_regs *hc_regs = &regs->host_regs.hc[hc_num];
	u32 hcchar = FIELD_PREP(HCCHAR_DEVADDR_MASK, dev_addr) |
			  FIELD_PREP(HCCHAR_EPNUM_MASK, ep_num) |
			  FIELD_PREP(HCCHAR_EPDIR, ep_is_in) |
			  FIELD_PREP(HCCHAR_EPTYPE_MASK, ep_type) |
			  FIELD_PREP(HCCHAR_MPS_MASK, max_packet);

	if (dev->speed == USB_SPEED_LOW)
		hcchar |= HCCHAR_LSPDDEV;

	/*
	 * Program the HCCHARn register with the endpoint characteristics
	 * for the current transfer.
	 */
	writel(hcchar, &hc_regs->hcchar);

	/* Program the HCSPLIT register, default to no SPLIT */
	writel(0, &hc_regs->hcsplt);
}

static void dwc_otg_hc_init_split(struct dwc2_hc_regs *hc_regs,
				  u8 hub_devnum, u8 hub_port)
{
	u32 hcsplt = 0;

	hcsplt = HCSPLT_SPLTENA;
	hcsplt |= FIELD_PREP(HCSPLT_HUBADDR_MASK, hub_devnum);
	hcsplt |= FIELD_PREP(HCSPLT_PRTADDR_MASK, hub_port);

	/* Program the HCSPLIT register for SPLITs */
	writel(hcsplt, &hc_regs->hcsplt);
}

/*
 * DWC2 to USB API interface
 */
/* Direction: In ; Request: Status */
static int dwc_otg_submit_rh_msg_in_status(struct dwc2_core_regs *regs,
					   struct usb_device *dev, void *buffer,
					   int txlen, struct devrequest *cmd)
{
	u32 hprt0 = 0;
	u32 port_status = 0;
	u32 port_change = 0;
	int len = 0;
	int stat = 0;

	switch (cmd->requesttype & ~USB_DIR_IN) {
	case 0:
		*(u16 *)buffer = cpu_to_le16(1);
		len = 2;
		break;
	case USB_RECIP_INTERFACE:
	case USB_RECIP_ENDPOINT:
		*(u16 *)buffer = cpu_to_le16(0);
		len = 2;
		break;
	case USB_TYPE_CLASS:
		*(u32 *)buffer = cpu_to_le32(0);
		len = 4;
		break;
	case USB_RECIP_OTHER | USB_TYPE_CLASS:
		hprt0 = readl(&regs->host_regs.hprt0);
		if (hprt0 & HPRT0_CONNSTS)
			port_status |= USB_PORT_STAT_CONNECTION;
		if (hprt0 & HPRT0_ENA)
			port_status |= USB_PORT_STAT_ENABLE;
		if (hprt0 & HPRT0_SUSP)
			port_status |= USB_PORT_STAT_SUSPEND;
		if (hprt0 & HPRT0_OVRCURRACT)
			port_status |= USB_PORT_STAT_OVERCURRENT;
		if (hprt0 & HPRT0_RST)
			port_status |= USB_PORT_STAT_RESET;
		if (hprt0 & HPRT0_PWR)
			port_status |= USB_PORT_STAT_POWER;

		switch (FIELD_GET(HPRT0_SPD_MASK, hprt0)) {
		case HPRT0_SPD_LOW_SPEED:
			port_status |= USB_PORT_STAT_LOW_SPEED;
			break;
		case HPRT0_SPD_HIGH_SPEED:
			port_status |= USB_PORT_STAT_HIGH_SPEED;
			break;
		}

		if (hprt0 & HPRT0_ENACHG)
			port_change |= USB_PORT_STAT_C_ENABLE;
		if (hprt0 & HPRT0_CONNDET)
			port_change |= USB_PORT_STAT_C_CONNECTION;
		if (hprt0 & HPRT0_OVRCURRCHG)
			port_change |= USB_PORT_STAT_C_OVERCURRENT;

		*(u32 *)buffer = cpu_to_le32(port_status |
					(port_change << 16));
		len = 4;
		break;
	default:
		puts("unsupported root hub command\n");
		stat = USB_ST_STALLED;
	}

	dev->act_len = min(len, txlen);
	dev->status = stat;

	return stat;
}

/* Direction: In ; Request: Descriptor */
static int dwc_otg_submit_rh_msg_in_descriptor(struct usb_device *dev,
					       void *buffer, int txlen,
					       struct devrequest *cmd)
{
	unsigned char data[32];
	u32 dsc;
	int len = 0;
	int stat = 0;
	u16 wValue = cpu_to_le16(cmd->value);
	u16 wLength = cpu_to_le16(cmd->length);

	switch (cmd->requesttype & ~USB_DIR_IN) {
	case 0:
		switch (wValue & 0xff00) {
		case 0x0100:	/* device descriptor */
			len = min3(txlen, (int)sizeof(root_hub_dev_des), (int)wLength);
			memcpy(buffer, root_hub_dev_des, len);
			break;
		case 0x0200:	/* configuration descriptor */
			len = min3(txlen, (int)sizeof(root_hub_config_des), (int)wLength);
			memcpy(buffer, root_hub_config_des, len);
			break;
		case 0x0300:	/* string descriptors */
			switch (wValue & 0xff) {
			case 0x00:
				len = min3(txlen, (int)sizeof(root_hub_str_index0),
					   (int)wLength);
				memcpy(buffer, root_hub_str_index0, len);
				break;
			case 0x01:
				len = min3(txlen, (int)sizeof(root_hub_str_index1),
					   (int)wLength);
				memcpy(buffer, root_hub_str_index1, len);
				break;
			}
			break;
		default:
			stat = USB_ST_STALLED;
		}
		break;

	case USB_TYPE_CLASS:
		/* Root port config, set 1 port and nothing else. */
		dsc = 0x00000001;

		data[0] = 9;		/* min length; */
		data[1] = 0x29;
		data[2] = dsc & RH_A_NDP;
		data[3] = 0;
		if (dsc & RH_A_PSM)
			data[3] |= 0x1;
		if (dsc & RH_A_NOCP)
			data[3] |= 0x10;
		else if (dsc & RH_A_OCPM)
			data[3] |= 0x8;

		/* corresponds to data[4-7] */
		data[5] = (dsc & RH_A_POTPGT) >> 24;
		data[7] = dsc & RH_B_DR;
		if (data[2] < 7) {
			data[8] = 0xff;
		} else {
			data[0] += 2;
			data[8] = (dsc & RH_B_DR) >> 8;
			data[9] = 0xff;
			data[10] = data[9];
		}

		len = min3(txlen, (int)data[0], (int)wLength);
		memcpy(buffer, data, len);
		break;
	default:
		puts("unsupported root hub command\n");
		stat = USB_ST_STALLED;
	}

	dev->act_len = min(len, txlen);
	dev->status = stat;

	return stat;
}

/* Direction: In ; Request: Configuration */
static int dwc_otg_submit_rh_msg_in_configuration(struct usb_device *dev,
						  void *buffer, int txlen,
						  struct devrequest *cmd)
{
	int len = 0;
	int stat = 0;

	switch (cmd->requesttype & ~USB_DIR_IN) {
	case 0:
		*(u8 *)buffer = 0x01;
		len = 1;
		break;
	default:
		puts("unsupported root hub command\n");
		stat = USB_ST_STALLED;
	}

	dev->act_len = min(len, txlen);
	dev->status = stat;

	return stat;
}

/* Direction: In */
static int dwc_otg_submit_rh_msg_in(struct dwc2_priv *priv,
				    struct usb_device *dev, void *buffer,
				    int txlen, struct devrequest *cmd)
{
	switch (cmd->request) {
	case USB_REQ_GET_STATUS:
		return dwc_otg_submit_rh_msg_in_status(priv->regs, dev, buffer,
						       txlen, cmd);
	case USB_REQ_GET_DESCRIPTOR:
		return dwc_otg_submit_rh_msg_in_descriptor(dev, buffer,
							   txlen, cmd);
	case USB_REQ_GET_CONFIGURATION:
		return dwc_otg_submit_rh_msg_in_configuration(dev, buffer,
							      txlen, cmd);
	default:
		puts("unsupported root hub command\n");
		return USB_ST_STALLED;
	}
}

/* Direction: Out */
static int dwc_otg_submit_rh_msg_out(struct dwc2_priv *priv,
				     struct usb_device *dev,
				     void *buffer, int txlen,
				     struct devrequest *cmd)
{
	struct dwc2_core_regs *regs = priv->regs;
	int len = 0;
	int stat = 0;
	u16 bmrtype_breq = cmd->requesttype | (cmd->request << 8);
	u16 wValue = cpu_to_le16(cmd->value);

	switch (bmrtype_breq & ~USB_DIR_IN) {
	case (USB_REQ_CLEAR_FEATURE << 8) | USB_RECIP_ENDPOINT:
	case (USB_REQ_CLEAR_FEATURE << 8) | USB_TYPE_CLASS:
		break;

	case (USB_REQ_CLEAR_FEATURE << 8) | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_C_CONNECTION:
			clrsetbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK, HPRT0_CONNDET);
			break;
		}
		break;

	case (USB_REQ_SET_FEATURE << 8) | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			break;

		case USB_PORT_FEAT_RESET:
			clrsetbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK, HPRT0_RST);
			mdelay(50);
			clrbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK | HPRT0_RST);
			break;

		case USB_PORT_FEAT_POWER:
			clrsetbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK, HPRT0_RST);
			break;

		case USB_PORT_FEAT_ENABLE:
			break;
		}
		break;
	case (USB_REQ_SET_ADDRESS << 8):
		priv->root_hub_devnum = wValue;
		break;
	case (USB_REQ_SET_CONFIGURATION << 8):
		break;
	default:
		puts("unsupported root hub command\n");
		stat = USB_ST_STALLED;
	}

	len = min(len, txlen);

	dev->act_len = len;
	dev->status = stat;

	return stat;
}

static int dwc_otg_submit_rh_msg(struct dwc2_priv *priv, struct usb_device *dev,
				 unsigned long pipe, void *buffer, int txlen,
				 struct devrequest *cmd)
{
	int stat = 0;

	if (usb_pipeint(pipe)) {
		puts("Root-Hub submit IRQ: NOT implemented\n");
		return 0;
	}

	if (cmd->requesttype & USB_DIR_IN)
		stat = dwc_otg_submit_rh_msg_in(priv, dev, buffer, txlen, cmd);
	else
		stat = dwc_otg_submit_rh_msg_out(priv, dev, buffer, txlen, cmd);

	mdelay(1);

	return stat;
}

int wait_for_chhltd(struct dwc2_hc_regs *hc_regs, u32 *sub, u8 *toggle)
{
	int ret;
	u32 hcint, hctsiz;

	ret = wait_for_bit_le32(&hc_regs->hcint, HCINTMSK_CHHLTD, true,
				2000, false);
	if (ret)
		return ret;

	hcint = readl(&hc_regs->hcint);
	hctsiz = readl(&hc_regs->hctsiz);
	*sub = FIELD_GET(TSIZ_XFERSIZE_MASK, hctsiz);
	*toggle = FIELD_GET(TSIZ_SC_MC_PID_MASK, hctsiz);

	debug("%s: HCINT=%08x sub=%u toggle=%d\n", __func__, hcint, *sub,
	      *toggle);

	if (hcint & HCINTMSK_XFERCOMPL)
		return 0;

	if (hcint & (HCINTMSK_NAK | HCINTMSK_FRMOVRUN))
		return -EAGAIN;

	debug("%s: Error (HCINT=%08x)\n", __func__, hcint);
	return -EINVAL;
}

static int dwc2_eptype[] = {
	HCCHAR_EPTYPE_ISOC,
	HCCHAR_EPTYPE_INTR,
	HCCHAR_EPTYPE_CONTROL,
	HCCHAR_EPTYPE_BULK,
};

static int transfer_chunk(struct dwc2_hc_regs *hc_regs, void *aligned_buffer,
			  u8 *pid, int in, void *buffer, int num_packets,
			  int xfer_len, int *actual_len, int odd_frame)
{
	int ret = 0;
	u32 sub;

	debug("%s: chunk: pid %d xfer_len %u pkts %u\n", __func__,
	      *pid, xfer_len, num_packets);

	writel(FIELD_PREP(TSIZ_XFERSIZE_MASK, xfer_len) |
	       FIELD_PREP(TSIZ_PKTCNT_MASK, num_packets) |
	       FIELD_PREP(TSIZ_SC_MC_PID_MASK, *pid),
	       &hc_regs->hctsiz);

	if (xfer_len) {
		if (in) {
			invalidate_dcache_range(
					(uintptr_t)aligned_buffer,
					(uintptr_t)aligned_buffer +
					roundup(xfer_len, ARCH_DMA_MINALIGN));
		} else {
			memcpy(aligned_buffer, buffer, xfer_len);
			flush_dcache_range(
					(uintptr_t)aligned_buffer,
					(uintptr_t)aligned_buffer +
					roundup(xfer_len, ARCH_DMA_MINALIGN));
		}
	}

	writel(phys_to_bus((unsigned long)aligned_buffer), &hc_regs->hcdma);

	/* Clear old interrupt conditions for this host channel. */
	writel(0x3fff, &hc_regs->hcint);

	/* Set host channel enable after all other setup is complete. */
	clrsetbits_le32(&hc_regs->hcchar, HCCHAR_MULTICNT_MASK |
			HCCHAR_CHENA | HCCHAR_CHDIS |
			HCCHAR_ODDFRM,
			FIELD_PREP(HCCHAR_MULTICNT_MASK, 1) |
			FIELD_PREP(HCCHAR_ODDFRM, odd_frame) |
			HCCHAR_CHENA);

	ret = wait_for_chhltd(hc_regs, &sub, pid);
	if (ret < 0)
		return ret;

	if (in) {
		xfer_len -= sub;

		invalidate_dcache_range((unsigned long)aligned_buffer,
					(unsigned long)aligned_buffer +
					roundup(xfer_len, ARCH_DMA_MINALIGN));

		memcpy(buffer, aligned_buffer, xfer_len);
	}
	*actual_len = xfer_len;

	return ret;
}

int chunk_msg(struct dwc2_priv *priv, struct usb_device *dev,
	      unsigned long pipe, u8 *pid, int in, void *buffer, int len)
{
	struct dwc2_core_regs *regs = priv->regs;
	struct dwc2_hc_regs *hc_regs = &regs->host_regs.hc[DWC2_HC_CHANNEL];
	struct dwc2_host_regs *host_regs = &regs->host_regs;
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	int eptype = dwc2_eptype[usb_pipetype(pipe)];
	int done = 0;
	int ret = 0;
	int do_split = 0;
	int complete_split = 0;
	u32 xfer_len;
	u32 num_packets;
	int stop_transfer = 0;
	u32 max_xfer_len;
	int ssplit_frame_num = 0;

	debug("%s: msg: pipe %lx pid %d in %d len %d\n", __func__, pipe, *pid,
	      in, len);

	max_xfer_len = DWC2_MAX_PACKET_COUNT * max;
	if (max_xfer_len > DWC2_MAX_TRANSFER_SIZE)
		max_xfer_len = DWC2_MAX_TRANSFER_SIZE;
	if (max_xfer_len > DWC2_DATA_BUF_SIZE)
		max_xfer_len = DWC2_DATA_BUF_SIZE;

	/* Make sure that max_xfer_len is a multiple of max packet size. */
	num_packets = max_xfer_len / max;
	max_xfer_len = num_packets * max;

	/* Initialize channel */
	dwc_otg_hc_init(regs, DWC2_HC_CHANNEL, dev, devnum, ep, in,
			eptype, max);

	/* Check if the target is a FS/LS device behind a HS hub */
	if (dev->speed != USB_SPEED_HIGH) {
		u8 hub_addr;
		u8 hub_port;
		u32 hprt0 = readl(&regs->host_regs.hprt0);

		if (FIELD_GET(HPRT0_SPD_MASK, hprt0) == HPRT0_SPD_HIGH_SPEED) {
			usb_find_usb2_hub_address_port(dev, &hub_addr,
						       &hub_port);
			dwc_otg_hc_init_split(hc_regs, hub_addr, hub_port);

			do_split = 1;
			num_packets = 1;
			max_xfer_len = max;
		}
	}

	do {
		int actual_len = 0;
		u32 hcint;
		int odd_frame = 0;
		xfer_len = len - done;

		if (xfer_len > max_xfer_len)
			xfer_len = max_xfer_len;
		else if (xfer_len > max)
			num_packets = (xfer_len + max - 1) / max;
		else
			num_packets = 1;

		if (complete_split)
			setbits_le32(&hc_regs->hcsplt, HCSPLT_COMPSPLT);
		else if (do_split)
			clrbits_le32(&hc_regs->hcsplt, HCSPLT_COMPSPLT);

		if (eptype == HCCHAR_EPTYPE_INTR) {
			int uframe_num = readl(&host_regs->hfnum);
			if (!(uframe_num & 0x1))
				odd_frame = 1;
		}

		ret = transfer_chunk(hc_regs, priv->aligned_buffer, pid,
				     in, (char *)buffer + done, num_packets,
				     xfer_len, &actual_len, odd_frame);

		hcint = readl(&hc_regs->hcint);
		if (complete_split) {
			stop_transfer = 0;
			if (hcint & HCINTMSK_NYET) {
				ret = 0;
				int frame_num = FIELD_GET(HFNUM_FRNUM_MASK,
							  readl(&host_regs->hfnum));

				if (((frame_num - ssplit_frame_num) & HFNUM_FRNUM_MASK) > 4)
					ret = -EAGAIN;
			} else
				complete_split = 0;
		} else if (do_split) {
			if (hcint & HCINTMSK_ACK) {
				ssplit_frame_num = FIELD_GET(HFNUM_FRNUM_MASK,
							     readl(&host_regs->hfnum));
				ret = 0;
				complete_split = 1;
			}
		}

		if (ret)
			break;

		if (actual_len < xfer_len)
			stop_transfer = 1;

		done += actual_len;

	/* Transactions are done when when either all data is transferred or
	 * there is a short transfer. In case of a SPLIT make sure the CSPLIT
	 * is executed.
	 */
	} while (((done < len) && !stop_transfer) || complete_split);

	writel(0, &hc_regs->hcintmsk);
	writel(0xFFFFFFFF, &hc_regs->hcint);

	dev->status = 0;
	dev->act_len = done;

	return ret;
}

/* U-Boot USB transmission interface */
int _submit_bulk_msg(struct dwc2_priv *priv, struct usb_device *dev,
		     unsigned long pipe, void *buffer, int len)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	u8* pid;

	if ((devnum >= MAX_DEVICE) || (devnum == priv->root_hub_devnum)) {
		dev->status = 0;
		return -EINVAL;
	}

	if (usb_pipein(pipe))
		pid = &priv->in_data_toggle[devnum][ep];
	else
		pid = &priv->out_data_toggle[devnum][ep];

	return chunk_msg(priv, dev, pipe, pid, usb_pipein(pipe), buffer, len);
}

static int _submit_control_msg(struct dwc2_priv *priv, struct usb_device *dev,
			       unsigned long pipe, void *buffer, int len,
			       struct devrequest *setup)
{
	int devnum = usb_pipedevice(pipe);
	int ret, act_len;
	u8 pid;
	/* For CONTROL endpoint pid should start with DATA1 */
	int status_direction;

	if (devnum == priv->root_hub_devnum) {
		dev->status = 0;
		dev->speed = USB_SPEED_HIGH;
		return dwc_otg_submit_rh_msg(priv, dev, pipe, buffer, len,
					     setup);
	}

	/* SETUP stage */
	pid = DWC2_HC_PID_SETUP;
	do {
		ret = chunk_msg(priv, dev, pipe, &pid, 0, setup, 8);
	} while (ret == -EAGAIN);
	if (ret)
		return ret;

	/* DATA stage */
	act_len = 0;
	if (buffer) {
		pid = DWC2_HC_PID_DATA1;
		do {
			ret = chunk_msg(priv, dev, pipe, &pid, usb_pipein(pipe),
					buffer, len);
			act_len += dev->act_len;
			buffer += dev->act_len;
			len -= dev->act_len;
		} while (ret == -EAGAIN);
		if (ret)
			return ret;
		status_direction = usb_pipeout(pipe);
	} else {
		/* No-data CONTROL always ends with an IN transaction */
		status_direction = 1;
	}

	/* STATUS stage */
	pid = DWC2_HC_PID_DATA1;
	do {
		ret = chunk_msg(priv, dev, pipe, &pid, status_direction,
				priv->status_buffer, 0);
	} while (ret == -EAGAIN);
	if (ret)
		return ret;

	dev->act_len = act_len;

	return 0;
}

int _submit_int_msg(struct dwc2_priv *priv, struct usb_device *dev,
		    unsigned long pipe, void *buffer, int len, int interval,
		    bool nonblock)
{
	unsigned long timeout;
	int ret;

	/* FIXME: what is interval? */

	timeout = get_timer(0) + USB_TIMEOUT_MS(pipe);
	for (;;) {
		if (get_timer(0) > timeout) {
#if CONFIG_IS_ENABLED(DM_USB)
			dev_err(dev->dev,
				"Timeout poll on interrupt endpoint\n");
#else
			log_err("Timeout poll on interrupt endpoint\n");
#endif
			return -ETIMEDOUT;
		}
		ret = _submit_bulk_msg(priv, dev, pipe, buffer, len);
		if ((ret != -EAGAIN) || nonblock)
			return ret;
	}
}

static int dwc2_reset(struct udevice *dev)
{
	int ret;
	struct dwc2_priv *priv = dev_get_priv(dev);

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret) {
		dev_warn(dev, "Can't get reset: %d\n", ret);
		/* Return 0 if error due to !CONFIG_DM_RESET and reset
		 * DT property is not present.
		 */
		if (ret == -ENOENT || ret == -ENOTSUPP)
			return 0;
		else
			return ret;
	}

	/* force reset to clear all IP register */
	reset_assert_bulk(&priv->resets);
	ret = reset_deassert_bulk(&priv->resets);
	if (ret) {
		reset_release_bulk(&priv->resets);
		dev_err(dev, "Failed to reset: %d\n", ret);
		return ret;
	}

	return 0;
}

static int dwc2_init_common(struct udevice *dev, struct dwc2_priv *priv)
{
	struct dwc2_core_regs *regs = priv->regs;
	u32 snpsid;
	int i, j;
	int ret;

	ret = dwc2_reset(dev);
	if (ret)
		return ret;

	snpsid = readl(&regs->global_regs.gsnpsid);
	dev_info(dev, "Core Release: %x.%03x\n",
		 snpsid >> 12 & 0xf, snpsid & 0xfff);

	if (FIELD_GET(GSNPSID_ID_MASK, snpsid) != GSNPSID_OTG_ID) {
		dev_info(dev, "SNPSID invalid (not DWC2 OTG device): %08x\n",
			 snpsid);
		return -ENODEV;
	}

#ifdef DWC2_PHY_ULPI_EXT_VBUS
	priv->ext_vbus = 1;
#else
	priv->ext_vbus = 0;
#endif

	dwc_otg_core_init(dev);

	if (usb_get_dr_mode(dev_ofnode(dev)) == USB_DR_MODE_PERIPHERAL) {
		dev_dbg(dev, "USB device %s dr_mode set to %d. Skipping host_init.\n",
			dev->name, usb_get_dr_mode(dev_ofnode(dev)));
	} else {
		dwc_otg_core_host_init(dev, regs);
	}

	clrsetbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK, HPRT0_RST);
	mdelay(50);
	clrbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK | HPRT0_RST);

	for (i = 0; i < MAX_DEVICE; i++) {
		for (j = 0; j < MAX_ENDPOINT; j++) {
			priv->in_data_toggle[i][j] = DWC2_HC_PID_DATA0;
			priv->out_data_toggle[i][j] = DWC2_HC_PID_DATA0;
		}
	}

	/*
	 * Add a 1 second delay here. This gives the host controller
	 * a bit time before the comminucation with the USB devices
	 * is started (the bus is scanned) and  fixes the USB detection
	 * problems with some problematic USB keys.
	 */
	if (readl(&regs->global_regs.gintsts) & GINTSTS_CURMODE_HOST)
		mdelay(1000);

	printf("USB DWC2\n");

	return 0;
}

static void dwc2_uninit_common(struct dwc2_core_regs *regs)
{
	/* Put everything in reset. */
	clrsetbits_le32(&regs->host_regs.hprt0, HPRT0_W1C_MASK, HPRT0_RST);
}

#if !CONFIG_IS_ENABLED(DM_USB)
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len, struct devrequest *setup)
{
	return _submit_control_msg(&local, dev, pipe, buffer, len, setup);
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
	return _submit_bulk_msg(&local, dev, pipe, buffer, len);
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval, bool nonblock)
{
	return _submit_int_msg(&local, dev, pipe, buffer, len, interval,
			       nonblock);
}

/* U-Boot USB control interface */
int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	struct dwc2_priv *priv = &local;

	memset(priv, '\0', sizeof(*priv));
	priv->root_hub_devnum = 0;
	priv->regs = (struct dwc2_core_regs *)CONFIG_USB_DWC2_REG_ADDR;
	priv->aligned_buffer = aligned_buffer_addr;
	priv->status_buffer = status_buffer_addr;

	/* board-dependant init */
	if (board_usb_init(index, USB_INIT_HOST))
		return -1;

	return dwc2_init_common(NULL, priv);
}

int usb_lowlevel_stop(int index)
{
	dwc2_uninit_common(local.regs);

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(DM_USB)
static int dwc2_submit_control_msg(struct udevice *dev, struct usb_device *udev,
				   unsigned long pipe, void *buffer, int length,
				   struct devrequest *setup)
{
	struct dwc2_priv *priv = dev_get_priv(dev);

	debug("%s: dev='%s', udev=%p, udev->dev='%s', portnr=%d\n", __func__,
	      dev->name, udev, udev->dev->name, udev->portnr);

	return _submit_control_msg(priv, udev, pipe, buffer, length, setup);
}

static int dwc2_submit_bulk_msg(struct udevice *dev, struct usb_device *udev,
				unsigned long pipe, void *buffer, int length)
{
	struct dwc2_priv *priv = dev_get_priv(dev);

	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);

	return _submit_bulk_msg(priv, udev, pipe, buffer, length);
}

static int dwc2_submit_int_msg(struct udevice *dev, struct usb_device *udev,
			       unsigned long pipe, void *buffer, int length,
			       int interval, bool nonblock)
{
	struct dwc2_priv *priv = dev_get_priv(dev);

	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);

	return _submit_int_msg(priv, udev, pipe, buffer, length, interval,
			       nonblock);
}

static int dwc2_usb_of_to_plat(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->oc_disable = dev_read_bool(dev, "disable-over-current");
	priv->hnp_srp_disable = dev_read_bool(dev, "hnp-srp-disable");

	return 0;
}

static int dwc2_setup_phy(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = generic_phy_get_by_index(dev, 0, &priv->phy);
	if (ret) {
		if (ret == -ENOENT)
			return 0; /* no PHY, nothing to do */
		dev_err(dev, "Failed to get USB PHY: %d.\n", ret);
		return ret;
	}

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_dbg(dev, "Failed to init USB PHY: %d.\n", ret);
		return ret;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		dev_dbg(dev, "Failed to power on USB PHY: %d.\n", ret);
		generic_phy_exit(&priv->phy);
		return ret;
	}

	return 0;
}

static int dwc2_shutdown_phy(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	/* PHY is not valid when generic_phy_get_by_index() = -ENOENT */
	if (!generic_phy_valid(&priv->phy))
		return 0; /* no PHY, nothing to do */

	ret = generic_phy_power_off(&priv->phy);
	if (ret) {
		dev_dbg(dev, "Failed to power off USB PHY: %d.\n", ret);
		return ret;
	}

	ret = generic_phy_exit(&priv->phy);
	if (ret) {
		dev_dbg(dev, "Failed to power off USB PHY: %d.\n", ret);
		return ret;
	}

	return 0;
}

static int dwc2_clk_init(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret == -ENOSYS || ret == -ENOENT)
		return 0;
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		clk_release_bulk(&priv->clks);
		return ret;
	}

	return 0;
}

static int dwc2_usb_probe(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	struct usb_bus_priv *bus_priv = dev_get_uclass_priv(dev);
	int ret;

	bus_priv->desc_before_addr = true;

	ret = dwc2_clk_init(dev);
	if (ret)
		return ret;

	ret = dwc2_setup_phy(dev);
	if (ret)
		return ret;

	return dwc2_init_common(dev, priv);
}

static int dwc2_usb_remove(struct udevice *dev)
{
	struct dwc2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dwc_vbus_supply_exit(dev);
	if (ret)
		return ret;

	ret = dwc2_shutdown_phy(dev);
	if (ret) {
		dev_dbg(dev, "Failed to shutdown USB PHY: %d.\n", ret);
		return ret;
	}

	dwc2_uninit_common(priv->regs);

	reset_release_bulk(&priv->resets);
	clk_disable_bulk(&priv->clks);
	clk_release_bulk(&priv->clks);

	return 0;
}

struct dm_usb_ops dwc2_usb_ops = {
	.control = dwc2_submit_control_msg,
	.bulk = dwc2_submit_bulk_msg,
	.interrupt = dwc2_submit_int_msg,
};

static const struct udevice_id dwc2_usb_ids[] = {
	{ .compatible = "brcm,bcm2835-usb" },
	{ .compatible = "brcm,bcm2708-usb" },
	{ .compatible = "snps,dwc2" },
	{ }
};

U_BOOT_DRIVER(usb_dwc2) = {
	.name	= "dwc2_usb",
	.id	= UCLASS_USB,
	.of_match = dwc2_usb_ids,
	.of_to_plat = dwc2_usb_of_to_plat,
	.probe	= dwc2_usb_probe,
	.remove = dwc2_usb_remove,
	.ops	= &dwc2_usb_ops,
	.priv_auto	= sizeof(struct dwc2_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
