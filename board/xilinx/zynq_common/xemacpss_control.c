/* $Id: xemacpss_control.c,v 1.1.2.2 2009/07/07 22:54:36 wyang Exp $ */
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
 * @file xemacpss_control.c
 *
 * Functions in this file implement general purpose command and control related
 * functionality. See xemacpss.h for a detailed description of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a wsy  06/01/09 First release
 * </pre>
 *****************************************************************************/

/***************************** Include Files *********************************/

#include "xemacpss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * Set the MAC address for this driver/device.  The address is a 48-bit value.
 * The device must be stopped before calling this function.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is a pointer to a 6-byte MAC address.
 * @param Index is a index to which MAC (1-4) address.
 *
 * @return
 * - XST_SUCCESS if the MAC address was set successfully
 * - XST_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 *****************************************************************************/
int XEmacPss_SetMacAddress(XEmacPss *InstancePtr, void *AddressPtr, u8 Index)
{
	u32 MacAddr;
	u8 *Aptr = (u8 *) AddressPtr;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(AddressPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_NONVOID((Index <= XEMACPSS_MAX_MAC_ADDR) && (Index > 0));

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

        /* Index ranges 1 to 4, for offset calculation is 0 to 3. */
        Index--;

	/* Set the MAC bits [31:0] in BOT */
	MacAddr = Aptr[0];
	MacAddr |= Aptr[1] << 8;
	MacAddr |= Aptr[2] << 16;
	MacAddr |= Aptr[3] << 24;
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   (XEMACPSS_LADDR1L_OFFSET + Index * 8), MacAddr);

	/* There are reserved bits in TOP so don't affect them */
	MacAddr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    (XEMACPSS_LADDR1H_OFFSET + (Index * 8)));

	MacAddr &= ~XEMACPSS_LADDR_MACH_MASK;

	/* Set MAC bits [47:32] in TOP */
	MacAddr |= Aptr[4];
	MacAddr |= Aptr[5] << 8;

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   (XEMACPSS_LADDR1H_OFFSET + (Index * 8)), MacAddr);

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
 * Get the MAC address for this driver/device.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is an output parameter, and is a pointer to a buffer into
 *        which the current MAC address will be copied.
 * @param Index is a index to which MAC (1-4) address.
 *
 *****************************************************************************/
void XEmacPss_GetMacAddress(XEmacPss *InstancePtr, void *AddressPtr, u8 Index)
{
	u32 MacAddr;
	u8 *Aptr = (u8 *) AddressPtr;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(AddressPtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_VOID((Index <= XEMACPSS_MAX_MAC_ADDR) && (Index > 0));

        /* Index ranges 1 to 4, for offset calculation is 0 to 3. */
        Index--;

	MacAddr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    (XEMACPSS_LADDR1L_OFFSET + (Index * 8)));
	Aptr[0] = (u8) MacAddr;
	Aptr[1] = (u8) (MacAddr >> 8);
	Aptr[2] = (u8) (MacAddr >> 16);
	Aptr[3] = (u8) (MacAddr >> 24);

	/* Read MAC bits [47:32] in TOP */
	MacAddr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    (XEMACPSS_LADDR1H_OFFSET + (Index * 8)));
	Aptr[4] = (u8) MacAddr;
	Aptr[5] = (u8) (MacAddr >> 8);
}


