/******************************************************************************
*
*     Author: Xilinx, Inc.
*
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
*     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
*     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
*     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
*     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
*     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
*     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
*     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
*     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
*     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
*     FITNESS FOR A PARTICULAR PURPOSE.
*
*
*     Xilinx hardware products are not intended for use in life support
*     appliances, devices, or systems. Use in such applications is
*     expressly prohibited.
*
*
*     (c) Copyright 2002-2004 Xilinx Inc.
*     All rights reserved.
*
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/
/*****************************************************************************/
/*
*
* @file xpacket_fifo_v1_00_b.h
*
* This component is a common component because it's primary purpose is to
* prevent code duplication in drivers. A driver which must handle a packet
* FIFO uses this component rather than directly manipulating a packet FIFO.
*
* A FIFO is a device which has dual port memory such that one user may be
* inserting data into the FIFO while another is consuming data from the FIFO.
* A packet FIFO is designed for use with packet protocols such as Ethernet and
* ATM.  It is typically only used with devices when DMA and/or Scatter Gather
* is used.  It differs from a nonpacket FIFO in that it does not provide any
* interrupts for thresholds of the FIFO such that it is less useful without
* DMA.
*
* @note
*
* This component has the capability to generate an interrupt when an error
* condition occurs.  It is the user's responsibility to provide the interrupt
* processing to handle the interrupt. This component provides the ability to
* determine if that interrupt is active, a deadlock condition, and the ability
* to reset the FIFO to clear the condition. In this condition, the device which
* is using the FIFO should also be reset to prevent other problems. This error
* condition could occur as a normal part of operation if the size of the FIFO
* is not setup correctly.  See the hardware IP specification for more details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm 03/26/02  First release
* </pre>
*
*****************************************************************************/
#ifndef XPACKET_FIFO_H		/* prevent circular inclusions */
#define XPACKET_FIFO_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/*
 * These constants specify the FIFO type and are mutually exclusive
 */
#define XPF_READ_FIFO_TYPE      0	/* a read FIFO */
#define XPF_WRITE_FIFO_TYPE     1	/* a write FIFO */

/*
 * These constants define the offsets to each of the registers from the
 * register base address, each of the constants are a number of bytes
 */
#define XPF_RESET_REG_OFFSET            0UL
#define XPF_MODULE_INFO_REG_OFFSET      0UL
#define XPF_COUNT_STATUS_REG_OFFSET     4UL

/*
 * This constant is used with the Reset Register
 */
#define XPF_RESET_FIFO_MASK             0x0000000A

/*
 * These constants are used with the Occupancy/Vacancy Count Register. This
 * register also contains FIFO status
 */
#define XPF_COUNT_MASK                  0x0000FFFF
#define XPF_DEADLOCK_MASK               0x20000000
#define XPF_ALMOST_EMPTY_FULL_MASK      0x40000000
#define XPF_EMPTY_FULL_MASK             0x80000000

/**************************** Type Definitions *******************************/

/*
 * The XPacketFifo driver instance data. The driver is required to allocate a
 * variable of this type for every packet FIFO in the device.
 */
