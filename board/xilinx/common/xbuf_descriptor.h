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
* xbuf_descriptor.h
*
* DESCRIPTION:
*
* This file contains the interface for the XBufDescriptor component.
* The XBufDescriptor component is a passive component that only maps over
* a buffer descriptor data structure shared by the scatter gather DMA hardware
* and software. The component's primary purpose is to provide encapsulation of
* the buffer descriptor processing.  See the source file xbuf_descriptor.c for
* details.
*
* NOTES:
*
* Most of the functions of this component are implemented as macros in order
* to optimize the processing.  The names are not all uppercase such that they
* can be switched between macros and functions easily.
*
******************************************************************************/

#ifndef XBUF_DESCRIPTOR_H	/* prevent circular inclusions */
#define XBUF_DESCRIPTOR_H	/* by using protection macros */

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xdma_channel_i.h"

/************************** Constant Definitions *****************************/

/* The following constants allow access to all fields of a buffer descriptor
 * and are necessary at this level of visibility to allow macros to access
 * and modify the fields of a buffer descriptor.  It is not expected that the
 * user of a buffer descriptor would need to use these constants.
 */

#define XBD_DEVICE_STATUS_OFFSET    0
#define XBD_CONTROL_OFFSET	    1
#define XBD_SOURCE_OFFSET	    2
#define XBD_DESTINATION_OFFSET	    3
#define XBD_LENGTH_OFFSET	    4
#define XBD_STATUS_OFFSET	    5
#define XBD_NEXT_PTR_OFFSET	    6
#define XBD_ID_OFFSET		    7
#define XBD_FLAGS_OFFSET	    8
#define XBD_RQSTED_LENGTH_OFFSET    9

#define XBD_SIZE_IN_WORDS	    10

/*
 * The following constants define the bits of the flags field of a buffer
 * descriptor
 */

#define XBD_FLAGS_LOCKED_MASK	    1UL

/**************************** Type Definitions *******************************/

typedef u32 XBufDescriptor[XBD_SIZE_IN_WORDS];

/***************** Macros (Inline Functions) Definitions *********************/

/* each of the following macros are named the same as functions rather than all
 * upper case in order to allow either the macros or the functions to be
 * used, see the source file xbuf_descriptor.c for documentation
 */

#define XBufDescriptor_Initialize(InstancePtr)			\
{								\
    (*((u32 *)InstancePtr + XBD_CONTROL_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_SOURCE_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_DESTINATION_OFFSET) = 0);   \
    (*((u32 *)InstancePtr + XBD_LENGTH_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_STATUS_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_DEVICE_STATUS_OFFSET) = 0); \
    (*((u32 *)InstancePtr + XBD_NEXT_PTR_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_ID_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_FLAGS_OFFSET) = 0);	    \
    (*((u32 *)InstancePtr + XBD_RQSTED_LENGTH_OFFSET) = 0); \
}

#define XBufDescriptor_GetControl(InstancePtr)	 \
    (u32)(*((u32 *)InstancePtr + XBD_CONTROL_OFFSET))

#define XBufDescriptor_SetControl(InstancePtr, Control)	 \
    (*((u32 *)InstancePtr + XBD_CONTROL_OFFSET) = (u32)Control)

#define XBufDescriptor_IsLastControl(InstancePtr) \
    (u32)(*((u32 *)InstancePtr + XBD_CONTROL_OFFSET) & \
	       XDC_CONTROL_LAST_BD_MASK)

#define XBufDescriptor_SetLast(InstancePtr) \
    (*((u32 *)InstancePtr + XBD_CONTROL_OFFSET) |= XDC_CONTROL_LAST_BD_MASK)

#define XBufDescriptor_GetSrcAddress(InstancePtr) \
    ((u32 *)(*((u32 *)InstancePtr + XBD_SOURCE_OFFSET)))

#define XBufDescriptor_SetSrcAddress(InstancePtr, Source) \
    (*((u32 *)InstancePtr + XBD_SOURCE_OFFSET) = (u32)Source)

#define XBufDescriptor_GetDestAddress(InstancePtr) \
    ((u32 *)(*((u32 *)InstancePtr + XBD_DESTINATION_OFFSET)))

#define XBufDescriptor_SetDestAddress(InstancePtr, Destination) \
    (*((u32 *)InstancePtr + XBD_DESTINATION_OFFSET) = (u32)Destination)

#define XBufDescriptor_GetLength(InstancePtr)				\
    (u32)(*((u32 *)InstancePtr + XBD_RQSTED_LENGTH_OFFSET) -	\
	      *((u32 *)InstancePtr + XBD_LENGTH_OFFSET))

#define XBufDescriptor_SetLength(InstancePtr, Length)			    \
{									    \
    (*((u32 *)InstancePtr + XBD_LENGTH_OFFSET) = (u32)(Length));    \
    (*((u32 *)InstancePtr + XBD_RQSTED_LENGTH_OFFSET) = (u32)(Length));\
}

