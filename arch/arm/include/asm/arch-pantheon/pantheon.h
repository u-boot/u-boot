/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _PANTHEON_H
#define _PANTHEON_H

#ifndef __ASSEMBLY__
#include <asm/types.h>
#include <asm/io.h>
#endif	/* __ASSEMBLY__ */

#include <asm/arch/cpu.h>

/* Common APB clock register bit definitions */
#define APBC_APBCLK     (1<<0)  /* APB Bus Clock Enable */
#define APBC_FNCLK      (1<<1)  /* Functional Clock Enable */
#define APBC_RST        (1<<2)  /* Reset Generation */
/* Functional Clock Selection Mask */
#define APBC_FNCLKSEL(x)        (((x) & 0xf) << 4)

/* Register Base Addresses */
#define PANTHEON_DRAM_BASE	0xB0000000
#define PANTHEON_TIMER_BASE	0xD4014000
#define PANTHEON_WD_TIMER_BASE	0xD4080000
#define PANTHEON_APBC_BASE	0xD4015000
#define PANTHEON_UART1_BASE	0xD4017000
#define PANTHEON_UART2_BASE	0xD4018000
#define PANTHEON_GPIO_BASE	0xD4019000
#define PANTHEON_MFPR_BASE	0xD401E000
#define PANTHEON_MPMU_BASE	0xD4050000
#define PANTHEON_CPU_BASE	0xD4282C00

#endif /* _PANTHEON_H */
