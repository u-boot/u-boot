/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Portions Copyright 2011-2015 NVIDIA Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bouncebuf.h>
#include <common.h>
#include <dm/device.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#ifndef CONFIG_TEGRA186
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#endif
#include <asm/arch-tegra/mmc.h>
#include <asm/arch-tegra/tegra_mmc.h>
#include <mmc.h>

/*
 * FIXME: TODO: This driver contains a number of ifdef CONFIG_TEGRA186 that
 * should not be present. These are needed because newer Tegra SoCs support
 * only the standard clock/reset APIs, whereas older Tegra SoCs support only
 * a custom Tegra-specific API. ASAP the older Tegra SoCs' code should be
 * fixed to implement the standard APIs, and all drivers converted to solely
 * use the new standard APIs, with no ifdefs.
 */

DECLARE_GLOBAL_DATA_PTR;

struct mmc_host mmc_host[CONFIG_SYS_MMC_MAX_DEVICE];

#if !CONFIG_IS_ENABLED(OF_CONTROL)
#error "Please enable device tree support to use this driver"
#endif

static void mmc_set_power(struct mmc_host *host, unsigned short power)
{
	u8 pwr = 0;
	debug("%s: power = %x\n", __func__, power);

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V1_8;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_0;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_3;
			break;
		}
	}
	debug("%s: pwr = %X\n", __func__, pwr);

	/* Set the bus voltage first (if any) */
	writeb(pwr, &host->reg->pwrcon);
	if (pwr == 0)
		return;

	/* Now enable bus power */
	pwr |= TEGRA_MMC_PWRCTL_SD_BUS_POWER;
	writeb(pwr, &host->reg->pwrcon);
}

static void mmc_prepare_data(struct mmc_host *host, struct mmc_data *data,
				struct bounce_buffer *bbstate)
{
	unsigned char ctrl;


	debug("buf: %p (%p), data->blocks: %u, data->blocksize: %u\n",
		bbstate->bounce_buffer, bbstate->user_buffer, data->blocks,
		data->blocksize);

	writel((u32)(unsigned long)bbstate->bounce_buffer, &host->reg->sysad);
	/*
	 * DMASEL[4:3]
	 * 00 = Selects SDMA
	 * 01 = Reserved
	 * 10 = Selects 32-bit Address ADMA2
	 * 11 = Selects 64-bit Address ADMA2
	 */
	ctrl = readb(&host->reg->hostctl);
	ctrl &= ~TEGRA_MMC_HOSTCTL_DMASEL_MASK;
	ctrl |= TEGRA_MMC_HOSTCTL_DMASEL_SDMA;
	writeb(ctrl, &host->reg->hostctl);

	/* We do not handle DMA boundaries, so set it to max (512 KiB) */
	writew((7 << 12) | (data->blocksize & 0xFFF), &host->reg->blksize);
	writew(data->blocks, &host->reg->blkcnt);
}

static void mmc_set_transfer_mode(struct mmc_host *host, struct mmc_data *data)
{
	unsigned short mode;
	debug(" mmc_set_transfer_mode called\n");
	/*
	 * TRNMOD
	 * MUL1SIN0[5]	: Multi/Single Block Select
	 * RD1WT0[4]	: Data Transfer Direction Select
	 *	1 = read
	 *	0 = write
	 * ENACMD12[2]	: Auto CMD12 Enable
	 * ENBLKCNT[1]	: Block Count Enable
	 * ENDMA[0]	: DMA Enable
	 */
	mode = (TEGRA_MMC_TRNMOD_DMA_ENABLE |
		TEGRA_MMC_TRNMOD_BLOCK_COUNT_ENABLE);

	if (data->blocks > 1)
		mode |= TEGRA_MMC_TRNMOD_MULTI_BLOCK_SELECT;

	if (data->flags & MMC_DATA_READ)
		mode |= TEGRA_MMC_TRNMOD_DATA_XFER_DIR_SEL_READ;

	writew(mode, &host->reg->trnmod);
}

