// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Rajeshawari Shinde <rajeshwari.s@samsung.com>
 */

#include <bouncebuf.h>
#include <cpu_func.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <dwmmc.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <linux/delay.h>
#include <power/regulator.h>

#define PAGE_SIZE 4096

/* Internal DMA Controller (IDMAC) descriptor for 32-bit addressing mode */
struct dwmci_idmac32 {
	u32 des0;	/* Control descriptor */
	u32 des1;	/* Buffer size */
	u32 des2;	/* Buffer physical address */
	u32 des3;	/* Next descriptor physical address */
} __aligned(ARCH_DMA_MINALIGN);

/* Internal DMA Controller (IDMAC) descriptor for 64-bit addressing mode */
struct dwmci_idmac64 {
	u32 des0;	/* Control descriptor */
	u32 des1;	/* Reserved */
	u32 des2;	/* Buffer sizes */
	u32 des3;	/* Reserved */
	u32 des4;	/* Lower 32-bits of Buffer Address Pointer 1 */
	u32 des5;	/* Upper 32-bits of Buffer Address Pointer 1 */
	u32 des6;	/* Lower 32-bits of Next Descriptor Address */
	u32 des7;	/* Upper 32-bits of Next Descriptor Address */
} __aligned(ARCH_DMA_MINALIGN);

/* Register offsets for DW MMC blocks with 32-bit IDMAC */
static const struct dwmci_idmac_regs dwmci_idmac_regs32 = {
	.dbaddrl	= DWMCI_DBADDR,
	.idsts		= DWMCI_IDSTS,
	.idinten	= DWMCI_IDINTEN,
	.dscaddrl	= DWMCI_DSCADDR,
	.bufaddrl	= DWMCI_BUFADDR,
};

/* Register offsets for DW MMC blocks with 64-bit IDMAC */
static const struct dwmci_idmac_regs dwmci_idmac_regs64 = {
	.dbaddrl	= DWMCI_DBADDRL,
	.dbaddru	= DWMCI_DBADDRU,
	.idsts		= DWMCI_IDSTS64,
	.idinten	= DWMCI_IDINTEN64,
	.dscaddrl	= DWMCI_DSCADDRL,
	.dscaddru	= DWMCI_DSCADDRU,
	.bufaddrl	= DWMCI_BUFADDRL,
	.bufaddru	= DWMCI_BUFADDRU,
};

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

static void dwmci_set_idma_desc32(struct dwmci_idmac32 *desc, u32 control,
				  u32 buf_size, u32 buf_addr)
{
	phys_addr_t desc_phys = virt_to_phys(desc);
	u32 next_desc_phys = desc_phys + sizeof(struct dwmci_idmac32);

	desc->des0 = control;
	desc->des1 = buf_size;
	desc->des2 = buf_addr;
	desc->des3 = next_desc_phys;
}

static void dwmci_set_idma_desc64(struct dwmci_idmac64 *desc, u32 control,
				  u32 buf_size, u64 buf_addr)
{
	phys_addr_t desc_phys = virt_to_phys(desc);
	u64 next_desc_phys = desc_phys + sizeof(struct dwmci_idmac64);

	desc->des0 = control;
	desc->des1 = 0;
	desc->des2 = buf_size;
	desc->des3 = 0;
	desc->des4 = buf_addr & 0xffffffff;
	desc->des5 = buf_addr >> 32;
	desc->des6 = next_desc_phys & 0xffffffff;
	desc->des7 = next_desc_phys >> 32;
}

