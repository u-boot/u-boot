// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <command.h>
#include <config.h>
#include <cpu_func.h>
#include <net.h>
#include <malloc.h>
#include <wait_bit.h>
#include <asm/byteorder.h>
#include <asm/cache.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <usb/ci_udc.h>
#include "../host/ehci.h"
#include "ci_udc.h"

/*
 * Check if the system has too long cachelines. If the cachelines are
 * longer then 128b, the driver will not be able flush/invalidate data
 * cache over separate QH entries. We use 128b because one QH entry is
 * 64b long and there are always two QH list entries for each endpoint.
 */
#if ARCH_DMA_MINALIGN > 128
#error This driver can not work on systems with caches longer than 128b
#endif

/*
 * Every QTD must be individually aligned, since we can program any
 * QTD's address into HW. Cache flushing requires ARCH_DMA_MINALIGN,
 * and the USB HW requires 32-byte alignment. Align to both:
 */
#define ILIST_ALIGN		roundup(ARCH_DMA_MINALIGN, 32)
/* Each QTD is this size */
#define ILIST_ENT_RAW_SZ	sizeof(struct ept_queue_item)
/*
 * Align the size of the QTD too, so we can add this value to each
 * QTD's address to get another aligned address.
 */
#define ILIST_ENT_SZ		roundup(ILIST_ENT_RAW_SZ, ILIST_ALIGN)
/* For each endpoint, we need 2 QTDs, one for each of IN and OUT */
#define ILIST_SZ		(NUM_ENDPOINTS * 2 * ILIST_ENT_SZ)

#define EP_MAX_LENGTH_TRANSFER	0x4000

#ifndef DEBUG
#define DBG(x...) do {} while (0)
#else
#define DBG(x...) printf(x)
static const char *reqname(unsigned r)
{
	switch (r) {
	case USB_REQ_GET_STATUS: return "GET_STATUS";
	case USB_REQ_CLEAR_FEATURE: return "CLEAR_FEATURE";
	case USB_REQ_SET_FEATURE: return "SET_FEATURE";
	case USB_REQ_SET_ADDRESS: return "SET_ADDRESS";
	case USB_REQ_GET_DESCRIPTOR: return "GET_DESCRIPTOR";
	case USB_REQ_SET_DESCRIPTOR: return "SET_DESCRIPTOR";
	case USB_REQ_GET_CONFIGURATION: return "GET_CONFIGURATION";
	case USB_REQ_SET_CONFIGURATION: return "SET_CONFIGURATION";
	case USB_REQ_GET_INTERFACE: return "GET_INTERFACE";
	case USB_REQ_SET_INTERFACE: return "SET_INTERFACE";
	default: return "*UNKNOWN*";
	}
}
#endif

static struct usb_endpoint_descriptor ep0_desc = {
	.bLength = sizeof(struct usb_endpoint_descriptor),
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes =	USB_ENDPOINT_XFER_CONTROL,
};

static int ci_pullup(struct usb_gadget *gadget, int is_on);

static const struct usb_gadget_ops ci_udc_ops = {
	.pullup = ci_pullup,
};

__weak void ci_init_after_reset(struct ehci_ctrl *ctrl)
{
}

static struct ci_drv controller = {
	.gadget	= {
		.name	= "ci_udc",
		.ops	= &ci_udc_ops,
		.is_dualspeed = 1,
		.max_speed = USB_SPEED_HIGH,
	},
};

/**
 * ci_get_qh() - return queue head for endpoint
 * @ep_num:	Endpoint number
 * @dir_in:	Direction of the endpoint (IN = 1, OUT = 0)
 *
 * This function returns the QH associated with particular endpoint
 * and it's direction.
 */
static struct ept_queue_head *ci_get_qh(int ep_num, int dir_in)
{
	return &controller.epts[(ep_num * 2) + dir_in];
}

