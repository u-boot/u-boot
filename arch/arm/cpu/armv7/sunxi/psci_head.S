/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Based on code by Carl van Schaik <carl@ok-labs.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <linux/linkage.h>

#include <asm/arch-armv7/generictimer.h>
#include <asm/gic.h>
#include <asm/macro.h>
#include <asm/psci.h>
#include <asm/arch/cpu.h>

/*
 * Memory layout:
 *
 * SECURE_RAM to text_end :
 *	._secure_text section
 * text_end to ALIGN_PAGE(text_end):
 *	nothing
 * ALIGN_PAGE(text_end) to ALIGN_PAGE(text_end) + 0x1000)
 *	1kB of stack per CPU (4 CPUs max).
 */

	.pushsection ._secure.text, "ax"

	.arch_extension sec

#define	GICD_BASE		(SUNXI_GIC400_BASE +  0x1000)
#define	GICC_BASE		(SUNXI_GIC400_BASE +  0x2000)

@ {r0, r1, r2, ip} from _do_nonsec_entry(kernel_entry, 0, machid, r2) in
@ arch/arm/lib/bootm.c:boot_jump_linux() must remain unchanged across
@ this function.
ENTRY(psci_arch_init)
	mov	r6, lr
	mov	r7, r0
	bl	psci_get_cpu_id		@ CPU ID => r0
	bl	psci_get_cpu_stack_top	@ stack top => r0
	sub	r0, r0, #4		@ Save space for target PC
	mov	sp, r0
	mov	r0, r7
	mov	lr, r6

	push	{r0, r1, r2, ip, lr}
	bl	sunxi_gic_init
	pop	{r0, r1, r2, ip, pc}
ENDPROC(psci_arch_init)

ENTRY(psci_text_end)
	.popsection
