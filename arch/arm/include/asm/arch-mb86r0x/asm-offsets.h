/*
 * (C) Copyright 2010
 * Matthias Weisser <weisserm@arcor.de>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef ASM_OFFSETS_H
#define ASM_OFFSETS_H

/*
 * Offset definitions for DDR controller
 */
#define DDR2_DRIC		0x00
#define DDR2_DRIC1		0x02
#define DDR2_DRIC2		0x04
#define DDR2_DRCA		0x06
#define DDR2_DRCM		0x08
#define DDR2_DRCST1		0x0a
#define DDR2_DRCST2		0x0c
#define DDR2_DRCR		0x0e
#define DDR2_DRCF		0x20
#define DDR2_DRASR		0x30
#define DDR2_DRIMS		0x50
#define DDR2_DROS		0x60
#define DDR2_DRIBSODT1		0x64
#define DDR2_DROABA		0x70
#define DDR2_DROBS		0x84

/*
 * Offset definitions Chip Control Module
 */
#define CCNT_CDCRC		0xec

/*
 * Offset definitions clock reset generator
 */
#define CRG_CRPR		0x00
#define CRG_CRHA		0x18
#define CRG_CRPA		0x1c
#define CRG_CRPB		0x20
#define CRG_CRHB		0x24
#define CRG_CRAM		0x28

/*
 * Offset definitions External bus interface
 */
#define MEMC_MCFMODE0		0x00
#define MEMC_MCFMODE2		0x08
#define MEMC_MCFMODE4		0x10
#define MEMC_MCFTIM0		0x20
#define MEMC_MCFTIM2		0x28
#define MEMC_MCFTIM4		0x30
#define MEMC_MCFAREA0		0x40
#define MEMC_MCFAREA2		0x48
#define MEMC_MCFAREA4		0x50

#endif /* ASM_OFFSETS_H */
