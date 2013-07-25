/*
 * TNETV107X: NAND definitions
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
