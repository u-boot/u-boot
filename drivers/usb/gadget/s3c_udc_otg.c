/*
 * drivers/usb/gadget/s3c_udc_otg.c
 * Samsung S3C on-chip full/high speed USB OTG 2.0 device controllers
 *
 * Copyright (C) 2008 for Samsung Electronics
 *
 * BSP Support for Samsung's UDC driver
 * available at:
 * git://git.kernel.org/pub/scm/linux/kernel/git/kki_ap/linux-2.6-samsung.git
 *
 * State machine bugfixes:
 * Marek Szyprowski <m.szyprowski@samsung.com>
 *
 * Ported to u-boot:
 * Marek Szyprowski <m.szyprowski@samsung.com>
 * Lukasz Majewski <l.majewski@samsumg.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <asm/errno.h>
#include <linux/list.h>
#include <malloc.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <asm/io.h>

#include <asm/mach-types.h>
#include <asm/arch/gpio.h>

#include "regs-otg.h"
#include <usb/lin_gadget_compat.h>

/***********************************************************/

#define OTG_DMA_MODE		1

#undef DEBUG_S3C_UDC_SETUP
#undef DEBUG_S3C_UDC_EP0
#undef DEBUG_S3C_UDC_ISR
#undef DEBUG_S3C_UDC_OUT_EP
#undef DEBUG_S3C_UDC_IN_EP
#undef DEBUG_S3C_UDC

/* #define DEBUG_S3C_UDC_SETUP */
/* #define DEBUG_S3C_UDC_EP0 */
/* #define DEBUG_S3C_UDC_ISR */
/* #define DEBUG_S3C_UDC_OUT_EP */
/* #define DEBUG_S3C_UDC_IN_EP */
/* #define DEBUG_S3C_UDC */

#include <usb/s3c_udc.h>

#define EP0_CON		0
#define EP_MASK		0xF

static char *state_names[] = {
	"WAIT_FOR_SETUP",
	"DATA_STATE_XMIT",
	"DATA_STATE_NEED_ZLP",
	"WAIT_FOR_OUT_STATUS",
	"DATA_STATE_RECV",
	"WAIT_FOR_COMPLETE",
	"WAIT_FOR_OUT_COMPLETE",
	"WAIT_FOR_IN_COMPLETE",
	"WAIT_FOR_NULL_COMPLETE",
};

#define DRIVER_DESC "S3C HS USB OTG Device Driver, (c) Samsung Electronics"
#define DRIVER_VERSION "15 March 2009"

struct s3c_udc	*the_controller;

static const char driver_name[] = "s3c-udc";
static const char driver_desc[] = DRIVER_DESC;
static const char ep0name[] = "ep0-control";

/* Max packet size*/
static unsigned int ep0_fifo_size = 64;
static unsigned int ep_fifo_size =  512;
static unsigned int ep_fifo_size2 = 1024;
static int reset_available = 1;

static struct usb_ctrlrequest *usb_ctrl;
static dma_addr_t usb_ctrl_dma_addr;

/*
  Local declarations.
*/
static int s3c_ep_enable(struct usb_ep *ep,
			 const struct usb_endpoint_descriptor *);
static int s3c_ep_disable(struct usb_ep *ep);
static struct usb_request *s3c_alloc_request(struct usb_ep *ep,
					     gfp_t gfp_flags);
static void s3c_free_request(struct usb_ep *ep, struct usb_request *);

static int s3c_queue(struct usb_ep *ep, struct usb_request *, gfp_t gfp_flags);
static int s3c_dequeue(struct usb_ep *ep, struct usb_request *);
static int s3c_fifo_status(struct usb_ep *ep);
static void s3c_fifo_flush(struct usb_ep *ep);
static void s3c_ep0_read(struct s3c_udc *dev);
static void s3c_ep0_kick(struct s3c_udc *dev, struct s3c_ep *ep);
static void s3c_handle_ep0(struct s3c_udc *dev);
static int s3c_ep0_write(struct s3c_udc *dev);
static int write_fifo_ep0(struct s3c_ep *ep, struct s3c_request *req);
static void done(struct s3c_ep *ep, struct s3c_request *req, int status);
static void stop_activity(struct s3c_udc *dev,
			  struct usb_gadget_driver *driver);