typedef struct {
	u32 RegBaseAddress;	/* Base address of registers */
	u32 IsReady;		/* Device is initialized and ready */
	u32 DataBaseAddress;	/* Base address of data for FIFOs */
} XPacketFifoV100b;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/*
*
* Reset the specified packet FIFO.  Resetting a FIFO will cause any data
* contained in the FIFO to be lost.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* None.
*
* @note
*
* Signature: void XPF_V100B_RESET(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_RESET(InstancePtr) \
    XIo_Out32((InstancePtr)->RegBaseAddress + XPF_RESET_REG_OFFSET, XPF_RESET_FIFO_MASK);

/*****************************************************************************/
/*
*
* Get the occupancy count for a read packet FIFO and the vacancy count for a
* write packet FIFO. These counts indicate the number of 32-bit words
* contained (occupancy) in the FIFO or the number of 32-bit words available
* to write (vacancy) in the FIFO.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* The occupancy or vacancy count for the specified packet FIFO.
*
* @note
*
* Signature: u32 XPF_V100B_GET_COUNT(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_GET_COUNT(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_COUNT_MASK)

/*****************************************************************************/
/*
*
* Determine if the specified packet FIFO is almost empty. Almost empty is
* defined for a read FIFO when there is only one data word in the FIFO.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* TRUE if the packet FIFO is almost empty, FALSE otherwise.
*
* @note
*
* Signature: u32 XPF_V100B_IS_ALMOST_EMPTY(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_IS_ALMOST_EMPTY(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_ALMOST_EMPTY_FULL_MASK)

/*****************************************************************************/
/*
*
* Determine if the specified packet FIFO is almost full. Almost full is
* defined for a write FIFO when there is only one available data word in the
* FIFO.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* TRUE if the packet FIFO is almost full, FALSE otherwise.
*
* @note
*
* Signature: u32 XPF_V100B_IS_ALMOST_FULL(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_IS_ALMOST_FULL(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_ALMOST_EMPTY_FULL_MASK)

/*****************************************************************************/
/*
*
* Determine if the specified packet FIFO is empty. This applies only to a
* read FIFO.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* TRUE if the packet FIFO is empty, FALSE otherwise.
*
* @note
*
* Signature: u32 XPF_V100B_IS_EMPTY(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_IS_EMPTY(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_EMPTY_FULL_MASK)

/*****************************************************************************/
/*
*
* Determine if the specified packet FIFO is full. This applies only to a
* write FIFO.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* TRUE if the packet FIFO is full, FALSE otherwise.
*
* @note
*
* Signature: u32 XPF_V100B_IS_FULL(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_IS_FULL(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_EMPTY_FULL_MASK)

/*****************************************************************************/
/*
*
* Determine if the specified packet FIFO is deadlocked.  This condition occurs
* when the FIFO is full and empty at the same time and is caused by a packet
* being written to the FIFO which exceeds the total data capacity of the FIFO.
* It occurs because of the mark/restore features of the packet FIFO which allow
* retransmission of a packet. The software should reset the FIFO and any devices
* using the FIFO when this condition occurs.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
*
* @return
*
* TRUE if the packet FIFO is deadlocked, FALSE otherwise.
*
* @note
*
* This component has the capability to generate an interrupt when an error
* condition occurs.  It is the user's responsibility to provide the interrupt
* processing to handle the interrupt. This function provides the ability to
* determine if a deadlock condition, and the ability to reset the FIFO to
* clear the condition.
*
* In this condition, the device which is using the FIFO should also be reset
* to prevent other problems. This error condition could occur as a normal part
* of operation if the size of the FIFO is not setup correctly.
*
* Signature: u32 XPF_V100B_IS_DEADLOCKED(XPacketFifoV100b *InstancePtr)
*
******************************************************************************/
#define XPF_V100B_IS_DEADLOCKED(InstancePtr) \
    (XIo_In32((InstancePtr)->RegBaseAddress + XPF_COUNT_STATUS_REG_OFFSET) & \
    XPF_DEADLOCK_MASK)

/************************** Function Prototypes ******************************/

/* Standard functions */

XStatus XPacketFifoV100b_Initialize(XPacketFifoV100b * InstancePtr,
				    u32 RegBaseAddress, u32 DataBaseAddress);
XStatus XPacketFifoV100b_SelfTest(XPacketFifoV100b * InstancePtr, u32 FifoType);

/* Data functions */

XStatus XPacketFifoV100b_Read(XPacketFifoV100b * InstancePtr,
			      u8 * ReadBufferPtr, u32 ByteCount);
XStatus XPacketFifoV100b_Write(XPacketFifoV100b * InstancePtr,
			       u8 * WriteBufferPtr, u32 ByteCount);

#endif				/* end of protection macro */
