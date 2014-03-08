/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

	/* driver private */
	void *dev_priv;
};

/*
 * Function definitions for each USB ethernet driver go here
 * (declaration is unconditional, compilation is conditional)
 */
void asix_eth_before_probe(void);
int asix_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int asix_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
		      struct eth_device *eth);

void mcs7830_eth_before_probe(void);
int mcs7830_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int mcs7830_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			 struct eth_device *eth);

void smsc95xx_eth_before_probe(void);
int smsc95xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
			struct ueth_data *ss);
int smsc95xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			struct eth_device *eth);

#endif /* __USB_ETHER_H__ */
