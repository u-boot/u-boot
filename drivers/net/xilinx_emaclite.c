/*
 * (C) Copyright 2007 Michal Simek
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <asm/io.h>

#ifdef XILINX_EMACLITE_BASEADDR

//#define DEBUG

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
	unsigned int BaseAddress;	/* Base address for device (IPIF) */
	unsigned int NextTxBufferToUse;	/* Next TX buffer to write to */
	unsigned int NextRxBufferToUse;	/* Next RX buffer to read from */
	unsigned char DeviceId;		/* Unique ID of device - for future */
} XEmacLite;

static XEmacLite EmacLite;

static char etherrxbuff[PKTSIZE_ALIGN]; /* Receive buffer */

/* hardcoded MAC address for the Xilinx EMAC Core when env is nowhere*/
#ifdef CFG_ENV_IS_NOWHERE
static u8 EMACAddr[ENET_ADDR_LENGTH] = { 0x00, 0x0a, 0x35, 0x00, 0x22, 0x01 };
#endif

void XEmacLite_AlignedRead (u32 * SrcPtr, void *DestPtr, unsigned ByteCount)
{
	unsigned i;
	unsigned Length = ByteCount;
	u32 AlignBuffer;
	u32 *To32Ptr;
	u32 *From32Ptr;
	u8 *To8Ptr;
	u8 *From8Ptr;

	From32Ptr = (u32 *) SrcPtr;

	/* Word aligned buffer, no correction needed. */
	To32Ptr = (u32 *) DestPtr;
	while (Length > 3) {
		*To32Ptr++ = *From32Ptr++;
		Length -= 4;
	}
	To8Ptr = (u8 *) To32Ptr;

	AlignBuffer = *From32Ptr++;
	From8Ptr = (u8 *) & AlignBuffer;

	for (i = 0; i < Length; i++) {
		*To8Ptr++ = *From8Ptr++;
	}
}

void XEmacLite_AlignedWrite (void *SrcPtr, u32 * DestPtr, unsigned ByteCount)
{
	unsigned i;
	unsigned Length = ByteCount;
	u32 AlignBuffer;
	u32 *To32Ptr;
	u32 *From32Ptr;
	u8 *To8Ptr;
	u8 *From8Ptr;
	To32Ptr = DestPtr;

	From32Ptr = (u32 *) SrcPtr;
	while (Length > 3) {

		*To32Ptr++ = *From32Ptr++;
		Length -= 4;
	}

	AlignBuffer = 0;
	To8Ptr = (u8 *) & AlignBuffer;
	From8Ptr = (u8 *) From32Ptr;

	for (i = 0; i < Length; i++) {
		*To8Ptr++ = *From8Ptr++;
	}

	*To32Ptr++ = AlignBuffer;
}

void eth_halt (void)
{
#ifdef DEBUG
	puts ("eth_halt\n");
#endif
}

int eth_init (bd_t * bis)
{
#ifdef DEBUG
	puts ("EmacLite Initialization Started\n");
#endif
	memset (&EmacLite, 0, sizeof (XEmacLite));
	EmacLite.BaseAddress = XILINX_EMACLITE_BASEADDR;

#ifdef CFG_ENV_IS_NOWHERE
	memcpy (bis->bi_enetaddr, EMACAddr, ENET_ADDR_LENGTH);
#endif
/*
 * TX - TX_PING & TX_PONG initialization
 */
	/* Restart PING TX */
	out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET, 0);
	/* Copy MAC address */
	XEmacLite_AlignedWrite (bis->bi_enetaddr,
		EmacLite.BaseAddress, ENET_ADDR_LENGTH);
	/* Set the length */
	out_be32 (EmacLite.BaseAddress + XEL_TPLR_OFFSET, ENET_ADDR_LENGTH);
	/* Update the MAC address in the EMAC Lite */
	out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET, XEL_TSR_PROG_MAC_ADDR);
	/* Wait for EMAC Lite to finish with the MAC address update */
	while ((in_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET) &
		XEL_TSR_PROG_MAC_ADDR) != 0) ;

