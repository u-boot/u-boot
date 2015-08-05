#include <common.h>
#include <watchdog.h>
#ifdef CONFIG_ARCH_SUNXI
#include <asm/arch/usb_phy.h>
#endif
#include <asm/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <usb.h>
#include "linux-compat.h"
#include "usb-compat.h"
#include "musb_core.h"
#include "musb_host.h"
#include "musb_gadget.h"

#ifdef CONFIG_MUSB_HOST
struct int_queue {
	struct usb_host_endpoint hep;
	struct urb urb;
};

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

static void construct_urb(struct urb *urb, struct usb_host_endpoint *hep,
			  struct usb_device *dev, int endpoint_type,
			  unsigned long pipe, void *buffer, int len,
			  struct devrequest *setup, int interval)
{
	int epnum = usb_pipeendpoint(pipe);
	int is_in = usb_pipein(pipe);

	memset(urb, 0, sizeof(struct urb));
	memset(hep, 0, sizeof(struct usb_host_endpoint));
	INIT_LIST_HEAD(&hep->urb_list);
	INIT_LIST_HEAD(&urb->urb_list);
	urb->ep = hep;
	urb->complete = musb_host_complete_urb;
	urb->status = -EINPROGRESS;
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = buffer;
	urb->transfer_dma = (unsigned long)buffer;
	urb->transfer_buffer_length = len;
	urb->setup_packet = (unsigned char *)setup;

	urb->ep->desc.wMaxPacketSize =
		__cpu_to_le16(is_in ? dev->epmaxpacketin[epnum] :
				dev->epmaxpacketout[epnum]);
	urb->ep->desc.bmAttributes = endpoint_type;
	urb->ep->desc.bEndpointAddress =
		(is_in ? USB_DIR_IN : USB_DIR_OUT) | epnum;
	urb->ep->desc.bInterval = interval;
}

static int submit_urb(struct usb_hcd *hcd, struct urb *urb)
{
	struct musb *host = hcd->hcd_priv;
	int ret;
	unsigned long timeout;

	ret = musb_urb_enqueue(hcd, urb, 0);
	if (ret < 0) {
		printf("Failed to enqueue URB to controller\n");
		return ret;
	}

	timeout = get_timer(0) + USB_TIMEOUT_MS(urb->pipe);
	do {
		if (ctrlc())
			return -EIO;
		host->isr(0, host);
	} while (urb->status == -EINPROGRESS &&
		 get_timer(0) < timeout);

	if (urb->status == -EINPROGRESS)
		musb_urb_dequeue(hcd, urb, -ETIME);

	return urb->status;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int len, struct devrequest *setup)
{
	construct_urb(&urb, &hep, dev, USB_ENDPOINT_XFER_CONTROL, pipe,
		      buffer, len, setup, 0);

	/* Fix speed for non hub-attached devices */
	if (!dev->parent)
		dev->speed = host_speed;

	return submit_urb(&hcd, &urb);
}


int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
					void *buffer, int len)
{
	construct_urb(&urb, &hep, dev, USB_ENDPOINT_XFER_BULK, pipe,
		      buffer, len, NULL, 0);
	return submit_urb(&hcd, &urb);
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe,
				void *buffer, int len, int interval)
{
	construct_urb(&urb, &hep, dev, USB_ENDPOINT_XFER_INT, pipe,
		      buffer, len, NULL, interval);
	return submit_urb(&hcd, &urb);
}

struct int_queue *create_int_queue(struct usb_device *dev, unsigned long pipe,
	int queuesize, int elementsize, void *buffer, int interval)
{
	struct int_queue *queue;
	int ret, index = usb_pipein(pipe) * 16 + usb_pipeendpoint(pipe);

	if (queuesize != 1) {
		printf("ERROR musb int-queues only support queuesize 1\n");
		return NULL;
	}

	if (dev->int_pending & (1 << index)) {
		printf("ERROR int-urb is already pending on pipe %lx\n", pipe);
		return NULL;
	}

	queue = malloc(sizeof(*queue));
	if (!queue)
		return NULL;

	construct_urb(&queue->urb, &queue->hep, dev, USB_ENDPOINT_XFER_INT,
		      pipe, buffer, elementsize, NULL, interval);

	ret = musb_urb_enqueue(&hcd, &queue->urb, 0);
	if (ret < 0) {
		printf("Failed to enqueue URB to controller\n");
		free(queue);
		return NULL;
	}

	dev->int_pending |= 1 << index;
	return queue;
}

int destroy_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	int index = usb_pipein(queue->urb.pipe) * 16 + 
		    usb_pipeendpoint(queue->urb.pipe);

	if (queue->urb.status == -EINPROGRESS)
		musb_urb_dequeue(&hcd, &queue->urb, -ETIME);

	dev->int_pending &= ~(1 << index);
	free(queue);
	return 0;
}

void *poll_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	if (queue->urb.status != -EINPROGRESS)
		return NULL; /* URB has already completed in a prev. poll */

	host->isr(0, host);

	if (queue->urb.status != -EINPROGRESS)
		return queue->urb.transfer_buffer; /* Done */

	return NULL; /* URB still pending */
}

int usb_reset_root_port(void)
{
	void *mbase = host->mregs;
	u8 power;

	power = musb_readb(mbase, MUSB_POWER);
	power &= 0xf0;
	musb_writeb(mbase, MUSB_POWER, MUSB_POWER_RESET | power);
	mdelay(50);
#ifdef CONFIG_ARCH_SUNXI
	/*
	 * sunxi phy has a bug and it will wrongly detect high speed squelch
	 * when clearing reset on low-speed devices, temporary disable
	 * squelch detection to work around this.
	 */
	sunxi_usb_phy_enable_squelch_detect(0, 0);
#endif
	power = musb_readb(mbase, MUSB_POWER);
	musb_writeb(mbase, MUSB_POWER, ~MUSB_POWER_RESET & power);
#ifdef CONFIG_ARCH_SUNXI
	sunxi_usb_phy_enable_squelch_detect(0, 1);
#endif
	host->isr(0, host);
	host_speed = (musb_readb(mbase, MUSB_POWER) & MUSB_POWER_HSMODE) ?
			USB_SPEED_HIGH :
			(musb_readb(mbase, MUSB_DEVCTL) & MUSB_DEVCTL_FSDEV) ?
			USB_SPEED_FULL : USB_SPEED_LOW;
	mdelay((host_speed == USB_SPEED_LOW) ? 200 : 50);

	return 0;
}

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	void *mbase;
	/* USB spec says it may take up to 1 second for a device to connect */
	unsigned long timeout = get_timer(0) + 1000;

	if (!host) {
		printf("MUSB host is not registered\n");
		return -ENODEV;
	}

	musb_start(host);
	mbase = host->mregs;
	do {
		if (musb_readb(mbase, MUSB_DEVCTL) & MUSB_DEVCTL_HM)
			break;
	} while (get_timer(0) < timeout);
	if (get_timer(0) >= timeout)
		return -ENODEV;

	usb_reset_root_port();
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

int usb_gadget_handle_interrupts(int index)
{
	WATCHDOG_RESET();
	if (!gadget || !gadget->isr)
		return -EINVAL;

	return gadget->isr(0, gadget);
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int ret;

	if (!driver || driver->speed < USB_SPEED_FULL || !driver->bind ||
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
	if (driver->disconnect)
		driver->disconnect(&gadget->g);
	if (driver->unbind)
		driver->unbind(&gadget->g);
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
