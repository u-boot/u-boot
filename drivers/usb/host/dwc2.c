/*
 * Copyright (C) 2012 Oleksandr Tymoshenko <gonzo@freebsd.org>
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <usb.h>
#include <malloc.h>
#include <usbroothubdes.h>
#include <asm/io.h>

#include "dwc2.h"

/* Use only HC channel 0. */
#define DWC2_HC_CHANNEL			0

#define DWC2_STATUS_BUF_SIZE		64
#define DWC2_DATA_BUF_SIZE		(64 * 1024)

/* We need doubleword-aligned buffers for DMA transfers */
DEFINE_ALIGN_BUFFER(uint8_t, aligned_buffer, DWC2_DATA_BUF_SIZE, 8);
DEFINE_ALIGN_BUFFER(uint8_t, status_buffer, DWC2_STATUS_BUF_SIZE, 8);

#define MAX_DEVICE			16
#define MAX_ENDPOINT			16
static int bulk_data_toggle[MAX_DEVICE][MAX_ENDPOINT];
static int control_data_toggle[MAX_DEVICE][MAX_ENDPOINT];

static int root_hub_devnum;

static struct dwc2_core_regs *regs =
	(struct dwc2_core_regs *)CONFIG_USB_DWC2_REG_ADDR;

/*
 * DWC2 IP interface
 */
static int wait_for_bit(void *reg, const uint32_t mask, bool set)
{
	unsigned int timeout = 1000000;
	uint32_t val;

	while (--timeout) {
		val = readl(reg);
		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		udelay(1);
	}

	debug("%s: Timeout (reg=%p mask=%08x wait_set=%i)\n",
	      __func__, reg, mask, set);

	return -ETIMEDOUT;
}

/*
 * Initializes the FSLSPClkSel field of the HCFG register
 * depending on the PHY type.
 */
static void init_fslspclksel(struct dwc2_core_regs *regs)
{
	uint32_t phyclk;

#if (CONFIG_DWC2_PHY_TYPE == DWC2_PHY_TYPE_FS)
	phyclk = DWC2_HCFG_FSLSPCLKSEL_48_MHZ;	/* Full speed PHY */
#else
	/* High speed PHY running at full speed or high speed */
	phyclk = DWC2_HCFG_FSLSPCLKSEL_30_60_MHZ;
#endif

#ifdef CONFIG_DWC2_ULPI_FS_LS
	uint32_t hwcfg2 = readl(&regs->ghwcfg2);
	uint32_t hval = (ghwcfg2 & DWC2_HWCFG2_HS_PHY_TYPE_MASK) >>
			DWC2_HWCFG2_HS_PHY_TYPE_OFFSET;
	uint32_t fval = (ghwcfg2 & DWC2_HWCFG2_FS_PHY_TYPE_MASK) >>
			DWC2_HWCFG2_FS_PHY_TYPE_OFFSET;

	if (hval == 2 && fval == 1)
		phyclk = DWC2_HCFG_FSLSPCLKSEL_48_MHZ;	/* Full speed PHY */
#endif

	clrsetbits_le32(&regs->host_regs.hcfg,
			DWC2_HCFG_FSLSPCLKSEL_MASK,
			phyclk << DWC2_HCFG_FSLSPCLKSEL_OFFSET);
}

/*
 * Flush a Tx FIFO.
 *
 * @param regs Programming view of DWC_otg controller.
 * @param num Tx FIFO to flush.
 */
static void dwc_otg_flush_tx_fifo(struct dwc2_core_regs *regs, const int num)
{
	int ret;

	writel(DWC2_GRSTCTL_TXFFLSH | (num << DWC2_GRSTCTL_TXFNUM_OFFSET),
	       &regs->grstctl);
	ret = wait_for_bit(&regs->grstctl, DWC2_GRSTCTL_TXFFLSH, 0);
	if (ret)
		printf("%s: Timeout!\n", __func__);

	/* Wait for 3 PHY Clocks */
	udelay(1);
}

/*
 * Flush Rx FIFO.
 *
 * @param regs Programming view of DWC_otg controller.
 */
static void dwc_otg_flush_rx_fifo(struct dwc2_core_regs *regs)
{
	int ret;

	writel(DWC2_GRSTCTL_RXFFLSH, &regs->grstctl);
	ret = wait_for_bit(&regs->grstctl, DWC2_GRSTCTL_RXFFLSH, 0);
	if (ret)
		printf("%s: Timeout!\n", __func__);

	/* Wait for 3 PHY Clocks */
	udelay(1);
}

/*
 * Do core a soft reset of the core.  Be careful with this because it
 * resets all the internal state machines of the core.
 */