static void dwmci_prepare_desc(struct dwmci_host *host, struct mmc_data *data,
			       void *cur_idmac, void *bounce_buffer)
{
	struct dwmci_idmac32 *desc32 = cur_idmac;
	struct dwmci_idmac64 *desc64 = cur_idmac;
	ulong data_start, data_end;
	unsigned int blk_cnt, i;

	data_start = (ulong)cur_idmac;
	blk_cnt = data->blocks;

	for (i = 0;; i++) {
		phys_addr_t buf_phys = virt_to_phys(bounce_buffer);
		unsigned int flags, cnt;

		flags = DWMCI_IDMAC_OWN | DWMCI_IDMAC_CH;
		if (i == 0)
			flags |= DWMCI_IDMAC_FS;
		if (blk_cnt <= 8) {
			flags |= DWMCI_IDMAC_LD;
			cnt = data->blocksize * blk_cnt;
		} else {
			cnt = data->blocksize * 8;
		}

		if (host->dma_64bit_address) {
			dwmci_set_idma_desc64(desc64, flags, cnt,
					      buf_phys + i * PAGE_SIZE);
			desc64++;
		} else {
			dwmci_set_idma_desc32(desc32, flags, cnt,
					      buf_phys + i * PAGE_SIZE);
			desc32++;
		}

		if (blk_cnt <= 8)
			break;
		blk_cnt -= 8;
	}

	if (host->dma_64bit_address)
		data_end = (ulong)desc64;
	else
		data_end = (ulong)desc32;
	flush_dcache_range(data_start, roundup(data_end, ARCH_DMA_MINALIGN));
}

static void dwmci_prepare_data(struct dwmci_host *host, struct mmc_data *data,
			       void *cur_idmac, void *bounce_buffer)
{
	const u32 idmacl = virt_to_phys(cur_idmac) & 0xffffffff;
	const u32 idmacu = (u64)virt_to_phys(cur_idmac) >> 32;
	unsigned long ctrl;

	dwmci_wait_reset(host, DWMCI_CTRL_FIFO_RESET);

	/* Clear IDMAC interrupt */
	dwmci_writel(host, host->regs->idsts, 0xffffffff);

	dwmci_writel(host, host->regs->dbaddrl, idmacl);
	if (host->dma_64bit_address)
		dwmci_writel(host, host->regs->dbaddru, idmacu);

	dwmci_prepare_desc(host, data, cur_idmac, bounce_buffer);

	ctrl = dwmci_readl(host, DWMCI_CTRL);
	ctrl |= DWMCI_IDMAC_EN | DWMCI_DMA_EN;
	dwmci_writel(host, DWMCI_CTRL, ctrl);

	ctrl = dwmci_readl(host, DWMCI_BMOD);
	ctrl |= DWMCI_BMOD_IDMAC_FB | DWMCI_BMOD_IDMAC_EN;
	dwmci_writel(host, DWMCI_BMOD, ctrl);

	dwmci_writel(host, DWMCI_BLKSIZ, data->blocksize);
	dwmci_writel(host, DWMCI_BYTCNT, data->blocksize * data->blocks);
}

static int dwmci_fifo_ready(struct dwmci_host *host, u32 bit, u32 *len)
{
	u32 timeout = 20000;

	*len = dwmci_readl(host, DWMCI_STATUS);
	while (--timeout && (*len & bit)) {
		udelay(200);
		*len = dwmci_readl(host, DWMCI_STATUS);
	}

	if (!timeout) {
		debug("%s: FIFO underflow timeout\n", __func__);
		return -ETIMEDOUT;
	}

	return 0;
}

static unsigned int dwmci_get_timeout(struct mmc *mmc, const unsigned int size)
{
	unsigned int timeout;

	timeout = size * 8;	/* counting in bits */
	timeout *= 10;		/* wait 10 times as long */
	timeout /= mmc->clock;
	timeout /= mmc->bus_width;
	timeout /= mmc->ddr_mode ? 2 : 1;
	timeout *= 1000;	/* counting in msec */
	timeout = (timeout < 1000) ? 1000 : timeout;

	return timeout;
}

