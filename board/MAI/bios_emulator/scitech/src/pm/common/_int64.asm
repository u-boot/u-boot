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
;* Environment: Intel 32 bit Protected Mode.
;*
;* Description: Code for 64-bit arhithmetic
;*
;****************************************************************************

        IDEAL

include "scitech.mac"

header      _int64

begcodeseg  _int64                  ; Start of code segment

a_low       EQU 04h                 ; Access a_low directly on stack
a_high      EQU 08h                 ; Access a_high directly on stack
b_low       EQU 0Ch                 ; Access b_low directly on stack
shift       EQU 0Ch                 ; Access shift directly on stack
result_2    EQU 0Ch                 ; Access result directly on stack
b_high      EQU 10h                 ; Access b_high directly on stack
result_3    EQU 10h                 ; Access result directly on stack
result_4    EQU 14h                 ; Access result directly on stack

;----------------------------------------------------------------------------
; void _PM_add64(u32 a_low,u32 a_high,u32 b_low,u32 b_high,__u64 *result);
;----------------------------------------------------------------------------
; Adds two 64-bit numbers.
;----------------------------------------------------------------------------
cprocstart  _PM_add64

        mov     eax,[esp+a_low]
        add     eax,[esp+b_low]
        mov     edx,[esp+a_high]
        adc     edx,[esp+b_high]
        mov     ecx,[esp+result_4]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; void _PM_sub64(u32 a_low,u32 a_high,u32 b_low,u32 b_high,__u64 *result);
;----------------------------------------------------------------------------
; Subtracts two 64-bit numbers.
;----------------------------------------------------------------------------
cprocstart  _PM_sub64

        mov     eax,[esp+a_low]
        sub     eax,[esp+b_low]
        mov     edx,[esp+a_high]
        sbb     edx,[esp+b_high]
        mov     ecx,[esp+result_4]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; void _PM_mul64(u32 a_high,u32 a_low,u32 b_high,u32 b_low,__u64 *result);
;----------------------------------------------------------------------------
; Multiples two 64-bit numbers.
;----------------------------------------------------------------------------
cprocstart  _PM_mul64

        mov     eax,[esp+a_high]
        mov     ecx,[esp+b_high]
        or      ecx,eax
        mov     ecx,[esp+b_low]
        jnz     @@FullMultiply
        mov     eax,[esp+a_low]         ; EDX:EAX = b.low * a.low
        mul     ecx
        mov     ecx,[esp+result_4]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

@@FullMultiply:
        push    ebx
        mul     ecx                     ; EDX:EAX = a.high * b.low
        mov     ebx,eax
        mov     eax,[esp+a_low+4]
        mul     [DWORD esp+b_high+4]    ; EDX:EAX = b.high * a.low
        add     ebx,eax
        mov     eax,[esp+a_low+4]
        mul     ecx                     ; EDX:EAX = a.low * b.low
        add     edx,ebx
        pop     ebx
        mov     ecx,[esp+result_4]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; void _PM_div64(u32 a_low,u32 a_high,u32 b_low,u32 b_high,__u64 *result);
;----------------------------------------------------------------------------
; Divides two 64-bit numbers.
;----------------------------------------------------------------------------
cprocstart  _PM_div64

        push    edi
        push    esi
        push    ebx
        xor     edi,edi
        mov     eax,[esp+a_high+0Ch]
        or      eax,eax
        jns     @@ANotNeg

; Dividend is negative, so negate it and save result for later

        inc     edi
        mov     edx,[esp+a_low+0Ch]
        neg     eax
        neg     edx
        sbb     eax,0
        mov     [esp+a_high+0Ch],eax
        mov     [esp+a_low+0Ch],edx

@@ANotNeg:
        mov     eax,[esp+b_high+0Ch]
        or      eax,eax
        jns     @@BNotNeg

; Divisor is negative, so negate it and save result for later

        inc     edi
        mov     edx,[esp+b_low+0Ch]
        neg     eax
        neg     edx
        sbb     eax,0
        mov     [esp+b_high+0Ch],eax
        mov     [esp+b_low+0Ch],edx

@@BNotNeg:
        or      eax,eax
        jnz     @@BHighNotZero

; b.high is zero, so handle this faster

        mov     ecx,[esp+b_low+0Ch]
        mov     eax,[esp+a_high+0Ch]
        xor     edx,edx
        div     ecx
        mov     ebx,eax
        mov     eax,[esp+a_low+0Ch]
        div     ecx
        mov     edx,ebx
        jmp     @@BHighZero

@@BHighNotZero:
        mov     ebx,eax
        mov     ecx,[esp+b_low+0Ch]
        mov     edx,[esp+a_high+0Ch]
        mov     eax,[esp+a_low+0Ch]

; Shift values right until b.high becomes zero

@@ShiftLoop:
        shr     ebx,1
        rcr     ecx,1
        shr     edx,1
        rcr     eax,1
        or      ebx,ebx
        jnz     @@ShiftLoop

; Now complete the divide process

        div     ecx
        mov     esi,eax
        mul     [DWORD esp+b_high+0Ch]
        mov     ecx,eax
        mov     eax,[esp+b_low+0Ch]
        mul     esi
        add     edx,ecx
        jb      @@8
        cmp     edx,[esp+a_high+0Ch]
        ja      @@8
        jb      @@9
        cmp     eax,[esp+a_low+0Ch]
        jbe     @@9
@@8:    dec     esi
@@9:    xor     edx,edx
        mov     eax,esi

@@BHighZero:
        dec     edi
        jnz     @@Done

; The result needs to be negated as either a or b was negative

        neg     edx
        neg     eax
        sbb     edx,0

@@Done: pop     ebx
        pop     esi
        pop     edi
        mov     ecx,[esp+result_4]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; __i64 _PM_shr64(u32 a_low,s32 a_high,s32 shift,__u64 *result);
;----------------------------------------------------------------------------
; Shift a 64-bit number right
;----------------------------------------------------------------------------
cprocstart  _PM_shr64

        mov     eax,[esp+a_low]
        mov     edx,[esp+a_high]
        mov     cl,[esp+shift]
        shrd    edx,eax,cl
        mov     ecx,[esp+result_3]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; __i64 _PM_sar64(u32 a_low,s32 a_high,s32 shift,__u64 *result);
;----------------------------------------------------------------------------
; Shift a 64-bit number right (signed)
;----------------------------------------------------------------------------
cprocstart  _PM_sar64

        mov     eax,[esp+a_low]
        mov     edx,[esp+a_high]
        mov     cl,[esp+shift]
        sar     edx,cl
        rcr     eax,cl
        mov     ecx,[esp+result_3]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; __i64 _PM_shl64(u32 a_low,s32 a_high,s32 shift,__u64 *result);
;----------------------------------------------------------------------------
; Shift a 64-bit number left
;----------------------------------------------------------------------------
cprocstart  _PM_shl64

        mov     eax,[esp+a_low]
        mov     edx,[esp+a_high]
        mov     cl,[esp+shift]
        shld    edx,eax,cl
        mov     ecx,[esp+result_3]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend

;----------------------------------------------------------------------------
; __i64 _PM_neg64(u32 a_low,s32 a_high,__u64 *result);
;----------------------------------------------------------------------------
; Shift a 64-bit number left
;----------------------------------------------------------------------------
cprocstart  _PM_neg64

        mov     eax,[esp+a_low]
        mov     edx,[esp+a_high]
        neg     eax
        neg     edx
        sbb     eax,0
        mov     ecx,[esp+result_2]
        mov     [ecx],eax
        mov     [ecx+4],edx
        ret

cprocend


endcodeseg  _int64

        END
