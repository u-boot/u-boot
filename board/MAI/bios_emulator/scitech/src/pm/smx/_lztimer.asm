;****************************************************************************
;*
;*                  SciTech OS Portability Manager Library
;*
;*  ========================================================================
;*
;*    The contents of this file are subject to the SciTech MGL Public
;*    License Version 1.0 (the "License"); you may not use this file
;*    except in compliance with the License. You may obtain a copy of
;*    the License at http://www.scitechsoft.com/mgl-license.txt
;*
;*    Software distributed under the License is distributed on an
;*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
;*    implied. See the License for the specific language governing
;*    rights and limitations under the License.
;*
;*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
;*
;*    The Initial Developer of the Original Code is SciTech Software, Inc.
;*    All Rights Reserved.
;*
;*  ========================================================================
;*
;* Language:    NASM or TASM Assembler
;* Environment: smx 32 bit intel CPU
;*
;* Description: SMX does not support 486's, so this module is not necessary.
;*
;*  All registers and all flags are preserved by all routines, except
;*  interrupts which are always turned on
;*
;****************************************************************************

        IDEAL

include "scitech.mac"

header      _lztimer

begdataseg  _lztimer

enddataseg  _lztimer

begcodeseg  _lztimer                ; Start of code segment

cprocstart   LZ_disable
        cli
        ret
cprocend

cprocstart   LZ_enable
        sti
        ret
cprocend

endcodeseg  _lztimer

        END