/**
 * ci_get_qtd() - return queue item for endpoint
 * @ep_num:	Endpoint number
 * @dir_in:	Direction of the endpoint (IN = 1, OUT = 0)
 *
 * This function returns the QH associated with particular endpoint
 * and it's direction.
 */
static struct ept_queue_item *ci_get_qtd(int ep_num, int dir_in)
{
	int index = (ep_num * 2) + dir_in;
	uint8_t *imem = controller.items_mem + (index * ILIST_ENT_SZ);
	return (struct ept_queue_item *)imem;
}

/**
 * ci_flush_qh - flush cache over queue head
 * @ep_num:	Endpoint number
 *
 * This function flushes cache over QH for particular endpoint.
 */
static void ci_flush_qh(int ep_num)
{
	struct ept_queue_head *head = ci_get_qh(ep_num, 0);
	const unsigned long start = (unsigned long)head;
	const unsigned long end = start + 2 * sizeof(*head);

	flush_dcache_range(start, end);
}

/**
 * ci_invalidate_qh - invalidate cache over queue head
 * @ep_num:	Endpoint number
 *
 * This function invalidates cache over QH for particular endpoint.
 */
static void ci_invalidate_qh(int ep_num)
{
	struct ept_queue_head *head = ci_get_qh(ep_num, 0);
	unsigned long start = (unsigned long)head;
	unsigned long end = start + 2 * sizeof(*head);

	invalidate_dcache_range(start, end);
}

/**
 * ci_flush_qtd - flush cache over queue item
 * @ep_num:	Endpoint number
 *
 * This function flushes cache over qTD pair for particular endpoint.
 */
static void ci_flush_qtd(int ep_num)
{
	struct ept_queue_item *item = ci_get_qtd(ep_num, 0);
	const unsigned long start = (unsigned long)item;
	const unsigned long end = start + 2 * ILIST_ENT_SZ;

	flush_dcache_range(start, end);
}

/**
 * ci_flush_td - flush cache over queue item
 * @td:	td pointer
 *
 * This function flushes cache for particular transfer descriptor.
 */
static void ci_flush_td(struct ept_queue_item *td)
{
	const unsigned long start = (unsigned long)td;
	const unsigned long end = (unsigned long)td + ILIST_ENT_SZ;
	flush_dcache_range(start, end);
}

/**
 * ci_invalidate_qtd - invalidate cache over queue item
 * @ep_num:	Endpoint number
 *
 * This function invalidates cache over qTD pair for particular endpoint.
 */
static void ci_invalidate_qtd(int ep_num)
{
	struct ept_queue_item *item = ci_get_qtd(ep_num, 0);
	const unsigned long start = (unsigned long)item;
	const unsigned long end = start + 2 * ILIST_ENT_SZ;

	invalidate_dcache_range(start, end);
}

/**
 * ci_invalidate_td - invalidate cache over queue item
 * @td:	td pointer
 *
 * This function invalidates cache for particular transfer descriptor.
 */
static void ci_invalidate_td(struct ept_queue_item *td)
{
	const unsigned long start = (unsigned long)td;
	const unsigned long end = start + ILIST_ENT_SZ;
	invalidate_dcache_range(start, end);
}

static void ep_enable(int num, int in, int maxpacket)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	unsigned n;

	n = readl(&udc->epctrl[num]);
	if (in)
		n |= (CTRL_TXE | CTRL_TXR | CTRL_TXT_BULK);
	else
		n |= (CTRL_RXE | CTRL_RXR | CTRL_RXT_BULK);

	if (num != 0) {
		struct ept_queue_head *head = ci_get_qh(num, in);

		head->config = CFG_MAX_PKT(maxpacket) | CFG_ZLT;
		ci_flush_qh(num);
	}
	writel(n, &udc->epctrl[num]);
}