static void dwc_otg_core_reset(struct dwc2_core_regs *regs)
{
	int ret;

	/* Wait for AHB master IDLE state. */
	ret = wait_for_bit(&regs->grstctl, DWC2_GRSTCTL_AHBIDLE, 1);
	if (ret)
		printf("%s: Timeout!\n", __func__);

	/* Core Soft Reset */
	writel(DWC2_GRSTCTL_CSFTRST, &regs->grstctl);
	ret = wait_for_bit(&regs->grstctl, DWC2_GRSTCTL_CSFTRST, 0);
	if (ret)
		printf("%s: Timeout!\n", __func__);

	/*
	 * Wait for core to come out of reset.
	 * NOTE: This long sleep is _very_ important, otherwise the core will
	 *       not stay in host mode after a connector ID change!
	 */
	mdelay(100);
}

/*
 * This function initializes the DWC_otg controller registers for
 * host mode.
 *
 * This function flushes the Tx and Rx FIFOs and it flushes any entries in the
 * request queues. Host channels are reset to ensure that they are ready for
 * performing transfers.
 *
 * @param regs Programming view of DWC_otg controller
 *
 */
static void dwc_otg_core_host_init(struct dwc2_core_regs *regs)
{
	uint32_t nptxfifosize = 0;
	uint32_t ptxfifosize = 0;
	uint32_t hprt0 = 0;
	int i, ret, num_channels;

	/* Restart the Phy Clock */
	writel(0, &regs->pcgcctl);

	/* Initialize Host Configuration Register */
	init_fslspclksel(regs);
#ifdef CONFIG_DWC2_DFLT_SPEED_FULL
	setbits_le32(&regs->host_regs.hcfg, DWC2_HCFG_FSLSSUPP);
#endif

	/* Configure data FIFO sizes */
#ifdef CONFIG_DWC2_ENABLE_DYNAMIC_FIFO
	if (readl(&regs->ghwcfg2) & DWC2_HWCFG2_DYNAMIC_FIFO) {
		/* Rx FIFO */
		writel(CONFIG_DWC2_HOST_RX_FIFO_SIZE, &regs->grxfsiz);

		/* Non-periodic Tx FIFO */
		nptxfifosize |= CONFIG_DWC2_HOST_NPERIO_TX_FIFO_SIZE <<
				DWC2_FIFOSIZE_DEPTH_OFFSET;
		nptxfifosize |= CONFIG_DWC2_HOST_RX_FIFO_SIZE <<
				DWC2_FIFOSIZE_STARTADDR_OFFSET;
		writel(nptxfifosize, &regs->gnptxfsiz);

		/* Periodic Tx FIFO */
		ptxfifosize |= CONFIG_DWC2_HOST_PERIO_TX_FIFO_SIZE <<
				DWC2_FIFOSIZE_DEPTH_OFFSET;
		ptxfifosize |= (CONFIG_DWC2_HOST_RX_FIFO_SIZE +
				CONFIG_DWC2_HOST_NPERIO_TX_FIFO_SIZE) <<
				DWC2_FIFOSIZE_STARTADDR_OFFSET;
		writel(ptxfifosize, &regs->hptxfsiz);
	}
#endif

	/* Clear Host Set HNP Enable in the OTG Control Register */
	clrbits_le32(&regs->gotgctl, DWC2_GOTGCTL_HSTSETHNPEN);

	/* Make sure the FIFOs are flushed. */
	dwc_otg_flush_tx_fifo(regs, 0x10);	/* All Tx FIFOs */
	dwc_otg_flush_rx_fifo(regs);

	/* Flush out any leftover queued requests. */
	num_channels = readl(&regs->ghwcfg2);
	num_channels &= DWC2_HWCFG2_NUM_HOST_CHAN_MASK;
	num_channels >>= DWC2_HWCFG2_NUM_HOST_CHAN_OFFSET;
	num_channels += 1;

	for (i = 0; i < num_channels; i++)
		clrsetbits_le32(&regs->hc_regs[i].hcchar,
				DWC2_HCCHAR_CHEN | DWC2_HCCHAR_EPDIR,
				DWC2_HCCHAR_CHDIS);

	/* Halt all channels to put them into a known state. */
	for (i = 0; i < num_channels; i++) {
		clrsetbits_le32(&regs->hc_regs[i].hcchar,
				DWC2_HCCHAR_EPDIR,
				DWC2_HCCHAR_CHEN | DWC2_HCCHAR_CHDIS);
		ret = wait_for_bit(&regs->hc_regs[i].hcchar,
				   DWC2_HCCHAR_CHEN, 0);
		if (ret)
			printf("%s: Timeout!\n", __func__);
	}

	/* Turn on the vbus power. */
	if (readl(&regs->gintsts) & DWC2_GINTSTS_CURMODE_HOST) {
		hprt0 = readl(&regs->hprt0);
		hprt0 &= ~(DWC2_HPRT0_PRTENA | DWC2_HPRT0_PRTCONNDET);
		hprt0 &= ~(DWC2_HPRT0_PRTENCHNG | DWC2_HPRT0_PRTOVRCURRCHNG);
		if (!(hprt0 & DWC2_HPRT0_PRTPWR)) {
			hprt0 |= DWC2_HPRT0_PRTPWR;
			writel(hprt0, &regs->hprt0);
		}
	}
}

