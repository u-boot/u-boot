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
;*            Based on original code Copyright 1994 Otto Chrons
;*
;* Language:    80386 Assembler, TASM 4.0 or later
;* Environment: IBM PC 32 bit protected mode
;*
;* Description: Low level page fault handler for virtual linear framebuffers.
;*
;****************************************************************************

        IDEAL
        JUMPS

include "scitech.mac"           ; Memory model macros

header      _vflat              ; Set up memory model

VFLAT_START     EQU 0F0000000h
VFLAT_END       EQU 0F03FFFFFh
PAGE_PRESENT    EQU 1
PAGE_NOTPRESENT EQU 0
PAGE_READ       EQU 0
PAGE_WRITE      EQU 2

ifdef   DOS4GW

;----------------------------------------------------------------------------
; DOS4G/W flat linear framebuffer emulation.
;----------------------------------------------------------------------------

begdataseg  _vflat

; Near pointers to the page directory base and our page tables. All of
; this memory is always located in the first Mb of DOS memory.

PDBR            dd  0               ; Page directory base register (CR3)
accessPageAddr  dd  0
accessPageTable dd  0

; CauseWay page directory & 1st page table linear addresses.

CauseWayDIRLinear dd 0
CauseWay1stLinear dd 0

; Place to store a copy of the original Page Table Directory before we
; intialised our virtual buffer code.

pageDirectory:  resd 1024           ; Saved page table directory

ValidCS         dw  0               ; Valid CS for page faults
Ring0CS         dw  0               ; Our ring 0 code selector
LastPage        dd  0               ; Last page we mapped in
BankFuncBuf:    resb 101            ; Place to store bank switch code
BankFuncPtr     dd  offset BankFuncBuf

INT14Gate:
INT14Offset     dd      0           ; eip of original vector
INT14Selector   dw      0           ; cs of original vector

        cextern _PM_savedDS,USHORT
        cextern VF_haveCauseWay,BOOL

enddataseg  _vflat

begcodeseg  _vflat              ; Start of code segment

        cextern VF_malloc,FPTR

;----------------------------------------------------------------------------
; PF_handler64k - Page fault handler for 64k banks
;----------------------------------------------------------------------------
; The handler below is a 32 bit ring 0 page fault handler.  It receives
; control immediately after any page fault or after an IRQ6 (hardware
; interrupt). This provides the fastest possible handling of page faults
; since it jump directly here.  If this is a page fault, the number
; immediately on the stack will be an error code, at offset 4 will be
; the eip of the faulting instruction, at offset 8 will be the cs of the
; faulting instruction.  If it is a hardware interrupt, it will not have
; the error code and the eflags will be at offset 8.
;----------------------------------------------------------------------------
cprocfar    PF_handler64k

; Check if this is a processor exeception or a page fault

        push    eax
        mov     ax,[cs:ValidCS]     ; Use CS override to access data
        cmp     [ss:esp+12],ax      ; Is this a page fault?
        jne     @@ToOldHandler      ; Nope, jump to the previous handler

; Get address of page fault and check if within our handlers range

        mov     eax,cr2             ; EBX has page fault linear address
        cmp     eax,VFLAT_START     ; Is the fault less than ours?
        jb      @@ToOldHandler      ; Yep, go to previous handler
        cmp     eax,VFLAT_END       ; Is the fault more than ours?
        jae     @@ToOldHandler      ; Yep, go to previous handler

; This is our page fault, so we need to handle it

        pushad
        push    ds
        push    es
        mov     ebx,eax             ; EBX := page fault address
        and     ebx,invert 0FFFFh   ; Mask to 64k bank boundary
        mov     ds,[cs:_PM_savedDS]; Load segment registers
        mov     es,[cs:_PM_savedDS]

