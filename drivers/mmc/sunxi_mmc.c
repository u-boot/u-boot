/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mmc.h>

struct sunxi_mmc_des {
	u32 reserved1_1:1;
	u32 dic:1;		/* disable interrupt on completion */
	u32 last_des:1;		/* 1-this data buffer is the last buffer */
	u32 first_des:1;		/* 1-data buffer is the first buffer,
				   0-data buffer contained in the next
				   descriptor is 1st buffer */
	u32 des_chain:1;	/* 1-the 2nd address in the descriptor is the
				   next descriptor address */
	u32 end_of_ring:1;	/* 1-last descriptor flag when using dual
				   data buffer in descriptor */
	u32 reserved1_2:24;
	u32 card_err_sum:1;	/* transfer error flag */
	u32 own:1;		/* des owner:1-idma owns it, 0-host owns it */
#define SDXC_DES_NUM_SHIFT 16
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32 data_buf1_sz:16;
	u32 data_buf2_sz:16;
	u32 buf_addr_ptr1;
	u32 buf_addr_ptr2;
};

struct sunxi_mmc_host {
	unsigned mmc_no;
	uint32_t *mclkreg;
	unsigned database;
	unsigned fatal_err;
	unsigned mod_clk;
	struct sunxi_mmc *reg;
	struct mmc_config cfg;
};

/* support 4 mmc hosts */
struct sunxi_mmc_host mmc_host[4];

static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_host *mmchost = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	debug("init mmc %d resource\n", sdc_no);

	switch (sdc_no) {
	case 0:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
		mmchost->mclkreg = &ccm->sd0_clk_cfg;
		break;
	case 1:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
		mmchost->mclkreg = &ccm->sd1_clk_cfg;
		break;
	case 2:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
		mmchost->mclkreg = &ccm->sd2_clk_cfg;
		break;
	case 3:
		mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
		mmchost->mclkreg = &ccm->sd3_clk_cfg;
		break;
	default:
		printf("Wrong mmc number %d\n", sdc_no);
		return -1;
	}
	mmchost->database = (unsigned int)mmchost->reg + 0x100;
	mmchost->mmc_no = sdc_no;

	return 0;
}

static int mmc_clk_io_on(int sdc_no)
{
	unsigned int pll_clk;
	unsigned int divider;
	struct sunxi_mmc_host *mmchost = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	debug("init mmc %d clock and io\n", sdc_no);

	/* config ahb clock */
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MMC(sdc_no));

	/* config mod clock */
	pll_clk = clock_get_pll6();
	/* should be close to 100 MHz but no more, so round up */
	divider = ((pll_clk + 99999999) / 100000000) - 1;
	writel(CCM_MMC_CTRL_ENABLE | CCM_MMC_CTRL_PLL6 | divider,
	       mmchost->mclkreg);
	mmchost->mod_clk = pll_clk / (divider + 1);

	return 0;
}

static int mmc_update_clk(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int cmd;
	unsigned timeout_msecs = 2000;

	cmd = SUNXI_MMC_CMD_START |
	      SUNXI_MMC_CMD_UPCLK_ONLY |
	      SUNXI_MMC_CMD_WAIT_PRE_OVER;
	writel(cmd, &mmchost->reg->cmd);
	while (readl(&mmchost->reg->cmd) & SUNXI_MMC_CMD_START) {
		if (!timeout_msecs--)
			return -1;
		udelay(1000);
	}

	/* clock update sets various irq status bits, clear these */
	writel(readl(&mmchost->reg->rint), &mmchost->reg->rint);

	return 0;
}

