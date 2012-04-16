/*
 * Freescale i.MX28 SSP MMC driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 * Terry Lv
 *
 * Copyright 2007, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
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
#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/dma.h>

/*
 * CONFIG_MXS_MMC_DMA: This feature is highly experimental and has no
 *                     performance benefit unless you operate the platform with
 *                     data cache enabled. This is disabled by default, enable
 *                     only if you know what you're doing.
 */

struct mxsmmc_priv {
	int			id;
	struct mx28_ssp_regs	*regs;
	uint32_t		clkseq_bypass;
	uint32_t		*clkctrl_ssp;
	uint32_t		buswidth;
	int			(*mmc_is_wp)(int);
	struct mxs_dma_desc	*desc;
};

#define	MXSMMC_MAX_TIMEOUT	10000

/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int
mxsmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct mxsmmc_priv *priv = (struct mxsmmc_priv *)mmc->priv;
	struct mx28_ssp_regs *ssp_regs = priv->regs;
	uint32_t reg;
	int timeout;
	uint32_t data_count;
	uint32_t ctrl0;
#ifndef CONFIG_MXS_MMC_DMA
	uint32_t *data_ptr;
#else
	uint32_t cache_data_count;
#endif

	debug("MMC%d: CMD%d\n", mmc->block_dev.dev, cmd->cmdidx);

	/* Check bus busy */
	timeout = MXSMMC_MAX_TIMEOUT;
	while (--timeout) {
		udelay(1000);
		reg = readl(&ssp_regs->hw_ssp_status);
		if (!(reg &
			(SSP_STATUS_BUSY | SSP_STATUS_DATA_BUSY |
			SSP_STATUS_CMD_BUSY))) {
			break;
		}
	}

	if (!timeout) {
		printf("MMC%d: Bus busy timeout!\n", mmc->block_dev.dev);
		return TIMEOUT;
	}

	/* See if card is present */
	if (readl(&ssp_regs->hw_ssp_status) & SSP_STATUS_CARD_DETECT) {
		printf("MMC%d: No card detected!\n", mmc->block_dev.dev);
		return NO_CARD_ERR;
	}

	/* Start building CTRL0 contents */
	ctrl0 = priv->buswidth;

	/* Set up command */
	if (!(cmd->resp_type & MMC_RSP_CRC))
		ctrl0 |= SSP_CTRL0_IGNORE_CRC;
	if (cmd->resp_type & MMC_RSP_PRESENT)	/* Need to get response */
		ctrl0 |= SSP_CTRL0_GET_RESP;
	if (cmd->resp_type & MMC_RSP_136)	/* It's a 136 bits response */
		ctrl0 |= SSP_CTRL0_LONG_RESP;

	/* Command index */
	reg = readl(&ssp_regs->hw_ssp_cmd0);
	reg &= ~(SSP_CMD0_CMD_MASK | SSP_CMD0_APPEND_8CYC);
	reg |= cmd->cmdidx << SSP_CMD0_CMD_OFFSET;
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		reg |= SSP_CMD0_APPEND_8CYC;
	writel(reg, &ssp_regs->hw_ssp_cmd0);

	/* Command argument */
	writel(cmd->cmdarg, &ssp_regs->hw_ssp_cmd1);

	/* Set up data */
	if (data) {
		/* READ or WRITE */
		if (data->flags & MMC_DATA_READ) {
			ctrl0 |= SSP_CTRL0_READ;
		} else if (priv->mmc_is_wp(mmc->block_dev.dev)) {
			printf("MMC%d: Can not write a locked card!\n",
				mmc->block_dev.dev);
			return UNUSABLE_ERR;
		}

		ctrl0 |= SSP_CTRL0_DATA_XFER;
		reg = ((data->blocks - 1) <<
			SSP_BLOCK_SIZE_BLOCK_COUNT_OFFSET) |
			((ffs(data->blocksize) - 1) <<
			SSP_BLOCK_SIZE_BLOCK_SIZE_OFFSET);
		writel(reg, &ssp_regs->hw_ssp_block_size);

		reg = data->blocksize * data->blocks;
		writel(reg, &ssp_regs->hw_ssp_xfer_size);
	}

	/* Kick off the command */
	ctrl0 |= SSP_CTRL0_WAIT_FOR_IRQ | SSP_CTRL0_ENABLE | SSP_CTRL0_RUN;
	writel(ctrl0, &ssp_regs->hw_ssp_ctrl0);

	/* Wait for the command to complete */
	timeout = MXSMMC_MAX_TIMEOUT;
	while (--timeout) {
		udelay(1000);
		reg = readl(&ssp_regs->hw_ssp_status);
		if (!(reg & SSP_STATUS_CMD_BUSY))
			break;
	}

	if (!timeout) {
		printf("MMC%d: Command %d busy\n",
			mmc->block_dev.dev, cmd->cmdidx);
		return TIMEOUT;
	}

	/* Check command timeout */
	if (reg & SSP_STATUS_RESP_TIMEOUT) {
		printf("MMC%d: Command %d timeout (status 0x%08x)\n",
			mmc->block_dev.dev, cmd->cmdidx, reg);
		return TIMEOUT;
	}

	/* Check command errors */
	if (reg & (SSP_STATUS_RESP_CRC_ERR | SSP_STATUS_RESP_ERR)) {
		printf("MMC%d: Command %d error (status 0x%08x)!\n",
			mmc->block_dev.dev, cmd->cmdidx, reg);
		return COMM_ERR;
	}

	/* Copy response to response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[3] = readl(&ssp_regs->hw_ssp_sdresp0);
		cmd->response[2] = readl(&ssp_regs->hw_ssp_sdresp1);
		cmd->response[1] = readl(&ssp_regs->hw_ssp_sdresp2);
		cmd->response[0] = readl(&ssp_regs->hw_ssp_sdresp3);
	} else
		cmd->response[0] = readl(&ssp_regs->hw_ssp_sdresp0);

	/* Return if no data to process */
	if (!data)
		return 0;

	data_count = data->blocksize * data->blocks;
	timeout = MXSMMC_MAX_TIMEOUT;