#ifdef XILINX_EMACLITE_TX_PING_PONG
	/* The same operation with PONG TX */
	out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET, 0);	
	XEmacLite_AlignedWrite (bis->bi_enetaddr,
		EmacLite.BaseAddress + XEL_BUFFER_OFFSET, ENET_ADDR_LENGTH);
	out_be32 (EmacLite.BaseAddress + XEL_TPLR_OFFSET, ENET_ADDR_LENGTH);
	out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET,
		XEL_TSR_PROG_MAC_ADDR);
	while ((in_be32	(EmacLite.BaseAddress + XEL_TSR_OFFSET +
		 XEL_BUFFER_OFFSET) & XEL_TSR_PROG_MAC_ADDR) != 0) ;
#endif

/*
 * RX - RX_PING & RX_PONG initialization
 */
	/* Write out the value to flush the RX buffer */
	out_be32 (EmacLite.BaseAddress + XEL_RSR_OFFSET, XEL_RSR_RECV_IE_MASK);
#ifdef XILINX_EMACLITE_RX_PING_PONG
	out_be32 (EmacLite.BaseAddress + XEL_RSR_OFFSET + XEL_BUFFER_OFFSET,
		XEL_RSR_RECV_IE_MASK);
#endif

#ifdef DEBUG
	puts ("EmacLite Initialization complete\n");
#endif
	return 0;
}

