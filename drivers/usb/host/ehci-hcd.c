/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Excito Elektronik i Skåne AB
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/byteorder.h>
#include <usb.h>
#include <asm/io.h>
#include <malloc.h>
#include <watchdog.h>

#include "ehci.h"

int rootdev;
struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
volatile struct ehci_hcor *hcor;

static uint16_t portreset;
DEFINE_ALIGN_BUFFER(struct QH, qh_list, 1, USB_DMA_MINALIGN);

#define ALIGN_END_ADDR(type, ptr, size)			\
	((uint32_t)(ptr) + roundup((size) * sizeof(type), USB_DMA_MINALIGN))

static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_linux_config_descriptor config;
	struct usb_linux_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
}  __attribute__ ((packed)) descriptor = {
	{
		0x8,		/* bDescLength */
		0x29,		/* bDescriptorType: hub descriptor */
		2,		/* bNrPorts -- runtime modified */
		0,		/* wHubCharacteristics */
		10,		/* bPwrOn2PwrGood */
		0,		/* bHubCntrCurrent */
		{},		/* Device removable */
		{}		/* at most 7 ports! XXX */
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		cpu_to_le16(0x0200), /* bcdUSB: v2.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
		64,		/* bMaxPacketSize: 64 bytes */
		0x0000,		/* idVendor */
		0x0000,		/* idProduct */
		cpu_to_le16(0x0100), /* bcdDevice */
		1,		/* iManufacturer */
		2,		/* iProduct */
		0,		/* iSerialNumber */
		1		/* bNumConfigurations: 1 */
	},
	{
		0x9,
		2,		/* bDescriptorType: UDESC_CONFIG */
		cpu_to_le16(0x19),
		1,		/* bNumInterface */
		1,		/* bConfigurationValue */
		0,		/* iConfiguration */
		0x40,		/* bmAttributes: UC_SELF_POWER */
		0		/* bMaxPower */
	},
	{
		0x9,		/* bLength */
		4,		/* bDescriptorType: UDESC_INTERFACE */
		0,		/* bInterfaceNumber */
		0,		/* bAlternateSetting */
		1,		/* bNumEndpoints */
		9,		/* bInterfaceClass: UICLASS_HUB */
		0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
		0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
		0		/* iInterface */
	},
	{
		0x7,		/* bLength */
		5,		/* bDescriptorType: UDESC_ENDPOINT */
		0x81,		/* bEndpointAddress:
				 * UE_DIR_IN | EHCI_INTR_ENDPT
				 */
		3,		/* bmAttributes: UE_INTERRUPT */
		8,		/* wMaxPacketSize */
		255		/* bInterval */
	},
};

#if defined(CONFIG_EHCI_IS_TDI)
#define ehci_is_TDI()	(1)
#else
#define ehci_is_TDI()	(0)
#endif

void __ehci_powerup_fixup(uint32_t *status_reg, uint32_t *reg)
{
	mdelay(50);
}

void ehci_powerup_fixup(uint32_t *status_reg, uint32_t *reg)
	__attribute__((weak, alias("__ehci_powerup_fixup")));

static int handshake(uint32_t *ptr, uint32_t mask, uint32_t done, int usec)
{
	uint32_t result;
	do {
		result = ehci_readl(ptr);
		udelay(5);
		if (result == ~(uint32_t)0)
			return -1;
		result &= mask;
		if (result == done)
			return 0;
		usec--;
	} while (usec > 0);
	return -1;
}

static int ehci_reset(void)
{
	uint32_t cmd;
	uint32_t tmp;
	uint32_t *reg_ptr;
	int ret = 0;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd = (cmd & ~CMD_RUN) | CMD_RESET;
	ehci_writel(&hcor->or_usbcmd, cmd);
	ret = handshake((uint32_t *)&hcor->or_usbcmd, CMD_RESET, 0, 250 * 1000);
	if (ret < 0) {
		printf("EHCI fail to reset\n");
		goto out;
	}

	if (ehci_is_TDI()) {
		reg_ptr = (uint32_t *)((u8 *)hcor + USBMODE);
		tmp = ehci_readl(reg_ptr);
		tmp |= USBMODE_CM_HC;
#if defined(CONFIG_EHCI_MMIO_BIG_ENDIAN)
		tmp |= USBMODE_BE;
#endif
		ehci_writel(reg_ptr, tmp);
	}

#ifdef CONFIG_USB_EHCI_TXFIFO_THRESH
	cmd = ehci_readl(&hcor->or_txfilltuning);
	cmd &= ~TXFIFO_THRESH(0x3f);
	cmd |= TXFIFO_THRESH(CONFIG_USB_EHCI_TXFIFO_THRESH);
	ehci_writel(&hcor->or_txfilltuning, cmd);
#endif
out:
	return ret;
}

