/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __MUSB_UDC_H__
#define __MUSB_UDC_H__

#include <usbdevice.h>

/* UDC level routines */
void udc_irq(void);
void udc_set_nak(int ep_num);
void udc_unset_nak(int ep_num);
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);
void udc_setup_ep(struct usb_device_instance *device, unsigned int id,
		  struct usb_endpoint_instance *endpoint);
void udc_connect(void);
void udc_disconnect(void);
void udc_enable(struct usb_device_instance *device);
void udc_disable(void);
void udc_startup_events(struct usb_device_instance *device);
int udc_init(void);

/* usbtty */
#ifdef CONFIG_USB_TTY

#define EP0_MAX_PACKET_SIZE	64 /* MUSB_EP0_FIFOSIZE */
#define UDC_INT_ENDPOINT	1
#define UDC_INT_PACKET_SIZE	64
#define UDC_OUT_ENDPOINT	2
#define UDC_OUT_PACKET_SIZE	64
#define UDC_IN_ENDPOINT		3
#define UDC_IN_PACKET_SIZE	64
#define UDC_BULK_PACKET_SIZE	64

#endif /* CONFIG_USB_TTY */

#endif /* __MUSB_UDC_H__ */