; Map in the page table for our virtual framebuffer area for modification

        mov     edi,[PDBR]          ; EDI points to page directory
        mov     edx,ebx             ; EDX = linear address
        shr     edx,22              ; EDX = offset to page directory
        mov     edx,[edx*4+edi]     ; EDX = physical page table address
        mov     eax,edx
        mov     edx,[accessPageTable]
        or      eax,7
        mov     [edx],eax
        mov     eax,cr3
        mov     cr3,eax             ; Update page table cache

; Mark all pages valid for the new page fault area

        mov     esi,ebx             ; ESI := linear address for page
        shr     esi,10
        and     esi,0FFFh           ; Offset into page table
        add     esi,[accessPageAddr]
ifdef   USE_NASM
%assign off 0
%rep 16
        or      [DWORD esi+off],0000000001h ; Enable pages
%assign off off+4
%endrep
else
off = 0
REPT    16
        or      [DWORD esi+off],0000000001h ; Enable pages
off = off+4
ENDM
endif

; Mark all pages invalid for the previously mapped area

        xchg    esi,[LastPage]      ; Save last page for next page fault
        test    esi,esi
        jz      @@DoneMapping       ; Dont update if first time round
ifdef   USE_NASM
%assign off 0
%rep 16
        or      [DWORD esi+off],0FFFFFFFEh  ; Disable pages
%assign off off+4
%endrep
else
off = 0
REPT    16
        and     [DWORD esi+off],0FFFFFFFEh  ; Disable pages
off = off+4
ENDM
endif

@@DoneMapping:
        mov     eax,cr3
        mov     cr3,eax             ; Flush the TLB

; Now program the new SuperVGA starting bank address

        mov     eax,ebx             ; EAX := page fault address
        shr     eax,16
        and     eax,0FFh            ; Mask to 0-255
        call    [BankFuncPtr]       ; Call the bank switch function

        pop     es
        pop     ds
        popad
        pop     eax
        add     esp,4               ; Pop the error code from stack
        iretd                       ; Return to faulting instruction

@@ToOldHandler:
        pop     eax
ifdef   USE_NASM
        jmp far dword [cs:INT14Gate]; Chain to previous handler
else
        jmp     [FWORD cs:INT14Gate]; Chain to previous handler
endif

cprocend

;----------------------------------------------------------------------------
; PF_handler4k  - Page fault handler for 4k banks
;----------------------------------------------------------------------------
; The handler below is a 32 bit ring 0 page fault handler.  It receives
; control immediately after any page fault or after an IRQ6 (hardware
; interrupt). This provides the fastest possible handling of page faults
; since it jump directly here.  If this is a page fault, the number
; immediately on the stack will be an error code, at offset 4 will be
; the eip of the faulting instruction, at offset 8 will be the cs of the
; faulting instruction.  If it is a hardware interrupt, it will not have
; the error code and the eflags will be at offset 8.
;----------------------------------------------------------------------------
cprocfar    PF_handler4k

; Fill in when we have tested all the 64Kb code

ifdef   USE_NASM
        jmp far dword [cs:INT14Gate]; Chain to previous handler
else
        jmp     [FWORD cs:INT14Gate]; Chain to previous handler
endif

cprocend

;----------------------------------------------------------------------------
; void InstallFaultHandler(void *baseAddr,int bankSize)
;----------------------------------------------------------------------------
; Installes the page fault handler directly int the interrupt descriptor
; table for maximum performance. This of course requires ring 0 access,
; but none of this stuff will run without ring 0!
;----------------------------------------------------------------------------
cprocstart  InstallFaultHandler

        ARG     baseAddr:ULONG, bankSize:UINT

        enter_c

        mov     [DWORD LastPage],0  ; No pages have been mapped
        mov     ax,cs
        mov     [ValidCS],ax        ; Save CS value for page faults

; Put address of our page fault handler into the IDT directly

        sub     esp,6               ; Allocate space on stack
ifdef   USE_NASM
        sidt    [ss:esp]            ; Store pointer to IDT
else
        sidt    [FWORD ss:esp]      ; Store pointer to IDT
