/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Include file defining the external ring 0 helper functions
*               needed by the MTRR module. These functions may be included
*               directly for native ring 0 device drivers, or they may
*               be calls down to a ring 0 helper device driver where
*               appropriate (or the entire MTRR module may be located in
*               the device driver if the device driver is 32-bit).
*
****************************************************************************/

#ifndef __MTRR_H
#define __MTRR_H

#include "scitech.h"

/*--------------------------- Function Prototypes -------------------------*/

#ifdef  __cplusplus
extern "C" {            /* Use "C" linkage when in C++ mode */
#endif

/* Internal functions (requires ring 0 access or helper functions!) */

void    MTRR_init(void);
int     MTRR_enableWriteCombine(ulong base,ulong size,uint type);

/* External assembler helper functions */

ibool   _ASMAPI _MTRR_isRing0(void);
ulong   _ASMAPI _MTRR_disableInt(void);
void    _ASMAPI _MTRR_restoreInt(ulong flags);
ulong   _ASMAPI _MTRR_saveCR4(void);
void    _ASMAPI _MTRR_restoreCR4(ulong cr4Val);
uchar   _ASMAPI _MTRR_getCx86(uchar reg);
void    _ASMAPI _MTRR_setCx86(uchar reg,uchar data);
#ifdef  __16BIT__
void    _ASMAPI _MTRR_readMSR(ulong reg, ulong far *eax, ulong far *edx);
#else
void    _ASMAPI _MTRR_readMSR(ulong reg, ulong *eax, ulong *edx);
#endif
void    _ASMAPI _MTRR_writeMSR(ulong reg, ulong eax, ulong edx);

#ifdef  __cplusplus
}                       /* End of "C" linkage for C++   */
#endif

#endif  /* __MTRR_H */
