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
;* Environment: 32-bit SMX embedded systems development
;*
;* Description: Low level assembly support for the PM library specific to
;*              SMX interrupt handling.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"           ; Memory model macros

header      _pmsmx              ; Set up memory model

; Define the size of our local stacks. For real mode code they cant be
; that big, but for 32 bit protected mode code we can make them nice and
; large so that complex C functions can be used.

MOUSE_STACK EQU 4096
TIMER_STACK EQU 4096
KEY_STACK   EQU 1024
INT10_STACK EQU 1024

ifdef   USE_NASM

; Macro to load DS and ES registers with correct value.

%imacro   LOAD_DS 0
        mov     ds,[cs:_PM_savedDS]
        mov     es,[cs:_PM_savedDS]
%endmacro

; Note that interrupts we disable interrupts during the following stack
; %imacro for correct operation, but we do not enable them again. Normally
; these %imacros are used within interrupt handlers so interrupts should
; already be off. We turn them back on explicitly later if the user code
; needs them to be back on.

; Macro to switch to a new local stack.

%imacro NEWSTK  1
        cli
        mov     [seg_%1],ss
        mov     [ptr_%1],_sp
        mov     [TempSeg],ds
        mov     ss,[TempSeg]
        mov     _sp,offset %1
%endmacro

; %imacro to switch back to the old stack.

%imacro   RESTSTK   1
        cli
        mov     ss,[seg_%1]
        mov     _sp,[ptr_%1]
%endmacro

; %imacro to swap the current stack with the one saved away.

%imacro SWAPSTK 1
        cli
        mov     ax,ss
        xchg    ax,[seg_%1]
        mov     ss,ax
        xchg    _sp,[ptr_%1]
%endmacro

else

; Macro to load DS and ES registers with correct value.

MACRO   LOAD_DS
        mov     ds,[cs:_PM_savedDS]
        mov     es,[cs:_PM_savedDS]
ENDM

; Note that interrupts we disable interrupts during the following stack
; macro for correct operation, but we do not enable them again. Normally
; these macros are used within interrupt handlers so interrupts should
; already be off. We turn them back on explicitly later if the user code
; needs them to be back on.

; Macro to switch to a new local stack.

MACRO   NEWSTK  stkname
        cli
        mov     [seg_&stkname&],ss
        mov     [ptr_&stkname&],_sp
        mov     [TempSeg],ds
        mov     ss,[TempSeg]
        mov     _sp,offset stkname
ENDM

; Macro to switch back to the old stack.

MACRO   RESTSTK stkname
        cli
        mov     ss,[seg_&stkname&]
        mov     _sp,[ptr_&stkname&]
ENDM

; Macro to swap the current stack with the one saved away.

MACRO   SWAPSTK stkname
        cli
        mov     ax,ss
        xchg    ax,[seg_&stkname&]
        mov     ss,ax
        xchg    _sp,[ptr_&stkname&]
ENDM

endif

begdataseg  _pmsmx

    cextern _PM_savedDS,USHORT
    cextern _PM_critHandler,CPTR
    cextern _PM_breakHandler,CPTR
    cextern _PM_timerHandler,CPTR
    cextern _PM_rtcHandler,CPTR
    cextern _PM_keyHandler,CPTR
    cextern _PM_key15Handler,CPTR
    cextern _PM_mouseHandler,CPTR
    cextern _PM_int10Handler,CPTR

    cextern _PM_ctrlCPtr,DPTR
    cextern _PM_ctrlBPtr,DPTR
    cextern _PM_critPtr,DPTR

    cextern _PM_prevTimer,FCPTR
    cextern _PM_prevRTC,FCPTR
    cextern _PM_prevKey,FCPTR
    cextern _PM_prevKey15,FCPTR
    cextern _PM_prevBreak,FCPTR
    cextern _PM_prevCtrlC,FCPTR
    cextern _PM_prevCritical,FCPTR
    cextern _PM_prevRealTimer,ULONG
    cextern _PM_prevRealRTC,ULONG
    cextern _PM_prevRealKey,ULONG
    cextern _PM_prevRealKey15,ULONG
    cextern _PM_prevRealInt10,ULONG

cpublic _PM_pmsmxDataStart

