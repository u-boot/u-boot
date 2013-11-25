/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * (C) Copyright 2013 Andreas Bießmann <andreas.devel@googlemail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_OMAP_GPMC_H
#define __ASM_OMAP_GPMC_H

#include <asm/arch/omap_gpmc.h>

#define GPMC_BUF_EMPTY	0
#define GPMC_BUF_FULL	1

#define ECCCLEAR	(0x1 << 8)
#define ECCRESULTREG1	(0x1 << 0)
#define ECCSIZE512BYTE	0xFF
#define ECCSIZE1	(ECCSIZE512BYTE << 22)
#define ECCSIZE0	(ECCSIZE512BYTE << 12)
#define ECCSIZE0SEL	(0x000 << 0)

/* Generic ECC Layouts */
/* Large Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {1, 2, 3, 4, 5, 6, 7, 8,\
		9, 10, 11, 12},\
	.oobfree = {\
		{.offset = 13,\
		 .length = 51 } } \
}
#endif

/* Large Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9,\
		10, 11, 12, 13},\
	.oobfree = {\
		{.offset = 14,\
		 .length = 50 } } \
}
#endif

/* Small Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {1, 2, 3},\
	.oobfree = {\
		{.offset = 4,\
		 .length = 12 } } \
}
#endif

/* Small Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {2, 3, 4},\
	.oobfree = {\
		{.offset = 5,\
		 .length = 11 } } \
}
#endif

enum omap_ecc {
	/* 1-bit  ECC calculation by Software, Error detection by Software */
	OMAP_ECC_HAM1_CODE_SW = 1, /* avoid un-initialized int can be 0x0 */
	/* 1-bit  ECC calculation by GPMC, Error detection by Software */
	/* ECC layout compatible to legacy ROMCODE. */
	OMAP_ECC_HAM1_CODE_HW,
	/* 4-bit  ECC calculation by GPMC, Error detection by Software */
	OMAP_ECC_BCH4_CODE_HW_DETECTION_SW,
	/* 4-bit  ECC calculation by GPMC, Error detection by ELM */
	OMAP_ECC_BCH4_CODE_HW,
	/* 8-bit  ECC calculation by GPMC, Error detection by Software */
	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW,
	/* 8-bit  ECC calculation by GPMC, Error detection by ELM */
	OMAP_ECC_BCH8_CODE_HW,
};

#endif /* __ASM_OMAP_GPMC_H */
