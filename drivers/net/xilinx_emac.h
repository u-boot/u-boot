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
#define XEM_ECR_FULL_DUPLEX_MASK	 0x80000000UL
/* Reset transmitter */
#define XEM_ECR_XMIT_RESET_MASK		 0x40000000UL
/* Enable transmitter */
#define XEM_ECR_XMIT_ENABLE_MASK	 0x20000000UL
/* Reset receiver */
#define XEM_ECR_RECV_RESET_MASK		 0x10000000UL
/* Enable receiver */
#define XEM_ECR_RECV_ENABLE_MASK	 0x08000000UL
/* Enable PHY */
#define XEM_ECR_PHY_ENABLE_MASK		 0x04000000UL
/* Enable xmit pad insert */
#define XEM_ECR_XMIT_PAD_ENABLE_MASK	 0x02000000UL
/* Enable xmit FCS insert */
#define XEM_ECR_XMIT_FCS_ENABLE_MASK	 0x01000000UL
/* Enable unicast addr */
#define XEM_ECR_UNICAST_ENABLE_MASK	 0x00020000UL
/* Enable broadcast addr */
#define XEM_ECR_BROAD_ENABLE_MASK	 0x00008000UL

/*
 * Transmit Status Register (TSR)
 */
/* Transmit excess deferral */
#define XEM_TSR_EXCESS_DEFERRAL_MASK	0x80000000UL
/* Transmit late collision */
#define XEM_TSR_LATE_COLLISION_MASK	0x01000000UL
