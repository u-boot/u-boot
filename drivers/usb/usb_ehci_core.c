/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Excito Elektronik i Skåne AB
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
#include <usb.h>
#include <asm/io.h>
#include "usb_ehci.h"

int rootdev;
struct ehci_hccr *hccr;		/* R/O registers, not need for volatile */
volatile struct ehci_hcor *hcor;

static uint16_t portreset;
static struct QH qh_list __attribute__((aligned(32)));

struct usb_device_descriptor device = {
	sizeof(struct usb_device_descriptor),	/* bLength */
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
};

struct usb_config_descriptor config = {
	sizeof(struct usb_config_descriptor),
	2,		/* bDescriptorType: UDESC_CONFIG */
	sizeof(struct usb_config_descriptor) +
	sizeof(struct usb_interface_descriptor) +
	sizeof(struct usb_endpoint_descriptor),
	0,
	1,		/* bNumInterface */
	1,		/* bConfigurationValue */
	0,		/* iConfiguration */
	0x40,		/* bmAttributes: UC_SELF_POWER */
	0		/* bMaxPower */
};

struct usb_interface_descriptor interface = {
	sizeof(struct usb_interface_descriptor),	/* bLength */
	4,		/* bDescriptorType: UDESC_INTERFACE */
	0,		/* bInterfaceNumber */
	0,		/* bAlternateSetting */
	1,		/* bNumEndpoints */
	9,		/* bInterfaceClass: UICLASS_HUB */
	0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
	0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
	0		/* iInterface */
};

struct usb_endpoint_descriptor endpoint = {
sizeof(struct usb_endpoint_descriptor),	/* bLength */
	5,		/* bDescriptorType: UDESC_ENDPOINT */
	0x81,		/* bEndpointAddress: UE_DIR_IN | EHCI_INTR_ENDPT */
	3,		/* bmAttributes: UE_INTERRUPT */
	8, 0,		/* wMaxPacketSize */
	255		/* bInterval */
};

struct usb_hub_descriptor hub = {
	sizeof(struct usb_hub_descriptor),	/* bDescLength */
	0x29,		/* bDescriptorType: hub descriptor */
	2,		/* bNrPorts -- runtime modified */
	0, 0,		/* wHubCharacteristics */
	0xff,		/* bPwrOn2PwrGood */
	{},		/* bHubCntrCurrent */
	{}		/* at most 7 ports! XXX */
};

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
			debug("out of TDs");
			return NULL;
		}
		p = &td[ntds];
		ntds++;
		break;
	default:
		debug("unknown allocation size");
		return NULL;
	}

	memset(p, sz, 0);
	return p;
}

static void ehci_free(void *p, size_t sz)
{
}

