/******************************************************************************
 *
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 * AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 * SOLUTIONS FOR XILINX DEVICES. BY PROVIDING THIS DESIGN, CODE,
 * OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 * APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 * THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 * AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 * FOR YOUR IMPLEMENTATION. XILINX EXPRESSLY DISCLAIMS ANY
 * WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 * IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 * REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 * INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * (C) Copyright 2007-2008 Michal Simek
 * Michal SIMEK <monstr@monstr.eu>
 *
 * (c) Copyright 2003 Xilinx Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <config.h>
#include <common.h>
#include <net.h>
#include <asm/io.h>

#include <asm/asm.h>

#undef DEBUG

typedef struct {
	u32 regbaseaddress;	/* Base address of registers */
	u32 databaseaddress;	/* Base address of data for FIFOs */
} xpacketfifov100b;

typedef struct {
	u32 baseaddress;	/* Base address (of IPIF) */
	u32 isstarted;		/* Device is currently started 0-no, 1-yes */
	xpacketfifov100b recvfifo;	/* FIFO used to receive frames */
	xpacketfifov100b sendfifo;	/* FIFO used to send frames */
} xemac;

#define XIIF_V123B_IISR_OFFSET	32UL /* IP interrupt status register */
#define XIIF_V123B_RESET_MASK		0xAUL
#define XIIF_V123B_RESETR_OFFSET	64UL /* reset register */

/* This constant is used with the Reset Register */
#define XPF_RESET_FIFO_MASK		0x0000000A
#define XPF_COUNT_STATUS_REG_OFFSET	4UL

/* These constants are used with the Occupancy/Vacancy Count Register. This
 * register also contains FIFO status */
#define XPF_COUNT_MASK			0x0000FFFF
#define XPF_DEADLOCK_MASK		0x20000000

/* Offset of the MAC registers from the IPIF base address */
#define XEM_REG_OFFSET		0x1100UL

/*
 * Register offsets for the Ethernet MAC. Each register is 32 bits.
 */
#define XEM_ECR_OFFSET	(XEM_REG_OFFSET + 0x4)	/* MAC Control */
#define XEM_SAH_OFFSET	(XEM_REG_OFFSET + 0xC)	/* Station addr, high */
#define XEM_SAL_OFFSET	(XEM_REG_OFFSET + 0x10)	/* Station addr, low */
#define XEM_RPLR_OFFSET	(XEM_REG_OFFSET + 0x1C)	/* Rx packet length */
#define XEM_TPLR_OFFSET	(XEM_REG_OFFSET + 0x20)	/* Tx packet length */
#define XEM_TSR_OFFSET	(XEM_REG_OFFSET + 0x24)	/* Tx status */

#define XEM_PFIFO_OFFSET	0x2000UL
/* Tx registers */
#define XEM_PFIFO_TXREG_OFFSET	(XEM_PFIFO_OFFSET + 0x0)
/* Rx registers */
#define XEM_PFIFO_RXREG_OFFSET	(XEM_PFIFO_OFFSET + 0x10)
/* Tx keyhole */
#define XEM_PFIFO_TXDATA_OFFSET	(XEM_PFIFO_OFFSET + 0x100)
/* Rx keyhole */
#define XEM_PFIFO_RXDATA_OFFSET	(XEM_PFIFO_OFFSET + 0x200)

/*
 * EMAC Interrupt Registers (Status and Enable) masks. These registers are
 * part of the IPIF IP Interrupt registers
 */
/* A mask for all transmit interrupts, used in polled mode */
#define XEM_EIR_XMIT_ALL_MASK	(XEM_EIR_XMIT_DONE_MASK |\
				XEM_EIR_XMIT_ERROR_MASK | \
				XEM_EIR_XMIT_SFIFO_EMPTY_MASK |\
				XEM_EIR_XMIT_LFIFO_FULL_MASK)

