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
;* Environment: 32-bit Windows NT device driver
;*
;* Description: Low level assembly support for the PM library specific to
;*              Windows NT device drivers.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

header      _irq                ; Set up memory model

begdataseg  _irq

    cextern _PM_rtcHandler,CPTR
    cextern _PM_prevRTC,FCPTR

RtcInside   dw      0           ; Are we still handling current interrupt
sidtBuf     df      0           ; Buffer for sidt instruction

enddataseg  _irq

begcodeseg  _irq                ; Start of code segment

cpublic _PM_irqCodeStart

; Macro to delay briefly to ensure that enough time has elapsed between
; successive I/O accesses so that the device being accessed can respond
; to both accesses even on a very fast PC.

ifdef   USE_NASM
%macro  DELAY 0
        jmp     short $+2
        jmp     short $+2
        jmp     short $+2
%endmacro
%macro  IODELAYN 1
%rep    %1
        DELAY
%endrep
%endmacro
else
macro   DELAY
        jmp     short $+2
        jmp     short $+2
        jmp     short $+2
endm
macro   IODELAYN    N
    rept    N
        DELAY
    endm
endm
endif

;----------------------------------------------------------------------------
; PM_rtcISR - Real time clock interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Hardware interrupt handler for the timer interrupt, to dispatch control
; to high level C based subroutines. We save the state of all registers
; in this routine, and switch to a local stack. Interrupts are *off*
; when we call the user code.
;
; NOTE: This routine switches to a local stack before calling any C code,
;       and hence is _not_ re-entrant. Make sure your C code executes as
;       quickly as possible, since a timer overrun will simply hang the
;       system.
;----------------------------------------------------------------------------
cprocfar    _PM_rtcISR

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; If we enable interrupts and call into any C based interrupt handling code,
; we need to setup a bunch of important information for the NT kernel. The
; code below takes care of this housekeeping for us (see Undocumented NT for
; details). If we don't do this housekeeping and interrupts are enabled,
; the kernel will become very unstable and crash within 10 seconds or so.
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        pushad
        pushfd
        push    fs

        mov     ebx,00000030h
        mov     fs,bx
        sub     esp,50h
        mov     ebp,esp

; Setup the exception frame to NULL

        mov     ebx,[DWORD cs:0FFDFF000h]
        mov     [DWORD ds:0FFDFF000h], 0FFFFFFFFh
        mov     [DWORD ebp],ebx

; Save away the existing KSS ebp

        mov     esi,[DWORD cs:0FFDFF124h]
        mov     ebx,[DWORD esi+00000128h]
        mov     [DWORD ebp+4h],ebx
        mov     [DWORD esi+00000128h],ebp

; Save away the kernel time and the thread mode (kernel/user)

        mov     edi,[DWORD esi+00000137h]
        mov     [DWORD ebp+8h],edi

; Set the thread mode (kernel/user) based on the code selector

        mov     ebx,[DWORD ebp+7Ch]
        and     ebx,01
        mov     [BYTE esi+00000137h],bl

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; End of special interrupt Prolog code
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; Clear priority interrupt controller and re-enable interrupts so we
; dont lock things up for long.

        mov     al,20h
        out     0A0h,al
        out     020h,al

; Clear real-time clock timeout

        in      al,70h              ; Read CMOS index register
        push    eax                 ;  and save for later
        IODELAYN 3
        mov     al,0Ch
        out     70h,al
        IODELAYN 5
        in      al,71h

; Call the C interrupt handler function

        cmp     [BYTE RtcInside],1  ; Check for mutual exclusion
        je      @@Exit
        mov     [BYTE RtcInside],1
        sti                         ; Enable interrupts
        cld                         ; Clear direction flag for C code
        call    [CPTR _PM_rtcHandler]
        cli                         ; Disable interrupts on exit!
        mov     [BYTE RtcInside],0

