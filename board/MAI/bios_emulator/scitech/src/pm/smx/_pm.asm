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
;*              SMX.
;*
;****************************************************************************

        IDEAL

include "scitech.mac"               ; Memory model macros

header      _pm                     ; Set up memory model

begdataseg  _pm

    cextern _PM_savedDS,USHORT

intel_id        db  "GenuineIntel"  ; Intel vendor ID

enddataseg  _pm

begcodeseg  _pm                 ; Start of code segment

;----------------------------------------------------------------------------
; void PM_segread(PMSREGS *sregs)
;----------------------------------------------------------------------------
; Read the current value of all segment registers
;----------------------------------------------------------------------------
cprocstartdll16 PM_segread

        ARG     sregs:DPTR

        enter_c

        mov     ax,es
        _les    _si,[sregs]
        mov     [_ES _si],ax
        mov     [_ES _si+2],cs
        mov     [_ES _si+4],ss
        mov     [_ES _si+6],ds
        mov     [_ES _si+8],fs
        mov     [_ES _si+10],gs

        leave_c
        ret

cprocend

; Create a table of the 256 different interrupt calls that we can jump
; into

ifdef   USE_NASM

%assign intno 0

intTable:
%rep    256
        db      0CDh
        db      intno
%assign intno   intno + 1
        ret
        nop
%endrep

else

intno = 0

intTable:
        REPT    256
        db      0CDh
        db      intno
intno = intno + 1
        ret
        nop
        ENDM

endif

;----------------------------------------------------------------------------
; _PM_genInt    - Generate the appropriate interrupt
;----------------------------------------------------------------------------
cprocnear   _PM_genInt

        push    _ax                     ; Save _ax
        push    _bx                     ; Save _bx
        mov     ebx,[UINT esp+12]       ; EBX := interrupt number
        mov     _ax,offset intTable     ; Point to interrupt generation table
        shl     _bx,2                   ; _BX := index into table
        add     _ax,_bx                 ; _AX := pointer to interrupt code
        xchg    eax,[esp+4]             ; Restore eax, and set for int
        pop     _bx                     ; restore _bx
        ret

cprocend

;----------------------------------------------------------------------------
; int PM_int386x(int intno, PMREGS *in, PMREGS *out,PMSREGS *sregs)
;----------------------------------------------------------------------------
; Issues a software interrupt in protected mode. This routine has been
; written to allow user programs to load CS and DS with different values
; other than the default.
;----------------------------------------------------------------------------
cprocstartdll16 PM_int386x

        ARG     intno:UINT, inptr:DPTR, outptr:DPTR, sregs:DPTR

        LOCAL   flags:UINT, sv_ds:UINT, sv_esi:ULONG = LocalSize

        enter_c
        push    ds
        push    es                  ; Save segment registers
        push    fs
        push    gs

        _lds    _si,[sregs]         ; DS:_SI -> Load segment registers
        mov     es,[_si]
        mov     bx,[_si+6]
        mov     [sv_ds],_bx         ; Save value of user DS on stack
        mov     fs,[_si+8]
        mov     gs,[_si+10]

        _lds    _si,[inptr]         ; Load CPU registers
        mov     eax,[_si]
        mov     ebx,[_si+4]
        mov     ecx,[_si+8]
        mov     edx,[_si+12]
        mov     edi,[_si+20]
        mov     esi,[_si+16]

        push    ds                  ; Save value of DS
        push    _bp                 ; Some interrupts trash this!
        clc                         ; Generate the interrupt
        push    [UINT intno]
        mov     ds,[WORD sv_ds]     ; Set value of user's DS selector
        call    _PM_genInt
        pop     _bp                 ; Pop intno from stack (flags unchanged)
        pop     _bp                 ; Restore value of stack frame pointer
        pop     ds                  ; Restore value of DS

        pushf                       ; Save flags for later
        pop     [UINT flags]
        push    esi                 ; Save ESI for later
        pop     [DWORD sv_esi]
        push    ds                  ; Save DS for later
        pop     [UINT sv_ds]

        _lds    _si,[outptr]        ; Save CPU registers
        mov     [_si],eax
        mov     [_si+4],ebx
        mov     [_si+8],ecx
        mov     [_si+12],edx
        push    [DWORD sv_esi]
        pop     [DWORD _si+16]
        mov     [_si+20],edi

        mov     _bx,[flags]         ; Return flags
        and     ebx,1h              ; Isolate carry flag
        mov     [_si+24],ebx        ; Save carry flag status

        _lds    _si,[sregs]         ; Save segment registers
        mov     [_si],es
        mov     _bx,[sv_ds]
        mov     [_si+6],bx          ; Get returned DS from stack
        mov     [_si+8],fs
        mov     [_si+10],gs

        pop     gs                  ; Restore segment registers
        pop     fs
        pop     es
        pop     ds
        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_saveDS(void)