static int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t delta, next;
	uint32_t addr = (uint32_t)buf;
	size_t rsz = roundup(sz, 32);
	int idx;

	if (sz != rsz)
		debug("EHCI-HCD: Misaligned buffer size (%08x)\n", sz);

	if (addr & 31)
		debug("EHCI-HCD: Misaligned buffer address (%p)\n", buf);

	idx = 0;
	while (idx < 5) {
		flush_dcache_range(addr, addr + rsz);
		td->qt_buffer[idx] = cpu_to_hc32(addr);
		td->qt_buffer_hi[idx] = 0;
		next = (addr + 4096) & ~4095;
		delta = next - addr;
		if (delta >= sz)
			break;
		sz -= delta;
		addr = next;
		idx++;
	}

	if (idx == 5) {
		debug("out of buffer pointers (%u bytes left)\n", sz);
		return -1;
	}

	return 0;
}

static int
ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *req)
{
	ALLOC_ALIGN_BUFFER(struct QH, qh, 1, USB_DMA_MINALIGN);
	ALLOC_ALIGN_BUFFER(struct qTD, qtd, 3, USB_DMA_MINALIGN);
	int qtd_counter = 0;

	volatile struct qTD *vtd;
	unsigned long ts;
	uint32_t *tdp;
	uint32_t endpt, token, usbsts;
	uint32_t c, toggle;
	uint32_t cmd;
	int timeout;
	int ret = 0;

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe,
	      buffer, length, req);
	if (req != NULL)
		debug("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
		      req->request, req->request,
		      req->requesttype, req->requesttype,
		      le16_to_cpu(req->value), le16_to_cpu(req->value),
		      le16_to_cpu(req->index));

	memset(qh, 0, sizeof(struct QH));
	memset(qtd, 0, 3 * sizeof(*qtd));

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/*
	 * Setup QH (3.6 in ehci-r10.pdf)
	 *
	 *   qh_link ................. 03-00 H
	 *   qh_endpt1 ............... 07-04 H
	 *   qh_endpt2 ............... 0B-08 H
	 * - qh_curtd
	 *   qh_overlay.qt_next ...... 13-10 H
	 * - qh_overlay.qt_altnext
	 */
	qh->qh_link = cpu_to_hc32((uint32_t)qh_list | QH_LINK_TYPE_QH);
	c = (usb_pipespeed(pipe) != USB_SPEED_HIGH &&
	     usb_pipeendpoint(pipe) == 0) ? 1 : 0;
	endpt = (8 << 28) |
	    (c << 27) |
	    (usb_maxpacket(dev, pipe) << 16) |
	    (0 << 15) |
	    (1 << 14) |
	    (usb_pipespeed(pipe) << 12) |
	    (usb_pipeendpoint(pipe) << 8) |
	    (0 << 7) | (usb_pipedevice(pipe) << 0);
	qh->qh_endpt1 = cpu_to_hc32(endpt);
	endpt = (1 << 30) |
	    (dev->portnr << 23) |
	    (dev->parent->devnum << 16) | (0 << 8) | (0 << 0);
	qh->qh_endpt2 = cpu_to_hc32(endpt);
	qh->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);

	tdp = &qh->qh_overlay.qt_next;

	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 *
		 *   [ buffer, buffer_hi ] loaded with "req".
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (0 << 31) |
		    (sizeof(*req) << 16) |
		    (0 << 15) | (0 << 12) | (3 << 10) | (2 << 8) | (0x80 << 0);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], req, sizeof(*req)) != 0) {
			debug("unable construct SETUP td\n");
			goto fail;
		}
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 *
		 *   [ buffer, buffer_hi ] loaded with "buffer".
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (length << 16) |
		    ((req == NULL ? 1 : 0) << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 1 : 0) << 8) | (0x80 << 0);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], buffer, length) != 0) {
			debug("unable construct DATA td\n");
			goto fail;
		}
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
	}

	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (0 << 16) |
		    (1 << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 0 : 1) << 8) | (0x80 << 0);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
	}

	qh_list->qh_link = cpu_to_hc32((uint32_t)qh | QH_LINK_TYPE_QH);

	/* Flush dcache */
	flush_dcache_range((uint32_t)qh_list,
		ALIGN_END_ADDR(struct QH, qh_list, 1));
	flush_dcache_range((uint32_t)qh, ALIGN_END_ADDR(struct QH, qh, 1));
	flush_dcache_range((uint32_t)qtd, ALIGN_END_ADDR(struct qTD, qtd, 3));

	/* Set async. queue head pointer. */
	ehci_writel(&hcor->or_asynclistaddr, (uint32_t)qh_list);

	usbsts = ehci_readl(&hcor->or_usbsts);
	ehci_writel(&hcor->or_usbsts, (usbsts & 0x3f));

	/* Enable async. schedule. */
	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |= CMD_ASE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts, STD_ASS, STD_ASS,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STD_ASS set\n");
		goto fail;
	}

	/* Wait for TDs to be processed. */
	ts = get_timer(0);
	vtd = &qtd[qtd_counter - 1];
	timeout = USB_TIMEOUT_MS(pipe);
	do {
		/* Invalidate dcache */
		invalidate_dcache_range((uint32_t)qh_list,
			ALIGN_END_ADDR(struct QH, qh_list, 1));
		invalidate_dcache_range((uint32_t)qh,
			ALIGN_END_ADDR(struct QH, qh, 1));
		invalidate_dcache_range((uint32_t)qtd,
			ALIGN_END_ADDR(struct qTD, qtd, 3));

		token = hc32_to_cpu(vtd->qt_token);
		if (!(token & 0x80))
			break;
		WATCHDOG_RESET();
	} while (get_timer(ts) < timeout);

	/* Invalidate the memory area occupied by buffer */
	invalidate_dcache_range(((uint32_t)buffer & ~31),
		((uint32_t)buffer & ~31) + roundup(length, 32));

	/* Check that the TD processing happened */
	if (token & 0x80) {
		printf("EHCI timed out on TD - token=%#x\n", token);
	}

	/* Disable async schedule. */
	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd &= ~CMD_ASE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts, STD_ASS, 0,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STD_ASS reset\n");
		goto fail;
	}

	token = hc32_to_cpu(qh->qh_overlay.qt_token);
	if (!(token & 0x80)) {
		debug("TOKEN=%#x\n", token);
		switch (token & 0xfc) {
		case 0:
			toggle = token >> 31;
			usb_settoggle(dev, usb_pipeendpoint(pipe),
				       usb_pipeout(pipe), toggle);
			dev->status = 0;
			break;
		case 0x40:
			dev->status = USB_ST_STALLED;
			break;
		case 0xa0:
		case 0x20:
			dev->status = USB_ST_BUF_ERR;
			break;
		case 0x50:
		case 0x10:
			dev->status = USB_ST_BABBLE_DET;
			break;
		default:
			dev->status = USB_ST_CRC_ERR;
			if ((token & 0x40) == 0x40)
				dev->status |= USB_ST_STALLED;
			break;
		}
		dev->act_len = length - ((token >> 16) & 0x7fff);
	} else {
		dev->act_len = 0;
		debug("dev=%u, usbsts=%#x, p[1]=%#x, p[2]=%#x\n",
		      dev->devnum, ehci_readl(&hcor->or_usbsts),
		      ehci_readl(&hcor->or_portsc[0]),
		      ehci_readl(&hcor->or_portsc[1]));
	}

	return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:
	return -1;
}

