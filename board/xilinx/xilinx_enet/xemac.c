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
* @file xemac.c
*
* The XEmac driver. Functions in this file are the minimum required functions
* for this driver. See xemac.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rpm  07/31/01 First release
* 1.00b rpm  02/20/02 Repartitioned files and functions
* 1.00b rpm  07/23/02 Removed the PHY reset from Initialize()
* 1.00b rmm  09/23/02 Removed commented code in Initialize(). Recycled as
*                     XEmac_mPhyReset macro in xemac_l.h.
* 1.00c rpm  12/05/02 New version includes support for simple DMA
* 1.00c rpm  12/12/02 Changed location of IsStarted assignment in XEmac_Start
*                     to be sure the flag is set before the device and
*                     interrupts are enabled.
* 1.00c rpm  02/03/03 SelfTest was not clearing polled mode. Take driver out
*                     of polled mode in XEmac_Reset() to fix this problem.
* 1.00c rmm  05/13/03 Fixed diab compiler warnings relating to asserts.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xemac_i.h"
#include "xio.h"
#include "xipif_v1_23_b.h"	/* Uses v1.23b of the IPIF */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static XStatus ConfigureDma(XEmac * InstancePtr);
static XStatus ConfigureFifo(XEmac * InstancePtr);
static void StubFifoHandler(void *CallBackRef);
static void StubErrorHandler(void *CallBackRef, XStatus ErrorCode);
static void StubSgHandler(void *CallBackRef, XBufDescriptor * BdPtr,
			  u32 NumBds);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initialize a specific XEmac instance/driver.  The initialization entails:
