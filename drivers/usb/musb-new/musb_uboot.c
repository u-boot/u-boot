#include <common.h>
#include <asm/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#define __UBOOT__
#include <usb.h>
#include "linux-compat.h"
#include "usb-compat.h"
#include "musb_core.h"
#include "musb_host.h"
#include "musb_gadget.h"

#ifdef CONFIG_MUSB_HOST
static struct musb *host;
static struct usb_hcd hcd;
static enum usb_device_speed host_speed;

static void musb_host_complete_urb(struct urb *urb)
{
	urb->dev->status &= ~USB_ST_NOT_PROC;
	urb->dev->act_len = urb->actual_length;
}

static struct usb_host_endpoint hep;
static struct urb urb;

static struct urb *construct_urb(struct usb_device *dev, int endpoint_type,
				unsigned long pipe, void *buffer, int len,
				struct devrequest *setup, int interval)
{
	int epnum = usb_pipeendpoint(pipe);
	int is_in = usb_pipein(pipe);

	memset(&urb, 0, sizeof(struct urb));
	memset(&hep, 0, sizeof(struct usb_host_endpoint));
	INIT_LIST_HEAD(&hep.urb_list);
	INIT_LIST_HEAD(&urb.urb_list);
	urb.ep = &hep;
	urb.complete = musb_host_complete_urb;
	urb.status = -EINPROGRESS;
	urb.dev = dev;
	urb.pipe = pipe;
	urb.transfer_buffer = buffer;
	urb.transfer_dma = (unsigned long)buffer;
	urb.transfer_buffer_length = len;
	urb.setup_packet = (unsigned char *)setup;

	urb.ep->desc.wMaxPacketSize =
		__cpu_to_le16(is_in ? dev->epmaxpacketin[epnum] :
				dev->epmaxpacketout[epnum]);
	urb.ep->desc.bmAttributes = endpoint_type;
	urb.ep->desc.bEndpointAddress =
		(is_in ? USB_DIR_IN : USB_DIR_OUT) | epnum;
	urb.ep->desc.bInterval = interval;

	return &urb;
}

#define MUSB_HOST_TIMEOUT	0x3ffffff

static int submit_urb(struct usb_hcd *hcd, struct urb *urb)
{
	struct musb *host = hcd->hcd_priv;
	int ret;
	int timeout;

	ret = musb_urb_enqueue(hcd, urb, 0);
	if (ret < 0) {
		printf("Failed to enqueue URB to controller\n");
		return ret;
	}

	timeout = MUSB_HOST_TIMEOUT;
	do {
		if (ctrlc())
			return -EIO;
		host->isr(0, host);
	} while ((urb->dev->status & USB_ST_NOT_PROC) && --timeout);

	return urb->status;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int len, struct devrequest *setup)
{
	struct urb *urb = construct_urb(dev, USB_ENDPOINT_XFER_CONTROL, pipe,
					buffer, len, setup, 0);

	/* Fix speed for non hub-attached devices */
	if (!dev->parent)
		dev->speed = host_speed;

	return submit_urb(&hcd, urb);
}


int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
					void *buffer, int len)
{
	struct urb *urb = construct_urb(dev, USB_ENDPOINT_XFER_BULK, pipe,
					buffer, len, NULL, 0);
	return submit_urb(&hcd, urb);
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe,
				void *buffer, int len, int interval)
{
	struct urb *urb = construct_urb(dev, USB_ENDPOINT_XFER_INT, pipe,
					buffer, len, NULL, interval);
	return submit_urb(&hcd, urb);
}

int usb_lowlevel_init(int index, void **controller)
{
	u8 power;
	void *mbase;
	int timeout = MUSB_HOST_TIMEOUT;

	if (!host) {
		printf("MUSB host is not registered\n");
		return -ENODEV;
	}

	musb_start(host);
	mbase = host->mregs;
	do {
		if (musb_readb(mbase, MUSB_DEVCTL) & MUSB_DEVCTL_HM)
			break;
	} while (--timeout);
	if (!timeout)
		return -ENODEV;

	power = musb_readb(mbase, MUSB_POWER);
	musb_writeb(mbase, MUSB_POWER, MUSB_POWER_RESET | power);
	udelay(30000);
	power = musb_readb(mbase, MUSB_POWER);
	musb_writeb(mbase, MUSB_POWER, ~MUSB_POWER_RESET & power);
	host->isr(0, host);
	host_speed = (musb_readb(mbase, MUSB_POWER) & MUSB_POWER_HSMODE) ?
			USB_SPEED_HIGH :
			(musb_readb(mbase, MUSB_DEVCTL) & MUSB_DEVCTL_FSDEV) ?
			USB_SPEED_FULL : USB_SPEED_LOW;
	host->is_active = 1;
	hcd.hcd_priv = host;

	return 0;
}

int usb_lowlevel_stop(int index)
{
	if (!host) {
		printf("MUSB host is not registered\n");
		return -ENODEV;
	}

	musb_stop(host);
	return 0;
}
#endif /* CONFIG_MUSB_HOST */

#ifdef CONFIG_MUSB_GADGET
static struct musb *gadget;

int usb_gadget_handle_interrupts(void)
{
	if (!gadget || !gadget->isr)
		return -EINVAL;

	return gadget->isr(0, gadget);
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int ret;

	if (!driver || driver->speed < USB_SPEED_HIGH || !driver->bind ||
	    !driver->setup) {
		printf("bad parameter.\n");
		return -EINVAL;
	}

	if (!gadget) {
		printf("Controller uninitialized\n");
		return -ENXIO;
	}

	ret = musb_gadget_start(&gadget->g, driver);
	if (ret < 0) {
		printf("gadget_start failed with %d\n", ret);
		return ret;
	}

	ret = driver->bind(&gadget->g);
	if (ret < 0) {
		printf("bind failed with %d\n", ret);
		return ret;
	}

	return 0;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	/* TODO: implement me */
	return 0;
}
#endif /* CONFIG_MUSB_GADGET */

int musb_register(struct musb_hdrc_platform_data *plat, void *bdata,
			void *ctl_regs)
{
	struct musb **musbp;

	switch (plat->mode) {
#ifdef CONFIG_MUSB_HOST
	case MUSB_HOST:
		musbp = &host;
		break;
#endif
#ifdef CONFIG_MUSB_GADGET
	case MUSB_PERIPHERAL:
		musbp = &gadget;
		break;
#endif
	default:
		return -EINVAL;
	}

	*musbp = musb_init_controller(plat, (struct device *)bdata, ctl_regs);
	if (!musbp) {
		printf("Failed to init the controller\n");
		return -EIO;
	}

	return 0;
}