#define XBufDescriptor_GetStatus(InstancePtr)	 \
    (u32)(*((u32 *)InstancePtr + XBD_STATUS_OFFSET))

#define XBufDescriptor_SetStatus(InstancePtr, Status)	 \
    (*((u32 *)InstancePtr + XBD_STATUS_OFFSET) = (u32)Status)

#define XBufDescriptor_IsLastStatus(InstancePtr) \
    (u32)(*((u32 *)InstancePtr + XBD_STATUS_OFFSET) & \
	       XDC_STATUS_LAST_BD_MASK)

#define XBufDescriptor_GetDeviceStatus(InstancePtr) \
    ((u32)(*((u32 *)InstancePtr + XBD_DEVICE_STATUS_OFFSET)))

#define XBufDescriptor_SetDeviceStatus(InstancePtr, Status) \
    (*((u32 *)InstancePtr + XBD_DEVICE_STATUS_OFFSET) = (u32)Status)

#define XBufDescriptor_GetNextPtr(InstancePtr) \
    (XBufDescriptor *)(*((u32 *)InstancePtr + XBD_NEXT_PTR_OFFSET))

#define XBufDescriptor_SetNextPtr(InstancePtr, NextPtr) \
    (*((u32 *)InstancePtr + XBD_NEXT_PTR_OFFSET) = (u32)NextPtr)

#define XBufDescriptor_GetId(InstancePtr) \
    (u32)(*((u32 *)InstancePtr + XBD_ID_OFFSET))

#define XBufDescriptor_SetId(InstancePtr, Id) \
    (*((u32 *)InstancePtr + XBD_ID_OFFSET) = (u32)Id)

#define XBufDescriptor_GetFlags(InstancePtr) \
    (u32)(*((u32 *)InstancePtr + XBD_FLAGS_OFFSET))

#define XBufDescriptor_SetFlags(InstancePtr, Flags) \
    (*((u32 *)InstancePtr + XBD_FLAGS_OFFSET) = (u32)Flags)

#define XBufDescriptor_Lock(InstancePtr) \
    (*((u32 *)InstancePtr + XBD_FLAGS_OFFSET) |= XBD_FLAGS_LOCKED_MASK)

#define XBufDescriptor_Unlock(InstancePtr) \
    (*((u32 *)InstancePtr + XBD_FLAGS_OFFSET) &= ~XBD_FLAGS_LOCKED_MASK)

#define XBufDescriptor_IsLocked(InstancePtr) \
    (*((u32 *)InstancePtr + XBD_FLAGS_OFFSET) & XBD_FLAGS_LOCKED_MASK)

/************************** Function Prototypes ******************************/

/* The following prototypes are provided to allow each of the functions to
 * be implemented as a function rather than a macro, and to provide the
 * syntax to allow users to understand how to call the macros, they are
 * commented out to prevent linker errors
 *

u32 XBufDescriptor_Initialize(XBufDescriptor* InstancePtr);

u32 XBufDescriptor_GetControl(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetControl(XBufDescriptor* InstancePtr, u32 Control);

u32 XBufDescriptor_IsLastControl(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetLast(XBufDescriptor* InstancePtr);

u32 XBufDescriptor_GetLength(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetLength(XBufDescriptor* InstancePtr, u32 Length);

u32 XBufDescriptor_GetStatus(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetStatus(XBufDescriptor* InstancePtr, u32 Status);
u32 XBufDescriptor_IsLastStatus(XBufDescriptor* InstancePtr);

u32 XBufDescriptor_GetDeviceStatus(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetDeviceStatus(XBufDescriptor* InstancePtr,
				    u32 Status);

u32 XBufDescriptor_GetSrcAddress(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetSrcAddress(XBufDescriptor* InstancePtr,
				  u32 SourceAddress);

u32 XBufDescriptor_GetDestAddress(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetDestAddress(XBufDescriptor* InstancePtr,
				   u32 DestinationAddress);

XBufDescriptor* XBufDescriptor_GetNextPtr(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetNextPtr(XBufDescriptor* InstancePtr,
			       XBufDescriptor* NextPtr);

u32 XBufDescriptor_GetId(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetId(XBufDescriptor* InstancePtr, u32 Id);

u32 XBufDescriptor_GetFlags(XBufDescriptor* InstancePtr);
void XBufDescriptor_SetFlags(XBufDescriptor* InstancePtr, u32 Flags);

void XBufDescriptor_Lock(XBufDescriptor* InstancePtr);
void XBufDescriptor_Unlock(XBufDescriptor* InstancePtr);
u32 XBufDescriptor_IsLocked(XBufDescriptor* InstancePtr);

void XBufDescriptor_Copy(XBufDescriptor* InstancePtr,
			 XBufDescriptor* DestinationPtr);

*/

#endif				/* end of protection macro */
