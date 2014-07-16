/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Rajeshawari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bouncebuf.h>
#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <dwmmc.h>
#include <asm-generic/errno.h>

#define PAGE_SIZE 4096

static int dwmci_wait_reset(struct dwmci_host *host, u32 value)
{
	unsigned long timeout = 1000;
	u32 ctrl;

	dwmci_writel(host, DWMCI_CTRL, value);

	while (timeout--) {
		ctrl = dwmci_readl(host, DWMCI_CTRL);
		if (!(ctrl & DWMCI_RESET_ALL))
			return 1;
	}
	return 0;
}

static void dwmci_set_idma_desc(struct dwmci_idmac *idmac,
		u32 desc0, u32 desc1, u32 desc2)
{
	struct dwmci_idmac *desc = idmac;

	desc->flags = desc0;
	desc->cnt = desc1;
	desc->addr = desc2;
	desc->next_addr = (unsigned int)desc + sizeof(struct dwmci_idmac);
}

static void dwmci_prepare_data(struct dwmci_host *host,
			       struct mmc_data *data,
			       struct dwmci_idmac *cur_idmac,
			       void *bounce_buffer)
{
	unsigned long ctrl;
	unsigned int i = 0, flags, cnt, blk_cnt;
	ulong data_start, data_end;


	blk_cnt = data->blocks;

	dwmci_wait_reset(host, DWMCI_CTRL_FIFO_RESET);

	data_start = (ulong)cur_idmac;
	dwmci_writel(host, DWMCI_DBADDR, (unsigned int)cur_idmac);

	do {
		flags = DWMCI_IDMAC_OWN | DWMCI_IDMAC_CH ;
		flags |= (i == 0) ? DWMCI_IDMAC_FS : 0;
		if (blk_cnt <= 8) {
			flags |= DWMCI_IDMAC_LD;
			cnt = data->blocksize * blk_cnt;
		} else
			cnt = data->blocksize * 8;

		dwmci_set_idma_desc(cur_idmac, flags, cnt,
				    (u32)bounce_buffer + (i * PAGE_SIZE));

		if (blk_cnt <= 8)
			break;
		blk_cnt -= 8;
		cur_idmac++;
		i++;
	} while(1);

	data_end = (ulong)cur_idmac;
	flush_dcache_range(data_start, data_end + ARCH_DMA_MINALIGN);

	ctrl = dwmci_readl(host, DWMCI_CTRL);
	ctrl |= DWMCI_IDMAC_EN | DWMCI_DMA_EN;
	dwmci_writel(host, DWMCI_CTRL, ctrl);

	ctrl = dwmci_readl(host, DWMCI_BMOD);
	ctrl |= DWMCI_BMOD_IDMAC_FB | DWMCI_BMOD_IDMAC_EN;
	dwmci_writel(host, DWMCI_BMOD, ctrl);

	dwmci_writel(host, DWMCI_BLKSIZ, data->blocksize);
	dwmci_writel(host, DWMCI_BYTCNT, data->blocksize * data->blocks);
}

static int dwmci_set_transfer_mode(struct dwmci_host *host,
		struct mmc_data *data)
{
	unsigned long mode;

	mode = DWMCI_CMD_DATA_EXP;
	if (data->flags & MMC_DATA_WRITE)
		mode |= DWMCI_CMD_RW;

	return mode;
}

static int dwmci_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
		struct mmc_data *data)
{
	struct dwmci_host *host = mmc->priv;
	ALLOC_CACHE_ALIGN_BUFFER(struct dwmci_idmac, cur_idmac,
				 data ? DIV_ROUND_UP(data->blocks, 8) : 0);
	int flags = 0, i;
	unsigned int timeout = 100000;
	u32 retry = 10000;
	u32 mask, ctrl;
	ulong start = get_timer(0);
	struct bounce_buffer bbstate;

	while (dwmci_readl(host, DWMCI_STATUS) & DWMCI_BUSY) {
		if (get_timer(start) > timeout) {
			printf("Timeout on data busy\n");
			return TIMEOUT;
		}
	}

	dwmci_writel(host, DWMCI_RINTSTS, DWMCI_INTMSK_ALL);

