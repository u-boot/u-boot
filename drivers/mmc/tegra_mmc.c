/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Portions Copyright 2011-2012 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <bouncebuf.h>
#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/tegra_mmc.h>
#include <mmc.h>

/* support 4 mmc hosts */
struct mmc mmc_dev[4];
struct mmc_host mmc_host[4];


/**
 * Get the host address and peripheral ID for a device. Devices are numbered
 * from 0 to 3.
 *
 * @param host		Structure to fill in (base, reg, mmc_id)
 * @param dev_index	Device index (0-3)
 */
static void tegra_get_setup(struct mmc_host *host, int dev_index)
{
	debug("tegra_get_setup: dev_index = %d\n", dev_index);

	switch (dev_index) {
	case 1:
		host->base = TEGRA_SDMMC3_BASE;
		host->mmc_id = PERIPH_ID_SDMMC3;
		break;
	case 2:
		host->base = TEGRA_SDMMC2_BASE;
		host->mmc_id = PERIPH_ID_SDMMC2;
		break;
	case 3:
		host->base = TEGRA_SDMMC1_BASE;
		host->mmc_id = PERIPH_ID_SDMMC1;
		break;
	case 0:
	default:
		host->base = TEGRA_SDMMC4_BASE;
		host->mmc_id = PERIPH_ID_SDMMC4;
		break;
	}

	host->reg = (struct tegra_mmc *)host->base;
}

static void mmc_prepare_data(struct mmc_host *host, struct mmc_data *data,
				struct bounce_buffer *bbstate)
{
	unsigned char ctrl;


	debug("buf: %p (%p), data->blocks: %u, data->blocksize: %u\n",
		bbstate->bounce_buffer, bbstate->user_buffer, data->blocks,
		data->blocksize);

	writel((u32)bbstate->bounce_buffer, &host->reg->sysad);
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
	struct mmc_host *host = (struct mmc_host *)mmc->priv;
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
		return TIMEOUT;
	}

	if (mask & TEGRA_MMC_NORINTSTS_CMD_TIMEOUT) {
		/* Timeout Error */
		debug("timeout: %08x cmd %d\n", mask, cmd->cmdidx);
		writel(mask, &host->reg->norintsts);
		return TIMEOUT;
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
				unsigned int offset =
					(unsigned int)(&host->reg->rspreg3 - i);
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
				return TIMEOUT;
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
			} else if (get_timer(start) > 2000UL) {
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

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
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
	 * Change Tegra SDMMCx clock divisor here. Source is 216MHz,
	 * PLLP_OUT0
	 */
	if (clock == 0)
		goto out;
	clock_adjust_periph_pll_div(host->mmc_id, CLOCK_ID_PERIPH, clock,
				    &div);
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

static void mmc_set_ios(struct mmc *mmc)
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

static void mmc_reset(struct mmc_host *host)
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
}

static int mmc_core_init(struct mmc *mmc)
{
	struct mmc_host *host = (struct mmc_host *)mmc->priv;
	unsigned int mask;
	debug(" mmc_core_init called\n");

	mmc_reset(host);

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

int tegra_mmc_getcd(struct mmc *mmc)
{
	struct mmc_host *host = (struct mmc_host *)mmc->priv;

	debug("tegra_mmc_getcd called\n");

	if (host->cd_gpio >= 0)
		return !gpio_get_value(host->cd_gpio);

	return 1;
}

int tegra_mmc_init(int dev_index, int bus_width, int pwr_gpio, int cd_gpio)
{
	struct mmc_host *host;
	char gpusage[12]; /* "SD/MMCn PWR" or "SD/MMCn CD" */
	struct mmc *mmc;

	debug(" tegra_mmc_init: index %d, bus width %d "
		"pwr_gpio %d cd_gpio %d\n",
		dev_index, bus_width, pwr_gpio, cd_gpio);

	host = &mmc_host[dev_index];

	host->clock = 0;
	host->pwr_gpio = pwr_gpio;
	host->cd_gpio = cd_gpio;
	tegra_get_setup(host, dev_index);

	clock_start_periph_pll(host->mmc_id, CLOCK_ID_PERIPH, 20000000);

	if (host->pwr_gpio >= 0) {
		sprintf(gpusage, "SD/MMC%d PWR", dev_index);
		gpio_request(host->pwr_gpio, gpusage);
		gpio_direction_output(host->pwr_gpio, 1);
	}

	if (host->cd_gpio >= 0) {
		sprintf(gpusage, "SD/MMC%d CD", dev_index);
		gpio_request(host->cd_gpio, gpusage);
		gpio_direction_input(host->cd_gpio);
	}

	mmc = &mmc_dev[dev_index];

	sprintf(mmc->name, "Tegra SD/MMC");
	mmc->priv = host;
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_core_init;
	mmc->getcd = tegra_mmc_getcd;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	mmc->host_caps = 0;
	if (bus_width == 8)
		mmc->host_caps |= MMC_MODE_8BIT;
	if (bus_width >= 4)
		mmc->host_caps |= MMC_MODE_4BIT;
	mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;

	/*
	 * min freq is for card identification, and is the highest
	 *  low-speed SDIO card frequency (actually 400KHz)
	 * max freq is highest HS eMMC clock as per the SD/MMC spec
	 *  (actually 52MHz)
	 * Both of these are the closest equivalents w/216MHz source
	 *  clock and Tegra SDMMC divisors.
	 */
	mmc->f_min = 375000;
	mmc->f_max = 48000000;

	mmc_register(mmc);

	return 0;
}
