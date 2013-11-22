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

#define GPMC_BUF_EMPTY	0
#define GPMC_BUF_FULL	1

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
