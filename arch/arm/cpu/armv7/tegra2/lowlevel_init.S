/*
 * SoC-specific setup info
 *
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
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

#include <config.h>
#include <version.h>


_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	@ sdram load addr from config file

.global invalidate_dcache
invalidate_dcache:
	mov pc, lr

	.align	5
.global reset_cpu
reset_cpu:
	ldr	r1, rstctl			@ get addr for global reset
						@ reg
	ldr	r3, [r1]
	orr	r3, r3, #0x10
	str	r3, [r1]			@ force reset
	mov	r0, r0
_loop_forever:
	b	_loop_forever
rstctl:
	.word	PRM_RSTCTRL

.globl lowlevel_init
lowlevel_init:
	ldr	sp, SRAM_STACK
	str	ip, [sp]
	mov	ip, lr
	bl	s_init				@ go setup pll, mux & memory
	ldr	ip, [sp]
	mov	lr, ip

	mov	pc, lr				@ back to arch calling code


.globl startup_cpu
startup_cpu:
	@ Initialize the AVP, clocks, and memory controller
	@ SDRAM is guaranteed to be on at this point

	ldr     r0, =cold_boot			@ R0 = reset vector for CPU
	bl      start_cpu			@ start the CPU

	@ Transfer control to the AVP code
	bl      halt_avp

	@ Should never get here
_loop_forever2:
	b	_loop_forever2

.globl cache_configure
cache_configure:
	stmdb	r13!,{r14}
	@ invalidate instruction cache
	mov	r1, #0
	mcr	p15, 0, r1, c7, c5, 0

	@ invalidate the i&d tlb entries
	mcr	p15, 0, r1, c8, c5, 0
	mcr	p15, 0, r1, c8, c6, 0

	@ enable instruction cache
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #(1<<12)
	mcr	p15, 0, r1, c1, c0, 0

	bl	enable_scu

	@ enable SMP mode and FW for CPU0, by writing to Auxiliary Ctl reg
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x41
	mcr	p15, 0, r0, c1, c0, 1

	@ Now flush the Dcache
	mov	r0, #0
	@ 256 cache lines
	mov	r1, #256

invalidate_loop:
	add	r1, r1, #-1
	mov	r0, r1, lsl #5
	@ invalidate d-cache using line (way0)
	mcr	p15, 0, r0, c7, c6, 2

	orr	r2, r0, #(1<<30)
	@ invalidate d-cache using line (way1)
	mcr	p15, 0, r2, c7, c6, 2

	orr	r2, r0, #(2<<30)
	@ invalidate d-cache using line (way2)
	mcr	p15, 0, r2, c7, c6, 2

	orr	r2, r0, #(3<<30)
	@ invalidate d-cache using line (way3)
	mcr	p15, 0, r2, c7, c6, 2
	cmp	r1, #0
	bne	invalidate_loop

	@ FIXME: should have ap20's L2 disabled too?
invalidate_done:
	ldmia	r13!,{pc}

.globl cold_boot
cold_boot:
	msr	cpsr_c, #0xD3
	@ Check current processor: CPU or AVP?
	@  If CPU, go to CPU boot code, else continue on AVP path

	ldr	r0, =NV_PA_PG_UP_BASE
	ldr	r1, [r0]
	ldr	r2, =PG_UP_TAG_AVP

	@ are we the CPU?
	ldr	sp, CPU_STACK
	cmp	r1, r2
	@ yep, we are the CPU
	bne	_armboot_start

	@ AVP initialization follows this path
	ldr	sp, AVP_STACK
	@ Init AVP and start CPU
	b	startup_cpu

	@ the literal pools origin
	.ltorg

SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK
AVP_STACK:
	.word EARLY_AVP_STACK
CPU_STACK:
	.word EARLY_CPU_STACK
