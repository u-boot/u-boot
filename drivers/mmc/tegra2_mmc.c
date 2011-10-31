/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Portions Copyright 2011 NVIDIA Corporation
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

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include "tegra2_mmc.h"

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
static void tegra2_get_setup(struct mmc_host *host, int dev_index)
{
	debug("tegra2_get_base_mmc: dev_index = %d\n", dev_index);

	switch (dev_index) {
	case 1:
		host->base = TEGRA2_SDMMC3_BASE;
		host->mmc_id = PERIPH_ID_SDMMC3;
		break;
	case 2:
		host->base = TEGRA2_SDMMC2_BASE;
		host->mmc_id = PERIPH_ID_SDMMC2;
		break;
	case 3:
		host->base = TEGRA2_SDMMC1_BASE;
		host->mmc_id = PERIPH_ID_SDMMC1;
		break;
	case 0:
	default:
		host->base = TEGRA2_SDMMC4_BASE;
		host->mmc_id = PERIPH_ID_SDMMC4;
		break;
	}

	host->reg = (struct tegra2_mmc *)host->base;
}

static void mmc_prepare_data(struct mmc_host *host, struct mmc_data *data)
{
	unsigned char ctrl;

	debug("data->dest: %08X, data->blocks: %u, data->blocksize: %u\n",
	(u32)data->dest, data->blocks, data->blocksize);

	writel((u32)data->dest, &host->reg->sysad);
	/*
	 * DMASEL[4:3]
	 * 00 = Selects SDMA
	 * 01 = Reserved
	 * 10 = Selects 32-bit Address ADMA2
	 * 11 = Selects 64-bit Address ADMA2
	 */
	ctrl = readb(&host->reg->hostctl);
	ctrl &= ~(3 << 3);			/* SDMA */
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
	mode = (1 << 1) | (1 << 0);
	if (data->blocks > 1)
		mode |= (1 << 5);
	if (data->flags & MMC_DATA_READ)
		mode |= (1 << 4);

	writew(mode, &host->reg->trnmod);
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct mmc_host *host = (struct mmc_host *)mmc->priv;
	int flags, i;
	unsigned int timeout;
	unsigned int mask;
	unsigned int retry = 0x100000;
	debug(" mmc_send_cmd called\n");

	/* Wait max 10 ms */
	timeout = 10;

	/*
	 * PRNSTS
	 * CMDINHDAT[1]	: Command Inhibit (DAT)
	 * CMDINHCMD[0]	: Command Inhibit (CMD)
	 */
	mask = (1 << 0);
	if ((data != NULL) || (cmd->resp_type & MMC_RSP_BUSY))
		mask |= (1 << 1);

	/*
	 * We shouldn't wait for data inhibit for stop commands, even
	 * though they might use busy signaling
	 */
	if (data)
		mask &= ~(1 << 1);

	while (readl(&host->reg->prnsts) & mask) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return -1;
		}
		timeout--;
		udelay(1000);
	}

	if (data)
		mmc_prepare_data(host, data);

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
		flags = 0;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = (1 << 0);
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = (3 << 0);
	else
		flags = (2 << 0);

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= (1 << 3);
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= (1 << 4);
	if (data)
		flags |= (1 << 5);

	debug("cmd: %d\n", cmd->cmdidx);

	writew((cmd->cmdidx << 8) | flags, &host->reg->cmdreg);

	for (i = 0; i < retry; i++) {
		mask = readl(&host->reg->norintsts);
		/* Command Complete */
		if (mask & (1 << 0)) {
			if (!data)
				writel(mask, &host->reg->norintsts);
			break;
		}
	}

	if (i == retry) {
		printf("%s: waiting for status update\n", __func__);
		return TIMEOUT;
	}

	if (mask & (1 << 16)) {
		/* Timeout Error */
		debug("timeout: %08x cmd %d\n", mask, cmd->cmdidx);
		return TIMEOUT;
	} else if (mask & (1 << 15)) {
		/* Error Interrupt */
		debug("error: %08x cmd %d\n", mask, cmd->cmdidx);
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
		while (1) {
			mask = readl(&host->reg->norintsts);

			if (mask & (1 << 15)) {
				/* Error Interrupt */
				writel(mask, &host->reg->norintsts);
				printf("%s: error during transfer: 0x%08x\n",
						__func__, mask);
				return -1;
			} else if (mask & (1 << 3)) {
				/* DMA Interrupt */
				debug("DMA end\n");
				break;
			} else if (mask & (1 << 1)) {
				/* Transfer Complete */
				debug("r/w is done\n");
				break;
			}
		}
		writel(mask, &host->reg->norintsts);
	}

	udelay(1000);
	return 0;
}

static void mmc_change_clock(struct mmc_host *host, uint clock)
{
	int div;
	unsigned short clk;
	unsigned long timeout;

	debug(" mmc_change_clock called\n");

	/*
	 * Change Tegra2 SDMMCx clock divisor here. Source is 216MHz,
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
	clk = (div << 8) | (1 << 0);
	writew(clk, &host->reg->clkcon);

	/* Wait max 10 ms */
	timeout = 10;
	while (!(readw(&host->reg->clkcon) & (1 << 1))) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	clk |= (1 << 2);
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
	writeb((1 << 0), &host->reg->swrst);

	host->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (readb(&host->reg->swrst) & (1 << 0)) {
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
	 * [1] ENSTASTANSCMPLT : Transfre Complete Status Enable
	 * [0] ENSTACMDCMPLT : Command Complete Status Enable
	*/
	mask = readl(&host->reg->norintstsen);
	mask &= ~(0xffff);
	mask |= (1 << 5) | (1 << 4) | (1 << 1) | (1 << 0);
	writel(mask, &host->reg->norintstsen);

	/*
	 * NORMAL Interrupt Signal Enable Register init
	 * [1] ENSTACMDCMPLT : Transfer Complete Signal Enable
	 */
	mask = readl(&host->reg->norintsigen);
	mask &= ~(0xffff);
	mask |= (1 << 1);
	writel(mask, &host->reg->norintsigen);

	return 0;
}

static int tegra2_mmc_initialize(int dev_index, int bus_width)
{
	struct mmc_host *host;
	struct mmc *mmc;

	debug(" mmc_initialize called\n");

	host = &mmc_host[dev_index];

	host->clock = 0;
	tegra2_get_setup(host, dev_index);

	clock_start_periph_pll(host->mmc_id, CLOCK_ID_PERIPH, 20000000);

	mmc = &mmc_dev[dev_index];

	sprintf(mmc->name, "Tegra2 SD/MMC");
	mmc->priv = host;
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_core_init;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	if (bus_width == 8)
		mmc->host_caps = MMC_MODE_8BIT;
	else
		mmc->host_caps = MMC_MODE_4BIT;
	mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;

	/*
	 * min freq is for card identification, and is the highest
	 *  low-speed SDIO card frequency (actually 400KHz)
	 * max freq is highest HS eMMC clock as per the SD/MMC spec
	 *  (actually 52MHz)
	 * Both of these are the closest equivalents w/216MHz source
	 *  clock and Tegra2 SDMMC divisors.
	 */
	mmc->f_min = 375000;
	mmc->f_max = 48000000;

	mmc_register(mmc);

	return 0;
}

int tegra2_mmc_init(int dev_index, int bus_width)
{
	debug(" tegra2_mmc_init: index %d, bus width %d\n",
		dev_index, bus_width);
	return tegra2_mmc_initialize(dev_index, bus_width);
}
