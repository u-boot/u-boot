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
;* Environment: IBM PC (MS DOS)
;*
;* Description: Uses the 8253 timer and the BIOS time-of-day count to time
;*              the performance of code that takes less than an hour to
;*              execute.
;*
;*              The routines in this package only works with interrupts
;*              enabled, and in fact will explicitly turn interrupts on
;*              in order to ensure we get accurate results from the timer.
;*
;*  Externally 'C' callable routines:
;*
;*  LZ_timerOn:     Saves the BIOS time of day count and starts the
;*                  long period Zen Timer.
;*
;*  LZ_timerLap:    Latches the current count, and keeps the timer running
;*
;*  LZ_timerOff:    Stops the long-period Zen Timer and saves the timer
;*                  count and the BIOS time of day count.
;*
;*  LZ_timerCount:  Returns an unsigned long representing the timed count
;*                  in microseconds. If more than an hour passed during
;*                  the timing interval, LZ_timerCount will return the
;*                  value 0xFFFFFFFF (an invalid count).
;*
;*  Note:   If either more than an hour passes between calls to LZ_timerOn
;*          and LZ_timerOff, an error is reported. For timing code that takes
;*          more than a few minutes to execute, use the low resolution
;*          Ultra Long Period Zen Timer code, which should be accurate
;*          enough for most purposes.
;*
;*  Note:   Each block of code being timed should ideally be run several
;*          times, with at least two similar readings required to
;*          establish a true measurement, in order to eliminate any
;*          variability caused by interrupts.
;*
;*  Note:   Interrupts must not be disabled for more than 54 ms at a
;*          stretch during the timing interval. Because interrupts are
;*          enabled, key, mice, and other devices that generate interrupts
;*          should not be used during the timing interval.
;*
;*  Note:   Any extra code running off the timer interrupt (such as
;*          some memory resident utilities) will increase the time
;*          measured by the Zen Timer.
;*
;*  Note:   These routines can introduce inaccuracies of up to a few
;*          tenths of a second into the system clock count for each
;*          code section being timed. Consequently, it's a good idea to
;*          reboot at the conclusion of timing sessions. (The
;*          battery-backed clock, if any, is not affected by the Zen
;*          timer.)
;*
;*  All registers and all flags are preserved by all routines, except
;*  interrupts which are always turned on
;*
;****************************************************************************

        IDEAL

include "scitech.mac"

;****************************************************************************
;
; Equates used by long period Zen Timer
;
;****************************************************************************

; Base address of 8253 timer chip

BASE_8253       equ     40h

; The address of the timer 0 count registers in the 8253

TIMER_0_8253    equ     BASE_8253 + 0

; The address of the mode register in the 8253

MODE_8253       equ     BASE_8253 + 3

; The address of the BIOS timer count variable in the BIOS data area.

TIMER_COUNT     equ     6Ch

; Macro to delay briefly to ensure that enough time has elapsed between
; successive I/O accesses so that the device being accessed can respond
; to both accesses even on a very fast PC.

ifdef   USE_NASM
%macro  DELAY 0
        jmp     short $+2
        jmp     short $+2
        jmp     short $+2
%endmacro
else
macro   DELAY
        jmp     short $+2
        jmp     short $+2
        jmp     short $+2
endm
endif

header      _lztimer

begdataseg  _lztimer

        cextern  _ZTimerBIOSPtr,DPTR

StartBIOSCount      dd  0       ; Starting BIOS count dword
EndBIOSCount        dd  0       ; Ending BIOS count dword
EndTimedCount       dw  0       ; Timer 0 count at the end of timing period

enddataseg  _lztimer

begcodeseg  _lztimer                ; Start of code segment

;----------------------------------------------------------------------------
; void LZ_timerOn(void);
;----------------------------------------------------------------------------
; Starts the Long period Zen timer counting.
;----------------------------------------------------------------------------
cprocstart  LZ_timerOn

; Set the timer 0 of the 8253 to mode 2 (divide-by-N), to cause
; linear counting rather than count-by-two counting. Also stops
; timer 0 until the timer count is loaded, except on PS/2 computers.

        mov     al,00110100b        ; mode 2
        out     MODE_8253,al

