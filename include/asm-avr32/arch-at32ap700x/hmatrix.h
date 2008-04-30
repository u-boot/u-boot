/*
 * Copyright (C) 2008 Atmel Corporation
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
#ifndef __ASM_AVR32_ARCH_HMATRIX_H__
#define __ASM_AVR32_ARCH_HMATRIX_H__

#include <asm/hmatrix-common.h>

/* Bitfields in SFR4 (EBI) */
#define HMATRIX_EBI_SDRAM_ENABLE_OFFSET		1
#define HMATRIX_EBI_SDRAM_ENABLE_SIZE		1
#define HMATRIX_EBI_NAND_ENABLE_OFFSET		3
#define HMATRIX_EBI_NAND_ENABLE_SIZE		1
#define HMATRIX_EBI_CF0_ENABLE_OFFSET		4
#define HMATRIX_EBI_CF0_ENABLE_SIZE		1
#define HMATRIX_EBI_CF1_ENABLE_OFFSET		5
#define HMATRIX_EBI_CF1_ENABLE_SIZE		1
#define HMATRIX_EBI_PULLUP_DISABLE_OFFSET	8
#define HMATRIX_EBI_PULLUP_DISABLE_SIZE		1

/* HSB masters */
#define HMATRIX_MASTER_CPU_DCACHE		0
#define HMATRIX_MASTER_CPU_ICACHE		1
#define HMATRIX_MASTER_PDC			2
#define HMATRIX_MASTER_ISI			3
#define HMATRIX_MASTER_USBA			4
#define HMATRIX_MASTER_LCDC			5
#define HMATRIX_MASTER_MACB0			6
#define HMATRIX_MASTER_MACB1			7
#define HMATRIX_MASTER_DMACA_M0			8
#define HMATRIX_MASTER_DMACA_M1			9

/* HSB slaves */
#define HMATRIX_SLAVE_SRAM0			0
#define HMATRIX_SLAVE_SRAM1			1
#define HMATRIX_SLAVE_PBA			2
#define HMATRIX_SLAVE_PBB			3
#define HMATRIX_SLAVE_EBI			4
#define HMATRIX_SLAVE_USBA			5
#define HMATRIX_SLAVE_LCDC			6
#define HMATRIX_SLAVE_DMACA			7

#endif /* __ASM_AVR32_ARCH_HMATRIX_H__ */