static inline int min3(int a, int b, int c)
{

	if (b < a)
		a = b;
	if (c < a)
		a = c;
	return a;
}

int
ehci_submit_root(struct usb_device *dev, unsigned long pipe, void *buffer,
		 int length, struct devrequest *req)
{
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr = NULL;
	int len, srclen;
	uint32_t reg;
	uint32_t *status_reg;

	if (le16_to_cpu(req->index) > CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS) {
		printf("The request port(%d) is not configured\n",
			le16_to_cpu(req->index) - 1);
		return -1;
	}
	status_reg = (uint32_t *)&hcor->or_portsc[
						le16_to_cpu(req->index) - 1];
	srclen = 0;

	debug("req=%u (%#x), type=%u (%#x), value=%u, index=%u\n",
	      req->request, req->request,
	      req->requesttype, req->requesttype,
	      le16_to_cpu(req->value), le16_to_cpu(req->index));

	typeReq = req->request | req->requesttype << 8;

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			debug("USB_DT_DEVICE request\n");
			srcptr = &descriptor.device;
			srclen = 0x12;
			break;
		case USB_DT_CONFIG:
			debug("USB_DT_CONFIG config\n");
			srcptr = &descriptor.config;
			srclen = 0x19;
			break;
		case USB_DT_STRING:
			debug("USB_DT_STRING config\n");
			switch (le16_to_cpu(req->value) & 0xff) {
			case 0:	/* Language */
				srcptr = "\4\3\1\0";
				srclen = 4;
				break;
			case 1:	/* Vendor */
				srcptr = "\16\3u\0-\0b\0o\0o\0t\0";
				srclen = 14;
				break;
			case 2:	/* Product */
				srcptr = "\52\3E\0H\0C\0I\0 "
					 "\0H\0o\0s\0t\0 "
					 "\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
				srclen = 42;
				break;
			default:
				debug("unknown value DT_STRING %x\n",
					le16_to_cpu(req->value));
				goto unknown;
			}
			break;
		default:
			debug("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
			debug("USB_DT_HUB config\n");
			srcptr = &descriptor.hub;
			srclen = 0x8;
			break;
		default:
			debug("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
		debug("USB_REQ_SET_ADDRESS\n");
		rootdev = le16_to_cpu(req->value);
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		debug("USB_REQ_SET_CONFIGURATION\n");
		/* Nothing to do */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
		memset(tmpbuf, 0, 4);
		reg = ehci_readl(status_reg);
		if (reg & EHCI_PS_CS)
			tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
		if (reg & EHCI_PS_PE)
			tmpbuf[0] |= USB_PORT_STAT_ENABLE;
		if (reg & EHCI_PS_SUSP)
			tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
		if (reg & EHCI_PS_OCA)
			tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
		if (reg & EHCI_PS_PR)
			tmpbuf[0] |= USB_PORT_STAT_RESET;
		if (reg & EHCI_PS_PP)
			tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;

		if (ehci_is_TDI()) {
			switch ((reg >> 26) & 3) {
			case 0:
				break;
			case 1:
				tmpbuf[1] |= USB_PORT_STAT_LOW_SPEED >> 8;
				break;
			case 2:
			default:
				tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
				break;
			}
		} else {
			tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
		}

		if (reg & EHCI_PS_CSC)
			tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
		if (reg & EHCI_PS_PEC)
			tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
		if (reg & EHCI_PS_OCC)
			tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
		if (portreset & (1 << le16_to_cpu(req->index)))
			tmpbuf[2] |= USB_PORT_STAT_C_RESET;

		srcptr = tmpbuf;
		srclen = 4;
		break;
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg |= EHCI_PS_PE;
			ehci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&hccr->cr_hcsparams))) {
				reg |= EHCI_PS_PP;
				ehci_writel(status_reg, reg);
			}
			break;
		case USB_PORT_FEAT_RESET:
			if ((reg & (EHCI_PS_PE | EHCI_PS_CS)) == EHCI_PS_CS &&
			    !ehci_is_TDI() &&
			    EHCI_PS_IS_LOWSPEED(reg)) {
				/* Low speed device, give up ownership. */
				debug("port %d low speed --> companion\n",
				      req->index - 1);
				reg |= EHCI_PS_PO;
				ehci_writel(status_reg, reg);
				break;
			} else {
				int ret;

				reg |= EHCI_PS_PR;
				reg &= ~EHCI_PS_PE;
				ehci_writel(status_reg, reg);
				/*
				 * caller must wait, then call GetPortStatus
				 * usb 2.0 specification say 50 ms resets on
				 * root
				 */
				ehci_powerup_fixup(status_reg, &reg);

				ehci_writel(status_reg, reg & ~EHCI_PS_PR);
				/*
				 * A host controller must terminate the reset
				 * and stabilize the state of the port within
				 * 2 milliseconds
				 */
				ret = handshake(status_reg, EHCI_PS_PR, 0,
						2 * 1000);
				if (!ret)
					portreset |=
						1 << le16_to_cpu(req->index);
				else
					printf("port(%d) reset error\n",
					le16_to_cpu(req->index) - 1);
			}
			break;
		default:
			debug("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		/* unblock posted writes */
		(void) ehci_readl(&hcor->or_usbcmd);
		break;
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg &= ~EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_C_ENABLE:
			reg = (reg & ~EHCI_PS_CLEAR) | EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&hccr->cr_hcsparams)))
				reg = reg & ~(EHCI_PS_CLEAR | EHCI_PS_PP);
		case USB_PORT_FEAT_C_CONNECTION:
			reg = (reg & ~EHCI_PS_CLEAR) | EHCI_PS_CSC;
			break;
		case USB_PORT_FEAT_OVER_CURRENT:
			reg = (reg & ~EHCI_PS_CLEAR) | EHCI_PS_OCC;
			break;
		case USB_PORT_FEAT_C_RESET:
			portreset &= ~(1 << le16_to_cpu(req->index));
			break;
		default:
			debug("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		ehci_writel(status_reg, reg);
		/* unblock posted write */
		(void) ehci_readl(&hcor->or_usbcmd);
		break;
	default:
		debug("Unknown request\n");
		goto unknown;
	}

	mdelay(1);
	len = min3(srclen, le16_to_cpu(req->length), length);
	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	else
		debug("Len is 0\n");

	dev->act_len = len;
	dev->status = 0;
	return 0;

