/**
 * @file  IxOsalConfig.h
 *
 * @brief OSAL Configuration header file
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

/* 
 * This file contains user-editable fields for modules inclusion.
 */
#ifndef IxOsalConfig_H
#define IxOsalConfig_H


/*
 * Note: in the future these config options may
 * become build time decision. 
 */

/* Choose cache */
#define IX_OSAL_CACHED
/* #define IX_OSAL_UNCACHED */


/*
 * Select the module headers to include
 */
#include "IxOsalIoMem.h"	/* I/O Memory Management module API */
#include "IxOsalBufferMgt.h"	/* Buffer Management module API */

/*
 * Select main platform header file to use
 */
#include "IxOsalOem.h"



#endif /* IxOsalConfig_H */