; Allocate space for all of the local stacks that we need. These stacks
; are not very large, but should be large enough for most purposes
; (generally you want to handle these interrupts quickly, simply storing
; the information for later and then returning). If you need bigger
; stacks then change the appropriate value in here.

            ALIGN   4
            dclb MOUSE_STACK    ; Space for local stack (small)
MsStack:                        ; Stack starts at end!
ptr_MsStack DUINT   0           ; Place to store old stack offset
seg_MsStack dw      0           ; Place to store old stack segment

            ALIGN   4
            dclb INT10_STACK    ; Space for local stack (small)
Int10Stack:                     ; Stack starts at end!
ptr_Int10Stack  DUINT   0       ; Place to store old stack offset
seg_Int10Stack  dw      0       ; Place to store old stack segment

            ALIGN   4
            dclb TIMER_STACK    ; Space for local stack (small)
TmStack:                        ; Stack starts at end!
ptr_TmStack DUINT   0           ; Place to store old stack offset
seg_TmStack dw      0           ; Place to store old stack segment

            ALIGN   4
            dclb TIMER_STACK    ; Space for local stack (small)
RtcStack:                       ; Stack starts at end!
ptr_RtcStack DUINT  0           ; Place to store old stack offset
seg_RtcStack dw     0           ; Place to store old stack segment
RtcInside   dw      0           ; Are we still handling current interrupt

            ALIGN   4
            dclb KEY_STACK      ; Space for local stack (small)
KyStack:                        ; Stack starts at end!
ptr_KyStack DUINT   0           ; Place to store old stack offset
seg_KyStack dw      0           ; Place to store old stack segment
KyInside    dw      0           ; Are we still handling current interrupt

            ALIGN   4
            dclb KEY_STACK      ; Space for local stack (small)
Ky15Stack:                      ; Stack starts at end!
ptr_Ky15Stack   DUINT   0       ; Place to store old stack offset
seg_Ky15Stack   dw      0       ; Place to store old stack segment

TempSeg     dw      0           ; Place to store stack segment

cpublic _PM_pmsmxDataEnd

enddataseg  _pmsmx

begcodeseg  _pmsmx              ; Start of code segment

cpublic _PM_pmsmxCodeStart

;----------------------------------------------------------------------------
; PM_mouseISR - Mouse interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Interrupt subroutine called by the mouse driver upon interrupts, to
; dispatch control to high level C based subroutines. Interrupts are on
; when we call the user code.
;
; It is _extremely_ important to save the state of the extended registers
; as these may well be trashed by the routines called from here and not
; restored correctly by the mouse interface module.
;
; NOTE: This routine switches to a local stack before calling any C code,
;       and hence is _not_ re-entrant. For mouse handlers this is not a
;       problem, as the mouse driver arbitrates calls to the user mouse
;       handler for us.
;
; Entry:    AX  - Condition mask giving reason for call
;           BX  - Mouse button state
;           CX  - Horizontal cursor coordinate
;           DX  - Vertical cursor coordinate
;           SI  - Horizontal mickey value
;           DI  - Vertical mickey value
;
;----------------------------------------------------------------------------
cprocfar    _PM_mouseISR

        push    ds              ; Save value of DS
        push    es
        pushad                  ; Save _all_ extended registers
        cld                     ; Clear direction flag

        LOAD_DS                 ; Load DS register
        NEWSTK  MsStack         ; Switch to local stack

; Call the installed high level C code routine

        clrhi   dx              ; Clear out high order values
        clrhi   cx
        clrhi   bx
        clrhi   ax
        sgnhi   si
        sgnhi   di

        push    _di
        push    _si
        push    _dx
        push    _cx
        push    _bx
        push    _ax
        sti                     ; Enable interrupts
        call    [CPTR _PM_mouseHandler]
        _add    sp,12,24

        RESTSTK MsStack         ; Restore previous stack

        popad                   ; Restore all extended registers
        pop     es
        pop     ds
        ret                     ; We are done!!

cprocend

;----------------------------------------------------------------------------
; PM_timerISR - Timer interrupt subroutine dispatcher
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
cprocfar    _PM_timerISR

        push    ds              ; Save value of DS
        push    es
        pushad                  ; Save _all_ extended registers
        cld                     ; Clear direction flag

        LOAD_DS                 ; Load DS register

        NEWSTK  TmStack         ; Switch to local stack
        call    [CPTR _PM_timerHandler]
        RESTSTK TmStack         ; Restore previous stack

        popad                   ; Restore all extended registers
        pop     es
        pop     ds
        iret                    ; Return from interrupt

