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
 *
 * Based on Xilinx drivers
 *
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <asm/io.h>
#include "xilinx_emac.h"

#ifdef XILINX_EMAC

#undef DEBUG

#define ENET_MAX_MTU		PKTSIZE
#define ENET_ADDR_LENGTH	6

static unsigned int etherrxbuff[PKTSIZE_ALIGN/4]; /* Receive buffer */

static u8 emacaddr[ENET_ADDR_LENGTH] = { 0x00, 0x0a, 0x35, 0x00, 0x22, 0x01 };

static xemac emac;

void eth_halt(void)
{
#ifdef DEBUG
	puts ("eth_halt\n");
#endif
}

int eth_init(bd_t * bis)
{
	u32 helpreg;
#ifdef DEBUG
	printf("EMAC Initialization Started\n\r");
#endif
	if (emac.isstarted) {
		puts("Emac is started\n");
		return 0;
	}

	memset (&emac, 0, sizeof (xemac));

	emac.baseaddress = XILINX_EMAC_BASEADDR;

	/* Setting up FIFOs */
	emac.recvfifo.regbaseaddress = emac.baseaddress +
					XEM_PFIFO_RXREG_OFFSET;
	emac.recvfifo.databaseaddress = emac.baseaddress +
					XEM_PFIFO_RXDATA_OFFSET;
	out_be32 (emac.recvfifo.regbaseaddress, XPF_RESET_FIFO_MASK);

	emac.sendfifo.regbaseaddress = emac.baseaddress +
					XEM_PFIFO_TXREG_OFFSET;
	emac.sendfifo.databaseaddress = emac.baseaddress +
					XEM_PFIFO_TXDATA_OFFSET;
	out_be32 (emac.sendfifo.regbaseaddress, XPF_RESET_FIFO_MASK);

	/* Reset the entire IPIF */
	out_be32 (emac.baseaddress + XIIF_V123B_RESETR_OFFSET,
					XIIF_V123B_RESET_MASK);

	/* Stopping EMAC for setting up MAC */
	helpreg = in_be32 (emac.baseaddress + XEM_ECR_OFFSET);
	helpreg &= ~(XEM_ECR_XMIT_ENABLE_MASK | XEM_ECR_RECV_ENABLE_MASK);
	out_be32 (emac.baseaddress + XEM_ECR_OFFSET, helpreg);

	if (!getenv("ethaddr")) {
		memcpy(bis->bi_enetaddr, emacaddr, ENET_ADDR_LENGTH);
	}

	/* Set the device station address high and low registers */
	helpreg = (bis->bi_enetaddr[0] << 8) | bis->bi_enetaddr[1];
	out_be32 (emac.baseaddress + XEM_SAH_OFFSET, helpreg);
	helpreg = (bis->bi_enetaddr[2] << 24) | (bis->bi_enetaddr[3] << 16) |
			(bis->bi_enetaddr[4] << 8) | bis->bi_enetaddr[5];
	out_be32 (emac.baseaddress + XEM_SAL_OFFSET, helpreg);


	helpreg = XEM_ECR_UNICAST_ENABLE_MASK | XEM_ECR_BROAD_ENABLE_MASK |
		XEM_ECR_FULL_DUPLEX_MASK | XEM_ECR_XMIT_FCS_ENABLE_MASK |
		XEM_ECR_XMIT_PAD_ENABLE_MASK | XEM_ECR_PHY_ENABLE_MASK;
	out_be32 (emac.baseaddress + XEM_ECR_OFFSET, helpreg);

	emac.isstarted = 1;

	/* Enable the transmitter, and receiver */
	helpreg = in_be32 (emac.baseaddress + XEM_ECR_OFFSET);
	helpreg &= ~(XEM_ECR_XMIT_RESET_MASK | XEM_ECR_RECV_RESET_MASK);
	helpreg |= (XEM_ECR_XMIT_ENABLE_MASK | XEM_ECR_RECV_ENABLE_MASK);
	out_be32 (emac.baseaddress + XEM_ECR_OFFSET, helpreg);

	printf("EMAC Initialization complete\n\r");
	return 0;
}