static int udc_enable(struct s3c_udc *dev);
static void udc_set_address(struct s3c_udc *dev, unsigned char address);
static void reconfig_usbd(void);
static void set_max_pktsize(struct s3c_udc *dev, enum usb_device_speed speed);
static void nuke(struct s3c_ep *ep, int status);
static int s3c_udc_set_halt(struct usb_ep *_ep, int value);
static void s3c_udc_set_nak(struct s3c_ep *ep);

static struct usb_ep_ops s3c_ep_ops = {
	.enable = s3c_ep_enable,
	.disable = s3c_ep_disable,

	.alloc_request = s3c_alloc_request,
	.free_request = s3c_free_request,

	.queue = s3c_queue,
	.dequeue = s3c_dequeue,

	.set_halt = s3c_udc_set_halt,
	.fifo_status = s3c_fifo_status,
	.fifo_flush = s3c_fifo_flush,
};

#define create_proc_files() do {} while (0)
#define remove_proc_files() do {} while (0)

/***********************************************************/

void __iomem		*regs_otg;
struct s3c_usbotg_reg *reg;
struct s3c_usbotg_phy *phy;
static unsigned int usb_phy_ctrl;

void otg_phy_init(struct s3c_udc *dev)
{
	dev->pdata->phy_control(1);

	/*USB PHY0 Enable */
	printf("USB PHY0 Enable\n");

	/* Enable PHY */
	writel(readl(usb_phy_ctrl) | USB_PHY_CTRL_EN0, usb_phy_ctrl);

	if (dev->pdata->usb_flags == PHY0_SLEEP) /* C210 Universal */
		writel((readl(&phy->phypwr)
			&~(PHY_0_SLEEP | OTG_DISABLE_0 | ANALOG_PWRDOWN)
			&~FORCE_SUSPEND_0), &phy->phypwr);
	else /* C110 GONI */
		writel((readl(&phy->phypwr) &~(OTG_DISABLE_0 | ANALOG_PWRDOWN)
			&~FORCE_SUSPEND_0), &phy->phypwr);

	writel((readl(&phy->phyclk) &~(ID_PULLUP0 | COMMON_ON_N0)) |
	       CLK_SEL_24MHZ, &phy->phyclk); /* PLL 24Mhz */

	writel((readl(&phy->rstcon) &~(LINK_SW_RST | PHYLNK_SW_RST))
	       | PHY_SW_RST0, &phy->rstcon);
	udelay(10);
	writel(readl(&phy->rstcon)
	       &~(PHY_SW_RST0 | LINK_SW_RST | PHYLNK_SW_RST), &phy->rstcon);
	udelay(10);
}

void otg_phy_off(struct s3c_udc *dev)
{
	/* reset controller just in case */
	writel(PHY_SW_RST0, &phy->rstcon);
	udelay(20);
	writel(readl(&phy->phypwr) &~PHY_SW_RST0, &phy->rstcon);
	udelay(20);

	writel(readl(&phy->phypwr) | OTG_DISABLE_0 | ANALOG_PWRDOWN
	       | FORCE_SUSPEND_0, &phy->phypwr);

	writel(readl(usb_phy_ctrl) &~USB_PHY_CTRL_EN0, usb_phy_ctrl);

	writel((readl(&phy->phyclk) & ~(ID_PULLUP0 | COMMON_ON_N0)),
	      &phy->phyclk);

	udelay(10000);

	dev->pdata->phy_control(0);
}

/***********************************************************/

#include "s3c_udc_otg_xfer_dma.c"

/*
 *	udc_disable - disable USB device controller
 */
static void udc_disable(struct s3c_udc *dev)
{
	DEBUG_SETUP("%s: %p\n", __func__, dev);

	udc_set_address(dev, 0);

	dev->ep0state = WAIT_FOR_SETUP;
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->usb_address = 0;

	otg_phy_off(dev);
}

/*
 *	udc_reinit - initialize software state
 */
