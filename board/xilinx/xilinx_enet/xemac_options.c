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
* @file xemac_options.c
*
* Functions in this file handle configuration of the XEmac driver.
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

/************************** Constant Definitions *****************************/

#define XEM_MAX_IFG         32	/* Maximum Interframe gap value */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*
 * A table of options and masks. This table maps the user-visible options with
 * the control register masks. It is used in Set/GetOptions as an alternative
 * to a series of if/else pairs. Note that the polled options does not have a
 * corresponding entry in the control register, so it does not exist in the
 * table.
 */
typedef struct {
	u32 Option;
	u32 Mask;
} OptionMap;

static OptionMap OptionsTable[] = {
	{XEM_UNICAST_OPTION, XEM_ECR_UNICAST_ENABLE_MASK},
	{XEM_BROADCAST_OPTION, XEM_ECR_BROAD_ENABLE_MASK},
	{XEM_PROMISC_OPTION, XEM_ECR_PROMISC_ENABLE_MASK},
	{XEM_FDUPLEX_OPTION, XEM_ECR_FULL_DUPLEX_MASK},
	{XEM_LOOPBACK_OPTION, XEM_ECR_LOOPBACK_MASK},
	{XEM_MULTICAST_OPTION, XEM_ECR_MULTI_ENABLE_MASK},
	{XEM_FLOW_CONTROL_OPTION, XEM_ECR_PAUSE_FRAME_MASK},
	{XEM_INSERT_PAD_OPTION, XEM_ECR_XMIT_PAD_ENABLE_MASK},
	{XEM_INSERT_FCS_OPTION, XEM_ECR_XMIT_FCS_ENABLE_MASK},
	{XEM_INSERT_ADDR_OPTION, XEM_ECR_XMIT_ADDR_INSERT_MASK},
	{XEM_OVWRT_ADDR_OPTION, XEM_ECR_XMIT_ADDR_OVWRT_MASK},
	{XEM_STRIP_PAD_FCS_OPTION, XEM_ECR_RECV_STRIP_ENABLE_MASK}
};

#define XEM_NUM_OPTIONS     (sizeof(OptionsTable) / sizeof(OptionMap))