static void ci_debounce(struct ci_req *ci_req, int in)
{
	struct usb_request *req = &ci_req->req;
	unsigned long addr = (unsigned long)req->buf;
	unsigned long hwaddr = (unsigned long)ci_req->hw_buf;
	uint32_t aligned_used_len;

	if (in || !hwaddr)
		return;

	aligned_used_len = roundup(req->actual, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(hwaddr, hwaddr + aligned_used_len);

	if (addr == hwaddr)
		return; /* not a bounce */

	memcpy(req->buf, ci_req->hw_buf, req->actual);
}

static void ci_ep_submit_next_request(struct ci_ep *ci_ep)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	struct ept_queue_item *item;
	struct ept_queue_head *head;
	int bit, num, len, in;
	struct ci_req *ci_req;
	u8 *buf;
	uint32_t len_left, len_this_dtd;
	struct ept_queue_item *dtd, *qtd;

	ci_ep->req_primed = true;

	num = ci_ep->desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (ci_ep->desc->bEndpointAddress & USB_DIR_IN) != 0;
	item = ci_get_qtd(num, in);
	head = ci_get_qh(num, in);

	ci_req = list_first_entry(&ci_ep->queue, struct ci_req, queue);
	len = ci_req->req.length;

	head->next = (unsigned long)item;
	head->info = 0;

	ci_req->dtd_count = 0;
	buf = ci_req->hw_buf;
	len_left = len;
	dtd = item;

	do {
		len_this_dtd = min(len_left, (unsigned)EP_MAX_LENGTH_TRANSFER);

		dtd->info = INFO_BYTES(len_this_dtd) | INFO_ACTIVE;
		dtd->page0 = (unsigned long)buf;
		dtd->page1 = ((unsigned long)buf & 0xfffff000) + 0x1000;
		dtd->page2 = ((unsigned long)buf & 0xfffff000) + 0x2000;
		dtd->page3 = ((unsigned long)buf & 0xfffff000) + 0x3000;
		dtd->page4 = ((unsigned long)buf & 0xfffff000) + 0x4000;

		len_left -= len_this_dtd;
		buf += len_this_dtd;

		if (len_left) {
			qtd = (struct ept_queue_item *)
			       memalign(ILIST_ALIGN, ILIST_ENT_SZ);
			dtd->next = (unsigned long)qtd;
			dtd = qtd;
			memset(dtd, 0, ILIST_ENT_SZ);
		}

		ci_req->dtd_count++;
	} while (len_left);

	item = dtd;
	/*
	 * When sending the data for an IN transaction, the attached host
	 * knows that all data for the IN is sent when one of the following
	 * occurs:
	 * a) A zero-length packet is transmitted.
	 * b) A packet with length that isn't an exact multiple of the ep's
	 *    maxpacket is transmitted.
	 * c) Enough data is sent to exactly fill the host's maximum expected
	 *    IN transaction size.
	 *
	 * One of these conditions MUST apply at the end of an IN transaction,
	 * or the transaction will not be considered complete by the host. If
	 * none of (a)..(c) already applies, then we must force (a) to apply
	 * by explicitly sending an extra zero-length packet.
	 */
	/*  IN    !a     !b                              !c */
	if (in && len && !(len % ci_ep->ep.maxpacket) && ci_req->req.zero) {
		/*
		 * Each endpoint has 2 items allocated, even though typically
		 * only 1 is used at a time since either an IN or an OUT but
		 * not both is queued. For an IN transaction, item currently
		 * points at the second of these items, so we know that we
		 * can use the other to transmit the extra zero-length packet.
		 */
		struct ept_queue_item *other_item = ci_get_qtd(num, 0);
		item->next = (unsigned long)other_item;
		item = other_item;
		item->info = INFO_ACTIVE;
	}

	item->next = TERMINATE;
	item->info |= INFO_IOC;

	ci_flush_qtd(num);

	item = (struct ept_queue_item *)(unsigned long)head->next;
	while (item->next != TERMINATE) {
		ci_flush_td((struct ept_queue_item *)(unsigned long)item->next);
		item = (struct ept_queue_item *)(unsigned long)item->next;
	}

	DBG("ept%d %s queue len %x, req %p, buffer %p\n",
	    num, in ? "in" : "out", len, ci_req, ci_req->hw_buf);
	ci_flush_qh(num);

	if (in)
		bit = EPT_TX(num);
	else
		bit = EPT_RX(num);

	writel(bit, &udc->epprime);
}

