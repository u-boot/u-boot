#ifndef __USB_COMPAT_H__
#define __USB_COMPAT_H__

#include "usb.h"

struct usb_hcd {
	void *hcd_priv;
};

struct usb_host_endpoint {
	struct usb_endpoint_descriptor		desc;
	struct list_head urb_list;
	void *hcpriv;
};

/*
 * urb->transfer_flags:
 *
 * Note: URB_DIR_IN/OUT is automatically set in usb_submit_urb().
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */

struct urb;

typedef void (*usb_complete_t)(struct urb *);

struct urb {
	void *hcpriv;			/* private data for host controller */
	struct list_head urb_list;	/* list head for use by the urb's
					 * current owner */
	struct usb_device *dev;		/* (in) pointer to associated device */
	struct usb_host_endpoint *ep;	/* (internal) pointer to endpoint */
	unsigned int pipe;		/* (in) pipe information */
	int status;			/* (return) non-ISO status */
	unsigned int transfer_flags;	/* (in) URB_SHORT_NOT_OK | ...*/
	void *transfer_buffer;		/* (in) associated data buffer */
	dma_addr_t transfer_dma;	/* (in) dma addr for transfer_buffer */
	u32 transfer_buffer_length;	/* (in) data buffer length */
	u32 actual_length;		/* (return) actual transfer length */
	unsigned char *setup_packet;	/* (in) setup packet (control only) */
	int start_frame;		/* (modify) start frame (ISO) */
	usb_complete_t complete;	/* (in) completion routine */
};

#define usb_hcd_link_urb_to_ep(hcd, urb)	({		\
	int ret = 0;						\
	list_add_tail(&urb->urb_list, &urb->ep->urb_list);	\
	ret; })
#define usb_hcd_unlink_urb_from_ep(hcd, urb)	list_del_init(&urb->urb_list)

static inline void usb_hcd_giveback_urb(struct usb_hcd *hcd,
					struct urb *urb,
					int status)
{
	urb->status = status;
	if (urb->complete)
		urb->complete(urb);
}

static inline int usb_hcd_unmap_urb_for_dma(struct usb_hcd *hcd,
					struct urb *urb)
{
	/* TODO: add cache invalidation here */
	return 0;
}

static inline u16 find_tt(struct usb_device *dev)
{
	u8 chid;
	u8 hub;

	/* Find out the nearest parent which is high speed */
	while (dev->parent->parent != NULL)
		if (dev->parent->speed != USB_SPEED_HIGH)
			dev = dev->parent;
		else
			break;

	/* determine the port address at that hub */
	hub = dev->parent->devnum;
	for (chid = 0; chid < USB_MAXCHILDREN; chid++)
		if (dev->parent->children[chid] == dev)
			break;

	return (hub << 8) | chid;
}
#endif /* __USB_COMPAT_H__ */
