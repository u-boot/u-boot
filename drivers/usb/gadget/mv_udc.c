/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <net.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/types.h>
#include <usb/mv_udc.h>

#if CONFIG_USB_MAX_CONTROLLER_COUNT > 1
#error This driver only supports one single controller.
#endif

/*
 * Check if the system has too long cachelines. If the cachelines are
 * longer then 128b, the driver will not be able flush/invalidate data
 * cache over separate QH entries. We use 128b because one QH entry is
 * 64b long and there are always two QH list entries for each endpoint.
 */
#if ARCH_DMA_MINALIGN > 128
#error This driver can not work on systems with caches longer than 128b
#endif

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

static struct usb_endpoint_descriptor ep0_out_desc = {
	.bLength = sizeof(struct usb_endpoint_descriptor),
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0,
	.bmAttributes =	USB_ENDPOINT_XFER_CONTROL,
};

static struct usb_endpoint_descriptor ep0_in_desc = {
	.bLength = sizeof(struct usb_endpoint_descriptor),
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes =	USB_ENDPOINT_XFER_CONTROL,
};

static int mv_pullup(struct usb_gadget *gadget, int is_on);
static int mv_ep_enable(struct usb_ep *ep,
		const struct usb_endpoint_descriptor *desc);
static int mv_ep_disable(struct usb_ep *ep);
static int mv_ep_queue(struct usb_ep *ep,
		struct usb_request *req, gfp_t gfp_flags);
static struct usb_request *
mv_ep_alloc_request(struct usb_ep *ep, unsigned int gfp_flags);
static void mv_ep_free_request(struct usb_ep *ep, struct usb_request *_req);

static struct usb_gadget_ops mv_udc_ops = {
	.pullup = mv_pullup,
};

static struct usb_ep_ops mv_ep_ops = {
	.enable         = mv_ep_enable,
	.disable        = mv_ep_disable,
	.queue          = mv_ep_queue,
	.alloc_request  = mv_ep_alloc_request,
	.free_request   = mv_ep_free_request,
};

/* Init values for USB endpoints. */
static const struct usb_ep mv_ep_init[2] = {
	[0] = {	/* EP 0 */
		.maxpacket	= 64,
		.name		= "ep0",
		.ops		= &mv_ep_ops,
	},
	[1] = {	/* EP 1..n */
		.maxpacket	= 512,
		.name		= "ep-",
		.ops		= &mv_ep_ops,
	},
};

static struct mv_drv controller = {
	.gadget	= {
		.name	= "mv_udc",
		.ops	= &mv_udc_ops,
	},
};

/**
 * mv_get_qh() - return queue head for endpoint
 * @ep_num:	Endpoint number
 * @dir_in:	Direction of the endpoint (IN = 1, OUT = 0)
 *
 * This function returns the QH associated with particular endpoint
 * and it's direction.
 */
static struct ept_queue_head *mv_get_qh(int ep_num, int dir_in)
{
	return &controller.epts[(ep_num * 2) + dir_in];
}

/**
 * mv_get_qtd() - return queue item for endpoint
 * @ep_num:	Endpoint number
 * @dir_in:	Direction of the endpoint (IN = 1, OUT = 0)
 *
 * This function returns the QH associated with particular endpoint
 * and it's direction.
 */
static struct ept_queue_item *mv_get_qtd(int ep_num, int dir_in)
{
	return controller.items[(ep_num * 2) + dir_in];
}

/**
 * mv_flush_qh - flush cache over queue head
 * @ep_num:	Endpoint number
 *
 * This function flushes cache over QH for particular endpoint.
 */
static void mv_flush_qh(int ep_num)
{
	struct ept_queue_head *head = mv_get_qh(ep_num, 0);
	const uint32_t start = (uint32_t)head;
	const uint32_t end = start + 2 * sizeof(*head);

	flush_dcache_range(start, end);
}

/**
 * mv_invalidate_qh - invalidate cache over queue head
 * @ep_num:	Endpoint number
 *
 * This function invalidates cache over QH for particular endpoint.
 */
static void mv_invalidate_qh(int ep_num)
{
	struct ept_queue_head *head = mv_get_qh(ep_num, 0);
	uint32_t start = (uint32_t)head;
	uint32_t end = start + 2 * sizeof(*head);

	invalidate_dcache_range(start, end);
}

/**
 * mv_flush_qtd - flush cache over queue item
 * @ep_num:	Endpoint number
 *
 * This function flushes cache over qTD pair for particular endpoint.
 */
