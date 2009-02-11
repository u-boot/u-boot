/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
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
#ifndef __ASM_ARCH_OMAP_GPMC_H
#define __ASM_ARCH_OMAP_GPMC_H

#define GPMC_BUF_EMPTY	0
#define GPMC_BUF_FULL	1

#define ECCCLEAR	(0x1 << 8)
#define ECCRESULTREG1	(0x1 << 0)
#define ECCSIZE512BYTE	0xFF
#define ECCSIZE1	(ECCSIZE512BYTE << 22)
#define ECCSIZE0	(ECCSIZE512BYTE << 12)
#define ECCSIZE0SEL	(0x000 << 0)

/* Generic ECC Layouts */
/* Large Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {1, 2, 3, 4, 5, 6, 7, 8,\
		9, 10, 11, 12},\
	.oobfree = {\
		{.offset = 13,\
		 .length = 51 } } \
}
#endif

/* Large Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9,\
		10, 11, 12, 13},\
	.oobfree = {\
		{.offset = 14,\
		 .length = 50 } } \
}
#endif

/* Small Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {1, 2, 3},\
	.oobfree = {\
		{.offset = 4,\
		 .length = 12 } } \
}
#endif

/* Small Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {2, 3, 4},\
	.oobfree = {\
		{.offset = 5,\
		 .length = 11 } } \
}
#endif

#endif /* __ASM_ARCH_OMAP_GPMC_H */
