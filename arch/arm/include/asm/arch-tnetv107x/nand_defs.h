/*
 * TNETV107X: NAND definitions
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _NAND_DEFS_H_
#define _NAND_DEFS_H_

#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>

#define DAVINCI_ASYNC_EMIF_CNTRL_BASE	TNETV107X_ASYNC_EMIF_CNTRL_BASE

#define	MASK_CLE		0x4000
#define	MASK_ALE		0x2000

#define NAND_READ_START		0x00
#define NAND_READ_END		0x30
#define NAND_STATUS		0x70

extern void davinci_nand_init(struct nand_chip *nand);

#endif
