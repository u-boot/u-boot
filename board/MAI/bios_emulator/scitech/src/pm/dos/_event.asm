;****************************************************************************
;*
;*                  SciTech Multi-platform Graphics Library
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
;* Language:    80386 Assembler
;* Environment: IBM PC (MS DOS)
;*
;* Description: Assembly language support routines for the event module.
;*
;****************************************************************************

        ideal

include "scitech.mac"           ; Memory model macros

ifdef flatmodel

header  _event                  ; Set up memory model

begdataseg  _event

    cextern  _EVT_biosPtr,DPTR

ifdef   USE_NASM
%define KB_HEAD     WORD esi+01Ah   ; Keyboard buffer head in BIOS data area
%define KB_TAIL     WORD esi+01Ch   ; Keyboard buffer tail in BIOS data area
%define KB_START    WORD esi+080h   ; Start of keyboard buffer in BIOS data area
%define KB_END      WORD esi+082h   ; End of keyboard buffer in BIOS data area
else
KB_HEAD     EQU WORD esi+01Ah       ; Keyboard buffer head in BIOS data area
KB_TAIL     EQU WORD esi+01Ch       ; Keyboard buffer tail in BIOS data area
KB_START    EQU WORD esi+080h       ; Start of keyboard buffer in BIOS data area
KB_END      EQU WORD esi+082h       ; End of keyboard buffer in BIOS data area
endif

enddataseg  _event

begcodeseg  _event              ; Start of code segment

    cpublic _EVT_codeStart

;----------------------------------------------------------------------------
; int _EVT_getKeyCode(void)
;----------------------------------------------------------------------------
; Returns the key code for the next available key by extracting it from
; the BIOS keyboard buffer.
;----------------------------------------------------------------------------
cprocstart  _EVT_getKeyCode

        enter_c

        mov     esi,[_EVT_biosPtr]
        xor     ebx,ebx
        xor     eax,eax
        mov     bx,[KB_HEAD]
        cmp     bx,[KB_TAIL]
        jz      @@Done
        xor     eax,eax
        mov     ax,[esi+ebx]    ; EAX := character from keyboard buffer
        inc     _bx
        inc     _bx
        cmp     bx,[KB_END]     ; Hit the end of the keyboard buffer?
        jl      @@1
        mov     bx,[KB_START]
@@1:    mov     [KB_HEAD],bx    ; Update keyboard buffer head pointer

@@Done: leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void _EVT_pumpMessages(void)
;----------------------------------------------------------------------------
; This function would normally do nothing, however due to strange bugs
; in the Windows 3.1 and OS/2 DOS boxes, we don't get any hardware keyboard
; interrupts unless we periodically call the BIOS keyboard functions. Hence
; this function gets called every time that we check for events, and works
; around this problem (in essence it tells the DOS VDM to pump the
; keyboard events to our program ;-).
;
; Note that this bug is not present under Win 9x DOS boxes.
;----------------------------------------------------------------------------
cprocstart  _EVT_pumpMessages

        mov     ah,11h          ; Function - Check keyboard status
        int     16h             ; Call BIOS
        
        mov     ax, 0Bh         ; Reset Move Mouse
        int     33h
        ret

cprocend

;----------------------------------------------------------------------------
; int _EVT_disableInt(void);
;----------------------------------------------------------------------------
; Return processor interrupt status and disable interrupts.
;----------------------------------------------------------------------------
cprocstart  _EVT_disableInt

        pushf                   ; Put flag word on stack
        cli                     ; Disable interrupts!
        pop     eax             ; deposit flag word in return register
        ret

cprocend

;----------------------------------------------------------------------------
; void _EVT_restoreInt(int ps);
;----------------------------------------------------------------------------
; Restore processor interrupt status.
;----------------------------------------------------------------------------
cprocstart  _EVT_restoreInt

        ARG     ps:UINT

        push    ebp
        mov     ebp,esp         ; Set up stack frame
        push    [DWORD ps]
        popf                    ; Restore processor status (and interrupts)
        pop     ebp
        ret

cprocend

;----------------------------------------------------------------------------
; int EVT_rdinx(int port,int index)
;----------------------------------------------------------------------------
; Reads an indexed register value from an I/O port.
;----------------------------------------------------------------------------
cprocstart  EVT_rdinx

        ARG     port:UINT, index:UINT

        push    ebp
        mov     ebp,esp
        mov     edx,[port]
        mov     al,[BYTE index]
        out     dx,al
        inc     dx
        in      al,dx
        movzx   eax,al
        pop     ebp
        ret

cprocend

;----------------------------------------------------------------------------
; void EVT_wrinx(int port,int index,int value)
;----------------------------------------------------------------------------
; Writes an indexed register value to an I/O port.
;----------------------------------------------------------------------------
cprocstart  EVT_wrinx

        ARG     port:UINT, index:UINT, value:UINT

        push    ebp
        mov     ebp,esp
        mov     edx,[port]
        mov     al,[BYTE index]
        mov     ah,[BYTE value]
        out     dx,ax
        pop     ebp
        ret

cprocend

    cpublic _EVT_codeEnd

endcodeseg  _event

endif

        END                         ; End of module
