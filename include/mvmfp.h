/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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

#ifndef __MVMFP_H
#define __MVMFP_H

/*
 * Header file for MultiFunctionPin (MFP) Configururation framework
 *
 * Processors Supported:
 * 1. Marvell ARMADA100 Processors
 *
 * processor to be supported should be added here
 */

/*
 * MFP configuration is represented by a 32-bit unsigned integer
 */
#define MFP(_off, _pull, _pF, _drv, _dF, _edge, _eF, _afn, _aF) ( \
	/* bits 31..16 - MFP Register Offset */	(((_off) & 0xffff) << 16) | \
	/* bits 15..13 - Run Mode Pull State */	(((_pull) & 0x7) << 13) | \
	/* bit  12     - Unused */ \
	/* bits 11..10 - Driver Strength */	(((_drv) & 0x3) << 10) | \
	/* bit  09     - Pull State flag */	(((_pF) & 0x1) << 9) | \
	/* bit  08     - Drv-strength flag */	(((_dF) & 0x1) << 8) | \
	/* bit  07     - Edge-det flag */	(((_eF) & 0x1) << 7) | \
	/* bits 06..04 - Edge Detection */	(((_edge) & 0x7) << 4) | \
	/* bits 03..00 - Alt-fun flag */	(((_aF) & 0x1) << 3) | \
	/* bits Alternate-fun select */		((_afn) & 0x7))

/*
 * to facilitate the definition, the following macros are provided
 *
 * 				    offset, pull,pF, drv,dF, edge,eF ,afn,aF
 */
#define MFP_OFFSET_MASK		MFP(0xffff,    0,0,    0,0,     0,0,   0,0)
#define MFP_REG(x)		MFP(x,         0,0,    0,0,     0,0,   0,0)
#define MFP_REG_GET_OFFSET(x)	((x & MFP_OFFSET_MASK) >> 16)

#define MFP_AF_FLAG		MFP(0x0000,    0,0,    0,0,     0,0,   0,1)
#define MFP_DRIVE_FLAG		MFP(0x0000,    0,0,    0,1,     0,0,   0,0)
#define MFP_EDGE_FLAG		MFP(0x0000,    0,0,    0,0,     0,1,   0,0)
#define MFP_PULL_FLAG		MFP(0x0000,    0,1,    0,0,     0,0,   0,0)

#define MFP_AF0			MFP(0x0000,    0,0,    0,0,     0,0,   0,1)
#define MFP_AF1			MFP(0x0000,    0,0,    0,0,     0,0,   1,1)
#define MFP_AF2			MFP(0x0000,    0,0,    0,0,     0,0,   2,1)
#define MFP_AF3			MFP(0x0000,    0,0,    0,0,     0,0,   3,1)
#define MFP_AF4			MFP(0x0000,    0,0,    0,0,     0,0,   4,1)
#define MFP_AF5			MFP(0x0000,    0,0,    0,0,     0,0,   5,1)
#define MFP_AF6			MFP(0x0000,    0,0,    0,0,     0,0,   6,1)
#define MFP_AF7			MFP(0x0000,    0,0,    0,0,     0,0,   7,1)
#define MFP_AF_MASK		MFP(0x0000,    0,0,    0,0,     0,0,   7,0)

#define MFP_LPM_EDGE_NONE	MFP(0x0000,    0,0,    0,0,     0,1,   0,0)
#define MFP_LPM_EDGE_RISE	MFP(0x0000,    0,0,    0,0,     1,1,   0,0)
#define MFP_LPM_EDGE_FALL	MFP(0x0000,    0,0,    0,0,     2,1,   0,0)
#define MFP_LPM_EDGE_BOTH	MFP(0x0000,    0,0,    0,0,     3,1,   0,0)
#define MFP_LPM_EDGE_MASK	MFP(0x0000,    0,0,    0,0,     3,0,   0,0)

#define MFP_DRIVE_VERY_SLOW	MFP(0x0000,    0,0,    0,1,     0,0,   0,0)
#define MFP_DRIVE_SLOW		MFP(0x0000,    0,0,    1,1,     0,0,   0,0)
#define MFP_DRIVE_MEDIUM	MFP(0x0000,    0,0,    2,1,     0,0,   0,0)
#define MFP_DRIVE_FAST		MFP(0x0000,    0,0,    3,1,     0,0,   0,0)
#define MFP_DRIVE_MASK		MFP(0x0000,    0,0,    3,0,     0,0,   0,0)

#define MFP_PULL_NONE		MFP(0x0000,    0,1,    0,0,     0,0,   0,0)
#define MFP_PULL_LOW		MFP(0x0000,    1,1,    0,0,     0,0,   0,0)
#define MFP_PULL_HIGH		MFP(0x0000,    2,1,    0,0,     0,0,   0,0)
#define MFP_PULL_BOTH		MFP(0x0000,    3,1,    0,0,     0,0,   0,0)
#define MFP_PULL_FLOAT		MFP(0x0000,    4,1,    0,0,     0,0,   0,0)
#define MFP_PULL_MASK		MFP(0x0000,    7,0,    0,0,     0,0,   0,0)

#define MFP_EOC			0xffffffff	/* indicates end-of-conf */

/* Functions */
void mfp_config(u32 *mfp_cfgs);

#endif /* __MVMFP_H */