	if (data) {
		if (data->flags == MMC_DATA_READ) {
			bounce_buffer_start(&bbstate, (void*)data->dest,
					    data->blocksize *
					    data->blocks, GEN_BB_WRITE);
		} else {
			bounce_buffer_start(&bbstate, (void*)data->src,
					    data->blocksize *
					    data->blocks, GEN_BB_READ);
		}
		dwmci_prepare_data(host, data, cur_idmac,
				   bbstate.bounce_buffer);
	}

	dwmci_writel(host, DWMCI_CMDARG, cmd->cmdarg);

	if (data)
		flags = dwmci_set_transfer_mode(host, data);

	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY))
		return -1;

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		flags |= DWMCI_CMD_ABORT_STOP;
	else
		flags |= DWMCI_CMD_PRV_DAT_WAIT;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		flags |= DWMCI_CMD_RESP_EXP;
		if (cmd->resp_type & MMC_RSP_136)
			flags |= DWMCI_CMD_RESP_LENGTH;
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= DWMCI_CMD_CHECK_CRC;

	flags |= (cmd->cmdidx | DWMCI_CMD_START | DWMCI_CMD_USE_HOLD_REG);

	debug("Sending CMD%d\n",cmd->cmdidx);

	dwmci_writel(host, DWMCI_CMD, flags);

	for (i = 0; i < retry; i++) {
		mask = dwmci_readl(host, DWMCI_RINTSTS);
		if (mask & DWMCI_INTMSK_CDONE) {
			if (!data)
				dwmci_writel(host, DWMCI_RINTSTS, mask);
			break;
		}
	}

	if (i == retry)
		return TIMEOUT;

	if (mask & DWMCI_INTMSK_RTO) {
		debug("Response Timeout..\n");
		return TIMEOUT;
	} else if (mask & DWMCI_INTMSK_RE) {
		debug("Response Error..\n");
		return -1;
	}


	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[0] = dwmci_readl(host, DWMCI_RESP3);
			cmd->response[1] = dwmci_readl(host, DWMCI_RESP2);
			cmd->response[2] = dwmci_readl(host, DWMCI_RESP1);
			cmd->response[3] = dwmci_readl(host, DWMCI_RESP0);
		} else {
			cmd->response[0] = dwmci_readl(host, DWMCI_RESP0);
		}
	}

	if (data) {
		do {
			mask = dwmci_readl(host, DWMCI_RINTSTS);
			if (mask & (DWMCI_DATA_ERR | DWMCI_DATA_TOUT)) {
				debug("DATA ERROR!\n");
				return -1;
			}
		} while (!(mask & DWMCI_INTMSK_DTO));

		dwmci_writel(host, DWMCI_RINTSTS, mask);

		ctrl = dwmci_readl(host, DWMCI_CTRL);
		ctrl &= ~(DWMCI_DMA_EN);
		dwmci_writel(host, DWMCI_CTRL, ctrl);

		bounce_buffer_stop(&bbstate);
	}

	udelay(100);

	return 0;
}

static int dwmci_setup_bus(struct dwmci_host *host, u32 freq)
{
	u32 div, status;
	int timeout = 10000;
	unsigned long sclk;

	if ((freq == host->clock) || (freq == 0))
		return 0;
	/*
	 * If host->get_mmc_clk didn't define,
	 * then assume that host->bus_hz is source clock value.
	 * host->bus_hz should be set from user.
	 */
	if (host->get_mmc_clk)
		sclk = host->get_mmc_clk(host);
	else if (host->bus_hz)
		sclk = host->bus_hz;
	else {
		printf("Didn't get source clock value..\n");
		return -EINVAL;
	}

	div = DIV_ROUND_UP(sclk, 2 * freq);

	dwmci_writel(host, DWMCI_CLKENA, 0);
	dwmci_writel(host, DWMCI_CLKSRC, 0);

	dwmci_writel(host, DWMCI_CLKDIV, div);
	dwmci_writel(host, DWMCI_CMD, DWMCI_CMD_PRV_DAT_WAIT |
			DWMCI_CMD_UPD_CLK | DWMCI_CMD_START);

	do {
		status = dwmci_readl(host, DWMCI_CMD);
		if (timeout-- < 0) {
			printf("TIMEOUT error!!\n");
			return -ETIMEDOUT;
		}
	} while (status & DWMCI_CMD_START);

	dwmci_writel(host, DWMCI_CLKENA, DWMCI_CLKEN_ENABLE |
			DWMCI_CLKEN_LOW_PWR);

	dwmci_writel(host, DWMCI_CMD, DWMCI_CMD_PRV_DAT_WAIT |
			DWMCI_CMD_UPD_CLK | DWMCI_CMD_START);

	timeout = 10000;
	do {
		status = dwmci_readl(host, DWMCI_CMD);
		if (timeout-- < 0) {
			printf("TIMEOUT error!!\n");
			return -ETIMEDOUT;
		}
	} while (status & DWMCI_CMD_START);

	host->clock = freq;

	return 0;
}

