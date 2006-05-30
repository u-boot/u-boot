/**
 * @file IxOsServicesComponents.h (Replaced by OSAL)
 *
 * @brief Header file for memory access
 *
 * @par
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

#ifndef IxOsServicesComponents_H
#define IxOsServicesComponents_H

#include "IxOsalBackward.h"
 * codelets_parityENAcc
 * timeSyncAcc
 * parityENAcc
 * sspAcc
 * i2c
 * integration_sspAcc
 * integration_i2c
#define ix_timeSyncAcc         36
#define ix_parityENAcc         37
#define ix_codelets_parityENAcc     38
#define ix_sspAcc              39
#define ix_i2c                 40
#define ix_integration_sspAcc  41
#define ix_integration_i2c     42
#define ix_osal		       43
#define ix_integration_parityENAcc  44
#define ix_integration_timeSyncAcc  45

/***************************
 * timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_timeSyncAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* timeSyncAcc */

/***************************
 * parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* parityENAcc */

/***************************
 * codelets_parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* codelets_parityENAcc */

#endif /* IxOsServicesComponents_H */

/***************************
 * integration_timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_integration_timeSyncAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* integration_timeSyncAcc */

/***************************
 * integration_parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_integration_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* integration_parityENAcc */