static void udc_reinit(struct s3c_udc *dev)
{
	unsigned int i;

	DEBUG_SETUP("%s: %p\n", __func__, dev);

	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	dev->ep0state = WAIT_FOR_SETUP;

	/* basic endpoint records init */
	for (i = 0; i < S3C_MAX_ENDPOINTS; i++) {
		struct s3c_ep *ep = &dev->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);

		ep->desc = 0;
		ep->stopped = 0;
		INIT_LIST_HEAD(&ep->queue);
		ep->pio_irqs = 0;
	}

	/* the rest was statically initialized, and is read-only */
}

#define BYTES2MAXP(x)	(x / 8)
#define MAXP2BYTES(x)	(x * 8)

/* until it's enabled, this UDC should be completely invisible
 * to any USB host.
 */
static int udc_enable(struct s3c_udc *dev)
{
	DEBUG_SETUP("%s: %p\n", __func__, dev);

	otg_phy_init(dev);
	reconfig_usbd();

	DEBUG_SETUP("S3C USB 2.0 OTG Controller Core Initialized : 0x%x\n",
		    readl(&reg->gintmsk));

	dev->gadget.speed = USB_SPEED_UNKNOWN;

	return 0;
}

/*
  Register entry point for the peripheral controller driver.
*/
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct s3c_udc *dev = the_controller;
	int retval = 0;
	unsigned long flags;

	DEBUG_SETUP("%s: %s\n", __func__, "no name");

	if (!driver
	    || (driver->speed != USB_SPEED_FULL
		&& driver->speed != USB_SPEED_HIGH)
	    || !driver->bind || !driver->disconnect || !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;

	spin_lock_irqsave(&dev->lock, flags);
	/* first hook up the driver ... */
	dev->driver = driver;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (retval) { /* TODO */
		printf("target device_add failed, error %d\n", retval);
		return retval;
	}

	retval = driver->bind(&dev->gadget);
	if (retval) {
		DEBUG_SETUP("%s: bind to driver --> error %d\n",
			    dev->gadget.name, retval);
		dev->driver = 0;
		return retval;
	}

	enable_irq(IRQ_OTG);

	DEBUG_SETUP("Registered gadget driver %s\n", dev->gadget.name);
	udc_enable(dev);

	return 0;
}

/*
 * Unregister entry point for the peripheral controller driver.
 */
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct s3c_udc *dev = the_controller;
	unsigned long flags;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);
	dev->driver = 0;
	stop_activity(dev, driver);
	spin_unlock_irqrestore(&dev->lock, flags);

	driver->unbind(&dev->gadget);

	disable_irq(IRQ_OTG);

	udc_disable(dev);
	return 0;
}

/*
 *	done - retire a request; caller blocked irqs
 */
static void done(struct s3c_ep *ep, struct s3c_request *req, int status)
{
	unsigned int stopped = ep->stopped;

	debug("%s: %s %p, req = %p, stopped = %d\n",
	      __func__, ep->ep.name, ep, &req->req, stopped);

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN) {
		debug("complete %s req %p stat %d len %u/%u\n",
		      ep->ep.name, &req->req, status,
		      req->req.actual, req->req.length);
	}

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

#ifdef DEBUG_S3C_UDC
	printf("calling complete callback\n");
	{
		int i, len = req->req.length;

		printf("pkt[%d] = ", req->req.length);
		if (len > 64)
			len = 64;
		for (i = 0; i < len; i++) {
			printf("%02x", ((u8 *)req->req.buf)[i]);
			if ((i & 7) == 7)
				printf(" ");
		}
		printf("\n");
	}
#endif
	spin_unlock(&ep->dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->dev->lock);

	debug("callback completed\n");

	ep->stopped = stopped;
}

/*
 *	nuke - dequeue ALL requests
 */
static void nuke(struct s3c_ep *ep, int status)
{
	struct s3c_request *req;

	debug("%s: %s %p\n", __func__, ep->ep.name, ep);

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct s3c_request, queue);
		done(ep, req, status);
	}
}

static void stop_activity(struct s3c_udc *dev,
			  struct usb_gadget_driver *driver)
{
	int i;

	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;
	dev->gadget.speed = USB_SPEED_UNKNOWN;

	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < S3C_MAX_ENDPOINTS; i++) {
		struct s3c_ep *ep = &dev->ep[i];
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	/* re-init driver-visible data structures */
	udc_reinit(dev);
}

