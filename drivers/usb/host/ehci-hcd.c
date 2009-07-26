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

#include "ehci.h"

int rootdev;
struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
volatile struct ehci_hcor *hcor;

static uint16_t portreset;
static struct QH qh_list __attribute__((aligned(32)));

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
		0xff,		/* bPwrOn2PwrGood */
		0,		/* bHubCntrCurrent */
		{},		/* Device removable */
		{}		/* at most 7 ports! XXX */
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		0x0002,		/* bcdUSB: v2.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
		64,		/* bMaxPacketSize: 64 bytes */
		0x0000,		/* idVendor */
		0x0000,		/* idProduct */
		0x0001,		/* bcdDevice */
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
		8, 0,		/* wMaxPacketSize */
		255		/* bInterval */
	},
};

#if defined(CONFIG_EHCI_IS_TDI)
#define ehci_is_TDI()	(1)
#else
#define ehci_is_TDI()	(0)
#endif

#if defined(CONFIG_EHCI_DCACHE)
/*
 * Routines to handle (flush/invalidate) the dcache for the QH and qTD
 * structures and data buffers. This is needed on platforms using this
 * EHCI support with dcache enabled.
 */
static void flush_invalidate(u32 addr, int size, int flush)
{
	if (flush)
		flush_dcache_range(addr, addr + size);
	else
		invalidate_dcache_range(addr, addr + size);
}

static void cache_qtd(struct qTD *qtd, int flush)
{
	u32 *ptr = (u32 *)qtd->qt_buffer[0];
	int len = (qtd->qt_token & 0x7fff0000) >> 16;

	flush_invalidate((u32)qtd, sizeof(struct qTD), flush);
	if (ptr && len)
		flush_invalidate((u32)ptr, len, flush);
}


static inline struct QH *qh_addr(struct QH *qh)
{
	return (struct QH *)((u32)qh & 0xffffffe0);
}

static void cache_qh(struct QH *qh, int flush)
{
	struct qTD *qtd;
	struct qTD *next;
	static struct qTD *first_qtd;

	/*
	 * Walk the QH list and flush/invalidate all entries
	 */
	while (1) {
		flush_invalidate((u32)qh_addr(qh), sizeof(struct QH), flush);
		if ((u32)qh & QH_LINK_TYPE_QH)
			break;
		qh = qh_addr(qh);
		qh = (struct QH *)qh->qh_link;
	}
	qh = qh_addr(qh);

	/*
	 * Save first qTD pointer, needed for invalidating pass on this QH
	 */
	if (flush)
		first_qtd = qtd = (struct qTD *)(*(u32 *)&qh->qh_overlay &
						 0xffffffe0);
	else
		qtd = first_qtd;

	/*
	 * Walk the qTD list and flush/invalidate all entries
	 */
	while (1) {
		if (qtd == NULL)
			break;
		cache_qtd(qtd, flush);
		next = (struct qTD *)((u32)qtd->qt_next & 0xffffffe0);
		if (next == qtd)
			break;
		qtd = next;
	}
}

static inline void ehci_flush_dcache(struct QH *qh)
{
	cache_qh(qh, 1);
}

static inline void ehci_invalidate_dcache(struct QH *qh)
{
	cache_qh(qh, 0);
}
#else /* CONFIG_EHCI_DCACHE */
/*
 *
 */
static inline void ehci_flush_dcache(struct QH *qh)
{
}

static inline void ehci_invalidate_dcache(struct QH *qh)
{
}
#endif /* CONFIG_EHCI_DCACHE */

static int handshake(uint32_t *ptr, uint32_t mask, uint32_t done, int usec)
{
	uint32_t result;
	do {
		result = ehci_readl(ptr);
		if (result == ~(uint32_t)0)
			return -1;
		result &= mask;
		if (result == done)
			return 0;
		udelay(1);
		usec--;
	} while (usec > 0);
	return -1;
}

static void ehci_free(void *p, size_t sz)
{

}

static int ehci_reset(void)
{
	uint32_t cmd;
	uint32_t tmp;
	uint32_t *reg_ptr;
	int ret = 0;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |= CMD_RESET;
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
out:
	return ret;
}

static void *ehci_alloc(size_t sz, size_t align)
{
	static struct QH qh __attribute__((aligned(32)));
	static struct qTD td[3] __attribute__((aligned (32)));
	static int ntds;
	void *p;

	switch (sz) {
	case sizeof(struct QH):
		p = &qh;
		ntds = 0;
		break;
	case sizeof(struct qTD):
		if (ntds == 3) {
			debug("out of TDs\n");
			return NULL;
		}
		p = &td[ntds];
		ntds++;
		break;
	default:
		debug("unknown allocation size\n");
		return NULL;
	}

	memset(p, sz, 0);
	return p;
}

