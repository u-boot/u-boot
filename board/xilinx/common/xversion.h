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
/*****************************************************************************
*
* This file contains the interface for the XVersion component. This
* component represents a version ID.  It is encapsulated within a component
* so that it's type and implementation can change without affecting users of
* it.
*
* The version is formatted as X.YYZ where X = 0 - 9, Y = 00 - 99, Z = a - z
* X is the major revision, YY is the minor revision, and Z is the
* compatability revision.
*
* Packed versions are also utilized for the configuration ROM such that
* memory is minimized. A packed version consumes only 16 bits and is
* formatted as follows.
*
* <pre>
* Revision                  Range       Bit Positions
*
* Major Revision            0 - 9       Bits 15 - 12
* Minor Revision            0 - 99      Bits 11 - 5
* Compatability Revision    a - z       Bits 4 - 0
* </pre>
*
******************************************************************************/

#ifndef XVERSION_H		/* prevent circular inclusions */
#define XVERSION_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* the following data type is used to hold a null terminated version string
 * consisting of the following format, "X.YYX"
 */
typedef s8 XVersion[6];

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XVersion_UnPack(XVersion * InstancePtr, u16 PackedVersion);

XStatus XVersion_Pack(XVersion * InstancePtr, u16 * PackedVersion);

u32 XVersion_IsEqual(XVersion * InstancePtr, XVersion * VersionPtr);

void XVersion_ToString(XVersion * InstancePtr, s8 * StringPtr);

XStatus XVersion_FromString(XVersion * InstancePtr, s8 * StringPtr);

void XVersion_Copy(XVersion * InstancePtr, XVersion * VersionPtr);

#endif				/* end of protection macro */