cprocend

;----------------------------------------------------------------------------
; PM_chainPrevTimer - Chain to previous timer interrupt and return
;----------------------------------------------------------------------------
; Chains to the previous timer interrupt routine and returns control
; back to the high level interrupt handler.
;----------------------------------------------------------------------------
cprocstart  PM_chainPrevTimer

ifdef   TNT
        push    eax
        push    ebx
        push    ecx
        pushfd                  ; Push flags on stack to simulate interrupt
        mov     ax,250Eh        ; Call real mode procedure function
        mov     ebx,[_PM_prevRealTimer]
        mov     ecx,1           ; Copy real mode flags to real mode stack
        int     21h             ; Call the real mode code
        popfd
        pop     ecx
        pop     ebx
        pop     eax
        ret
else
        SWAPSTK TmStack         ; Swap back to previous stack
        pushf                   ; Save state of interrupt flag
        pushf                   ; Push flags on stack to simulate interrupt
ifdef   USE_NASM
        call far dword [_PM_prevTimer]
else
        call    [_PM_prevTimer]
endif
        popf                    ; Restore state of interrupt flag
        SWAPSTK TmStack         ; Swap back to C stack again
        ret
endif

cprocend

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

        push    ds                  ; Save value of DS
        push    es
        pushad                      ; Save _all_ extended registers
        cld                         ; Clear direction flag

; Clear priority interrupt controller and re-enable interrupts so we
; dont lock things up for long.

        mov     al,20h
        out     0A0h,al
        out     020h,al

; Clear real-time clock timeout

        in      al,70h              ; Read CMOS index register
        push    _ax                 ;  and save for later
        IODELAYN 3
        mov     al,0Ch
        out     70h,al
        IODELAYN 5
        in      al,71h

; Call the C interrupt handler function

        LOAD_DS                     ; Load DS register
        cmp     [BYTE RtcInside],1  ; Check for mutual exclusion
        je      @@Exit
        mov     [BYTE RtcInside],1
        sti                         ; Re-enable interrupts
        NEWSTK  RtcStack            ; Switch to local stack
        call    [CPTR _PM_rtcHandler]
        RESTSTK RtcStack            ; Restore previous stack
        mov     [BYTE RtcInside],0

@@Exit: pop     _ax
        out     70h,al              ; Restore CMOS index register
        popad                       ; Restore all extended registers
        pop     es
        pop     ds
        iret                        ; Return from interrupt

cprocend

;----------------------------------------------------------------------------
; PM_keyISR - keyboard interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Hardware interrupt handler for the keyboard interrupt, to dispatch control
; to high level C based subroutines. We save the state of all registers
; in this routine, and switch to a local stack. Interrupts are *off*
; when we call the user code.
;
; NOTE: This routine switches to a local stack before calling any C code,
;       and hence is _not_ re-entrant. However we ensure within this routine
;       mutual exclusion to the keyboard handling routine.
;----------------------------------------------------------------------------
cprocfar    _PM_keyISR

        push    ds              ; Save value of DS
        push    es
        pushad                  ; Save _all_ extended registers
        cld                     ; Clear direction flag

        LOAD_DS                 ; Load DS register

        cmp     [BYTE KyInside],1   ; Check for mutual exclusion
        je      @@Reissued

        mov     [BYTE KyInside],1
        NEWSTK  KyStack         ; Switch to local stack
        call    [CPTR _PM_keyHandler]   ; Call C code
        RESTSTK KyStack         ; Restore previous stack
        mov     [BYTE KyInside],0

@@Exit: popad                   ; Restore all extended registers
        pop     es
        pop     ds
        iret                    ; Return from interrupt

; When the BIOS keyboard handler needs to change the SHIFT status lights
; on the keyboard, in the process of doing this the keyboard controller
; re-issues another interrupt, while the current handler is still executing.
; If we recieve another interrupt while still handling the current one,
; then simply chain directly to the previous handler.
;
; Note that for most DOS extenders, the real mode interrupt handler that we
; install takes care of this for us.