static int mmc_config_clock(struct mmc *mmc, unsigned div)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned rval = readl(&mmchost->reg->clkcr);

	/* Disable Clock */
	rval &= ~SUNXI_MMC_CLK_ENABLE;
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmc))
		return -1;

	/* Change Divider Factor */
	rval &= ~SUNXI_MMC_CLK_DIVIDER_MASK;
	rval |= div;
	writel(rval, &mmchost->reg->clkcr);
	if (mmc_update_clk(mmc))
		return -1;
	/* Re-enable Clock */
	rval |= SUNXI_MMC_CLK_ENABLE;
	writel(rval, &mmchost->reg->clkcr);

	if (mmc_update_clk(mmc))
		return -1;

	return 0;
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int clkdiv = 0;

	debug("set ios: bus_width: %x, clock: %d, mod_clk: %d\n",
	      mmc->bus_width, mmc->clock, mmchost->mod_clk);

	/* Change clock first */
	clkdiv = (mmchost->mod_clk + (mmc->clock >> 1)) / mmc->clock / 2;
	if (mmc->clock) {
		if (mmc_config_clock(mmc, clkdiv)) {
			mmchost->fatal_err = 1;
			return;
		}
	}

	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(0x2, &mmchost->reg->width);
	else if (mmc->bus_width == 4)
		writel(0x1, &mmchost->reg->width);
	else
		writel(0x0, &mmchost->reg->width);
}

static int mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &mmchost->reg->gctrl);

	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	const int reading = !!(data->flags & MMC_DATA_READ);
	const uint32_t status_bit = reading ? SUNXI_MMC_STATUS_FIFO_EMPTY :
					      SUNXI_MMC_STATUS_FIFO_FULL;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned timeout_msecs = 2000;
	unsigned *buff = (unsigned int *)(reading ? data->dest : data->src);

	for (i = 0; i < (byte_cnt >> 2); i++) {
		while (readl(&mmchost->reg->status) & status_bit) {
			if (!timeout_msecs--)
				return -1;
			udelay(1000);
		}

		if (reading)
			buff[i] = readl(mmchost->database);
		else
			writel(buff[i], mmchost->database);
	}

	return 0;
}

static int mmc_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned char *buff;
	unsigned des_idx = 0;
	unsigned buff_frag_num =
		(byte_cnt + SDXC_DES_BUFFER_MAX_LEN - 1) >> SDXC_DES_NUM_SHIFT;
	unsigned remain;
	unsigned i, rval;
	ALLOC_CACHE_ALIGN_BUFFER(struct sunxi_mmc_des, pdes, buff_frag_num);

	buff = data->flags & MMC_DATA_READ ?
	    (unsigned char *)data->dest : (unsigned char *)data->src;
	remain = byte_cnt & (SDXC_DES_BUFFER_MAX_LEN - 1);

	flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
	for (i = 0; i < buff_frag_num; i++, des_idx++) {
		memset((void *)&pdes[des_idx], 0, sizeof(struct sunxi_mmc_des));
		pdes[des_idx].des_chain = 1;
		pdes[des_idx].own = 1;
		pdes[des_idx].dic = 1;
		if (buff_frag_num > 1 && i != buff_frag_num - 1)
			pdes[des_idx].data_buf1_sz = 0; /* 0 == max_len */
		else
			pdes[des_idx].data_buf1_sz = remain;

		pdes[des_idx].buf_addr_ptr1 =
		    (u32) buff + i * SDXC_DES_BUFFER_MAX_LEN;
		if (i == 0)
			pdes[des_idx].first_des = 1;

		if (i == buff_frag_num - 1) {
			pdes[des_idx].dic = 0;
			pdes[des_idx].last_des = 1;
			pdes[des_idx].end_of_ring = 1;
			pdes[des_idx].buf_addr_ptr2 = 0;
		} else {
			pdes[des_idx].buf_addr_ptr2 = (u32)&pdes[des_idx + 1];
		}
	}
	flush_cache((unsigned long)pdes,
		    sizeof(struct sunxi_mmc_des) * (des_idx + 1));

	rval = readl(&mmchost->reg->gctrl);
	/* Enable DMA */
	writel(rval | SUNXI_MMC_GCTRL_DMA_RESET | SUNXI_MMC_GCTRL_DMA_ENABLE,
	       &mmchost->reg->gctrl);
	/* Reset iDMA */
	writel(SUNXI_MMC_IDMAC_RESET, &mmchost->reg->dmac);
	/* Enable iDMA */
	writel(SUNXI_MMC_IDMAC_FIXBURST | SUNXI_MMC_IDMAC_ENABLE,
	       &mmchost->reg->dmac);
	rval = readl(&mmchost->reg->idie) &
		~(SUNXI_MMC_IDIE_TXIRQ|SUNXI_MMC_IDIE_RXIRQ);
	if (data->flags & MMC_DATA_WRITE)
		rval |= SUNXI_MMC_IDIE_TXIRQ;
	else
		rval |= SUNXI_MMC_IDIE_RXIRQ;
	writel(rval, &mmchost->reg->idie);
	writel((u32) pdes, &mmchost->reg->dlba);
	writel((0x2 << 28) | (0x7 << 16) | (0x01 << 3),
	       &mmchost->reg->ftrglevel);

	return 0;
}