static void flip_ep0_direction(void)
{
	if (ep0_desc.bEndpointAddress == USB_DIR_IN) {
		DBG("%s: Flipping ep0 to OUT\n", __func__);
		ep0_desc.bEndpointAddress = 0;
	} else {
		DBG("%s: Flipping ep0 to IN\n", __func__);
		ep0_desc.bEndpointAddress = USB_DIR_IN;
	}
}

/*
 * This function explicitly sets the address, without the "USBADRA" (advance)
 * feature, which is not supported by older versions of the controller.
 */
static void ci_set_address(struct ci_udc *udc, u8 address)
{
	DBG("%s %x\n", __func__, address);
	writel(address << 25, &udc->devaddr);
}

static void handle_ep_complete(struct ci_ep *ci_ep)
{
	struct ept_queue_item *item, *next_td;
	int num, in, len, j;
	struct ci_req *ci_req;

	/* Set the device address that was previously sent by SET_ADDRESS */
	if (controller.next_device_address != 0) {
		struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;

		ci_set_address(udc, controller.next_device_address);
		controller.next_device_address = 0;
	}

	num = ci_ep->desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (ci_ep->desc->bEndpointAddress & USB_DIR_IN) != 0;
	item = ci_get_qtd(num, in);
	ci_invalidate_qtd(num);
	ci_req = list_first_entry(&ci_ep->queue, struct ci_req, queue);

	next_td = item;
	len = 0;
	for (j = 0; j < ci_req->dtd_count; j++) {
		ci_invalidate_td(next_td);
		item = next_td;
		len += (item->info >> 16) & 0x7fff;
		if (item->info & 0xff)
			printf("EP%d/%s FAIL info=%x pg0=%x\n",
			       num, in ? "in" : "out", item->info, item->page0);
		if (j != ci_req->dtd_count - 1)
			next_td = (struct ept_queue_item *)(unsigned long)
				item->next;
		if (j != 0)
			free(item);
	}

	list_del_init(&ci_req->queue);
	ci_ep->req_primed = false;

	if (!list_empty(&ci_ep->queue))
		ci_ep_submit_next_request(ci_ep);

	ci_req->req.actual = ci_req->req.length - len;
	ci_debounce(ci_req, in);

	DBG("ept%d %s req %p, complete %x\n",
	    num, in ? "in" : "out", ci_req, len);
	if (num != 0 || controller.ep0_data_phase)
		ci_req->req.complete(&ci_ep->ep, &ci_req->req);
	if (num == 0 && controller.ep0_data_phase) {
		/*
		 * Data Stage is complete, so flip ep0 dir for Status Stage,
		 * which always transfers a packet in the opposite direction.
		 */
		DBG("%s: flip ep0 dir for Status Stage\n", __func__);
		flip_ep0_direction();
		controller.ep0_data_phase = false;
		ci_req->req.length = 0;
		usb_ep_queue(&ci_ep->ep, &ci_req->req, 0);
	}
}

#define SETUP(type, request) (((type) << 8) | (request))