static int mmc_wait_inhibit(struct mmc_host *host,
			    struct mmc_cmd *cmd,
			    struct mmc_data *data,
			    unsigned int timeout)
{
	/*
	 * PRNSTS
	 * CMDINHDAT[1] : Command Inhibit (DAT)
	 * CMDINHCMD[0] : Command Inhibit (CMD)
	 */
	unsigned int mask = TEGRA_MMC_PRNSTS_CMD_INHIBIT_CMD;

	/*
	 * We shouldn't wait for data inhibit for stop commands, even
	 * though they might use busy signaling
	 */
	if ((data == NULL) && (cmd->resp_type & MMC_RSP_BUSY))
		mask |= TEGRA_MMC_PRNSTS_CMD_INHIBIT_DAT;

	while (readl(&host->reg->prnsts) & mask) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return -1;
		}
		timeout--;
		udelay(1000);
	}

	return 0;
}

static int mmc_send_cmd_bounced(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data, struct bounce_buffer *bbstate)
{
	struct mmc_host *host = mmc->priv;
	int flags, i;
	int result;
	unsigned int mask = 0;
	unsigned int retry = 0x100000;
	debug(" mmc_send_cmd called\n");

	result = mmc_wait_inhibit(host, cmd, data, 10 /* ms */);

	if (result < 0)
		return result;

	if (data)
		mmc_prepare_data(host, data, bbstate);

	debug("cmd->arg: %08x\n", cmd->cmdarg);
	writel(cmd->cmdarg, &host->reg->argument);

	if (data)
		mmc_set_transfer_mode(host, data);

	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY))
		return -1;

	/*
	 * CMDREG
	 * CMDIDX[13:8]	: Command index
	 * DATAPRNT[5]	: Data Present Select
	 * ENCMDIDX[4]	: Command Index Check Enable
	 * ENCMDCRC[3]	: Command CRC Check Enable
	 * RSPTYP[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_NO_RESPONSE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_136;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48_BUSY;
	else
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= TEGRA_MMC_TRNMOD_CMD_CRC_CHECK;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= TEGRA_MMC_TRNMOD_CMD_INDEX_CHECK;
	if (data)
		flags |= TEGRA_MMC_TRNMOD_DATA_PRESENT_SELECT_DATA_TRANSFER;

	debug("cmd: %d\n", cmd->cmdidx);

	writew((cmd->cmdidx << 8) | flags, &host->reg->cmdreg);

	for (i = 0; i < retry; i++) {
		mask = readl(&host->reg->norintsts);
		/* Command Complete */
		if (mask & TEGRA_MMC_NORINTSTS_CMD_COMPLETE) {
			if (!data)
				writel(mask, &host->reg->norintsts);
			break;
		}
	}

	if (i == retry) {
		printf("%s: waiting for status update\n", __func__);
		writel(mask, &host->reg->norintsts);
		return -ETIMEDOUT;
	}

	if (mask & TEGRA_MMC_NORINTSTS_CMD_TIMEOUT) {
		/* Timeout Error */
		debug("timeout: %08x cmd %d\n", mask, cmd->cmdidx);
		writel(mask, &host->reg->norintsts);
		return -ETIMEDOUT;
	} else if (mask & TEGRA_MMC_NORINTSTS_ERR_INTERRUPT) {
		/* Error Interrupt */
		debug("error: %08x cmd %d\n", mask, cmd->cmdidx);
		writel(mask, &host->reg->norintsts);
		return -1;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				unsigned long offset =
					(unsigned long)(&host->reg->rspreg3 - i);
				cmd->response[i] = readl(offset) << 8;

				if (i != 3) {
					cmd->response[i] |=
						readb(offset - 1);
				}
				debug("cmd->resp[%d]: %08x\n",
						i, cmd->response[i]);
			}
		} else if (cmd->resp_type & MMC_RSP_BUSY) {
			for (i = 0; i < retry; i++) {
				/* PRNTDATA[23:20] : DAT[3:0] Line Signal */
				if (readl(&host->reg->prnsts)
					& (1 << 20))	/* DAT[0] */
					break;
			}

			if (i == retry) {
				printf("%s: card is still busy\n", __func__);
				writel(mask, &host->reg->norintsts);
				return -ETIMEDOUT;
			}

			cmd->response[0] = readl(&host->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		} else {
			cmd->response[0] = readl(&host->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		}
	}

	if (data) {
		unsigned long	start = get_timer(0);

		while (1) {
			mask = readl(&host->reg->norintsts);

			if (mask & TEGRA_MMC_NORINTSTS_ERR_INTERRUPT) {
				/* Error Interrupt */
				writel(mask, &host->reg->norintsts);
				printf("%s: error during transfer: 0x%08x\n",
						__func__, mask);
				return -1;
			} else if (mask & TEGRA_MMC_NORINTSTS_DMA_INTERRUPT) {
				/*
				 * DMA Interrupt, restart the transfer where
				 * it was interrupted.
				 */
				unsigned int address = readl(&host->reg->sysad);

				debug("DMA end\n");
				writel(TEGRA_MMC_NORINTSTS_DMA_INTERRUPT,
				       &host->reg->norintsts);
				writel(address, &host->reg->sysad);
			} else if (mask & TEGRA_MMC_NORINTSTS_XFER_COMPLETE) {
				/* Transfer Complete */
				debug("r/w is done\n");
				break;
			} else if (get_timer(start) > 8000UL) {
				writel(mask, &host->reg->norintsts);
				printf("%s: MMC Timeout\n"
				       "    Interrupt status        0x%08x\n"
				       "    Interrupt status enable 0x%08x\n"
				       "    Interrupt signal enable 0x%08x\n"
				       "    Present status          0x%08x\n",
				       __func__, mask,
				       readl(&host->reg->norintstsen),
				       readl(&host->reg->norintsigen),
				       readl(&host->reg->prnsts));
				return -1;
			}
		}
		writel(mask, &host->reg->norintsts);
	}

	udelay(1000);
	return 0;
}

