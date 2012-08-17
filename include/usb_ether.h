/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __USB_ETHER_H__
#define __USB_ETHER_H__

#include <net.h>

/*
 *	IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *	and FCS/CRC (frame check sequence).
 */
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	PKTSIZE_ALIGN	/* Max. octets in frame sans FCS */
#define ETH_FCS_LEN	4		/* Octets in the FCS		 */

struct ueth_data {
	/* eth info */
	struct eth_device eth_dev;		/* used with eth_register */
	int phy_id;						/* mii phy id */

	/* usb info */
	struct usb_device *pusb_dev;	/* this usb_device */
	unsigned char	ifnum;			/* interface number */
	unsigned char	ep_in;			/* in endpoint */
	unsigned char	ep_out;			/* out ....... */
	unsigned char	ep_int;			/* interrupt . */
	unsigned char	subclass;		/* as in overview */
	unsigned char	protocol;		/* .............. */
	unsigned char	irqinterval;	/* Intervall for IRQ Pipe */

	/* private fields for each driver can go here if needed */
#ifdef CONFIG_USB_ETHER_SMSC95XX
	size_t rx_urb_size;  /* maximum USB URB size */
	u32 mac_cr;  /* MAC control register value */
	int have_hwaddr;  /* 1 if we have a hardware MAC address */
#endif
};

/*
 * Function definitions for each USB ethernet driver go here, bracketed by
 * #ifdef CONFIG_USB_ETHER_xxx...#endif
 */
#ifdef CONFIG_USB_ETHER_ASIX
void asix_eth_before_probe(void);
int asix_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int asix_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
		      struct eth_device *eth);
#endif

#ifdef CONFIG_USB_ETHER_SMSC95XX
void smsc95xx_eth_before_probe(void);
int smsc95xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
			struct ueth_data *ss);
int smsc95xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			struct eth_device *eth);
#endif

#endif /* __USB_ETHER_H__ */
