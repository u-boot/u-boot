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
	u32 csar0;		/* Chip-select Address */
	u32 csmr0;		/* Chip-select Mask */
	u32 cscr0;		/* Chip-select Control */
	u32 csar1;
	u32 csmr1;
	u32 cscr1;
	u32 csar2;
	u32 csmr2;
	u32 cscr2;
	u32 csar3;
	u32 csmr3;
	u32 cscr3;
	u32 csar4;
	u32 csmr4;
	u32 cscr4;
	u32 csar5;
	u32 csmr5;
	u32 cscr5;
	u32 csar6;
	u32 csmr6;
	u32 cscr6;
	u32 csar7;
	u32 csmr7;
	u32 cscr7;
} fbcs_t;

#define FBCS_CSAR_BA(x)			((x) & 0xFFFF0000)

#define FBCS_CSMR_BAM(x)		(((x) & 0xFFFF) << 16)
#define FBCS_CSMR_BAM_MASK		(0x0000FFFF)
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

#ifdef CONFIG_M5249
#define FBCS_CSMR_WP			(0x00000080)
#define FBCS_CSMR_AM			(0x00000040)
#define FBCS_CSMR_CI			(0x00000020)
#define FBCS_CSMR_SC			(0x00000010)
#define FBCS_CSMR_SD			(0x00000008)
#define FBCS_CSMR_UC			(0x00000004)
#define FBCS_CSMR_UD			(0x00000002)
#else
#define FBCS_CSMR_WP			(0x00000100)
#endif
#define FBCS_CSMR_V			(0x00000001)	/* Valid bit */

#define FBCS_CSCR_SWS(x)		(((x) & 0x3F) << 26)
#define FBCS_CSCR_SWS_MASK		(0x03FFFFFF)
#define FBCS_CSCR_SWSEN			(0x00800000)
#define FBCS_CSCR_ASET(x)		(((x) & 0x03) << 20)
#define FBCS_CSCR_ASET_MASK		(0xFFCFFFFF)
#define FBCS_CSCR_RDAH(x)		(((x) & 0x03) << 18)
#define FBCS_CSCR_RDAH_MASK		(0xFFF3FFFF)
#define FBCS_CSCR_WRAH(x)		(((x) & 0x03) << 16)
#define FBCS_CSCR_WRAH_MASK		(0xFFFCFFFF)
#define FBCS_CSCR_WS(x)			(((x) & 0x3F) << 10)
#define FBCS_CSCR_WS_MASK		(0xFFFF03FF)
#define FBCS_CSCR_SBM			(0x00000200)
#define FBCS_CSCR_AA			(0x00000100)
#define FBCS_CSCR_PS(x)			(((x) & 0x03) << 6)
#define FBCS_CSCR_PS_MASK		(0xFFFFFF3F)
#define FBCS_CSCR_BEM			(0x00000020)
#define FBCS_CSCR_BSTR			(0x00000010)
#define FBCS_CSCR_BSTW			(0x00000008)

#define FBCS_CSCR_PS_16			(0x00000080)
#define FBCS_CSCR_PS_8			(0x00000040)
#define FBCS_CSCR_PS_32			(0x00000000)

#endif				/* __FLEXBUS_H */