static int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t addr, delta, next;
	int idx;

	addr = (uint32_t) buf;
	idx = 0;
	while (idx < 5) {
		td->qt_buffer[idx] = cpu_to_le32(addr);
		next = (addr + 4096) & ~4095;
		delta = next - addr;
		if (delta >= sz)
			break;
		sz -= delta;
		addr = next;
		idx++;
	}

	if (idx == 5) {
		debug("out of buffer pointers (%u bytes left)", sz);
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

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p", dev, pipe,
	      buffer, length, req);
	if (req != NULL)
		debug("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u",
		      req->request, req->request,
		      req->requesttype, req->requesttype,
		      le16_to_cpu(req->value), le16_to_cpu(req->value),
		      le16_to_cpu(req->index), le16_to_cpu(req->index));

	qh = ehci_alloc(sizeof(struct QH), 32);
	if (qh == NULL) {
		debug("unable to allocate QH");
		return -1;
	}
	qh->qh_link = cpu_to_le32((uint32_t)&qh_list | QH_LINK_TYPE_QH);
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
	qh->qh_endpt1 = cpu_to_le32(endpt);
	endpt = (1 << 30) |
	    (dev->portnr << 23) |
	    (dev->parent->devnum << 16) | (0 << 8) | (0 << 0);
	qh->qh_endpt2 = cpu_to_le32(endpt);
	qh->qh_overlay.qt_next = cpu_to_le32(QT_NEXT_TERMINATE);
	qh->qh_overlay.qt_altnext = cpu_to_le32(QT_NEXT_TERMINATE);

	td = NULL;
	tdp = &qh->qh_overlay.qt_next;

	toggle =
	    usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	if (req != NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate SETUP td");
			goto fail;
		}
		td->qt_next = cpu_to_le32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_le32(QT_NEXT_TERMINATE);
		token = (0 << 31) |
		    (sizeof(*req) << 16) |
		    (0 << 15) | (0 << 12) | (3 << 10) | (2 << 8) | (0x80 << 0);
		td->qt_token = cpu_to_le32(token);
		if (ehci_td_buffer(td, req, sizeof(*req)) != 0) {
			debug("unable construct SETUP td");
			ehci_free(td, sizeof(*td));
			goto fail;
		}
		*tdp = cpu_to_le32((uint32_t) td);
		tdp = &td->qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate DATA td");
			goto fail;
		}
		td->qt_next = cpu_to_le32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_le32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (length << 16) |
		    ((req == NULL ? 1 : 0) << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 1 : 0) << 8) | (0x80 << 0);
		td->qt_token = cpu_to_le32(token);
		if (ehci_td_buffer(td, buffer, length) != 0) {
			debug("unable construct DATA td");
			ehci_free(td, sizeof(*td));
			goto fail;
		}
		*tdp = cpu_to_le32((uint32_t) td);
		tdp = &td->qt_next;
	}

	if (req != NULL) {
		td = ehci_alloc(sizeof(struct qTD), 32);
		if (td == NULL) {
			debug("unable to allocate ACK td");
			goto fail;
		}
		td->qt_next = cpu_to_le32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_le32(QT_NEXT_TERMINATE);
		token = (toggle << 31) |
		    (0 << 16) |
		    (1 << 15) |
		    (0 << 12) |
		    (3 << 10) |
		    ((usb_pipein(pipe) ? 0 : 1) << 8) | (0x80 << 0);
		td->qt_token = cpu_to_le32(token);
		*tdp = cpu_to_le32((uint32_t) td);
		tdp = &td->qt_next;
	}

	qh_list.qh_link = cpu_to_le32((uint32_t) qh | QH_LINK_TYPE_QH);

	usbsts = le32_to_cpu(hcor->or_usbsts);
	hcor->or_usbsts = cpu_to_le32(usbsts & 0x3f);

	/* Enable async. schedule. */
	hcor->or_usbcmd |= cpu_to_le32(0x20);
	while ((hcor->or_usbsts & cpu_to_le32(0x8000)) == 0)
		udelay(1);

	/* Wait for TDs to be processed. */
	ts = get_timer(0);
	vtd = td;
	do {
		token = le32_to_cpu(vtd->qt_token);
		if (!(token & 0x80))
			break;
	} while (get_timer(ts) < CONFIG_SYS_HZ);

	/* Disable async schedule. */
	hcor->or_usbcmd &= ~cpu_to_le32(0x20);
	while ((hcor->or_usbsts & cpu_to_le32(0x8000)) != 0)
		udelay(1);

	qh_list.qh_link = cpu_to_le32((uint32_t)&qh_list | QH_LINK_TYPE_QH);

	token = le32_to_cpu(qh->qh_overlay.qt_token);
	if (!(token & 0x80)) {
		debug("TOKEN=%#x", token);
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
		debug("dev=%u, usbsts=%#x, p[1]=%#x, p[2]=%#x",
		      dev->devnum, le32_to_cpu(hcor->or_usbsts),
		      le32_to_cpu(hcor->or_portsc[0]),
		      le32_to_cpu(hcor->or_portsc[1]));
	}

	return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:
	td = (void *)le32_to_cpu(qh->qh_overlay.qt_next);
	while (td != (void *)QT_NEXT_TERMINATE) {
		qh->qh_overlay.qt_next = td->qt_next;
		ehci_free(td, sizeof(*td));
		td = (void *)le32_to_cpu(qh->qh_overlay.qt_next);
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

ehci_submit_root(struct usb_device *dev, unsigned long pipe, void *buffer,
		 int length, struct devrequest *req)
{
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr;
	int len, srclen;
	uint32_t reg;

	srclen = 0;
	srcptr = NULL;

	debug("req=%u (%#x), type=%u (%#x), value=%u, index=%u",
	      req->request, req->request,
	      req->requesttype, req->requesttype,
	      le16_to_cpu(req->value), le16_to_cpu(req->index));

	typeReq = req->request << 8 | req->requesttype;

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			srcptr = &device;
			srclen = sizeof(struct usb_device_descriptor);
			break;
		case USB_DT_CONFIG:
			srcptr = &config;
			srclen = sizeof(config) +
				sizeof(struct usb_interface_descriptor) +
				sizeof(struct usb_hub_descriptor);
			break;
		case USB_DT_STRING:
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
				goto unknown;
			}
			break;
		default:
			debug("unknown value %x", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
			srcptr = &hub;
			srclen = sizeof(hub);
			break;
		default:
			debug("unknown value %x", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
		rootdev = le16_to_cpu(req->value);
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		/* Nothing to do */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case DeviceRequest | USB_REQ_GET_STATUS:
		memset(tmpbuf, 0, 4);
		reg = le32_to_cpu(hcor->or_portsc[le16_to_cpu(req->index)
				   - 1]);
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
		tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;

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
	case DeviceOutRequest | USB_REQ_SET_FEATURE:
		reg = le32_to_cpu(hcor->or_portsc[le16_to_cpu(req->index) - 1]);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_POWER:
			reg |= EHCI_PS_PP;
			break;
		case USB_PORT_FEAT_RESET:
			if (EHCI_PS_IS_LOWSPEED(reg)) {
				/* Low speed device, give up ownership. */
				reg |= EHCI_PS_PO;
				break;
			}
			/* Start reset sequence. */
			reg &= ~EHCI_PS_PE;
			reg |= EHCI_PS_PR;
			hcor->or_portsc[le16_to_cpu(req->index) - 1] =
			    cpu_to_le32(reg);
			/* Wait for reset to complete. */
			udelay(500000);
			/* Terminate reset sequence. */
			reg &= ~EHCI_PS_PR;
			/* TODO: is it only fsl chip that requires this
			 * manual setting of port enable?
			 */
			reg |= EHCI_PS_PE;
			hcor->or_portsc[le16_to_cpu(req->index) - 1] =
			    cpu_to_le32(reg);
			/* Wait for HC to complete reset. */
			udelay(2000);
			reg =
			    le32_to_cpu(hcor->or_portsc[le16_to_cpu(req->index)
							- 1]);
			reg &= ~EHCI_PS_CLEAR;
			if ((reg & EHCI_PS_PE) == 0) {
				/* Not a high speed device, give up
				 * ownership. */
				reg |= EHCI_PS_PO;
				break;
			}
			portreset |= 1 << le16_to_cpu(req->index);
			break;
		default:
			debug("unknown feature %x", le16_to_cpu(req->value));
			goto unknown;
		}
		hcor->or_portsc[le16_to_cpu(req->index) - 1] =
					cpu_to_le32(reg);
		break;
	case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
		reg = le32_to_cpu(hcor->or_portsc[le16_to_cpu(req->index) - 1]);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg &= ~EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			reg |= EHCI_PS_CSC;
			break;
		case USB_PORT_FEAT_C_RESET:
			portreset &= ~(1 << le16_to_cpu(req->index));
			break;
		default:
			debug("unknown feature %x", le16_to_cpu(req->value));
			goto unknown;
		}
		hcor->or_portsc[le16_to_cpu(req->index) - 1] =
					cpu_to_le32(reg);
		break;
	default:
		debug("Unknown request");
		goto unknown;
	}

	len = min3(srclen, le16_to_cpu(req->length), length);
	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	dev->act_len = len;
	dev->status = 0;
	return 0;