/*****************************************************************************/
/**
*
* Set Ethernet driver/device options.  The device must be stopped before
* calling this function.  The options are contained within a bit-mask with each
* bit representing an option (i.e., you can OR the options together). A one (1)
* in the bit-mask turns an option on, and a zero (0) turns the option off.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param OptionsFlag is a bit-mask representing the Ethernet options to turn on
*        or off. See xemac.h for a description of the available options.
*
* @return
*
* - XST_SUCCESS if the options were set successfully
* - XST_DEVICE_IS_STARTED if the device has not yet been stopped
*
* @note
*
* This function is not thread-safe and makes use of internal resources that are
* shared between the Start, Stop, and SetOptions functions, so if one task
* might be setting device options while another is trying to start the device,
* protection of this shared data (typically using a semaphore) is required.
*
******************************************************************************/
XStatus
XEmac_SetOptions(XEmac * InstancePtr, u32 OptionsFlag)
{
	u32 ControlReg;
	int Index;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	ControlReg = XIo_In32(InstancePtr->BaseAddress + XEM_ECR_OFFSET);

	/*
	 * Loop through the options table, turning the option on or off
	 * depending on whether the bit is set in the incoming options flag.
	 */
	for (Index = 0; Index < XEM_NUM_OPTIONS; Index++) {
		if (OptionsFlag & OptionsTable[Index].Option) {
			ControlReg |= OptionsTable[Index].Mask;	/* turn it on */
		} else {
			ControlReg &= ~OptionsTable[Index].Mask;	/* turn it off */
		}
	}

	/*
	 * TODO: need to validate addr-overwrite only if addr-insert?
	 */

	/*
	 * Now write the control register. Leave it to the upper layers
	 * to restart the device.
	 */
	XIo_Out32(InstancePtr->BaseAddress + XEM_ECR_OFFSET, ControlReg);

	/*
	 * Check the polled option
	 */
	if (OptionsFlag & XEM_POLLED_OPTION) {
		InstancePtr->IsPolled = TRUE;
	} else {
		InstancePtr->IsPolled = FALSE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Get Ethernet driver/device options. The 32-bit value returned is a bit-mask
* representing the options.  A one (1) in the bit-mask means the option is on,
* and a zero (0) means the option is off.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
*
* @return
*
* The 32-bit value of the Ethernet options. The value is a bit-mask
* representing all options that are currently enabled. See xemac.h for a
* description of the available options.
*
* @note
*
* None.
*
******************************************************************************/
u32
XEmac_GetOptions(XEmac * InstancePtr)
{
	u32 OptionsFlag = 0;
	u32 ControlReg;
	int Index;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Get the control register to determine which options are currently set.
	 */
	ControlReg = XIo_In32(InstancePtr->BaseAddress + XEM_ECR_OFFSET);

	/*
	 * Loop through the options table to determine which options are set
	 */
	for (Index = 0; Index < XEM_NUM_OPTIONS; Index++) {
		if (ControlReg & OptionsTable[Index].Mask) {
			OptionsFlag |= OptionsTable[Index].Option;
		}
	}

	if (InstancePtr->IsPolled) {
		OptionsFlag |= XEM_POLLED_OPTION;
	}

	return OptionsFlag;
}

/*****************************************************************************/
/**
*
* Set the Interframe Gap (IFG), which is the time the MAC delays between
* transmitting frames.  There are two parts required.  The total interframe gap
* is the total of the two parts.  The values provided for the Part1 and Part2
* parameters are multiplied by 4 to obtain the bit-time interval. The first
* part should be the first 2/3 of the total interframe gap. The MAC will reset
* the interframe gap timer if carrier sense becomes true during the period
* defined by interframe gap Part1. Part1 may be shorter than 2/3 the total and
* can be as small as zero. The second part should be the last 1/3 of the total
* interframe gap, but can be as large as the total interframe gap. The MAC
* will not reset the interframe gap timer if carrier sense becomes true during
* the period defined by interframe gap Part2.
*
* The device must be stopped before setting the interframe gap.
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param Part1 is the interframe gap part 1 (which will be multiplied by 4 to
*        get the bit-time interval).
* @param Part2 is the interframe gap part 2 (which will be multiplied by 4 to
*        get the bit-time interval).
*
* @return
*
* - XST_SUCCESS if the interframe gap was set successfully
* - XST_DEVICE_IS_STARTED if the device has not been stopped
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XEmac_SetInterframeGap(XEmac * InstancePtr, u8 Part1, u8 Part2)
{
	u32 Ifg;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(Part1 < XEM_MAX_IFG);
	XASSERT_NONVOID(Part2 < XEM_MAX_IFG);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Be sure device has been stopped
	 */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	Ifg = Part1 << XEM_IFGP_PART1_SHIFT;
	Ifg |= (Part2 << XEM_IFGP_PART2_SHIFT);
	XIo_Out32(InstancePtr->BaseAddress + XEM_IFGP_OFFSET, Ifg);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Get the interframe gap, parts 1 and 2. See the description of interframe gap
* above in XEmac_SetInterframeGap().
*
* @param InstancePtr is a pointer to the XEmac instance to be worked on.
* @param Part1Ptr is a pointer to an 8-bit buffer into which the interframe gap
*        part 1 value will be copied.
* @param Part2Ptr is a pointer to an 8-bit buffer into which the interframe gap
*        part 2 value will be copied.
*
* @return
*
* None. The values of the interframe gap parts are copied into the
* output parameters.
*
******************************************************************************/
void
XEmac_GetInterframeGap(XEmac * InstancePtr, u8 * Part1Ptr, u8 * Part2Ptr)
{
	u32 Ifg;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(Part1Ptr != NULL);
	XASSERT_VOID(Part2Ptr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	Ifg = XIo_In32(InstancePtr->BaseAddress + XEM_IFGP_OFFSET);
	*Part1Ptr = (Ifg & XEM_IFGP_PART1_MASK) >> XEM_IFGP_PART1_SHIFT;
	*Part2Ptr = (Ifg & XEM_IFGP_PART2_MASK) >> XEM_IFGP_PART2_SHIFT;
}