; Set the timer count to 0, so we know we won't get another timer
; interrupt right away. Note: this introduces an inaccuracy of up to 54 ms
; in the system clock count each time it is executed.

        DELAY
        sub     al,al
        out     TIMER_0_8253,al     ; lsb
        DELAY
        out     TIMER_0_8253,al     ; msb

; Store the timing start BIOS count

        use_es
ifdef   flatmodel
        mov     ebx,[_ZTimerBIOSPtr]
else
        les     bx,[_ZTimerBIOSPtr]
endif
        cli                         ; No interrupts while we grab the count
        mov     eax,[_ES _bx+TIMER_COUNT]
        sti
        mov     [StartBIOSCount],eax
        unuse_es

; Set the timer count to 0 again to start the timing interval.

        mov     al,00110100b        ; set up to load initial
        out     MODE_8253,al        ; timer count
        DELAY
        sub     al,al
        out     TIMER_0_8253,al     ; load count lsb
        DELAY
        out     TIMER_0_8253,al     ; load count msb

        ret

cprocend

;----------------------------------------------------------------------------
; void LZ_timerOff(void);
;----------------------------------------------------------------------------
; Stops the long period Zen timer and saves count.
;----------------------------------------------------------------------------
cprocstart  LZ_timerOff

; Latch the timer count.

        mov     al,00000000b        ; latch timer 0
        out     MODE_8253,al
        cli                         ; Stop the BIOS count