@@Reissued:
ifdef   TNT
        push    eax
        push    ebx
        push    ecx
        pushfd                  ; Push flags on stack to simulate interrupt
        mov     ax,250Eh        ; Call real mode procedure function
        mov     ebx,[_PM_prevRealKey]
        mov     ecx,1           ; Copy real mode flags to real mode stack
        int     21h             ; Call the real mode code
        popfd
        pop     ecx
        pop     ebx
        pop     eax
else
        pushf
ifdef   USE_NASM
        call far dword [_PM_prevKey]
else
        call    [_PM_prevKey]
endif
endif
        jmp     @@Exit

cprocend

;----------------------------------------------------------------------------
; PM_chainPrevkey - Chain to previous key interrupt and return
;----------------------------------------------------------------------------
; Chains to the previous key interrupt routine and returns control
; back to the high level interrupt handler.
;----------------------------------------------------------------------------
cprocstart  PM_chainPrevKey

ifdef   TNT
        push    eax
        push    ebx
        push    ecx
        pushfd                  ; Push flags on stack to simulate interrupt
        mov     ax,250Eh        ; Call real mode procedure function
        mov     ebx,[_PM_prevRealKey]
        mov     ecx,1           ; Copy real mode flags to real mode stack
        int     21h             ; Call the real mode code
        popfd
        pop     ecx
        pop     ebx
        pop     eax
        ret
else

; YIKES! For some strange reason, when execution returns from the
; previous keyboard handler, interrupts are re-enabled!! Since we expect
; interrupts to remain off during the duration of our handler, this can
; cause havoc. However our stack macros always turn off interrupts, so they
; will be off when we exit this routine. Obviously there is a tiny weeny
; window when interrupts will be enabled, but there is nothing we can
; do about this.

        SWAPSTK KyStack         ; Swap back to previous stack
        pushf                   ; Push flags on stack to simulate interrupt
ifdef   USE_NASM
        call far dword [_PM_prevKey]
else
        call    [_PM_prevKey]
endif
        SWAPSTK KyStack         ; Swap back to C stack again
        ret
endif

cprocend

;----------------------------------------------------------------------------
; PM_key15ISR - Int 15h keyboard interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; This routine gets called if we have been called to handle the Int 15h
; keyboard interrupt callout from real mode.
;
;   Entry:  AX  - Hardware scan code to process
;   Exit:   AX  - Hardware scan code to process (0 to ignore)
;----------------------------------------------------------------------------
cprocfar    _PM_key15ISR

        push    ds
        push    es
        LOAD_DS
        cmp     ah,4Fh
        jnz     @@NotOurs       ; Quit if not keyboard callout

        pushad
        cld                     ; Clear direction flag
        xor     ah,ah           ; AX := scan code
        NEWSTK  Ky15Stack       ; Switch to local stack
        push    _ax
        call    [CPTR _PM_key15Handler] ; Call C code
        _add    sp,2,4
        RESTSTK Ky15Stack       ; Restore previous stack
        test    ax,ax
        jz      @@1
        stc                     ; Set carry to process as normal
        jmp     @@2
@@1:    clc                     ; Clear carry to ignore scan code
@@2:    popad
        jmp     @@Exit          ; We are done

@@NotOurs:
ifdef   TNT
        push    eax
        push    ebx
        push    ecx
        pushfd                  ; Push flags on stack to simulate interrupt
        mov     ax,250Eh        ; Call real mode procedure function
        mov     ebx,[_PM_prevRealKey15]
        mov     ecx,1           ; Copy real mode flags to real mode stack
        int     21h             ; Call the real mode code
        popfd
        pop     ecx
        pop     ebx
        pop     eax
else
        pushf
ifdef   USE_NASM
        call far dword [_PM_prevKey15]
else
        call    [_PM_prevKey15]
endif
endif
@@Exit: pop     es
        pop     ds
        retf    4

cprocend

;----------------------------------------------------------------------------
; PM_breakISR - Control Break interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Hardware interrupt handler for the Ctrl-Break interrupt. We simply set
; the Ctrl-Break flag to a 1 and leave (note that this is accessed through
; a far pointer, as it may well be located in conventional memory).
;----------------------------------------------------------------------------
cprocfar    _PM_breakISR

        sti
        push    ds              ; Save value of DS
        push    es
        push    _bx

        LOAD_DS                 ; Load DS register
        mov     ebx,[_PM_ctrlBPtr]
        mov     [UINT _ES _bx],1

