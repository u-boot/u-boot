/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

#include <common.h>
#include <asm/interrupt.h>

asm (".globl exp_return\n"
     "exp_return:\n"
     "     addl  $12, %esp\n"
     "     pop   %esp\n"
     "     popa\n"
     "     iret\n");

char exception_stack[4096];

/*
 * For detailed description of each exception, refer to:
 * Intel® 64 and IA-32 Architectures Software Developer's Manual
 * Volume 1: Basic Architecture
 * Order Number: 253665-029US, November 2008
 * Table 6-1. Exceptions and Interrupts
 */
DECLARE_EXCEPTION(0, divide_error_entry);
DECLARE_EXCEPTION(1, debug_entry);
DECLARE_EXCEPTION(2, nmi_interrupt_entry);
DECLARE_EXCEPTION(3, breakpoint_entry);
DECLARE_EXCEPTION(4, overflow_entry);
DECLARE_EXCEPTION(5, bound_range_exceeded_entry);
DECLARE_EXCEPTION(6, invalid_opcode_entry);
DECLARE_EXCEPTION(7, device_not_available_entry);
DECLARE_EXCEPTION(8, double_fault_entry);
DECLARE_EXCEPTION(9, coprocessor_segment_overrun_entry);
DECLARE_EXCEPTION(10, invalid_tss_entry);
DECLARE_EXCEPTION(11, segment_not_present_entry);
DECLARE_EXCEPTION(12, stack_segment_fault_entry);
DECLARE_EXCEPTION(13, general_protection_entry);
DECLARE_EXCEPTION(14, page_fault_entry);
DECLARE_EXCEPTION(15, reserved_exception_entry);
DECLARE_EXCEPTION(16, floating_point_error_entry);
DECLARE_EXCEPTION(17, alignment_check_entry);
DECLARE_EXCEPTION(18, machine_check_entry);
DECLARE_EXCEPTION(19, simd_floating_point_exception_entry);
DECLARE_EXCEPTION(20, reserved_exception_entry);
DECLARE_EXCEPTION(21, reserved_exception_entry);
DECLARE_EXCEPTION(22, reserved_exception_entry);
DECLARE_EXCEPTION(23, reserved_exception_entry);
DECLARE_EXCEPTION(24, reserved_exception_entry);
DECLARE_EXCEPTION(25, reserved_exception_entry);
DECLARE_EXCEPTION(26, reserved_exception_entry);
DECLARE_EXCEPTION(27, reserved_exception_entry);
DECLARE_EXCEPTION(28, reserved_exception_entry);
DECLARE_EXCEPTION(29, reserved_exception_entry);
DECLARE_EXCEPTION(30, reserved_exception_entry);
DECLARE_EXCEPTION(31, reserved_exception_entry);

__isr__ reserved_exception_entry(int cause, int ip, int seg)
{
	printf("Reserved Exception %d at %04x:%08x\n", cause, seg, ip);
}

__isr__ divide_error_entry(int cause, int ip, int seg)
{
	printf("Divide Error (Division by zero) at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ debug_entry(int cause, int ip, int seg)
{
	printf("Debug Interrupt (Single step) at %04x:%08x\n", seg, ip);
}

__isr__ nmi_interrupt_entry(int cause, int ip, int seg)
{
	printf("NMI Interrupt at %04x:%08x\n", seg, ip);
}

__isr__ breakpoint_entry(int cause, int ip, int seg)
{
	printf("Breakpoint at %04x:%08x\n", seg, ip);
}

__isr__ overflow_entry(int cause, int ip, int seg)
{
	printf("Overflow at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ bound_range_exceeded_entry(int cause, int ip, int seg)
{
	printf("BOUND Range Exceeded at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ invalid_opcode_entry(int cause, int ip, int seg)
{
	printf("Invalid Opcode (UnDefined Opcode) at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ device_not_available_entry(int cause, int ip, int seg)
{
	printf("Device Not Available (No Math Coprocessor) at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ double_fault_entry(int cause, int ip, int seg)
{
	printf("Double fault at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ coprocessor_segment_overrun_entry(int cause, int ip, int seg)
{
	printf("Co-processor segment overrun at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ invalid_tss_entry(int cause, int ip, int seg)
{
	printf("Invalid TSS at %04x:%08x\n", seg, ip);
}

__isr__ segment_not_present_entry(int cause, int ip, int seg)
{
	printf("Segment Not Present at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ stack_segment_fault_entry(int cause, int ip, int seg)
{
	printf("Stack Segment Fault at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ general_protection_entry(int cause, int ip, int seg)
{
	printf("General Protection at %04x:%08x\n", seg, ip);
}

__isr__ page_fault_entry(int cause, int ip, int seg)
{
	printf("Page fault at %04x:%08x\n", seg, ip);
	while(1);
}

__isr__ floating_point_error_entry(int cause, int ip, int seg)
{
	printf("Floating-Point Error (Math Fault) at %04x:%08x\n", seg, ip);
}

__isr__ alignment_check_entry(int cause, int ip, int seg)
{
	printf("Alignment check at %04x:%08x\n", seg, ip);
}

__isr__ machine_check_entry(int cause, int ip, int seg)
{
	printf("Machine Check at %04x:%08x\n", seg, ip);
}

__isr__ simd_floating_point_exception_entry(int cause, int ip, int seg)
{
	printf("SIMD Floating-Point Exception at %04x:%08x\n", seg, ip);
}

int cpu_init_exceptions(void)
{
	/* Just in case... */
	disable_interrupts();

	/* Setup exceptions */
	set_vector(0x00, exp_0);
	set_vector(0x01, exp_1);
	set_vector(0x02, exp_2);
	set_vector(0x03, exp_3);
	set_vector(0x04, exp_4);
	set_vector(0x05, exp_5);
	set_vector(0x06, exp_6);
	set_vector(0x07, exp_7);
	set_vector(0x08, exp_8);
	set_vector(0x09, exp_9);
	set_vector(0x0a, exp_10);
	set_vector(0x0b, exp_11);
	set_vector(0x0c, exp_12);
	set_vector(0x0d, exp_13);
	set_vector(0x0e, exp_14);
	set_vector(0x0f, exp_15);
	set_vector(0x10, exp_16);
	set_vector(0x11, exp_17);
	set_vector(0x12, exp_18);
	set_vector(0x13, exp_19);
	set_vector(0x14, exp_20);
	set_vector(0x15, exp_21);
	set_vector(0x16, exp_22);
	set_vector(0x17, exp_23);
	set_vector(0x18, exp_24);
	set_vector(0x19, exp_25);
	set_vector(0x1a, exp_26);
	set_vector(0x1b, exp_27);
	set_vector(0x1c, exp_28);
	set_vector(0x1d, exp_29);
	set_vector(0x1e, exp_30);
	set_vector(0x1f, exp_31);

	/* It is now safe to enable interrupts */
	enable_interrupts();

	return 0;
}
