/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Feroceon CPU core 88FR131 Based KW88F6281 SOC.
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

#ifndef _ASM_ARCH_KW88F6281_H
#define _ASM_ARCH_KW88F6281_H

/* SOC specific definitions */
#define KW88F6281_REGS_PHYS_BASE	0xf1000000
#define KW_REGS_PHY_BASE		KW88F6281_REGS_PHYS_BASE

/* TCLK Core Clock definition */
#ifndef CONFIG_SYS_TCLK
#define CONFIG_SYS_TCLK	200000000 /* 200MHz */
#endif

#endif /* _ASM_ARCH_KW88F6281_H */