static void mmc_enable_dma_accesses(struct mmc *mmc, int dma)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;

	unsigned int gctrl = readl(&mmchost->reg->gctrl);
	if (dma)
		gctrl &= ~SUNXI_MMC_GCTRL_ACCESS_BY_AHB;
	else
		gctrl |= SUNXI_MMC_GCTRL_ACCESS_BY_AHB;
	writel(gctrl, &mmchost->reg->gctrl);
}

static int mmc_rint_wait(struct mmc *mmc, unsigned int timeout_msecs,
			 unsigned int done_bit, const char *what)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int status;

	do {
		status = readl(&mmchost->reg->rint);
		if (!timeout_msecs-- ||
		    (status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT)) {
			debug("%s timeout %x\n", what,
			      status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT);
			return TIMEOUT;
		}
		udelay(1000);
	} while (!(status & done_bit));

	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sunxi_mmc_host *mmchost = mmc->priv;
	unsigned int cmdval = SUNXI_MMC_CMD_START;
	unsigned int timeout_msecs;
	int error = 0;
	unsigned int status = 0;
	unsigned int usedma = 0;
	unsigned int bytecnt = 0;

	if (mmchost->fatal_err)
		return -1;
	if (cmd->resp_type & MMC_RSP_BUSY)
		debug("mmc cmd %d check rsp busy\n", cmd->cmdidx);
	if (cmd->cmdidx == 12)
		return 0;

	if (!cmd->cmdidx)
		cmdval |= SUNXI_MMC_CMD_SEND_INIT_SEQ;
	if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdval |= SUNXI_MMC_CMD_RESP_EXPIRE;
	if (cmd->resp_type & MMC_RSP_136)
		cmdval |= SUNXI_MMC_CMD_LONG_RESPONSE;
	if (cmd->resp_type & MMC_RSP_CRC)
		cmdval |= SUNXI_MMC_CMD_CHK_RESPONSE_CRC;

	if (data) {
		if ((u32) data->dest & 0x3) {
			error = -1;
			goto out;
		}

		cmdval |= SUNXI_MMC_CMD_DATA_EXPIRE|SUNXI_MMC_CMD_WAIT_PRE_OVER;
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= SUNXI_MMC_CMD_WRITE;
		if (data->blocks > 1)
			cmdval |= SUNXI_MMC_CMD_AUTO_STOP;
		writel(data->blocksize, &mmchost->reg->blksz);
		writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
	}

	debug("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", mmchost->mmc_no,
	      cmd->cmdidx, cmdval | cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &mmchost->reg->arg);

	if (!data)
		writel(cmdval | cmd->cmdidx, &mmchost->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		debug("trans data %d bytes\n", bytecnt);
#if defined(CONFIG_MMC_SUNXI_USE_DMA) && !defined(CONFIG_SPL_BUILD)
		if (bytecnt > 64) {
#else
		if (0) {
#endif
			usedma = 1;
			mmc_enable_dma_accesses(mmc, 1);
			ret = mmc_trans_data_by_dma(mmc, data);
			writel(cmdval | cmd->cmdidx, &mmchost->reg->cmd);
		} else {
			mmc_enable_dma_accesses(mmc, 0);
			writel(cmdval | cmd->cmdidx, &mmchost->reg->cmd);
			ret = mmc_trans_data_by_cpu(mmc, data);
		}
		if (ret) {
			error = readl(&mmchost->reg->rint) & \
				SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT;
			error = TIMEOUT;
			goto out;
		}
	}

	error = mmc_rint_wait(mmc, 0xfffff, SUNXI_MMC_RINT_COMMAND_DONE, "cmd");
	if (error)
		goto out;

	if (data) {
		timeout_msecs = usedma ? 120 * bytecnt : 120;
		debug("cacl timeout %x msec\n", timeout_msecs);
		error = mmc_rint_wait(mmc, timeout_msecs,
				      data->blocks > 1 ?
				      SUNXI_MMC_RINT_AUTO_COMMAND_DONE :
				      SUNXI_MMC_RINT_DATA_OVER,
				      "data");
		if (error)
			goto out;
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		timeout_msecs = 2000;
		do {
			status = readl(&mmchost->reg->status);
			if (!timeout_msecs--) {
				debug("busy timeout\n");
				error = TIMEOUT;
				goto out;
			}
			udelay(1000);
		} while (status & SUNXI_MMC_STATUS_CARD_DATA_BUSY);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mmchost->reg->resp3);
		cmd->response[1] = readl(&mmchost->reg->resp2);
		cmd->response[2] = readl(&mmchost->reg->resp1);
		cmd->response[3] = readl(&mmchost->reg->resp0);
		debug("mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
		      cmd->response[3], cmd->response[2],
		      cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&mmchost->reg->resp0);
		debug("mmc resp 0x%08x\n", cmd->response[0]);
	}