; Run alternate break handler code if installed

        cmp     [CPTR _PM_breakHandler],0
        je      @@Exit

        pushad
        mov     _ax,1
        push    _ax
        call    [CPTR _PM_breakHandler] ; Call C code
        pop     _ax
        popad

@@Exit: pop     _bx
        pop     es
        pop     ds
        iret                    ; Return from interrupt

cprocend

;----------------------------------------------------------------------------
; int PM_ctrlBreakHit(int clearFlag)
;----------------------------------------------------------------------------
; Returns the current state of the Ctrl-Break flag and possibly clears it.
;----------------------------------------------------------------------------
cprocstart  PM_ctrlBreakHit

        ARG     clearFlag:UINT

        enter_c
        pushf                   ; Save interrupt status
        push    es
        mov     ebx,[_PM_ctrlBPtr]
        cli                     ; No interrupts thanks!
        mov     _ax,[_ES _bx]
        test    [BYTE clearFlag],1
        jz      @@Done
        mov     [UINT _ES _bx],0

@@Done: pop     es
        popf                    ; Restore interrupt status
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; PM_ctrlCISR - Control Break interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Hardware interrupt handler for the Ctrl-C interrupt. We simply set
; the Ctrl-C flag to a 1 and leave (note that this is accessed through
; a far pointer, as it may well be located in conventional memory).
;----------------------------------------------------------------------------
cprocfar    _PM_ctrlCISR

        sti
        push    ds              ; Save value of DS
        push    es
        push    _bx

        LOAD_DS                 ; Load DS register
        mov     ebx,[_PM_ctrlCPtr]
        mov     [UINT _ES _bx],1

; Run alternate break handler code if installed

        cmp     [CPTR _PM_breakHandler],0
        je      @@Exit

        pushad
        mov     _ax,0
        push    _ax
        call    [CPTR _PM_breakHandler] ; Call C code
        pop     _ax
        popad

@@Exit: pop     _bx
        pop     es
        pop     ds
        iret                    ; Return from interrupt
        iretd

cprocend

;----------------------------------------------------------------------------
; int PM_ctrlCHit(int clearFlag)
;----------------------------------------------------------------------------
; Returns the current state of the Ctrl-C flag and possibly clears it.
;----------------------------------------------------------------------------
cprocstart  PM_ctrlCHit

        ARG     clearFlag:UINT

        enter_c
        pushf                   ; Save interrupt status
        push    es
        mov     ebx,[_PM_ctrlCPtr]
        cli                     ; No interrupts thanks!
        mov     _ax,[_ES _bx]
        test    [BYTE clearFlag],1
        jz      @@Done
        mov     [UINT _ES _bx],0

@@Done:
        pop     es
        popf                    ; Restore interrupt status
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; PM_criticalISR - Control Error handler interrupt subroutine dispatcher
;----------------------------------------------------------------------------
; Interrupt handler for the MSDOS Critical Error interrupt, to dispatch
; control to high level C based subroutines. We save the state of all
; registers in this routine, and switch to a local stack. We also pass
; the values of the AX and DI registers to the as pointers, so that the
; values can be modified before returning to MSDOS.
;----------------------------------------------------------------------------
cprocfar    _PM_criticalISR

        sti
        push    ds              ; Save value of DS
        push    es
        push    _bx             ; Save register values changed
        cld                     ; Clear direction flag

        LOAD_DS                 ; Load DS register
        mov     ebx,[_PM_critPtr]
        mov     [_ES _bx],ax
        mov     [_ES _bx+2],di

; Run alternate critical handler code if installed

        cmp     [CPTR _PM_critHandler],0
        je      @@NoAltHandler

        pushad
        push    _di
        push    _ax
        call    [CPTR _PM_critHandler]  ; Call C code
        _add    sp,4,8
        popad

        pop     _bx
        pop     es
        pop     ds
        iret                    ; Return from interrupt

@@NoAltHandler:
        mov     ax,3            ; Tell MSDOS to fail the operation
        pop     _bx
        pop     es
        pop     ds
        iret                    ; Return from interrupt

cprocend

