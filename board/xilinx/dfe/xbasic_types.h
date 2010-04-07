/* $Id: xbasic_types.h,v 1.1.2.1 2009/05/19 14:56:55 meinelte Exp $ */
/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2002-2007 Xilinx Inc.
*       All rights reserved.
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
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rmm  12/14/01 First release
*       rmm  05/09/03 Added "xassert always" macros to rid ourselves of diab
*                     compiler warnings
* 1.00a rpm  11/07/03 Added XNullHandler function as a stub interrupt handler
* 1.00a rpm  07/21/04 Added XExceptionHandler typedef for processor exceptions
* 1.00a xd   11/03/04 Improved support for doxygen.
* 1.00a wre  01/25/07 Added Linux style data types u32, u16, u8, TRUE, FALSE
* 1.00a rpm  04/02/07 Added ifndef KERNEL around u32, u16, u8 data types
* </pre>
*
******************************************************************************/

#ifndef XBASIC_TYPES_H	/* prevent circular inclusions */
#define XBASIC_TYPES_H	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

#ifndef TRUE
#  define TRUE		1
#endif

#ifndef FALSE
#  define FALSE		0
#endif

#ifndef NULL
#define NULL		0
#endif

/** Xilinx NULL, TRUE and FALSE legacy support. Deprecated. */
#define XNULL		NULL
#define XTRUE		TRUE
#define XFALSE		FALSE


#define XCOMPONENT_IS_READY     0x11111111  /**< component has been initialized */
#define XCOMPONENT_IS_STARTED   0x22222222  /**< component has been started */

/* the following constants and declarations are for unit test purposes and are
 * designed to be used in test applications.
 */
#define XTEST_PASSED    0
#define XTEST_FAILED    1

#define XASSERT_NONE     0
#define XASSERT_OCCURRED 1

extern unsigned int XAssertStatus;
extern void XAssert(char *, int);

/**************************** Type Definitions *******************************/

/** @name Legacy types
 * Deprecated legacy types.
 * @{
 */
typedef unsigned char	Xuint8;		/**< unsigned 8-bit */
typedef char		Xint8;		/**< signed 8-bit */
typedef unsigned short	Xuint16;	/**< unsigned 16-bit */
typedef short		Xint16;		/**< signed 16-bit */
typedef unsigned long	Xuint32;	/**< unsigned 32-bit */
typedef long		Xint32;		/**< signed 32-bit */
typedef float		Xfloat32;	/**< 32-bit floating point */
typedef double		Xfloat64;	/**< 64-bit double precision FP */
typedef unsigned long	Xboolean;	/**< boolean (XTRUE or XFALSE) */

typedef struct
{
	Xuint32 Upper;
	Xuint32 Lower;
} Xuint64;

/** @name New types
 * New simple types.
 * @{
 */
#ifndef __KERNEL__
typedef Xuint32         u32;
typedef Xuint16         u16;
typedef Xuint8          u8;
#else
#include <linux/types.h>
#endif

/*@}*/

/**
 * This data type defines an interrupt handler for a device.
 * The argument points to the instance of the component
 */
typedef void (*XInterruptHandler) (void *InstancePtr);

/**
 * This data type defines an exception handler for a processor.
 * The argument points to the instance of the component
 */
typedef void (*XExceptionHandler) (void *InstancePtr);

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
* @param    x is the 64 bit word.
*
* @return   The upper 32 bits of the 64 bit word.
*
* @note     None.
*
******************************************************************************/
#define XUINT64_MSW(x) ((x).Upper)

/*****************************************************************************/
/**
* Return the least significant half of the 64 bit data type.
*
* @param    x is the 64 bit word.
*
* @return   The lower 32 bits of the 64 bit word.
*
* @note     None.
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
* @param    expression is the expression to evaluate. If it evaluates to
*           false, the assert occurs.
*
* @return   Returns void unless the XWaitInAssert variable is true, in which
*           case no return is made and an infinite loop is entered.
*
* @note     None.
*
******************************************************************************/
#define XASSERT_VOID(expression)                   \
{                                                  \
    if (expression)                                \
    {                                              \
        XAssertStatus = XASSERT_NONE;              \
    }                                              \
    else                                           \
    {                                              \
        XAssert(__FILE__, __LINE__);               \
                XAssertStatus = XASSERT_OCCURRED;  \
        return;                                    \
    }                                              \
}

/*****************************************************************************/
/**
* This assert macro is to be used for functions that do return a value. This in
* conjunction with the XWaitInAssert boolean can be used to accomodate tests so
* that asserts which fail allow execution to continue.
*
* @param    expression is the expression to evaluate. If it evaluates to false,
*           the assert occurs.
*
* @return   Returns 0 unless the XWaitInAssert variable is true, in which case
*           no return is made and an infinite loop is entered.
*
* @note     None.
*
******************************************************************************/
#define XASSERT_NONVOID(expression)                \
{                                                  \
    if (expression)                                \
    {                                              \
        XAssertStatus = XASSERT_NONE;              \
    }                                              \
    else                                           \
    {                                              \
        XAssert(__FILE__, __LINE__);               \
                XAssertStatus = XASSERT_OCCURRED;  \
        return 0;                                  \
    }                                              \
}

/*****************************************************************************/
/**
* Always assert. This assert macro is to be used for functions that do not
* return anything (void). Use for instances where an assert should always
* occur.
*
* @return Returns void unless the XWaitInAssert variable is true, in which case
*         no return is made and an infinite loop is entered.
*
* @note   None.
*
******************************************************************************/
#define XASSERT_VOID_ALWAYS()                      \
{                                                  \
   XAssert(__FILE__, __LINE__);                    \
           XAssertStatus = XASSERT_OCCURRED;       \
   return;                                         \
}

/*****************************************************************************/
/**
* Always assert. This assert macro is to be used for functions that do return
* a value. Use for instances where an assert should always occur.
*
* @return Returns void unless the XWaitInAssert variable is true, in which case
*         no return is made and an infinite loop is entered.
*
* @note   None.
*
******************************************************************************/
#define XASSERT_NONVOID_ALWAYS()                   \
{                                                  \
   XAssert(__FILE__, __LINE__);                    \
           XAssertStatus = XASSERT_OCCURRED;       \
   return 0;                                       \
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

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