endif
        pop     ax                  ; add esp,2
        pop     eax                 ; Absolute address of IDT
        add     eax,14*8            ; Point to Int #14

; Note that Interrupt gates do not have the high and low word of the
; offset in adjacent words in memory, there are 4 bytes separating them.

        mov     ecx,[eax]           ; Get cs and low 16 bits of offset
        mov     edx,[eax+6]         ; Get high 16 bits of offset in dx
        shl     edx,16
        mov     dx,cx               ; edx has offset
        mov     [INT14Offset],edx   ; Save offset
        shr     ecx,16
        mov     [INT14Selector],cx  ; Save original cs
        mov     [eax+2],cs          ; Install new cs
        mov     edx,offset PF_handler64k
        cmp     [UINT bankSize],4
        jne     @@1
        mov     edx,offset PF_handler4k
@@1:    mov     [eax],dx            ; Install low word of offset
        shr     edx,16
        mov     [eax+6],dx          ; Install high word of offset

        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void RemoveFaultHandler(void)
;----------------------------------------------------------------------------
; Closes down the virtual framebuffer services and restores the previous
; page fault handler.
;----------------------------------------------------------------------------
cprocstart  RemoveFaultHandler

        enter_c

; Remove page fault handler from IDT

        sub     esp,6               ; Allocate space on stack
ifdef   USE_NASM
        sidt    [ss:esp]            ; Store pointer to IDT
else
        sidt    [FWORD ss:esp]      ; Store pointer to IDT
endif

        pop     ax                  ; add esp,2
        pop     eax                 ; Absolute address of IDT
        add     eax,14*8            ; Point to Int #14
        mov     cx,[INT14Selector]
        mov     [eax+2],cx          ; Restore original CS
        mov     edx,[INT14Offset]
        mov     [eax],dx            ; Install low word of offset
        shr     edx,16
        mov     [eax+6],dx          ; Install high word of offset

        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; void InstallBankFunc(int codeLen,void *bankFunc)
;----------------------------------------------------------------------------
; Installs the bank switch function by relocating it into our data segment
; and making it into a callable function. We do it this way to make the
; code identical to the way that the VflatD devices work under Windows.
;----------------------------------------------------------------------------
cprocstart  InstallBankFunc

        ARG     codeLen:UINT, bankFunc:DPTR

        enter_c

        mov     esi,[bankFunc]      ; Copy the code into buffer
        mov     edi,offset BankFuncBuf
        mov     ecx,[codeLen]
    rep movsb
        mov     [BYTE edi],0C3h     ; Terminate the function with a near ret

        leave_c
        ret

cprocend

;----------------------------------------------------------------------------
; int InitPaging(void)
;----------------------------------------------------------------------------
; Initializes paging system. If paging is not enabled, builds a page table
; directory and page tables for physical memory
;
;   Exit:       0   - Successful
;               -1  - Couldn't initialize paging mechanism
;----------------------------------------------------------------------------
cprocstart  InitPaging

        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

; Are we running under CauseWay?

        mov     ax,0FFF9h
        int     31h
        jc      @@NotCauseway
        cmp     ecx,"CAUS"
        jnz     @@NotCauseway
        cmp     edx,"EWAY"
        jnz     @@NotCauseway

        mov     [BOOL VF_haveCauseWay],1
        mov     [CauseWayDIRLinear],esi
        mov     [CauseWay1stLinear],edi

; Check for DPMI

        mov     ax,0ff00h
        push    es
        int     31h
        pop     es
        shr     edi,2
        and     edi,3
        cmp     edi,2
        jz      @@ErrExit               ; Not supported under DPMI

        mov     eax,[CauseWayDIRLinear]
        jmp     @@CopyCR3

