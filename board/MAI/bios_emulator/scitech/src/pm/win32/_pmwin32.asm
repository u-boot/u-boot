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
;* Language:    80386 Assembler, TASM 4.0 or NASM
;* Environment: Win32
;*
;* Description: Low level assembly support for the PM library specific
;*              to Windows.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"               ; Memory model macros

header      _pmwin32                    ; Set up memory model

begdataseg  _pmwin32

        cglobal _PM_ioentry
        cglobal _PM_gdt
_PM_ioentry     dd  0               ; Offset to call gate
_PM_gdt         dw  0               ; Selector to call gate

enddataseg  _pmwin32

begcodeseg  _pmwin32                    ; Start of code segment

;----------------------------------------------------------------------------
; int PM_setIOPL(int iopl)
;----------------------------------------------------------------------------
; Change the IOPL level for the 32-bit task. Returns the previous level
; so it can be restored for the task correctly.
;----------------------------------------------------------------------------
cprocstart  _PM_setIOPLViaCallGate

        ARG     iopl:UINT

        enter_c
        pushfd                      ; Save the old EFLAGS for later
        mov     ecx,[iopl]          ; ECX := IOPL level
        xor     ebx,ebx             ; Change IOPL level function code
ifdef   USE_NASM
        call far dword [_PM_ioentry]
else
        call    [FWORD _PM_ioentry]
endif
        pop     eax
        and     eax,0011000000000000b
        shr     eax,12
        leave_c
        ret

cprocend

endcodeseg  _pmwin32

        END                         ; End of module