int eth_send(volatile void *ptr, int len)
{
	u32 intrstatus;
	u32 xmitstatus;
	u32 fifocount;
	u32 wordcount;
	u32 extrabytecount;
	u32 *wordbuffer = (u32 *) ptr;

	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	/*
	 * Check for overruns and underruns for the transmit status and length
	 * FIFOs and make sure the send packet FIFO is not deadlocked.
	 * Any of these conditions is bad enough that we do not want to
	 * continue. The upper layer software should reset the device to resolve
	 * the error.
	 */
	intrstatus = in_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET);
	if (intrstatus & (XEM_EIR_XMIT_SFIFO_OVER_MASK |
			XEM_EIR_XMIT_LFIFO_OVER_MASK)) {
#ifdef DEBUG
		puts ("Transmitting overrun error\n");
#endif
		return 0;
	} else if (intrstatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
			XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
#ifdef DEBUG
		puts ("Transmitting underrun error\n");
#endif
		return 0;
	} else if (in_be32 (emac.sendfifo.regbaseaddress +
			XPF_COUNT_STATUS_REG_OFFSET) & XPF_DEADLOCK_MASK) {
#ifdef DEBUG
		puts("Transmitting fifo error\n");
#endif
		return 0;
	}

	/*
	 * Before writing to the data FIFO, make sure the length FIFO is not
	 * full. The data FIFO might not be full yet even though the length FIFO
	 * is. This avoids an overrun condition on the length FIFO and keeps the
	 * FIFOs in sync.
	 *
	 * Clear the latched LFIFO_FULL bit so next time around the most
	 * current status is represented
	 */
	if (intrstatus & XEM_EIR_XMIT_LFIFO_FULL_MASK) {
		out_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET,
			intrstatus & XEM_EIR_XMIT_LFIFO_FULL_MASK);
#ifdef DEBUG
		puts ("Fifo is full\n");
#endif
		return 0;
	}

	/* get the count of how many words may be inserted into the FIFO */
	fifocount = in_be32 (emac.sendfifo.regbaseaddress +
				XPF_COUNT_STATUS_REG_OFFSET) & XPF_COUNT_MASK;
	wordcount = len >> 2;
	extrabytecount = len & 0x3;

	if (fifocount < wordcount) {
#ifdef DEBUG
		puts ("Sending packet is larger then size of FIFO\n");
#endif
		return 0;
	}

	for (fifocount = 0; fifocount < wordcount; fifocount++) {
		out_be32 (emac.sendfifo.databaseaddress, wordbuffer[fifocount]);
	}
	if (extrabytecount > 0) {
		u32 lastword = 0;
		u8 *extrabytesbuffer = (u8 *) (wordbuffer + wordcount);

		if (extrabytecount == 1) {
			lastword = extrabytesbuffer[0] << 24;
		} else if (extrabytecount == 2) {
			lastword = extrabytesbuffer[0] << 24 |
				extrabytesbuffer[1] << 16;
		} else if (extrabytecount == 3) {
			lastword = extrabytesbuffer[0] << 24 |
				extrabytesbuffer[1] << 16 |
				extrabytesbuffer[2] << 8;
		}
		out_be32 (emac.sendfifo.databaseaddress, lastword);
	}

	/* Loop on the MAC's status to wait for any pause to complete */
	intrstatus = in_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET);
	while ((intrstatus & XEM_EIR_XMIT_PAUSE_MASK) != 0) {
		intrstatus = in_be32 ((emac.baseaddress) +
					XIIF_V123B_IISR_OFFSET);
		/* Clear the pause status from the transmit status register */
		out_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET,
				intrstatus & XEM_EIR_XMIT_PAUSE_MASK);
	}

	/*
	 * Set the MAC's transmit packet length register to tell it to transmit
	 */
	out_be32 (emac.baseaddress + XEM_TPLR_OFFSET, len);

	/*
	 * Loop on the MAC's status to wait for the transmit to complete.
	 * The transmit status is in the FIFO when the XMIT_DONE bit is set.
	 */
	do {
		intrstatus = in_be32 ((emac.baseaddress) +
						XIIF_V123B_IISR_OFFSET);
	}
	while ((intrstatus & XEM_EIR_XMIT_DONE_MASK) == 0);

	xmitstatus = in_be32 (emac.baseaddress + XEM_TSR_OFFSET);

	if (intrstatus & (XEM_EIR_XMIT_SFIFO_OVER_MASK |
					XEM_EIR_XMIT_LFIFO_OVER_MASK)) {
#ifdef DEBUG
		puts ("Transmitting overrun error\n");
#endif
		return 0;
	} else if (intrstatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
					XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
#ifdef DEBUG
		puts ("Transmitting underrun error\n");
#endif
		return 0;
	}

	/* Clear the interrupt status register of transmit statuses */
	out_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET,
				intrstatus & XEM_EIR_XMIT_ALL_MASK);

	/*
	 * Collision errors are stored in the transmit status register
	 * instead of the interrupt status register
	 */
	if ((xmitstatus & XEM_TSR_EXCESS_DEFERRAL_MASK) ||
				(xmitstatus & XEM_TSR_LATE_COLLISION_MASK)) {
#ifdef DEBUG
		puts ("Transmitting collision error\n");
#endif
		return 0;
	}
	return 1;
}

