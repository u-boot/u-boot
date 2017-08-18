/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <mmc.h>
#include <sdhci.h>

#if defined(CONFIG_FIXED_SDHCI_ALIGNED_BUFFER)
void *aligned_buffer = (void *)CONFIG_FIXED_SDHCI_ALIGNED_BUFFER;
#else
void *aligned_buffer;
#endif

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;

	/* Wait max 100 ms */
	timeout = 100;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			printf("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		udelay(1000);
	}
}

static void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
{
	int i;
	if (cmd->resp_type & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0; i < 4; i++) {
			cmd->response[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
			if (i != 3)
				cmd->response[i] |= sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
		}
	} else {
		cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
	}
}

static void sdhci_transfer_pio(struct sdhci_host *host, struct mmc_data *data)
{
	int i;
	char *offs;
	for (i = 0; i < data->blocksize; i += 4) {
		offs = data->dest + i;
		if (data->flags == MMC_DATA_READ)
			*(u32 *)offs = sdhci_readl(host, SDHCI_BUFFER);
		else
			sdhci_writel(host, *(u32 *)offs, SDHCI_BUFFER);
	}
}

static int sdhci_transfer_data(struct sdhci_host *host, struct mmc_data *data,
				unsigned int start_addr)
{
	unsigned int stat, rdy, mask, timeout, block = 0;
	bool transfer_done = false;
#ifdef CONFIG_MMC_SDHCI_SDMA
	unsigned char ctrl;
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl &= ~SDHCI_CTRL_DMA_MASK;
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
#endif

	timeout = 1000000;
	rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
	mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			printf("%s: Error detected in status(0x%X)!\n",
			       __func__, stat);
			return -EIO;
		}
		if (!transfer_done && (stat & rdy)) {
			if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
				continue;
			sdhci_writel(host, rdy, SDHCI_INT_STATUS);
			sdhci_transfer_pio(host, data);
			data->dest += data->blocksize;
			if (++block >= data->blocks) {
				/* Keep looping until the SDHCI_INT_DATA_END is
				 * cleared, even if we finished sending all the
				 * blocks.
				 */
				transfer_done = true;
				continue;
			}
		}
#ifdef CONFIG_MMC_SDHCI_SDMA
		if (!transfer_done && (stat & SDHCI_INT_DMA_END)) {
			sdhci_writel(host, SDHCI_INT_DMA_END, SDHCI_INT_STATUS);
			start_addr &= ~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1);
			start_addr += SDHCI_DEFAULT_BOUNDARY_SIZE;
			sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
		}
#endif
		if (timeout-- > 0)
			udelay(10);
		else {
			printf("%s: Transfer data timeout\n", __func__);
			return -ETIMEDOUT;
		}
	} while (!(stat & SDHCI_INT_DATA_END));
	return 0;
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value.
 */
#define SDHCI_CMD_MAX_TIMEOUT			3200
#define SDHCI_CMD_DEFAULT_TIMEOUT		100
#define SDHCI_READ_STATUS_TIMEOUT		1000

#ifdef CONFIG_DM_MMC
static int sdhci_send_command(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

#else
static int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
#endif
	struct sdhci_host *host = mmc->priv;
	unsigned int stat = 0;
	int ret = 0;
	int trans_bytes = 0, is_aligned = 1;
	u32 mask, flags, mode;
	unsigned int time = 0, start_addr = 0;
	int mmc_dev = mmc_get_blk_desc(mmc)->devnum;
	unsigned start = get_timer(0);

	/* Timeout unit - ms */
	static unsigned int cmd_timeout = SDHCI_CMD_DEFAULT_TIMEOUT;

	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			printf("%s: MMC: %d busy ", __func__, mmc_dev);
			if (2 * cmd_timeout <= SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				printf("timeout increasing to: %u ms.\n",
				       cmd_timeout);
			} else {
				puts("timeout.\n");
				return -ECOMM;
			}
		}
		time++;
		udelay(1000);
	}

	mask = SDHCI_INT_RESPONSE;
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
		if (data)
			mask |= SDHCI_INT_DATA_END;
	} else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (data)
		flags |= SDHCI_CMD_DATA;

	/* Set Transfer mode regarding to data flag */
	if (data != 0) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
		mode = SDHCI_TRNS_BLK_CNT_EN;
		trans_bytes = data->blocks * data->blocksize;
		if (data->blocks > 1)
			mode |= SDHCI_TRNS_MULTI;

		if (data->flags == MMC_DATA_READ)
			mode |= SDHCI_TRNS_READ;

