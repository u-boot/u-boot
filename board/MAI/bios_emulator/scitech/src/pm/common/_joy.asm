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
;* Language:    80386 Assembler
;* Environment: Intel x86, any OS
;*
;* Description: Assembly language support routines for reading analogue
;*              joysticks.
;*
;****************************************************************************

        ideal

include "scitech.mac"           ; Memory model macros

ifdef flatmodel

header  _joy                    ; Set up memory model

begcodeseg  _joy                ; Start of code segment

;----------------------------------------------------------------------------
; initTimer
;----------------------------------------------------------------------------
; Sets up 8253 timer 2 (PC speaker) to start timing, but not produce output.
;----------------------------------------------------------------------------
cprocstatic initTimer

; Start timer 2 counting

        in      al,61h
        and     al,0FDh             ; Disable speaker output (just in case)
        or      al,1
        out     61h,al

; Set the timer 2 count to 0 again to start the timing interval.

        mov     al,10110100b        ; set up to load initial (timer 2)
        out     43h,al              ; timer count
        sub     al,al
        out     42h,al              ; load count lsb
        out     42h,al              ; load count msb
        ret

cprocend

;----------------------------------------------------------------------------
; readTimer2
;----------------------------------------------------------------------------
; Reads the number of ticks from the 8253 timer chip using channel 2 (PC
; speaker). This is non-destructive and does not screw up other libraries.
;----------------------------------------------------------------------------
cprocstatic readTimer

        xor     al,al               ; Latch timer 0 command
        out     43h,al              ; Latch timer
        in      al,42h              ; least significant byte
        mov     ah,al
        in      al,42h              ; most significant byte
        xchg    ah,al
        and     eax,0FFFFh
        ret

cprocend

;----------------------------------------------------------------------------
; exitTimer
;----------------------------------------------------------------------------
; Stops the 8253 timer 2 (PC speaker) counting
;----------------------------------------------------------------------------
cprocstatic exitTimer

; Stop timer 2 from counting

        push    eax
        in      al,61h
        and     al,0FEh
        out     61h,al
        
; Some programs have a problem if we change the control port; better change it
; to something they expect (mode 3 - square wave generator)...
        mov     al,0B6h
        out     43h,al
        
        pop     eax
        ret

cprocend

;----------------------------------------------------------------------------
; int _EVT_readJoyAxis(int jmask,int *axis);
;----------------------------------------------------------------------------
; Function to poll the joystick to read the current axis positions.
;----------------------------------------------------------------------------
cprocstart  _EVT_readJoyAxis

        ARG     jmask:UINT, axis:DPTR

        LOCAL   firstTick:UINT, lastTick:UINT, totalTicks:UINT = LocalSize

        enter_c

        mov     ebx,[jmask]
        mov     edi,[axis]
        mov     ecx,(1193180/100)
        and     ebx,01111b          ; Mask out supported axes
        mov     dx,201h             ; DX := joystick I/O port
        call    initTimer           ; Start timer 2 counting
        call    readTimer           ; Returns counter in EAX
        mov     [lastTick],eax

@@WaitStable:
        in      al,dx
        and     al,bl               ; Wait for the axes in question to be
        jz      @@Stable            ;  done reading...
        call    readTimer           ; Returns counter in EAX
        xchg    eax,[lastTick]
        cmp     eax,[lastTick]
        jb      @@1
        sub     eax,[lastTick]
@@1:    add     [totalTicks],eax
        cmp     [totalTicks],ecx    ; Check for timeout
        jae     @@Stable
        jmp     @@WaitStable

@@Stable:
        mov     al,0FFh
        out     dx,al               ; Start joystick reading
        call    initTimer           ; Start timer 2 counting
        call    readTimer           ; Returns counter in EAX
        mov     [firstTick],eax     ; Store initial count
        mov     [lastTick],eax
        mov     [DWORD totalTicks],0
        cli

@@PollLoop:
        in      al,dx               ; Read Joystick port
        not     al
        and     al,bl               ; Mask off channels we don't want to read
        jnz     @@AxisFlipped       ; See if any of the channels flipped
        call    readTimer           ; Returns counter in EAX
        xchg    eax,[lastTick]
        cmp     eax,[lastTick]
        jb      @@2
        sub     eax,[lastTick]
@@2:    add     [totalTicks],eax
        cmp     [totalTicks],ecx    ; Check for timeout
        jae     @@TimedOut
        jmp     @@PollLoop

@@AxisFlipped:
        xor     esi,esi
        mov     ah,1
        test    al,ah
        jnz     @@StoreCount        ; Joystick 1, X axis flipped
        add     esi,4
        mov     ah,2
        test    al,ah
        jnz     @@StoreCount        ; Joystick 1, Y axis flipped
        add     esi,4
        mov     ah,4
        test    al,ah
        jnz     @@StoreCount        ; Joystick 2, X axis flipped
        add     esi,4               ; Joystick 2, Y axis flipped
        mov     ah,8

@@StoreCount:
        or      bh,ah               ; Indicate this axis is active
        xor     bl,ah               ; Unmark the channels that just tripped
        call    readTimer           ; Returns counter in EAX
        xchg    eax,[lastTick]
        cmp     eax,[lastTick]
        jb      @@3
        sub     eax,[lastTick]
@@3:    add     [totalTicks],eax
        mov     eax,[totalTicks]
        mov     [edi+esi],eax       ; Record the time this channel flipped
        cmp     bl,0                ; If there are more channels to read,
        jne     @@PollLoop          ;   keep looping

@@TimedOut:
        sti
        call    exitTimer           ; Stop timer 2 counting
        movzx   eax,bh              ; Return the mask of working axes
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; int _EVT_readJoyButtons(void);
;----------------------------------------------------------------------------
; Function to poll the current joystick buttons
;----------------------------------------------------------------------------
cprocstart  _EVT_readJoyButtons

        mov     dx,0201h
        in      al,dx
        shr     al,4
        not     al
        and     eax,0Fh
        ret

cprocend

endcodeseg  _joy

endif

        END                         ; End of module