/*****************************************************************************/
/**
 * Set 48-bit MAC addresses in hash table.
 * The device must be stopped before calling this function.
 *
 * The hash address register is 64 bits long and takes up two locations in
 * the memory map. The least significant bits are stored in hash register
 * bottom and the most significant bits in hash register top.
 *
 * The unicast hash enable and the multicast hash enable bits in the network
 * configuration register enable the reception of hash matched frames. The
 * destination address is reduced to a 6 bit index into the 64 bit hash
 * register using the following hash function. The hash function is an XOR
 * of every sixth bit of the destination address.
 *
 * <pre>
 * hash_index[05] = da[05]^da[11]^da[17]^da[23]^da[29]^da[35]^da[41]^da[47]
 * hash_index[04] = da[04]^da[10]^da[16]^da[22]^da[28]^da[34]^da[40]^da[46]
 * hash_index[03] = da[03]^da[09]^da[15]^da[21]^da[27]^da[33]^da[39]^da[45]
 * hash_index[02] = da[02]^da[08]^da[14]^da[20]^da[26]^da[32]^da[38]^da[44]
 * hash_index[01] = da[01]^da[07]^da[13]^da[19]^da[25]^da[31]^da[37]^da[43]
 * hash_index[00] = da[00]^da[06]^da[12]^da[18]^da[24]^da[30]^da[36]^da[42]
 * </pre>
 *
 * da[0] represents the least significant bit of the first byte received,
 * that is, the multicast/unicast indicator, and da[47] represents the most
 * significant bit of the last byte received.
 *
 * If the hash index points to a bit that is set in the hash register then
 * the frame will be matched according to whether the frame is multicast
 * or unicast.
 *
 * A multicast match will be signaled if the multicast hash enable bit is
 * set, da[0] is logic 1 and the hash index points to a bit set in the hash
 * register.
 *
 * A unicast match will be signaled if the unicast hash enable bit is set,
 * da[0] is logic 0 and the hash index points to a bit set in the hash
 * register.
 *
 * To receive all multicast frames, the hash register should be set with
 * all ones and the multicast hash enable bit should be set in the network
 * configuration register.
 *
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is a pointer to a 6-byte MAC address.
 *
 * @return
 * - XST_SUCCESS if the HASH MAC address was set successfully
 * - XST_DEVICE_IS_STARTED if the device has not yet been stopped
 * - XST_INVALID_PARAM if the HASH MAC address passed in does not meet
 *   requirement after calculation
 *
 * @note
 * Having Aptr be unsigned type prevents the following operations from sign
 * extending.
 *****************************************************************************/
int XEmacPss_SetHash(XEmacPss *InstancePtr, void *AddressPtr)
{
	u32 HashAddr;
	u8 *Aptr = (u8 *) AddressPtr;
        u8 Temp1, Temp2, Temp3, Temp4, Temp5, Temp6, Temp7, Temp8;
        int Result;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(AddressPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}
        Temp1 =   Aptr[0] & 0x3F;
        Temp2 = ((Aptr[0] >> 6) & 0x3) | ((Aptr[1] & 0xF) << 2);
        Temp3 = ((Aptr[1] >> 4) & 0xF) | ((Aptr[2] & 0x3) << 4);
        Temp4 = ((Aptr[2] >> 2) & 0x3F);
        Temp5 =   Aptr[3] & 0x3F;
        Temp6 = ((Aptr[3] >> 6) & 0x3) | ((Aptr[4] & 0xF) << 2);
        Temp7 = ((Aptr[4] >> 4) & 0xF) | ((Aptr[5] & 0x3) << 4);
        Temp8 = ((Aptr[5] >> 2) & 0x3F);

        Result = Temp1 ^ Temp2 ^ Temp3 ^ Temp4 ^ Temp5 ^ Temp6 ^ Temp7 ^ Temp8;

        if (Result >= XEMACPSS_MAX_HASH_BITS) {
	        return (XST_INVALID_PARAM);
        }

        if (Result < 32) {
	        HashAddr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHL_OFFSET);
                HashAddr |= (1 << Result);
	        XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
                                    XEMACPSS_HASHL_OFFSET, HashAddr);
        } else {
	        HashAddr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHH_OFFSET);
                HashAddr |= (1 << (Result - 32));
	        XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
                                    XEMACPSS_HASHH_OFFSET, HashAddr);
        }

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * Clear the Hash registers for the mac address pointed by AddressPtr.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 *****************************************************************************/
void XEmacPss_ClearHash(XEmacPss *InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHL_OFFSET, 0x0);

	/* write bits [63:32] in TOP */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHH_OFFSET, 0x0);
}


