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
* @file xemac_intr.c
*
* This file contains general interrupt-related functions of the XEmac driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rpm  07/31/01 First release
* 1.00b rpm  02/20/02 Repartitioned files and functions
* 1.00c rpm  12/05/02 New version includes support for simple DMA
* 1.00c rpm  03/31/03 Added comment to indicate that no Receive Length FIFO
*                     overrun interrupts occur in v1.00l and later of the EMAC
*                     device. This avoids the need to reset the device on
*                     receive overruns.
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
* Set the callback function for handling asynchronous errors.  The upper layer
* software should call this function during initialization.
*
* The error callback is invoked by the driver within interrupt context, so it
* needs to do its job quickly. If there are potentially slow operations within
* the callback, these should be done at task-level.
*
* The Xilinx errors that must be handled by the callback are:
* - XST_DMA_ERROR indicates an unrecoverable DMA error occurred. This is
*   typically a bus error or bus timeout. The handler must reset and
*   re-configure the device.
* - XST_FIFO_ERROR indicates an unrecoverable FIFO error occurred. This is a
*   deadlock condition in the packet FIFO. The handler must reset and
*   re-configure the device.
* - XST_RESET_ERROR indicates an unrecoverable MAC error occurred, usually an
*   overrun or underrun. The handler must reset and re-configure the device.
* - XST_DMA_SG_NO_LIST indicates an attempt was made to access a scatter-gather
*   DMA list that has not yet been created.
* - XST_DMA_SG_LIST_EMPTY indicates the driver tried to get a descriptor from
*   the receive descriptor list, but the list was empty.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param CallBackRef is a reference pointer to be passed back to the adapter in
*        the callback. This helps the adapter correlate the callback to a
*        particular driver.
* @param FuncPtr is the pointer to the callback function.
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
void
XEmac_SetErrorHandler(XEmac * InstancePtr, void *CallBackRef,
		      XEmac_ErrorHandler FuncPtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(FuncPtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	InstancePtr->ErrorHandler = FuncPtr;
	InstancePtr->ErrorRef = CallBackRef;
}

/****************************************************************************/
/*
*
* Check the interrupt status bits of the Ethernet MAC for errors. Errors
* currently handled are:
* - Receive length FIFO overrun. Indicates data was lost due to the receive
*   length FIFO becoming full during the reception of a packet. Only a device
*   reset clears this condition.
* - Receive length FIFO underrun. An attempt to read an empty FIFO. Only a
*   device reset clears this condition.
* - Transmit status FIFO overrun. Indicates data was lost due to the transmit
*   status FIFO becoming full following the transmission of a packet. Only a
*   device reset clears this condition.
* - Transmit status FIFO underrun. An attempt to read an empty FIFO. Only a
*   device reset clears this condition.
* - Transmit length FIFO overrun. Indicates data was lost due to the transmit
*   length FIFO becoming full following the transmission of a packet. Only a
*   device reset clears this condition.
* - Transmit length FIFO underrun. An attempt to read an empty FIFO. Only a
*   device reset clears this condition.
* - Receive data FIFO overrun. Indicates data was lost due to the receive data
*   FIFO becoming full during the reception of a packet.
* - Receive data errors:
*   - Receive missed frame error. Valid data was lost by the MAC.
*   - Receive collision error. Data was lost by the MAC due to a collision.
*   - Receive FCS error.  Data was dicarded by the MAC due to FCS error.
*   - Receive length field error. Data was dicarded by the MAC due to an invalid
*     length field in the packet.
*   - Receive short error. Data was dicarded by the MAC because a packet was
*     shorter than allowed.
*   - Receive long error. Data was dicarded by the MAC because a packet was
*     longer than allowed.
*   - Receive alignment error. Data was truncated by the MAC because its length
*     was not byte-aligned.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param IntrStatus is the contents of the interrupt status register to be checked
*
* @return
*
* None.
*
* @note
*
* This function is intended for internal use only.
*
******************************************************************************/
void
XEmac_CheckEmacError(XEmac * InstancePtr, u32 IntrStatus)
{
	u32 ResetError = FALSE;

	/*
	 * First check for receive fifo overrun/underrun errors. Most require a
	 * reset by the user to clear, but the data FIFO overrun error does not.
	 */
	if (IntrStatus & XEM_EIR_RECV_DFIFO_OVER_MASK) {
		InstancePtr->Stats.RecvOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
	}

	if (IntrStatus & XEM_EIR_RECV_LFIFO_OVER_MASK) {
		/*
		 * Receive Length FIFO overrun interrupts no longer occur in v1.00l
		 * and later of the EMAC device. Frames are just dropped by the EMAC
		 * if the length FIFO is full. The user would notice the Receive Missed
		 * Frame count incrementing without any other errors being reported.
		 * This code is left here for backward compatibility with v1.00k and
		 * older EMAC devices.
		 */
		InstancePtr->Stats.RecvOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		ResetError = TRUE;	/* requires a reset */
	}

	if (IntrStatus & XEM_EIR_RECV_LFIFO_UNDER_MASK) {
		InstancePtr->Stats.RecvUnderrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		ResetError = TRUE;	/* requires a reset */
	}

	/*
	 * Now check for general receive errors. Get the latest count where
	 * available, otherwise just bump the statistic so we know the interrupt
	 * occurred.
	 */
	if (IntrStatus & XEM_EIR_RECV_ERROR_MASK) {
		if (IntrStatus & XEM_EIR_RECV_MISSED_FRAME_MASK) {
			/*
			 * Caused by length FIFO or data FIFO overruns on receive side
			 */
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

		/*
		 * Bump recv interrupts stats only if not scatter-gather DMA (this
		 * stat gets bumped elsewhere in that case)
		 */
		if (!XEmac_mIsSgDma(InstancePtr)) {
			InstancePtr->Stats.RecvInterrupts++;	/* TODO: double bump? */
		}

	}

	/*
	 * Check for transmit errors. These apply to both DMA and non-DMA modes
	 * of operation. The entire device should be reset after overruns or
	 * underruns.
	 */
	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_OVER_MASK |
			  XEM_EIR_XMIT_LFIFO_OVER_MASK)) {
		InstancePtr->Stats.XmitOverrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		ResetError = TRUE;
	}

	if (IntrStatus & (XEM_EIR_XMIT_SFIFO_UNDER_MASK |
			  XEM_EIR_XMIT_LFIFO_UNDER_MASK)) {
		InstancePtr->Stats.XmitUnderrunErrors++;
		InstancePtr->Stats.FifoErrors++;
		ResetError = TRUE;
	}

	if (ResetError) {
		/*
		 * If a reset error occurred, disable the EMAC interrupts since the
		 * reset-causing interrupt(s) is latched in the EMAC - meaning it will
		 * keep occurring until the device is reset. In order to give the higher
		 * layer software time to reset the device, we have to disable the
		 * overrun/underrun interrupts until that happens. We trust that the
		 * higher layer resets the device. We are able to get away with disabling
		 * all EMAC interrupts since the only interrupts it generates are for
		 * error conditions, and we don't care about any more errors right now.
		 */
		XIIF_V123B_WRITE_IIER(InstancePtr->BaseAddress, 0);

		/*
		 * Invoke the error handler callback, which should result in a reset
		 * of the device by the upper layer software.
		 */
		InstancePtr->ErrorHandler(InstancePtr->ErrorRef,
					  XST_RESET_ERROR);
	}
}

