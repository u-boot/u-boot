/* $Id: xdma_channel_sg.c,v 1.6 2003/02/03 19:50:33 moleres Exp $ */
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
* FILENAME:
*
* xdma_channel_sg.c
*
* DESCRIPTION:
*
* This file contains the implementation of the XDmaChannel component which is
* related to scatter gather operations.
*
* Scatter Gather Operations
*
* The DMA channel may support scatter gather operations. A scatter gather
* operation automates the DMA channel such that multiple buffers can be
* sent or received with minimal software interaction with the hardware.	 Buffer
* descriptors, contained in the XBufDescriptor component, are used by the
* scatter gather operations of the DMA channel to describe the buffers to be
* processed.
*
* Scatter Gather List Operations
*
* A scatter gather list may be supported by each DMA channel.  The scatter
* gather list allows buffer descriptors to be put into the list by a device
* driver which requires scatter gather.	 The hardware processes the buffer
* descriptors which are contained in the list and modifies the buffer
* descriptors to reflect the status of the DMA operations.  The device driver
* is notified by interrupt that specific DMA events occur including scatter
* gather events.  The device driver removes the completed buffer descriptors
* from the scatter gather list to evaluate the status of each DMA operation.
*
* The scatter gather list is created and buffer descriptors are inserted into
* the list.  Buffer descriptors are never removed from the list after it's
* creation such that a put operation copies from a temporary buffer descriptor
* to a buffer descriptor in the list.  Get operations don't copy from the list
* to a temporary, but return a pointer to the buffer descriptor in the list.
* A buffer descriptor in the list may be locked to prevent it from being
* overwritten by a put operation.  This allows the device driver to get a
* descriptor from a scatter gather list and prevent it from being overwritten
* until the buffer associated with the buffer descriptor has been processed.
*
* The get and put functions only operate on the list and are asynchronous from
* the hardware which may be using the list of descriptors.  This is important
* because there are no checks in the get and put functions to ensure that the
* hardware has processed the descriptors.  This must be handled by the driver
* using the DMA scatter gather channel through the use of the other functions.
* When a scatter gather operation is started, the start function does ensure
* that the descriptor to start has not already been processed by the hardware
* and is not the first of a series of descriptors that have not been committed
* yet.
*
* Descriptors are put into the list but not marked as ready to use by the
* hardware until a commit operation is done.  This allows multiple descriptors
* which may contain a single packet of information for a protocol to be
* guaranteed not to cause any underflow conditions during transmission. The
* hardware design only allows descriptors to cause it to stop after a descriptor
* has been processed rather than before it is processed.  A series of
* descriptors are put into the list followed by a commit operation, or each
* descriptor may be commited.  A commit operation is performed by changing a
* single descriptor, the first of the series of puts, to indicate that the
* hardware may now use all descriptors after it.  The last descriptor in the
* list is always set to cause the hardware to stop after it is processed.
*
* Typical Scatter Gather Processing
*
* The following steps illustrate the typical processing to use the
* scatter gather features of a DMA channel.
*
* 1. Create a scatter gather list for the DMA channel which puts empty buffer
*    descriptors into the list.
* 2. Create buffer descriptors which describe the buffers to be filled with
*    receive data or the buffers which contain data to be sent.
* 3. Put buffer descriptors into the DMA channel scatter list such that scatter
*    gather operations are requested.
* 4. Commit the buffer descriptors in the list such that they are ready to be
*    used by the DMA channel hardware.
* 5. Start the scatter gather operations of the DMA channel.
* 6. Process any interrupts which occur as a result of the scatter gather
*    operations or poll the DMA channel to determine the status.  This may
*    be accomplished by getting the packet count for the channel and then
*    getting the appropriate number of descriptors from the list for that
*    number of packets.
*
* Minimizing Interrupts
*
* The Scatter Gather operating mode is designed to reduce the amount of CPU
* throughput necessary to manage the hardware for devices. A key to the CPU
* throughput is the number and rate of interrupts that the CPU must service.
* Devices with higher data rates can cause larger numbers of interrupts and
* higher frequency interrupts. Ideally the number of interrupts can be reduced
* by only generating an interrupt when a specific amount of data has been
* received from the interface. This design suffers from a lack of interrupts
* when the amount of data received is less than the specified amount of data
* to generate an interrupt. In order to help minimize the number of interrupts
* which the CPU must service, an algorithm referred to as "interrupt coalescing"
* is utilized.
*
* Interrupt Coalescing
*
* The principle of interrupt coalescing is to wait before generating an
* interrupt until a certain number of packets have been received or sent. An
* interrupt is also generated if a smaller number of packets have been received
* followed by a certain period of time with no packet reception. This is a
* trade-off of latency for bandwidth and is accomplished using several
* mechanisms of the hardware including a counter for packets received or
* transmitted and a packet timer. These two hardware mechanisms work in
* combination to allow a reduction in the number of interrupts processed by the
* CPU for packet reception.
*
* Unserviced Packet Count
*
* The purpose of the packet counter is to count the number of packets received
* or transmitted and provide an interrupt when a specific number of packets
* have been processed by the hardware. An interrupt is generated whenever the
* counter is greater than or equal to the Packet Count Threshold. This counter
* contains an accurate count of the number of packets that the hardware has
* processed, either received or transmitted, and the software has not serviced.
*
* The packet counter allows the number of interrupts to be reduced by waiting
* to generate an interrupt until enough packets are received. For packet
* reception, packet counts of less than the number to generate an interrupt
* would not be serviced without the addition of a packet timer. This counter is
* continuously updated by the hardware, not latched to the value at the time
* the interrupt occurred.
*
* The packet counter can be used within the interrupt service routine for the
* device to reduce the number of interrupts. The interrupt service routine
* loops while performing processing for each packet which has been received or
* transmitted and decrements the counter by a specified value. At the same time,
* the hardware is possibly continuing to receive or transmit more packets such
* that the software may choose, based upon the value in the packet counter, to
* remain in the interrupt service routine rather than exiting and immediately
* returning. This feature should be used with caution as reducing the number of
* interrupts is beneficial, but unbounded interrupt processing is not desirable.
*
* Since the hardware may be incrementing the packet counter simultaneously
* with the software decrementing the counter, there is a need for atomic
* operations. The hardware ensures that the operation is atomic such that
* simultaneous accesses are properly handled.
*
* Packet Wait Bound
*
* The purpose of the packet wait bound is to augment the unserviced packet
* count. Whenever there is no pending interrupt for the channel and the
* unserviced packet count is non-zero, a timer starts counting timeout at the
* value contained the the packet wait bound register.  If the timeout is
* reached, an interrupt is generated such that the software may service the
* data which was buffered.
*
* NOTES:
*
* Special Test Conditions:
*
* The scatter gather list processing must be thoroughly tested if changes are
* made.	 Testing should include putting and committing single descriptors and
* putting multiple descriptors followed by a single commit.  There are some
* conditions in the code which handle the exception conditions.
*
* The Put Pointer points to the next location in the descriptor list to copy
* in a new descriptor. The Get Pointer points to the next location in the
* list to get a descriptor from.  The Get Pointer only allows software to
* have a traverse the list after the hardware has finished processing some
* number of descriptors.  The Commit Pointer points to the descriptor in the
* list which is to be committed.  It is also used to determine that no
* descriptor is waiting to be commited (NULL).	The Last Pointer points to
* the last descriptor that was put into the list.  It typically points
* to the previous descriptor to the one pointed to by the Put Pointer.
* Comparisons are done between these pointers to determine when the following
* special conditions exist.