/*****************************************************************************/
/**
 * Get the Hash address for this driver/device.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is an output parameter, and is a pointer to a buffer into
 *        which the current HASH MAC address will be copied.
 *
 *****************************************************************************/
void XEmacPss_GetHash(XEmacPss *InstancePtr, void *AddressPtr)
{
	u32 *Aptr = (u32 *) AddressPtr;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(AddressPtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	Aptr[0] = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHL_OFFSET);

	/* Read Hash bits [63:32] in TOP */
	Aptr[1] = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				    XEMACPSS_HASHH_OFFSET);
}


/*****************************************************************************/
/**
 * Set the Type ID match for this driver/device.  The register is a 32-bit
 * value. The device must be stopped before calling this function.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Id_Check.
 * @param Index is a index to which Type ID (1-4).
 *
 * @return
 * - XST_SUCCESS if the MAC address was set successfully
 * - XST_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 *****************************************************************************/
int XEmacPss_SetTypeIdCheck(XEmacPss *InstancePtr, u32 Id_Check, u8 Index)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
	XASSERT_NONVOID((Index <= XEMACPSS_MAX_TYPE_ID) && (Index > 0));

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	/* Index ranges 1 to 4, for offset calculation is 0 to 3. */
	Index--;

	/* Set the ID bits in MATCHx register */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   (XEMACPSS_MATCH1_OFFSET + (Index * 4)), Id_Check);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * Set options for the driver/device. The driver should be stopped with
 * XEmacPss_Stop() before changing options.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Options are the options to set. Multiple options can be set by OR'ing
 *        XTE_*_OPTIONS constants together. Options not specified are not
 *        affected.
 *
 * @return
 * - XST_SUCCESS if the options were set successfully
 * - XST_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 * @note
 * See xemacpss.h for a description of the available options.
 *
 *****************************************************************************/
int XEmacPss_SetOptions(XEmacPss *InstancePtr, u32 Options)
{
	u32 Reg;		/* Generic register contents */
	u32 RegNetCfg;		/* Reflects original contents of NET_CONFIG */
	u32 RegNewNetCfg;	/* Reflects new contents of NET_CONFIG */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	/* Many of these options will change the NET_CONFIG registers.
	 * To reduce the amount of IO to the device, group these options here
	 * and change them all at once. 
	 */

	/* Grab current register contents */
	RegNetCfg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				      XEMACPSS_NWCFG_OFFSET);
	RegNewNetCfg = RegNetCfg;

	/*
	 * It is configured to max 1536.
	 */
	if (Options & XEMACPSS_FRAME1536_OPTION) {
		RegNewNetCfg |= (XEMACPSS_NWCFG_1536RXEN_MASK);
	}

	/* Turn on VLAN packet only, only VLAN tagged will be accepted */
	if (Options & XEMACPSS_VLAN_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_NVLANDISC_MASK;
	}

	/* Turn on FCS stripping on receive packets */
	if (Options & XEMACPSS_FCS_STRIP_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_FCSREM_MASK;
	}

	/* Turn on length/type field checking on receive packets */
	if (Options & XEMACPSS_LENTYPE_ERR_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_LENGTHERRDSCRD_MASK;
	}

	/* Turn on flow control */
	if (Options & XEMACPSS_FLOW_CONTROL_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_PAUSEEN_MASK;
	}

	/* Turn on promiscuous frame filtering (all frames are received) */
	if (Options & XEMACPSS_PROMISC_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_COPYALLEN_MASK;
	}

	/* Allow broadcast address reception */
	if (Options & XEMACPSS_BROADCAST_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_BCASTDI_MASK;
	}

	/* Allow multicast address filtering */
	if (Options & XEMACPSS_MULTICAST_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_MCASTHASHEN_MASK;
	}

	/* enable RX checksum offload */
	if (Options & XEMACPSS_RX_CHKSUM_ENABLE_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_RXCHKSUMEN_MASK;
	}

	/* Officially change the NET_CONFIG registers if it needs to be
	 * modified.
	 */
	if (RegNetCfg != RegNewNetCfg) {
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCFG_OFFSET, RegNewNetCfg);
	}

	/* Enable TX checksum offload */
	if (Options & XEMACPSS_TX_CHKSUM_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_DMACR_OFFSET);
		Reg |= XEMACPSS_DMACR_TCPCKSUM_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
					 XEMACPSS_DMACR_OFFSET, Reg);
	}

	/* Enable transmitter */
	if (Options & XEMACPSS_TRANSMITTER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		Reg |= XEMACPSS_NWCTRL_TXEN_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCTRL_OFFSET, Reg);
	}

	/* Enable receiver */
	if (Options & XEMACPSS_RECEIVER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		Reg |= XEMACPSS_NWCTRL_RXEN_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCTRL_OFFSET, Reg);
	}

	/* The remaining options not handled here are managed elsewhere in the
	 * driver. No register modifications are needed at this time. Reflecting
	 * the option in InstancePtr->Options is good enough for now.
	 */

	/* Set options word to its new value */
	InstancePtr->Options |= Options;

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
 * Clear options for the driver/device
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Options are the options to clear. Multiple options can be cleared by
 *        OR'ing XEMACPSS_*_OPTIONS constants together. Options not specified
 *        are not affected.
 *
 * @return
 * - XST_SUCCESS if the options were set successfully
 * - XST_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 * @note
 * See xemacpss.h for a description of the available options.
 *
 *****************************************************************************/