* - Initialize fields of the XEmac structure
* - Clear the Ethernet statistics for this device
* - Initialize the IPIF component with its register base address
* - Configure the FIFO components with their register base addresses.
* - If the device is configured with DMA, configure the DMA channel components
*   with their register base addresses. At some later time, memory pools for
*   the scatter-gather descriptor lists may be passed to the driver.
* - Reset the Ethernet MAC
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param DeviceId is the unique id of the device controlled by this XEmac
*        instance.  Passing in a device id associates the generic XEmac
*        instance to a specific device, as chosen by the caller or application
*        developer.
*
* @return
*
* - XST_SUCCESS if initialization was successful
* - XST_DEVICE_IS_STARTED if the device has already been started
* - XST_DEVICE_NOT_FOUND if device configuration information was not found for
*   a device with the supplied device ID.
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XEmac_Initialize(XEmac * InstancePtr, u16 DeviceId)
{
	XStatus Result;
	XEmac_Config *ConfigPtr;	/* configuration information */

	XASSERT_NONVOID(InstancePtr != NULL);

	/*
	 * If the device is started, disallow the initialize and return a status
	 * indicating it is started.  This allows the user to stop the device
	 * and reinitialize, but prevents a user from inadvertently initializing
	 */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	ConfigPtr = XEmac_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Set some default values
	 */
	InstancePtr->IsReady = 0;
	InstancePtr->IsStarted = 0;
	InstancePtr->IpIfDmaConfig = ConfigPtr->IpIfDmaConfig;
	InstancePtr->HasMii = ConfigPtr->HasMii;
	InstancePtr->HasMulticastHash = FALSE;

	/* Always default polled to false, let user configure this mode */
	InstancePtr->IsPolled = FALSE;
	InstancePtr->FifoRecvHandler = StubFifoHandler;
	InstancePtr->FifoSendHandler = StubFifoHandler;
	InstancePtr->ErrorHandler = StubErrorHandler;
	InstancePtr->SgRecvHandler = StubSgHandler;
	InstancePtr->SgSendHandler = StubSgHandler;

	/*
	 * Clear the statistics for this driver
	 */
	XEmac_mClearStruct((u8 *) & InstancePtr->Stats, sizeof (XEmac_Stats));

	/*
	 * Initialize the device register base addresses
	 */
	InstancePtr->BaseAddress = ConfigPtr->BaseAddress;

	/*
	 * Configure the send and receive FIFOs in the MAC
	 */
	Result = ConfigureFifo(InstancePtr);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/*
	 * If the device is configured for DMA, configure the send and receive DMA
	 * channels in the MAC.
	 */
	if (XEmac_mIsDma(InstancePtr)) {
		Result = ConfigureDma(InstancePtr);
		if (Result != XST_SUCCESS) {
			return Result;
		}
	}

	/*
	 * Indicate the component is now ready to use. Note that this is done before
	 * we reset the device and the PHY below, which may seem a bit odd. The
	 * choice was made to move it here rather than remove the asserts in various
	 * functions (e.g., Reset() and all functions that it calls).  Applications
	 * that use multiple threads, one to initialize the XEmac driver and one
	 * waiting on the IsReady condition could have a problem with this sequence.
	 */
	InstancePtr->IsReady = XCOMPONENT_IS_READY;

	/*
	 * Reset the MAC to get it into its initial state. It is expected that
	 * device configuration by the user will take place after this
	 * initialization is done, but before the device is started.
	 */
	XEmac_Reset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Start the Ethernet controller as follows:
*   - If not in polled mode
*       - Set the internal interrupt enable registers appropriately
*       - Enable interrupts within the device itself. Note that connection of
*         the driver's interrupt handler to the interrupt source (typically
*         done using the interrupt controller component) is done by the higher
*         layer software.
*       - If the device is configured with scatter-gather DMA, start the DMA
*         channels if the descriptor lists are not empty
*   - Enable the transmitter
*   - Enable the receiver
*
* The PHY is enabled after driver initialization. We assume the upper layer
* software has configured it and the EMAC appropriately before this function
* is called.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* - XST_SUCCESS if the device was started successfully
* - XST_NO_CALLBACK if a callback function has not yet been registered using
*   the SetxxxHandler function. This is required if in interrupt mode.
* - XST_DEVICE_IS_STARTED if the device is already started
* - XST_DMA_SG_NO_LIST if configured for scatter-gather DMA and a descriptor
*   list has not yet been created for the send or receive channel.
*
* @note
*
* The driver tries to match the hardware configuration. So if the hardware
* is configured with scatter-gather DMA, the driver expects to start the
* scatter-gather channels and expects that the user has set up the buffer
* descriptor lists already. If the user expects to use the driver in a mode
* different than how the hardware is configured, the user should modify the
* configuration table to reflect the mode to be used. Modifying the config
* table is a workaround for now until we get some experience with how users
* are intending to use the hardware in its different configurations. For
* example, if the hardware is built with scatter-gather DMA but the user is
* intending to use only simple DMA, the user either needs to modify the config
* table as a workaround or rebuild the hardware with only simple DMA.
*
* This function makes use of internal resources that are shared between the
* Start, Stop, and SetOptions functions. So if one task might be setting device
* options while another is trying to start the device, the user is required to
* provide protection of this shared data (typically using a semaphore).
*
******************************************************************************/
XStatus
XEmac_Start(XEmac * InstancePtr)
{
	u32 ControlReg;
	XStatus Result;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * If it is already started, return a status indicating so
	 */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * If not polled, enable interrupts
	 */
	if (!InstancePtr->IsPolled) {
		/*
		 * Verify that the callbacks have been registered, then enable
		 * interrupts
		 */
		if (XEmac_mIsSgDma(InstancePtr)) {
			if ((InstancePtr->SgRecvHandler == StubSgHandler) ||
			    (InstancePtr->SgSendHandler == StubSgHandler)) {
				return XST_NO_CALLBACK;
			}

			/* Enable IPIF interrupts */
			XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
					      XEM_IPIF_DMA_DFT_MASK |
					      XIIF_V123B_ERROR_MASK);
			XIIF_V123B_WRITE_IIER(InstancePtr->BaseAddress,
					      XEM_EIR_DFT_SG_MASK);

			/* Enable scatter-gather DMA interrupts */
			XDmaChannel_SetIntrEnable(&InstancePtr->RecvChannel,
						  XEM_DMA_SG_INTR_MASK);
			XDmaChannel_SetIntrEnable(&InstancePtr->SendChannel,
						  XEM_DMA_SG_INTR_MASK);
		} else {
			if ((InstancePtr->FifoRecvHandler == StubFifoHandler) ||
			    (InstancePtr->FifoSendHandler == StubFifoHandler)) {
				return XST_NO_CALLBACK;
			}

			/* Enable IPIF interrupts (used by simple DMA also) */
			XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
					      XEM_IPIF_FIFO_DFT_MASK |
					      XIIF_V123B_ERROR_MASK);
			XIIF_V123B_WRITE_IIER(InstancePtr->BaseAddress,
					      XEM_EIR_DFT_FIFO_MASK);
		}

		/* Enable the global IPIF interrupt output */
		XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
	}

	/*
	 * Indicate that the device is started before we enable the transmitter
	 * or receiver. This needs to be done before because as soon as the
	 * receiver is enabled we may get an interrupt, and there are functions
	 * in the interrupt handling path that rely on the IsStarted flag.
	 */
	InstancePtr->IsStarted = XCOMPONENT_IS_STARTED;

	/*
	 * Enable the transmitter, and receiver (do a read/modify/write to preserve
	 * current settings). There is no critical section here since this register
	 * is not modified during interrupt context.
	 */
	ControlReg = XIo_In32(InstancePtr->BaseAddress + XEM_ECR_OFFSET);
	ControlReg &= ~(XEM_ECR_XMIT_RESET_MASK | XEM_ECR_RECV_RESET_MASK);
	ControlReg |= (XEM_ECR_XMIT_ENABLE_MASK | XEM_ECR_RECV_ENABLE_MASK);

	XIo_Out32(InstancePtr->BaseAddress + XEM_ECR_OFFSET, ControlReg);

	/*
	 * If configured with scatter-gather DMA and not polled, restart the
	 * DMA channels in case there are buffers ready to be sent or received into.
	 * The DMA SgStart function uses data that can be modified during interrupt
	 * context, so a critical section is required here.
	 */
	if ((XEmac_mIsSgDma(InstancePtr)) && (!InstancePtr->IsPolled)) {
		XIIF_V123B_GINTR_DISABLE(InstancePtr->BaseAddress);

		/*
		 * The only error we care about is if the list has not yet been
		 * created, or on receive, if no buffer descriptors have been
		 * added yet (the list is empty). Other errors are benign at this point.
		 */
		Result = XDmaChannel_SgStart(&InstancePtr->RecvChannel);
		if ((Result == XST_DMA_SG_NO_LIST)
		    || (Result == XST_DMA_SG_LIST_EMPTY)) {
			XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
			return Result;
		}

		Result = XDmaChannel_SgStart(&InstancePtr->SendChannel);
		if (Result == XST_DMA_SG_NO_LIST) {
			XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
			return Result;
		}

		XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Stop the Ethernet MAC as follows:
*   - If the device is configured with scatter-gather DMA, stop the DMA
*     channels (wait for acknowledgment of stop)
*   - Disable the transmitter and receiver
*   - Disable interrupts if not in polled mode (the higher layer software is
*     responsible for disabling interrupts at the interrupt controller)
*
* The PHY is left enabled after a Stop is called.
*
* If the device is configured for scatter-gather DMA, the DMA engine stops at
* the next buffer descriptor in its list. The remaining descriptors in the list
* are not removed, so anything in the list will be transmitted or received when
* the device is restarted. The side effect of doing this is that the last
* buffer descriptor processed by the DMA engine before stopping may not be the
* last descriptor in the Ethernet frame. So when the device is restarted, a
* partial frame (i.e., a bad frame) may be transmitted/received. This is only a
* concern if a frame can span multiple buffer descriptors, which is dependent
* on the size of the network buffers.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* - XST_SUCCESS if the device was stopped successfully
* - XST_DEVICE_IS_STOPPED if the device is already stopped
*
* @note
*
* This function makes use of internal resources that are shared between the
* Start, Stop, and SetOptions functions. So if one task might be setting device
* options while another is trying to start the device, the user is required to
* provide protection of this shared data (typically using a semaphore).
*
******************************************************************************/
XStatus
XEmac_Stop(XEmac * InstancePtr)
{
	u32 ControlReg;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * If the device is already stopped, do nothing but return a status
	 * indicating so
	 */
	if (InstancePtr->IsStarted != XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STOPPED;
	}

	/*
	 * If configured for scatter-gather DMA, stop the DMA channels. Ignore
	 * the XST_DMA_SG_IS_STOPPED return code. There is a critical section
	 * here between SgStart and SgStop, and SgStart can be called in interrupt
	 * context, so disable interrupts while calling SgStop.
	 */
	if (XEmac_mIsSgDma(InstancePtr)) {
		XBufDescriptor *BdTemp;	/* temporary descriptor pointer */

		XIIF_V123B_GINTR_DISABLE(InstancePtr->BaseAddress);

		(void) XDmaChannel_SgStop(&InstancePtr->SendChannel, &BdTemp);
		(void) XDmaChannel_SgStop(&InstancePtr->RecvChannel, &BdTemp);

		XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
	}

	/*
	 * Disable the transmitter and receiver. There is no critical section
	 * here since this register is not modified during interrupt context.
	 */
	ControlReg = XIo_In32(InstancePtr->BaseAddress + XEM_ECR_OFFSET);
	ControlReg &= ~(XEM_ECR_XMIT_ENABLE_MASK | XEM_ECR_RECV_ENABLE_MASK);
	XIo_Out32(InstancePtr->BaseAddress + XEM_ECR_OFFSET, ControlReg);

	/*
	 * If not in polled mode, disable interrupts for IPIF (includes MAC and
	 * DMAs)
	 */
	if (!InstancePtr->IsPolled) {
		XIIF_V123B_GINTR_DISABLE(InstancePtr->BaseAddress);
	}

	InstancePtr->IsStarted = 0;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Reset the Ethernet MAC. This is a graceful reset in that the device is stopped
* first. Resets the DMA channels, the FIFOs, the transmitter, and the receiver.
* The PHY is not reset. Any frames in the scatter-gather descriptor lists will
* remain in the lists. The side effect of doing this is that after a reset and
* following a restart of the device, frames that were in the list before the
* reset may be transmitted or received. Reset must only be called after the
* driver has been initialized.
*
* The driver is also taken out of polled mode if polled mode was set. The user
* is responsbile for re-configuring the driver into polled mode after the
* reset if desired.
*
* The configuration after this reset is as follows:
*   - Half duplex
*   - Disabled transmitter and receiver
*   - Enabled PHY (the PHY is not reset)
*   - MAC transmitter does pad insertion, FCS insertion, and source address
*     overwrite.
*   - MAC receiver does not strip padding or FCS
*   - Interframe Gap as recommended by IEEE Std. 802.3 (96 bit times)
*   - Unicast addressing enabled
*   - Broadcast addressing enabled
*   - Multicast addressing disabled (addresses are preserved)
*   - Promiscuous addressing disabled
*   - Default packet threshold and packet wait bound register values for
*     scatter-gather DMA operation
*   - MAC address of all zeros
*   - Non-polled mode
*
* The upper layer software is responsible for re-configuring (if necessary)
* and restarting the MAC after the reset. Note that the PHY is not reset. PHY
* control is left to the upper layer software. Note also that driver statistics
* are not cleared on reset. It is up to the upper layer software to clear the
* statistics if needed.
*
* When a reset is required due to an internal error, the driver notifies the
* upper layer software of this need through the ErrorHandler callback and
* specific status codes.  The upper layer software is responsible for calling
* this Reset function and then re-configuring the device.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* None.
*
* @note
*
* None.
*
* @internal
*
* The reset is accomplished by setting the IPIF reset register.  This takes
* care of resetting all hardware blocks, including the MAC.
*
******************************************************************************/
void
XEmac_Reset(XEmac * InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Stop the device first
	 */
	(void) XEmac_Stop(InstancePtr);

	/*
	 * Take the driver out of polled mode
	 */
	InstancePtr->IsPolled = FALSE;

	/*
	 * Reset the entire IPIF at once.  If we choose someday to reset each
	 * hardware block separately, the reset should occur in the direction of
	 * data flow. For example, for the send direction the reset order is DMA
	 * first, then FIFO, then the MAC transmitter.
	 */
	XIIF_V123B_RESET(InstancePtr->BaseAddress);

	if (XEmac_mIsSgDma(InstancePtr)) {
		/*
		 * After reset, configure the scatter-gather DMA packet threshold and
		 * packet wait bound registers to default values. Ignore the return
		 * values of these functions since they only return error if the device
		 * is not stopped.
		 */
		(void) XEmac_SetPktThreshold(InstancePtr, XEM_SEND,
					     XEM_SGDMA_DFT_THRESHOLD);
		(void) XEmac_SetPktThreshold(InstancePtr, XEM_RECV,
					     XEM_SGDMA_DFT_THRESHOLD);
		(void) XEmac_SetPktWaitBound(InstancePtr, XEM_SEND,
					     XEM_SGDMA_DFT_WAITBOUND);
		(void) XEmac_SetPktWaitBound(InstancePtr, XEM_RECV,
					     XEM_SGDMA_DFT_WAITBOUND);
	}
}

/*****************************************************************************/
/**
*
* Set the MAC address for this driver/device.  The address is a 48-bit value.
* The device must be stopped before calling this function.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param AddressPtr is a pointer to a 6-byte MAC address.
*
* @return
*
* - XST_SUCCESS if the MAC address was set successfully
* - XST_DEVICE_IS_STARTED if the device has not yet been stopped
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XEmac_SetMacAddress(XEmac * InstancePtr, u8 * AddressPtr)
{
	u32 MacAddr = 0;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(AddressPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * The device must be stopped before setting the MAC address
	 */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Set the device station address high and low registers
	 */
	MacAddr = (AddressPtr[0] << 8) | AddressPtr[1];
	XIo_Out32(InstancePtr->BaseAddress + XEM_SAH_OFFSET, MacAddr);

	MacAddr = (AddressPtr[2] << 24) | (AddressPtr[3] << 16) |
	    (AddressPtr[4] << 8) | AddressPtr[5];

	XIo_Out32(InstancePtr->BaseAddress + XEM_SAL_OFFSET, MacAddr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Get the MAC address for this driver/device.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param BufferPtr is an output parameter, and is a pointer to a buffer into
*        which the current MAC address will be copied. The buffer must be at
*        least 6 bytes.
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
XEmac_GetMacAddress(XEmac * InstancePtr, u8 * BufferPtr)
{
	u32 MacAddrHi;
	u32 MacAddrLo;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(BufferPtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	MacAddrHi = XIo_In32(InstancePtr->BaseAddress + XEM_SAH_OFFSET);
	MacAddrLo = XIo_In32(InstancePtr->BaseAddress + XEM_SAL_OFFSET);

	BufferPtr[0] = (u8) (MacAddrHi >> 8);
	BufferPtr[1] = (u8) MacAddrHi;
	BufferPtr[2] = (u8) (MacAddrLo >> 24);
	BufferPtr[3] = (u8) (MacAddrLo >> 16);
	BufferPtr[4] = (u8) (MacAddrLo >> 8);
	BufferPtr[5] = (u8) MacAddrLo;
}

/******************************************************************************/
/**
*
* Configure DMA capabilities.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* - XST_SUCCESS  if successful initialization of DMA
*
* @note
*
* None.
*
******************************************************************************/
static XStatus
ConfigureDma(XEmac * InstancePtr)
{
	XStatus Result;

	/*
	 * Initialize the DMA channels with their base addresses. We assume
	 * scatter-gather DMA is the only possible configuration. Descriptor space
	 * will need to be set later by the upper layer.
	 */
	Result = XDmaChannel_Initialize(&InstancePtr->RecvChannel,
					InstancePtr->BaseAddress +
					XEM_DMA_RECV_OFFSET);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	Result = XDmaChannel_Initialize(&InstancePtr->SendChannel,
					InstancePtr->BaseAddress +
					XEM_DMA_SEND_OFFSET);

	return Result;
}

/******************************************************************************/
/**
*
* Configure the send and receive FIFO components with their base addresses
* and interrupt masks.  Currently the base addresses are defined constants.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* XST_SUCCESS if successful initialization of the packet FIFOs
*
* @note
*
* None.
*
******************************************************************************/
static XStatus
ConfigureFifo(XEmac * InstancePtr)
{
	XStatus Result;

	/*
	 * Return status from the packet FIFOs initialization is ignored since
	 * they always return success.
	 */
	Result = XPacketFifoV100b_Initialize(&InstancePtr->RecvFifo,
					     InstancePtr->BaseAddress +
					     XEM_PFIFO_RXREG_OFFSET,
					     InstancePtr->BaseAddress +
					     XEM_PFIFO_RXDATA_OFFSET);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	Result = XPacketFifoV100b_Initialize(&InstancePtr->SendFifo,
					     InstancePtr->BaseAddress +
					     XEM_PFIFO_TXREG_OFFSET,
					     InstancePtr->BaseAddress +
					     XEM_PFIFO_TXDATA_OFFSET);
	return Result;
}

/******************************************************************************/
/**
*
* This is a stub for the scatter-gather send and recv callbacks. The stub
* is here in case the upper layers forget to set the handlers.
*
* @param CallBackRef is a pointer to the upper layer callback reference
* @param BdPtr is a pointer to the first buffer descriptor in a list
* @param NumBds is the number of descriptors in the list.
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
static void
StubSgHandler(void *CallBackRef, XBufDescriptor * BdPtr, u32 NumBds)
{
	XASSERT_VOID_ALWAYS();
}

/******************************************************************************/
/**
*
* This is a stub for the non-DMA send and recv callbacks.  The stub is here in
* case the upper layers forget to set the handlers.
*
* @param CallBackRef is a pointer to the upper layer callback reference
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
static void
StubFifoHandler(void *CallBackRef)
{
	XASSERT_VOID_ALWAYS();
}

/******************************************************************************/
/**
*
* This is a stub for the asynchronous error callback.  The stub is here in
* case the upper layers forget to set the handler.
*
* @param CallBackRef is a pointer to the upper layer callback reference
* @param ErrorCode is the Xilinx error code, indicating the cause of the error
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
static void
StubErrorHandler(void *CallBackRef, XStatus ErrorCode)
{
	XASSERT_VOID_ALWAYS();
}

/*****************************************************************************/
/**
*
* Lookup the device configuration based on the unique device ID.  The table
* EmacConfigTable contains the configuration info for each device in the system.
*
* @param DeviceId is the unique device ID of the device being looked up.
*
* @return
*
* A pointer to the configuration table entry corresponding to the given
* device ID, or NULL if no match is found.
*
* @note
*
* None.
*
******************************************************************************/
XEmac_Config *
XEmac_LookupConfig(u16 DeviceId)
{
	XEmac_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XEMAC_NUM_INSTANCES; i++) {
		if (XEmac_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XEmac_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}