* Single Put And Commit
*
* The buffer descriptor is ready to be used by the hardware so it is important
* for the descriptor to not appear to be waiting to be committed.  The commit
* pointer is reset when a commit is done indicating there are no descriptors
* waiting to be committed.  In all cases but this one, the descriptor is
* changed to cause the hardware to go to the next descriptor after processing
* this one.  But in this case, this is the last descriptor in the list such
* that it must not be changed.
*
* 3 Or More Puts And Commit
*
* A series of 3 or more puts followed by a single commit is different in that
* only the 1st descriptor put into the list is changed when the commit is done.
* This requires each put starting on the 3rd to change the previous descriptor
* so that it allows the hardware to continue to the next descriptor in the list.
*
* The 1st Put Following A Commit
*
* The commit caused the commit pointer to be NULL indicating that there are no
* descriptors waiting to be committed.	It is necessary for the next put to set
* the commit pointer so that a commit must follow the put for the hardware to
* use the descriptor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a rpm  02/03/03 Removed the XST_DMA_SG_COUNT_EXCEEDED return code
*		      from SetPktThreshold.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdma_channel.h"
#include "xbasic_types.h"
#include "xio.h"
#include "xbuf_descriptor.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

#define XDC_SWCR_SG_ENABLE_MASK 0x80000000UL	/* scatter gather enable */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* the following macro copies selected fields of a buffer descriptor to another
 * buffer descriptor, this was provided by the buffer descriptor component but
 * was moved here since it is only used internally to this component and since
 * it does not copy all fields
 */
