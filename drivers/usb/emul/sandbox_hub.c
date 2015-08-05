/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

/* We only support up to 8 */
#define SANDBOX_NUM_PORTS	2

struct sandbox_hub_platdata {
	struct usb_dev_platdata plat;
	int port;	/* Port number (numbered from 0) */
};

enum {
	STRING_MANUFACTURER = 1,
	STRING_PRODUCT,
	STRING_SERIAL,

	STRING_count,
};

static struct usb_string hub_strings[] = {
	{STRING_MANUFACTURER,	"sandbox"},
	{STRING_PRODUCT,	"hub"},
	{STRING_SERIAL,		"2345"},
	{},
};

static struct usb_device_descriptor hub_device_desc = {
	.bLength =		sizeof(hub_device_desc),
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),

	.bDeviceClass =		USB_CLASS_HUB,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,

	.idVendor =		__constant_cpu_to_le16(0x1234),
	.idProduct =		__constant_cpu_to_le16(0x5678),
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
};

static struct usb_config_descriptor hub_config1 = {
	.bLength		= sizeof(hub_config1),
	.bDescriptorType	= USB_DT_CONFIG,

	/* wTotalLength is set up by usb-emul-uclass */
	.bNumInterfaces		= 1,
	.bConfigurationValue	= 0,
	.iConfiguration		= 0,
	.bmAttributes		= 1 << 7,
	.bMaxPower		= 50,
};

static struct usb_interface_descriptor hub_interface0 = {
	.bLength		= sizeof(hub_interface0),
	.bDescriptorType	= USB_DT_INTERFACE,

	.bInterfaceNumber	= 0,
	.bAlternateSetting	= 0,
	.bNumEndpoints		= 1,
	.bInterfaceClass	= USB_CLASS_HUB,
	.bInterfaceSubClass	= 0,
	.bInterfaceProtocol	= US_PR_CB,
	.iInterface		= 0,
};

static struct usb_endpoint_descriptor hub_endpoint0_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,

	.bEndpointAddress	= 1 | USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize		= __constant_cpu_to_le16(1024),
	.bInterval		= 0,
};

static struct usb_hub_descriptor hub_desc = {
	.bLength		= sizeof(hub_desc),
	.bDescriptorType	= USB_DT_HUB,
	.bNbrPorts		= SANDBOX_NUM_PORTS,
	.wHubCharacteristics	= __constant_cpu_to_le16(1 << 0 | 1 << 3 |
								1 << 7),
	.bPwrOn2PwrGood		= 2,
	.bHubContrCurrent	= 5,
	.DeviceRemovable	= {0, 0xff}, /* all ports removeable */
#if SANDBOX_NUM_PORTS > 8
#error "This code sets up an incorrect mask"
#endif
};

static void *hub_desc_list[] = {
	&hub_device_desc,
	&hub_config1,
	&hub_interface0,
	&hub_endpoint0_in,
	&hub_desc,
	NULL,
};

struct sandbox_hub_priv {
	int status[SANDBOX_NUM_PORTS];
	int change[SANDBOX_NUM_PORTS];
};

static struct udevice *hub_find_device(struct udevice *hub, int port)
{
	struct udevice *dev;

	for (device_find_first_child(hub, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct sandbox_hub_platdata *plat;

		plat = dev_get_parent_platdata(dev);
		if (plat->port == port)
			return dev;
	}

	return NULL;
}

static int clrset_post_state(struct udevice *hub, int port, int clear, int set)
{
	struct sandbox_hub_priv *priv = dev_get_priv(hub);
	int *status = &priv->status[port];
	int *change = &priv->change[port];
	int ret = 0;

	if ((clear | set) & USB_PORT_STAT_POWER) {
		struct udevice *dev = hub_find_device(hub, port);

		if (dev) {
			if (set & USB_PORT_STAT_POWER) {
				ret = device_probe(dev);
				debug("%s: %s: power on, probed, ret=%d\n",
				      __func__, dev->name, ret);
				if (!ret) {
					set |= USB_PORT_STAT_CONNECTION |
						USB_PORT_STAT_ENABLE;
				}

			} else if (clear & USB_PORT_STAT_POWER) {
				debug("%s: %s: power off, removed, ret=%d\n",
				      __func__, dev->name, ret);
				ret = device_remove(dev);
				clear |= USB_PORT_STAT_CONNECTION;
			}
		}
	}
	*change |= *status & clear;
	*change |= ~*status & set;
	*change &= 0x1f;
	*status = (*status & ~clear) | set;

	return ret;
}

static int sandbox_hub_submit_control_msg(struct udevice *bus,
					  struct usb_device *udev,
					  unsigned long pipe,
					  void *buffer, int length,
					  struct devrequest *setup)
{
	struct sandbox_hub_priv *priv = dev_get_priv(bus);
	int ret = 0;

