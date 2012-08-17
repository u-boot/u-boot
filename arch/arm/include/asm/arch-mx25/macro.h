/*
 * (C) Copyright 2011
 * Matthias Weisser <weisserm@arcor.de>
 *
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Common asm macros for imx25
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

#ifndef __ASM_ARM_ARCH_MACRO_H__
#define __ASM_ARM_ARCH_MACRO_H__
#ifdef __ASSEMBLY__

#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>

.macro init_aips
	write32	IMX_AIPS1_BASE + AIPS_MPR_0_7, 0x77777777
	write32	IMX_AIPS1_BASE + AIPS_MPR_8_15, 0x77777777
	write32	IMX_AIPS2_BASE + AIPS_MPR_0_7, 0x77777777
	write32	IMX_AIPS2_BASE + AIPS_MPR_8_15, 0x77777777
.endm

.macro init_max
	write32	IMX_MAX_BASE + MAX_MPR0, 0x43210
	write32	IMX_MAX_BASE + MAX_MPR1, 0x43210
	write32	IMX_MAX_BASE + MAX_MPR2, 0x43210
	write32	IMX_MAX_BASE + MAX_MPR3, 0x43210
	write32	IMX_MAX_BASE + MAX_MPR4, 0x43210

	write32	IMX_MAX_BASE + MAX_SGPCR0, 0x10
	write32	IMX_MAX_BASE + MAX_SGPCR1, 0x10
	write32	IMX_MAX_BASE + MAX_SGPCR2, 0x10
	write32	IMX_MAX_BASE + MAX_SGPCR3, 0x10
	write32	IMX_MAX_BASE + MAX_SGPCR4, 0x10

	write32	IMX_MAX_BASE + MAX_MGPCR0, 0x0
	write32	IMX_MAX_BASE + MAX_MGPCR1, 0x0
	write32	IMX_MAX_BASE + MAX_MGPCR2, 0x0
	write32	IMX_MAX_BASE + MAX_MGPCR3, 0x0
	write32	IMX_MAX_BASE + MAX_MGPCR4, 0x0
.endm

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARM_ARCH_MACRO_H__ */
