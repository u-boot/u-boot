/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#define PAGE_SIZE	4096
#define QH_MAXNUM	32
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

struct ept_queue_head *epts;
struct ept_queue_item *items[2 * NUM_ENDPOINTS];
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

static struct mv_ep ep[2 * NUM_ENDPOINTS];
static struct mv_drv controller = {
	.gadget = {
		.ep0 = &ep[0].ep,
		.name = "mv_udc",
	},
};

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
	struct mv_udc *udc = controller.udc;
	unsigned n;
	head = epts + 2*num + in;

	n = readl(&udc->epctrl[num]);
	if (in)
		n |= (CTRL_TXE | CTRL_TXR | CTRL_TXT_BULK);
	else
		n |= (CTRL_RXE | CTRL_RXR | CTRL_RXT_BULK);

	if (num != 0)
		head->config = CONFIG_MAX_PKT(EP_MAX_PACKET_SIZE) | CONFIG_ZLT;
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

static int mv_ep_queue(struct usb_ep *ep,
		struct usb_request *req, gfp_t gfp_flags)
{
	struct mv_ep *mv_ep = container_of(ep, struct mv_ep, ep);
	struct mv_udc *udc = controller.udc;
	struct ept_queue_item *item;
	struct ept_queue_head *head;
	unsigned phys;
	int bit, num, len, in;
	num = mv_ep->desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	in = (mv_ep->desc->bEndpointAddress & USB_DIR_IN) != 0;
	item = items[2 * num + in];
	head = epts + 2 * num + in;
	phys = (unsigned)req->buf;
	len = req->length;

	item->next = TERMINATE;
	item->info = INFO_BYTES(len) | INFO_IOC | INFO_ACTIVE;
	item->page0 = phys;
	item->page1 = (phys & 0xfffff000) + 0x1000;

	head->next = (unsigned) item;
	head->info = 0;

	DBG("ept%d %s queue len %x, buffer %x\n",
			num, in ? "in" : "out", len, phys);

	if (in)
		bit = EPT_TX(num);
	else
		bit = EPT_RX(num);

	flush_cache(phys, len);
	flush_cache((unsigned long)item, sizeof(struct ept_queue_item));
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
	item = items[2 * num + in];

	if (item->info & 0xff)
		printf("EP%d/%s FAIL nfo=%x pg0=%x\n",
			num, in ? "in" : "out", item->info, item->page0);

	len = (item->info >> 16) & 0x7fff;
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
	struct usb_request *req = &ep[0].req;
	struct mv_udc *udc = controller.udc;
	struct ept_queue_head *head;
	struct usb_ctrlrequest r;
	int status = 0;
	int num, in, _num, _in, i;
	char *buf;
	head = epts;

	flush_cache((unsigned long)head, sizeof(struct ept_queue_head));
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
				if (!ep[i].desc)
					continue;
				num = ep[i].desc->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK;
				in = (ep[i].desc->bEndpointAddress
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
	struct mv_udc *udc = controller.udc;
	writel(readl(&udc->epcomp), &udc->epcomp);
	writel(readl(&udc->epstat), &udc->epstat);
	writel(0xffffffff, &udc->epflush);

	/* error out any pending reqs */
	for (i = 0; i < NUM_ENDPOINTS; i++) {
		if (i != 0)
			writel(0, &udc->epctrl[i]);
		if (ep[i].desc) {
			num = ep[i].desc->bEndpointAddress
				& USB_ENDPOINT_NUMBER_MASK;
			in = (ep[i].desc->bEndpointAddress & USB_DIR_IN) != 0;
			head = epts + (num * 2) + (in);
			head->info = INFO_ACTIVE;
		}
	}
}

void udc_irq(void)
{
	struct mv_udc *udc = controller.udc;
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
				if (ep[i].desc)
					ep[i].ep.maxpacket = 512;
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
			if (ep[i].desc) {
				num = ep[i].desc->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK;
				in = (ep[i].desc->bEndpointAddress
						& USB_DIR_IN) != 0;
				bit = (in) ? EPT_TX(num) : EPT_RX(num);
				if (n & bit)
					handle_ep_complete(&ep[i]);
			}
		}
	}
}

int usb_gadget_handle_interrupts(void)
{
	u32 value;
	struct mv_udc *udc = controller.udc;

	value = readl(&udc->usbsts);
	if (value)
		udc_irq();

	return value;
}

static int mv_pullup(struct usb_gadget *gadget, int is_on)
{
	struct mv_udc *udc = controller.udc;
	if (is_on) {
		/* RESET */
		writel(USBCMD_ITC(MICRO_8FRAME) | USBCMD_RST, &udc->usbcmd);
		udelay(200);

		writel((unsigned) epts, &udc->epinitaddr);

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
	struct mv_udc *udc = controller.udc;
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
	int i;

	controller.gadget.ops = &mv_udc_ops;
	controller.udc = (struct mv_udc *)CONFIG_USB_REG_BASE;
	epts = memalign(PAGE_SIZE, QH_MAXNUM * sizeof(struct ept_queue_head));
	memset(epts, 0, QH_MAXNUM * sizeof(struct ept_queue_head));
	for (i = 0; i < 2 * NUM_ENDPOINTS; i++) {
		/*
		 * For item0 and item1, they are served as ep0
		 * out&in seperately
		 */
		head = epts + i;
		if (i < 2)
			head->config = CONFIG_MAX_PKT(EP0_MAX_PACKET_SIZE)
				| CONFIG_ZLT | CONFIG_IOS;
		else
			head->config = CONFIG_MAX_PKT(EP_MAX_PACKET_SIZE)
				| CONFIG_ZLT;
		head->next = TERMINATE;
		head->info = 0;

		items[i] = memalign(PAGE_SIZE, sizeof(struct ept_queue_item));
	}

	INIT_LIST_HEAD(&controller.gadget.ep_list);
	ep[0].ep.maxpacket = 64;
	ep[0].ep.name = "ep0";
	ep[0].desc = &ep0_in_desc;
	INIT_LIST_HEAD(&controller.gadget.ep0->ep_list);
	for (i = 0; i < 2 * NUM_ENDPOINTS; i++) {
		if (i != 0) {
			ep[i].ep.maxpacket = 512;
			ep[i].ep.name = "ep-";
			list_add_tail(&ep[i].ep.ep_list,
				      &controller.gadget.ep_list);
			ep[i].desc = NULL;
		}
		ep[i].ep.ops = &mv_ep_ops;
	}
	return 0;
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct mv_udc *udc = controller.udc;
	int             retval;

	if (!driver
			|| driver->speed < USB_SPEED_FULL
			|| !driver->bind
			|| !driver->setup) {
		DBG("bad parameter.\n");
		return -EINVAL;
	}

	if (!mvudc_probe()) {
		usb_lowlevel_init();
		/* select ULPI phy */
		writel(PTS(PTS_ENABLE) | PFSC, &udc->portsc);
	}
	retval = driver->bind(&controller.gadget);
	if (retval) {
		DBG("driver->bind() returned %d\n", retval);
		return retval;
	}
	controller.driver = driver;

	return 0;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	return 0;
}