static int tegra_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	void *buf;
	unsigned int bbflags;
	size_t len;
	struct bounce_buffer bbstate;
	int ret;

	if (data) {
		if (data->flags & MMC_DATA_READ) {
			buf = data->dest;
			bbflags = GEN_BB_WRITE;
		} else {
			buf = (void *)data->src;
			bbflags = GEN_BB_READ;
		}
		len = data->blocks * data->blocksize;

		bounce_buffer_start(&bbstate, buf, len, bbflags);
	}

	ret = mmc_send_cmd_bounced(mmc, cmd, data, &bbstate);

	if (data)
		bounce_buffer_stop(&bbstate);

	return ret;
}

static void mmc_change_clock(struct mmc_host *host, uint clock)
{
	int div;
	unsigned short clk;
	unsigned long timeout;

	debug(" mmc_change_clock called\n");

	/*
	 * Change Tegra SDMMCx clock divisor here. Source is PLLP_OUT0
	 */
	if (clock == 0)
		goto out;
#ifdef CONFIG_TEGRA186
	{
		ulong rate = clk_set_rate(&host->clk, clock);
		div = (rate + clock - 1) / clock;
	}
#else
	clock_adjust_periph_pll_div(host->mmc_id, CLOCK_ID_PERIPH, clock,
				    &div);
#endif
	debug("div = %d\n", div);

	writew(0, &host->reg->clkcon);

	/*
	 * CLKCON
	 * SELFREQ[15:8]	: base clock divided by value
	 * ENSDCLK[2]		: SD Clock Enable
	 * STBLINTCLK[1]	: Internal Clock Stable
	 * ENINTCLK[0]		: Internal Clock Enable
	 */
	div >>= 1;
	clk = ((div << TEGRA_MMC_CLKCON_SDCLK_FREQ_SEL_SHIFT) |
	       TEGRA_MMC_CLKCON_INTERNAL_CLOCK_ENABLE);
	writew(clk, &host->reg->clkcon);

	/* Wait max 10 ms */
	timeout = 10;
	while (!(readw(&host->reg->clkcon) &
		 TEGRA_MMC_CLKCON_INTERNAL_CLOCK_STABLE)) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	clk |= TEGRA_MMC_CLKCON_SD_CLOCK_ENABLE;
	writew(clk, &host->reg->clkcon);

	debug("mmc_change_clock: clkcon = %08X\n", clk);

out:
	host->clock = clock;
}