static int dwmci_data_transfer(struct dwmci_host *host, struct mmc_data *data)
{
	struct mmc *mmc = host->mmc;
	int ret = 0;
	u32 timeout, mask, size, i, len = 0;
	u32 *buf = NULL;
	ulong start = get_timer(0);

	size = data->blocksize * data->blocks;
	if (data->flags == MMC_DATA_READ)
		buf = (unsigned int *)data->dest;
	else
		buf = (unsigned int *)data->src;

	timeout = dwmci_get_timeout(mmc, size);

	size /= 4;

	for (;;) {
		mask = dwmci_readl(host, DWMCI_RINTSTS);
		/* Error during data transfer */
		if (mask & (DWMCI_DATA_ERR | DWMCI_DATA_TOUT)) {
			debug("%s: DATA ERROR!\n", __func__);
			ret = -EINVAL;
			break;
		}

		if (host->fifo_mode && size) {
			len = 0;
			if (data->flags == MMC_DATA_READ &&
			    (mask & (DWMCI_INTMSK_RXDR | DWMCI_INTMSK_DTO))) {
				dwmci_writel(host, DWMCI_RINTSTS,
					     mask & (DWMCI_INTMSK_RXDR |
						     DWMCI_INTMSK_DTO));
				while (size) {
					ret = dwmci_fifo_ready(host,
							DWMCI_FIFO_EMPTY,
							&len);
					if (ret < 0)
						break;

					len = (len >> DWMCI_FIFO_SHIFT) &
						    DWMCI_FIFO_MASK;
					len = min(size, len);
					for (i = 0; i < len; i++)
						*buf++ =
						dwmci_readl(host, DWMCI_DATA);
					size = size > len ? (size - len) : 0;
				}
			} else if (data->flags == MMC_DATA_WRITE &&
				   (mask & DWMCI_INTMSK_TXDR)) {
				while (size) {
					ret = dwmci_fifo_ready(host,
							DWMCI_FIFO_FULL,
							&len);
					if (ret < 0)
						break;

					len = host->fifo_depth - ((len >>
						   DWMCI_FIFO_SHIFT) &
						   DWMCI_FIFO_MASK);
					len = min(size, len);
					for (i = 0; i < len; i++)
						dwmci_writel(host, DWMCI_DATA,
							     *buf++);
					size = size > len ? (size - len) : 0;
				}
				dwmci_writel(host, DWMCI_RINTSTS,
					     DWMCI_INTMSK_TXDR);
			}
		}

		/* Data arrived correctly */
		if (mask & DWMCI_INTMSK_DTO) {
			ret = 0;
			break;
		}

		/* Check for timeout */
		if (get_timer(start) > timeout) {
			debug("%s: Timeout waiting for data!\n", __func__);
			ret = -ETIMEDOUT;
			break;
		}
	}

	dwmci_writel(host, DWMCI_RINTSTS, mask);

	return ret;
}

static int dwmci_dma_transfer(struct dwmci_host *host, uint flags,
			      struct bounce_buffer *bbstate)
{
	int ret;
	u32 mask, ctrl;

	if (flags == MMC_DATA_READ)
		mask = DWMCI_IDINTEN_RI;
	else
		mask = DWMCI_IDINTEN_TI;

	ret = wait_for_bit_le32(host->ioaddr + host->regs->idsts, mask, true,
				1000, false);
	if (ret)
		debug("%s: DWMCI_IDINTEN mask 0x%x timeout\n", __func__, mask);

	/* Clear interrupts */
	dwmci_writel(host, host->regs->idsts, DWMCI_IDINTEN_MASK);

	ctrl = dwmci_readl(host, DWMCI_CTRL);
	ctrl &= ~DWMCI_DMA_EN;
	dwmci_writel(host, DWMCI_CTRL, ctrl);

	bounce_buffer_stop(bbstate);
	return ret;
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

static void dwmci_wait_while_busy(struct dwmci_host *host, struct mmc_cmd *cmd)
{
	unsigned int timeout = 500; /* msec */
	ulong start;

	start = get_timer(0);
	while (dwmci_readl(host, DWMCI_STATUS) & DWMCI_BUSY) {
		if (get_timer(start) > timeout) {
			debug("%s: Timeout on data busy, continue anyway\n",
			      __func__);
			break;
		}
	}
}

static int dwmci_send_cmd_common(struct dwmci_host *host, struct mmc_cmd *cmd,
				 struct mmc_data *data, void *cur_idmac)
{
	int ret, flags = 0, i;
	u32 retry = 100000;
	u32 mask;
	struct bounce_buffer bbstate;

