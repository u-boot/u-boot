/*
 * PXA27x register declarations and HCD data structures
 *
 * Copyright (C) 2007 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2007 Eurotech S.p.A. <info@eurotech.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __PXA270X_UDC_H__
#define __PXA270X_UDC_H__

#include <asm/byteorder.h>

/* Endpoint 0 states */
#define EP0_IDLE		0
#define EP0_IN_DATA		1
#define EP0_OUT_DATA		2
#define EP0_XFER_COMPLETE	3


/* Endpoint parameters */
#define MAX_ENDPOINTS		4
#define EP_MAX_PACKET_SIZE	64

#define EP0_MAX_PACKET_SIZE     16
#define UDC_OUT_ENDPOINT        0x02
#define UDC_OUT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_IN_ENDPOINT         0x01
#define UDC_IN_PACKET_SIZE      EP_MAX_PACKET_SIZE
#define UDC_INT_ENDPOINT        0x05
#define UDC_INT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_BULK_PACKET_SIZE    EP_MAX_PACKET_SIZE

void udc_irq(void);
/* Flow control */
void udc_set_nak(int epid);
void udc_unset_nak(int epid);

/* Higher level functions for abstracting away from specific device */
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);

int  udc_init(void);

void udc_enable(struct usb_device_instance *device);
void udc_disable(void);

void udc_connect(void);
void udc_disconnect(void);

void udc_startup_events(struct usb_device_instance *device);
void udc_setup_ep(struct usb_device_instance *device,
	 unsigned int ep, struct usb_endpoint_instance *endpoint);

#endif
