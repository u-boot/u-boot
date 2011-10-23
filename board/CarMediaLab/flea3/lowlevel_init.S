/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011, Stefano Babic <sbabic@denx.de>
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
#include <asm-offsets.h>
#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>

/*
 * Configuration for the flea3 board.
 * These defines are used by the included macros and must
 * be defined first
 */
#define AIPS_MPR_CONFIG		0x77777777
#define AIPS_OPACR_CONFIG	0x00000000

/* MPR - priority is M4 > M2 > M3 > M5 > M0 > M1 */
#define MAX_MPR_CONFIG		0x00302154

/* SGPCR - always park on last master */
#define MAX_SGPCR_CONFIG	0x00000010

/* MGPCR - restore default values */
#define MAX_MGPCR_CONFIG	0x00000000

/*
 * M3IF Control Register (M3IFCTL)
 * MRRP[0] = L2CC0 not on priority list (0 << 0) = 0x00000000
 * MRRP[1] = L2CC1 not on priority list (0 << 0) = 0x00000000
 * MRRP[2] = MBX not on priority list (0 << 0)   = 0x00000000
 * MRRP[3] = MAX1 not on priority list (0 << 0)  = 0x00000000
 * MRRP[4] = SDMA not on priority list (0 << 0)  = 0x00000000
 * MRRP[5] = MPEG4 not on priority list (0 << 0) = 0x00000000
 * MRRP[6] = IPU1 on priority list (1 << 6)      = 0x00000040
 * MRRP[7] = IPU2 not on priority list (0 << 0)  = 0x00000000
 *                                               ------------
 *                                                 0x00000040
 */
#define M3IF_CONFIG		0x00000040

#define CCM_PDR0_CONFIG		0x00801000

/*
 * includes MX35 utility macros
 */
#include <asm/arch/lowlevel_macro.S>

.globl lowlevel_init
lowlevel_init:

	core_init

	init_aips

	init_max

	init_m3if

	mov pc, lr
