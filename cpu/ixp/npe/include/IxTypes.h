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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
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
