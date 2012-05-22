/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * FIFO sub-controller interface
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */
#ifndef _XILINX_LL_TEMAC_FIFO_
#define _XILINX_LL_TEMAC_FIFO_

#include <net.h>

#include <asm/types.h>
#include <asm/byteorder.h>

#if !defined(__BIG_ENDIAN)
# error LL_TEMAC requires big endianess
#endif

/*
 * FIFO Register Definition
 *
 * Used for memory mapped access from and to (Rd/Td) the LocalLink (LL)
 * Tri-Mode Ether MAC (TEMAC) via the 2 kb full duplex FIFO Controller,
 * one for each.
 *
 * [1]: [0]/ip_documentation/xps_ll_fifo.pdf
 *      page 10, Registers Definition
 */
struct fifo_ctrl {
	u32 isr;	/* Interrupt Status Register (RW) */
	u32 ier;	/* Interrupt Enable Register (RW) */
	u32 tdfr;	/* Transmit Data FIFO Reset (WO) */
	u32 tdfv;	/* Transmit Data FIFO Vacancy (RO) */
	u32 tdfd;	/* Transmit Data FIFO 32bit wide Data write port (WO) */
	u32 tlf;	/* Transmit Length FIFO (WO) */
	u32 rdfr;	/* Receive Data FIFO Reset (WO) */
	u32 rdfo;	/* Receive Data FIFO Occupancy (RO) */
	u32 rdfd;	/* Receive Data FIFO 32bit wide Data read port (RO) */
	u32 rlf;	/* Receive Length FIFO (RO) */
	u32 llr;	/* LocalLink Reset (WO) */
};

/* Interrupt Status Register (ISR), [1] p11 */
#define LL_FIFO_ISR_RPURE	(1 << 31) /* Receive Packet Underrun Read Err */
#define LL_FIFO_ISR_RPORE	(1 << 30) /* Receive Packet Overrun Read Err */
#define LL_FIFO_ISR_RPUE	(1 << 29) /* Receive Packet Underrun Error */
#define LL_FIFO_ISR_TPOE	(1 << 28) /* Transmit Packet Overrun Error */
#define LL_FIFO_ISR_TC		(1 << 27) /* Transmit Complete */
#define LL_FIFO_ISR_RC		(1 << 26) /* Receive Complete */
#define LL_FIFO_ISR_TSE		(1 << 25) /* Transmit Size Error */
#define LL_FIFO_ISR_TRC		(1 << 24) /* Transmit Reset Complete */
#define LL_FIFO_ISR_RRC		(1 << 23) /* Receive Reset Complete */

/* Interrupt Enable Register (IER), [1] p12/p13 */
#define LL_FIFO_IER_RPURE	(1 << 31) /* Receive Packet Underrun Read Err */
#define LL_FIFO_IER_RPORE	(1 << 30) /* Receive Packet Overrun Read Err */
#define LL_FIFO_IER_RPUE	(1 << 29) /* Receive Packet Underrun Error */
#define LL_FIFO_IER_TPOE	(1 << 28) /* Transmit Packet Overrun Error */
#define LL_FIFO_IER_TC		(1 << 27) /* Transmit Complete */
#define LL_FIFO_IER_RC		(1 << 26) /* Receive Complete */
#define LL_FIFO_IER_TSE		(1 << 25) /* Transmit Size Error */
#define LL_FIFO_IER_TRC		(1 << 24) /* Transmit Reset Complete */
#define LL_FIFO_IER_RRC		(1 << 23) /* Receive Reset Complete */

/* Transmit Data FIFO Reset (TDFR), [1] p13/p14 */
#define LL_FIFO_TDFR_KEY	0x000000A5UL

/* Transmit Data FIFO Vacancy (TDFV), [1] p14 */
#define LL_FIFO_TDFV_POS	0
#define LL_FIFO_TDFV_MASK	(0x000001FFUL << LL_FIFO_TDFV_POS)

/* Transmit Length FIFO (TLF), [1] p16/p17 */
#define LL_FIFO_TLF_POS		0
#define LL_FIFO_TLF_MASK	(0x000007FFUL << LL_FIFO_TLF_POS)
#define LL_FIFO_TLF_MIN		((4 * sizeof(u32)) & LL_FIFO_TLF_MASK)
#define LL_FIFO_TLF_MAX		((510 * sizeof(u32)) & LL_FIFO_TLF_MASK)

/* Receive Data FIFO Reset (RDFR), [1] p15 */
#define LL_FIFO_RDFR_KEY	0x000000A5UL

/* Receive Data FIFO Occupancy (RDFO), [1] p16 */
#define LL_FIFO_RDFO_POS	0
#define LL_FIFO_RDFO_MASK	(0x000001FFUL << LL_FIFO_RDFO_POS)

/* Receive Length FIFO (RLF), [1] p17/p18 */
#define LL_FIFO_RLF_POS		0
#define LL_FIFO_RLF_MASK	(0x000007FFUL << LL_FIFO_RLF_POS)
#define LL_FIFO_RLF_MIN		((4 * sizeof(uint32)) & LL_FIFO_RLF_MASK)
#define LL_FIFO_RLF_MAX		((510 * sizeof(uint32)) & LL_FIFO_RLF_MASK)

/* LocalLink Reset (LLR), [1] p18 */
#define LL_FIFO_LLR_KEY		0x000000A5UL


/* reset FIFO and IRQ, disable interrupts */
int ll_temac_reset_fifo(struct eth_device *dev);

/* receive buffered data from FIFO (polling ISR) */
int ll_temac_recv_fifo(struct eth_device *dev);

/* send buffered data to FIFO */
int ll_temac_send_fifo(struct eth_device *dev, void *packet, int length);

#endif /* _XILINX_LL_TEMAC_FIFO_ */