#ifdef CONFIG_MXS_MMC_DMA
	if (data_count % ARCH_DMA_MINALIGN)
		cache_data_count = roundup(data_count, ARCH_DMA_MINALIGN);
	else
		cache_data_count = data_count;

	if (data->flags & MMC_DATA_READ) {
		priv->desc->cmd.data = MXS_DMA_DESC_COMMAND_DMA_WRITE;
		priv->desc->cmd.address = (dma_addr_t)data->dest;
	} else {
		priv->desc->cmd.data = MXS_DMA_DESC_COMMAND_DMA_READ;
		priv->desc->cmd.address = (dma_addr_t)data->src;

		/* Flush data to DRAM so DMA can pick them up */
		flush_dcache_range((uint32_t)priv->desc->cmd.address,
			(uint32_t)(priv->desc->cmd.address + cache_data_count));
	}

	priv->desc->cmd.data |= MXS_DMA_DESC_IRQ | MXS_DMA_DESC_DEC_SEM |
				(data_count << MXS_DMA_DESC_BYTES_OFFSET);


	mxs_dma_desc_append(MXS_DMA_CHANNEL_AHB_APBH_SSP0, priv->desc);
	if (mxs_dma_go(MXS_DMA_CHANNEL_AHB_APBH_SSP0)) {
		printf("MMC%d: DMA transfer failed\n", mmc->block_dev.dev);
		return COMM_ERR;
	}

	/* The data arrived into DRAM, invalidate cache over them */
	if (data->flags & MMC_DATA_READ) {
		invalidate_dcache_range((uint32_t)priv->desc->cmd.address,
			(uint32_t)(priv->desc->cmd.address + cache_data_count));
	}
#else
	if (data->flags & MMC_DATA_READ) {
		data_ptr = (uint32_t *)data->dest;
		while (data_count && --timeout) {
			reg = readl(&ssp_regs->hw_ssp_status);
			if (!(reg & SSP_STATUS_FIFO_EMPTY)) {
				*data_ptr++ = readl(&ssp_regs->hw_ssp_data);
				data_count -= 4;
				timeout = MXSMMC_MAX_TIMEOUT;
			} else
				udelay(1000);
		}
	} else {
		data_ptr = (uint32_t *)data->src;
		timeout *= 100;
		while (data_count && --timeout) {
			reg = readl(&ssp_regs->hw_ssp_status);
			if (!(reg & SSP_STATUS_FIFO_FULL)) {
				writel(*data_ptr++, &ssp_regs->hw_ssp_data);
				data_count -= 4;
				timeout = MXSMMC_MAX_TIMEOUT;
			} else
				udelay(1000);
		}
	}

	if (!timeout) {
		printf("MMC%d: Data timeout with command %d (status 0x%08x)!\n",
			mmc->block_dev.dev, cmd->cmdidx, reg);
		return COMM_ERR;
	}
#endif

	/* Check data errors */
	reg = readl(&ssp_regs->hw_ssp_status);
	if (reg &
		(SSP_STATUS_TIMEOUT | SSP_STATUS_DATA_CRC_ERR |
		SSP_STATUS_FIFO_OVRFLW | SSP_STATUS_FIFO_UNDRFLW)) {
		printf("MMC%d: Data error with command %d (status 0x%08x)!\n",
			mmc->block_dev.dev, cmd->cmdidx, reg);
		return COMM_ERR;
	}

	return 0;
}

