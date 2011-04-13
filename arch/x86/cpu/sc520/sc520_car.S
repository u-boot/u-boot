/*
 * (C) Copyright 2010-2011
 * Graeme Russ, <graeme.russ@gmail.com>
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
#include <asm/processor-flags.h>
#include <asm/ic/sc520.h>

.section .text

.globl car_init
car_init:
	/*
	 * How to enable Cache-As-RAM for the AMD Elan SC520:
	 *  1. Turn off the CPU Cache (may not be strictly required)
	 *  2. Set code execution PAR (usually the BOOTCS region) to be
	 *     non-cachable
	 *  3. Create a Cachable PAR Region for an area of memory which is
	 *       a) NOT where the code is being executed
	 *       b) NOT SDRAM (Controller not initialised yet)
	 *       c) WILL response to read requests
	 *     The easiest way to do this is to create a second BOOTCS
	 *     PAR mappnig with an address != the PAR in step 2
	 *  4. Issue a wbinvd to invalidate the CPU cache
	 *  5. Turn on the CPU Cache
	 *  6. Read 16kB from the cached PAR region setup in step 3
	 *  7. Turn off the CPU Cache (but DO NOT issue a wbinvd)
	 *
	 * The following code uses PAR2 as the cached PAR (PAR0 and PAR1
	 * are avoided as these are the only two PARs which can be used
	 * as PCI BUS Memory regions which the board might require)
	 *
	 * The configuration of PAR2 must be set in the board configuration
	 * file as CONFIG_SYS_SC520_CAR_PAR
	 */

	/* Configure Cache-As-RAM PAR */
	movl	$CONFIG_SYS_SC520_CAR_PAR, %eax
	movl	$SC520_PAR2, %edi
	movl	%eax, (%edi)

	/* Trash the cache then turn it on */
	wbinvd
	movl	%cr0, %eax
	andl	$~(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0

	/*
	 * The cache is now enabled and empty. Map a region of memory to
	 * it by reading that region.
	 */
	movl	$CONFIG_SYS_CAR_ADDR, %esi
	movl	$CONFIG_SYS_CAR_SIZE, %ecx
	shrl	$2, %ecx			/* we are reading longs */
	cld
	rep	lodsl

	/* Turn off the cache, but don't trash it */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0

	/* Clear the CAR region */
	xorl	%eax, %eax
	movl	$CONFIG_SYS_CAR_ADDR, %edi
	movl	$CONFIG_SYS_CAR_SIZE, %ecx
	shrl	$2, %ecx			/* we are writing longs */
	rep	stosl

	/*
	 * Done - We should now have CONFIG_SYS_CAR_SIZE bytes of
	 * Cache-As-RAM
	 */
	jmp	car_init_ret