	if (pipe == usb_rcvctrlpipe(udev, 0)) {
		switch (setup->requesttype) {
		case USB_RT_HUB | USB_DIR_IN:
			switch (setup->request) {
			case USB_REQ_GET_STATUS: {
				struct usb_hub_status *hubsts = buffer;

				hubsts->wHubStatus = 0;
				hubsts->wHubChange = 0;
				udev->status = 0;
				udev->act_len = sizeof(*hubsts);
				return 0;
			}
			default:
				debug("%s: rx ctl requesttype=%x, request=%x\n",
				      __func__, setup->requesttype,
				      setup->request);
				break;
			}
		case USB_RT_PORT | USB_DIR_IN:
			switch (setup->request) {
			case USB_REQ_GET_STATUS: {
				struct usb_port_status *portsts = buffer;
				int port;

				port = (setup->index & USB_HUB_PORT_MASK) - 1;
				portsts->wPortStatus = priv->status[port];
				portsts->wPortChange = priv->change[port];
				udev->status = 0;
				udev->act_len = sizeof(*portsts);
				return 0;
			}
			}
		default:
			debug("%s: rx ctl requesttype=%x, request=%x\n",
			      __func__, setup->requesttype, setup->request);
			break;
		}
	} else if (pipe == usb_sndctrlpipe(udev, 0)) {
		switch (setup->requesttype) {
		case USB_RT_PORT:
			switch (setup->request) {
			case USB_REQ_SET_FEATURE: {
				int port;

				port = (setup->index & USB_HUB_PORT_MASK) - 1;
				debug("set feature port=%x, feature=%x\n",
				      port, setup->value);
				if (setup->value < USB_PORT_FEAT_C_CONNECTION) {
					ret = clrset_post_state(bus, port, 0,
							1 << setup->value);
				} else {
					debug("  ** Invalid feature\n");
				}
				return ret;
			}
			case USB_REQ_CLEAR_FEATURE: {
				int port;

				port = (setup->index & USB_HUB_PORT_MASK) - 1;
				debug("clear feature port=%x, feature=%x\n",
				      port, setup->value);
				if (setup->value < USB_PORT_FEAT_C_CONNECTION) {
					ret = clrset_post_state(bus, port,
							1 << setup->value, 0);
				} else {
					priv->change[port] &= 1 <<
						(setup->value - 16);
				}
				udev->status = 0;
				return 0;
			}
			default:
				debug("%s: tx ctl requesttype=%x, request=%x\n",
				      __func__, setup->requesttype,
				      setup->request);
				break;
			}
		default:
			debug("%s: tx ctl requesttype=%x, request=%x\n",
			      __func__, setup->requesttype, setup->request);
			break;
		}
	}
	debug("pipe=%lx\n", pipe);

	return -EIO;
}

static int sandbox_hub_bind(struct udevice *dev)
{
	return usb_emul_setup_device(dev, PACKET_SIZE_64, hub_strings,
				     hub_desc_list);
}

static int sandbox_child_post_bind(struct udevice *dev)
{
	struct sandbox_hub_platdata *plat = dev_get_parent_platdata(dev);

	plat->port = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "reg", -1);

	return 0;
}

static const struct dm_usb_ops sandbox_usb_hub_ops = {
	.control	= sandbox_hub_submit_control_msg,
};

static const struct udevice_id sandbox_usb_hub_ids[] = {
	{ .compatible = "sandbox,usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_sandbox_hub) = {
	.name	= "usb_sandbox_hub",
	.id	= UCLASS_USB_EMUL,
	.of_match = sandbox_usb_hub_ids,
	.bind	= sandbox_hub_bind,
	.ops	= &sandbox_usb_hub_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_hub_priv),
	.per_child_platdata_auto_alloc_size =
			sizeof(struct sandbox_hub_platdata),
	.child_post_bind = sandbox_child_post_bind,
};
