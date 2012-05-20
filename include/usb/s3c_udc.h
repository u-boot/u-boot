/*
 * drivers/usb/gadget/s3c_udc.h
 * Samsung S3C on-chip full/high speed USB device controllers
 * Copyright (C) 2005 for Samsung Electronics
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

#ifndef __S3C_USB_GADGET
#define __S3C_USB_GADGET

#include <asm/errno.h>
#include <linux/usb/ch9.h>
#include <usbdescriptors.h>
#include <linux/usb/gadget.h>
#include <linux/list.h>
#include <usb/lin_gadget_compat.h>

#define PHY0_SLEEP              (1 << 5)

/*-------------------------------------------------------------------------*/
/* DMA bounce buffer size, 16K is enough even for mass storage */
#define DMA_BUFFER_SIZE	(4096*4)

#define EP0_FIFO_SIZE		64
#define EP_FIFO_SIZE		512
#define EP_FIFO_SIZE2		1024
/* ep0-control, ep1in-bulk, ep2out-bulk, ep3in-int */
#define S3C_MAX_ENDPOINTS	4
#define S3C_MAX_HW_ENDPOINTS	16

#define WAIT_FOR_SETUP          0
#define DATA_STATE_XMIT         1
#define DATA_STATE_NEED_ZLP     2
#define WAIT_FOR_OUT_STATUS     3
#define DATA_STATE_RECV         4
#define WAIT_FOR_COMPLETE	5
#define WAIT_FOR_OUT_COMPLETE	6
#define WAIT_FOR_IN_COMPLETE	7
#define WAIT_FOR_NULL_COMPLETE	8

#define TEST_J_SEL		0x1
#define TEST_K_SEL		0x2
#define TEST_SE0_NAK_SEL	0x3
#define TEST_PACKET_SEL		0x4
#define TEST_FORCE_ENABLE_SEL	0x5

/* ************************************************************************* */
/* IO
 */

enum ep_type {
	ep_control, ep_bulk_in, ep_bulk_out, ep_interrupt
};

struct s3c_ep {
	struct usb_ep ep;
	struct s3c_udc *dev;

	const struct usb_endpoint_descriptor *desc;
	struct list_head queue;
	unsigned long pio_irqs;
	int len;
	void *dma_buf;

	u8 stopped;
	u8 bEndpointAddress;
	u8 bmAttributes;

	enum ep_type ep_type;
	int fifo_num;
};

struct s3c_request {
	struct usb_request req;
	struct list_head queue;
};

struct s3c_udc {
	struct usb_gadget gadget;
	struct usb_gadget_driver *driver;

	struct s3c_plat_otg_data *pdata;

	void *dma_buf[S3C_MAX_ENDPOINTS+1];
	dma_addr_t dma_addr[S3C_MAX_ENDPOINTS+1];

	int ep0state;
	struct s3c_ep ep[S3C_MAX_ENDPOINTS];

	unsigned char usb_address;

	unsigned req_pending:1, req_std:1;
};

extern struct s3c_udc *the_controller;

#define ep_is_in(EP) (((EP)->bEndpointAddress&USB_DIR_IN) == USB_DIR_IN)
#define ep_index(EP) ((EP)->bEndpointAddress&0xF)
#define ep_maxpacket(EP) ((EP)->ep.maxpacket)

extern void otg_phy_init(struct s3c_udc *dev);
extern void otg_phy_off(struct s3c_udc *dev);

extern void s3c_udc_ep_set_stall(struct s3c_ep *ep);
extern int s3c_udc_probe(struct s3c_plat_otg_data *pdata);

struct s3c_plat_otg_data {
	int		(*phy_control)(int on);
	unsigned int	regs_phy;
	unsigned int	regs_otg;
	unsigned int    usb_phy_ctrl;
	unsigned int    usb_flags;
};
#endif
