/*
 * Copyright 2004,2007,2008 Freescale Semiconductor, Inc.
 * (C) Copyright 2002, 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <config.h>
#include <common.h>
#include <asm/fsl_dma.h>

#if defined(CONFIG_MPC85xx)
volatile ccsr_dma_t *dma_base = (void *)(CONFIG_SYS_MPC85xx_DMA_ADDR);
#elif defined(CONFIG_MPC86xx)
volatile ccsr_dma_t *dma_base = (void *)(CONFIG_SYS_MPC86xx_DMA_ADDR);
#else
#error "Freescale DMA engine not supported on your processor"
#endif

static void dma_sync(void)
{
#if defined(CONFIG_MPC85xx)
	asm("sync; isync; msync");
#elif defined(CONFIG_MPC86xx)
	asm("sync; isync");
#endif
}

static uint dma_check(void) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];
	volatile uint status = dma->sr;

	/* While the channel is busy, spin */
	while (status & FSL_DMA_SR_CB)
		status = dma->sr;

	/* clear MR[CS] channel start bit */
	dma->mr &= FSL_DMA_MR_CS;
	dma_sync();

	if (status != 0)
		printf ("DMA Error: status = %x\n", status);

	return status;
}

void dma_init(void) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];

	dma->satr = FSL_DMA_SATR_SREAD_NO_SNOOP;
	dma->datr = FSL_DMA_DATR_DWRITE_NO_SNOOP;
	dma->sr = 0xffffffff; /* clear any errors */
	dma_sync();
}

int dma_xfer(void *dest, uint count, void *src) {
	volatile fsl_dma_t *dma = &dma_base->dma[0];

	dma->dar = (uint) dest;
	dma->sar = (uint) src;
	dma->bcr = count;

	/* Disable bandwidth control, use direct transfer mode */
	dma->mr = FSL_DMA_MR_BWC_DIS | FSL_DMA_MR_CTM_DIRECT;
	dma_sync();

	/* Start the transfer */
	dma->mr = FSL_DMA_MR_BWC_DIS | FSL_DMA_MR_CTM_DIRECT | FSL_DMA_MR_CS;
	dma_sync();

	return dma_check();
}
