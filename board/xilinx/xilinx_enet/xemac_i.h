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
* @file xemac_i.h
*
* This header file contains internal identifiers, which are those shared
* between XEmac components.  The identifiers in this file are not intended for
* use external to the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rpm  07/31/01 First release
* 1.00b rpm  02/20/02 Repartitioned files and functions
* 1.00b rpm  04/29/02 Moved register definitions to xemac_l.h
* 1.00c rpm  12/05/02 New version includes support for simple DMA
* </pre>
*
******************************************************************************/

#ifndef XEMAC_I_H		/* prevent circular inclusions */
#define XEMAC_I_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xemac.h"
#include "xemac_l.h"

/************************** Constant Definitions *****************************/

/*
 * Default buffer descriptor control word masks. The default send BD control
 * is set for incrementing the source address by one for each byte transferred,
 * and specify that the destination address (FIFO) is local to the device. The
 * default receive BD control is set for incrementing the destination address
 * by one for each byte transferred, and specify that the source address is
 * local to the device.
 */
#define XEM_DFT_SEND_BD_MASK	(XDC_DMACR_SOURCE_INCR_MASK | \
				 XDC_DMACR_DEST_LOCAL_MASK)
#define XEM_DFT_RECV_BD_MASK	(XDC_DMACR_DEST_INCR_MASK |  \
				 XDC_DMACR_SOURCE_LOCAL_MASK)

/*
 * Masks for the IPIF Device Interrupt enable and status registers.
 */
#define XEM_IPIF_EMAC_MASK	0x00000004UL	/* MAC interrupt */
#define XEM_IPIF_SEND_DMA_MASK	0x00000008UL	/* Send DMA interrupt */
#define XEM_IPIF_RECV_DMA_MASK	0x00000010UL	/* Receive DMA interrupt */
#define XEM_IPIF_RECV_FIFO_MASK 0x00000020UL	/* Receive FIFO interrupt */
#define XEM_IPIF_SEND_FIFO_MASK 0x00000040UL	/* Send FIFO interrupt */

/*
 * Default IPIF Device Interrupt mask when configured for DMA
 */
#define XEM_IPIF_DMA_DFT_MASK	(XEM_IPIF_SEND_DMA_MASK |   \
				 XEM_IPIF_RECV_DMA_MASK |   \
				 XEM_IPIF_EMAC_MASK |	    \
				 XEM_IPIF_SEND_FIFO_MASK |  \
				 XEM_IPIF_RECV_FIFO_MASK)

/*
 * Default IPIF Device Interrupt mask when configured without DMA
 */
#define XEM_IPIF_FIFO_DFT_MASK	(XEM_IPIF_EMAC_MASK |	    \
				 XEM_IPIF_SEND_FIFO_MASK |  \
				 XEM_IPIF_RECV_FIFO_MASK)

#define XEM_IPIF_DMA_DEV_INTR_COUNT   7 /* Number of interrupt sources */
#define XEM_IPIF_FIFO_DEV_INTR_COUNT  5 /* Number of interrupt sources */
#define XEM_IPIF_DEVICE_INTR_COUNT  7	/* Number of interrupt sources */
#define XEM_IPIF_IP_INTR_COUNT	    22	/* Number of MAC interrupts */

/* a mask for all transmit interrupts, used in polled mode */
#define XEM_EIR_XMIT_ALL_MASK	(XEM_EIR_XMIT_DONE_MASK |	    \
				 XEM_EIR_XMIT_ERROR_MASK |	    \
				 XEM_EIR_XMIT_SFIFO_EMPTY_MASK |    \
				 XEM_EIR_XMIT_LFIFO_FULL_MASK)

