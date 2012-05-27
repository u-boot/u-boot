/* $Id: xemacpss.c,v 1.1.2.2 2009/07/07 22:54:36 wyang Exp $ */
/******************************************************************************
*
* (c) Copyright 2009-2010 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xemacpss.c
*
* The XEmacPss driver. Functions in this file are the minimum required functions
* for this driver. See xemacpss.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  06/01/09 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xemacpss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

void XEmacPss_StubHandler(void);	/* Default handler routine */

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* Initialize a specific XEmacPss instance/driver. The initialization entails:
* - Initialize fields of the XEmacPss instance structure
* - Reset hardware and apply default options
* - Configure the DMA channels
*
* The PHY is setup independently from the device. Use the MII or whatever other
* interface may be present for setup.
*
* @param InstancePtr is a pointer to the instance to be worked on.
* @param CfgPtr is the device configuration structure containing required
*        hardware build data.
* @param EffectiveAddress is the base address of the device. If address
*        translation is not utilized, this parameter can be passed in using
*        CfgPtr->Config.BaseAddress to specify the physical base address.
*        
* @return
* - XST_SUCCESS if initialization was successful
*
******************************************************************************/
int XEmacPss_CfgInitialize(XEmacPss *InstancePtr, XEmacPss_Config * CfgPtr,
			   u32 EffectiveAddress)
{
	/* Verify arguments */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(CfgPtr != NULL);

	/* Set device base address and ID */
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Set callbacks to an initial stub routine */
	InstancePtr->SendHandler = (XEmacPss_Handler) XEmacPss_StubHandler;
	InstancePtr->RecvHandler = (XEmacPss_Handler) XEmacPss_StubHandler;
	InstancePtr->ErrorHandler = (XEmacPss_ErrHandler) XEmacPss_StubHandler;

	/* Reset the hardware and set default options */
	InstancePtr->IsReady = XCOMPONENT_IS_READY;
	XEmacPss_Reset(InstancePtr);

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* Start the Ethernet controller as follows:
*   - Enable transmitter if XTE_TRANSMIT_ENABLE_OPTION is set
*   - Enable receiver if XTE_RECEIVER_ENABLE_OPTION is set
*   - Start the SG DMA send and receive channels and enable the device
*     interrupt
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return N/A
*
* @note
* Hardware is configured with scatter-gather DMA, the driver expects to start
* the scatter-gather channels and expects that the user has previously set up 
* the buffer descriptor lists.
*
* This function makes use of internal resources that are shared between the
* Start, Stop, and Set/ClearOptions functions. So if one task might be setting 
* device options while another is trying to start the device, the user is 
* required to provide protection of this shared data (typically using a
* semaphore).
*
* This function must not be preempted by an interrupt that may service the
* device.
*
******************************************************************************/
void XEmacPss_Start(XEmacPss *InstancePtr)
{
	u32 Reg;

	/* Assert bad arguments and conditions */
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_VOID(InstancePtr->RxBdRing.BaseBdAddr != 0);
	XASSERT_VOID(InstancePtr->TxBdRing.BaseBdAddr != 0);

        /* If already started, then there is nothing to do */
        if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
                return;
        }

	/* Start DMA */
	/* When starting the DMA channels, both transmit and receive sides
	 * need an initialized BD list.
	 */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_RXQBASE_OFFSET,
			   InstancePtr->RxBdRing.BaseBdAddr);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_TXQBASE_OFFSET,
			   InstancePtr->TxBdRing.BaseBdAddr);

	/* clear any existed int status */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress, XEMACPSS_ISR_OFFSET,
			   XEMACPSS_IXR_ALL_MASK);

	/* Enable transmitter if not already enabled */
	if (InstancePtr->Options & XEMACPSS_TRANSMITTER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		if (!(Reg & XEMACPSS_NWCTRL_TXEN_MASK)) {
			XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
					   XEMACPSS_NWCTRL_OFFSET,
					   Reg | XEMACPSS_NWCTRL_TXEN_MASK);
		}
	}

	/* Enable receiver if not already enabled */
	if (InstancePtr->Options & XEMACPSS_RECEIVER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		if (!(Reg & XEMACPSS_NWCTRL_RXEN_MASK)) {
			XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
					   XEMACPSS_NWCTRL_OFFSET,
					   Reg | XEMACPSS_NWCTRL_RXEN_MASK);
		}
	}

#if 0
        /* Enable TX and RX interrupts */
        XEmacPss_IntEnable(InstancePtr, (XEMACPSS_IXR_TX_ERR_MASK |
		XEMACPSS_IXR_RX_ERR_MASK | XEMACPSS_IXR_FRAMERX_MASK |
		XEMACPSS_IXR_TXCOMPL_MASK));
#endif
	/* Mark as started */
	InstancePtr->IsStarted = XCOMPONENT_IS_STARTED;

	return;
}