@@NotCauseway:
        mov     ax,cs
        test    ax,3                    ; Which ring are we running
        jnz     @@ErrExit               ; Needs zero ring to access
                                        ; page tables (CR3)
        mov     eax,cr0                 ; Load CR0
        test    eax,80000000h           ; Is paging enabled?
        jz      @@ErrExit               ; No, we must have paging!

        mov     eax,cr3                 ; Load directory address
        and     eax,0FFFFF000h

@@CopyCR3:
        mov     [PDBR],eax              ; Save it
        mov     esi,eax
        mov     edi,offset pageDirectory
        mov     ecx,1024
        cld
        rep     movsd                   ; Copy the original page table directory
        cmp     [DWORD accessPageAddr],0; Check if we have allocated page
        jne     @@HaveRealMem           ; table already (we cant free it)

        mov     eax,0100h               ; DPMI DOS allocate
        mov     ebx,8192/16
        int     31h                     ; Allocate 8192 bytes
        and     eax,0FFFFh
        shl     eax,4                   ; EAX points to newly allocated memory
        add     eax,4095
        and     eax,0FFFFF000h          ; Page align
        mov     [accessPageAddr],eax

@@HaveRealMem:
        mov     eax,[accessPageAddr]    ; EAX -> page table in 1st Mb
        shr     eax,12
        and     eax,3FFh                ; Page table offset
        shl     eax,2
        cmp     [BOOL VF_haveCauseWay],0
        jz      @@NotCW0
        mov     ebx,[CauseWay1stLinear]
        jmp     @@Put1st

@@NotCW0:
        mov     ebx,[PDBR]
        mov     ebx,[ebx]
        and     ebx,0FFFFF000h          ; Page table for 1st megabyte

@@Put1st:
        add     eax,ebx
        mov     [accessPageTable],eax
        sub     eax,eax                 ; No error
        jmp     @@Exit

@@ErrExit:
        mov     eax,-1

@@Exit: pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        ret

cprocend

;----------------------------------------------------------------------------
; void ClosePaging(void)
;----------------------------------------------------------------------------
; Closes the paging system
;----------------------------------------------------------------------------
cprocstart  ClosePaging

        push    eax
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     eax,[accessPageAddr]
        call    AccessPage              ; Restore AccessPage mapping
        mov     edi,[PDBR]
        mov     esi,offset pageDirectory
        mov     ecx,1024
        cld
        rep     movsd                   ; Restore the original page table directory

@@Exit: pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     eax
        ret

cprocend

;----------------------------------------------------------------------------
; long AccessPage(long phys)
;----------------------------------------------------------------------------
; Maps a known page to given physical memory
;   Entry:      EAX - Physical memory
;   Exit:       EAX - Linear memory address of mapped phys mem
;----------------------------------------------------------------------------
cprocstatic     AccessPage

        push    edx
        mov     edx,[accessPageTable]
        or      eax,7
        mov     [edx],eax
        mov     eax,cr3
        mov     cr3,eax                 ; Update page table cache
        mov     eax,[accessPageAddr]
        pop     edx
        ret

cprocend

;----------------------------------------------------------------------------
; long GetPhysicalAddress(long linear)
;----------------------------------------------------------------------------
; Returns the physical address of linear address
;   Entry:      EAX - Linear address to convert
;   Exit:       EAX - Physical address
;----------------------------------------------------------------------------
cprocstatic     GetPhysicalAddress

        push    ebx
        push    edx
        mov     edx,eax
        shr     edx,22                  ; EDX is the directory offset
        mov     ebx,[PDBR]
        mov     edx,[edx*4+ebx]         ; Load page table address
        push    eax
        mov     eax,edx
        call    AccessPage              ; Access the page table
        mov     edx,eax
        pop     eax
        shr     eax,12
        and     eax,03FFh               ; EAX offset into page table
        mov     eax,[edx+eax*4]         ; Load physical address
        and     eax,0FFFFF000h
        pop     edx
        pop     ebx
        ret

cprocend