int eth_rx(void)
{
	u32 pktlength;
	u32 intrstatus;
	u32 fifocount;
	u32 wordcount;
	u32 extrabytecount;
	u32 lastword;
	u8 *extrabytesbuffer;

	if (in_be32 (emac.recvfifo.regbaseaddress + XPF_COUNT_STATUS_REG_OFFSET)
			& XPF_DEADLOCK_MASK) {
		out_be32 (emac.recvfifo.regbaseaddress, XPF_RESET_FIFO_MASK);
#ifdef DEBUG
		puts ("Receiving FIFO deadlock\n");
#endif
		return 0;
	}

	/*
	 * Get the interrupt status to know what happened (whether an error
	 * occurred and/or whether frames have been received successfully).
	 * When clearing the intr status register, clear only statuses that
	 * pertain to receive.
	 */
	intrstatus = in_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET);
	/*
	 * Before reading from the length FIFO, make sure the length FIFO is not
	 * empty. We could cause an underrun error if we try to read from an
	 * empty FIFO.
	 */
	if (!(intrstatus & XEM_EIR_RECV_DONE_MASK)) {
#ifdef DEBUG
		/* puts("Receiving FIFO is empty\n"); */
#endif
		return 0;
	}

	/*
	 * Determine, from the MAC, the length of the next packet available
	 * in the data FIFO (there should be a non-zero length here)
	 */
	pktlength = in_be32 (emac.baseaddress + XEM_RPLR_OFFSET);
	if (!pktlength) {
		return 0;
	}

	/*
	 * Write the RECV_DONE bit in the status register to clear it. This bit
	 * indicates the RPLR is non-empty, and we know it's set at this point.
	 * We clear it so that subsequent entry into this routine will reflect
	 * the current status. This is done because the non-empty bit is latched
	 * in the IPIF, which means it may indicate a non-empty condition even
	 * though there is something in the FIFO.
	 */
	out_be32 ((emac.baseaddress) + XIIF_V123B_IISR_OFFSET,
						XEM_EIR_RECV_DONE_MASK);

	fifocount = in_be32 (emac.recvfifo.regbaseaddress +
				XPF_COUNT_STATUS_REG_OFFSET) & XPF_COUNT_MASK;

	if ((fifocount * 4) < pktlength) {
#ifdef DEBUG
		puts ("Receiving FIFO is smaller than packet size.\n");
#endif
		return 0;
	}

	wordcount = pktlength >> 2;
	extrabytecount = pktlength & 0x3;

	for (fifocount = 0; fifocount < wordcount; fifocount++) {
		etherrxbuff[fifocount] =
				in_be32 (emac.recvfifo.databaseaddress);
	}

	/*
	 * if there are extra bytes to handle, read the last word from the FIFO
	 * and insert the extra bytes into the buffer
	 */
	if (extrabytecount > 0) {
		extrabytesbuffer = (u8 *) (etherrxbuff + wordcount);

		lastword = in_be32 (emac.recvfifo.databaseaddress);

		/*
		 * one extra byte in the last word, put the byte into the next
		 * location of the buffer, bytes in a word of the FIFO are
		 * ordered from most significant byte to least
		 */
		if (extrabytecount == 1) {
			extrabytesbuffer[0] = (u8) (lastword >> 24);
		} else if (extrabytecount == 2) {
			extrabytesbuffer[0] = (u8) (lastword >> 24);
			extrabytesbuffer[1] = (u8) (lastword >> 16);
		} else if (extrabytecount == 3) {
			extrabytesbuffer[0] = (u8) (lastword >> 24);
			extrabytesbuffer[1] = (u8) (lastword >> 16);
			extrabytesbuffer[2] = (u8) (lastword >> 8);
		}
	}
	NetReceive((uchar *)etherrxbuff, pktlength);
	return 1;
}
#endif
