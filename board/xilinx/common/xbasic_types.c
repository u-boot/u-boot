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
* @file xbasic_types.c
*
* This file contains basic functions for Xilinx software IP.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rpm  11/07/03 Added XNullHandler function as a stub interrupt handler
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**
 * This variable allows testing to be done easier with asserts. An assert
 * sets this variable such that a driver can evaluate this variable
 * to determine if an assert occurred.
 */
unsigned int XAssertStatus;

/**
 * This variable allows the assert functionality to be changed for testing
 * such that it does not wait infinitely. Use the debugger to disable the
 * waiting during testing of asserts.
 */
u32 XWaitInAssert = TRUE;

/* The callback function to be invoked when an assert is taken */
static XAssertCallback XAssertCallbackRoutine = (XAssertCallback) NULL;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Implements assert. Currently, it calls a user-defined callback function
* if one has been set.  Then, it potentially enters an infinite loop depending
* on the value of the XWaitInAssert variable.
*
* @param    File is the name of the filename of the source
* @param    Line is the linenumber within File
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
XAssert(char *File, int Line)
{
	/* if the callback has been set then invoke it */
	if (XAssertCallbackRoutine != NULL) {
		(*XAssertCallbackRoutine) (File, Line);
	}

	/* if specified, wait indefinitely such that the assert will show up
	 * in testing
	 */
	while (XWaitInAssert) {
	}
}

/*****************************************************************************/
/**
*
* Sets up a callback function to be invoked when an assert occurs. If there
* was already a callback installed, then it is replaced.
*
* @param    Routine is the callback to be invoked when an assert is taken
*
* @return
*
* None.
*
* @note
*
* This function has no effect if NDEBUG is set
*
******************************************************************************/
void
XAssertSetCallback(XAssertCallback Routine)
{
	XAssertCallbackRoutine = Routine;
}

/*****************************************************************************/
/**
*
* Null handler function. This follows the XInterruptHandler signature for
* interrupt handlers. It can be used to assign a null handler (a stub) to an
* interrupt controller vector table.
*
* @param    NullParameter is an arbitrary void pointer and not used.
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
XNullHandler(void *NullParameter)
{
}
