/*
 * (C) Copyright 2007-2009 Michal Simek
 * (C) Copyright 2003 Xilinx Inc.
 *
 * Michal SIMEK <monstr@monstr.eu>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <net.h>
#include <config.h>
#include <malloc.h>
#include <asm/io.h>

#undef DEBUG

#define ENET_MAX_MTU		PKTSIZE
#define ENET_MAX_MTU_ALIGNED	PKTSIZE_ALIGN
#define ENET_ADDR_LENGTH	6

/* EmacLite constants */
#define XEL_BUFFER_OFFSET	0x0800	/* Next buffer's offset */
#define XEL_TPLR_OFFSET		0x07F4	/* Tx packet length */
#define XEL_TSR_OFFSET		0x07FC	/* Tx status */
#define XEL_RSR_OFFSET		0x17FC	/* Rx status */
#define XEL_RXBUFF_OFFSET	0x1000	/* Receive Buffer */

/* Xmit complete */
#define XEL_TSR_XMIT_BUSY_MASK		0x00000001UL
/* Xmit interrupt enable bit */
#define XEL_TSR_XMIT_IE_MASK		0x00000008UL
/* Buffer is active, SW bit only */
#define XEL_TSR_XMIT_ACTIVE_MASK	0x80000000UL
/* Program the MAC address */
#define XEL_TSR_PROGRAM_MASK		0x00000002UL
/* define for programming the MAC address into the EMAC Lite */
#define XEL_TSR_PROG_MAC_ADDR	(XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_PROGRAM_MASK)

/* Transmit packet length upper byte */
#define XEL_TPLR_LENGTH_MASK_HI		0x0000FF00UL
/* Transmit packet length lower byte */
#define XEL_TPLR_LENGTH_MASK_LO		0x000000FFUL

/* Recv complete */
#define XEL_RSR_RECV_DONE_MASK		0x00000001UL
/* Recv interrupt enable bit */
#define XEL_RSR_RECV_IE_MASK		0x00000008UL

typedef struct {
	u32 baseaddress;	/* Base address for device (IPIF) */
	u32 nexttxbuffertouse;	/* Next TX buffer to write to */
	u32 nextrxbuffertouse;	/* Next RX buffer to read from */
	uchar deviceid;		/* Unique ID of device - for future */
} xemaclite;

static xemaclite emaclite;

static u32 etherrxbuff[PKTSIZE_ALIGN/4]; /* Receive buffer */

static void xemaclite_alignedread (u32 *srcptr, void *destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;

	/* Word aligned buffer, no correction needed. */
	to32ptr = (u32 *) destptr;
	while (bytecount > 3) {
		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}
	to8ptr = (u8 *) to32ptr;

	alignbuffer = *from32ptr++;
	from8ptr = (u8 *) & alignbuffer;

	for (i = 0; i < bytecount; i++) {
		*to8ptr++ = *from8ptr++;
	}
}

static void xemaclite_alignedwrite (void *srcptr, u32 destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr = (u32 *) destptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;
	while (bytecount > 3) {

		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}

	alignbuffer = 0;
	to8ptr = (u8 *) & alignbuffer;
	from8ptr = (u8 *) from32ptr;

	for (i = 0; i < bytecount; i++) {
		*to8ptr++ = *from8ptr++;
	}

	*to32ptr++ = alignbuffer;
}

static void emaclite_halt(struct eth_device *dev)
{
	debug ("eth_halt\n");
}

static int emaclite_init(struct eth_device *dev, bd_t *bis)
{
	debug ("EmacLite Initialization Started\n");
	memset (&emaclite, 0, sizeof (xemaclite));
	emaclite.baseaddress = dev->iobase;

/*
 * TX - TX_PING & TX_PONG initialization
 */
	/* Restart PING TX */
	out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET, 0);
	/* Copy MAC address */
	xemaclite_alignedwrite (dev->enetaddr,
		emaclite.baseaddress, ENET_ADDR_LENGTH);
	/* Set the length */
	out_be32 (emaclite.baseaddress + XEL_TPLR_OFFSET, ENET_ADDR_LENGTH);
	/* Update the MAC address in the EMAC Lite */
	out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET, XEL_TSR_PROG_MAC_ADDR);
	/* Wait for EMAC Lite to finish with the MAC address update */
	while ((in_be32 (emaclite.baseaddress + XEL_TSR_OFFSET) &
		XEL_TSR_PROG_MAC_ADDR) != 0) ;

#ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	/* The same operation with PONG TX */
	out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET, 0);
	xemaclite_alignedwrite (dev->enetaddr, emaclite.baseaddress +
		XEL_BUFFER_OFFSET, ENET_ADDR_LENGTH);
	out_be32 (emaclite.baseaddress + XEL_TPLR_OFFSET, ENET_ADDR_LENGTH);
	out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET,
		XEL_TSR_PROG_MAC_ADDR);
	while ((in_be32 (emaclite.baseaddress + XEL_TSR_OFFSET +
		XEL_BUFFER_OFFSET) & XEL_TSR_PROG_MAC_ADDR) != 0) ;
#endif

/*
 * RX - RX_PING & RX_PONG initialization
 */
	/* Write out the value to flush the RX buffer */
	out_be32 (emaclite.baseaddress + XEL_RSR_OFFSET, XEL_RSR_RECV_IE_MASK);
#ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	out_be32 (emaclite.baseaddress + XEL_RSR_OFFSET + XEL_BUFFER_OFFSET,
		XEL_RSR_RECV_IE_MASK);
#endif

	debug ("EmacLite Initialization complete\n");
	return 0;
}

static int xemaclite_txbufferavailable (xemaclite *instanceptr)
{
	u32 reg;
	u32 txpingbusy;
	u32 txpongbusy;
	/*
	 * Read the other buffer register
	 * and determine if the other buffer is available
	 */
	reg = in_be32 (instanceptr->baseaddress +
			instanceptr->nexttxbuffertouse + 0);
	txpingbusy = ((reg & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	reg = in_be32 (instanceptr->baseaddress +
			(instanceptr->nexttxbuffertouse ^ XEL_TSR_OFFSET) + 0);
	txpongbusy = ((reg & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	return (!(txpingbusy && txpongbusy));
}

static int emaclite_send (struct eth_device *dev, volatile void *ptr, int len)
{
	u32 reg;
	u32 baseaddress;

	u32 maxtry = 1000;

	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	while (!xemaclite_txbufferavailable (&emaclite) && maxtry) {
		udelay (10);
		maxtry--;
	}

	if (!maxtry) {
		printf ("Error: Timeout waiting for ethernet TX buffer\n");
		/* Restart PING TX */
		out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET, 0);
#ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
		out_be32 (emaclite.baseaddress + XEL_TSR_OFFSET +
		XEL_BUFFER_OFFSET, 0);
#endif
		return -1;
	}

	/* Determine the expected TX buffer address */
	baseaddress = (emaclite.baseaddress + emaclite.nexttxbuffertouse);

	/* Determine if the expected buffer address is empty */
	reg = in_be32 (baseaddress + XEL_TSR_OFFSET);
	if (((reg & XEL_TSR_XMIT_BUSY_MASK) == 0)
		&& ((in_be32 ((baseaddress) + XEL_TSR_OFFSET)
			& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {

#ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
		emaclite.nexttxbuffertouse ^= XEL_BUFFER_OFFSET;
#endif
		debug ("Send packet from 0x%x\n", baseaddress);
		/* Write the frame to the buffer */
		xemaclite_alignedwrite ((void *) ptr, baseaddress, len);
		out_be32 (baseaddress + XEL_TPLR_OFFSET,(len &
			(XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO)));
		reg = in_be32 (baseaddress + XEL_TSR_OFFSET);
		reg |= XEL_TSR_XMIT_BUSY_MASK;
		if ((reg & XEL_TSR_XMIT_IE_MASK) != 0) {
			reg |= XEL_TSR_XMIT_ACTIVE_MASK;
		}
		out_be32 (baseaddress + XEL_TSR_OFFSET, reg);
		return 0;
	}
#ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	/* Switch to second buffer */
	baseaddress ^= XEL_BUFFER_OFFSET;
	/* Determine if the expected buffer address is empty */
	reg = in_be32 (baseaddress + XEL_TSR_OFFSET);
	if (((reg & XEL_TSR_XMIT_BUSY_MASK) == 0)
		&& ((in_be32 ((baseaddress) + XEL_TSR_OFFSET)
			& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {
		debug ("Send packet from 0x%x\n", baseaddress);
		/* Write the frame to the buffer */
		xemaclite_alignedwrite ((void *) ptr, baseaddress, len);
		out_be32 (baseaddress + XEL_TPLR_OFFSET,(len &
			(XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO)));
		reg = in_be32 (baseaddress + XEL_TSR_OFFSET);
		reg |= XEL_TSR_XMIT_BUSY_MASK;
		if ((reg & XEL_TSR_XMIT_IE_MASK) != 0) {
			reg |= XEL_TSR_XMIT_ACTIVE_MASK;
		}
		out_be32 (baseaddress + XEL_TSR_OFFSET, reg);
		return 0;
	}
#endif
	puts ("Error while sending frame\n");
	return -1;
}

static int emaclite_recv(struct eth_device *dev)
{
	u32 length;
	u32 reg;
	u32 baseaddress;

	baseaddress = emaclite.baseaddress + emaclite.nextrxbuffertouse;
	reg = in_be32 (baseaddress + XEL_RSR_OFFSET);
	debug ("Testing data at address 0x%x\n", baseaddress);
	if ((reg & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
#ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
		emaclite.nextrxbuffertouse ^= XEL_BUFFER_OFFSET;
#endif
	} else {
#ifndef CONFIG_XILINX_EMACLITE_RX_PING_PONG
		debug ("No data was available - address 0x%x\n", baseaddress);
		return 0;
#else
		baseaddress ^= XEL_BUFFER_OFFSET;
		reg = in_be32 (baseaddress + XEL_RSR_OFFSET);
		if ((reg & XEL_RSR_RECV_DONE_MASK) !=
					XEL_RSR_RECV_DONE_MASK) {
			debug ("No data was available - address 0x%x\n",
					baseaddress);
			return 0;
		}
#endif
	}
	/* Get the length of the frame that arrived */
	switch(((ntohl(in_be32 (baseaddress + XEL_RXBUFF_OFFSET + 0xC))) &
			0xFFFF0000 ) >> 16) {
		case 0x806:
			length = 42 + 20; /* FIXME size of ARP */
			debug ("ARP Packet\n");
			break;
		case 0x800:
			length = 14 + 14 +
			(((ntohl(in_be32 (baseaddress + XEL_RXBUFF_OFFSET + 0x10))) &
			0xFFFF0000) >> 16); /* FIXME size of IP packet */
			debug ("IP Packet\n");
			break;
		default:
			debug ("Other Packet\n");
			length = ENET_MAX_MTU;
			break;
	}

	xemaclite_alignedread ((u32 *) (baseaddress + XEL_RXBUFF_OFFSET),
			etherrxbuff, length);

	/* Acknowledge the frame */
	reg = in_be32 (baseaddress + XEL_RSR_OFFSET);
	reg &= ~XEL_RSR_RECV_DONE_MASK;
	out_be32 (baseaddress + XEL_RSR_OFFSET, reg);

	debug ("Packet receive from 0x%x, length %dB\n", baseaddress, length);
	NetReceive ((uchar *) etherrxbuff, length);
	return length;

}

int xilinx_emaclite_initialize (bd_t *bis, int base_addr)
{
	struct eth_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "Xilinx_Emaclite");

	dev->iobase = base_addr;
	dev->priv = 0;
	dev->init = emaclite_init;
	dev->halt = emaclite_halt;
	dev->send = emaclite_send;
	dev->recv = emaclite_recv;

	eth_register(dev);

	return 1;
}