int XEmacPss_ClearOptions(XEmacPss *InstancePtr, u32 Options)
{
	u32 Reg;		/* Generic */
	u32 RegNetCfg;		/* Reflects original contents of NET_CONFIG */
	u32 RegNewNetCfg;	/* Reflects new contents of NET_CONFIG */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	/* Many of these options will change the NET_CONFIG registers.
	 * To reduce the amount of IO to the device, group these options here
	 * and change them all at once. 
	 */

	/* Grab current register contents */
	RegNetCfg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				      XEMACPSS_NWCFG_OFFSET);
	RegNewNetCfg = RegNetCfg;

	/* There is only RX configuration!?
	 * It is configured in two different length, upto 1536 and 10240 bytes
	 */
	if (Options & XEMACPSS_FRAME1536_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_1536RXEN_MASK;
	}

	/* Turn off VLAN packet only */
	if (Options & XEMACPSS_VLAN_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_NVLANDISC_MASK;
	}

	/* Turn off FCS stripping on receive packets */
	if (Options & XEMACPSS_FCS_STRIP_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_FCSREM_MASK;
	}

	/* Turn off length/type field checking on receive packets */
	if (Options & XEMACPSS_LENTYPE_ERR_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_LENGTHERRDSCRD_MASK;
	}

	/* Turn off flow control */
	if (Options & XEMACPSS_FLOW_CONTROL_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_PAUSEEN_MASK;
	}

	/* Turn off promiscuous frame filtering (all frames are received) */
	if (Options & XEMACPSS_PROMISC_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_COPYALLEN_MASK;
	}

	/* Disallow broadcast address filtering => broadcast reception */
	if (Options & XEMACPSS_BROADCAST_OPTION) {
		RegNewNetCfg |= XEMACPSS_NWCFG_BCASTDI_MASK;
	}

	/* Disallow multicast address filtering */
	if (Options & XEMACPSS_MULTICAST_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_MCASTHASHEN_MASK;
	}

	/* Disable RX checksum offload */
	if (Options & XEMACPSS_RX_CHKSUM_ENABLE_OPTION) {
		RegNewNetCfg &= ~XEMACPSS_NWCFG_RXCHKSUMEN_MASK;
	}

	/* Officially change the NET_CONFIG registers if it needs to be
	 * modified.
	 */
	if (RegNetCfg != RegNewNetCfg) {
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCFG_OFFSET, RegNewNetCfg);
	}

	/* Disable TX checksum offload */
	if (Options & XEMACPSS_TX_CHKSUM_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_DMACR_OFFSET);
		Reg &= ~XEMACPSS_DMACR_TCPCKSUM_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
					 XEMACPSS_DMACR_OFFSET, Reg);
	}

	/* Disable transmitter */
	if (Options & XEMACPSS_TRANSMITTER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		Reg &= ~XEMACPSS_NWCTRL_TXEN_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCTRL_OFFSET, Reg);
	}

	/* Disable receiver */
	if (Options & XEMACPSS_RECEIVER_ENABLE_OPTION) {
		Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_NWCTRL_OFFSET);
		Reg &= ~XEMACPSS_NWCTRL_RXEN_MASK;
		XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
				   XEMACPSS_NWCTRL_OFFSET, Reg);
	}

	/* The remaining options not handled here are managed elsewhere in the
	 * driver. No register modifications are needed at this time. Reflecting
	 * option in InstancePtr->Options is good enough for now.
	 */

	/* Set options word to its new value */
	InstancePtr->Options &= ~Options;

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
 * Get current option settings
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * A bitmask of XTE_*_OPTION constants. Any bit set to 1 is to be interpreted
 * as a set opion.
 *
 * @note
 * See xemacpss.h for a description of the available options.
 *
 *****************************************************************************/