static void dwmci_set_ios(struct mmc *mmc)
{
	struct dwmci_host *host = (struct dwmci_host *)mmc->priv;
	u32 ctype, regs;

	debug("Buswidth = %d, clock: %d\n",mmc->bus_width, mmc->clock);

	dwmci_setup_bus(host, mmc->clock);
	switch (mmc->bus_width) {
	case 8:
		ctype = DWMCI_CTYPE_8BIT;
		break;
	case 4:
		ctype = DWMCI_CTYPE_4BIT;
		break;
	default:
		ctype = DWMCI_CTYPE_1BIT;
		break;
	}

	dwmci_writel(host, DWMCI_CTYPE, ctype);

	regs = dwmci_readl(host, DWMCI_UHS_REG);
	if (mmc->card_caps & MMC_MODE_DDR_52MHz)
		regs |= DWMCI_DDR_MODE;
	else
		regs &= DWMCI_DDR_MODE;

	dwmci_writel(host, DWMCI_UHS_REG, regs);

	if (host->clksel)
		host->clksel(host);
}

static int dwmci_init(struct mmc *mmc)
{
	struct dwmci_host *host = mmc->priv;

	if (host->board_init)
		host->board_init(host);

	dwmci_writel(host, DWMCI_PWREN, 1);

	if (!dwmci_wait_reset(host, DWMCI_RESET_ALL)) {
		debug("%s[%d] Fail-reset!!\n",__func__,__LINE__);
		return -1;
	}

	/* Enumerate at 400KHz */
	dwmci_setup_bus(host, mmc->cfg->f_min);

	dwmci_writel(host, DWMCI_RINTSTS, 0xFFFFFFFF);
	dwmci_writel(host, DWMCI_INTMASK, 0);

	dwmci_writel(host, DWMCI_TMOUT, 0xFFFFFFFF);

	dwmci_writel(host, DWMCI_IDINTEN, 0);
	dwmci_writel(host, DWMCI_BMOD, 1);

	if (host->fifoth_val) {
		dwmci_writel(host, DWMCI_FIFOTH, host->fifoth_val);
	}

	dwmci_writel(host, DWMCI_CLKENA, 0);
	dwmci_writel(host, DWMCI_CLKSRC, 0);

	return 0;
}

static const struct mmc_ops dwmci_ops = {
	.send_cmd	= dwmci_send_cmd,
	.set_ios	= dwmci_set_ios,
	.init		= dwmci_init,
};

int add_dwmci(struct dwmci_host *host, u32 max_clk, u32 min_clk)
{
	host->cfg.name = host->name;
	host->cfg.ops = &dwmci_ops;
	host->cfg.f_min = min_clk;
	host->cfg.f_max = max_clk;

	host->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	host->cfg.host_caps = host->caps;

	if (host->buswidth == 8) {
		host->cfg.host_caps |= MMC_MODE_8BIT;
		host->cfg.host_caps &= ~MMC_MODE_4BIT;
	} else {
		host->cfg.host_caps |= MMC_MODE_4BIT;
		host->cfg.host_caps &= ~MMC_MODE_8BIT;
	}
	host->cfg.host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_HC;

	host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	host->mmc = mmc_create(&host->cfg, host);
	if (host->mmc == NULL)
		return -1;

	return 0;
}