#define CopyBufferDescriptor(InstancePtr, DestinationPtr)	   \
{								   \
    *((u32 *)DestinationPtr + XBD_CONTROL_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_CONTROL_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_SOURCE_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_SOURCE_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_DESTINATION_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_DESTINATION_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_LENGTH_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_LENGTH_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_STATUS_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_STATUS_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_DEVICE_STATUS_OFFSET) =      \
	*((u32 *)InstancePtr + XBD_DEVICE_STATUS_OFFSET);      \
    *((u32 *)DestinationPtr + XBD_ID_OFFSET) =		       \
	*((u32 *)InstancePtr + XBD_ID_OFFSET);		       \
    *((u32 *)DestinationPtr + XBD_FLAGS_OFFSET) =	       \
	*((u32 *)InstancePtr + XBD_FLAGS_OFFSET);	       \
    *((u32 *)DestinationPtr + XBD_RQSTED_LENGTH_OFFSET) =      \
	*((u32 *)InstancePtr + XBD_RQSTED_LENGTH_OFFSET);      \
}

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_SgStart
*
* DESCRIPTION:
*
* This function starts a scatter gather operation for a scatter gather
* DMA channel.	The first buffer descriptor in the buffer descriptor list
* will be started with the scatter gather operation.  A scatter gather list
* should have previously been created for the DMA channel and buffer
* descriptors put into the scatter gather list such that there are scatter
* operations ready to be performed.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* A status containing XST_SUCCESS if scatter gather was started successfully
* for the DMA channel.
*
* A value of XST_DMA_SG_NO_LIST indicates the scatter gather list has not
* been created.
*
* A value of XST_DMA_SG_LIST_EMPTY indicates scatter gather was not started
* because the scatter gather list of the DMA channel does not contain any
* buffer descriptors that are ready to be processed by the hardware.
*
* A value of XST_DMA_SG_IS_STARTED indicates scatter gather was not started
* because the scatter gather was not stopped, but was already started.
*
* A value of XST_DMA_SG_BD_NOT_COMMITTED indicates the buffer descriptor of
* scatter gather list which was to be started is not committed to the list.
* This status is more likely if this function is being called from an ISR
* and non-ISR processing is putting descriptors into the list.
*
* A value of XST_DMA_SG_NO_DATA indicates that the buffer descriptor of the
* scatter gather list which was to be started had already been used by the
* hardware for a DMA transfer that has been completed.
*
* NOTES:
*
* It is the responsibility of the caller to get all the buffer descriptors
* after performing a stop operation and before performing a start operation.
* If buffer descriptors are not retrieved between stop and start operations,
* buffer descriptors may be processed by the hardware more than once.
*
******************************************************************************/
XStatus
XDmaChannel_SgStart(XDmaChannel * InstancePtr)
{
	u32 Register;
	XBufDescriptor *LastDescriptorPtr;

	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if a scatter gather list has not been created yet, return a status */

	if (InstancePtr->TotalDescriptorCount == 0) {
		return XST_DMA_SG_NO_LIST;
	}

	/* if the scatter gather list exists but is empty then return a status */

	if (XDmaChannel_IsSgListEmpty(InstancePtr)) {
		return XST_DMA_SG_LIST_EMPTY;
	}

	/* if scatter gather is busy for the DMA channel, return a status because
	 * restarting it could lose data
	 */

	Register = XIo_In32(InstancePtr->RegBaseAddress + XDC_DMAS_REG_OFFSET);
	if (Register & XDC_DMASR_SG_BUSY_MASK) {
		return XST_DMA_SG_IS_STARTED;
	}

	/* get the address of the last buffer descriptor which the DMA hardware
	 * finished processing
	 */
	LastDescriptorPtr =
	    (XBufDescriptor *) XIo_In32(InstancePtr->RegBaseAddress +
					XDC_BDA_REG_OFFSET);

	/* setup the first buffer descriptor that will be sent when the scatter
	 * gather channel is enabled, this is only necessary one time since
	 * the BDA register of the channel maintains the last buffer descriptor
	 * processed
	 */
	if (LastDescriptorPtr == NULL) {
		XIo_Out32(InstancePtr->RegBaseAddress + XDC_BDA_REG_OFFSET,
			  (u32) InstancePtr->GetPtr);
	} else {
		XBufDescriptor *NextDescriptorPtr;

		/* get the next descriptor to be started, if the status indicates it
		 * hasn't already been used by the h/w, then it's OK to start it,
		 * s/w sets the status of each descriptor to busy and then h/w clears
		 * the busy when it is complete
		 */
		NextDescriptorPtr =
		    XBufDescriptor_GetNextPtr(LastDescriptorPtr);

		if ((XBufDescriptor_GetStatus(NextDescriptorPtr) &
		     XDC_DMASR_BUSY_MASK) == 0) {
			return XST_DMA_SG_NO_DATA;
		}
		/* don't start the DMA SG channel if the descriptor to be processed
		 * by h/w is to be committed by the s/w, this function can be called
		 * such that it interrupts a thread that was putting into the list
		 */
		if (NextDescriptorPtr == InstancePtr->CommitPtr) {
			return XST_DMA_SG_BD_NOT_COMMITTED;
		}
	}

	/* start the scatter gather operation by clearing the stop bit in the
	 * control register and setting the enable bit in the s/w control register,
	 * both of these are necessary to cause it to start, right now the order of
	 * these statements is important, the software control register should be
	 * set 1st.  The other order can cause the CPU to have a loss of sync
	 * because it cannot read/write the register while the DMA operation is
	 * running
	 */

	Register = XIo_In32(InstancePtr->RegBaseAddress + XDC_SWCR_REG_OFFSET);

	XIo_Out32(InstancePtr->RegBaseAddress + XDC_SWCR_REG_OFFSET,
		  Register | XDC_SWCR_SG_ENABLE_MASK);

	Register = XIo_In32(InstancePtr->RegBaseAddress + XDC_DMAC_REG_OFFSET);

	XIo_Out32(InstancePtr->RegBaseAddress + XDC_DMAC_REG_OFFSET,
		  Register & ~XDC_DMACR_SG_DISABLE_MASK);

	/* indicate the DMA channel scatter gather operation was started
	 * successfully
	 */
	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_SgStop
*
* DESCRIPTION:
*
* This function stops a scatter gather operation for a scatter gather
* DMA channel. This function starts the process of stopping a scatter
* gather operation that is in progress and waits for the stop to be completed.
* Since it waits for the operation to stopped before returning, this function
* could take an amount of time relative to the size of the DMA scatter gather
* operation which is in progress.  The scatter gather list of the DMA channel
* is not modified by this function such that starting the scatter gather
* channel after stopping it will cause it to resume.  This operation is
* considered to be a graceful stop in that the scatter gather operation
* completes the current buffer descriptor before stopping.
*
* If the interrupt is enabled, an interrupt will be generated when the
* operation is stopped and the caller is responsible for handling the
* interrupt.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* BufDescriptorPtr is also a return value which contains a pointer to the
* buffer descriptor which the scatter gather operation completed when it
* was stopped.
*
* RETURN VALUE:
*
* A status containing XST_SUCCESS if scatter gather was stopped successfully
* for the DMA channel.
*
* A value of XST_DMA_SG_IS_STOPPED indicates scatter gather was not stoppped
* because the scatter gather is not started, but was already stopped.
*
* BufDescriptorPtr contains a pointer to the buffer descriptor which was
* completed when the operation was stopped.
*
* NOTES:
*
* This function implements a loop which polls the hardware for an infinite
* amount of time. If the hardware is not operating correctly, this function
* may never return.
*
******************************************************************************/
XStatus
XDmaChannel_SgStop(XDmaChannel * InstancePtr,
		   XBufDescriptor ** BufDescriptorPtr)
{
	u32 Register;

	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufDescriptorPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the contents of the software control register, if scatter gather is not
	 * enabled (started), then return a status because the disable acknowledge
	 * would not be generated
	 */
	Register = XIo_In32(InstancePtr->RegBaseAddress + XDC_SWCR_REG_OFFSET);

	if ((Register & XDC_SWCR_SG_ENABLE_MASK) == 0) {
		return XST_DMA_SG_IS_STOPPED;
	}

	/* Ensure the interrupt status for the scatter gather is cleared such
	 * that this function will wait til the disable has occurred, writing
	 * a 1 to only that bit in the register will clear only it
	 */
	XIo_Out32(InstancePtr->RegBaseAddress + XDC_IS_REG_OFFSET,
		  XDC_IXR_SG_DISABLE_ACK_MASK);

	/* disable scatter gather by writing to the software control register
	 * without modifying any other bits of the register
	 */
	XIo_Out32(InstancePtr->RegBaseAddress + XDC_SWCR_REG_OFFSET,
		  Register & ~XDC_SWCR_SG_ENABLE_MASK);

	/* scatter gather does not disable immediately, but after the current
	 * buffer descriptor is complete, so wait for the DMA channel to indicate
	 * the disable is complete
	 */
	do {
		Register =
		    XIo_In32(InstancePtr->RegBaseAddress + XDC_IS_REG_OFFSET);
	} while ((Register & XDC_IXR_SG_DISABLE_ACK_MASK) == 0);

	/* Ensure the interrupt status for the scatter gather disable is cleared,
	 * writing a 1 to only that bit in the register will clear only it
	 */
	XIo_Out32(InstancePtr->RegBaseAddress + XDC_IS_REG_OFFSET,
		  XDC_IXR_SG_DISABLE_ACK_MASK);

	/* set the specified buffer descriptor pointer to point to the buffer
	 * descriptor that the scatter gather DMA channel was processing
	 */
	*BufDescriptorPtr =
	    (XBufDescriptor *) XIo_In32(InstancePtr->RegBaseAddress +
					XDC_BDA_REG_OFFSET);

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_CreateSgList
*
* DESCRIPTION:
*
* This function creates a scatter gather list in the DMA channel.  A scatter
* gather list consists of a list of buffer descriptors that are available to
* be used for scatter gather operations.  Buffer descriptors are put into the
* list to request a scatter gather operation to be performed.
*
* A number of buffer descriptors are created from the specified memory and put
* into a buffer descriptor list as empty buffer descriptors. This function must
* be called before non-empty buffer descriptors may be put into the DMA channel
* to request scatter gather operations.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* MemoryPtr contains a pointer to the memory which is to be used for buffer
* descriptors and must not be cached.
*
* ByteCount contains the number of bytes for the specified memory to be used
* for buffer descriptors.
*
* RETURN VALUE:
*
* A status contains XST_SUCCESS if the scatter gather list was successfully
* created.
*
* A value of XST_DMA_SG_LIST_EXISTS indicates that the scatter gather list
* was not created because the list has already been created.
*
* NOTES:
*
* None.
*
******************************************************************************/
XStatus
XDmaChannel_CreateSgList(XDmaChannel * InstancePtr,
			 u32 * MemoryPtr, u32 ByteCount)
{
	XBufDescriptor *BufferDescriptorPtr = (XBufDescriptor *) MemoryPtr;
	XBufDescriptor *PreviousDescriptorPtr = NULL;
	XBufDescriptor *StartOfListPtr = BufferDescriptorPtr;
	u32 UsedByteCount;

	/* assert to verify valid input arguments, alignment for those
	 * arguments that have alignment restrictions, and at least enough
	 * memory for one buffer descriptor
	 */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(MemoryPtr != NULL);
	XASSERT_NONVOID(((u32) MemoryPtr & 3) == 0);
	XASSERT_NONVOID(ByteCount != 0);
	XASSERT_NONVOID(ByteCount >= sizeof (XBufDescriptor));
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if the scatter gather list has already been created, then return
	 * with a status
	 */
	if (InstancePtr->TotalDescriptorCount != 0) {
		return XST_DMA_SG_LIST_EXISTS;
	}

	/* loop thru the specified memory block and create as many buffer
	 * descriptors as possible putting each into the list which is
	 * implemented as a ring buffer, make sure not to use any memory which
	 * is not large enough for a complete buffer descriptor
	 */
	UsedByteCount = 0;
	while ((UsedByteCount + sizeof (XBufDescriptor)) <= ByteCount) {
		/* setup a pointer to the next buffer descriptor in the memory and
		 * update # of used bytes to know when all of memory is used
		 */
		BufferDescriptorPtr = (XBufDescriptor *) ((u32) MemoryPtr +
							  UsedByteCount);

		/* initialize the new buffer descriptor such that it doesn't contain
		 * garbage which could be used by the DMA hardware
		 */
		XBufDescriptor_Initialize(BufferDescriptorPtr);

		/* if this is not the first buffer descriptor to be created,
		 * then link it to the last created buffer descriptor
		 */
		if (PreviousDescriptorPtr != NULL) {
			XBufDescriptor_SetNextPtr(PreviousDescriptorPtr,
						  BufferDescriptorPtr);
		}

		/* always keep a pointer to the last created buffer descriptor such
		 * that they can be linked together in the ring buffer
		 */
		PreviousDescriptorPtr = BufferDescriptorPtr;

		/* keep a count of the number of descriptors in the list to allow
		 * error processing to be performed
		 */
		InstancePtr->TotalDescriptorCount++;

		UsedByteCount += sizeof (XBufDescriptor);
	}

	/* connect the last buffer descriptor created and inserted in the list
	 * to the first such that a ring buffer is created
	 */
	XBufDescriptor_SetNextPtr(BufferDescriptorPtr, StartOfListPtr);

	/* initialize the ring buffer to indicate that there are no
	 * buffer descriptors in the list which point to valid data buffers
	 */
	InstancePtr->PutPtr = BufferDescriptorPtr;
	InstancePtr->GetPtr = BufferDescriptorPtr;
	InstancePtr->CommitPtr = NULL;
	InstancePtr->LastPtr = BufferDescriptorPtr;
	InstancePtr->ActiveDescriptorCount = 0;

	/* indicate the scatter gather list was successfully created */

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_IsSgListEmpty
*
* DESCRIPTION:
*
* This function determines if the scatter gather list of a DMA channel is
* empty with regard to buffer descriptors which are pointing to buffers to be
* used for scatter gather operations.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* A value of TRUE if the scatter gather list is empty, otherwise a value of
* FALSE.
*
* NOTES:
*
* None.
*
******************************************************************************/
u32
XDmaChannel_IsSgListEmpty(XDmaChannel * InstancePtr)
{
	/* assert to verify valid input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if the number of descriptors which are being used in the list is zero
	 * then the list is empty
	 */
	return (InstancePtr->ActiveDescriptorCount == 0);
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_PutDescriptor
*
* DESCRIPTION:
*
* This function puts a buffer descriptor into the DMA channel scatter
* gather list. A DMA channel maintains a list of buffer descriptors which are
* to be processed.  This function puts the specified buffer descriptor
* at the next location in the list.  Note that since the list is already intact,
* the information in the parameter is copied into the list (rather than modify
* list pointers on the fly).
*
* After buffer descriptors are put into the list, they must also be committed
* by calling another function.	This allows multiple buffer descriptors which
* span a single packet to be put into the list while preventing the hardware
* from starting the first buffer descriptor of the packet.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* BufferDescriptorPtr is a pointer to the buffer descriptor to be put into
* the next available location of the scatter gather list.
*
* RETURN VALUE:
*
* A status which indicates XST_SUCCESS if the buffer descriptor was
* successfully put into the scatter gather list.
*
* A value of XST_DMA_SG_NO_LIST indicates the scatter gather list has not
* been created.
*
* A value of XST_DMA_SG_LIST_FULL indicates the buffer descriptor was not
* put into the list because the list was full.
*
* A value of XST_DMA_SG_BD_LOCKED indicates the buffer descriptor was not
* put into the list because the buffer descriptor in the list which is to
* be overwritten was locked.  A locked buffer descriptor indicates the higher
* layered software is still using the buffer descriptor.
*
* NOTES:
*
* It is necessary to create a scatter gather list for a DMA channel before
* putting buffer descriptors into it.
*
******************************************************************************/
XStatus
XDmaChannel_PutDescriptor(XDmaChannel * InstancePtr,
			  XBufDescriptor * BufferDescriptorPtr)
{
	u32 Control;

	/* assert to verify valid input arguments and alignment for those
	 * arguments that have alignment restrictions
	 */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufferDescriptorPtr != NULL);
	XASSERT_NONVOID(((u32) BufferDescriptorPtr & 3) == 0);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if a scatter gather list has not been created yet, return a status */

	if (InstancePtr->TotalDescriptorCount == 0) {
		return XST_DMA_SG_NO_LIST;
	}

	/* if the list is full because all descriptors are pointing to valid
	 * buffers, then indicate an error, this code assumes no list or an
	 * empty list is detected above
	 */
	if (InstancePtr->ActiveDescriptorCount ==
	    InstancePtr->TotalDescriptorCount) {
		return XST_DMA_SG_LIST_FULL;
	}

	/* if the buffer descriptor in the list which is to be overwritten is
	 * locked, then don't overwrite it and return a status
	 */
	if (XBufDescriptor_IsLocked(InstancePtr->PutPtr)) {
		return XST_DMA_SG_BD_LOCKED;
	}

	/* set the scatter gather stop bit in the control word of the descriptor
	 * to cause the h/w to stop after it processes this descriptor since it
	 * will be the last in the list
	 */
	Control = XBufDescriptor_GetControl(BufferDescriptorPtr);
	XBufDescriptor_SetControl(BufferDescriptorPtr,
				  Control | XDC_DMACR_SG_DISABLE_MASK);

	/* set both statuses in the descriptor so we tell if they are updated with
	 * the status of the transfer, the hardware should change the busy in the
	 * DMA status to be false when it completes
	 */
	XBufDescriptor_SetStatus(BufferDescriptorPtr, XDC_DMASR_BUSY_MASK);
	XBufDescriptor_SetDeviceStatus(BufferDescriptorPtr, 0);

	/* copy the descriptor into the next position in the list so it's ready to
	 * be used by the h/w, this assumes the descriptor in the list prior to this
	 * one still has the stop bit in the control word set such that the h/w
	 * use this one yet
	 */
	CopyBufferDescriptor(BufferDescriptorPtr, InstancePtr->PutPtr);

	/* only the last in the list and the one to be committed have scatter gather
	 * disabled in the control word, a commit requires only one descriptor
	 * to be changed, when # of descriptors to commit > 2 all others except the
	 * 1st and last have scatter gather enabled
	 */
	if ((InstancePtr->CommitPtr != InstancePtr->LastPtr) &&
	    (InstancePtr->CommitPtr != NULL)) {
		Control = XBufDescriptor_GetControl(InstancePtr->LastPtr);
		XBufDescriptor_SetControl(InstancePtr->LastPtr,
					  Control & ~XDC_DMACR_SG_DISABLE_MASK);
	}

	/* update the list data based upon putting a descriptor into the list,
	 * these operations must be last
	 */
	InstancePtr->ActiveDescriptorCount++;

	/* only update the commit pointer if it is not already active, this allows
	 * it to be deactivated after every commit such that a single descriptor
	 * which is committed does not appear to be waiting to be committed
	 */
	if (InstancePtr->CommitPtr == NULL) {
		InstancePtr->CommitPtr = InstancePtr->LastPtr;
	}

	/* these updates MUST BE LAST after the commit pointer update in order for
	 * the commit pointer to track the correct descriptor to be committed
	 */
	InstancePtr->LastPtr = InstancePtr->PutPtr;
	InstancePtr->PutPtr = XBufDescriptor_GetNextPtr(InstancePtr->PutPtr);

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_CommitPuts
*
* DESCRIPTION:
*
* This function commits the buffer descriptors which have been put into the
* scatter list for the DMA channel since the last commit operation was
* performed.  This enables the calling functions to put several buffer
* descriptors into the list (e.g.,a packet's worth) before allowing the scatter
* gather operations to start.  This prevents the DMA channel hardware from
* starting to use the buffer descriptors in the list before they are ready
* to be used (multiple buffer descriptors for a single packet).
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* A status indicating XST_SUCCESS if the buffer descriptors of the list were
* successfully committed.
*
* A value of XST_DMA_SG_NOTHING_TO_COMMIT indicates that the buffer descriptors
* were not committed because there was nothing to commit in the list.  All the
* buffer descriptors which are in the list are commited.
*
* NOTES:
*
* None.
*
******************************************************************************/
XStatus
XDmaChannel_CommitPuts(XDmaChannel * InstancePtr)
{
	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if the buffer descriptor to be committed is already committed or
	 * the list is empty (none have been put in), then indicate an error
	 */
	if ((InstancePtr->CommitPtr == NULL) ||
	    XDmaChannel_IsSgListEmpty(InstancePtr)) {
		return XST_DMA_SG_NOTHING_TO_COMMIT;
	}

	/* last descriptor in the list must have scatter gather disabled so the end
	 * of the list is hit by h/w, if descriptor to commit is not last in list,
	 * commit descriptors by enabling scatter gather in the descriptor
	 */
	if (InstancePtr->CommitPtr != InstancePtr->LastPtr) {
		u32 Control;

		Control = XBufDescriptor_GetControl(InstancePtr->CommitPtr);
		XBufDescriptor_SetControl(InstancePtr->CommitPtr, Control &
					  ~XDC_DMACR_SG_DISABLE_MASK);
	}
	/* Update the commit pointer to indicate that there is nothing to be
	 * committed, this state is used by start processing to know that the
	 * buffer descriptor to start is not waiting to be committed
	 */
	InstancePtr->CommitPtr = NULL;

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_GetDescriptor
*
* DESCRIPTION:
*
* This function gets a buffer descriptor from the scatter gather list of the
* DMA channel. The buffer descriptor is retrieved from the scatter gather list
* and the scatter gather list is updated to not include the retrieved buffer
* descriptor.  This is typically done after a scatter gather operation
* completes indicating that a data buffer has been successfully sent or data
* has been received into the data buffer. The purpose of this function is to
* allow the device using the scatter gather operation to get the results of the
* operation.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* BufDescriptorPtr is a pointer to a pointer to the buffer descriptor which
* was retrieved from the list.	The buffer descriptor is not really removed
* from the list, but it is changed to a state such that the hardware will not
* use it again until it is put into the scatter gather list of the DMA channel.
*
* RETURN VALUE:
*
* A status indicating XST_SUCCESS if a buffer descriptor was retrieved from
* the scatter gather list of the DMA channel.
*
* A value of XST_DMA_SG_NO_LIST indicates the scatter gather list has not
* been created.
*
* A value of XST_DMA_SG_LIST_EMPTY indicates no buffer descriptor was
* retrieved from the list because there are no buffer descriptors to be
* processed in the list.
*
* BufDescriptorPtr is updated to point to the buffer descriptor which was
* retrieved from the list if the status indicates success.
*
* NOTES:
*
* None.
*
******************************************************************************/
XStatus
XDmaChannel_GetDescriptor(XDmaChannel * InstancePtr,
			  XBufDescriptor ** BufDescriptorPtr)
{
	u32 Control;

	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufDescriptorPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if a scatter gather list has not been created yet, return a status */

	if (InstancePtr->TotalDescriptorCount == 0) {
		return XST_DMA_SG_NO_LIST;
	}

	/* if the buffer descriptor list is empty, then indicate an error */

	if (XDmaChannel_IsSgListEmpty(InstancePtr)) {
		return XST_DMA_SG_LIST_EMPTY;
	}

	/* retrieve the next buffer descriptor which is ready to be processed from
	 * the buffer descriptor list for the DMA channel, set the control word
	 * such that hardware will stop after the descriptor has been processed
	 */
	Control = XBufDescriptor_GetControl(InstancePtr->GetPtr);
	XBufDescriptor_SetControl(InstancePtr->GetPtr,
				  Control | XDC_DMACR_SG_DISABLE_MASK);

	/* set the input argument, which is also an output, to point to the
	 * buffer descriptor which is to be retrieved from the list
	 */
	*BufDescriptorPtr = InstancePtr->GetPtr;

	/* update the pointer of the DMA channel to reflect the buffer descriptor
	 * was retrieved from the list by setting it to the next buffer descriptor
	 * in the list and indicate one less descriptor in the list now
	 */
	InstancePtr->GetPtr = XBufDescriptor_GetNextPtr(InstancePtr->GetPtr);
	InstancePtr->ActiveDescriptorCount--;

	return XST_SUCCESS;
}

/*********************** Interrupt Collescing Functions **********************/

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_GetPktCount
*
* DESCRIPTION:
*
* This function returns the value of the unserviced packet count register of
* the DMA channel.  This count represents the number of packets that have been
* sent or received by the hardware, but not processed by software.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* The unserviced packet counter register contents for the DMA channel.
*
* NOTES:
*
* None.
*
******************************************************************************/
u32
XDmaChannel_GetPktCount(XDmaChannel * InstancePtr)
{
	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the unserviced packet count from the register and return it */

	return XIo_In32(InstancePtr->RegBaseAddress + XDC_UPC_REG_OFFSET);
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_DecrementPktCount
*
* DESCRIPTION:
*
* This function decrements the value of the unserviced packet count register.
* This informs the hardware that the software has processed a packet.  The
* unserviced packet count register may only be decremented by one in the
* hardware.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
void
XDmaChannel_DecrementPktCount(XDmaChannel * InstancePtr)
{
	u32 Register;

	/* assert to verify input arguments */

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* if the unserviced packet count register can be decremented (rather
	 * than rolling over) decrement it by writing a 1 to the register,
	 * this is the only valid write to the register as it serves as an
	 * acknowledge that a packet was handled by the software
	 */
	Register = XIo_In32(InstancePtr->RegBaseAddress + XDC_UPC_REG_OFFSET);
	if (Register > 0) {
		XIo_Out32(InstancePtr->RegBaseAddress + XDC_UPC_REG_OFFSET,
			  1UL);
	}
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_SetPktThreshold
*
* DESCRIPTION:
*
* This function sets the value of the packet count threshold register of the
* DMA channel.	It reflects the number of packets that must be sent or
* received before generating an interrupt.  This value helps implement
* a concept called "interrupt coalescing", which is used to reduce the number
* of interrupts from devices with high data rates.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* Threshold is the value that is written to the threshold register of the
* DMA channel.
*
* RETURN VALUE:
*
* A status containing XST_SUCCESS if the packet count threshold was
* successfully set.
*
* NOTES:
*
* The packet threshold could be set to larger than the number of descriptors
* allocated to the DMA channel. In this case, the wait bound will take over
* and always indicate data arrival. There was a check in this function that
* returned an error if the treshold was larger than the number of descriptors,
* but that was removed because users would then have to set the threshold
* only after they set descriptor space, which is an order dependency that
* caused confustion.
*
******************************************************************************/
XStatus
XDmaChannel_SetPktThreshold(XDmaChannel * InstancePtr, u8 Threshold)
{
	/* assert to verify input arguments, don't assert the threshold since
	 * it's range is unknown
	 */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* set the packet count threshold in the register such that an interrupt
	 * may be generated, if enabled, when the packet count threshold is
	 * reached or exceeded
	 */
	XIo_Out32(InstancePtr->RegBaseAddress + XDC_PCT_REG_OFFSET,
		  (u32) Threshold);

	/* indicate the packet count threshold was successfully set */

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_GetPktThreshold
*
* DESCRIPTION:
*
* This function gets the value of the packet count threshold register of the
* DMA channel.	This value reflects the number of packets that must be sent or
* received before generating an interrupt.  This value helps implement a concept
* called "interrupt coalescing", which is used to reduce the number of
* interrupts from devices with high data rates.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* The packet threshold register contents for the DMA channel and is a value in
* the range 0 - 1023.  A value of 0 indicates the packet wait bound timer is
* disabled.
*
* NOTES:
*
* None.
*
******************************************************************************/
u8
XDmaChannel_GetPktThreshold(XDmaChannel * InstancePtr)
{
	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the packet count threshold from the register and return it,
	 * since only 8 bits are used, cast it to return only those bits */

	return (u8) XIo_In32(InstancePtr->RegBaseAddress + XDC_PCT_REG_OFFSET);
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_SetPktWaitBound
*
* DESCRIPTION:
*
* This function sets the value of the packet wait bound register of the
* DMA channel.	This value reflects the timer value used to trigger an
* interrupt when not enough packets have been received to reach the packet
* count threshold.
*
* The timer is in millisecond units with +/- 33% accuracy.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* WaitBound is the value, in milliseconds, to be stored in the wait bound
* register of the DMA channel and is a value in the range 0  - 1023.  A value
* of 0 disables the packet wait bound timer.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
void
XDmaChannel_SetPktWaitBound(XDmaChannel * InstancePtr, u32 WaitBound)
{
	/* assert to verify input arguments */

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(WaitBound < 1024);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* set the packet wait bound in the register such that interrupt may be
	 * generated, if enabled, when packets have not been handled for a specific
	 * amount of time
	 */
	XIo_Out32(InstancePtr->RegBaseAddress + XDC_PWB_REG_OFFSET, WaitBound);
}

/******************************************************************************
*
* FUNCTION:
*
* XDmaChannel_GetPktWaitBound
*
* DESCRIPTION:
*
* This function gets the value of the packet wait bound register of the
* DMA channel.	This value contains the timer value used to trigger an
* interrupt when not enough packets have been received to reach the packet
* count threshold.
*
* The timer is in millisecond units with +/- 33% accuracy.
*
* ARGUMENTS:
*
* InstancePtr contains a pointer to the DMA channel to operate on.  The DMA
* channel should be configured to use scatter gather in order for this function
* to be called.
*
* RETURN VALUE:
*
* The packet wait bound register contents for the DMA channel.
*
* NOTES:
*
* None.
*
******************************************************************************/
u32
XDmaChannel_GetPktWaitBound(XDmaChannel * InstancePtr)
{
	/* assert to verify input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the packet wait bound from the register and return it */

	return XIo_In32(InstancePtr->RegBaseAddress + XDC_PWB_REG_OFFSET);
}