u32 XEmacPss_GetOptions(XEmacPss *InstancePtr)
{
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	return (InstancePtr->Options);
}


/*****************************************************************************/
/**
 * Send a pause packet
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - XST_SUCCESS if pause frame transmission was initiated
 * - XST_DEVICE_IS_STOPPED if the device has not been started.
 *
 *****************************************************************************/
int XEmacPss_SendPausePacket(XEmacPss *InstancePtr)
{
	u32 Reg;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* Make sure device is ready for this operation */
	if (InstancePtr->IsStarted != XCOMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STOPPED);
	}

	/* Send flow control frame */
	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWCTRL_OFFSET);
	Reg |= XEMACPSS_NWCTRL_PAUSETX_MASK;
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCTRL_OFFSET, Reg);
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * XEmacPss_GetOperatingSpeed gets the current operating link speed. This may
 * be the value set by XEmacPss_SetOperatingSpeed() or a hardware default.
 *
 * @param InstancePtr references the TEMAC channel on which to operate.
 *
 * @return XEmacPss_GetOperatingSpeed returns the link speed in units of
 *         megabits per second.
 *
 * @note
 *
 *****************************************************************************/
u16 XEmacPss_GetOperatingSpeed(XEmacPss *InstancePtr)
{
	u32 Reg;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
			XEMACPSS_NWCFG_OFFSET);

	if (Reg & XEMACPSS_NWCFG_1000_MASK) {
		return (1000);
	} else {
		if (Reg & XEMACPSS_NWCFG_100_MASK) {
			return (100);
		} else {
			return (10);
		}
	}
}


/*****************************************************************************/
/**
 * XEmacPss_SetOperatingSpeed sets the current operating link speed. For any
 * traffic to be passed, this speed must match the current MII/GMII/SGMII/RGMII
 * link speed.
 *
 * @param InstancePtr references the TEMAC channel on which to operate.
 * @param Speed is the speed to set in units of Mbps. Valid values are 10, 100,
 *        or 1000. XEmacPss_SetOperatingSpeed ignores invalid values.
 *
 * @note
 *
 *****************************************************************************/
void XEmacPss_SetOperatingSpeed(XEmacPss *InstancePtr, u16 Speed)
{
        u32 Reg;

        XASSERT_VOID(InstancePtr != NULL);
        XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
        XASSERT_VOID((Speed == 10) || (Speed == 100) || (Speed == 1000));

        Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
			XEMACPSS_NWCFG_OFFSET);
	Reg &= ~(XEMACPSS_NWCFG_1000_MASK | XEMACPSS_NWCFG_100_MASK);

	switch (Speed) {
	case 10:
                break;

        case 100:
                Reg |= XEMACPSS_NWCFG_100_MASK;
                break;

        case 1000:
                Reg |= XEMACPSS_NWCFG_1000_MASK;
                break;

        default:
                return;
        }

        /* Set register and return */
        XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
                XEMACPSS_NWCFG_OFFSET, Reg);
}


