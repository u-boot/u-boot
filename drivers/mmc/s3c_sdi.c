/*
 * S3C24xx SD/MMC driver
 *
 * Based on OpenMoko S3C24xx driver by Harald Welte <laforge@openmoko.org>
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <errno.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>
#include <asm/unaligned.h>

#define S3C2440_SDICON_SDRESET		(1 << 8)
#define S3C2410_SDICON_FIFORESET	(1 << 1)
#define S3C2410_SDICON_CLOCKTYPE	(1 << 0)

#define S3C2410_SDICMDCON_LONGRSP	(1 << 10)
#define S3C2410_SDICMDCON_WAITRSP	(1 << 9)
#define S3C2410_SDICMDCON_CMDSTART	(1 << 8)
#define S3C2410_SDICMDCON_SENDERHOST	(1 << 6)
#define S3C2410_SDICMDCON_INDEX		0x3f

#define S3C2410_SDICMDSTAT_CRCFAIL	(1 << 12)
#define S3C2410_SDICMDSTAT_CMDSENT	(1 << 11)
#define S3C2410_SDICMDSTAT_CMDTIMEOUT	(1 << 10)
#define S3C2410_SDICMDSTAT_RSPFIN	(1 << 9)

#define S3C2440_SDIDCON_DS_WORD		(2 << 22)
#define S3C2410_SDIDCON_TXAFTERRESP	(1 << 20)
#define S3C2410_SDIDCON_RXAFTERCMD	(1 << 19)
#define S3C2410_SDIDCON_BLOCKMODE	(1 << 17)
#define S3C2410_SDIDCON_WIDEBUS		(1 << 16)
#define S3C2440_SDIDCON_DATSTART	(1 << 14)
#define S3C2410_SDIDCON_XFER_RXSTART	(2 << 12)
#define S3C2410_SDIDCON_XFER_TXSTART	(3 << 12)
#define S3C2410_SDIDCON_BLKNUM		0x7ff

#define S3C2410_SDIDSTA_FIFOFAIL	(1 << 8)
#define S3C2410_SDIDSTA_CRCFAIL		(1 << 7)
#define S3C2410_SDIDSTA_RXCRCFAIL	(1 << 6)
#define S3C2410_SDIDSTA_DATATIMEOUT	(1 << 5)
#define S3C2410_SDIDSTA_XFERFINISH	(1 << 4)

#define S3C2410_SDIFSTA_TFHALF		(1 << 11)
#define S3C2410_SDIFSTA_COUNTMASK	0x7f

/*
 * WARNING: We only support one SD IP block.
 * NOTE: It's not likely there will ever exist an S3C24xx with two,
 *       at least not in this universe all right.
 */
static int wide_bus;

static int
s3cmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct s3c24x0_sdi *sdi_regs = s3c24x0_get_base_sdi();
	uint32_t sdiccon, sdicsta, sdidcon, sdidsta, sdidat, sdifsta;
	uint32_t sdicsta_wait_bit = S3C2410_SDICMDSTAT_CMDSENT;
	unsigned int timeout = 100000;
	int ret = 0, xfer_len, data_offset = 0;
	const uint32_t sdidsta_err_mask = S3C2410_SDIDSTA_FIFOFAIL |
		S3C2410_SDIDSTA_CRCFAIL | S3C2410_SDIDSTA_RXCRCFAIL |
		S3C2410_SDIDSTA_DATATIMEOUT;


	writel(0xffffffff, &sdi_regs->sdicsta);
	writel(0xffffffff, &sdi_regs->sdidsta);
	writel(0xffffffff, &sdi_regs->sdifsta);

	/* Set up data transfer (if applicable). */
	if (data) {
		writel(data->blocksize, &sdi_regs->sdibsize);

		sdidcon = data->blocks & S3C2410_SDIDCON_BLKNUM;
		sdidcon |= S3C2410_SDIDCON_BLOCKMODE;
#if defined(CONFIG_S3C2440)
		sdidcon |= S3C2440_SDIDCON_DS_WORD | S3C2440_SDIDCON_DATSTART;
#endif
		if (wide_bus)
			sdidcon |= S3C2410_SDIDCON_WIDEBUS;

		if (data->flags & MMC_DATA_READ) {
			sdidcon |= S3C2410_SDIDCON_RXAFTERCMD;
			sdidcon |= S3C2410_SDIDCON_XFER_RXSTART;
		} else {
			sdidcon |= S3C2410_SDIDCON_TXAFTERRESP;
			sdidcon |= S3C2410_SDIDCON_XFER_TXSTART;
		}

		writel(sdidcon, &sdi_regs->sdidcon);
	}

	/* Write CMD arg. */
	writel(cmd->cmdarg, &sdi_regs->sdicarg);

	/* Write CMD index. */
	sdiccon = cmd->cmdidx & S3C2410_SDICMDCON_INDEX;
	sdiccon |= S3C2410_SDICMDCON_SENDERHOST;
	sdiccon |= S3C2410_SDICMDCON_CMDSTART;

	/* Command with short response. */
	if (cmd->resp_type & MMC_RSP_PRESENT) {
		sdiccon |= S3C2410_SDICMDCON_WAITRSP;
		sdicsta_wait_bit = S3C2410_SDICMDSTAT_RSPFIN;
	}

	/* Command with long response. */
	if (cmd->resp_type & MMC_RSP_136)
		sdiccon |= S3C2410_SDICMDCON_LONGRSP;

	/* Start the command. */
	writel(sdiccon, &sdi_regs->sdiccon);

	/* Wait for the command to complete or for response. */
	for (timeout = 100000; timeout; timeout--) {
		sdicsta = readl(&sdi_regs->sdicsta);
		if (sdicsta & sdicsta_wait_bit)
			break;

		if (sdicsta & S3C2410_SDICMDSTAT_CMDTIMEOUT)
			timeout = 1;
	}

	/* Clean the status bits. */
	setbits_le32(&sdi_regs->sdicsta, 0xf << 9);

	if (!timeout) {
		puts("S3C SDI: Command timed out!\n");
		ret = -ETIMEDOUT;
		goto error;
	}

	/* Read out the response. */
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&sdi_regs->sdirsp0);
		cmd->response[1] = readl(&sdi_regs->sdirsp1);
		cmd->response[2] = readl(&sdi_regs->sdirsp2);
		cmd->response[3] = readl(&sdi_regs->sdirsp3);
	} else {
		cmd->response[0] = readl(&sdi_regs->sdirsp0);
	}

	/* If there are no data, we're done. */
	if (!data)
		return 0;

	xfer_len = data->blocksize * data->blocks;

	while (xfer_len > 0) {
		sdidsta = readl(&sdi_regs->sdidsta);
		sdifsta = readl(&sdi_regs->sdifsta);

		if (sdidsta & sdidsta_err_mask) {
			printf("S3C SDI: Data error (sdta=0x%08x)\n", sdidsta);
			ret = -EIO;
			goto error;
		}

		if (data->flags & MMC_DATA_READ) {
			if ((sdifsta & S3C2410_SDIFSTA_COUNTMASK) < 4)
				continue;
			sdidat = readl(&sdi_regs->sdidat);
			put_unaligned_le32(sdidat, data->dest + data_offset);
		} else {	/* Write */
			/* TX FIFO half full. */
			if (!(sdifsta & S3C2410_SDIFSTA_TFHALF))
				continue;

			/* TX FIFO is below 32b full, write. */
			sdidat = get_unaligned_le32(data->src + data_offset);
			writel(sdidat, &sdi_regs->sdidat);
		}
		data_offset += 4;
		xfer_len -= 4;
	}

	/* Wait for the command to complete or for response. */
	for (timeout = 100000; timeout; timeout--) {
		sdidsta = readl(&sdi_regs->sdidsta);
		if (sdidsta & S3C2410_SDIDSTA_XFERFINISH)
			break;

		if (sdidsta & S3C2410_SDIDSTA_DATATIMEOUT)
			timeout = 1;
	}

	/* Clear status bits. */
	writel(0x6f8, &sdi_regs->sdidsta);

	if (!timeout) {
		puts("S3C SDI: Command timed out!\n");
		ret = -ETIMEDOUT;
		goto error;
	}

	writel(0, &sdi_regs->sdidcon);

	return 0;
