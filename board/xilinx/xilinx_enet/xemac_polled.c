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
/**
*
* @file xemac_polled.c
*
* Contains functions used when the driver is in polled mode. Use the
* XEmac_SetOptions() function to put the driver into polled mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rpm  07/31/01 First release
* 1.00b rpm  02/20/02 Repartitioned files and functions
* 1.00c rpm  12/05/02 New version includes support for simple DMA
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xemac_i.h"
#include "xio.h"
#include "xipif_v1_23_b.h"	/* Uses v1.23b of the IPIF */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Send an Ethernet frame in polled mode.  The device/driver must be in polled
* mode before calling this function. The driver writes the frame directly to
* the MAC's packet FIFO, then enters a loop checking the device status for
* completion or error. Statistics are updated if an error occurs. The buffer
* to be sent must be word-aligned.
*
* It is assumed that the upper layer software supplies a correctly formatted
* Ethernet frame, including the destination and source addresses, the
* type/length field, and the data field.  It is also assumed that upper layer
* software does not append FCS at the end of the frame.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param BufPtr is a pointer to a word-aligned buffer containing the Ethernet
*        frame to be sent.
* @param ByteCount is the size of the Ethernet frame.
*
* @return
*
* - XST_SUCCESS if the frame was sent successfully
* - XST_DEVICE_IS_STOPPED if the device has not yet been started
* - XST_NOT_POLLED if the device is not in polled mode
* - XST_FIFO_NO_ROOM if there is no room in the EMAC's length FIFO for this frame
* - XST_FIFO_ERROR if the FIFO was overrun or underrun. This error is critical
*   and requires the caller to reset the device.
* - XST_EMAC_COLLISION if the send failed due to excess deferral or late
*   collision
*
* @note
*
* There is the possibility that this function will not return if the hardware
* is broken (i.e., it never sets the status bit indicating that transmission is
* done). If this is of concern to the user, the user should provide protection
* from this problem - perhaps by using a different timer thread to monitor the
* PollSend thread. On a 10Mbps MAC, it takes about 1.21 msecs to transmit a
* maximum size Ethernet frame (1518 bytes). On a 100Mbps MAC, it takes about
* 121 usecs to transmit a maximum size Ethernet frame.
*
* @internal
*
* The EMAC uses FIFOs behind its length and status registers. For this reason,
* it is important to keep the length, status, and data FIFOs in sync when
* reading or writing to them.
*
******************************************************************************/
XStatus
XEmac_PollSend(XEmac * InstancePtr, u8 * BufPtr, u32 ByteCount)
{
	u32 IntrStatus;
	u32 XmitStatus;
	XStatus Result;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufPtr != NULL);
	XASSERT_NONVOID(ByteCount > XEM_HDR_SIZE);	/* send at least 1 byte */
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Be sure the device is configured for polled mode and it is started
	 */
	if (!InstancePtr->IsPolled) {
		return XST_NOT_POLLED;
	}

	if (InstancePtr->IsStarted != XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STOPPED;
	}

	/*
	 * Check for overruns and underruns for the transmit status and length
	 * FIFOs and make sure the send packet FIFO is not deadlocked. Any of these
	 * conditions is bad enough that we do not want to continue. The upper layer
	 * software should reset the device to resolve the error.
	 */
	IntrStatus = XIIF_V123B_READ_IISR(InstancePtr->BaseAddress);

	/*
	 * Overrun errors
	 */
	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_OVER_MASK |
			  XEM_EIR_XMIT_LFIFO_OVER_MASK)) {
		InstancePtr->Stats.XmitOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	/*
	 * Underrun errors
	 */
	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
			  XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
		InstancePtr->Stats.XmitUnderrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	if (XPF_V100B_IS_DEADLOCKED(&InstancePtr->SendFifo)) {
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	/*
	 * Before writing to the data FIFO, make sure the length FIFO is not
	 * full.  The data FIFO might not be full yet even though the length FIFO
	 * is. This avoids an overrun condition on the length FIFO and keeps the
	 * FIFOs in sync.
	 */
	if (IntrStatus & XEM_EIR_XMIT_LFIFO_FULL_MASK) {
		/*
		 * Clear the latched LFIFO_FULL bit so next time around the most
		 * current status is represented
		 */
		XIIF_V123B_WRITE_IISR(InstancePtr->BaseAddress,
				      XEM_EIR_XMIT_LFIFO_FULL_MASK);
		return XST_FIFO_NO_ROOM;
	}

	/*
	 * This is a non-blocking write. The packet FIFO returns an error if there
	 * is not enough room in the FIFO for this frame.
	 */
	Result =
	    XPacketFifoV100b_Write(&InstancePtr->SendFifo, BufPtr, ByteCount);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/*
	 * Loop on the MAC's status to wait for any pause to complete.
	 */
	IntrStatus = XIIF_V123B_READ_IISR(InstancePtr->BaseAddress);

	while ((IntrStatus & XEM_EIR_XMIT_PAUSE_MASK) != 0) {
		IntrStatus = XIIF_V123B_READ_IISR(InstancePtr->BaseAddress);
		/*
		   * Clear the pause status from the transmit status register
		 */
		XIIF_V123B_WRITE_IISR(InstancePtr->BaseAddress,
				      IntrStatus & XEM_EIR_XMIT_PAUSE_MASK);
	}

	/*
	 * Set the MAC's transmit packet length register to tell it to transmit
	 */
	XIo_Out32(InstancePtr->BaseAddress + XEM_TPLR_OFFSET, ByteCount);

	/*
	 * Loop on the MAC's status to wait for the transmit to complete. The
	 * transmit status is in the FIFO when the XMIT_DONE bit is set.
	 */
	do {
		IntrStatus = XIIF_V123B_READ_IISR(InstancePtr->BaseAddress);
	}
	while ((IntrStatus & XEM_EIR_XMIT_DONE_MASK) == 0);

	XmitStatus = XIo_In32(InstancePtr->BaseAddress + XEM_TSR_OFFSET);

	InstancePtr->Stats.XmitFrames++;
	InstancePtr->Stats.XmitBytes += ByteCount;

	/*
	 * Check for various errors, bump statistics, and return an error status.
	 */

	/*
	 * Overrun errors
	 */
	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_OVER_MASK |
			  XEM_EIR_XMIT_LFIFO_OVER_MASK)) {
		InstancePtr->Stats.XmitOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	/*
	 * Underrun errors
	 */
	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
			  XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
		InstancePtr->Stats.XmitUnderrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	/*
	 * Clear the interrupt status register of transmit statuses
	 */
	XIIF_V123B_WRITE_IISR(InstancePtr->BaseAddress,
			      IntrStatus & XEM_EIR_XMIT_ALL_MASK);

	/*
	 * Collision errors are stored in the transmit status register
	 * instead of the interrupt status register
	 */
	if (XmitStatus & XEM_TSR_EXCESS_DEFERRAL_MASK) {
		InstancePtr->Stats.XmitExcessDeferral++;
		return XST_EMAC_COLLISION_ERROR;
	}

	if (XmitStatus & XEM_TSR_LATE_COLLISION_MASK) {
		InstancePtr->Stats.XmitLateCollisionErrors++;
		return XST_EMAC_COLLISION_ERROR;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Receive an Ethernet frame in polled mode. The device/driver must be in polled
* mode before calling this function. The driver receives the frame directly
* from the MAC's packet FIFO. This is a non-blocking receive, in that if there
* is no frame ready to be received at the device, the function returns with an
* error. The MAC's error status is not checked, so statistics are not updated
* for polled receive. The buffer into which the frame will be received must be
* word-aligned.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param BufPtr is a pointer to a word-aligned buffer into which the received
*        Ethernet frame will be copied.
* @param ByteCountPtr is both an input and an output parameter. It is a pointer
*        to a 32-bit word that contains the size of the buffer on entry into the
*        function and the size the received frame on return from the function.
*
* @return
*
* - XST_SUCCESS if the frame was sent successfully
* - XST_DEVICE_IS_STOPPED if the device has not yet been started
* - XST_NOT_POLLED if the device is not in polled mode
* - XST_NO_DATA if there is no frame to be received from the FIFO
* - XST_BUFFER_TOO_SMALL if the buffer to receive the frame is too small for
*   the frame waiting in the FIFO.
*
* @note
*
* Input buffer must be big enough to hold the largest Ethernet frame. Buffer
* must also be 32-bit aligned.
*
* @internal
*
* The EMAC uses FIFOs behind its length and status registers. For this reason,
* it is important to keep the length, status, and data FIFOs in sync when
* reading or writing to them.
*
******************************************************************************/
XStatus
XEmac_PollRecv(XEmac * InstancePtr, u8 * BufPtr, u32 * ByteCountPtr)
{
	XStatus Result;
	u32 PktLength;
	u32 IntrStatus;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufPtr != NULL);
	XASSERT_NONVOID(ByteCountPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Be sure the device is configured for polled mode and it is started
	 */
	if (!InstancePtr->IsPolled) {
		return XST_NOT_POLLED;
	}

	if (InstancePtr->IsStarted != XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STOPPED;
	}

	/*
	 * Make sure the buffer is big enough to hold the maximum frame size.
	 * We need to do this because as soon as we read the MAC's packet length
	 * register, which is actually a FIFO, we remove that length from the
	 * FIFO.  We do not want to read the length FIFO without also reading the
	 * data FIFO since this would get the FIFOs out of sync.  So we have to
	 * make this restriction.
	 */
	if (*ByteCountPtr < XEM_MAX_FRAME_SIZE) {
		return XST_BUFFER_TOO_SMALL;
	}

	/*
	 * First check for packet FIFO deadlock and return an error if it has
	 * occurred. A reset by the caller is necessary to correct this problem.
	 */
	if (XPF_V100B_IS_DEADLOCKED(&InstancePtr->RecvFifo)) {
		InstancePtr->Stats.FifoErrors++;
		return XST_FIFO_ERROR;
	}

	/*
	 * Get the interrupt status to know what happened (whether an error occurred
	 * and/or whether frames have been received successfully). When clearing the
	 * intr status register, clear only statuses that pertain to receive.
	 */
	IntrStatus = XIIF_V123B_READ_IISR(InstancePtr->BaseAddress);
	XIIF_V123B_WRITE_IISR(InstancePtr->BaseAddress,
			      IntrStatus & XEM_EIR_RECV_ALL_MASK);

	/*
	 * Check receive errors and bump statistics so the caller will have a clue
	 * as to why data may not have been received. We continue on if an error
	 * occurred since there still may be frames that were received successfully.
	 */
	if (IntrStatus & (XEM_EIR_RECV_LFIFO_OVER_MASK |
			  XEM_EIR_RECV_DFIFO_OVER_MASK)) {
		InstancePtr->Stats.RecvOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
	}

	if (IntrStatus & XEM_EIR_RECV_LFIFO_UNDER_MASK) {
		InstancePtr->Stats.RecvUnderrunErrors++;
		InstancePtr->Stats.FifoErrors++;
	}

	/*
	 * General receive errors
	 */
	if (IntrStatus & XEM_EIR_RECV_ERROR_MASK) {
		if (IntrStatus & XEM_EIR_RECV_MISSED_FRAME_MASK) {
			InstancePtr->Stats.RecvMissedFrameErrors =
			    XIo_In32(InstancePtr->BaseAddress +
				     XEM_RMFC_OFFSET);
		}

		if (IntrStatus & XEM_EIR_RECV_COLLISION_MASK) {
			InstancePtr->Stats.RecvCollisionErrors =
			    XIo_In32(InstancePtr->BaseAddress + XEM_RCC_OFFSET);
		}

		if (IntrStatus & XEM_EIR_RECV_FCS_ERROR_MASK) {
			InstancePtr->Stats.RecvFcsErrors =
			    XIo_In32(InstancePtr->BaseAddress +
				     XEM_RFCSEC_OFFSET);
		}

		if (IntrStatus & XEM_EIR_RECV_LEN_ERROR_MASK) {
			InstancePtr->Stats.RecvLengthFieldErrors++;
		}

		if (IntrStatus & XEM_EIR_RECV_SHORT_ERROR_MASK) {
			InstancePtr->Stats.RecvShortErrors++;
		}

		if (IntrStatus & XEM_EIR_RECV_LONG_ERROR_MASK) {
			InstancePtr->Stats.RecvLongErrors++;
		}

		if (IntrStatus & XEM_EIR_RECV_ALIGN_ERROR_MASK) {
			InstancePtr->Stats.RecvAlignmentErrors =
			    XIo_In32(InstancePtr->BaseAddress +
				     XEM_RAEC_OFFSET);
		}
	}

	/*
	 * Before reading from the length FIFO, make sure the length FIFO is not
	 * empty. We could cause an underrun error if we try to read from an
	 * empty FIFO.
	 */
	if ((IntrStatus & XEM_EIR_RECV_DONE_MASK) == 0) {
		return XST_NO_DATA;
	}

	/*
	 * Determine, from the MAC, the length of the next packet available
	 * in the data FIFO (there should be a non-zero length here)
	 */
	PktLength = XIo_In32(InstancePtr->BaseAddress + XEM_RPLR_OFFSET);
	if (PktLength == 0) {
		return XST_NO_DATA;
	}

	/*
	 * Write the RECV_DONE bit in the status register to clear it. This bit
	 * indicates the RPLR is non-empty, and we know it's set at this point.
	 * We clear it so that subsequent entry into this routine will reflect the
	 * current status. This is done because the non-empty bit is latched in the
	 * IPIF, which means it may indicate a non-empty condition even though
	 * there is something in the FIFO.
	 */
	XIIF_V123B_WRITE_IISR(InstancePtr->BaseAddress, XEM_EIR_RECV_DONE_MASK);

	/*
	 * We assume that the MAC never has a length bigger than the largest
	 * Ethernet frame, so no need to make another check here.
	 */

	/*
	 * This is a non-blocking read. The FIFO returns an error if there is
	 * not at least the requested amount of data in the FIFO.
	 */
	Result =
	    XPacketFifoV100b_Read(&InstancePtr->RecvFifo, BufPtr, PktLength);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	InstancePtr->Stats.RecvFrames++;
	InstancePtr->Stats.RecvBytes += PktLength;

	*ByteCountPtr = PktLength;

	return XST_SUCCESS;
}