static void handle_setup(void)
{
	struct ci_ep *ci_ep = &controller.ep[0];
	struct ci_req *ci_req;
	struct usb_request *req;
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	struct ept_queue_head *head;
	struct usb_ctrlrequest r;
	int status = 0;
	int num, in, _num, _in, i;
	char *buf;

	ci_req = controller.ep0_req;
	req = &ci_req->req;
	head = ci_get_qh(0, 0);	/* EP0 OUT */

	ci_invalidate_qh(0);
	memcpy(&r, head->setup_data, sizeof(struct usb_ctrlrequest));
#ifdef CONFIG_CI_UDC_HAS_HOSTPC
	writel(EPT_RX(0), &udc->epsetupstat);
#else
	writel(EPT_RX(0), &udc->epstat);
#endif
	DBG("handle setup %s, %x, %x index %x value %x length %x\n",
	    reqname(r.bRequest), r.bRequestType, r.bRequest, r.wIndex,
	    r.wValue, r.wLength);

	/* Set EP0 dir for Data Stage based on Setup Stage data */
	if (r.bRequestType & USB_DIR_IN) {
		DBG("%s: Set ep0 to IN for Data Stage\n", __func__);
		ep0_desc.bEndpointAddress = USB_DIR_IN;
	} else {
		DBG("%s: Set ep0 to OUT for Data Stage\n", __func__);
		ep0_desc.bEndpointAddress = 0;
	}
	if (r.wLength) {
		controller.ep0_data_phase = true;
	} else {
		/* 0 length -> no Data Stage. Flip dir for Status Stage */
		DBG("%s: 0 length: flip ep0 dir for Status Stage\n", __func__);
		flip_ep0_direction();
		controller.ep0_data_phase = false;
	}

	list_del_init(&ci_req->queue);
	ci_ep->req_primed = false;

	switch (SETUP(r.bRequestType, r.bRequest)) {
	case SETUP(USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE):
		_num = r.wIndex & 15;
		_in = !!(r.wIndex & 0x80);

		if ((r.wValue == 0) && (r.wLength == 0)) {
			req->length = 0;
			for (i = 0; i < NUM_ENDPOINTS; i++) {
				struct ci_ep *ep = &controller.ep[i];

				if (!ep->desc)
					continue;
				num = ep->desc->bEndpointAddress
						& USB_ENDPOINT_NUMBER_MASK;
				in = (ep->desc->bEndpointAddress
						& USB_DIR_IN) != 0;
				if ((num == _num) && (in == _in)) {
					ep_enable(num, in, ep->ep.maxpacket);
					usb_ep_queue(controller.gadget.ep0,
							req, 0);
					break;
				}
			}
		}
		return;

	case SETUP(USB_RECIP_DEVICE, USB_REQ_SET_ADDRESS):
		/*
		 * write address delayed (will take effect
		 * after the next IN txn)
		 */
		controller.next_device_address = r.wValue;
		req->length = 0;
		usb_ep_queue(controller.gadget.ep0, req, 0);
		return;

	case SETUP(USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_STATUS):
		req->length = 2;
		buf = (char *)req->buf;
		buf[0] = 1 << USB_DEVICE_SELF_POWERED;
		buf[1] = 0;
		usb_ep_queue(controller.gadget.ep0, req, 0);
		return;
	}
	/* pass request up to the gadget driver */
	if (controller.driver)
		status = controller.driver->setup(&controller.gadget, &r);
	else
		status = -ENODEV;

	if (!status)
		return;
	DBG("STALL reqname %s type %x value %x, index %x\n",
	    reqname(r.bRequest), r.bRequestType, r.wValue, r.wIndex);
	writel((1<<16) | (1 << 0), &udc->epctrl[0]);
}

static void stop_activity(void)
{
	int i, num, in;
	struct ept_queue_head *head;
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;

	ci_set_address(udc, 0);

	writel(readl(&udc->epcomp), &udc->epcomp);
#ifdef CONFIG_CI_UDC_HAS_HOSTPC
	writel(readl(&udc->epsetupstat), &udc->epsetupstat);
#endif
	writel(readl(&udc->epstat), &udc->epstat);
	writel(0xffffffff, &udc->epflush);

	/* error out any pending reqs */
	for (i = 0; i < NUM_ENDPOINTS; i++) {
		if (i != 0)
			writel(0, &udc->epctrl[i]);
		if (controller.ep[i].desc) {
			num = controller.ep[i].desc->bEndpointAddress
				& USB_ENDPOINT_NUMBER_MASK;
			in = (controller.ep[i].desc->bEndpointAddress
				& USB_DIR_IN) != 0;
			head = ci_get_qh(num, in);
			head->info = INFO_ACTIVE;
			ci_flush_qh(num);
		}
	}
}