/*****************************************************************************/
/**
 * Set the MDIO clock divisor.
 *
 * Calculating the divisor:
 *
 * <pre>
 *              f[HOSTCLK]
 *   f[MDC] = -----------------
 *            (1 + Divisor) * 2
 * </pre>
 *
 * where f[HOSTCLK] is the bus clock frequency in MHz, and f[MDC] is the
 * MDIO clock frequency in MHz to the PHY. Typically, f[MDC] should not
 * exceed 2.5 MHz. Some PHYs can tolerate faster speeds which means faster
 * access. Here is the table to show values to generate MDC,
 *
 * <pre>
 * 000 : divide pclk by   8 (pclk up to  20 MHz)
 * 001 : divide pclk by  16 (pclk up to  40 MHz)
 * 010 : divide pclk by  32 (pclk up to  80 MHz)
 * 011 : divide pclk by  48 (pclk up to 120 MHz)
 * 100 : divide pclk by  64 (pclk up to 160 MHz)
 * 101 : divide pclk by  96 (pclk up to 240 MHz)
 * 110 : divide pclk by 128 (pclk up to 320 MHz)
 * 111 : divide pclk by 224 (pclk up to 540 MHz)
 * </pre>
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Divisor is the divisor to set. Range is 0b000 to 0b111.
 *
 *****************************************************************************/
void XEmacPss_SetMdioDivisor(XEmacPss *InstancePtr, XEmacPss_MdcDiv Divisor)
{
	u32 Reg;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY)
		XASSERT_VOID(Divisor <= 0x7);	/* only last three bits are valid */

	Reg = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWCFG_OFFSET);
	/* clear these three bits, could be done with mask */
	Reg &= ~XEMACPSS_NWCFG_MDCCLKDIV_MASK;

	Reg |= (Divisor << XEMACPSS_NWCFG_MDC_SHIFT_MASK);

	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_NWCFG_OFFSET, Reg);

}