	dwmci_wait_while_busy(host, cmd);
	dwmci_writel(host, DWMCI_RINTSTS, DWMCI_INTMSK_ALL);

	if (data) {
		if (host->fifo_mode) {
			dwmci_writel(host, DWMCI_BLKSIZ, data->blocksize);
			dwmci_writel(host, DWMCI_BYTCNT,
				     data->blocksize * data->blocks);
			dwmci_wait_reset(host, DWMCI_CTRL_FIFO_RESET);
		} else {
			if (data->flags == MMC_DATA_READ) {
				ret = bounce_buffer_start(&bbstate,
						(void *)data->dest,
						data->blocksize *
						data->blocks, GEN_BB_WRITE);
			} else {
				ret = bounce_buffer_start(&bbstate,
						(void *)data->src,
						data->blocksize *
						data->blocks, GEN_BB_READ);
			}

			if (ret)
				return ret;

			dwmci_prepare_data(host, data, cur_idmac,
					   bbstate.bounce_buffer);
		}
	}

	dwmci_writel(host, DWMCI_CMDARG, cmd->cmdarg);

	if (data)
		flags = dwmci_set_transfer_mode(host, data);

	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY))
		return -EBUSY;

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

	flags |= cmd->cmdidx | DWMCI_CMD_START | DWMCI_CMD_USE_HOLD_REG;

	debug("Sending CMD%d\n", cmd->cmdidx);

	dwmci_writel(host, DWMCI_CMD, flags);

	for (i = 0; i < retry; i++) {
		mask = dwmci_readl(host, DWMCI_RINTSTS);
		if (mask & DWMCI_INTMSK_CDONE) {
			if (!data)
				dwmci_writel(host, DWMCI_RINTSTS, mask);
			break;
		}
	}

	if (i == retry) {
		debug("%s: Timeout\n", __func__);
		return -ETIMEDOUT;
	}

	if (mask & DWMCI_INTMSK_RTO) {
		/*
		 * Timeout here is not necessarily fatal. (e)MMC cards
		 * will splat here when they receive CMD55 as they do
		 * not support this command and that is exactly the way
		 * to tell them apart from SD cards. Thus, this output
		 * below shall be debug(). eMMC cards also do not favor
		 * CMD8, please keep that in mind.
		 */
		debug("%s: Response Timeout\n", __func__);
		return -ETIMEDOUT;
	} else if (mask & DWMCI_INTMSK_RE) {
		debug("%s: Response Error\n", __func__);
		return -EIO;
	} else if ((cmd->resp_type & MMC_RSP_CRC) &&
		   (mask & DWMCI_INTMSK_RCRC)) {
		debug("%s: Response CRC Error\n", __func__);
		return -EIO;
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
		ret = dwmci_data_transfer(host, data);
		if (!host->fifo_mode)
			ret = dwmci_dma_transfer(host, data->flags, &bbstate);
	}

	udelay(100);

	return ret;
}

#ifdef CONFIG_DM_MMC
static int dwmci_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
#else
static int dwmci_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
#endif
	struct dwmci_host *host = mmc->priv;
	const size_t buf_size = data ? DIV_ROUND_UP(data->blocks, 8) : 0;

	if (host->dma_64bit_address) {
		ALLOC_CACHE_ALIGN_BUFFER(struct dwmci_idmac64, idmac, buf_size);
		return dwmci_send_cmd_common(host, cmd, data, idmac);
	} else {
		ALLOC_CACHE_ALIGN_BUFFER(struct dwmci_idmac32, idmac, buf_size);
		return dwmci_send_cmd_common(host, cmd, data, idmac);
	}
}