;----------------------------------------------------------------------------
; Save the value of DS into a section of the code segment, so that we can
; quickly load this value at a later date in the PM_loadDS() routine from
; inside interrupt handlers etc. The method to do this is different
; depending on the DOS extender being used.
;----------------------------------------------------------------------------
cprocstartdll16 PM_saveDS

        mov     [_PM_savedDS],ds    ; Store away in data segment
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_loadDS(void)
;----------------------------------------------------------------------------
; Routine to load the DS register with the default value for the current
; DOS extender. Only the DS register is loaded, not the ES register, so
; if you wish to call C code, you will need to also load the ES register
; in 32 bit protected mode.
;----------------------------------------------------------------------------
cprocstartdll16 PM_loadDS

        mov     ds,[cs:_PM_savedDS] ; We can access the proper DS through CS
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_setBankA(int bank)
;----------------------------------------------------------------------------
cprocstart      PM_setBankA

        ARG     bank:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        mov     _bx,0
        mov     _ax,4F05h
        mov     _dx,[bank]
        int     10h
        pop     ebx
        pop     ebp
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_setBankAB(int bank)
;----------------------------------------------------------------------------
cprocstart      PM_setBankAB

        ARG     bank:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        mov     _bx,0
        mov     _ax,4F05h
        mov     _dx,[bank]
        int     10h
        mov     _bx,1
        mov     _ax,4F05h
        mov     _dx,[bank]
        int     10h
        pop     ebx
        pop     ebp
        ret

cprocend

;----------------------------------------------------------------------------
; void PM_setCRTStart(int x,int y,int waitVRT)
;----------------------------------------------------------------------------
cprocstart      PM_setCRTStart

        ARG     x:UINT, y:UINT, waitVRT:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        mov     _bx,[waitVRT]
        mov     _cx,[x]
        mov     _dx,[y]
        mov     _ax,4F07h
        int     10h
        pop     ebx
        pop     ebp
        ret

cprocend

;----------------------------------------------------------------------------
; int _PM_inp(int port)
;----------------------------------------------------------------------------
; Reads a byte from the specified port
;----------------------------------------------------------------------------
cprocstart  _PM_inp

        ARG     port:UINT

        push    _bp
        mov     _bp,_sp
        xor     _ax,_ax
        mov     _dx,[port]
        in      al,dx
        pop     _bp
        ret

cprocend

;----------------------------------------------------------------------------
; void _PM_outp(int port,int value)
;----------------------------------------------------------------------------
; Write a byte to the specified port.
;----------------------------------------------------------------------------
cprocstart  _PM_outp

        ARG     port:UINT, value:UINT

        push    _bp
        mov     _bp,_sp
        mov     _dx,[port]
        mov     _ax,[value]
        out     dx,al
        pop     _bp
        ret

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
; uchar _PM_readCMOS(int index)
;----------------------------------------------------------------------------
; Read the value of a specific CMOS register. We do this with both
; normal interrupts and NMI disabled.
;----------------------------------------------------------------------------
cprocstart  _PM_readCMOS

        ARG     index:UINT

        push    _bp
        mov     _bp,_sp
        pushfd
        mov     al,[BYTE index]
        or      al,80h              ; Add disable NMI flag
        cli
        out     70h,al
        IODELAYN 5
        in      al,71h
        mov     ah,al
        xor     al,al
        IODELAYN 5
        out     70h,al              ; Re-enable NMI
        sti
        mov     al,ah               ; Return value in AL
        popfd
        pop     _bp
        ret

cprocend

;----------------------------------------------------------------------------
; void _PM_writeCMOS(int index,uchar value)
;----------------------------------------------------------------------------
; Read the value of a specific CMOS register. We do this with both
; normal interrupts and NMI disabled.
;----------------------------------------------------------------------------
cprocstart  _PM_writeCMOS

        ARG     index:UINT, value:UCHAR

        push    _bp
        mov     _bp,_sp
        pushfd
        mov     al,[BYTE index]
        or      al,80h              ; Add disable NMI flag
        cli
        out     70h,al
        IODELAYN 5
        mov     al,[value]
        out     71h,al
        xor     al,al
        IODELAYN 5
        out     70h,al              ; Re-enable NMI
        sti
        popfd
        pop     _bp
        ret

cprocend

;----------------------------------------------------------------------------
; _PM_getPDB - Return the Page Table Directory Base address
;----------------------------------------------------------------------------
cprocstart  _PM_getPDB

        mov     eax,cr3
        and     eax,0FFFFF000h
        ret

cprocend

;----------------------------------------------------------------------------
; _PM_flushTLB - Flush the Translation Lookaside buffer
;----------------------------------------------------------------------------
cprocstart  PM_flushTLB

        wbinvd                  ; Flush the CPU cache
        mov     eax,cr3         
        mov     cr3,eax         ; Flush the TLB
        ret

cprocend

endcodeseg  _pm

        END                     ; End of module
