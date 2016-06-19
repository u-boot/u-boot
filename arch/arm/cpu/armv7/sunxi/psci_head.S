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

ENTRY(psci_arch_init)
	b	sunxi_gic_init
ENDPROC(psci_arch_init)

ENTRY(psci_text_end)
	.popsection