static void tegra_mmc_set_ios(struct mmc *mmc)
{
	struct mmc_host *host = mmc->priv;
	unsigned char ctrl;
	debug(" mmc_set_ios called\n");

	debug("bus_width: %x, clock: %d\n", mmc->bus_width, mmc->clock);

	/* Change clock first */
	mmc_change_clock(host, mmc->clock);

	ctrl = readb(&host->reg->hostctl);

	/*
	 * WIDE8[5]
	 * 0 = Depend on WIDE4
	 * 1 = 8-bit mode
	 * WIDE4[1]
	 * 1 = 4-bit mode
	 * 0 = 1-bit mode
	 */
	if (mmc->bus_width == 8)
		ctrl |= (1 << 5);
	else if (mmc->bus_width == 4)
		ctrl |= (1 << 1);
	else
		ctrl &= ~(1 << 1);

	writeb(ctrl, &host->reg->hostctl);
	debug("mmc_set_ios: hostctl = %08X\n", ctrl);
}

static void mmc_reset(struct mmc_host *host, struct mmc *mmc)
{
	unsigned int timeout;
	debug(" mmc_reset called\n");

	/*
	 * RSTALL[0] : Software reset for all
	 * 1 = reset
	 * 0 = work
	 */
	writeb(TEGRA_MMC_SWRST_SW_RESET_FOR_ALL, &host->reg->swrst);

	host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (readb(&host->reg->swrst) & TEGRA_MMC_SWRST_SW_RESET_FOR_ALL) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	/* Set SD bus voltage & enable bus power */
	mmc_set_power(host, fls(mmc->cfg->voltages) - 1);
	debug("%s: power control = %02X, host control = %02X\n", __func__,
		readb(&host->reg->pwrcon), readb(&host->reg->hostctl));

	/* Make sure SDIO pads are set up */
	pad_init_mmc(host);
}

static int tegra_mmc_core_init(struct mmc *mmc)
{
	struct mmc_host *host = mmc->priv;
	unsigned int mask;
	debug(" mmc_core_init called\n");

	mmc_reset(host, mmc);

	host->version = readw(&host->reg->hcver);
	debug("host version = %x\n", host->version);

	/* mask all */
	writel(0xffffffff, &host->reg->norintstsen);
	writel(0xffffffff, &host->reg->norintsigen);

	writeb(0xe, &host->reg->timeoutcon);	/* TMCLK * 2^27 */
	/*
	 * NORMAL Interrupt Status Enable Register init
	 * [5] ENSTABUFRDRDY : Buffer Read Ready Status Enable
	 * [4] ENSTABUFWTRDY : Buffer write Ready Status Enable
	 * [3] ENSTADMAINT   : DMA boundary interrupt
	 * [1] ENSTASTANSCMPLT : Transfre Complete Status Enable
	 * [0] ENSTACMDCMPLT : Command Complete Status Enable
	*/
	mask = readl(&host->reg->norintstsen);
	mask &= ~(0xffff);
	mask |= (TEGRA_MMC_NORINTSTSEN_CMD_COMPLETE |
		 TEGRA_MMC_NORINTSTSEN_XFER_COMPLETE |
		 TEGRA_MMC_NORINTSTSEN_DMA_INTERRUPT |
		 TEGRA_MMC_NORINTSTSEN_BUFFER_WRITE_READY |
		 TEGRA_MMC_NORINTSTSEN_BUFFER_READ_READY);
	writel(mask, &host->reg->norintstsen);

	/*
	 * NORMAL Interrupt Signal Enable Register init
	 * [1] ENSTACMDCMPLT : Transfer Complete Signal Enable
	 */
	mask = readl(&host->reg->norintsigen);
	mask &= ~(0xffff);
	mask |= TEGRA_MMC_NORINTSIGEN_XFER_COMPLETE;
	writel(mask, &host->reg->norintsigen);

	return 0;
}

static int tegra_mmc_getcd(struct mmc *mmc)
{
	struct mmc_host *host = mmc->priv;

	debug("tegra_mmc_getcd called\n");

	if (dm_gpio_is_valid(&host->cd_gpio))
		return dm_gpio_get_value(&host->cd_gpio);

	return 1;
}

static const struct mmc_ops tegra_mmc_ops = {
	.send_cmd	= tegra_mmc_send_cmd,
	.set_ios	= tegra_mmc_set_ios,
	.init		= tegra_mmc_core_init,
	.getcd		= tegra_mmc_getcd,
};

