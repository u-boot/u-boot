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


/* 32bit -> 16bit -> 32bit mode switch code */

/*
 * Stack frame at 0xe00
 *	e00 ebx;
 *	e04 ecx;
 *	e08 edx;
 *	e0c esi;
 *	e10 edi;
 *	e14 ebp;
 *	e18 eax;
 *	e1c ds;
 *	e20 es;
 *	e24 fs;
 *	e28 gs;
 *	e2c orig_eax;
 *	e30 eip;
 *	e34 cs;
 *	e38 eflags;
 *	e3c esp;
 *	e40 ss;
 */

#define a32		.byte 0x67;		/* address size prefix 32 */
#define o32		.byte 0x66;		/* operand size prefix 32 */

.section .realmode, "ax"
.code16
						/* 16bit protected mode code here */
.globl realmode_enter
realmode_enter:
o32	pusha
o32	pushf
	cli
	sidt	saved_idt
	sgdt	saved_gdt
	movl	%esp, %eax
	movl	%eax, saved_protected_mode_esp

	movl	$0x10, %eax
	movl	%eax, %esp
	movw	$0x28, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	lidt	realmode_idt_ptr
	movl	%cr0, %eax			/* Go back into real mode by */
	andl	$0x7ffffffe, %eax		/* clearing PE to 0 */
	movl	%eax, %cr0
	ljmp	$0x0,$do_realmode		/* switch to real mode */

do_realmode:					/* realmode code from here */
	movw	%cs,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%fs
	movw	%ax,%gs

						/* create a temporary stack */

	movw	$0xc0, %ax
	movw	%ax, %ss
	movw	$0x200, %ax
	movw	%ax, %sp

	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%eax
	movl	%eax, temp_eax
	popl	%eax
	movw	%ax, %ds
	popl	%eax
	movw	%ax, %es
	popl	%eax
	movw	%ax, %fs
	popl	%eax
	movw	%ax, %gs
	popl	%eax				/* orig_eax */
	popl	%eax
cs	movw	%ax, temp_ip
	popl	%eax
cs	movw	%ax, temp_cs
o32	popf
	popl	%eax
	popw	%ss
	movl	%eax, %esp
cs	movl	temp_eax, %eax
	wbinvd					/* self-modifying code,
						 * better flush the cache */

	.byte	0x9a				/* lcall */
temp_ip:
	.word	0				/* new ip */
temp_cs:
	.word	0				/* new cs */
realmode_ret:
						/* save eax, esp and ss */
cs	movl	%eax, saved_eax
	movl	%esp, %eax
cs	movl	%eax, saved_esp
	movw	%ss, %ax
cs	movw	%ax, saved_ss

	/* restore the stack, note that we set sp to 0x244;
	 * pt_regs is 0x44 bytes long and we push the structure
	 * backwards on to the stack, bottom first */

	movw	$0xc0, %ax
	movw	%ax, %ss
	movw	$0x244, %ax
	movw	%ax, %sp

	xorl	%eax,%eax
cs	movw	saved_ss, %ax
	pushl	%eax
cs	movl	saved_esp, %eax
	pushl	%eax
o32	pushf
	xorl	%eax,%eax
cs	movw	temp_cs, %ax
	pushl	%eax
cs	movw	temp_ip, %ax
	pushl	%eax
	pushl	$0
	movw	%gs, %ax
	pushl	%eax
	movw	%fs, %ax
	pushl	%eax
	movw	%es, %ax
	pushl	%eax
	movw	%ds, %ax
	pushl	%eax
	movl	saved_eax, %eax
	pushl	%eax
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx

o32 cs	lidt	saved_idt
o32 cs	lgdt	saved_gdt			/* Set GDTR */

	movl	%cr0, %eax			/* Go back into protected mode */
	orl	$1,%eax	/* reset PE to 1 */
	movl	%eax, %cr0
	jmp	next_line			/* flush prefetch queue */
next_line:
	movw	$return_ptr, %ax
	movw	%ax,%bp
o32 cs	ljmp	*(%bp)

.code32
protected_mode:
	movl	$0x18,%eax			/* reload GDT[3] */
	movw	%ax,%fs				/* reset FS */
	movw	%ax,%ds				/* reset DS */
	movw	%ax,%gs				/* reset GS */
	movw	%ax,%es				/* reset ES */
	movw	%ax,%ss				/* reset SS */
	movl	saved_protected_mode_esp, %eax
	movl	%eax, %esp
	popf
	popa
	ret

temp_eax:
	.long	0

saved_ss:
	.word	0
saved_esp:
	.long	0
saved_eax:
	.long	0

realmode_idt_ptr:
	.word	0x400
	.word	0x0, 0x0

saved_gdt:
	.word	0, 0, 0, 0
saved_idt:
	.word	0, 0, 0, 0

saved_protected_mode_esp:
	.long	0

return_ptr:
	.long	protected_mode
	.word	0x10
