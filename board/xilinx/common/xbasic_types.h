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
* @file xbasic_types.h
*
* This file contains basic types for Xilinx software IP.  These types do not
* follow the standard naming convention with respect to using the component
* name in front of each name because they are considered to be primitives.
*
* @note
*
* This file contains items which are architecture dependent.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rmm  12/14/01 First release
*	rmm  05/09/03 Added "xassert always" macros to rid ourselves of diab
*		      compiler warnings
* 1.00a rpm  11/07/03 Added XNullHandler function as a stub interrupt handler
* </pre>
*
******************************************************************************/

#ifndef XBASIC_TYPES_H		/* prevent circular inclusions */
#define XBASIC_TYPES_H		/* by using protection macros */

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif
/** Null */

#define XCOMPONENT_IS_READY	0x11111111	/* component has been initialized */
#define XCOMPONENT_IS_STARTED	0x22222222	/* component has been started */

/* the following constants and declarations are for unit test purposes and are
 * designed to be used in test applications.
 */
#define XTEST_PASSED	0
#define XTEST_FAILED	1

#define XASSERT_NONE	 0
#define XASSERT_OCCURRED 1

extern unsigned int XAssertStatus;
extern void XAssert(char *, int);

/**************************** Type Definitions *******************************/

/** @name Primitive types
 * These primitive types are created for transportability.
 * They are dependent upon the target architecture.
 * @{
 */
#include <linux/types.h>

typedef struct {
	u32 Upper;
	u32 Lower;
} Xuint64;

/*@}*/

/**
 * This data type defines an interrupt handler for a device.
 * The argument points to the instance of the component
 */
typedef void (*XInterruptHandler) (void *InstancePtr);

/**
 * This data type defines a callback to be invoked when an
 * assert occurs. The callback is invoked only when asserts are enabled
 */
typedef void (*XAssertCallback) (char *FilenamePtr, int LineNumber);

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Return the most significant half of the 64 bit data type.
*
* @param x is the 64 bit word.
*
* @return
*
* The upper 32 bits of the 64 bit word.
*
* @note
*
* None.
*
******************************************************************************/
#define XUINT64_MSW(x) ((x).Upper)

/*****************************************************************************/
/**
* Return the least significant half of the 64 bit data type.
*
* @param x is the 64 bit word.
*
* @return
*
* The lower 32 bits of the 64 bit word.
*
* @note
*
* None.
*
******************************************************************************/
#define XUINT64_LSW(x) ((x).Lower)

#ifndef NDEBUG

/*****************************************************************************/
/**
* This assert macro is to be used for functions that do not return anything
* (void). This in conjunction with the XWaitInAssert boolean can be used to
* accomodate tests so that asserts which fail allow execution to continue.
*
* @param expression is the expression to evaluate. If it evaluates to false,
*	 the assert occurs.
*
* @return
*
* Returns void unless the XWaitInAssert variable is true, in which case
* no return is made and an infinite loop is entered.
*
* @note
*
* None.
*
******************************************************************************/
#define XASSERT_VOID(expression)			\
{							\
	if (expression) {				\
		XAssertStatus = XASSERT_NONE;		\
	} else {					\
		XAssert(__FILE__, __LINE__);		\
		XAssertStatus = XASSERT_OCCURRED;	\
		return;					\
	}						\
}

/*****************************************************************************/
/**
* This assert macro is to be used for functions that do return a value. This in
* conjunction with the XWaitInAssert boolean can be used to accomodate tests so
* that asserts which fail allow execution to continue.
*
* @param expression is the expression to evaluate. If it evaluates to false,
*	 the assert occurs.
*
* @return
*
* Returns 0 unless the XWaitInAssert variable is true, in which case
* no return is made and an infinite loop is entered.
*
* @note
*
* None.
*
******************************************************************************/
#define XASSERT_NONVOID(expression)		   \
{						   \
	if (expression) {			   \
		XAssertStatus = XASSERT_NONE;	   \
	} else {				   \
		XAssert(__FILE__, __LINE__);	   \
		XAssertStatus = XASSERT_OCCURRED;  \
		return 0;			   \
	}					   \
}

/*****************************************************************************/
/**
* Always assert. This assert macro is to be used for functions that do not
* return anything (void). Use for instances where an assert should always
* occur.
*
* @return
*
* Returns void unless the XWaitInAssert variable is true, in which case
* no return is made and an infinite loop is entered.
*
* @note
*
* None.
*
******************************************************************************/
#define XASSERT_VOID_ALWAYS()			   \
{						   \
	XAssert(__FILE__, __LINE__);		   \
	XAssertStatus = XASSERT_OCCURRED;	   \
	return;					   \
}

/*****************************************************************************/
/**
* Always assert. This assert macro is to be used for functions that do return
* a value. Use for instances where an assert should always occur.
*
* @return
*
* Returns void unless the XWaitInAssert variable is true, in which case
* no return is made and an infinite loop is entered.
*
* @note
*
* None.
*
******************************************************************************/
#define XASSERT_NONVOID_ALWAYS()		   \
{						   \
	XAssert(__FILE__, __LINE__);		   \
	XAssertStatus = XASSERT_OCCURRED;	   \
	return 0;				   \
}

#else

#define XASSERT_VOID(expression)
#define XASSERT_VOID_ALWAYS()
#define XASSERT_NONVOID(expression)
#define XASSERT_NONVOID_ALWAYS()
#endif

/************************** Function Prototypes ******************************/

void XAssertSetCallback(XAssertCallback Routine);
void XNullHandler(void *NullParameter);

#endif	/* end of protection macro */