error:
	return ret;
}

static int s3cmmc_set_ios(struct mmc *mmc)
{
	struct s3c24x0_sdi *sdi_regs = s3c24x0_get_base_sdi();
	uint32_t divider = 0;

	wide_bus = (mmc->bus_width == 4);

	if (!mmc->clock)
		return 0;

	divider = DIV_ROUND_UP(get_PCLK(), mmc->clock);
	if (divider)
		divider--;

	writel(divider, &sdi_regs->sdipre);
	mdelay(125);

	return 0;
}

static int s3cmmc_init(struct mmc *mmc)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	struct s3c24x0_sdi *sdi_regs = s3c24x0_get_base_sdi();

	/* Start the clock. */
	setbits_le32(&clk_power->clkcon, 1 << 9);

#if defined(CONFIG_S3C2440)
	writel(S3C2440_SDICON_SDRESET, &sdi_regs->sdicon);
	mdelay(10);
	writel(0x7fffff, &sdi_regs->sdidtimer);
#else
	writel(0xffff, &sdi_regs->sdidtimer);
#endif
	writel(MMC_MAX_BLOCK_LEN, &sdi_regs->sdibsize);
	writel(0x0, &sdi_regs->sdiimsk);

	writel(S3C2410_SDICON_FIFORESET | S3C2410_SDICON_CLOCKTYPE,
	       &sdi_regs->sdicon);

	mdelay(125);

	return 0;
}

struct s3cmmc_priv {
	struct mmc_config	cfg;
	int (*getcd)(struct mmc *);
	int (*getwp)(struct mmc *);
};

static int s3cmmc_getcd(struct mmc *mmc)
{
	struct s3cmmc_priv *priv = mmc->priv;
	if (priv->getcd)
		return priv->getcd(mmc);
	else
		return 0;
}

static int s3cmmc_getwp(struct mmc *mmc)
{
	struct s3cmmc_priv *priv = mmc->priv;
	if (priv->getwp)
		return priv->getwp(mmc);
	else
		return 0;
}

static const struct mmc_ops s3cmmc_ops = {
	.send_cmd	= s3cmmc_send_cmd,
	.set_ios	= s3cmmc_set_ios,
	.init		= s3cmmc_init,
	.getcd		= s3cmmc_getcd,
	.getwp		= s3cmmc_getwp,
};

int s3cmmc_initialize(bd_t *bis, int (*getcd)(struct mmc *),
		      int (*getwp)(struct mmc *))
{
	struct s3cmmc_priv	*priv;
	struct mmc		*mmc;
	struct mmc_config	*cfg;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	cfg = &priv->cfg;

	cfg->name = "S3C MMC";
	cfg->ops = &s3cmmc_ops;
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	cfg->f_min = 400000;
	cfg->f_max = get_PCLK() / 2;
	cfg->b_max = 0x80;

#if defined(CONFIG_S3C2410)
	/*
	 * S3C2410 has some bug that prevents reliable
	 * operation at higher speed
	 */
	cfg->f_max /= 2;
#endif

	mmc = mmc_create(cfg, priv);
	if (!mmc) {
		free(priv);
		return -ENOMEM;
	}

	return 0;
}
