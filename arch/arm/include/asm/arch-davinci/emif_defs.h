/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
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
#ifndef _EMIF_DEFS_H_
#define _EMIF_DEFS_H_

#include <asm/arch/hardware.h>

struct davinci_emif_regs {
	u_int32_t	ercsr;
	u_int32_t	awccr;
	u_int32_t	sdbcr;
	u_int32_t	sdrcr;
	u_int32_t	ab1cr;
	u_int32_t	ab2cr;
	u_int32_t	ab3cr;
	u_int32_t	ab4cr;
	u_int32_t	sdtimr;
	u_int32_t	ddrsr;
	u_int32_t	ddrphycr;
	u_int32_t	ddrphysr;
	u_int32_t	totar;
	u_int32_t	totactr;
	u_int32_t	ddrphyid_rev;
	u_int32_t	sdsretr;
	u_int32_t	eirr;
	u_int32_t	eimr;
	u_int32_t	eimsr;
	u_int32_t	eimcr;
	u_int32_t	ioctrlr;
	u_int32_t	iostatr;
	u_int8_t	rsvd0[8];
	u_int32_t	nandfcr;
	u_int32_t	nandfsr;
	u_int8_t	rsvd1[8];
	u_int32_t	nandfecc[4];
	u_int8_t	rsvd2[60];
	u_int32_t	nand4biteccload;
	u_int32_t	nand4bitecc[4];
	u_int32_t	nanderradd1;
	u_int32_t	nanderradd2;
	u_int32_t	nanderrval1;
	u_int32_t	nanderrval2;
};

#define davinci_emif_regs \
	((struct davinci_emif_regs *)DAVINCI_ASYNC_EMIF_CNTRL_BASE)

#define DAVINCI_NANDFCR_NAND_ENABLE(n)			(1 << (n-2))
#define DAVINCI_NANDFCR_4BIT_ECC_SEL_MASK		(3 << 4)
#define DAVINCI_NANDFCR_4BIT_ECC_SEL(n)			((n-2) << 4)
#define DAVINCI_NANDFCR_1BIT_ECC_START(n)		(1 << (8 + (n-2)))
#define DAVINCI_NANDFCR_4BIT_ECC_START			(1 << 12)
#define DAVINCI_NANDFCR_4BIT_CALC_START			(1 << 13)

/* Chip Select setup */
#define DAVINCI_ABCR_STROBE_SELECT			(1 << 31)
#define DAVINCI_ABCR_EXT_WAIT				(1 << 30)
#define DAVINCI_ABCR_WSETUP(n)				(n << 26)
#define DAVINCI_ABCR_WSTROBE(n)				(n << 20)
#define DAVINCI_ABCR_WHOLD(n)				(n << 17)
#define DAVINCI_ABCR_RSETUP(n)				(n << 13)
#define DAVINCI_ABCR_RSTROBE(n)				(n << 7)
#define DAVINCI_ABCR_RHOLD(n)				(n << 4)
#define DAVINCI_ABCR_TA(n)				(n << 2)
#define DAVINCI_ABCR_ASIZE_16BIT			1
#define DAVINCI_ABCR_ASIZE_8BIT				0

#endif
