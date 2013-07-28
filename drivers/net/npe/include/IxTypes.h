/**
 * @file IxTypes.h (Replaced by OSAL)
 *
 * @date 28-NOV-2001

 * @brief This file contains basic types used by the IXP400 software
 *
 * Design Notes:
 *    This file shall only include fundamental types and definitions to be
 *    shared by all the IXP400 components.
 *    Please DO NOT add component-specific types here.
 *
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

/**
 * @defgroup IxTypes IXP400 Types (IxTypes) 
 *
 * @brief Basic data types used by the IXP400 project
 *
 * @{
 */

#ifndef IxTypes_H

#ifndef __doxygen_HIDE

#define IxTypes_H

#endif /* __doxygen_HIDE */


/* WR51880: Undefined data types workaround for backward compatibility */
#ifdef __linux
#ifndef __INCvxTypesOldh
typedef int (*FUNCPTR)(void);
typedef int STATUS;
#define OK (0)
#define ERROR (-1)
#endif
#endif

#include "IxOsalBackward.h"

#endif /* IxTypes_H */

/**
 * @} addtogroup IxTypes
 */