unknown:
	debug("requesttype=%x, request=%x, value=%x, index=%x, length=%x\n",
	      req->requesttype, req->request, le16_to_cpu(req->value),
	      le16_to_cpu(req->index), le16_to_cpu(req->length));

	dev->act_len = 0;
	dev->status = USB_ST_STALLED;
	return -1;
}

int usb_lowlevel_stop(void)
{
	return ehci_hcd_stop();
}

int usb_lowlevel_init(void)
{
	uint32_t reg;
	uint32_t cmd;

	if (ehci_hcd_init() != 0)
		return -1;

	/* EHCI spec section 4.1 */
	if (ehci_reset() != 0)
		return -1;

#if defined(CONFIG_EHCI_HCD_INIT_AFTER_RESET)
	if (ehci_hcd_init() != 0)
		return -1;
#endif

	/* Set head of reclaim list */
	memset(qh_list, 0, sizeof(*qh_list));
	qh_list->qh_link = cpu_to_hc32((uint32_t)qh_list | QH_LINK_TYPE_QH);
	qh_list->qh_endpt1 = cpu_to_hc32((1 << 15) | (USB_SPEED_HIGH << 12));
	qh_list->qh_curtd = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_token = cpu_to_hc32(0x40);

	reg = ehci_readl(&hccr->cr_hcsparams);
	descriptor.hub.bNbrPorts = HCS_N_PORTS(reg);
	printf("Register %x NbrPorts %d\n", reg, descriptor.hub.bNbrPorts);
	/* Port Indicators */
	if (HCS_INDICATOR(reg))
		descriptor.hub.wHubCharacteristics |= 0x80;
	/* Port Power Control */
	if (HCS_PPC(reg))
		descriptor.hub.wHubCharacteristics |= 0x01;

	/* Start the host controller. */
	cmd = ehci_readl(&hcor->or_usbcmd);
	/*
	 * Philips, Intel, and maybe others need CMD_RUN before the
	 * root hub will detect new devices (why?); NEC doesn't
	 */
	cmd &= ~(CMD_LRESET|CMD_IAAD|CMD_PSE|CMD_ASE|CMD_RESET);
	cmd |= CMD_RUN;
	ehci_writel(&hcor->or_usbcmd, cmd);

	/* take control over the ports */
	cmd = ehci_readl(&hcor->or_configflag);
	cmd |= FLAG_CF;
	ehci_writel(&hcor->or_configflag, cmd);
	/* unblock posted write */
	cmd = ehci_readl(&hcor->or_usbcmd);
	mdelay(5);
	reg = HC_VERSION(ehci_readl(&hccr->cr_capbase));
	printf("USB EHCI %x.%02x\n", reg >> 8, reg & 0xff);

	rootdev = 0;

	return 0;
}

int
submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		int length)
{

	if (usb_pipetype(pipe) != PIPE_BULK) {
		debug("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}
	return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

int
submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *setup)
{

	if (usb_pipetype(pipe) != PIPE_CONTROL) {
		debug("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}

	if (usb_pipedevice(pipe) == rootdev) {
		if (rootdev == 0)
			dev->speed = USB_SPEED_HIGH;
		return ehci_submit_root(dev, pipe, buffer, length, setup);
	}
	return ehci_submit_async(dev, pipe, buffer, length, setup);
}

int
submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
	       int length, int interval)
{

	debug("dev=%p, pipe=%lu, buffer=%p, length=%d, interval=%d",
	      dev, pipe, buffer, length, interval);
	return ehci_submit_async(dev, pipe, buffer, length, NULL);
}