static int do_mmc_init(int dev_index, bool removable)
{
	struct mmc_host *host;
	struct mmc *mmc;
#ifdef CONFIG_TEGRA186
	int ret;
#endif

	/* DT should have been read & host config filled in */
	host = &mmc_host[dev_index];
	if (!host->enabled)
		return -1;

	debug(" do_mmc_init: index %d, bus width %d pwr_gpio %d cd_gpio %d\n",
	      dev_index, host->width, gpio_get_number(&host->pwr_gpio),
	      gpio_get_number(&host->cd_gpio));

	host->clock = 0;

#ifdef CONFIG_TEGRA186
	ret = reset_assert(&host->reset_ctl);
	if (ret)
		return ret;
	ret = clk_enable(&host->clk);
	if (ret)
		return ret;
	ret = clk_set_rate(&host->clk, 20000000);
	if (IS_ERR_VALUE(ret))
		return ret;
	ret = reset_deassert(&host->reset_ctl);
	if (ret)
		return ret;
#else
	clock_start_periph_pll(host->mmc_id, CLOCK_ID_PERIPH, 20000000);
#endif

	if (dm_gpio_is_valid(&host->pwr_gpio))
		dm_gpio_set_value(&host->pwr_gpio, 1);

	memset(&host->cfg, 0, sizeof(host->cfg));

	host->cfg.name = "Tegra SD/MMC";
	host->cfg.ops = &tegra_mmc_ops;

	host->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	host->cfg.host_caps = 0;
	if (host->width == 8)
		host->cfg.host_caps |= MMC_MODE_8BIT;
	if (host->width >= 4)
		host->cfg.host_caps |= MMC_MODE_4BIT;
	host->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	/*
	 * min freq is for card identification, and is the highest
	 *  low-speed SDIO card frequency (actually 400KHz)
	 * max freq is highest HS eMMC clock as per the SD/MMC spec
	 *  (actually 52MHz)
	 */
	host->cfg.f_min = 375000;
	host->cfg.f_max = 48000000;

	host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	mmc = mmc_create(&host->cfg, host);
	mmc->block_dev.removable = removable;
	if (mmc == NULL)
		return -1;

	return 0;
}

/**
 * Get the host address and peripheral ID for a node.
 *
 * @param blob		fdt blob
 * @param node		Device index (0-3)
 * @param host		Structure to fill in (reg, width, mmc_id)
 */