static void mv_flush_qtd(int ep_num)
{
	struct ept_queue_item *item = mv_get_qtd(ep_num, 0);
	const uint32_t start = (uint32_t)item;
	const uint32_t end_raw = start + 2 * sizeof(*item);
	const uint32_t end = roundup(end_raw, ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

/**
 * mv_invalidate_qtd - invalidate cache over queue item
 * @ep_num:	Endpoint number
 *
 * This function invalidates cache over qTD pair for particular endpoint.
 */
static void mv_invalidate_qtd(int ep_num)
{
	struct ept_queue_item *item = mv_get_qtd(ep_num, 0);
	const uint32_t start = (uint32_t)item;
	const uint32_t end_raw = start + 2 * sizeof(*item);
	const uint32_t end = roundup(end_raw, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static struct usb_request *
mv_ep_alloc_request(struct usb_ep *ep, unsigned int gfp_flags)
{
	struct mv_ep *mv_ep = container_of(ep, struct mv_ep, ep);
	return &mv_ep->req;
}

static void mv_ep_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	return;
}

static void ep_enable(int num, int in)
{
	struct ept_queue_head *head;
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	unsigned n;
	head = mv_get_qh(num, in);

	n = readl(&udc->epctrl[num]);
	if (in)
		n |= (CTRL_TXE | CTRL_TXR | CTRL_TXT_BULK);
	else
		n |= (CTRL_RXE | CTRL_RXR | CTRL_RXT_BULK);

	if (num != 0) {
		head->config = CONFIG_MAX_PKT(EP_MAX_PACKET_SIZE) | CONFIG_ZLT;
		mv_flush_qh(num);
	}
	writel(n, &udc->epctrl[num]);
}

static int mv_ep_enable(struct usb_ep *ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct mv_ep *mv_ep = container_of(ep, struct mv_ep, ep);
	int num, in;
	num = desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (desc->bEndpointAddress & USB_DIR_IN) != 0;
	ep_enable(num, in);
	mv_ep->desc = desc;
	return 0;
}

static int mv_ep_disable(struct usb_ep *ep)
{
	return 0;
}

static int mv_bounce(struct mv_ep *ep)
{
	uint32_t addr = (uint32_t)ep->req.buf;
	uint32_t ba;

	/* Input buffer address is not aligned. */
	if (addr & (ARCH_DMA_MINALIGN - 1))
		goto align;

	/* Input buffer length is not aligned. */
	if (ep->req.length & (ARCH_DMA_MINALIGN - 1))
		goto align;

	/* The buffer is well aligned, only flush cache. */
	ep->b_len = ep->req.length;
	ep->b_buf = ep->req.buf;
	goto flush;

align:
	/* Use internal buffer for small payloads. */
	if (ep->req.length <= 64) {
		ep->b_len = 64;
		ep->b_buf = ep->b_fast;
	} else {
		ep->b_len = roundup(ep->req.length, ARCH_DMA_MINALIGN);
		ep->b_buf = memalign(ARCH_DMA_MINALIGN, ep->b_len);
		if (!ep->b_buf)
			return -ENOMEM;
	}

	memcpy(ep->b_buf, ep->req.buf, ep->req.length);

flush:
	ba = (uint32_t)ep->b_buf;
	flush_dcache_range(ba, ba + ep->b_len);

	return 0;
}

static void mv_debounce(struct mv_ep *ep)
{
	uint32_t addr = (uint32_t)ep->req.buf;
	uint32_t ba = (uint32_t)ep->b_buf;

	invalidate_dcache_range(ba, ba + ep->b_len);

	/* Input buffer address is not aligned. */
	if (addr & (ARCH_DMA_MINALIGN - 1))
		goto copy;

	/* Input buffer length is not aligned. */
	if (ep->req.length & (ARCH_DMA_MINALIGN - 1))
		goto copy;

	/* The buffer is well aligned, only invalidate cache. */
	return;

copy:
	memcpy(ep->req.buf, ep->b_buf, ep->req.length);

	/* Large payloads use allocated buffer, free it. */
	if (ep->req.length > 64)
		free(ep->b_buf);
}

static int mv_ep_queue(struct usb_ep *ep,
		struct usb_request *req, gfp_t gfp_flags)
{
	struct mv_ep *mv_ep = container_of(ep, struct mv_ep, ep);
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	struct ept_queue_item *item;
	struct ept_queue_head *head;
	int bit, num, len, in, ret;
	num = mv_ep->desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (mv_ep->desc->bEndpointAddress & USB_DIR_IN) != 0;
	item = mv_get_qtd(num, in);
	head = mv_get_qh(num, in);
	len = req->length;

	ret = mv_bounce(mv_ep);
	if (ret)
		return ret;

	item->next = TERMINATE;
	item->info = INFO_BYTES(len) | INFO_IOC | INFO_ACTIVE;
	item->page0 = (uint32_t)mv_ep->b_buf;
	item->page1 = ((uint32_t)mv_ep->b_buf & 0xfffff000) + 0x1000;

	head->next = (unsigned) item;
	head->info = 0;

	DBG("ept%d %s queue len %x, buffer %p\n",
	    num, in ? "in" : "out", len, mv_ep->b_buf);

	if (in)
		bit = EPT_TX(num);
	else
		bit = EPT_RX(num);

	mv_flush_qh(num);
	mv_flush_qtd(num);

	writel(bit, &udc->epprime);

	return 0;
}

static void handle_ep_complete(struct mv_ep *ep)
{
	struct ept_queue_item *item;
	int num, in, len;
	num = ep->desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (ep->desc->bEndpointAddress & USB_DIR_IN) != 0;
	if (num == 0)
		ep->desc = &ep0_out_desc;
	item = mv_get_qtd(num, in);
	mv_invalidate_qtd(num);
	
	if (item->info & 0xff)
		printf("EP%d/%s FAIL nfo=%x pg0=%x\n",
			num, in ? "in" : "out", item->info, item->page0);

	len = (item->info >> 16) & 0x7fff;

	mv_debounce(ep);

	ep->req.length -= len;
	DBG("ept%d %s complete %x\n",
			num, in ? "in" : "out", len);
	ep->req.complete(&ep->ep, &ep->req);
	if (num == 0) {
		ep->req.length = 0;
		usb_ep_queue(&ep->ep, &ep->req, 0);
		ep->desc = &ep0_in_desc;
	}
}

#define SETUP(type, request) (((type) << 8) | (request))

static void handle_setup(void)
{
	struct usb_request *req = &controller.ep[0].req;
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	struct ept_queue_head *head;
	struct usb_ctrlrequest r;
	int status = 0;
	int num, in, _num, _in, i;
	char *buf;
	head = mv_get_qh(0, 0);	/* EP0 OUT */

	mv_invalidate_qh(0);
	memcpy(&r, head->setup_data, sizeof(struct usb_ctrlrequest));
	writel(EPT_RX(0), &udc->epstat);
	DBG("handle setup %s, %x, %x index %x value %x\n", reqname(r.bRequest),
	    r.bRequestType, r.bRequest, r.wIndex, r.wValue);

	switch (SETUP(r.bRequestType, r.bRequest)) {
	case SETUP(USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE):
		_num = r.wIndex & 15;
		_in = !!(r.wIndex & 0x80);

		if ((r.wValue == 0) && (r.wLength == 0)) {
			req->length = 0;
			for (i = 0; i < NUM_ENDPOINTS; i++) {
				if (!controller.ep[i].desc)
					continue;
				num = controller.ep[i].desc->bEndpointAddress
						& USB_ENDPOINT_NUMBER_MASK;
				in = (controller.ep[i].desc->bEndpointAddress
						& USB_DIR_IN) != 0;
				if ((num == _num) && (in == _in)) {
					ep_enable(num, in);
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
		writel((r.wValue << 25) | (1 << 24), &udc->devaddr);
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
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	writel(readl(&udc->epcomp), &udc->epcomp);
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
			head = mv_get_qh(num, in);
			head->info = INFO_ACTIVE;
			mv_flush_qh(num);
		}
	}
}

void udc_irq(void)
{
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
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
		DBG("-- portchange --\n");
		bit = (readl(&udc->portsc) >> 26) & 3;
		if (bit == 2) {
			controller.gadget.speed = USB_SPEED_HIGH;
			for (i = 1; i < NUM_ENDPOINTS && n; i++)
				if (controller.ep[i].desc)
					controller.ep[i].ep.maxpacket = 512;
		} else {
			controller.gadget.speed = USB_SPEED_FULL;
		}
	}

	if (n & STS_UEI)
		printf("<UEI %x>\n", readl(&udc->epcomp));

	if ((n & STS_UI) || (n & STS_UEI)) {
		n = readl(&udc->epstat);
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

int usb_gadget_handle_interrupts(void)
{
	u32 value;
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;

	value = readl(&udc->usbsts);
	if (value)
		udc_irq();

	return value;
}

static int mv_pullup(struct usb_gadget *gadget, int is_on)
{
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	if (is_on) {
		/* RESET */
		writel(USBCMD_ITC(MICRO_8FRAME) | USBCMD_RST, &udc->usbcmd);
		udelay(200);

		writel((unsigned)controller.epts, &udc->epinitaddr);

		/* select DEVICE mode */
		writel(USBMODE_DEVICE, &udc->usbmode);

		writel(0xffffffff, &udc->epflush);

		/* Turn on the USB connection by enabling the pullup resistor */
		writel(USBCMD_ITC(MICRO_8FRAME) | USBCMD_RUN, &udc->usbcmd);
	} else {
		stop_activity();
		writel(USBCMD_FS2, &udc->usbcmd);
		udelay(800);
		if (controller.driver)
			controller.driver->disconnect(gadget);
	}

	return 0;
}

void udc_disconnect(void)
{
	struct mv_udc *udc = (struct mv_udc *)controller.ctrl->hcor;
	/* disable pullup */
	stop_activity();
	writel(USBCMD_FS2, &udc->usbcmd);
	udelay(800);
	if (controller.driver)
		controller.driver->disconnect(&controller.gadget);
}

static int mvudc_probe(void)
{
	struct ept_queue_head *head;
	uint8_t *imem;
	int i;

	const int num = 2 * NUM_ENDPOINTS;

	const int eplist_min_align = 4096;
	const int eplist_align = roundup(eplist_min_align, ARCH_DMA_MINALIGN);
	const int eplist_raw_sz = num * sizeof(struct ept_queue_head);
	const int eplist_sz = roundup(eplist_raw_sz, ARCH_DMA_MINALIGN);

	const int ilist_align = roundup(ARCH_DMA_MINALIGN, 32);
	const int ilist_ent_raw_sz = 2 * sizeof(struct ept_queue_item);
	const int ilist_ent_sz = roundup(ilist_ent_raw_sz, ARCH_DMA_MINALIGN);
	const int ilist_sz = NUM_ENDPOINTS * ilist_ent_sz;

	/* The QH list must be aligned to 4096 bytes. */
	controller.epts = memalign(eplist_align, eplist_sz);
	if (!controller.epts)
		return -ENOMEM;
	memset(controller.epts, 0, eplist_sz);

	/*
	 * Each qTD item must be 32-byte aligned, each qTD touple must be
	 * cacheline aligned. There are two qTD items for each endpoint and
	 * only one of them is used for the endpoint at time, so we can group
	 * them together.
	 */
	controller.items_mem = memalign(ilist_align, ilist_sz);
	if (!controller.items_mem) {
		free(controller.epts);
		return -ENOMEM;
	}

	for (i = 0; i < 2 * NUM_ENDPOINTS; i++) {
		/*
		 * Configure QH for each endpoint. The structure of the QH list
		 * is such that each two subsequent fields, N and N+1 where N is
		 * even, in the QH list represent QH for one endpoint. The Nth
		 * entry represents OUT configuration and the N+1th entry does
		 * represent IN configuration of the endpoint.
		 */
		head = controller.epts + i;
		if (i < 2)
			head->config = CONFIG_MAX_PKT(EP0_MAX_PACKET_SIZE)
				| CONFIG_ZLT | CONFIG_IOS;
		else
			head->config = CONFIG_MAX_PKT(EP_MAX_PACKET_SIZE)
				| CONFIG_ZLT;
		head->next = TERMINATE;
		head->info = 0;

		imem = controller.items_mem + ((i >> 1) * ilist_ent_sz);
		if (i & 1)
			imem += sizeof(struct ept_queue_item);

		controller.items[i] = (struct ept_queue_item *)imem;

		if (i & 1) {
			mv_flush_qh(i - 1);
			mv_flush_qtd(i - 1);
		}
	}

	INIT_LIST_HEAD(&controller.gadget.ep_list);

	/* Init EP 0 */
	memcpy(&controller.ep[0].ep, &mv_ep_init[0], sizeof(*mv_ep_init));
	controller.ep[0].desc = &ep0_in_desc;
	controller.gadget.ep0 = &controller.ep[0].ep;
	INIT_LIST_HEAD(&controller.gadget.ep0->ep_list);

	/* Init EP 1..n */
	for (i = 1; i < NUM_ENDPOINTS; i++) {
		memcpy(&controller.ep[i].ep, &mv_ep_init[1],
		       sizeof(*mv_ep_init));
		list_add_tail(&controller.ep[i].ep.ep_list,
			      &controller.gadget.ep_list);
	}

	return 0;
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct mv_udc *udc;
	int ret;

	if (!driver)
		return -EINVAL;
	if (!driver->bind || !driver->setup || !driver->disconnect)
		return -EINVAL;
	if (driver->speed != USB_SPEED_FULL && driver->speed != USB_SPEED_HIGH)
		return -EINVAL;

	ret = usb_lowlevel_init(0, (void **)&controller.ctrl);
	if (ret)
		return ret;

	ret = mvudc_probe();
	if (!ret) {
		udc = (struct mv_udc *)controller.ctrl->hcor;

		/* select ULPI phy */
		writel(PTS(PTS_ENABLE) | PFSC, &udc->portsc);
	}

	ret = driver->bind(&controller.gadget);
	if (ret) {
		DBG("driver->bind() returned %d\n", ret);
		return ret;
	}
	controller.driver = driver;

	return 0;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	return 0;
}