;----------------------------------------------------------------------------
; void CreatePageTable(long pageDEntry)
;----------------------------------------------------------------------------
; Creates a page table for specific address (4MB)
;       Entry:  EAX - Page directory entry (top 10-bits of address)
;----------------------------------------------------------------------------
cprocstatic     CreatePageTable

        push    ebx
        push    ecx
        push    edx
        push    edi
        mov     ebx,eax                 ; Save address
        mov     eax,8192
        push    eax
        call    VF_malloc              ; Allocate page table directory
        add     esp,4
        add     eax,0FFFh
        and     eax,0FFFFF000h          ; Page align (4KB)
        mov     edi,eax                 ; Save page table linear address
        sub     eax,eax                 ; Fill with zero
        mov     ecx,1024
        cld
        rep     stosd                   ; Clear page table
        sub     edi,4096
        mov     eax,edi
        call    GetPhysicalAddress
        mov     edx,[PDBR]
        or      eax,7                   ; Present/write/user bit
        mov     [edx+ebx*4],eax         ; Save physical address into page directory
        mov     eax,cr3
        mov     cr3,eax                 ; Update page table cache
        pop     edi
        pop     edx
        pop     ecx
        pop     ebx
        ret

cprocend

;----------------------------------------------------------------------------
; void MapPhysical2Linear(ulong pAddr, ulong lAddr, int pages, int flags);
;----------------------------------------------------------------------------
; Maps physical memory into linear memory
;   Entry:      pAddr   - Physical address
;               lAddr   - Linear address
;               pages   - Number of 4K pages to map
;               flags   - Page flags
;                           bit 0   =       present
;                           bit 1   =       Read(0)/Write(1)
;----------------------------------------------------------------------------
cprocstart  MapPhysical2Linear

        ARG     pAddr:ULONG, lAddr:ULONG, pages:UINT, pflags:UINT

        enter_c

        and     [ULONG pAddr],0FFFFF000h; Page boundary
        and     [ULONG lAddr],0FFFFF000h; Page boundary
        mov     ecx,[pflags]
        and     ecx,11b                 ; Just two bits
        or      ecx,100b                ; Supervisor bit
        mov     [pflags],ecx

        mov     edx,[lAddr]
        shr     edx,22                  ; EDX = Directory
        mov     esi,[PDBR]
        mov     edi,[pages]             ; EDI page count
        mov     ebx,[lAddr]

@@CreateLoop:
        mov     ecx,[esi+edx*4]         ; Load page table address
        test    ecx,1                   ; Is it present?
        jnz     @@TableOK
        mov     eax,edx
        call    CreatePageTable         ; Create a page table
@@TableOK:
        mov     eax,ebx
        shr     eax,12
        and     eax,3FFh
        sub     eax,1024
        neg     eax                     ; EAX = page count in this table
        inc     edx                     ; Next table
        mov     ebx,0                   ; Next time we'll map 1K pages
        sub     edi,eax                 ; Subtract mapped pages from page count
        jns     @@CreateLoop            ; Create more tables if necessary

        mov     ecx,[pages]             ; ECX = Page count
        mov     esi,[lAddr]
        shr     esi,12                  ; Offset part isn't needed
        mov     edi,[pAddr]
@@MappingLoop:
        mov     eax,esi
        shr     eax,10                  ; EAX = offset to page directory
        mov     ebx,[PDBR]
        mov     eax,[eax*4+ebx]         ; EAX = page table address
        call    AccessPage
        mov     ebx,esi
        and     ebx,3FFh                ; EBX = offset to page table
        mov     edx,edi
        add     edi,4096                ; Next physical address
        inc     esi                     ; Next linear page
        or      edx,[pflags]            ; Update flags...
        mov     [eax+ebx*4],edx         ; Store page table entry
        loop    @@MappingLoop
        mov     eax,cr3
        mov     cr3,eax                 ; Update page table cache

        leave_c
        ret

cprocend

endcodeseg  _vflat

endif

        END                     ; End of module
