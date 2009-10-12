/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelesly stolen from Linux Kernel source tree.
 *
 * ------------------------------------------------------------
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
#ifndef _NAND_DEFS_H_
#define _NAND_DEFS_H_

#include <asm/arch/hardware.h>

#ifdef CONFIG_SOC_DM646X
#define	MASK_CLE	0x80000
#define	MASK_ALE	0x40000
#else
#define	MASK_CLE	0x10
#define	MASK_ALE	0x08
#endif

#define NAND_READ_START		0x00
#define NAND_READ_END		0x30
#define NAND_STATUS		0x70

extern void davinci_nand_init(struct nand_chip *nand);

#endif
