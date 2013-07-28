/** 
 * This file is intended to provide backward 
 * compatibility for main osService/OSSL 
 * APIs. 
 *
 * It shall be phased out gradually and users
 * are strongly recommended to use IX_OSAL API.
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

#ifndef IX_OSAL_BACKWARD_ASSERT_H
#define IX_OSAL_BACKWARD_ASSERT_H

#define IX_ENSURE(c, str)	IX_OSAL_ENSURE(c, str)
#define IX_ASSERT(c)		IX_OSAL_ASSERT(c)

#endif /* IX_OSAL_BACKWARD_ASSERT_H */