static int dwmci_control_clken(struct dwmci_host *host, bool on)
{
	const u32 val = on ? DWMCI_CLKEN_ENABLE | DWMCI_CLKEN_LOW_PWR : 0;
	const u32 cmd_only_clk = DWMCI_CMD_PRV_DAT_WAIT | DWMCI_CMD_UPD_CLK;
	int timeout = 10000;
	u32 status;

	dwmci_writel(host, DWMCI_CLKENA, val);

	/* Inform CIU */
	dwmci_writel(host, DWMCI_CMD, DWMCI_CMD_START | cmd_only_clk);
	do {
		status = dwmci_readl(host, DWMCI_CMD);
		if (timeout-- < 0) {
			debug("%s: Timeout!\n", __func__);
			return -ETIMEDOUT;
		}
	} while (status & DWMCI_CMD_START);

	return 0;
}

/*
 * Update the clock divider.
 *
 * To prevent a clock glitch keep the clock stopped during the update of
 * clock divider and clock source.
 */
static int dwmci_update_div(struct dwmci_host *host, u32 div)
{
	int ret;

	/* Disable clock */
	ret = dwmci_control_clken(host, false);
	if (ret)
		return ret;

	/* Set clock to desired speed */
	dwmci_writel(host, DWMCI_CLKDIV, div);
	dwmci_writel(host, DWMCI_CLKSRC, 0);

	/* Enable clock */
	return dwmci_control_clken(host, true);
}

static int dwmci_setup_bus(struct dwmci_host *host, u32 freq)
{
	u32 div;
	unsigned long sclk;
	int ret;

	if (freq == host->clock || freq == 0)
		return 0;

	/*
	 * If host->get_mmc_clk isn't defined, then assume that host->bus_hz is
	 * source clock value. host->bus_hz should be set by user.
	 */
	if (host->get_mmc_clk) {
		sclk = host->get_mmc_clk(host, freq);
	} else if (host->bus_hz) {
		sclk = host->bus_hz;
	} else {
		debug("%s: Didn't get source clock value\n", __func__);
		return -EINVAL;
	}

	if (sclk == freq)
		div = 0; /* bypass mode */
	else
		div = DIV_ROUND_UP(sclk, 2 * freq);

	ret = dwmci_update_div(host, div);
	if (ret)
		return ret;

	host->clock = freq;

	return 0;
}

#ifdef CONFIG_DM_MMC
static int dwmci_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
#else
static int dwmci_set_ios(struct mmc *mmc)
{
#endif
	struct dwmci_host *host = (struct dwmci_host *)mmc->priv;
	u32 ctype, regs;

	debug("Bus width = %d, clock: %d\n", mmc->bus_width, mmc->clock);

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
	if (mmc->ddr_mode)
		regs |= DWMCI_DDR_MODE;
	else
		regs &= ~DWMCI_DDR_MODE;

	dwmci_writel(host, DWMCI_UHS_REG, regs);

	if (host->clksel) {
		int ret;

		ret = host->clksel(host);
		if (ret)
			return ret;
	}

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (mmc->vqmmc_supply) {
		int ret;

		ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, false);
		if (ret)
			return ret;

		if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
			regulator_set_value(mmc->vqmmc_supply, 1800000);
		else
			regulator_set_value(mmc->vqmmc_supply, 3300000);

		ret = regulator_set_enable_if_allowed(mmc->vqmmc_supply, true);
		if (ret)
			return ret;
	}
#endif

	return 0;
}

static void dwmci_init_fifo(struct dwmci_host *host)
{
	u32 fifo_thr, fifoth_val;

	if (!host->fifo_depth) {
		u32 fifo_size;

		/*
		 * Automatically detect FIFO depth from FIFOTH register.
		 * Power-on value of RX_WMark is FIFO_DEPTH-1.
		 */
		fifo_size = dwmci_readl(host, DWMCI_FIFOTH);
		fifo_size = ((fifo_size & RX_WMARK_MASK) >> RX_WMARK_SHIFT) + 1;
		host->fifo_depth = fifo_size;
	}

	fifo_thr = host->fifo_depth / 2;
	fifoth_val = MSIZE(0x2) | RX_WMARK(fifo_thr - 1) | TX_WMARK(fifo_thr);
	dwmci_writel(host, DWMCI_FIFOTH, fifoth_val);
}