static void reconfig_usbd(void)
{
	/* 2. Soft-reset OTG Core and then unreset again. */
	int i;
	unsigned int uTemp = writel(CORE_SOFT_RESET, &reg->grstctl);

	debug("Reseting OTG controller\n");

	writel(0<<15		/* PHY Low Power Clock sel*/
		|1<<14		/* Non-Periodic TxFIFO Rewind Enable*/
		|0x5<<10	/* Turnaround time*/
		|0<<9 | 0<<8	/* [0:HNP disable,1:HNP enable][ 0:SRP disable*/
				/* 1:SRP enable] H1= 1,1*/
		|0<<7		/* Ulpi DDR sel*/
		|0<<6		/* 0: high speed utmi+, 1: full speed serial*/
		|0<<4		/* 0: utmi+, 1:ulpi*/
		|1<<3		/* phy i/f  0:8bit, 1:16bit*/
		|0x7<<0,	/* HS/FS Timeout**/
		&reg->gusbcfg);

	/* 3. Put the OTG device core in the disconnected state.*/
	uTemp = readl(&reg->dctl);
	uTemp |= SOFT_DISCONNECT;
	writel(uTemp, &reg->dctl);

	udelay(20);

	/* 4. Make the OTG device core exit from the disconnected state.*/
	uTemp = readl(&reg->dctl);
	uTemp = uTemp & ~SOFT_DISCONNECT;
	writel(uTemp, &reg->dctl);

	/* 5. Configure OTG Core to initial settings of device mode.*/
	/* [][1: full speed(30Mhz) 0:high speed]*/
	writel(EP_MISS_CNT(1) | DEV_SPEED_HIGH_SPEED_20, &reg->dcfg);

	mdelay(1);

	/* 6. Unmask the core interrupts*/
	writel(GINTMSK_INIT, &reg->gintmsk);

	/* 7. Set NAK bit of EP0, EP1, EP2*/
	writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->out_endp[EP0_CON].doepctl);
	writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->in_endp[EP0_CON].diepctl);

	for (i = 1; i < S3C_MAX_ENDPOINTS; i++) {
		writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->out_endp[i].doepctl);
		writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->in_endp[i].diepctl);
	}

	/* 8. Unmask EPO interrupts*/
	writel(((1 << EP0_CON) << DAINT_OUT_BIT)
	       | (1 << EP0_CON), &reg->daintmsk);

	/* 9. Unmask device OUT EP common interrupts*/
	writel(DOEPMSK_INIT, &reg->doepmsk);

	/* 10. Unmask device IN EP common interrupts*/
	writel(DIEPMSK_INIT, &reg->diepmsk);

	/* 11. Set Rx FIFO Size (in 32-bit words) */
	writel(RX_FIFO_SIZE >> 2, &reg->grxfsiz);

	/* 12. Set Non Periodic Tx FIFO Size */
	writel((NPTX_FIFO_SIZE >> 2) << 16 | ((RX_FIFO_SIZE >> 2)) << 0,
	       &reg->gnptxfsiz);

	for (i = 1; i < S3C_MAX_HW_ENDPOINTS; i++)
		writel((PTX_FIFO_SIZE >> 2) << 16 |
		       ((RX_FIFO_SIZE + NPTX_FIFO_SIZE +
			 PTX_FIFO_SIZE*(i-1)) >> 2) << 0,
		       &reg->dieptxf[i-1]);

	/* Flush the RX FIFO */
	writel(RX_FIFO_FLUSH, &reg->grstctl);
	while (readl(&reg->grstctl) & RX_FIFO_FLUSH)
		debug("%s: waiting for S3C_UDC_OTG_GRSTCTL\n", __func__);

	/* Flush all the Tx FIFO's */
	writel(TX_FIFO_FLUSH_ALL, &reg->grstctl);
	writel(TX_FIFO_FLUSH_ALL | TX_FIFO_FLUSH, &reg->grstctl);
	while (readl(&reg->grstctl) & TX_FIFO_FLUSH)
		debug("%s: waiting for S3C_UDC_OTG_GRSTCTL\n", __func__);

	/* 13. Clear NAK bit of EP0, EP1, EP2*/
	/* For Slave mode*/
	/* EP0: Control OUT */
	writel(DEPCTL_EPDIS | DEPCTL_CNAK,
	       &reg->out_endp[EP0_CON].doepctl);

	/* 14. Initialize OTG Link Core.*/
	writel(GAHBCFG_INIT, &reg->gahbcfg);
}

