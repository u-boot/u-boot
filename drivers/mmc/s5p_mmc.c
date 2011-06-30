/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
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
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>

/* support 4 mmc hosts */
struct mmc mmc_dev[4];
struct mmc_host mmc_host[4];

static inline struct s5p_mmc *s5p_get_base_mmc(int dev_index)
{
	unsigned long offset = dev_index * sizeof(struct s5p_mmc);
	return (struct s5p_mmc *)(samsung_get_base_mmc() + offset);
}

static void mmc_prepare_data(struct mmc_host *host, struct mmc_data *data)
{
	unsigned char ctrl;

	debug("data->dest: %08x\n", (u32)data->dest);
	writel((u32)data->dest, &host->reg->sysad);
	/*
	 * DMASEL[4:3]
	 * 00 = Selects SDMA
	 * 01 = Reserved
	 * 10 = Selects 32-bit Address ADMA2
	 * 11 = Selects 64-bit Address ADMA2
	 */
	ctrl = readb(&host->reg->hostctl);
	ctrl &= ~(3 << 3);
	writeb(ctrl, &host->reg->hostctl);

	/* We do not handle DMA boundaries, so set it to max (512 KiB) */
	writew((7 << 12) | (data->blocksize & 0xFFF), &host->reg->blksize);
	writew(data->blocks, &host->reg->blkcnt);
}

static void mmc_set_transfer_mode(struct mmc_host *host, struct mmc_data *data)
{
	unsigned short mode;

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
	 * We shouldn't wait for data inihibit for stop commands, even
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
				/*
				 * DMA Interrupt, restart the transfer where
				 * it was interrupted.
				 */
				unsigned int address = readl(&host->reg->sysad);

				debug("DMA end\n");
				writel((1 << 3), &host->reg->norintsts);
				writel(address, &host->reg->sysad);
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
	unsigned long ctrl2;

	/*
	 * SELBASECLK[5:4]
	 * 00/01 = HCLK
	 * 10 = EPLL
	 * 11 = XTI or XEXTCLK
	 */
	ctrl2 = readl(&host->reg->control2);
	ctrl2 &= ~(3 << 4);
	ctrl2 |= (2 << 4);
	writel(ctrl2, &host->reg->control2);

	writew(0, &host->reg->clkcon);

	/* XXX: we assume that clock is between 40MHz and 50MHz */
	if (clock == 0)
		goto out;
	else if (clock <= 400000)
		div = 0x100;
	else if (clock <= 20000000)
		div = 4;
	else if (clock <= 26000000)
		div = 2;
	else
		div = 1;
	debug("div: %d\n", div);

	div >>= 1;
	/*
	 * CLKCON
	 * SELFREQ[15:8]	: base clock divied by value
	 * ENSDCLK[2]		: SD Clock Enable
	 * STBLINTCLK[1]	: Internal Clock Stable
	 * ENINTCLK[0]		: Internal Clock Enable
	 */
	clk = (div << 8) | (1 << 0);
	writew(clk, &host->reg->clkcon);

	set_mmc_clk(host->dev_index, div);

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

out:
	host->clock = clock;
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct mmc_host *host = mmc->priv;
	unsigned char ctrl;
	unsigned long val;

	debug("bus_width: %x, clock: %d\n", mmc->bus_width, mmc->clock);

	/*
	 * SELCLKPADDS[17:16]
	 * 00 = 2mA
	 * 01 = 4mA
	 * 10 = 7mA
	 * 11 = 9mA
	 */
	writel(0x3 << 16, &host->reg->control4);

	val = readl(&host->reg->control2);
	val &= (0x3 << 4);

	val |=	(1 << 31) |	/* write status clear async mode enable */
		(1 << 30) |	/* command conflict mask enable */
		(1 << 14) |	/* Feedback Clock Enable for Rx Clock */
		(1 << 8);	/* SDCLK hold enable */

	writel(val, &host->reg->control2);

	/*
	 * FCSEL1[15] FCSEL0[7]
	 * FCSel[1:0] : Rx Feedback Clock Delay Control
	 *	Inverter delay means10ns delay if SDCLK 50MHz setting
	 *	01 = Delay1 (basic delay)
	 *	11 = Delay2 (basic delay + 2ns)
	 *	00 = Delay3 (inverter delay)
	 *	10 = Delay4 (inverter delay + 2ns)
	 */
	writel(0x8080, &host->reg->control3);

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

	/*
	 * OUTEDGEINV[2]
	 * 1 = Riging edge output
	 * 0 = Falling edge output
	 */
	ctrl &= ~(1 << 2);

	writeb(ctrl, &host->reg->hostctl);
}

static void mmc_reset(struct mmc_host *host)
{
	unsigned int timeout;

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

	mmc_reset(host);

	host->version = readw(&host->reg->hcver);

	/* mask all */
	writel(0xffffffff, &host->reg->norintstsen);
	writel(0xffffffff, &host->reg->norintsigen);

	writeb(0xe, &host->reg->timeoutcon);	/* TMCLK * 2^27 */

	/*
	 * NORMAL Interrupt Status Enable Register init
	 * [5] ENSTABUFRDRDY : Buffer Read Ready Status Enable
	 * [4] ENSTABUFWTRDY : Buffer write Ready Status Enable
	 * [3] ENSTADMAINT : DMA Interrupt Status Enable
	 * [1] ENSTASTANSCMPLT : Transfre Complete Status Enable
	 * [0] ENSTACMDCMPLT : Command Complete Status Enable
	 */
	mask = readl(&host->reg->norintstsen);
	mask &= ~(0xffff);
	mask |= (1 << 5) | (1 << 4) | (1 << 3) | (1 << 1) | (1 << 0);
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

static int s5p_mmc_initialize(int dev_index, int bus_width)
{
	struct mmc *mmc;

	mmc = &mmc_dev[dev_index];

	sprintf(mmc->name, "SAMSUNG SD/MMC");
	mmc->priv = &mmc_host[dev_index];
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_core_init;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	if (bus_width == 8)
		mmc->host_caps = MMC_MODE_8BIT;
	else
		mmc->host_caps = MMC_MODE_4BIT;
	mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;

	mmc->f_min = 400000;
	mmc->f_max = 52000000;

	mmc_host[dev_index].dev_index = dev_index;
	mmc_host[dev_index].clock = 0;
	mmc_host[dev_index].reg = s5p_get_base_mmc(dev_index);
	mmc->b_max = 0;
	mmc_register(mmc);

	return 0;
}

int s5p_mmc_init(int dev_index, int bus_width)
{
	return s5p_mmc_initialize(dev_index, bus_width);
}