; Read the BIOS count. (Since interrupts are disabled, the BIOS
; count won't change).

        use_es
ifdef   flatmodel
        mov     ebx,[_ZTimerBIOSPtr]
else
        les     bx,[_ZTimerBIOSPtr]
endif
        mov     eax,[_ES _bx+TIMER_COUNT]
        mov     [EndBIOSCount],eax
        unuse_es

; Read out the count we latched earlier.

        in      al,TIMER_0_8253     ; least significant byte
        DELAY
        mov     ah,al
        in      al,TIMER_0_8253     ; most significant byte
        xchg    ah,al
        neg     ax                  ; Convert from countdown remaining
                                    ;  to elapsed count
        mov     [EndTimedCount],ax
        sti                         ; Let the BIOS count continue

        ret

cprocend

;----------------------------------------------------------------------------
; unsigned long LZ_timerLap(void)
;----------------------------------------------------------------------------
; Latches the current count and converts it to a microsecond timing value,
; but leaves the timer still running. We dont check for and overflow,
; where the time has gone over an hour in this routine, since we want it
; to execute as fast as possible.
;----------------------------------------------------------------------------
cprocstart  LZ_timerLap

        push    ebx                 ; Save EBX for 32 bit code

; Latch the timer count.

        mov     al,00000000b        ; latch timer 0
        out     MODE_8253,al
        cli                         ; Stop the BIOS count

; Read the BIOS count. (Since interrupts are disabled, the BIOS
; count wont change).

        use_es
ifdef   flatmodel
        mov     ebx,[_ZTimerBIOSPtr]
else
        les     bx,[_ZTimerBIOSPtr]
endif
        mov     eax,[_ES _bx+TIMER_COUNT]
        mov     [EndBIOSCount],eax
        unuse_es

; Read out the count we latched earlier.

        in      al,TIMER_0_8253     ; least significant byte
        DELAY
        mov     ah,al
        in      al,TIMER_0_8253     ; most significant byte
        xchg    ah,al
        neg     ax                  ; Convert from countdown remaining
                                    ;  to elapsed count
        mov     [EndTimedCount],ax
        sti                         ; Let the BIOS count continue

; See if a midnight boundary has passed and adjust the finishing BIOS
; count by the number of ticks in 24 hours. We wont be able to detect
; more than 24 hours, but at least we can time across a midnight
; boundary

        mov     eax,[EndBIOSCount]      ; Is end < start?
        cmp     eax,[StartBIOSCount]
        jae     @@CalcBIOSTime          ; No, calculate the time taken

; Adjust the finishing time by adding the number of ticks in 24 hours
; (1573040).

        add     [DWORD EndBIOSCount],1800B0h

; Convert the BIOS time to microseconds

@@CalcBIOSTime:
        mov     ax,[WORD EndBIOSCount]
        sub     ax,[WORD StartBIOSCount]
        mov     dx,54925            ; Number of microseconds each
                                    ;  BIOS count represents.
        mul     dx
        mov     bx,ax               ; set aside BIOS count in
        mov     cx,dx               ;  microseconds

; Convert timer count to microseconds

        push    _si
        mov     ax,[EndTimedCount]
        mov     si,8381
        mul     si
        mov     si,10000
        div     si                  ; * 0.8381 = * 8381 / 10000
        pop     _si

; Add the timer and BIOS counts together to get an overall time in
; microseconds.

        add     ax,bx
        adc     cx,0
ifdef flatmodel
        shl     ecx,16
        mov     cx,ax
        mov     eax,ecx             ; EAX := timer count
else
        mov     dx,cx
endif
        pop     ebx                 ; Restore EBX for 32 bit code
        ret

cprocend

;----------------------------------------------------------------------------
; unsigned long LZ_timerCount(void);
;----------------------------------------------------------------------------
; Returns an unsigned long representing the net time in microseconds.
;
; If an hour has passed while timing, we return 0xFFFFFFFF as the count
; (which is not a possible count in itself).
;----------------------------------------------------------------------------
cprocstart  LZ_timerCount

        push    ebx                 ; Save EBX for 32 bit code

; See if a midnight boundary has passed and adjust the finishing BIOS
; count by the number of ticks in 24 hours. We wont be able to detect
; more than 24 hours, but at least we can time across a midnight
; boundary

        mov     eax,[EndBIOSCount]      ; Is end < start?
        cmp     eax,[StartBIOSCount]
        jae     @@CheckForHour          ; No, check for hour passing

; Adjust the finishing time by adding the number of ticks in 24 hours
; (1573040).

        add     [DWORD EndBIOSCount],1800B0h

; See if more than an hour passed during timing. If so, notify the user.

@@CheckForHour:
        mov     ax,[WORD StartBIOSCount+2]
        cmp     ax,[WORD EndBIOSCount+2]
        jz      @@CalcBIOSTime      ; Hour count didn't change, so
                                    ;  everything is fine

        inc     ax
        cmp     ax,[WORD EndBIOSCount+2]
        jnz     @@TestTooLong       ; Two hour boundaries passed, so the
                                    ;  results are no good
        mov     ax,[WORD EndBIOSCount]
        cmp     ax,[WORD StartBIOSCount]
        jb      @@CalcBIOSTime      ; a single hour boundary passed. That's
                                    ; OK, so long as the total time wasn't
                                    ; more than an hour.

; Over an hour elapsed passed during timing, which renders
; the results invalid. Notify the user. This misses the case where a
; multiple of 24 hours has passed, but we'll rely on the perspicacity of
; the user to detect that case :-).

@@TestTooLong:
ifdef   flatmodel
        mov     eax,0FFFFFFFFh
else
        mov     ax,0FFFFh
        mov     dx,0FFFFh
endif
        jmp     short @@Done

; Convert the BIOS time to microseconds

@@CalcBIOSTime:
        mov     ax,[WORD EndBIOSCount]
        sub     ax,[WORD StartBIOSCount]
        mov     dx,54925            ; Number of microseconds each
                                    ;  BIOS count represents.
        mul     dx
        mov     bx,ax               ; set aside BIOS count in
        mov     cx,dx               ;  microseconds

; Convert timer count to microseconds

        push    _si
        mov     ax,[EndTimedCount]
        mov     si,8381
        mul     si
        mov     si,10000
        div     si                  ; * 0.8381 = * 8381 / 10000
        pop     _si

; Add the timer and BIOS counts together to get an overall time in
; microseconds.

        add     ax,bx
        adc     cx,0
ifdef flatmodel
        shl     ecx,16
        mov     cx,ax
        mov     eax,ecx             ; EAX := timer count
else
        mov     dx,cx
endif

@@Done: pop     ebx                 ; Restore EBX for 32 bit code
        ret

cprocend

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
