/*
 * emif definitions to re-use davinci emif driver on Keystone2
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 * (C) Copyright 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef _EMIF_DEFS_H_
#define _EMIF_DEFS_H_

#include <asm/arch/hardware.h>

struct davinci_emif_regs {
	uint32_t	ercsr;
	uint32_t	awccr;
	uint32_t	sdbcr;
	uint32_t	sdrcr;
	uint32_t	abncr[4];
	uint32_t	sdtimr;
	uint32_t	ddrsr;
	uint32_t	ddrphycr;
	uint32_t	ddrphysr;
	uint32_t	totar;
	uint32_t	totactr;
	uint32_t	ddrphyid_rev;
	uint32_t	sdsretr;
	uint32_t	eirr;
	uint32_t	eimr;
	uint32_t	eimsr;
	uint32_t	eimcr;
	uint32_t	ioctrlr;
	uint32_t	iostatr;
	uint32_t	rsvd0;
	uint32_t	one_nand_cr;
	uint32_t	nandfcr;
	uint32_t	nandfsr;
	uint32_t	rsvd1[2];
	uint32_t	nandfecc[4];
	uint32_t	rsvd2[15];
	uint32_t	nand4biteccload;
	uint32_t	nand4bitecc[4];
	uint32_t	nanderradd1;
	uint32_t	nanderradd2;
	uint32_t	nanderrval1;
	uint32_t	nanderrval2;
};

#define davinci_emif_regs \
	((struct davinci_emif_regs *)DAVINCI_ASYNC_EMIF_CNTRL_BASE)

#define DAVINCI_NANDFCR_NAND_ENABLE(n)			(1 << ((n) - 2))
#define DAVINCI_NANDFCR_4BIT_ECC_SEL_MASK		(3 << 4)
#define DAVINCI_NANDFCR_4BIT_ECC_SEL(n)			(((n) - 2) << 4)
#define DAVINCI_NANDFCR_1BIT_ECC_START(n)		(1 << (8 + ((n) - 2)))
#define DAVINCI_NANDFCR_4BIT_ECC_START			(1 << 12)
#define DAVINCI_NANDFCR_4BIT_CALC_START			(1 << 13)

/* Chip Select setup */
#define DAVINCI_ABCR_STROBE_SELECT			(1 << 31)
#define DAVINCI_ABCR_EXT_WAIT				(1 << 30)
#define DAVINCI_ABCR_WSETUP(n)				((n) << 26)
#define DAVINCI_ABCR_WSTROBE(n)				((n) << 20)
#define DAVINCI_ABCR_WHOLD(n)				((n) << 17)
#define DAVINCI_ABCR_RSETUP(n)				((n) << 13)
#define DAVINCI_ABCR_RSTROBE(n)				((n) << 7)
#define DAVINCI_ABCR_RHOLD(n)				((n) << 4)
#define DAVINCI_ABCR_TA(n)				((n) << 2)
#define DAVINCI_ABCR_ASIZE_16BIT			1
#define DAVINCI_ABCR_ASIZE_8BIT				0

#endif