/* a mask for all receive interrupts, used in polled mode */
#define XEM_EIR_RECV_ALL_MASK	(XEM_EIR_RECV_DONE_MASK |	    \
				 XEM_EIR_RECV_ERROR_MASK |	    \
				 XEM_EIR_RECV_LFIFO_EMPTY_MASK |    \
				 XEM_EIR_RECV_LFIFO_OVER_MASK |	    \
				 XEM_EIR_RECV_LFIFO_UNDER_MASK |    \
				 XEM_EIR_RECV_DFIFO_OVER_MASK |	    \
				 XEM_EIR_RECV_MISSED_FRAME_MASK |   \
				 XEM_EIR_RECV_COLLISION_MASK |	    \
				 XEM_EIR_RECV_FCS_ERROR_MASK |	    \
				 XEM_EIR_RECV_LEN_ERROR_MASK |	    \
				 XEM_EIR_RECV_SHORT_ERROR_MASK |    \
				 XEM_EIR_RECV_LONG_ERROR_MASK |	    \
				 XEM_EIR_RECV_ALIGN_ERROR_MASK)

/* a default interrupt mask for scatter-gather DMA operation */
#define XEM_EIR_DFT_SG_MASK    (XEM_EIR_RECV_ERROR_MASK |	    \
				XEM_EIR_RECV_LFIFO_OVER_MASK |	    \
				XEM_EIR_RECV_LFIFO_UNDER_MASK |	    \
				XEM_EIR_XMIT_SFIFO_OVER_MASK |	    \
				XEM_EIR_XMIT_SFIFO_UNDER_MASK |	    \
				XEM_EIR_XMIT_LFIFO_OVER_MASK |	    \
				XEM_EIR_XMIT_LFIFO_UNDER_MASK |	    \
				XEM_EIR_RECV_DFIFO_OVER_MASK |	    \
				XEM_EIR_RECV_MISSED_FRAME_MASK |    \
				XEM_EIR_RECV_COLLISION_MASK |	    \
				XEM_EIR_RECV_FCS_ERROR_MASK |	    \
				XEM_EIR_RECV_LEN_ERROR_MASK |	    \
				XEM_EIR_RECV_SHORT_ERROR_MASK |	    \
				XEM_EIR_RECV_LONG_ERROR_MASK |	    \
				XEM_EIR_RECV_ALIGN_ERROR_MASK)

/* a default interrupt mask for non-DMA operation (direct FIFOs) */
#define XEM_EIR_DFT_FIFO_MASK  (XEM_EIR_XMIT_DONE_MASK |	    \
				XEM_EIR_RECV_DONE_MASK |	    \
				XEM_EIR_DFT_SG_MASK)

/*
 * Mask for the DMA interrupt enable and status registers when configured
 * for scatter-gather DMA.
 */
#define XEM_DMA_SG_INTR_MASK	(XDC_IXR_DMA_ERROR_MASK	 |	\
				 XDC_IXR_PKT_THRESHOLD_MASK |	\
				 XDC_IXR_PKT_WAIT_BOUND_MASK |	\
				 XDC_IXR_SG_END_MASK)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/*
*
* Clears a structure of given size, in bytes, by setting each byte to 0.
*
* @param StructPtr is a pointer to the structure to be cleared.
* @param NumBytes is the number of bytes in the structure.
*
* @return
*
* None.
*
* @note
*
* Signature: void XEmac_mClearStruct(u8 *StructPtr, unsigned int NumBytes)
*
******************************************************************************/
#define XEmac_mClearStruct(StructPtr, NumBytes)	    \
{						    \
    int i;					    \
    u8 *BytePtr = (u8 *)(StructPtr);	    \
    for (i=0; i < (unsigned int)(NumBytes); i++)    \
    {						    \
	*BytePtr++ = 0;				    \
    }						    \
}

/************************** Variable Definitions *****************************/

extern XEmac_Config XEmac_ConfigTable[];

/************************** Function Prototypes ******************************/

void XEmac_CheckEmacError(XEmac * InstancePtr, u32 IntrStatus);
void XEmac_CheckFifoRecvError(XEmac * InstancePtr);
void XEmac_CheckFifoSendError(XEmac * InstancePtr);

#endif				/* end of protection macro */
