/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * x86 realmode assembly implementation of a PCI BIOS
 * for platforms that use one PCI hose and configuration
 * access type 1. (The common case for low-end PC's)
 */

#include "bios.h"

#define PCI_BIOS_DEBUG

.section .bios, "ax"
.code16
.globl realmode_pci_bios_call_entry
.hidden realmode_pci_bios_call_entry
.type realmode_pci_bios_call_entry, @function
realmode_pci_bios_call_entry:
	MAKE_BIOS_STACK
	call realmode_pci_bios
	RESTORE_CALLERS_STACK
	ret


.globl realmode_pci_bios
realmode_pci_bios:
gs	movw	OFFS_AX(%bp), %ax
	cmpb	$1, %al
	je	pci_bios_present
	cmpb	$2, %al
	je	pci_bios_find_device
	cmpb	$3, %al
	je	pci_bios_find_class
	cmpb	$6, %al
	je	pci_bios_generate_special_cycle
	cmpb	$8, %al
	je	pci_bios_read_cfg_byte
	cmpb	$9, %al
	je	pci_bios_read_cfg_word
	cmpb	$10, %al
	je	pci_bios_read_cfg_dword
	cmpb	$11, %al
	je	pci_bios_write_cfg_byte
	cmpb	$12, %al
	je	pci_bios_write_cfg_word
	cmpb	$13, %al
	je	pci_bios_write_cfg_dword
	cmpb	$14, %al
	je	pci_bios_get_irq_routing
	cmpb	$15, %al
	je	pci_bios_set_irq
	jmp	unknown_function

/*****************************************************************************/

pci_bios_present:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_present
#endif
	movl	$0x20494350, %eax
gs	movl	%eax, OFFS_EDX(%bp)

	/* We support cfg type 1 version 2.10 */
	movb	$0x01, %al
gs	movb	%al, OFFS_AL(%bp)
	movw	$0x0210, %ax
gs	movw	%ax, OFFS_BX(%bp)

	/* last bus number */
cs	movb	pci_last_bus, %al
gs	movb	%al, OFFS_CL(%bp)
	jmp	clear_carry

/*****************************************************************************/

/* device 0-31, function 0-7 */
pci_bios_find_device:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_find_device
#endif
gs	movw	OFFS_CX(%bp), %di
	shll	$16, %edi
gs	movw	OFFS_DX(%bp), %di
	/* edi now holds device in upper 16 bits and vendor in lower 16 bits */

gs	movw	OFFS_SI(%bp), %si

	/* start at bus 0 dev 0 function 0 */
	xorw	%bx, %bx
pfd_loop:
	/* dword 0 is vendor/device */
	xorw	%ax, %ax
	call	__pci_bios_select_register
	movw	$0xcfc, %dx
	inl	%dx, %eax

	/* our device ? */
	cmpl	%edi, %eax
	je	pfd_found_one
pfd_next_dev:
	/* check for multi function devices */
	movw	%bx, %ax
	andw	$3, %ax
	jnz	pfd_function_not_zero
	movw	$0x000c, %ax
	call	__pci_bios_select_register
	movw	$0xcfe, %dx
	inb	%dx, %al
	andb	$0x80, %al
	jz	pfd_not_multi_function
pfd_function_not_zero:
	/* next function, overflows in to device number, then bus number */
	incw	%bx
	jmp	pfd_check_bus

pfd_not_multi_function:
	/* remove function bits */
	andw	$0xfff8, %bx

	/* next device, overflows in to bus number */
	addw	$0x0008, %bx
pfd_check_bus:
cs	movb	pci_last_bus, %ah
	cmpb	%ah, %bh
	ja	pfd_not_found
	jmp	pfd_loop
pfd_found_one:
	decw	%si
	js	pfd_done
	jmp	pfd_next_dev

pfd_done:
gs	movw	%bx, OFFS_BX(%bp)
	jmp	clear_carry

pfd_not_found:
	/* device not found */
	movb	$0x86, %ah
	jmp	set_carry

/*****************************************************************************/

pci_bios_find_class:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_find_class
#endif
gs	movl	OFFS_ECX(%bp), %edi

	/* edi now holds class-code in lower 24 bits */
	andl	$0x00ffffff, %edi
gs	movw	OFFS_SI(%bp), %si

	/* start at bus 0 dev 0 function 0 */
	xorw	%bx, %bx
pfc_loop:
	/* dword 8 is class-code high 24bits */
	movw	$8, %ax
	call	__pci_bios_select_register
	movw	$0xcfc, %dx
	inl	%dx, %eax
	shrl	$8, %eax
	andl	$0x00ffffff, %eax

	/* our device ? */
	cmpl	%edi, %eax
	je	pfc_found_one
pfc_next_dev:
	/* check for multi function devices */
	andw	$3, %bx
	jnz	pfc_function_not_zero
	movw	$0x000c, %ax
	call	__pci_bios_select_register
	movw	$0xcfe, %dx
	inb	%dx, %al
	andb	$0x80, %al
	jz	pfc_not_multi_function
pfc_function_not_zero:
	/* next function, overflows in to device number, then bus number */
	incw	%bx
	jmp	pfc_check_bus

pfc_not_multi_function:
	/* remove function bits */
	andw	$0xfff8, %bx

	/* next device, overflows in to bus number */
	addw	$0x0008, %bx
pfc_check_bus:
cs	movb	pci_last_bus, %ah
	cmpb	%ah, %bh
	ja	pfc_not_found
	jmp	pfc_loop
pfc_found_one:
	decw	%si
	js	pfc_done
	jmp	pfc_next_dev

pfc_done:
gs	movw	%bx, OFFS_BX(%bp)
	jmp	clear_carry

pfc_not_found:
	/* device not found */
	movb	$0x86, %ah
	jmp	set_carry

/*****************************************************************************/

pci_bios_generate_special_cycle:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_generate_special_cycle
#endif
	/* function not supported */
	movb	$0x81, %ah
	jmp	set_carry

/*****************************************************************************/

pci_bios_read_cfg_byte:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_read_cfg_byte
#endif
	call	pci_bios_select_register
gs	movw	OFFS_DI(%bp), %dx
	andw	$3, %dx
	addw	$0xcfc, %dx
	inb	%dx, %al
gs	movb	%al, OFFS_CL(%bp)
	jmp	clear_carry

/*****************************************************************************/

pci_bios_read_cfg_word:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_read_cfg_word
#endif
	call	pci_bios_select_register
gs	movw	OFFS_DI(%bp), %dx
	andw	$2, %dx
	addw	$0xcfc, %dx
	inw	%dx, %ax
gs	movw	%ax, OFFS_CX(%bp)
	jmp	clear_carry


/*****************************************************************************/

pci_bios_read_cfg_dword:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_read_cfg_dword
#endif
	call	pci_bios_select_register
	movw	$0xcfc, %dx
	inl	%dx, %eax
gs	movl	%eax, OFFS_ECX(%bp)
	jmp	clear_carry

/*****************************************************************************/

pci_bios_write_cfg_byte:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_write_cfg_byte
#endif
	call	pci_bios_select_register
gs	movw	OFFS_DI(%bp), %dx
gs	movb	OFFS_CL(%bp), %al
	andw	$3, %dx
	addw	$0xcfc, %dx
	outb	%al, %dx
	jmp	clear_carry

/*****************************************************************************/

pci_bios_write_cfg_word:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_write_cfg_word
#endif
	call	pci_bios_select_register
gs	movw	OFFS_DI(%bp), %dx
gs	movw	OFFS_CX(%bp), %ax
	andw	$2, %dx
	addw	$0xcfc, %dx
	outw	%ax, %dx
	jmp	clear_carry

/*****************************************************************************/

pci_bios_write_cfg_dword:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_write_cfg_dword
#endif
	call	pci_bios_select_register
gs	movl	OFFS_ECX(%bp), %eax
	movw	$0xcfc, %dx
	outl	%eax, %dx
	jmp	clear_carry

/*****************************************************************************/

pci_bios_get_irq_routing:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_get_irq_routing
#endif
	/* function not supported */
	movb	$0x81, %ah
	jmp	set_carry

/*****************************************************************************/

pci_bios_set_irq:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_set_irq
#endif
	/* function not supported */
	movb	$0x81, %ah
	jmp	set_carry

/*****************************************************************************/

unknown_function:
#ifdef PCI_BIOS_DEBUG
cs	incl	num_pci_bios_unknown_function
#endif
	/* function not supported */
	movb	$0x81, %ah
	jmp	set_carry

/*****************************************************************************/

pci_bios_select_register:
gs	movw	OFFS_BX(%bp), %bx
gs	movw	OFFS_DI(%bp), %ax
/* destroys eax, dx */
__pci_bios_select_register:
	/* BX holds device id, AX holds register index */
	pushl	%ebx
	andl	$0xfc, %eax
	andl	$0xffff, %ebx
	shll	$8, %ebx
	orl	%ebx, %eax
	orl	$0x80000000, %eax
	movw	$0xcf8, %dx
	outl	%eax, %dx
	popl	%ebx
	ret


clear_carry:
gs	movw	OFFS_FLAGS(%bp), %ax

	/* clear carry -- function succeeded */
	andw	$0xfffe, %ax
gs	movw	%ax, OFFS_FLAGS(%bp)
	xorw	%ax, %ax
gs	movb	%ah, OFFS_AH(%bp)
	ret

set_carry:
gs	movb	%ah, OFFS_AH(%bp)
gs	movw	OFFS_FLAGS(%bp), %ax

	/* return carry -- function not supported */
	orw	$1, %ax
gs	movw	%ax, OFFS_FLAGS(%bp)
	movw	$-1, %ax
	ret

/*****************************************************************************/

.globl pci_last_bus
pci_last_bus:
	.byte	0

#ifdef PCI_BIOS_DEBUG
.globl num_pci_bios_present
num_pci_bios_present:
	.long	0

.globl num_pci_bios_find_device
num_pci_bios_find_device:
	.long	0

.globl num_pci_bios_find_class
num_pci_bios_find_class:
	.long	0

.globl num_pci_bios_generate_special_cycle
num_pci_bios_generate_special_cycle:
	.long 0

.globl num_pci_bios_read_cfg_byte
num_pci_bios_read_cfg_byte:
	.long	0

.globl num_pci_bios_read_cfg_word
num_pci_bios_read_cfg_word:
	.long	0

.globl num_pci_bios_read_cfg_dword
num_pci_bios_read_cfg_dword:
	.long	0

.globl num_pci_bios_write_cfg_byte
num_pci_bios_write_cfg_byte:
	.long	0

.globl num_pci_bios_write_cfg_word
num_pci_bios_write_cfg_word:
	.long	0

.globl num_pci_bios_write_cfg_dword
num_pci_bios_write_cfg_dword:
	.long	0

.globl num_pci_bios_get_irq_routing
num_pci_bios_get_irq_routing:
	.long	0

.globl num_pci_bios_set_irq
num_pci_bios_set_irq:
	.long	0

.globl num_pci_bios_unknown_function
num_pci_bios_unknown_function:
	.long	0
#endif