unknown:
	debug("requesttype=%x, request=%x, value=%x, index=%x, length=%x",
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

	if (ehci_hcd_init() != 0)
		return -1;

	/* Set head of reclaim list */
	memset(&qh_list, 0, sizeof(qh_list));
	qh_list.qh_link = cpu_to_le32((uint32_t)&qh_list | QH_LINK_TYPE_QH);
	qh_list.qh_endpt1 = cpu_to_le32((1 << 15) | (USB_SPEED_HIGH << 12));
	qh_list.qh_curtd = cpu_to_le32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_next = cpu_to_le32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_altnext = cpu_to_le32(QT_NEXT_TERMINATE);
	qh_list.qh_overlay.qt_token = cpu_to_le32(0x40);

	/* Set async. queue head pointer. */
	hcor->or_asynclistaddr = cpu_to_le32((uint32_t)&qh_list);

	reg = le32_to_cpu(hccr->cr_hcsparams);
	hub.bNbrPorts = reg & 0xf;
	if (reg & 0x10000)	/* Port Indicators */
		hub.wHubCharacteristics |= 0x80;
	if (reg & 0x10)		/* Port Power Control */
		hub.wHubCharacteristics |= 0x01;

	/* take control over the ports */
	hcor->or_configflag |= cpu_to_le32(1);

	/* Start the host controller. */
	hcor->or_usbcmd |= cpu_to_le32(1);

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