#ifdef CONFIG_MMC_SDHCI_SDMA
		if (data->flags == MMC_DATA_READ)
			start_addr = (unsigned long)data->dest;
		else
			start_addr = (unsigned long)data->src;
		if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) &&
				(start_addr & 0x7) != 0x0) {
			is_aligned = 0;
			start_addr = (unsigned long)aligned_buffer;
			if (data->flags != MMC_DATA_READ)
				memcpy(aligned_buffer, data->src, trans_bytes);
		}

#if defined(CONFIG_FIXED_SDHCI_ALIGNED_BUFFER)
		/*
		 * Always use this bounce-buffer when
		 * CONFIG_FIXED_SDHCI_ALIGNED_BUFFER is defined
		 */
		is_aligned = 0;
		start_addr = (unsigned long)aligned_buffer;
		if (data->flags != MMC_DATA_READ)
			memcpy(aligned_buffer, data->src, trans_bytes);
#endif

		sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
		mode |= SDHCI_TRNS_DMA;
#endif
		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data->blocksize),
				SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	} else if (cmd->resp_type & MMC_RSP_BUSY) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
	}

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
#ifdef CONFIG_MMC_SDHCI_SDMA
	if (data != 0) {
		trans_bytes = ALIGN(trans_bytes, CONFIG_SYS_CACHELINE_SIZE);
		flush_cache(start_addr, trans_bytes);
	}
#endif
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
	start = get_timer(0);
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;

		if (get_timer(start) >= SDHCI_READ_STATUS_TIMEOUT) {
			if (host->quirks & SDHCI_QUIRK_BROKEN_R1B) {
				return 0;
			} else {
				printf("%s: Timeout for status update!\n",
				       __func__);
				return -ETIMEDOUT;
			}
		}
	} while ((stat & mask) != mask);

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	if (!ret && data)
		ret = sdhci_transfer_data(host, data, start_addr);

	if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
		udelay(1000);

	stat = sdhci_readl(host, SDHCI_INT_STATUS);
	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (!ret) {
		if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) &&
				!is_aligned && (data->flags == MMC_DATA_READ))
			memcpy(data->dest, aligned_buffer, trans_bytes);
		return 0;
	}

	sdhci_reset(host, SDHCI_RESET_CMD);
	sdhci_reset(host, SDHCI_RESET_DATA);
	if (stat & SDHCI_INT_TIMEOUT)
		return -ETIMEDOUT;
	else
		return -ECOMM;
}

static int sdhci_set_clock(struct mmc *mmc, unsigned int clock)
{
	struct sdhci_host *host = mmc->priv;
	unsigned int div, clk = 0, timeout;

	/* Wait max 20 ms */
	timeout = 200;
	while (sdhci_readl(host, SDHCI_PRESENT_STATE) &
			   (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)) {
		if (timeout == 0) {
			printf("%s: Timeout to wait cmd & data inhibit\n",
			       __func__);
			return -EBUSY;
		}

		timeout--;
		udelay(100);
	}

	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		return 0;

	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk / div) <= clock)
					break;
			}

			/*
			 * Set Programmable Clock Mode in the Clock
			 * Control register.
			 */
			clk = SDHCI_PROG_CLOCK_MODE;
			div--;
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock) {
				div = 1;
			} else {
				for (div = 2;
				     div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			div >>= 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		div >>= 1;
	}

	if (host->ops && host->ops->set_clock)
		host->ops->set_clock(host, div);

	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			printf("%s: Internal clock never stabilised.\n",
			       __func__);
			return -EBUSY;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
	return 0;
}

static void sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
	u8 pwr = 0;

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = SDHCI_POWER_180;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = SDHCI_POWER_300;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = SDHCI_POWER_330;
			break;
		}
	}

	if (pwr == 0) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		return;
	}

	pwr |= SDHCI_POWER_ON;

	sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

#ifdef CONFIG_DM_MMC
static int sdhci_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
#else
static int sdhci_set_ios(struct mmc *mmc)
{
#endif
	u32 ctrl;
	struct sdhci_host *host = mmc->priv;

	if (host->ops && host->ops->set_control_reg)
		host->ops->set_control_reg(host);

	if (mmc->clock != host->clock)
		sdhci_set_clock(mmc, mmc->clock);

	/* Set bus width */
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	if (mmc->bus_width == 8) {
		ctrl &= ~SDHCI_CTRL_4BITBUS;
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl |= SDHCI_CTRL_8BITBUS;
	} else {
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl &= ~SDHCI_CTRL_8BITBUS;
		if (mmc->bus_width == 4)
			ctrl |= SDHCI_CTRL_4BITBUS;
		else
			ctrl &= ~SDHCI_CTRL_4BITBUS;
	}

	if (mmc->clock > 26000000)
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= ~SDHCI_CTRL_HISPD;

	if (host->quirks & SDHCI_QUIRK_NO_HISPD_BIT)
		ctrl &= ~SDHCI_CTRL_HISPD;

	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

	/* If available, call the driver specific "post" set_ios() function */
	if (host->ops && host->ops->set_ios_post)
		host->ops->set_ios_post(host);

	return 0;
}