@@Exit: pop     eax
        out     70h,al              ; Restore CMOS index register

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; Start of special epilog code to restore stuff on exit from handler
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; Restore the KSS ebp

        mov     esi,[DWORD cs:0FFDFF124h]
        mov     ebx,[DWORD ebp+4]
        mov     [DWORD esi+00000128h],ebx

; Restore the exception frame

        mov     ebx,[DWORD ebp]
        mov     [DWORD fs:00000000],ebx

; Restore the thread mode

        mov     ebx,[DWORD ebp+8h]
        mov     esi,[DWORD fs:00000124h]
        mov     [BYTE esi+00000137h],bl
        add     esp, 50h
        pop     fs
        popfd
        popad

; Return from interrupt

        iret

cprocend

cpublic _PM_irqCodeEnd

;----------------------------------------------------------------------------
; void _PM_getISR(int irq,PMFARPTR *handler);
;----------------------------------------------------------------------------
; Function to return the specific IRQ handler direct from the IDT.
;----------------------------------------------------------------------------
cprocstart  _PM_getISR

        ARG     idtEntry:UINT, handler:DPTR

        enter_c 0
        mov     ecx,[handler]           ; Get address of handler to fill in
        sidt    [sidtBuf]               ; Get IDTR register into sidtBuf
        mov     eax,[DWORD sidtBuf+2]   ; Get address of IDT into EAX
        mov     ebx,[idtEntry]
        lea     eax,[eax+ebx*8]         ; Get entry in the IDT
        movzx   edx,[WORD eax+6]        ; Get high order 16-bits
        shl     edx,16                  ; Move into top 16-bits of address
        mov     dx,[WORD eax]           ; Get low order 16-bits
        mov     [DWORD ecx],edx         ; Store linear address of handler
        mov     dx,[WORD eax+2]         ; Get selector value
        mov     [WORD ecx+4],dx         ; Store selector value
        leave_c
        ret

cprocend    _PM_getISR

;----------------------------------------------------------------------------
; void _PM_setISR(int irq,void *handler);
;----------------------------------------------------------------------------
; Function to set the specific IRQ handler direct in the IDT.
;----------------------------------------------------------------------------
cprocstart  _PM_setISR

        ARG     irq:UINT, handler:CPTR

        enter_c 0
        mov     ecx,[handler]           ; Get address of new handler
        mov     dx,cs                   ; Get selector for new handler
        sidt    [sidtBuf]               ; Get IDTR register into sidtBuf
        mov     eax,[DWORD sidtBuf+2]   ; Get address of IDT into EAX
        mov     ebx,[idtEntry]
        lea     eax,[eax+ebx*8]         ; Get entry in the IDT
        cli
        mov     [WORD eax+2],dx         ; Store code segment selector
        mov     [WORD eax],cx           ; Store low order bits of handler
        shr     ecx,16
        mov     [WORD eax+6],cx         ; Store high order bits of handler
        sti
        leave_c
        ret

cprocend    _PM_setISR

;----------------------------------------------------------------------------
; void _PM_restoreISR(int irq,PMFARPTR *handler);
;----------------------------------------------------------------------------
; Function to set the specific IRQ handler direct in the IDT.
;----------------------------------------------------------------------------
cprocstart  _PM_restoreISR

        ARG     irq:UINT, handler:CPTR

        enter_c 0
        mov     ecx,[handler]
        mov     dx,[WORD ecx+4]         ; Get selector for old handler
        mov     ecx,[DWORD ecx]         ; Get address of old handler
        sidt    [sidtBuf]               ; Get IDTR register into sidtBuf
        mov     eax,[DWORD sidtBuf+2]   ; Get address of IDT into EAX
        mov     ebx,[idtEntry]
        lea     eax,[eax+ebx*8]         ; Get entry in the IDT
        cli
        mov     [WORD eax+2],dx         ; Store code segment selector
        mov     [WORD eax],cx           ; Store low order bits of handler
        shr     ecx,16
        mov     [WORD eax+6],cx         ; Store high order bits of handler
        sti
        leave_c
        ret

cprocend    _PM_restoreISR

endcodeseg  _irq

        END                     ; End of module

