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

typedef struct davinci_emif_regs {
	dv_reg		ERCSR;
	dv_reg		AWCCR;
	dv_reg		SDBCR;
	dv_reg		SDRCR;
	dv_reg		AB1CR;
	dv_reg		AB2CR;
	dv_reg		AB3CR;
	dv_reg		AB4CR;
	dv_reg		SDTIMR;
	dv_reg		DDRSR;
	dv_reg		DDRPHYCR;
	dv_reg		DDRPHYSR;
	dv_reg		TOTAR;
	dv_reg		TOTACTR;
	dv_reg		DDRPHYID_REV;
	dv_reg		SDSRETR;
	dv_reg		EIRR;
	dv_reg		EIMR;
	dv_reg		EIMSR;
	dv_reg		EIMCR;
	dv_reg		IOCTRLR;
	dv_reg		IOSTATR;
	u_int8_t	RSVD0[8];
	dv_reg		NANDFCR;
	dv_reg		NANDFSR;
	u_int8_t	RSVD1[8];
	dv_reg		NANDFECC[4];
	u_int8_t	RSVD2[60];
	dv_reg		NAND4BITECCLOAD;
	dv_reg		NAND4BITECC1;
	dv_reg		NAND4BITECC2;
	dv_reg		NAND4BITECC3;
	dv_reg		NAND4BITECC4;
	dv_reg		NANDERRADD1;
	dv_reg		NANDERRADD2;
	dv_reg		NANDERRVAL1;
	dv_reg		NANDERRVAL2;
} emif_registers;

typedef emif_registers	*emifregs;

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