int XEmacLite_TxBufferAvailable (XEmacLite * InstancePtr)
{
	u32 Register;
	u32 TxPingBusy;
	u32 TxPongBusy;
	/*
	 * Read the other buffer register
	 * and determine if the other buffer is available
	 */
	Register = in_be32 (InstancePtr->BaseAddress +
			InstancePtr->NextTxBufferToUse + 0);
	TxPingBusy = ((Register & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	Register = in_be32 (InstancePtr->BaseAddress +
			(InstancePtr->NextTxBufferToUse ^ XEL_TSR_OFFSET) + 0);
	TxPongBusy = ((Register & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	return (!(TxPingBusy && TxPongBusy));
}

int eth_send (volatile void *ptr, int len) {

	unsigned int Register;
	unsigned int BaseAddress;

	unsigned maxtry = 1000;

	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	while (!XEmacLite_TxBufferAvailable (&EmacLite) && maxtry) {
		udelay (10);
		maxtry--;
	}

	if (!maxtry) {
		printf ("Error: Timeout waiting for ethernet TX buffer\n");
		/* Restart PING TX */
		out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET, 0);
#ifdef XILINX_EMACLITE_TX_PING_PONG
		out_be32 (EmacLite.BaseAddress + XEL_TSR_OFFSET +
		XEL_BUFFER_OFFSET, 0);
#endif
		return 0;
	}

	/* Determine the expected TX buffer address */
	BaseAddress = (EmacLite.BaseAddress + EmacLite.NextTxBufferToUse);

	/* Determine if the expected buffer address is empty */
	Register = in_be32 (BaseAddress + XEL_TSR_OFFSET);
	if (((Register & XEL_TSR_XMIT_BUSY_MASK) == 0)
		&& ((in_be32 ((BaseAddress) + XEL_TSR_OFFSET)
			& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {

#ifdef XILINX_EMACLITE_TX_PING_PONG
		EmacLite.NextTxBufferToUse ^= XEL_BUFFER_OFFSET;
#endif
#ifdef DEBUG
		printf ("Send packet from 0x%x\n", BaseAddress);
#endif
		/* Write the frame to the buffer */
		XEmacLite_AlignedWrite (ptr, (u32 *) BaseAddress, len);
		out_be32 (BaseAddress + XEL_TPLR_OFFSET,(len &
			(XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO)));
		Register = in_be32 (BaseAddress + XEL_TSR_OFFSET);
		Register |= XEL_TSR_XMIT_BUSY_MASK;
		if ((Register & XEL_TSR_XMIT_IE_MASK) != 0) {
			Register |= XEL_TSR_XMIT_ACTIVE_MASK;
		}
		out_be32 (BaseAddress + XEL_TSR_OFFSET, Register);
		return 1;
	}
#ifdef XILINX_EMACLITE_TX_PING_PONG
	/* Switch to second buffer */
	BaseAddress ^= XEL_BUFFER_OFFSET;
	/* Determine if the expected buffer address is empty */
	Register = in_be32 (BaseAddress + XEL_TSR_OFFSET);
	if (((Register & XEL_TSR_XMIT_BUSY_MASK) == 0)
		&& ((in_be32 ((BaseAddress) + XEL_TSR_OFFSET)
			& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {
#ifdef DEBUG
		printf ("Send packet from 0x%x\n", BaseAddress);
#endif
		/* Write the frame to the buffer */
		XEmacLite_AlignedWrite (ptr, (u32 *) BaseAddress, len);
		out_be32 (BaseAddress + XEL_TPLR_OFFSET,(len &
			(XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO)));
		Register = in_be32 (BaseAddress + XEL_TSR_OFFSET);
		Register |= XEL_TSR_XMIT_BUSY_MASK;
		if ((Register & XEL_TSR_XMIT_IE_MASK) != 0) {
			Register |= XEL_TSR_XMIT_ACTIVE_MASK;
		}
		out_be32 (BaseAddress + XEL_TSR_OFFSET, Register);
		return 1;
	}
#endif
	puts ("Error while sending frame\n");
	return 0;
}

int eth_rx (void)
{
	unsigned int Length;
	unsigned int Register;
	unsigned int BaseAddress;

	BaseAddress = EmacLite.BaseAddress + EmacLite.NextRxBufferToUse;
	Register = in_be32 (BaseAddress + XEL_RSR_OFFSET);
#ifdef DEBUG
//	printf ("Testing data at address 0x%x\n", BaseAddress);
#endif
	if ((Register & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
#ifdef XILINX_EMACLITE_RX_PING_PONG
		EmacLite.NextRxBufferToUse ^= XEL_BUFFER_OFFSET;
#endif
	} else {
#ifndef XILINX_EMACLITE_RX_PING_PONG
#ifdef DEBUG
//		printf ("No data was available - address 0x%x\n", BaseAddress);
#endif
		return 0;
#else
		BaseAddress ^= XEL_BUFFER_OFFSET;
		Register = in_be32 (BaseAddress + XEL_RSR_OFFSET);
		if ((Register & XEL_RSR_RECV_DONE_MASK) !=
					XEL_RSR_RECV_DONE_MASK) {
#ifdef DEBUG
//			printf ("No data was available - address 0x%x\n",
//					BaseAddress);
#endif
			return 0;
		}
#endif
	}
	/* Get the length of the frame that arrived */
	switch(((in_be32(BaseAddress + XEL_RXBUFF_OFFSET + 0xC)) &
			0xFFFF0000 ) >> 16) {
		case 0x806:
			Length = 42 + 20; /* FIXME size of ARP */
#ifdef DEBUG
			puts ("ARP Packet\n");
#endif
			break;
		case 0x800:
			Length = 14 + 14 +
			(((in_be32(BaseAddress + XEL_RXBUFF_OFFSET + 0x10)) &
			0xFFFF0000) >> 16); /* FIXME size of IP packet */
#ifdef DEBUG
			puts("IP Packet\n");
#endif
			break;
		default:
#ifdef DEBUG
			puts("Other Packet\n");
#endif
			Length = ENET_MAX_MTU;
			break;
	}

	XEmacLite_AlignedRead ((BaseAddress + XEL_RXBUFF_OFFSET),
			etherrxbuff, Length);

	/* Acknowledge the frame */
	Register = in_be32 (BaseAddress + XEL_RSR_OFFSET);
	Register &= ~XEL_RSR_RECV_DONE_MASK;
	out_be32 (BaseAddress + XEL_RSR_OFFSET, Register);

#ifdef DEBUG
	printf ("Packet receive from 0x%x, length %dB\n", BaseAddress, Length);
#endif
	NetReceive ((uchar *) etherrxbuff, Length);
	return 1;

}
#endif
