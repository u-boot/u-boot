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
;* Environment: OS/2 32 bit protected mode
;*
;* Description: Low level assembly support for the PM library specific
;*              to OS/2
;*
;****************************************************************************

        IDEAL

include "scitech.mac"               ; Memory model macros

header      _pmos2                  ; Set up memory model

begdataseg  _pmos2

        cglobal _PM_ioentry
        cglobal _PM_gdt
_PM_ioentry     dd  0               ; Offset to call gate
_PM_gdt         dw  0               ; Selector to call gate

enddataseg  _pmos2

begcodeseg  _pmos2                  ; Start of code segment

;----------------------------------------------------------------------------
; int PM_setIOPL(int iopl)
;----------------------------------------------------------------------------
; Change the IOPL level for the 32-bit task. Returns the previous level
; so it can be restored for the task correctly.
;----------------------------------------------------------------------------
cprocstart  PM_setIOPL

        ARG     iopl:UINT

        enter_c
        pushfd                      ; Save the old EFLAGS for later
        mov     ecx,[iopl]          ; ECX := IOPL level
        xor     ebx,ebx             ; Change IOPL level function code (0)
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

;----------------------------------------------------------------------------
; void _PM_setGDTSelLimit(ushort selector, ulong limit);
;----------------------------------------------------------------------------
; Change the GDT selector limit to given value. Used to change selector
; limits to address the entire system address space.
;----------------------------------------------------------------------------
cprocstart  _PM_setGDTSelLimit

        ARG     selector:USHORT, limit:UINT
        
        enter_c
        sub     esp,20              ; Make room for selector data on stack
        mov     ecx,esp             ; ECX := selector data structure
        mov     bx,[selector]       ; Fill out the data structure
        and     bx,0FFF8h           ; Kick out the LDT/GDT and DPL bits     
        mov     [WORD ecx],bx
        mov     ebx,[limit]
        mov     [DWORD ecx+4],ebx
        mov     ebx,5               ; Set GDT selector limit function code      
ifdef   USE_NASM
        call far dword [_PM_ioentry]
else
        call    [FWORD _PM_ioentry]
endif
        add     esp,20
        leave_c
        ret

cprocend    

;----------------------------------------------------------------------------
; uchar _MTRR_getCx86(uchar reg);
;----------------------------------------------------------------------------
; Read a Cyrix CPU indexed register
;----------------------------------------------------------------------------
cprocstart  _MTRR_getCx86

        ARG     reg:UCHAR

        enter_c
        mov     al,[reg]
        out     22h,al
        in      al,23h
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; uchar _MTRR_setCx86(uchar reg,uchar val);
;----------------------------------------------------------------------------
; Write a Cyrix CPU indexed register
;----------------------------------------------------------------------------
cprocstart  _MTRR_setCx86

        ARG     reg:UCHAR, val:UCHAR

        enter_c
        mov     al,[reg]
        out     22h,al
        mov     al,[val]
        out     23h,al
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; ulong _MTRR_disableInt(void);
;----------------------------------------------------------------------------
; Return processor interrupt status and disable interrupts.
;----------------------------------------------------------------------------
cprocstart  _MTRR_disableInt

; Do nothing!
        ret

cprocend

;----------------------------------------------------------------------------
; void _MTRR_restoreInt(ulong ps);
;----------------------------------------------------------------------------
; Restore processor interrupt status.
;----------------------------------------------------------------------------
cprocstart  _MTRR_restoreInt

; Do nothing!
        ret

cprocend

;----------------------------------------------------------------------------
; void DebugInt(void)
;----------------------------------------------------------------------------
cprocstart  DebugInt

        int     3
        ret

cprocend

endcodeseg  _pmos2

        END                         ; End of module
        