/*
 * This function initializes the DWC_otg controller registers and
 * prepares the core for device mode or host mode operation.
 *
 * @param regs Programming view of the DWC_otg controller
 */
static void dwc_otg_core_init(struct dwc2_core_regs *regs)
{
	uint32_t ahbcfg = 0;
	uint32_t usbcfg = 0;
	uint8_t brst_sz = CONFIG_DWC2_DMA_BURST_SIZE;

	/* Common Initialization */
	usbcfg = readl(&regs->gusbcfg);

	/* Program the ULPI External VBUS bit if needed */
#ifdef CONFIG_DWC2_PHY_ULPI_EXT_VBUS
	usbcfg |= DWC2_GUSBCFG_ULPI_EXT_VBUS_DRV;
#else
	usbcfg &= ~DWC2_GUSBCFG_ULPI_EXT_VBUS_DRV;
#endif

	/* Set external TS Dline pulsing */
#ifdef CONFIG_DWC2_TS_DLINE
	usbcfg |= DWC2_GUSBCFG_TERM_SEL_DL_PULSE;
#else
	usbcfg &= ~DWC2_GUSBCFG_TERM_SEL_DL_PULSE;
#endif
	writel(usbcfg, &regs->gusbcfg);

	/* Reset the Controller */
	dwc_otg_core_reset(regs);

	/*
	 * This programming sequence needs to happen in FS mode before
	 * any other programming occurs
	 */
#if defined(CONFIG_DWC2_DFLT_SPEED_FULL) && \
	(CONFIG_DWC2_PHY_TYPE == DWC2_PHY_TYPE_FS)
	/* If FS mode with FS PHY */
	setbits_le32(&regs->gusbcfg, DWC2_GUSBCFG_PHYSEL);

	/* Reset after a PHY select */
	dwc_otg_core_reset(regs);

	/*
	 * Program DCFG.DevSpd or HCFG.FSLSPclkSel to 48Mhz in FS.
	 * Also do this on HNP Dev/Host mode switches (done in dev_init
	 * and host_init).
	 */
	if (readl(&regs->gintsts) & DWC2_GINTSTS_CURMODE_HOST)
		init_fslspclksel(regs);

#ifdef CONFIG_DWC2_I2C_ENABLE
	/* Program GUSBCFG.OtgUtmifsSel to I2C */
	setbits_le32(&regs->gusbcfg, DWC2_GUSBCFG_OTGUTMIFSSEL);

	/* Program GI2CCTL.I2CEn */
	clrsetbits_le32(&regs->gi2cctl, DWC2_GI2CCTL_I2CEN |
			DWC2_GI2CCTL_I2CDEVADDR_MASK,
			1 << DWC2_GI2CCTL_I2CDEVADDR_OFFSET);
	setbits_le32(&regs->gi2cctl, DWC2_GI2CCTL_I2CEN);
#endif

#else
	/* High speed PHY. */

	/*
	 * HS PHY parameters. These parameters are preserved during
	 * soft reset so only program the first time. Do a soft reset
	 * immediately after setting phyif.
	 */
	usbcfg &= ~(DWC2_GUSBCFG_ULPI_UTMI_SEL | DWC2_GUSBCFG_PHYIF);
	usbcfg |= CONFIG_DWC2_PHY_TYPE << DWC2_GUSBCFG_ULPI_UTMI_SEL_OFFSET;

	if (usbcfg & DWC2_GUSBCFG_ULPI_UTMI_SEL) {	/* ULPI interface */
#ifdef CONFIG_DWC2_PHY_ULPI_DDR
		usbcfg |= DWC2_GUSBCFG_DDRSEL;
#else
		usbcfg &= ~DWC2_GUSBCFG_DDRSEL;
#endif
	} else {	/* UTMI+ interface */
#if (CONFIG_DWC2_UTMI_PHY_WIDTH == 16)
		usbcfg |= DWC2_GUSBCFG_PHYIF;
#endif
	}

	writel(usbcfg, &regs->gusbcfg);

	/* Reset after setting the PHY parameters */
	dwc_otg_core_reset(regs);
#endif

	usbcfg = readl(&regs->gusbcfg);
	usbcfg &= ~(DWC2_GUSBCFG_ULPI_FSLS | DWC2_GUSBCFG_ULPI_CLK_SUS_M);
#ifdef CONFIG_DWC2_ULPI_FS_LS
	uint32_t hwcfg2 = readl(&regs->ghwcfg2);
	uint32_t hval = (ghwcfg2 & DWC2_HWCFG2_HS_PHY_TYPE_MASK) >>
			DWC2_HWCFG2_HS_PHY_TYPE_OFFSET;
	uint32_t fval = (ghwcfg2 & DWC2_HWCFG2_FS_PHY_TYPE_MASK) >>
			DWC2_HWCFG2_FS_PHY_TYPE_OFFSET;
	if (hval == 2 && fval == 1) {
		usbcfg |= DWC2_GUSBCFG_ULPI_FSLS;
		usbcfg |= DWC2_GUSBCFG_ULPI_CLK_SUS_M;
	}
#endif
	writel(usbcfg, &regs->gusbcfg);

	/* Program the GAHBCFG Register. */
	switch (readl(&regs->ghwcfg2) & DWC2_HWCFG2_ARCHITECTURE_MASK) {
	case DWC2_HWCFG2_ARCHITECTURE_SLAVE_ONLY:
		break;
	case DWC2_HWCFG2_ARCHITECTURE_EXT_DMA:
		while (brst_sz > 1) {
			ahbcfg |= ahbcfg + (1 << DWC2_GAHBCFG_HBURSTLEN_OFFSET);
			ahbcfg &= DWC2_GAHBCFG_HBURSTLEN_MASK;
			brst_sz >>= 1;
		}

#ifdef CONFIG_DWC2_DMA_ENABLE
		ahbcfg |= DWC2_GAHBCFG_DMAENABLE;
#endif
		break;

	case DWC2_HWCFG2_ARCHITECTURE_INT_DMA:
		ahbcfg |= DWC2_GAHBCFG_HBURSTLEN_INCR4;
#ifdef CONFIG_DWC2_DMA_ENABLE
		ahbcfg |= DWC2_GAHBCFG_DMAENABLE;
#endif
		break;
	}

	writel(ahbcfg, &regs->gahbcfg);

	/* Program the GUSBCFG register for HNP/SRP. */
	setbits_le32(&regs->gusbcfg, DWC2_GUSBCFG_HNPCAP | DWC2_GUSBCFG_SRPCAP);

#ifdef CONFIG_DWC2_IC_USB_CAP
	setbits_le32(&regs->gusbcfg, DWC2_GUSBCFG_IC_USB_CAP);
#endif
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
static void dwc_otg_hc_init(struct dwc2_core_regs *regs, uint8_t hc_num,
		uint8_t dev_addr, uint8_t ep_num, uint8_t ep_is_in,
		uint8_t ep_type, uint16_t max_packet)
{
	struct dwc2_hc_regs *hc_regs = &regs->hc_regs[hc_num];
	const uint32_t hcchar = (dev_addr << DWC2_HCCHAR_DEVADDR_OFFSET) |
				(ep_num << DWC2_HCCHAR_EPNUM_OFFSET) |
				(ep_is_in << DWC2_HCCHAR_EPDIR_OFFSET) |
				(ep_type << DWC2_HCCHAR_EPTYPE_OFFSET) |
				(max_packet << DWC2_HCCHAR_MPS_OFFSET);

	/* Clear old interrupt conditions for this host channel. */
	writel(0x3fff, &hc_regs->hcint);

	/*
	 * Program the HCCHARn register with the endpoint characteristics
	 * for the current transfer.
	 */
	writel(hcchar, &hc_regs->hcchar);

	/* Program the HCSPLIT register for SPLITs */
	writel(0, &hc_regs->hcsplt);
}

/*
 * DWC2 to USB API interface
 */
/* Direction: In ; Request: Status */
static int dwc_otg_submit_rh_msg_in_status(struct usb_device *dev, void *buffer,
					   int txlen, struct devrequest *cmd)
{
	uint32_t hprt0 = 0;
	uint32_t port_status = 0;
	uint32_t port_change = 0;
	int len = 0;
	int stat = 0;

	switch (cmd->requesttype & ~USB_DIR_IN) {
	case 0:
		*(uint16_t *)buffer = cpu_to_le16(1);
		len = 2;
		break;
	case USB_RECIP_INTERFACE:
	case USB_RECIP_ENDPOINT:
		*(uint16_t *)buffer = cpu_to_le16(0);
		len = 2;
		break;
	case USB_TYPE_CLASS:
		*(uint32_t *)buffer = cpu_to_le32(0);
		len = 4;
		break;
	case USB_RECIP_OTHER | USB_TYPE_CLASS:
		hprt0 = readl(&regs->hprt0);
		if (hprt0 & DWC2_HPRT0_PRTCONNSTS)
			port_status |= USB_PORT_STAT_CONNECTION;
		if (hprt0 & DWC2_HPRT0_PRTENA)
			port_status |= USB_PORT_STAT_ENABLE;
		if (hprt0 & DWC2_HPRT0_PRTSUSP)
			port_status |= USB_PORT_STAT_SUSPEND;
		if (hprt0 & DWC2_HPRT0_PRTOVRCURRACT)
			port_status |= USB_PORT_STAT_OVERCURRENT;
		if (hprt0 & DWC2_HPRT0_PRTRST)
			port_status |= USB_PORT_STAT_RESET;
		if (hprt0 & DWC2_HPRT0_PRTPWR)
			port_status |= USB_PORT_STAT_POWER;

		port_status |= USB_PORT_STAT_HIGH_SPEED;

		if (hprt0 & DWC2_HPRT0_PRTENCHNG)
			port_change |= USB_PORT_STAT_C_ENABLE;
		if (hprt0 & DWC2_HPRT0_PRTCONNDET)
			port_change |= USB_PORT_STAT_C_CONNECTION;
		if (hprt0 & DWC2_HPRT0_PRTOVRCURRCHNG)
			port_change |= USB_PORT_STAT_C_OVERCURRENT;

		*(uint32_t *)buffer = cpu_to_le32(port_status |
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
	uint32_t dsc;
	int len = 0;
	int stat = 0;
	uint16_t wValue = cpu_to_le16(cmd->value);
	uint16_t wLength = cpu_to_le16(cmd->length);

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
		*(uint8_t *)buffer = 0x01;
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
static int dwc_otg_submit_rh_msg_in(struct usb_device *dev,
				 void *buffer, int txlen,
				 struct devrequest *cmd)
{
	switch (cmd->request) {
	case USB_REQ_GET_STATUS:
		return dwc_otg_submit_rh_msg_in_status(dev, buffer,
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
static int dwc_otg_submit_rh_msg_out(struct usb_device *dev,
				 void *buffer, int txlen,
				 struct devrequest *cmd)
{
	int len = 0;
	int stat = 0;
	uint16_t bmrtype_breq = cmd->requesttype | (cmd->request << 8);
	uint16_t wValue = cpu_to_le16(cmd->value);

	switch (bmrtype_breq & ~USB_DIR_IN) {
	case (USB_REQ_CLEAR_FEATURE << 8) | USB_RECIP_ENDPOINT:
	case (USB_REQ_CLEAR_FEATURE << 8) | USB_TYPE_CLASS:
		break;

	case (USB_REQ_CLEAR_FEATURE << 8) | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_C_CONNECTION:
			setbits_le32(&regs->hprt0, DWC2_HPRT0_PRTCONNDET);
			break;
		}
		break;

	case (USB_REQ_SET_FEATURE << 8) | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			break;

		case USB_PORT_FEAT_RESET:
			clrsetbits_le32(&regs->hprt0, DWC2_HPRT0_PRTENA |
					DWC2_HPRT0_PRTCONNDET |
					DWC2_HPRT0_PRTENCHNG |
					DWC2_HPRT0_PRTOVRCURRCHNG,
					DWC2_HPRT0_PRTRST);
			mdelay(50);
			clrbits_le32(&regs->hprt0, DWC2_HPRT0_PRTRST);
			break;

		case USB_PORT_FEAT_POWER:
			clrsetbits_le32(&regs->hprt0, DWC2_HPRT0_PRTENA |
					DWC2_HPRT0_PRTCONNDET |
					DWC2_HPRT0_PRTENCHNG |
					DWC2_HPRT0_PRTOVRCURRCHNG,
					DWC2_HPRT0_PRTRST);
			break;

		case USB_PORT_FEAT_ENABLE:
			break;
		}
		break;
	case (USB_REQ_SET_ADDRESS << 8):
		root_hub_devnum = wValue;
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

static int dwc_otg_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
				 void *buffer, int txlen,
				 struct devrequest *cmd)
{
	int stat = 0;

	if (usb_pipeint(pipe)) {
		puts("Root-Hub submit IRQ: NOT implemented\n");
		return 0;
	}

	if (cmd->requesttype & USB_DIR_IN)
		stat = dwc_otg_submit_rh_msg_in(dev, buffer, txlen, cmd);
	else
		stat = dwc_otg_submit_rh_msg_out(dev, buffer, txlen, cmd);

	mdelay(1);

	return stat;
}

/* U-Boot USB transmission interface */
int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	int done = 0;
	uint32_t hctsiz, sub, tmp;
	struct dwc2_hc_regs *hc_regs = &regs->hc_regs[DWC2_HC_CHANNEL];
	uint32_t hcint;
	uint32_t xfer_len;
	uint32_t num_packets;
	int stop_transfer = 0;
	unsigned int timeout = 1000000;

	if (devnum == root_hub_devnum) {
		dev->status = 0;
		return -EINVAL;
	}

	if (len > DWC2_DATA_BUF_SIZE) {
		printf("%s: %d is more then available buffer size (%d)\n",
		       __func__, len, DWC2_DATA_BUF_SIZE);
		dev->status = 0;
		dev->act_len = 0;
		return -EINVAL;
	}

	while ((done < len) && !stop_transfer) {
		/* Initialize channel */
		dwc_otg_hc_init(regs, DWC2_HC_CHANNEL, devnum, ep,
				usb_pipein(pipe), DWC2_HCCHAR_EPTYPE_BULK, max);

		xfer_len = len - done;
		/* Make sure that xfer_len is a multiple of max packet size. */
		if (xfer_len > CONFIG_DWC2_MAX_TRANSFER_SIZE)
			xfer_len = CONFIG_DWC2_MAX_TRANSFER_SIZE - max + 1;

		if (xfer_len > 0) {
			num_packets = (xfer_len + max - 1) / max;
			if (num_packets > CONFIG_DWC2_MAX_PACKET_COUNT) {
				num_packets = CONFIG_DWC2_MAX_PACKET_COUNT;
				xfer_len = num_packets * max;
			}
		} else {
			num_packets = 1;
		}

		if (usb_pipein(pipe))
			xfer_len = num_packets * max;

		writel((xfer_len << DWC2_HCTSIZ_XFERSIZE_OFFSET) |
		       (num_packets << DWC2_HCTSIZ_PKTCNT_OFFSET) |
		       (bulk_data_toggle[devnum][ep] <<
				DWC2_HCTSIZ_PID_OFFSET),
		       &hc_regs->hctsiz);

		memcpy(aligned_buffer, (char *)buffer + done, len - done);
		writel((uint32_t)aligned_buffer, &hc_regs->hcdma);

		/* Set host channel enable after all other setup is complete. */
		clrsetbits_le32(&hc_regs->hcchar, DWC2_HCCHAR_MULTICNT_MASK |
				DWC2_HCCHAR_CHEN | DWC2_HCCHAR_CHDIS,
				(1 << DWC2_HCCHAR_MULTICNT_OFFSET) |
				DWC2_HCCHAR_CHEN);

		while (1) {
			hcint = readl(&hc_regs->hcint);

			if (!(hcint & DWC2_HCINT_CHHLTD))
				continue;

			if (hcint & DWC2_HCINT_XFERCOMP) {
				hctsiz = readl(&hc_regs->hctsiz);
				done += xfer_len;

				sub = hctsiz & DWC2_HCTSIZ_XFERSIZE_MASK;
				sub >>= DWC2_HCTSIZ_XFERSIZE_OFFSET;

				if (usb_pipein(pipe)) {
					done -= sub;
					if (hctsiz & DWC2_HCTSIZ_XFERSIZE_MASK)
						stop_transfer = 1;
				}

				tmp = hctsiz & DWC2_HCTSIZ_PID_MASK;
				tmp >>= DWC2_HCTSIZ_PID_OFFSET;
				if (tmp == DWC2_HC_PID_DATA1) {
					bulk_data_toggle[devnum][ep] =
						DWC2_HC_PID_DATA1;
				} else {
					bulk_data_toggle[devnum][ep] =
						DWC2_HC_PID_DATA0;
				}
				break;
			}

			if (hcint & DWC2_HCINT_STALL) {
				puts("DWC OTG: Channel halted\n");
				bulk_data_toggle[devnum][ep] =
					DWC2_HC_PID_DATA0;

				stop_transfer = 1;
				break;
			}

			if (!--timeout) {
				printf("%s: Timeout!\n", __func__);
				break;
			}
		}
	}

	if (done && usb_pipein(pipe))
		memcpy(buffer, aligned_buffer, done);

	writel(0, &hc_regs->hcintmsk);
	writel(0xFFFFFFFF, &hc_regs->hcint);

	dev->status = 0;
	dev->act_len = done;

	return 0;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len, struct devrequest *setup)
{
	struct dwc2_hc_regs *hc_regs = &regs->hc_regs[DWC2_HC_CHANNEL];
	int done = 0;
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	uint32_t hctsiz = 0, sub, tmp, ret;
	uint32_t hcint;
	const uint32_t hcint_comp_hlt_ack = DWC2_HCINT_XFERCOMP |
		DWC2_HCINT_CHHLTD | DWC2_HCINT_ACK;
	unsigned int timeout = 1000000;

	/* For CONTROL endpoint pid should start with DATA1 */
	int status_direction;

	if (devnum == root_hub_devnum) {
		dev->status = 0;
		dev->speed = USB_SPEED_HIGH;
		return dwc_otg_submit_rh_msg(dev, pipe, buffer, len, setup);
	}

	if (len > DWC2_DATA_BUF_SIZE) {
		printf("%s: %d is more then available buffer size(%d)\n",
		       __func__, len, DWC2_DATA_BUF_SIZE);
		dev->status = 0;
		dev->act_len = 0;
		return -EINVAL;
	}

	/* Initialize channel, OUT for setup buffer */
	dwc_otg_hc_init(regs, DWC2_HC_CHANNEL, devnum, ep, 0,
			DWC2_HCCHAR_EPTYPE_CONTROL, max);

	/* SETUP stage  */
	writel((8 << DWC2_HCTSIZ_XFERSIZE_OFFSET) |
	       (1 << DWC2_HCTSIZ_PKTCNT_OFFSET) |
	       (DWC2_HC_PID_SETUP << DWC2_HCTSIZ_PID_OFFSET),
	       &hc_regs->hctsiz);

	writel((uint32_t)setup, &hc_regs->hcdma);

	/* Set host channel enable after all other setup is complete. */
	clrsetbits_le32(&hc_regs->hcchar, DWC2_HCCHAR_MULTICNT_MASK |
			DWC2_HCCHAR_CHEN | DWC2_HCCHAR_CHDIS,
			(1 << DWC2_HCCHAR_MULTICNT_OFFSET) | DWC2_HCCHAR_CHEN);

	ret = wait_for_bit(&hc_regs->hcint, DWC2_HCINT_CHHLTD, 1);
	if (ret)
		printf("%s: Timeout!\n", __func__);

	hcint = readl(&hc_regs->hcint);

	if (!(hcint & DWC2_HCINT_CHHLTD) || !(hcint & DWC2_HCINT_XFERCOMP)) {
		printf("%s: Error (HCINT=%08x)\n", __func__, hcint);
		dev->status = 0;
		dev->act_len = 0;
		return -EINVAL;
	}

	/* Clear interrupts */
	writel(0, &hc_regs->hcintmsk);
	writel(0xFFFFFFFF, &hc_regs->hcint);

	if (buffer) {
		/* DATA stage */
		dwc_otg_hc_init(regs, DWC2_HC_CHANNEL, devnum, ep,
				usb_pipein(pipe),
				DWC2_HCCHAR_EPTYPE_CONTROL, max);

		/* TODO: check if len < 64 */
		control_data_toggle[devnum][ep] = DWC2_HC_PID_DATA1;
		writel((len << DWC2_HCTSIZ_XFERSIZE_OFFSET) |
		       (1 << DWC2_HCTSIZ_PKTCNT_OFFSET) |
		       (control_data_toggle[devnum][ep] <<
				DWC2_HCTSIZ_PID_OFFSET),
		       &hc_regs->hctsiz);

		writel((uint32_t)buffer, &hc_regs->hcdma);

		/* Set host channel enable after all other setup is complete */
		clrsetbits_le32(&hc_regs->hcchar, DWC2_HCCHAR_MULTICNT_MASK |
				DWC2_HCCHAR_CHEN | DWC2_HCCHAR_CHDIS,
				(1 << DWC2_HCCHAR_MULTICNT_OFFSET) |
				DWC2_HCCHAR_CHEN);

		while (1) {
			hcint = readl(&hc_regs->hcint);
			if (!(hcint & DWC2_HCINT_CHHLTD))
				continue;

			if (hcint & DWC2_HCINT_XFERCOMP) {
				hctsiz = readl(&hc_regs->hctsiz);
				done = len;

				sub = hctsiz & DWC2_HCTSIZ_XFERSIZE_MASK;
				sub >>= DWC2_HCTSIZ_XFERSIZE_OFFSET;

				if (usb_pipein(pipe))
					done -= sub;
			}

			if (hcint & DWC2_HCINT_ACK) {
				tmp = hctsiz & DWC2_HCTSIZ_PID_MASK;
				tmp >>= DWC2_HCTSIZ_PID_OFFSET;
				if (tmp == DWC2_HC_PID_DATA0) {
					control_data_toggle[devnum][ep] =
						DWC2_HC_PID_DATA0;
				} else {
					control_data_toggle[devnum][ep] =
						DWC2_HC_PID_DATA1;
				}
			}

			if (hcint != hcint_comp_hlt_ack) {
				printf("%s: Error (HCINT=%08x)\n",
				       __func__, hcint);
				goto out;
			}

			if (!--timeout) {
				printf("%s: Timeout!\n", __func__);
				goto out;
			}

			break;
		}
	} /* End of DATA stage */

	/* STATUS stage */
	if ((len == 0) || usb_pipeout(pipe))
		status_direction = 1;
	else
		status_direction = 0;

	dwc_otg_hc_init(regs, DWC2_HC_CHANNEL, devnum, ep,
			status_direction, DWC2_HCCHAR_EPTYPE_CONTROL, max);

	writel((1 << DWC2_HCTSIZ_PKTCNT_OFFSET) |
	       (DWC2_HC_PID_DATA1 << DWC2_HCTSIZ_PID_OFFSET),
	       &hc_regs->hctsiz);

	writel((uint32_t)status_buffer, &hc_regs->hcdma);

	/* Set host channel enable after all other setup is complete. */
	clrsetbits_le32(&hc_regs->hcchar, DWC2_HCCHAR_MULTICNT_MASK |
			DWC2_HCCHAR_CHEN | DWC2_HCCHAR_CHDIS,
			(1 << DWC2_HCCHAR_MULTICNT_OFFSET) | DWC2_HCCHAR_CHEN);

	while (1) {
		hcint = readl(&hc_regs->hcint);
		if (hcint & DWC2_HCINT_CHHLTD)
			break;
	}

	if (hcint != hcint_comp_hlt_ack)
		printf("%s: Error (HCINT=%08x)\n", __func__, hcint);

out:
	dev->act_len = done;
	dev->status = 0;

	return done;
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval)
{
	printf("dev = %p pipe = %#lx buf = %p size = %d int = %d\n",
	       dev, pipe, buffer, len, interval);
	return -ENOSYS;
}

/* U-Boot USB control interface */
int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	uint32_t snpsid;
	int i, j;

	root_hub_devnum = 0;

	snpsid = readl(&regs->gsnpsid);
	printf("Core Release: %x.%03x\n", snpsid >> 12 & 0xf, snpsid & 0xfff);

	if ((snpsid & DWC2_SNPSID_DEVID_MASK) != DWC2_SNPSID_DEVID_VER_2xx) {
		printf("SNPSID invalid (not DWC2 OTG device): %08x\n", snpsid);
		return -ENODEV;
	}

	dwc_otg_core_init(regs);
	dwc_otg_core_host_init(regs);

	clrsetbits_le32(&regs->hprt0, DWC2_HPRT0_PRTENA |
			DWC2_HPRT0_PRTCONNDET | DWC2_HPRT0_PRTENCHNG |
			DWC2_HPRT0_PRTOVRCURRCHNG,
			DWC2_HPRT0_PRTRST);
	mdelay(50);
	clrbits_le32(&regs->hprt0, DWC2_HPRT0_PRTENA | DWC2_HPRT0_PRTCONNDET |
		     DWC2_HPRT0_PRTENCHNG | DWC2_HPRT0_PRTOVRCURRCHNG |
		     DWC2_HPRT0_PRTRST);

	for (i = 0; i < MAX_DEVICE; i++) {
		for (j = 0; j < MAX_ENDPOINT; j++) {
			control_data_toggle[i][j] = DWC2_HC_PID_DATA1;
			bulk_data_toggle[i][j] = DWC2_HC_PID_DATA0;
		}
	}

	return 0;
}

int usb_lowlevel_stop(int index)
{
	/* Put everything in reset. */
	clrsetbits_le32(&regs->hprt0, DWC2_HPRT0_PRTENA |
			DWC2_HPRT0_PRTCONNDET | DWC2_HPRT0_PRTENCHNG |
			DWC2_HPRT0_PRTOVRCURRCHNG,
			DWC2_HPRT0_PRTRST);
	return 0;
}