static int mmc_get_config(const void *blob, int node, struct mmc_host *host,
			  bool *removablep)
{
	debug("%s: node = %d\n", __func__, node);

	host->enabled = fdtdec_get_is_enabled(blob, node);

	host->reg = (struct tegra_mmc *)fdtdec_get_addr(blob, node, "reg");
	if ((fdt_addr_t)host->reg == FDT_ADDR_T_NONE) {
		debug("%s: no sdmmc base reg info found\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}

#ifdef CONFIG_TEGRA186
	{
		/*
		 * FIXME: This variable should go away when the MMC device
		 * actually is a udevice.
		 */
		struct udevice dev;
		int ret;
		dev.of_offset = node;
		ret = reset_get_by_name(&dev, "sdmmc", &host->reset_ctl);
		if (ret) {
			debug("reset_get_by_index() failed: %d\n", ret);
			return ret;
		}
		ret = clk_get_by_name(&dev, "sdmmc", &host->clk);
		if (ret) {
			debug("clk_get_by_index() failed: %d\n", ret);
			return ret;
		}
	}
#else
	host->mmc_id = clock_decode_periph_id(blob, node);
	if (host->mmc_id == PERIPH_ID_NONE) {
		debug("%s: could not decode periph id\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}
#endif

	/*
	 * NOTE: mmc->bus_width is determined by mmc.c dynamically.
	 * TBD: Override it with this value?
	 */
	host->width = fdtdec_get_int(blob, node, "bus-width", 0);
	if (!host->width)
		debug("%s: no sdmmc width found\n", __func__);

	/* These GPIOs are optional */
	gpio_request_by_name_nodev(blob, node, "cd-gpios", 0, &host->cd_gpio,
				   GPIOD_IS_IN);
	gpio_request_by_name_nodev(blob, node, "wp-gpios", 0, &host->wp_gpio,
				   GPIOD_IS_IN);
	gpio_request_by_name_nodev(blob, node, "power-gpios", 0,
				   &host->pwr_gpio, GPIOD_IS_OUT);
	*removablep = !fdtdec_get_bool(blob, node, "non-removable");

	debug("%s: found controller at %p, width = %d, periph_id = %d\n",
		__func__, host->reg, host->width,
#ifndef CONFIG_TEGRA186
		host->mmc_id
#else
		-1
#endif
	);
	return 0;
}

/*
 * Process a list of nodes, adding them to our list of SDMMC ports.
 *
 * @param blob          fdt blob
 * @param node_list     list of nodes to process (any <=0 are ignored)
 * @param count         number of nodes to process
 * @return 0 if ok, -1 on error
 */
static int process_nodes(const void *blob, int node_list[], int count)
{
	struct mmc_host *host;
	bool removable;
	int i, node;

	debug("%s: count = %d\n", __func__, count);

	/* build mmc_host[] for each controller */
	for (i = 0; i < count; i++) {
		node = node_list[i];
		if (node <= 0)
			continue;

		host = &mmc_host[i];
		host->id = i;

		if (mmc_get_config(blob, node, host, &removable)) {
			printf("%s: failed to decode dev %d\n",	__func__, i);
			return -1;
		}
		do_mmc_init(i, removable);
	}
	return 0;
}

void tegra_mmc_init(void)
{
	int node_list[CONFIG_SYS_MMC_MAX_DEVICE], count;
	const void *blob = gd->fdt_blob;
	debug("%s entry\n", __func__);

	/* See if any Tegra186 MMC controllers are present */
	count = fdtdec_find_aliases_for_id(blob, "sdhci",
		COMPAT_NVIDIA_TEGRA186_SDMMC, node_list,
		CONFIG_SYS_MMC_MAX_DEVICE);
	debug("%s: count of Tegra186 sdhci nodes is %d\n", __func__, count);
	if (process_nodes(blob, node_list, count)) {
		printf("%s: Error processing T186 mmc node(s)!\n", __func__);
		return;
	}

	/* See if any Tegra210 MMC controllers are present */
	count = fdtdec_find_aliases_for_id(blob, "sdhci",
		COMPAT_NVIDIA_TEGRA210_SDMMC, node_list,
		CONFIG_SYS_MMC_MAX_DEVICE);
	debug("%s: count of Tegra210 sdhci nodes is %d\n", __func__, count);
	if (process_nodes(blob, node_list, count)) {
		printf("%s: Error processing T210 mmc node(s)!\n", __func__);
		return;
	}

	/* See if any Tegra124 MMC controllers are present */
	count = fdtdec_find_aliases_for_id(blob, "sdhci",
		COMPAT_NVIDIA_TEGRA124_SDMMC, node_list,
		CONFIG_SYS_MMC_MAX_DEVICE);
	debug("%s: count of Tegra124 sdhci nodes is %d\n", __func__, count);
	if (process_nodes(blob, node_list, count)) {
		printf("%s: Error processing T124 mmc node(s)!\n", __func__);
		return;
	}

	/* See if any Tegra30 MMC controllers are present */
	count = fdtdec_find_aliases_for_id(blob, "sdhci",
		COMPAT_NVIDIA_TEGRA30_SDMMC, node_list,
		CONFIG_SYS_MMC_MAX_DEVICE);
	debug("%s: count of T30 sdhci nodes is %d\n", __func__, count);
	if (process_nodes(blob, node_list, count)) {
		printf("%s: Error processing T30 mmc node(s)!\n", __func__);
		return;
	}

	/* Now look for any Tegra20 MMC controllers */
	count = fdtdec_find_aliases_for_id(blob, "sdhci",
		COMPAT_NVIDIA_TEGRA20_SDMMC, node_list,
		CONFIG_SYS_MMC_MAX_DEVICE);
	debug("%s: count of T20 sdhci nodes is %d\n", __func__, count);
	if (process_nodes(blob, node_list, count)) {
		printf("%s: Error processing T20 mmc node(s)!\n", __func__);
		return;
	}
}
