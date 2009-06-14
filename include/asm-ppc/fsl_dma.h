/*
 * Freescale DMA Controller
 *
 * Copyright 2006 Freescale Semiconductor, Inc.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef _ASM_FSL_DMA_H_
#define _ASM_FSL_DMA_H_

#include <asm/types.h>

typedef struct fsl_dma {
	uint	mr;		/* DMA mode register */
	uint	sr;		/* DMA status register */
	char	res0[4];
	uint	clndar;		/* DMA current link descriptor address register */
	uint	satr;		/* DMA source attributes register */
	uint	sar;		/* DMA source address register */
	uint	datr;		/* DMA destination attributes register */
	uint	dar;		/* DMA destination address register */
	uint	bcr;		/* DMA byte count register */
	char	res1[4];
	uint	nlndar;		/* DMA next link descriptor address register */
	char	res2[8];
	uint	clabdar;	/* DMA current List - alternate base descriptor address Register */
	char	res3[4];
	uint	nlsdar;		/* DMA next list descriptor address register */
	uint	ssr;		/* DMA source stride register */
	uint	dsr;		/* DMA destination stride register */
	char	res4[56];
} fsl_dma_t;

#endif	/* _ASM_DMA_H_ */
