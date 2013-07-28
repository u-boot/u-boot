/*
 * @file        IxOsalAssert.h 
 * @author	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
 *
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IX_OSAL_ASSERT_H
#define IX_OSAL_ASSERT_H

/*
 * Put the system defined include files required
 * @par
 * <TAGGED>
 */

#include "IxOsalOsAssert.h"

/**
 * @brief Assert macro, assert the condition is true. This
 *        will not be compiled out.
 *        N.B. will result in a system crash if it is false.
 */
#define IX_OSAL_ASSERT(c) IX_OSAL_OS_ASSERT(c)


/**
 * @brief Ensure macro, ensure the condition is true.
 *        This will be conditionally compiled out and
 *        may be used for test purposes.
 */
#ifdef IX_OSAL_ENSURE_ON
#define IX_OSAL_ENSURE(c, str) do { \
if (!(c)) ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, str, \
0, 0, 0, 0, 0, 0); } while (0)

#else
#define IX_OSAL_ENSURE(c, str)
#endif


#endif /* IX_OSAL_ASSERT_H */