/*****************************************************************************/
/**
* Read the current value of the PHY register indicated by the PhyAddress and
* the RegisterNum parameters. The MAC provides the driver with the ability to
* talk to a PHY that adheres to the Media Independent Interface (MII) as
* defined in the IEEE 802.3 standard.
*
* Prior to PHY access with this function, the user should have setup the MDIO
* clock with XEmacPss_SetMdioDivisor().
*
* @param InstancePtr is a pointer to the XEmacPss instance to be worked on.
* @param PhyAddress is the address of the PHY to be read (supports multiple
*        PHYs)
* @param RegisterNum is the register number, 0-31, of the specific PHY register
*        to read
* @param PhyDataPtr is an output parameter, and points to a 16-bit buffer into
*        which the current value of the register will be copied.
*
* @return
*
* - XST_SUCCESS if the PHY was read from successfully
* - XST_EMAC_MII_BUSY if there is another PHY operation in progress
*
* @note
*
* This function is not thread-safe. The user must provide mutually exclusive
* access to this function if there are to be multiple threads that can call it.
*
* There is the possibility that this function will not return if the hardware
* is broken (i.e., it never sets the status bit indicating that the read is
* done). If this is of concern to the user, the user should provide a mechanism
* suitable to their needs for recovery.
*
* For the duration of this function, all host interface reads and writes are
* blocked to the current XEmacPss instance.
*
******************************************************************************/
int XEmacPss_PhyRead(XEmacPss *InstancePtr, u32 PhyAddress,
		     u32 RegisterNum, u16 *PhyDataPtr)
{
	u32 Mgtcr;
	volatile u32 Ipisr;

	XASSERT_NONVOID(InstancePtr != NULL);

	/* Make sure no other PHY operation is currently in progress */
	if (!(XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWSR_OFFSET) &
	      XEMACPSS_NWSR_MDIOIDLE_MASK)) {
		return (XST_EMAC_MII_BUSY);
	}

	/* Construct Mgtcr mask for the operation */
	Mgtcr = XEMACPSS_PHYMNTNC_OP_MASK | XEMACPSS_PHYMNTNC_OP_R_MASK |
		(PhyAddress << XEMACPSS_PHYMNTNC_PHYAD_SHIFT_MASK) |
		(RegisterNum << XEMACPSS_PHYMNTNC_PHREG_SHIFT_MASK);

	/* Write Mgtcr and wait for completion */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_PHYMNTNC_OFFSET, Mgtcr);

	do {
		Ipisr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					  XEMACPSS_NWSR_OFFSET);
	} while ((Ipisr & XEMACPSS_NWSR_MDIOIDLE_MASK) == 0);

	/* Read data */
	*PhyDataPtr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					XEMACPSS_PHYMNTNC_OFFSET);

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* Write data to the specified PHY register. The Ethernet driver does not
* require the device to be stopped before writing to the PHY.  Although it is
* probably a good idea to stop the device, it is the responsibility of the
* application to deem this necessary. The MAC provides the driver with the
* ability to talk to a PHY that adheres to the Media Independent Interface
* (MII) as defined in the IEEE 802.3 standard.
*
* Prior to PHY access with this function, the user should have setup the MDIO
* clock with XEmacPss_SetMdioDivisor().
*
* @param InstancePtr is a pointer to the XEmacPss instance to be worked on.
* @param PhyAddress is the address of the PHY to be written (supports multiple
*        PHYs)
* @param RegisterNum is the register number, 0-31, of the specific PHY register
*        to write
* @param PhyData is the 16-bit value that will be written to the register
*
* @return
*
* - XST_SUCCESS if the PHY was written to successfully. Since there is no error
*   status from the MAC on a write, the user should read the PHY to verify the
*   write was successful.
* - XST_EMAC_MII_BUSY if there is another PHY operation in progress
*
* @note
*
* This function is not thread-safe. The user must provide mutually exclusive
* access to this function if there are to be multiple threads that can call it.
*
* There is the possibility that this function will not return if the hardware
* is broken (i.e., it never sets the status bit indicating that the write is
* done). If this is of concern to the user, the user should provide a mechanism
* suitable to their needs for recovery.
*
* For the duration of this function, all host interface reads and writes are
* blocked to the current XEmacPss instance.
*
******************************************************************************/
int XEmacPss_PhyWrite(XEmacPss *InstancePtr, u32 PhyAddress,
		      u32 RegisterNum, u16 PhyData)
{
	u32 Mgtcr;
	volatile u32 Ipisr;

	XASSERT_NONVOID(InstancePtr != NULL);

	/* Make sure no other PHY operation is currently in progress */
	if (!(XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
				XEMACPSS_NWSR_OFFSET) &
	      XEMACPSS_NWSR_MDIOIDLE_MASK)) {
		return (XST_EMAC_MII_BUSY);
	}

	/* Construct Mgtcr mask for the operation */
	Mgtcr = XEMACPSS_PHYMNTNC_OP_MASK | XEMACPSS_PHYMNTNC_OP_W_MASK |
		(PhyAddress << XEMACPSS_PHYMNTNC_PHYAD_SHIFT_MASK) |
		(RegisterNum << XEMACPSS_PHYMNTNC_PHREG_SHIFT_MASK) | PhyData;

	/* Write Mgtcr and wait for completion */
	XEmacPss_WriteReg(InstancePtr->Config.BaseAddress,
			   XEMACPSS_PHYMNTNC_OFFSET, Mgtcr);

	do {
		Ipisr = XEmacPss_ReadReg(InstancePtr->Config.BaseAddress,
					  XEMACPSS_NWSR_OFFSET);
	} while ((Ipisr & XEMACPSS_NWSR_MDIOIDLE_MASK) == 0);

	return (XST_SUCCESS);
}