static int sdhci_init(struct mmc *mmc)
{
	struct sdhci_host *host = mmc->priv;

	sdhci_reset(host, SDHCI_RESET_ALL);

	if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) && !aligned_buffer) {
		aligned_buffer = memalign(8, 512*1024);
		if (!aligned_buffer) {
			printf("%s: Aligned buffer alloc failed!!!\n",
			       __func__);
			return -ENOMEM;
		}
	}

	sdhci_set_power(host, fls(mmc->cfg->voltages) - 1);

	if (host->ops && host->ops->get_cd)
		host->ops->get_cd(host);

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

#ifdef CONFIG_DM_MMC
int sdhci_probe(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return sdhci_init(mmc);
}

const struct dm_mmc_ops sdhci_ops = {
	.send_cmd	= sdhci_send_command,
	.set_ios	= sdhci_set_ios,
};
#else
static const struct mmc_ops sdhci_ops = {
	.send_cmd	= sdhci_send_command,
	.set_ios	= sdhci_set_ios,
	.init		= sdhci_init,
};
#endif

int sdhci_setup_cfg(struct mmc_config *cfg, struct sdhci_host *host,
		u32 f_max, u32 f_min)
{
	u32 caps, caps_1;

	caps = sdhci_readl(host, SDHCI_CAPABILITIES);

#ifdef CONFIG_MMC_SDHCI_SDMA
	if (!(caps & SDHCI_CAN_DO_SDMA)) {
		printf("%s: Your controller doesn't support SDMA!!\n",
		       __func__);
		return -EINVAL;
	}
#endif
	if (host->quirks & SDHCI_QUIRK_REG32_RW)
		host->version =
			sdhci_readl(host, SDHCI_HOST_VERSION - 2) >> 16;
	else
		host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	cfg->name = host->name;
#ifndef CONFIG_DM_MMC
	cfg->ops = &sdhci_ops;
#endif

	/* Check whether the clock multiplier is supported or not */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		caps_1 = sdhci_readl(host, SDHCI_CAPABILITIES_1);
		host->clk_mul = (caps_1 & SDHCI_CLOCK_MUL_MASK) >>
				SDHCI_CLOCK_MUL_SHIFT;
	}

	if (host->max_clk == 0) {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			host->max_clk = (caps & SDHCI_CLOCK_V3_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		else
			host->max_clk = (caps & SDHCI_CLOCK_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		host->max_clk *= 1000000;
		if (host->clk_mul)
			host->max_clk *= host->clk_mul;
	}
	if (host->max_clk == 0) {
		printf("%s: Hardware doesn't specify base clock frequency\n",
		       __func__);
		return -EINVAL;
	}
	if (f_max && (f_max < host->max_clk))
		cfg->f_max = f_max;
	else
		cfg->f_max = host->max_clk;
	if (f_min)
		cfg->f_min = f_min;
	else {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_300;
		else
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_200;
	}
	cfg->voltages = 0;
	if (caps & SDHCI_CAN_VDD_330)
		cfg->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
	if (caps & SDHCI_CAN_VDD_300)
		cfg->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & SDHCI_CAN_VDD_180)
		cfg->voltages |= MMC_VDD_165_195;

	if (host->quirks & SDHCI_QUIRK_BROKEN_VOLTAGE)
		cfg->voltages |= host->voltages;

	cfg->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT;

	/* Since Host Controller Version3.0 */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		if (!(caps & SDHCI_CAN_DO_8BIT))
			cfg->host_caps &= ~MMC_MODE_8BIT;
	}

	if (host->host_caps)
		cfg->host_caps |= host->host_caps;

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	return 0;
}

#ifdef CONFIG_BLK
int sdhci_bind(struct udevice *dev, struct mmc *mmc, struct mmc_config *cfg)
{
	return mmc_bind(dev, mmc, cfg);
}
#else
int add_sdhci(struct sdhci_host *host, u32 f_max, u32 f_min)
{
	int ret;

	ret = sdhci_setup_cfg(&host->cfg, host, f_max, f_min);
	if (ret)
		return ret;

	host->mmc = mmc_create(&host->cfg, host);
	if (host->mmc == NULL) {
		printf("%s: mmc create fail!\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
#endif