void udc_irq(void)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	unsigned n = readl(&udc->usbsts);
	writel(n, &udc->usbsts);
	int bit, i, num, in;

	n &= (STS_SLI | STS_URI | STS_PCI | STS_UI | STS_UEI);
	if (n == 0)
		return;

	if (n & STS_URI) {
		DBG("-- reset --\n");
		stop_activity();
	}
	if (n & STS_SLI)
		DBG("-- suspend --\n");

	if (n & STS_PCI) {
		int max = 64;
		int speed = USB_SPEED_FULL;

#ifdef CONFIG_CI_UDC_HAS_HOSTPC
		bit = (readl(&udc->hostpc1_devlc) >> 25) & 3;
#else
		bit = (readl(&udc->portsc) >> 26) & 3;
#endif
		DBG("-- portchange %x %s\n", bit, (bit == 2) ? "High" : "Full");
		if (bit == 2) {
			speed = USB_SPEED_HIGH;
			max = 512;
		}
		controller.gadget.speed = speed;
		for (i = 1; i < NUM_ENDPOINTS; i++) {
			if (controller.ep[i].ep.maxpacket > max)
				controller.ep[i].ep.maxpacket = max;
		}
	}

	if (n & STS_UEI)
		printf("<UEI %x>\n", readl(&udc->epcomp));

	if ((n & STS_UI) || (n & STS_UEI)) {
#ifdef CONFIG_CI_UDC_HAS_HOSTPC
		n = readl(&udc->epsetupstat);
#else
		n = readl(&udc->epstat);
#endif
		if (n & EPT_RX(0))
			handle_setup();

		n = readl(&udc->epcomp);
		if (n != 0)
			writel(n, &udc->epcomp);

		for (i = 0; i < NUM_ENDPOINTS && n; i++) {
			if (controller.ep[i].desc) {
				num = controller.ep[i].desc->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK;
				in = (controller.ep[i].desc->bEndpointAddress
						& USB_DIR_IN) != 0;
				bit = (in) ? EPT_TX(num) : EPT_RX(num);
				if (n & bit)
					handle_ep_complete(&controller.ep[i]);
			}
		}
	}
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	u32 value;

	value = readl(&udc->usbsts);
	if (value)
		udc_irq();

	return value;
}

static void udc_disconnect(void)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	/* disable pullup */
	stop_activity();
	writel(USBCMD_FS2, &udc->usbcmd);
	udelay(800);
	if (controller.driver)
		controller.driver->disconnect(&controller.gadget);
}

static int ci_pullup(struct usb_gadget *gadget, int is_on)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;
	if (is_on) {
		/* RESET */
		controller.next_device_address = 0;
		writel(USBCMD_ITC(MICRO_8FRAME) | USBCMD_RST, &udc->usbcmd);
		udelay(200);

		ci_init_after_reset(controller.ctrl);

		writel((unsigned long)controller.epts, &udc->epinitaddr);

		/* select DEVICE mode */
		writel(USBMODE_DEVICE, &udc->usbmode);

#if !defined(CONFIG_USB_GADGET_DUALSPEED)
		/* Port force Full-Speed Connect */
		setbits_le32(&udc->portsc, PFSC);
#endif

		writel(0xffffffff, &udc->epflush);

		/* Turn on the USB connection by enabling the pullup resistor */
		setbits_le32(&udc->usbcmd, USBCMD_ITC(MICRO_8FRAME) |
			     USBCMD_RUN);
	} else {
		udc_disconnect();
	}

	return 0;
}

bool dfu_usb_get_reset(void)
{
	struct ci_udc *udc = (struct ci_udc *)controller.ctrl->hcor;

	return !!(readl(&udc->usbsts) & STS_URI);
}
