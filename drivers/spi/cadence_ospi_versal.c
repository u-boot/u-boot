// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018 Xilinx
 *
 * Cadence QSPI controller DMA operations
 */

#include <clk.h>
#include <common.h>
#include <memalign.h>
#include <wait_bit.h>
#include <asm/io.h>
#include "cadence_qspi.h"

#define CQSPI_DMA_DST_ADDR_REG			0x1800
#define CQSPI_DMA_DST_SIZE_REG			0x1804
#define CQSPI_DMA_DST_STS_REG			0x1808
#define CQSPI_DMA_DST_CTRL_REG			0x180C
#define CQSPI_DMA_DST_I_STS_REG			0x1814
#define CQSPI_DMA_DST_I_ENBL_REG		0x1818
#define CQSPI_DMA_DST_I_DISBL_REG		0x181C
#define CQSPI_DMA_DST_CTRL2_REG			0x1824
#define CQSPI_DMA_DST_ADDR_MSB_REG		0x1828

#define CQSPI_DMA_SRC_RD_ADDR_REG		0x1000

#define CQSPI_REG_DMA_PERIPH_CFG		0x20
#define CQSPI_REG_INDIR_TRIG_ADDR_RANGE		0x80
#define CQSPI_DFLT_INDIR_TRIG_ADDR_RANGE	6
#define CQSPI_DFLT_DMA_PERIPH_CFG		0x602
#define CQSPI_DFLT_DST_CTRL_REG_VAL		0xF43FFA00

#define CQSPI_DMA_DST_I_STS_DONE		BIT(1)
#define CQSPI_DMA_TIMEOUT			10000000

void cadence_qspi_apb_dma_read(struct cadence_spi_platdata *plat,
			       unsigned int n_rx, u8 *rxbuf)
{
	writel(CQSPI_DFLT_INDIR_TRIG_ADDR_RANGE,
	       plat->regbase + CQSPI_REG_INDIR_TRIG_ADDR_RANGE);
	writel(CQSPI_DFLT_DMA_PERIPH_CFG,
	       plat->regbase + CQSPI_REG_DMA_PERIPH_CFG);
	writel((unsigned long)rxbuf, plat->regbase + CQSPI_DMA_DST_ADDR_REG);
	writel(0x0, plat->regbase + CQSPI_DMA_SRC_RD_ADDR_REG);
	writel(roundup(n_rx, 4), plat->regbase + CQSPI_DMA_DST_SIZE_REG);
	flush_dcache_range((unsigned long)rxbuf, (unsigned long)rxbuf + n_rx);
	writel(CQSPI_DFLT_DST_CTRL_REG_VAL,
	       plat->regbase + CQSPI_DMA_DST_CTRL_REG);
}

int cadence_qspi_apb_wait_for_dma_cmplt(struct cadence_spi_platdata *plat)
{
	u32 timeout = CQSPI_DMA_TIMEOUT;

	while (!(readl(plat->regbase + CQSPI_DMA_DST_I_STS_REG) &
		 CQSPI_DMA_DST_I_STS_DONE) && timeout--)
		udelay(1);

	if (!timeout) {
		printf("DMA timeout\n");
		return -ETIMEDOUT;
	}

	writel(readl(plat->regbase + CQSPI_DMA_DST_I_STS_REG),
	       plat->regbase + CQSPI_DMA_DST_I_STS_REG);
	return 0;
}
