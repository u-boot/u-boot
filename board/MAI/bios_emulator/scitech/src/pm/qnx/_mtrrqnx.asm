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
;* Language:    NASM
;* Environment: QNX
;*
;* Description: Assembler support routines for the Memory Type Range Register
;*              (MTRR) module for QNX.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

header      _mtrrqnx                ; Set up memory model

begdataseg  _mtrrqnx                ; Start of code segment

ifdef   USE_NASM
%define R0_FLUSH_TLB    0
%define R0_SAVE_CR4     1
%define R0_RESTORE_CR4  2
%define R0_READ_MSR     3
%define R0_WRITE_MSR    4
else
R0_FLUSH_TLB        EQU 0
R0_SAVE_CR4         EQU 1
R0_RESTORE_CR4      EQU 2
R0_READ_MSR         EQU 3
R0_WRITE_MSR        EQU 4
endif

cpublic _PM_R0
_PM_R0_service      dd  0
_PM_R0_reg          dd  0
_PM_R0_eax          dd  0
_PM_R0_edx          dd  0

enddataseg  _mtrrqnx                ; Start of code segment

begcodeseg  _mtrrqnx                ; Start of code segment

P586

;----------------------------------------------------------------------------
; ulong _MTRR_disableInt(void);
;----------------------------------------------------------------------------
; Return processor interrupt status and disable interrupts.
;----------------------------------------------------------------------------
cprocstart  _MTRR_disableInt

        pushfd                  ; Put flag word on stack
;       cli                     ; Disable interrupts!
        pop     eax             ; deposit flag word in return register
        ret

cprocend

;----------------------------------------------------------------------------
; void _MTRR_restoreInt(ulong ps);
;----------------------------------------------------------------------------
; Restore processor interrupt status.
;----------------------------------------------------------------------------
cprocstart  _MTRR_restoreInt

        ARG     ps:ULONG

        push    ebp
        mov     ebp,esp         ; Set up stack frame
        push    [ULONG ps]
        popfd                   ; Restore processor status (and interrupts)
        pop     ebp
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
; ulong _PM_ring0_isr(void);
;----------------------------------------------------------------------------
; Ring 0 clock interrupt handler that we use to execute the MTRR support
; code.
;----------------------------------------------------------------------------
cprocnear   _PM_ring0_isr

;--------------------------------------------------------
; void PM_flushTLB(void);
;--------------------------------------------------------
        pushad
        cmp     [DWORD _PM_R0_service],R0_FLUSH_TLB
        jne     @@1
        wbinvd                  ; Flush the CPU cache
        mov     eax,cr3         
        mov     cr3,eax         ; Flush the TLB
        jmp     @@Exit

;--------------------------------------------------------
; ulong _MTRR_saveCR4(void);
;--------------------------------------------------------
@@1:    cmp     [DWORD _PM_R0_service],R0_SAVE_CR4
        jne     @@2

; Save value of CR4 and clear Page Global Enable (bit 7)

        mov     ebx,cr4
        mov     eax,ebx
        and     al,7Fh
        mov     cr4,eax

; Disable and flush caches

        mov     eax,cr0
        or      eax,40000000h
        wbinvd
        mov     cr0,eax
        wbinvd

; Return value from CR4

        mov     [_PM_R0_reg],ebx
        jmp     @@Exit

;--------------------------------------------------------
; void _MTRR_restoreCR4(ulong cr4Val)
;--------------------------------------------------------
@@2:    cmp     [DWORD _PM_R0_service],R0_RESTORE_CR4
        jne     @@3

        mov     eax,cr0
        and     eax,0BFFFFFFFh
        mov     cr0,eax
        mov     eax,[_PM_R0_reg]
        mov     cr4,eax
        jmp     @@Exit

;--------------------------------------------------------
; void _MTRR_readMSR(int reg, ulong FAR *eax, ulong FAR *edx);
;--------------------------------------------------------
@@3:    cmp     [DWORD _PM_R0_service],R0_READ_MSR
        jne     @@4

        mov     ecx,[_PM_R0_reg]
        rdmsr
        mov     [_PM_R0_eax],eax
        mov     [_PM_R0_edx],edx
        jmp     @@Exit

;--------------------------------------------------------
; void _MTRR_writeMSR(int reg, ulong eax, ulong edx);
;--------------------------------------------------------
@@4:    cmp     [DWORD _PM_R0_service],R0_WRITE_MSR
        jne     @@Exit

        mov     ecx,[_PM_R0_reg]
        mov     eax,[_PM_R0_eax]
        mov     edx,[_PM_R0_edx]
        wrmsr
        jmp     @@Exit

@@Exit: mov     [DWORD _PM_R0_service],-1
        popad
        mov     eax,0
        retf

cprocend

endcodeseg  _mtrrqnx

        END                     ; End of module
