/* $Id: xiic_l.h,v 1.2 2002/12/05 19:32:40 meinelte Exp $ */
/*****************************************************************************
*
*	XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*	AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*	SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*	OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*	APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*	THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*	AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*	FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*	WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*	IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*	REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*	INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*	FOR A PARTICULAR PURPOSE.
*
*	(c) Copyright 2002 Xilinx Inc.
*	All rights reserved.
*
*****************************************************************************/
/****************************************************************************/
/**
*
* @file xiic_l.h
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  High-level driver functions
* are defined in xiic.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  05/07/02 First release
* 1.01c ecm  12/05/02 new rev
* </pre>
*
*****************************************************************************/

#ifndef XIIC_L_H /* prevent circular inclusions */
#define XIIC_L_H /* by using protection macros */

/***************************** Include Files ********************************/

#include "xbasic_types.h"

/************************** Constant Definitions ****************************/

#define XIIC_MSB_OFFSET		       3

#define XIIC_REG_OFFSET 0x100 + XIIC_MSB_OFFSET

/*
 * Register offsets in bytes from RegisterBase. Three is added to the
 * base offset to access LSB (IBM style) of the word
 */
#define XIIC_CR_REG_OFFSET   0x00+XIIC_REG_OFFSET   /* Control Register	  */
#define XIIC_SR_REG_OFFSET   0x04+XIIC_REG_OFFSET   /* Status Register	  */
#define XIIC_DTR_REG_OFFSET  0x08+XIIC_REG_OFFSET   /* Data Tx Register	  */
#define XIIC_DRR_REG_OFFSET  0x0C+XIIC_REG_OFFSET   /* Data Rx Register	  */
#define XIIC_ADR_REG_OFFSET  0x10+XIIC_REG_OFFSET   /* Address Register	  */
#define XIIC_TFO_REG_OFFSET  0x14+XIIC_REG_OFFSET   /* Tx FIFO Occupancy  */
#define XIIC_RFO_REG_OFFSET  0x18+XIIC_REG_OFFSET   /* Rx FIFO Occupancy  */
#define XIIC_TBA_REG_OFFSET  0x1C+XIIC_REG_OFFSET   /* 10 Bit Address reg */
#define XIIC_RFD_REG_OFFSET  0x20+XIIC_REG_OFFSET   /* Rx FIFO Depth reg  */

/* Control Register masks */

#define XIIC_CR_ENABLE_DEVICE_MASK	  0x01	/* Device enable = 1	  */
#define XIIC_CR_TX_FIFO_RESET_MASK	  0x02	/* Transmit FIFO reset=1  */
#define XIIC_CR_MSMS_MASK		  0x04	/* Master starts Txing=1  */
#define XIIC_CR_DIR_IS_TX_MASK		  0x08	/* Dir of tx. Txing=1	  */
#define XIIC_CR_NO_ACK_MASK		  0x10	/* Tx Ack. NO ack = 1	  */
#define XIIC_CR_REPEATED_START_MASK	  0x20	/* Repeated start = 1	  */
#define XIIC_CR_GENERAL_CALL_MASK	  0x40	/* Gen Call enabled = 1	  */

/* Status Register masks */

#define XIIC_SR_GEN_CALL_MASK		  0x01	/* 1=a mstr issued a GC	  */
#define XIIC_SR_ADDR_AS_SLAVE_MASK	  0x02	/* 1=when addr as slave	  */
#define XIIC_SR_BUS_BUSY_MASK		  0x04	/* 1 = bus is busy	  */
#define XIIC_SR_MSTR_RDING_SLAVE_MASK	  0x08	/* 1=Dir: mstr <-- slave  */
#define XIIC_SR_TX_FIFO_FULL_MASK	  0x10	/* 1 = Tx FIFO full	  */
#define XIIC_SR_RX_FIFO_FULL_MASK	  0x20	/* 1 = Rx FIFO full	  */
#define XIIC_SR_RX_FIFO_EMPTY_MASK	  0x40	/* 1 = Rx FIFO empty	  */

/* IPIF Interrupt Status Register masks	   Interrupt occurs when...	  */

#define XIIC_INTR_ARB_LOST_MASK		  0x01	/* 1 = arbitration lost	  */
#define XIIC_INTR_TX_ERROR_MASK		  0x02	/* 1=Tx error/msg complete*/
#define XIIC_INTR_TX_EMPTY_MASK		  0x04	/* 1 = Tx FIFO/reg empty  */
#define XIIC_INTR_RX_FULL_MASK		  0x08	/* 1=Rx FIFO/reg=OCY level*/
#define XIIC_INTR_BNB_MASK		  0x10	/* 1 = Bus not busy	  */
#define XIIC_INTR_AAS_MASK		  0x20	/* 1 = when addr as slave */
#define XIIC_INTR_NAAS_MASK		  0x40	/* 1 = not addr as slave  */
#define XIIC_INTR_TX_HALF_MASK		  0x80	/* 1 = TX FIFO half empty */

/* IPIF Device Interrupt Register masks */

#define XIIC_IPIF_IIC_MASK	    0x00000004UL    /* 1=inter enabled */
#define XIIC_IPIF_ERROR_MASK	    0x00000001UL    /* 1=inter enabled */
#define XIIC_IPIF_INTER_ENABLE_MASK  (XIIC_IPIF_IIC_MASK |  \
				      XIIC_IPIF_ERROR_MASK)

#define XIIC_TX_ADDR_SENT	      0x00
#define XIIC_TX_ADDR_MSTR_RECV_MASK   0x02

/* The following constants specify the depth of the FIFOs */

#define IIC_RX_FIFO_DEPTH	  16   /* Rx fifo capacity		 */
#define IIC_TX_FIFO_DEPTH	  16   /* Tx fifo capacity		 */

/* The following constants specify groups of interrupts that are typically
 * enabled or disables at the same time
 */
#define XIIC_TX_INTERRUPTS					    \
	    (XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_TX_EMPTY_MASK |    \
	     XIIC_INTR_TX_HALF_MASK)

#define XIIC_TX_RX_INTERRUPTS (XIIC_INTR_RX_FULL_MASK | XIIC_TX_INTERRUPTS)

/* The following constants are used with the following macros to specify the
 * operation, a read or write operation.
 */
#define XIIC_READ_OPERATION  1
#define XIIC_WRITE_OPERATION 0

/* The following constants are used with the transmit FIFO fill function to
 * specify the role which the IIC device is acting as, a master or a slave.
 */
#define XIIC_MASTER_ROLE     1
#define XIIC_SLAVE_ROLE	     0

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

unsigned XIic_Recv(u32 BaseAddress, u8 Address,
		   u8 *BufferPtr, unsigned ByteCount);

unsigned XIic_Send(u32 BaseAddress, u8 Address,
		   u8 *BufferPtr, unsigned ByteCount);

#endif		  /* end of protection macro */
