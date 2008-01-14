/*
 * FlexBus Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef __FLEXBUS_H
#define __FLEXBUS_H

/*********************************************************************
* FlexBus Chip Selects (FBCS)
*********************************************************************/

typedef struct fbcs {
	u32 csar0;		/* Chip-select Address Register */
	u32 csmr0;		/* Chip-select Mask Register */
	u32 cscr0;		/* Chip-select Control Register */
	u32 csar1;		/* Chip-select Address Register */
	u32 csmr1;		/* Chip-select Mask Register */
	u32 cscr1;		/* Chip-select Control Register */
	u32 csar2;		/* Chip-select Address Register */
	u32 csmr2;		/* Chip-select Mask Register */
	u32 cscr2;		/* Chip-select Control Register */
	u32 csar3;		/* Chip-select Address Register */
	u32 csmr3;		/* Chip-select Mask Register */
	u32 cscr3;		/* Chip-select Control Register */
	u32 csar4;		/* Chip-select Address Register */
	u32 csmr4;		/* Chip-select Mask Register */
	u32 cscr4;		/* Chip-select Control Register */
	u32 csar5;		/* Chip-select Address Register */
	u32 csmr5;		/* Chip-select Mask Register */
	u32 cscr5;		/* Chip-select Control Register */
} fbcs_t;

/* Bit definitions and macros for CSAR group */
#define FBCS_CSAR_BA(x)			((x)&0xFFFF0000)

/* Bit definitions and macros for CSMR group */
#define FBCS_CSMR_V			(0x00000001)	/* Valid bit */
#define FBCS_CSMR_WP			(0x00000100)	/* Write protect */
#define FBCS_CSMR_BAM(x)		(((x)&0x0000FFFF)<<16)	/* Base address mask */
#define FBCS_CSMR_BAM_4G		(0xFFFF0000)
#define FBCS_CSMR_BAM_2G		(0x7FFF0000)
#define FBCS_CSMR_BAM_1G		(0x3FFF0000)
#define FBCS_CSMR_BAM_1024M		(0x3FFF0000)
#define FBCS_CSMR_BAM_512M		(0x1FFF0000)
#define FBCS_CSMR_BAM_256M		(0x0FFF0000)
#define FBCS_CSMR_BAM_128M		(0x07FF0000)
#define FBCS_CSMR_BAM_64M		(0x03FF0000)
#define FBCS_CSMR_BAM_32M		(0x01FF0000)
#define FBCS_CSMR_BAM_16M		(0x00FF0000)
#define FBCS_CSMR_BAM_8M		(0x007F0000)
#define FBCS_CSMR_BAM_4M		(0x003F0000)
#define FBCS_CSMR_BAM_2M		(0x001F0000)
#define FBCS_CSMR_BAM_1M		(0x000F0000)
#define FBCS_CSMR_BAM_1024K		(0x000F0000)
#define FBCS_CSMR_BAM_512K		(0x00070000)
#define FBCS_CSMR_BAM_256K		(0x00030000)
#define FBCS_CSMR_BAM_128K		(0x00010000)
#define FBCS_CSMR_BAM_64K		(0x00000000)

/* Bit definitions and macros for CSCR group */
#define FBCS_CSCR_BSTW			(0x00000008)	/* Burst-write enable */
#define FBCS_CSCR_BSTR			(0x00000010)	/* Burst-read enable */
#define FBCS_CSCR_BEM			(0x00000020)	/* Byte-enable mode */
#define FBCS_CSCR_PS(x)			(((x)&0x00000003)<<6)	/* Port size */
#define FBCS_CSCR_AA			(0x00000100)	/* Auto-acknowledge */
#define FBCS_CSCR_WS(x)			(((x)&0x0000003F)<<10)	/* Wait states */
#define FBCS_CSCR_WRAH(x)		(((x)&0x00000003)<<16)	/* Write address hold or deselect */
#define FBCS_CSCR_RDAH(x)		(((x)&0x00000003)<<18)	/* Read address hold or deselect */
#define FBCS_CSCR_ASET(x)		(((x)&0x00000003)<<20)	/* Address setup */
#define FBCS_CSCR_SWSEN			(0x00800000)	/* Secondary wait state enable */
#define FBCS_CSCR_SWS(x)		(((x)&0x0000003F)<<26)	/* Secondary wait states */

#define FBCS_CSCR_PS_8			(0x00000040)
#define FBCS_CSCR_PS_16			(0x00000080)
#define FBCS_CSCR_PS_32			(0x00000000)

#endif				/* __FLEXBUS_H */