/* Xmit complete */
#define XEM_EIR_XMIT_DONE_MASK		0x00000001UL
/* Recv complete */
#define XEM_EIR_RECV_DONE_MASK		0x00000002UL
/* Xmit error */
#define XEM_EIR_XMIT_ERROR_MASK		0x00000004UL
/* Recv error */
#define XEM_EIR_RECV_ERROR_MASK		0x00000008UL
/* Xmit status fifo empty */
#define XEM_EIR_XMIT_SFIFO_EMPTY_MASK	0x00000010UL
/* Recv length fifo empty */
#define XEM_EIR_RECV_LFIFO_EMPTY_MASK	0x00000020UL
/* Xmit length fifo full */
#define XEM_EIR_XMIT_LFIFO_FULL_MASK	0x00000040UL
/* Recv length fifo overrun */
#define XEM_EIR_RECV_LFIFO_OVER_MASK	0x00000080UL
/* Recv length fifo underrun */
#define XEM_EIR_RECV_LFIFO_UNDER_MASK	0x00000100UL
/* Xmit status fifo overrun */
#define XEM_EIR_XMIT_SFIFO_OVER_MASK	0x00000200UL
/* Transmit status fifo underrun */
#define XEM_EIR_XMIT_SFIFO_UNDER_MASK	0x00000400UL
/* Transmit length fifo overrun */
#define XEM_EIR_XMIT_LFIFO_OVER_MASK	0x00000800UL
/* Transmit length fifo underrun */
#define XEM_EIR_XMIT_LFIFO_UNDER_MASK	0x00001000UL
/* Transmit pause pkt received */
#define XEM_EIR_XMIT_PAUSE_MASK		0x00002000UL

/*
 * EMAC Control Register (ECR)
 */
/* Full duplex mode */
#define XEM_ECR_FULL_DUPLEX_MASK	0x80000000UL
/* Reset transmitter */
#define XEM_ECR_XMIT_RESET_MASK		0x40000000UL
/* Enable transmitter */
#define XEM_ECR_XMIT_ENABLE_MASK	0x20000000UL
/* Reset receiver */
#define XEM_ECR_RECV_RESET_MASK		0x10000000UL
/* Enable receiver */
#define XEM_ECR_RECV_ENABLE_MASK	0x08000000UL
/* Enable PHY */
#define XEM_ECR_PHY_ENABLE_MASK		0x04000000UL
/* Enable xmit pad insert */
#define XEM_ECR_XMIT_PAD_ENABLE_MASK	0x02000000UL
/* Enable xmit FCS insert */
#define XEM_ECR_XMIT_FCS_ENABLE_MASK	0x01000000UL
/* Enable unicast addr */
#define XEM_ECR_UNICAST_ENABLE_MASK	0x00020000UL
/* Enable broadcast addr */
#define XEM_ECR_BROAD_ENABLE_MASK	0x00008000UL

/*
 * Transmit Status Register (TSR)
 */
/* Transmit excess deferral */
#define XEM_TSR_EXCESS_DEFERRAL_MASK	0x80000000UL
/* Transmit late collision */
#define XEM_TSR_LATE_COLLISION_MASK	0x01000000UL

#define ENET_MAX_MTU		PKTSIZE
#define ENET_ADDR_LENGTH	6

static unsigned int etherrxbuff[PKTSIZE_ALIGN/4]; /* Receive buffer */

static u8 emacaddr[ENET_ADDR_LENGTH] = { 0x00, 0x0a, 0x35, 0x00, 0x22, 0x01 };

static xemac emac;

void eth_halt(void)
{
	debug ("eth_halt\n");
}

int eth_init(bd_t * bis)
{
	u32 helpreg;
	debug ("EMAC Initialization Started\n\r");

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
		debug ("Transmitting overrun error\n");
		return 0;
	} else if (intrstatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
			XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
		debug ("Transmitting underrun error\n");
		return 0;
	} else if (in_be32 (emac.sendfifo.regbaseaddress +
			XPF_COUNT_STATUS_REG_OFFSET) & XPF_DEADLOCK_MASK) {
		debug ("Transmitting fifo error\n");
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
		debug ("Fifo is full\n");
		return 0;
	}

	/* get the count of how many words may be inserted into the FIFO */
	fifocount = in_be32 (emac.sendfifo.regbaseaddress +
				XPF_COUNT_STATUS_REG_OFFSET) & XPF_COUNT_MASK;
	wordcount = len >> 2;
	extrabytecount = len & 0x3;

	if (fifocount < wordcount) {
		debug ("Sending packet is larger then size of FIFO\n");
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
		debug ("Transmitting overrun error\n");
		return 0;
	} else if (intrstatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
					XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
		debug ("Transmitting underrun error\n");
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
		debug ("Transmitting collision error\n");
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
		debug ("Receiving FIFO deadlock\n");
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
		/* debug ("Receiving FIFO is empty\n"); */
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
		debug ("Receiving FIFO is smaller than packet size.\n");
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