;----------------------------------------------------------------------------
; int PM_criticalError(int *axVal,int *diVal,int clearFlag)
;----------------------------------------------------------------------------
; Returns the current state of the critical error flags, and the values that
; MSDOS passed in the AX and DI registers to our handler.
;----------------------------------------------------------------------------
cprocstart  PM_criticalError

        ARG     axVal:DPTR, diVal:DPTR, clearFlag:UINT

        enter_c
        pushf                   ; Save interrupt status
        push    es
        mov     ebx,[_PM_critPtr]
        cli                     ; No interrupts thanks!
        xor     _ax,_ax
        xor     _di,_di
        mov     ax,[_ES _bx]
        mov     di,[_ES _bx+2]
        test    [BYTE clearFlag],1
        jz      @@NoClear
        mov     [ULONG _ES _bx],0
@@NoClear:
        _les    _bx,[axVal]
        mov     [_ES _bx],_ax
        _les    _bx,[diVal]
        mov     [_ES _bx],_di
        pop     es
        popf                    ; Restore interrupt status
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_setMouseHandler(int mask, PM_mouseHandler mh)
;----------------------------------------------------------------------------
cprocstart  _PM_setMouseHandler

        ARG     mouseMask:UINT

        enter_c
        push    es

        mov     ax,0Ch          ; AX := Function 12 - install interrupt sub
        mov     _cx,[mouseMask] ; CX := mouse mask
        mov     _dx,offset _PM_mouseISR
        push    cs
        pop     es              ; ES:_DX -> mouse handler
        int     33h             ; Call mouse driver

        pop     es
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_mousePMCB(void)
;----------------------------------------------------------------------------
; Mouse realmode callback routine. Upon entry to this routine, we recieve
; the following from the DPMI server:
;
;   Entry:  DS:_SI  -> Real mode stack at time of call
;           ES:_DI  -> Real mode register data structure
;           SS:_SP  -> Locked protected mode stack to use
;----------------------------------------------------------------------------
cprocfar    _PM_mousePMCB

        pushad
        mov     eax,[es:_di+1Ch]    ; Load register values from real mode
        mov     ebx,[es:_di+10h]
        mov     ecx,[es:_di+18h]
        mov     edx,[es:_di+14h]
        mov     esi,[es:_di+04h]
        mov     edi,[es:_di]
        call    _PM_mouseISR        ; Call the mouse handler
        popad

        mov     ax,[ds:_si]
        mov     [es:_di+2Ah],ax     ; Plug in return IP address
        mov     ax,[ds:_si+2]
        mov     [es:_di+2Ch],ax     ; Plug in return CS value
        add     [WORD es:_di+2Eh],4 ; Remove return address from stack
        iret                        ; Go back to real mode!

cprocend

;----------------------------------------------------------------------------
; void PM_int10PMCB(void)
;----------------------------------------------------------------------------
; int10 realmode callback routine. Upon entry to this routine, we recieve
; the following from the DPMI server:
;
;   Entry:  DS:ESI  -> Real mode stack at time of call
;           ES:EDI  -> Real mode register data structure
;           SS:ESP  -> Locked protected mode stack to use
;----------------------------------------------------------------------------
cprocfar        _PM_int10PMCB

        pushad
        push    ds
        push    es
        push    fs

        pushfd
        pop     eax
        mov     [es:edi+20h],ax     ; Save return flag status
        mov     ax,[ds:esi]
        mov     [es:edi+2Ah],ax     ; Plug in return IP address
        mov     ax,[ds:esi+2]
        mov     [es:edi+2Ch],ax     ; Plug in return CS value
        add     [WORD es:edi+2Eh],4 ; Remove return address from stack

; Call the install int10 handler in protected mode. This function gets called
; with DS set to the current data selector, and ES:EDI pointing the the
; real mode DPMI register structure at the time of the interrupt. The
; handle must be written in assembler to be able to extract the real mode
; register values from the structure

        push    es
        pop     fs                  ; FS:EDI -> real mode registers
        LOAD_DS
        NEWSTK  Int10Stack          ; Switch to local stack

        call    [_PM_int10Handler]

        RESTSTK Int10Stack          ; Restore previous stack
        pop     fs
        pop     es
        pop     ds
        popad
        iret                        ; Go back to real mode!

cprocend

cpublic _PM_pmsmxCodeEnd

endcodeseg  _pmsmx

        END                     ; End of module