static void set_max_pktsize(struct s3c_udc *dev, enum usb_device_speed speed)
{
	unsigned int ep_ctrl;
	int i;

	if (speed == USB_SPEED_HIGH) {
		ep0_fifo_size = 64;
		ep_fifo_size = 512;
		ep_fifo_size2 = 1024;
		dev->gadget.speed = USB_SPEED_HIGH;
	} else {
		ep0_fifo_size = 64;
		ep_fifo_size = 64;
		ep_fifo_size2 = 64;
		dev->gadget.speed = USB_SPEED_FULL;
	}

	dev->ep[0].ep.maxpacket = ep0_fifo_size;
	for (i = 1; i < S3C_MAX_ENDPOINTS; i++)
		dev->ep[i].ep.maxpacket = ep_fifo_size;

	/* EP0 - Control IN (64 bytes)*/
	ep_ctrl = readl(&reg->in_endp[EP0_CON].diepctl);
	writel(ep_ctrl|(0<<0), &reg->in_endp[EP0_CON].diepctl);

	/* EP0 - Control OUT (64 bytes)*/
	ep_ctrl = readl(&reg->out_endp[EP0_CON].doepctl);
	writel(ep_ctrl|(0<<0), &reg->out_endp[EP0_CON].doepctl);
}

static int s3c_ep_enable(struct usb_ep *_ep,
			 const struct usb_endpoint_descriptor *desc)
{
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
	    || desc->bDescriptorType != USB_DT_ENDPOINT
	    || ep->bEndpointAddress != desc->bEndpointAddress
	    || ep_maxpacket(ep) <
	    le16_to_cpu(get_unaligned(&desc->wMaxPacketSize))) {

		debug("%s: bad ep or descriptor\n", __func__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
	    && ep->bmAttributes != USB_ENDPOINT_XFER_BULK
	    && desc->bmAttributes != USB_ENDPOINT_XFER_INT) {

		debug("%s: %s type mismatch\n", __func__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
	     && le16_to_cpu(get_unaligned(&desc->wMaxPacketSize)) !=
	     ep_maxpacket(ep)) || !get_unaligned(&desc->wMaxPacketSize)) {

		debug("%s: bad %s maxpacket\n", __func__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {

		debug("%s: bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	ep->stopped = 0;
	ep->desc = desc;
	ep->pio_irqs = 0;
	ep->ep.maxpacket = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	/* Reset halt state */
	s3c_udc_set_nak(ep);
	s3c_udc_set_halt(_ep, 0);

	spin_lock_irqsave(&ep->dev->lock, flags);
	s3c_udc_ep_activate(ep);
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	debug("%s: enabled %s, stopped = %d, maxpacket = %d\n",
	      __func__, _ep->name, ep->stopped, ep->ep.maxpacket);
	return 0;
}

/*
 * Disable EP
 */
static int s3c_ep_disable(struct usb_ep *_ep)
{
	struct s3c_ep *ep;
	unsigned long flags;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || !ep->desc) {
		debug("%s: %s not enabled\n", __func__,
		      _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* Nuke all pending requests */
	nuke(ep, -ESHUTDOWN);

	ep->desc = 0;
	ep->stopped = 1;

	spin_unlock_irqrestore(&ep->dev->lock, flags);

	debug("%s: disabled %s\n", __func__, _ep->name);
	return 0;
}

static struct usb_request *s3c_alloc_request(struct usb_ep *ep,
					     gfp_t gfp_flags)
{
	struct s3c_request *req;

	debug("%s: %s %p\n", __func__, ep->name, ep);

	req = kmalloc(sizeof *req, gfp_flags);
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void s3c_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct s3c_request *req;

	debug("%s: %p\n", __func__, ep);

	req = container_of(_req, struct s3c_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/* dequeue JUST ONE request */
static int s3c_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct s3c_ep *ep;
	struct s3c_request *req;
	unsigned long flags;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ep->dev->lock, flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);

	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return 0;
}

/*
 * Return bytes in EP FIFO
 */
static int s3c_fifo_status(struct usb_ep *_ep)
{
	int count = 0;
	struct s3c_ep *ep;

	ep = container_of(_ep, struct s3c_ep, ep);
	if (!_ep) {
		debug("%s: bad ep\n", __func__);
		return -ENODEV;
	}

	debug("%s: %d\n", __func__, ep_index(ep));

	/* LPD can't report unclaimed bytes from IN fifos */
	if (ep_is_in(ep))
		return -EOPNOTSUPP;

	return count;
}

/*
 * Flush EP FIFO
 */
static void s3c_fifo_flush(struct usb_ep *_ep)
{
	struct s3c_ep *ep;

	ep = container_of(_ep, struct s3c_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		debug("%s: bad ep\n", __func__);
		return;
	}

	debug("%s: %d\n", __func__, ep_index(ep));
}

static const struct usb_gadget_ops s3c_udc_ops = {
	/* current versions must always be self-powered */
};

static struct s3c_udc memory = {
	.usb_address = 0,
	.gadget = {
		.ops = &s3c_udc_ops,
		.ep0 = &memory.ep[0].ep,
		.name = driver_name,
	},

	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name = ep0name,
			.ops = &s3c_ep_ops,
			.maxpacket = EP0_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = 0,
		.bmAttributes = 0,

		.ep_type = ep_control,
	},

	/* first group of endpoints */
	.ep[1] = {
		.ep = {
			.name = "ep1in-bulk",
			.ops = &s3c_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_out,
		.fifo_num = 1,
	},

	.ep[2] = {
		.ep = {
			.name = "ep2out-bulk",
			.ops = &s3c_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_OUT | 2,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_in,
		.fifo_num = 2,
	},

	.ep[3] = {
		.ep = {
			.name = "ep3in-int",
			.ops = &s3c_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 3,
		.bmAttributes = USB_ENDPOINT_XFER_INT,

		.ep_type = ep_interrupt,
		.fifo_num = 3,
	},
};

/*
 *	probe - binds to the platform device
 */

int s3c_udc_probe(struct s3c_plat_otg_data *pdata)
{
	struct s3c_udc *dev = &memory;
	int retval = 0, i;

	debug("%s: %p\n", __func__, pdata);

	dev->pdata = pdata;

	phy = (struct s3c_usbotg_phy *)pdata->regs_phy;
	reg = (struct s3c_usbotg_reg *)pdata->regs_otg;
	usb_phy_ctrl = pdata->usb_phy_ctrl;

	/* regs_otg = (void *)pdata->regs_otg; */

	dev->gadget.is_dualspeed = 1;	/* Hack only*/
	dev->gadget.is_otg = 0;
	dev->gadget.is_a_peripheral = 0;
	dev->gadget.b_hnp_enable = 0;
	dev->gadget.a_hnp_support = 0;
	dev->gadget.a_alt_hnp_support = 0;

	the_controller = dev;

	for (i = 0; i < S3C_MAX_ENDPOINTS+1; i++) {
		dev->dma_buf[i] = kmalloc(DMA_BUFFER_SIZE, GFP_KERNEL);
		dev->dma_addr[i] = (dma_addr_t) dev->dma_buf[i];
		invalidate_dcache_range((unsigned long) dev->dma_buf[i],
					(unsigned long) (dev->dma_buf[i]
							 + DMA_BUFFER_SIZE));
	}
	usb_ctrl = dev->dma_buf[0];
	usb_ctrl_dma_addr = dev->dma_addr[0];

	udc_reinit(dev);

	return retval;
}

int usb_gadget_handle_interrupts()
{
	u32 intr_status = readl(&reg->gintsts);
	u32 gintmsk = readl(&reg->gintmsk);

	if (intr_status & gintmsk)
		return s3c_udc_irq(1, (void *)the_controller);
	return 0;
}