out:
	if (data && usedma) {
		/* IDMASTAREG
		 * IDST[0] : idma tx int
		 * IDST[1] : idma rx int
		 * IDST[2] : idma fatal bus error
		 * IDST[4] : idma descriptor invalid
		 * IDST[5] : idma error summary
		 * IDST[8] : idma normal interrupt sumary
		 * IDST[9] : idma abnormal interrupt sumary
		 */
		status = readl(&mmchost->reg->idst);
		writel(status, &mmchost->reg->idst);
		writel(0, &mmchost->reg->idie);
		writel(0, &mmchost->reg->dmac);
		writel(readl(&mmchost->reg->gctrl) & ~SUNXI_MMC_GCTRL_DMA_ENABLE,
		       &mmchost->reg->gctrl);
	}
	if (error < 0) {
		writel(SUNXI_MMC_GCTRL_RESET, &mmchost->reg->gctrl);
		mmc_update_clk(mmc);
	}
	writel(0xffffffff, &mmchost->reg->rint);
	writel(readl(&mmchost->reg->gctrl) | SUNXI_MMC_GCTRL_FIFO_RESET,
	       &mmchost->reg->gctrl);

	return error;
}

static const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd	= mmc_send_cmd,
	.set_ios	= mmc_set_ios,
	.init		= mmc_core_init,
};

int sunxi_mmc_init(int sdc_no)
{
	struct mmc_config *cfg = &mmc_host[sdc_no].cfg;

	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));

	cfg->name = "SUNXI SD/MMC";
	cfg->ops  = &sunxi_mmc_ops;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT;
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	mmc_resource_init(sdc_no);
	mmc_clk_io_on(sdc_no);

	if (mmc_create(cfg, &mmc_host[sdc_no]) == NULL)
		return -1;

	return 0;
}