static void dwmci_init_dma(struct dwmci_host *host)
{
	int addr_config;

	if (host->fifo_mode)
		return;

	addr_config = (dwmci_readl(host, DWMCI_HCON) >> 27) & 0x1;
	if (addr_config == 1) {
		host->dma_64bit_address = true;
		host->regs = &dwmci_idmac_regs64;
		debug("%s: IDMAC supports 64-bit address mode\n", __func__);
	} else {
		host->dma_64bit_address = false;
		host->regs = &dwmci_idmac_regs32;
		debug("%s: IDMAC supports 32-bit address mode\n", __func__);
	}

	dwmci_writel(host, host->regs->idinten, DWMCI_IDINTEN_MASK);
}

static int dwmci_init(struct mmc *mmc)
{
	struct dwmci_host *host = mmc->priv;

	if (host->board_init)
		host->board_init(host);

	dwmci_writel(host, DWMCI_PWREN, 1);

	if (!dwmci_wait_reset(host, DWMCI_RESET_ALL)) {
		debug("%s[%d] Fail-reset!!\n", __func__, __LINE__);
		return -EIO;
	}

	/* Enumerate at 400KHz */
	dwmci_setup_bus(host, mmc->cfg->f_min);

	dwmci_writel(host, DWMCI_RINTSTS, 0xffffffff);
	dwmci_writel(host, DWMCI_INTMASK, 0);

	dwmci_writel(host, DWMCI_TMOUT, 0xffffffff);

	dwmci_writel(host, DWMCI_BMOD, 1);
	dwmci_init_fifo(host);
	dwmci_init_dma(host);

	dwmci_writel(host, DWMCI_CLKENA, 0);
	dwmci_writel(host, DWMCI_CLKSRC, 0);

	return 0;
}

#if CONFIG_IS_ENABLED(DM_MMC)
int dwmci_probe(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return dwmci_init(mmc);
}

const struct dm_mmc_ops dm_dwmci_ops = {
	.send_cmd	= dwmci_send_cmd,
	.set_ios	= dwmci_set_ios,
};

#else
static const struct mmc_ops dwmci_ops = {
	.send_cmd	= dwmci_send_cmd,
	.set_ios	= dwmci_set_ios,
	.init		= dwmci_init,
};
#endif

void dwmci_setup_cfg(struct mmc_config *cfg, struct dwmci_host *host,
		     u32 max_clk, u32 min_clk)
{
	cfg->name = host->name;
#if !CONFIG_IS_ENABLED(DM_MMC)
	cfg->ops = &dwmci_ops;
#endif
	cfg->f_min = min_clk;
	cfg->f_max = max_clk;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	cfg->host_caps = host->caps;

	if (host->buswidth == 8) {
		cfg->host_caps |= MMC_MODE_8BIT;
		cfg->host_caps &= ~MMC_MODE_4BIT;
	} else {
		cfg->host_caps |= MMC_MODE_4BIT;
		cfg->host_caps &= ~MMC_MODE_8BIT;
	}
	cfg->host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz;

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
}

#ifdef CONFIG_BLK
int dwmci_bind(struct udevice *dev, struct mmc *mmc, struct mmc_config *cfg)
{
	return mmc_bind(dev, mmc, cfg);
}
#else
int add_dwmci(struct dwmci_host *host, u32 max_clk, u32 min_clk)
{
	dwmci_setup_cfg(&host->cfg, host, max_clk, min_clk);

	host->mmc = mmc_create(&host->cfg, host);
	if (!host->mmc)
		return -1;

	return 0;
}
#endif
