/*
 * nand driver definitions to re-use davinci nand driver on Keystone2
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 * (C) Copyright 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef _NAND_DEFS_H_
#define _NAND_DEFS_H_

#include <asm/arch/hardware.h>
#include <linux/mtd/nand.h>

#define MASK_CLE         0x4000
#define	MASK_ALE         0x2000

#define NAND_READ_START  0x00
#define NAND_READ_END    0x30
#define NAND_STATUS      0x70

#endif