static int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t addr, delta, next;
	int idx;

	addr = (uint32_t) buf;
	idx = 0;
	while (idx < 5) {
		td->qt_buffer[idx] = cpu_to_hc32(addr);
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
	struct QH *qh;
	struct qTD *td;
	volatile struct qTD *vtd;
	unsigned long ts;
	uint32_t *tdp;
	uint32_t endpt, token, usbsts;
	uint32_t c, toggle;
	uint32_t cmd;
	int ret = 0;

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe,
	      buffer, length, req);
	if (req != NULL)
		debug("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
		      req->request, req->request,
		      req->requesttype, req->requesttype,
		      le16_to_cpu(req->value), le16_to_cpu(req->value),
		      le16_to_cpu(req->index));

	qh = ehci_alloc(sizeof(struct QH), 32);
	if (qh == NULL) {
		debug("unable to allocate QH\n");
		return -1;
	}
	qh->qh_link = cpu_to_hc32((uint32_t)&qh_list | QH_LINK_TYPE_QH);
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
	qh->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	td = NULL;
	tdp = &qh->qh_overlay.qt_next;

	toggle =
	    usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	if (req != NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate SETUP td\n");
			goto fail;
		}
		td->qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (0 << 31) |
		    (sizeof(*req) << 16) |
		    (0 << 15) | (0 << 12) | (3 << 10) | (2 << 8) | (0x80 << 0);
		td->qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(td, req, sizeof(*req)) != 0) {
			debug("unable construct SETUP td\n");
			ehci_free(td, sizeof(*td));
			goto fail;
		}
		*tdp = cpu_to_hc32((uint32_t) td);
		tdp = &td->qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate DATA td\n");
			goto fail;
		}
		td->qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (length << 16) |
		    ((req == NULL ? 1 : 0) << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 1 : 0) << 8) | (0x80 << 0);
		td->qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(td, buffer, length) != 0) {
			debug("unable construct DATA td\n");
			ehci_free(td, sizeof(*td));
			goto fail;
		}
		*tdp = cpu_to_hc32((uint32_t) td);
		tdp = &td->qt_next;
	}

	if (req != NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate ACK td\n");
			goto fail;
		}
		td->qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (0 << 16) |
		    (1 << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 0 : 1) << 8) | (0x80 << 0);
		td->qt_token = cpu_to_hc32(token);
		*tdp = cpu_to_hc32((uint32_t) td);
		tdp = &td->qt_next;
	}

	qh_list.qh_link = cpu_to_hc32((uint32_t) qh | QH_LINK_TYPE_QH);

	/* Flush dcache */
	ehci_flush_dcache(&qh_list);

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
	vtd = td;
	do {
		/* Invalidate dcache */
		ehci_invalidate_dcache(&qh_list);
		token = hc32_to_cpu(vtd->qt_token);
		if (!(token & 0x80))
			break;
	} while (get_timer(ts) < CONFIG_SYS_HZ);

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

	qh_list.qh_link = cpu_to_hc32((uint32_t)&qh_list | QH_LINK_TYPE_QH);

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
	td = (void *)hc32_to_cpu(qh->qh_overlay.qt_next);
	while (td != (void *)QT_NEXT_TERMINATE) {
		qh->qh_overlay.qt_next = td->qt_next;
		ehci_free(td, sizeof(*td));
		td = (void *)hc32_to_cpu(qh->qh_overlay.qt_next);
	}
	ehci_free(qh, sizeof(*qh));
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

	if (le16_to_cpu(req->index) >= CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS) {
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
		if (reg & EHCI_PS_PR &&
		    (portreset & (1 << le16_to_cpu(req->index)))) {
			int ret;
			/* force reset to complete */
			reg = reg & ~(EHCI_PS_PR | EHCI_PS_CLEAR);
			ehci_writel(status_reg, reg);
			ret = handshake(status_reg, EHCI_PS_PR, 0, 2 * 1000);
			if (!ret)
				tmpbuf[0] |= USB_PORT_STAT_RESET;
			else
				printf("port(%d) reset error\n",
					le16_to_cpu(req->index) - 1);
		}
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
				reg |= EHCI_PS_PR;
				reg &= ~EHCI_PS_PE;
				ehci_writel(status_reg, reg);
				/*
				 * caller must wait, then call GetPortStatus
				 * usb 2.0 specification say 50 ms resets on
				 * root
				 */
				wait_ms(50);
				portreset |= 1 << le16_to_cpu(req->index);
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

	wait_ms(1);
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
	memset(&qh_list, 0, sizeof(qh_list));
	qh_list.qh_link = cpu_to_hc32((uint32_t)&qh_list | QH_LINK_TYPE_QH);
	qh_list.qh_endpt1 = cpu_to_hc32((1 << 15) | (USB_SPEED_HIGH << 12));
	qh_list.qh_curtd = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_token = cpu_to_hc32(0x40);

	/* Set async. queue head pointer. */
	ehci_writel(&hcor->or_asynclistaddr, (uint32_t)&qh_list);

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
	wait_ms(5);
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
	return -1;
}