static void mxsmmc_set_ios(struct mmc *mmc)
{
	struct mxsmmc_priv *priv = (struct mxsmmc_priv *)mmc->priv;
	struct mx28_ssp_regs *ssp_regs = priv->regs;

	/* Set the clock speed */
	if (mmc->clock)
		mx28_set_ssp_busclock(priv->id, mmc->clock / 1000);

	switch (mmc->bus_width) {
	case 1:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_ONE_BIT;
		break;
	case 4:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_FOUR_BIT;
		break;
	case 8:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_EIGHT_BIT;
		break;
	}

	/* Set the bus width */
	clrsetbits_le32(&ssp_regs->hw_ssp_ctrl0,
			SSP_CTRL0_BUS_WIDTH_MASK, priv->buswidth);

	debug("MMC%d: Set %d bits bus width\n",
		mmc->block_dev.dev, mmc->bus_width);
}

static int mxsmmc_init(struct mmc *mmc)
{
	struct mxsmmc_priv *priv = (struct mxsmmc_priv *)mmc->priv;
	struct mx28_ssp_regs *ssp_regs = priv->regs;

	/* Reset SSP */
	mx28_reset_block(&ssp_regs->hw_ssp_ctrl0_reg);

	/* 8 bits word length in MMC mode */
	clrsetbits_le32(&ssp_regs->hw_ssp_ctrl1,
		SSP_CTRL1_SSP_MODE_MASK | SSP_CTRL1_WORD_LENGTH_MASK,
		SSP_CTRL1_SSP_MODE_SD_MMC | SSP_CTRL1_WORD_LENGTH_EIGHT_BITS |
		SSP_CTRL1_DMA_ENABLE);

	/* Set initial bit clock 400 KHz */
	mx28_set_ssp_busclock(priv->id, 400);

	/* Send initial 74 clock cycles (185 us @ 400 KHz)*/
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_set);
	udelay(200);
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_clr);

	return 0;
}

int mxsmmc_initialize(bd_t *bis, int id, int (*wp)(int))
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	struct mmc *mmc = NULL;
	struct mxsmmc_priv *priv = NULL;
	int ret;

	mmc = malloc(sizeof(struct mmc));
	if (!mmc)
		return -ENOMEM;

	priv = malloc(sizeof(struct mxsmmc_priv));
	if (!priv) {
		free(mmc);
		return -ENOMEM;
	}

	priv->desc = mxs_dma_desc_alloc();
	if (!priv->desc) {
		free(priv);
		free(mmc);
		return -ENOMEM;
	}

	ret = mxs_dma_init_channel(id);
	if (ret)
		return ret;

	priv->mmc_is_wp = wp;
	priv->id = id;
	switch (id) {
	case 0:
		priv->regs = (struct mx28_ssp_regs *)MXS_SSP0_BASE;
		priv->clkseq_bypass = CLKCTRL_CLKSEQ_BYPASS_SSP0;
		priv->clkctrl_ssp = &clkctrl_regs->hw_clkctrl_ssp0;
		break;
	case 1:
		priv->regs = (struct mx28_ssp_regs *)MXS_SSP1_BASE;
		priv->clkseq_bypass = CLKCTRL_CLKSEQ_BYPASS_SSP1;
		priv->clkctrl_ssp = &clkctrl_regs->hw_clkctrl_ssp1;
		break;
	case 2:
		priv->regs = (struct mx28_ssp_regs *)MXS_SSP2_BASE;
		priv->clkseq_bypass = CLKCTRL_CLKSEQ_BYPASS_SSP2;
		priv->clkctrl_ssp = &clkctrl_regs->hw_clkctrl_ssp2;
		break;
	case 3:
		priv->regs = (struct mx28_ssp_regs *)MXS_SSP3_BASE;
		priv->clkseq_bypass = CLKCTRL_CLKSEQ_BYPASS_SSP3;
		priv->clkctrl_ssp = &clkctrl_regs->hw_clkctrl_ssp3;
		break;
	}

	sprintf(mmc->name, "MXS MMC");
	mmc->send_cmd = mxsmmc_send_cmd;
	mmc->set_ios = mxsmmc_set_ios;
	mmc->init = mxsmmc_init;
	mmc->getcd = NULL;
	mmc->priv = priv;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT |
			 MMC_MODE_HS_52MHz | MMC_MODE_HS;

	/*
	 * SSPCLK = 480 * 18 / 29 / 1 = 297.731 MHz
	 * SSP bit rate = SSPCLK / (CLOCK_DIVIDE * (1 + CLOCK_RATE)),
	 * CLOCK_DIVIDE has to be an even value from 2 to 254, and
	 * CLOCK_RATE could be any integer from 0 to 255.
	 */
	mmc->f_min = 400000;
	mmc->f_max = mxc_get_clock(MXC_SSP0_CLK + id) * 1000 / 2;
	mmc->b_max = 0x40;

	mmc_register(mmc);
	return 0;
}
