/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author
 *		Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Initial Code from:
 *		Richard Woodruff <r-woodruff2@ti.com>
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

#ifndef _MEM_H_
#define _MEM_H_

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * M_NAND - Micron NAND
 */
#define GPMC_SIZE_256M		0x0
#define GPMC_SIZE_128M		0x8
#define GPMC_SIZE_64M		0xC
#define GPMC_SIZE_32M		0xE
#define GPMC_SIZE_16M		0xF

#define M_NAND_GPMC_CONFIG1	0x00000800
#define M_NAND_GPMC_CONFIG2	0x001e1e00
#define M_NAND_GPMC_CONFIG3	0x001e1e00
#define M_NAND_GPMC_CONFIG4	0x16051807
#define M_NAND_GPMC_CONFIG5	0x00151e1e
#define M_NAND_GPMC_CONFIG6	0x16000f80
#define M_NAND_GPMC_CONFIG7	0x00000008

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS		8
/* max number of GPMC regs */
#define GPMC_MAX_REG		7

#define PISMO1_NOR		1
#define PISMO1_NAND		2
#define PISMO2_CS0		3
#define PISMO2_CS1		4
#define PISMO1_ONENAND		5
#define DBG_MPDB		6
#define PISMO2_NAND_CS0		7
#define PISMO2_NAND_CS1		8

/* make it readable for the gpmc_init */
#define PISMO1_NOR_BASE	FLASH_BASE
#define PISMO1_NAND_BASE	CONFIG_SYS_NAND_BASE
#define PISMO1_NAND_SIZE	GPMC_SIZE_256M

#endif /* endif _MEM_H_ */
