/**
 * @file IxOsalOsIxp400CustomizedMapping.h 
 *
 * @brief Set LE coherency modes for components. 
 *        The default setting is IX_OSAL_NO_MAPPING for LE.
 * 
 *
 *		  By default IX_OSAL_STATIC_MEMORY_MAP is defined for all the components.
 *		  If any component uses a dynamic memory map it must define
 *		  IX_OSAL_DYNAMIC_MEMORY_MAP in its corresponding section.
 *        
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

#ifndef IxOsalOsIxp400CustomizedMapping_H
#define IxOsalOsIxp400CustomizedMapping_H

/*
 * only include this file in Little Endian
 */

#if defined (IX_OSAL_LINUX_BE)
#error Only include IxOsalOsIxp400CustomizedMapping.h in Little Endian
#endif

 /*
  * Components don't have to be in this list if
  * the default mapping is OK.
  */
#define ix_osal                1
#define ix_dmaAcc              2
#define ix_atmdAcc             3

#define ix_atmsch              5
#define ix_ethAcc              6
#define ix_npeMh               7
#define ix_qmgr                8
#define ix_npeDl               9
#define ix_atmm                10
#define ix_hssAcc              11
#define ix_ethDB               12
#define ix_ethMii              13
#define ix_timerCtrl           14
#define ix_adsl                15
#define ix_usb                 16
#define ix_uartAcc             17
#define ix_featureCtrl         18
#define ix_cryptoAcc           19
#define ix_unloadAcc           33
#define ix_perfProfAcc         34
#define ix_parityENAcc                 49
#define ix_sspAcc                      51
#define ix_timeSyncAcc                 52
#define ix_i2c                         53

#define ix_codelets_uartAcc    21
#define ix_codelets_timers     22
#define ix_codelets_atm        23
#define ix_codelets_ethAal5App 24
#define ix_codelets_demoUtils  26
#define ix_codelets_usb        27
#define ix_codelets_hssAcc     28
#define ix_codelets_dmaAcc         40
#define ix_codelets_cryptoAcc	   41
#define ix_codelets_perfProfAcc    42
#define ix_codelets_ethAcc         43
#define ix_codelets_parityENAcc        54
#define ix_codelets_timeSyncAcc        55


#endif /* IxOsalOsIxp400CustomizedMapping_H */


/***************************
 * osal
 ***************************/
#if (IX_COMPONENT_NAME == ix_osal)

#define IX_OSAL_LE_AC_MAPPING

#endif /* osal */

/***************************
 * dmaAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_dmaAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* dmaAcc */

/***************************
 * atmdAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_atmdAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* atmdAcc */

/***************************
 * atmsch
 ***************************/
#if (IX_COMPONENT_NAME == ix_atmsch)

#define IX_OSAL_LE_AC_MAPPING

#endif /* atmsch */

/***************************
 * ethAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_ethAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* ethAcc */

/***************************
 * npeMh
 ***************************/
#if (IX_COMPONENT_NAME == ix_npeMh)

#define IX_OSAL_LE_AC_MAPPING

#endif /* npeMh */

/***************************
 * qmgr
 ***************************/
#if (IX_COMPONENT_NAME == ix_qmgr)

#define IX_OSAL_LE_DC_MAPPING

#endif /* qmgr */

/***************************
 * npeDl
 ***************************/
#if (IX_COMPONENT_NAME == ix_npeDl)

#define IX_OSAL_LE_AC_MAPPING

#endif /* npeDl */

/***************************
 * atmm
 ***************************/
#if (IX_COMPONENT_NAME == ix_atmm)

#define IX_OSAL_LE_AC_MAPPING

#endif /* atmm */

/***************************
 * ethMii
 ***************************/
#if (IX_COMPONENT_NAME == ix_ethMii)

#define IX_OSAL_LE_AC_MAPPING

#endif /* ethMii */


/***************************
 * adsl
 ***************************/
#if (IX_COMPONENT_NAME == ix_adsl)

#define IX_OSAL_LE_AC_MAPPING

#endif /* adsl */

/***************************
 * usb
 ***************************/
#if (IX_COMPONENT_NAME == ix_usb)

#define IX_OSAL_LE_AC_MAPPING

#endif /* usb */

/***************************
 * uartAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_uartAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* uartAcc */

/***************************
 * featureCtrl
 ***************************/
#if (IX_COMPONENT_NAME == ix_featureCtrl)

#define IX_OSAL_LE_AC_MAPPING

#endif /* featureCtrl */

/***************************
 * cryptoAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_cryptoAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* cryptoAcc */

/***************************
 * codelets_usb
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_usb)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_usb */


/***************************
 * codelets_uartAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_uartAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_uartAcc */



/***************************
 * codelets_timers
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_timers)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_timers */

/***************************
 * codelets_atm
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_atm)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_atm */

/***************************
 * codelets_ethAal5App
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_ethAal5App)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_ethAal5App */

/***************************
 * codelets_ethAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_ethAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_ethAcc */


/***************************
 * codelets_demoUtils
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_demoUtils)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_demoUtils */



/***************************
 * perfProfAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_perfProfAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* perfProfAcc */


/***************************
 * unloadAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_unloadAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* unloadAcc */





/***************************
 * parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_parityENAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* parityENAcc */

/***************************
 * codelets_parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_parityENAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_parityENAcc */




/***************************
 * timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_timeSyncAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* timeSyncAcc */


/***************************
 * codelets_timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_timeSyncAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* codelets_timeSyncAcc */




/***************************
 * i2c
 ***************************/
#if (IX_COMPONENT_NAME == ix_i2c)

#define IX_OSAL_LE_AC_MAPPING

#endif /* i2c */



/***************************
 * sspAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_sspAcc)

#define IX_OSAL_LE_AC_MAPPING

#endif /* sspAcc */


