/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelesly stolen from Linux Kernel source tree.
 *
 * ------------------------------------------------------------
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#ifdef CONFIG_SYS_NAND_MASK_CLE
#undef MASK_CLE
#define MASK_CLE CONFIG_SYS_NAND_MASK_CLE
#endif
#ifdef CONFIG_SYS_NAND_MASK_ALE
#undef MASK_ALE
#define MASK_ALE CONFIG_SYS_NAND_MASK_ALE
#endif

#define NAND_READ_START		0x00
#define NAND_READ_END		0x30
#define NAND_STATUS		0x70

extern void davinci_nand_init(struct nand_chip *nand);

#endif