/*****************************************************************************/
/**
* Gracefully stop the Ethernet MAC as follows:
*   - Disable all interrupts from this device
*   - Stop DMA channels
*   - Disable the tansmitter and receiver
*
* Device options currently in effect are not changed.
*
* This function will disable all interrupts. Default interrupts settings that
* had been enabled will be restored when XEmacPss_Start() is called.
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @note
* This function makes use of internal resources that are shared between the
* Start, Stop, SetOptions, and ClearOptions functions. So if one task might be 
* setting device options while another is trying to start the device, the user
* is required to provide protection of this shared data (typically using a
* semaphore).
* 
* Stopping the DMA channels causes this function to block until the DMA
* operation is complete.
*
******************************************************************************/
void XEmacPss_Stop(XEmacPss *InstancePtr)
{
	u32 Reg;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Disable all interrupts */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress, XEMACPSS_IDR_OFFSET,
			   XEMACPSS_IXR_ALL_MASK);

	/* Disable the receiver & transmitter */
	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWCTRL_OFFSET);
	Reg &= ~XEMACPSS_NWCTRL_RXEN_MASK;
	Reg &= ~XEMACPSS_NWCTRL_TXEN_MASK;
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCTRL_OFFSET, Reg);

	/* Mark as stopped */
	InstancePtr->IsStarted = 0;
}


/*****************************************************************************/
/**
* Perform a graceful reset of the Ethernet MAC. Resets the DMA channels, the
* transmitter, and the receiver.
*
* Steps to reset
* - Stops transmit and receive channels
* - Stops DMA
* - Configure transmit and receive buffer size to default
* - Clear transmit and receive status register and counters
* - Clear all interrupt sources
* - Clear phy (if there is any previously detected) address
* - Clear MAC addresses (1-4) as well as Type IDs and hash value
*
* All options are placed in their default state. Any frames in the 
* descriptor lists will remain in the lists. The side effect of doing
* this is that after a reset and following a restart of the device, frames
* were in the list before the reset may be transmitted or received.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and restarting the MAC after the reset. Note also that driver statistics
* are not cleared on reset. It is up to the upper layer software to clear the
* statistics if needed.
*
* When a reset is required, the driver notifies the upper layer software of
* this need through the ErrorHandler callback and specific status codes.
* The upper layer software is responsible for calling this Reset function
* and then re-configuring the device.
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
******************************************************************************/
void XEmacPss_Reset(XEmacPss *InstancePtr)
{
	u32 Reg;
	u8 i;
	char EmacPss_zero_MAC[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Stop the device and reset hardware */
	XEmacPss_Stop(InstancePtr);
	InstancePtr->Options = XEMACPSS_DEFAULT_OPTIONS;

	/* Setup hardware with default values */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCTRL_OFFSET,
			   (XEMACPSS_NWCTRL_STATCLR_MASK |
			   XEMACPSS_NWCTRL_MDEN_MASK) &
			   ~XEMACPSS_NWCTRL_LOOPEN_MASK);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCFG_OFFSET,
                           XEMACPSS_NWCFG_100_MASK |
                           XEMACPSS_NWCFG_FDEN_MASK |
                           XEMACPSS_NWCFG_UCASTHASHEN_MASK);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_DMACR_OFFSET,
			   ((((XEMACPSS_RX_BUF_SIZE / XEMACPSS_RX_BUF_UNIT) +
			     ((XEMACPSS_RX_BUF_SIZE %
			       XEMACPSS_RX_BUF_UNIT) ? 1 : 0)) <<
			     XEMACPSS_DMACR_RXBUF_SHIFT) &
			    XEMACPSS_DMACR_RXBUF_MASK) |
			   XEMACPSS_DMACR_RXSIZE_MASK |
			   XEMACPSS_DMACR_TXSIZE_MASK);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_TXSR_OFFSET, 0x0);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_RXQBASE_OFFSET, 0x0);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_TXQBASE_OFFSET, 0x0);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_RXSR_OFFSET, 0x0);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress, XEMACPSS_IDR_OFFSET,
			   XEMACPSS_IXR_ALL_MASK);

	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_ISR_OFFSET);
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress, XEMACPSS_ISR_OFFSET,
			   Reg);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_PHYMNTNC_OFFSET, 0x0);

	XEmacPss_ClearHash(InstancePtr);

	for (i = 1; i < 5; i++) {
		XEmacPss_SetMacAddress(InstancePtr, EmacPss_zero_MAC, i);
		XEmacPss_SetTypeIdCheck(InstancePtr, 0x0, i);
	}

	/* clear all counters */
	for (i = 0; i < (XEMACPSS_LAST_OFFSET - XEMACPSS_OCTTXL_OFFSET) / 4;
	     i++) {
		XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
                                   XEMACPSS_OCTTXL_OFFSET + i * 4);
	}

	/* Disable the receiver */
	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWCTRL_OFFSET);
	Reg &= ~XEMACPSS_NWCTRL_RXEN_MASK;
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCTRL_OFFSET, Reg);

	/* Sync default options with hardware but leave receiver and
         * transmitter disabled. They get enabled with XEmacPss_Start() if
	 * XEMACPSS_TRANSMITTER_ENABLE_OPTION and
         * XEMACPSS_RECEIVER_ENABLE_OPTION are set.
	 */
	XEmacPss_SetOptions(InstancePtr, InstancePtr->Options &
			    ~(XEMACPSS_TRANSMITTER_ENABLE_OPTION |
			      XEMACPSS_RECEIVER_ENABLE_OPTION));

	XEmacPss_ClearOptions(InstancePtr, ~InstancePtr->Options);
}


/******************************************************************************/
/**
 * This is a stub for the asynchronous callbacks. The stub is here in case the
 * upper layer forgot to set the handler(s). On initialization, all handlers are
 * set to this callback. It is considered an error for this handler to be
 * invoked.
 *
 ******************************************************************************/
void XEmacPss_StubHandler(void)
{
	XASSERT_VOID_ALWAYS();
}