/*****************************************************************************/
/*
*
* Check the receive packet FIFO for errors. FIFO error interrupts are:
* - Deadlock.  See the XPacketFifo component for a description of deadlock on a
*   FIFO.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* Although the function returns void, it can return an asynchronous error to the
* application through the error handler.  It can return XST_FIFO_ERROR if a FIFO
* error occurred.
*
* @note
*
* This function is intended for internal use only.
*
******************************************************************************/
void
XEmac_CheckFifoRecvError(XEmac * InstancePtr)
{
	/*
	 * Although the deadlock is currently the only interrupt from a packet
	 * FIFO, make sure it is deadlocked before taking action. There is no
	 * need to clear this interrupt since it requires a reset of the device.
	 */
	if (XPF_V100B_IS_DEADLOCKED(&InstancePtr->RecvFifo)) {
		u32 IntrEnable;

		InstancePtr->Stats.FifoErrors++;

		/*
		 * Invoke the error callback function, which should result in a reset
		 * of the device by the upper layer software. We first need to disable
		 * the FIFO interrupt, since otherwise the upper layer thread that
		 * handles the reset may never run because this interrupt condition
		 * doesn't go away until a reset occurs (there is no way to ack it).
		 */
		IntrEnable = XIIF_V123B_READ_DIER(InstancePtr->BaseAddress);
		XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
				      IntrEnable & ~XEM_IPIF_RECV_FIFO_MASK);

		InstancePtr->ErrorHandler(InstancePtr->ErrorRef,
					  XST_FIFO_ERROR);
	}
}

/*****************************************************************************/
/*
*
* Check the send packet FIFO for errors. FIFO error interrupts are:
* - Deadlock. See the XPacketFifo component for a description of deadlock on a
*   FIFO.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* Although the function returns void, it can return an asynchronous error to the
* application through the error handler.  It can return XST_FIFO_ERROR if a FIFO
* error occurred.
*
* @note
*
* This function is intended for internal use only.
*
******************************************************************************/
void
XEmac_CheckFifoSendError(XEmac * InstancePtr)
{
	/*
	 * Although the deadlock is currently the only interrupt from a packet
	 * FIFO, make sure it is deadlocked before taking action. There is no
	 * need to clear this interrupt since it requires a reset of the device.
	 */
	if (XPF_V100B_IS_DEADLOCKED(&InstancePtr->SendFifo)) {
		u32 IntrEnable;

		InstancePtr->Stats.FifoErrors++;

		/*
		 * Invoke the error callback function, which should result in a reset
		 * of the device by the upper layer software. We first need to disable
		 * the FIFO interrupt, since otherwise the upper layer thread that
		 * handles the reset may never run because this interrupt condition
		 * doesn't go away until a reset occurs (there is no way to ack it).
		 */
		IntrEnable = XIIF_V123B_READ_DIER(InstancePtr->BaseAddress);
		XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
				      IntrEnable & ~XEM_IPIF_SEND_FIFO_MASK);

		InstancePtr->ErrorHandler(InstancePtr->ErrorRef,
					  XST_FIFO_ERROR);
	}
}
