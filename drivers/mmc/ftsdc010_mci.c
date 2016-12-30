/*
 * Faraday MMC/SD Host Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <asm/byteorder.h>
#include <faraday/ftsdc010.h>

#define CFG_CMD_TIMEOUT (CONFIG_SYS_HZ >> 4) /* 250 ms */
#define CFG_RST_TIMEOUT CONFIG_SYS_HZ /* 1 sec reset timeout */

struct ftsdc010_chip {
	void __iomem *regs;
	uint32_t wprot;   /* write protected (locked) */
	uint32_t rate;    /* actual SD clock in Hz */
	uint32_t sclk;    /* FTSDC010 source clock in Hz */
	uint32_t fifo;    /* fifo depth in bytes */
	uint32_t acmd;
	struct mmc_config cfg;	/* mmc configuration */
};

static inline int ftsdc010_send_cmd(struct mmc *mmc, struct mmc_cmd *mmc_cmd)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	int ret = -ETIMEDOUT;
	uint32_t ts, st;
	uint32_t cmd   = FTSDC010_CMD_IDX(mmc_cmd->cmdidx);
	uint32_t arg   = mmc_cmd->cmdarg;
	uint32_t flags = mmc_cmd->resp_type;

	cmd |= FTSDC010_CMD_CMD_EN;

	if (chip->acmd) {
		cmd |= FTSDC010_CMD_APP_CMD;
		chip->acmd = 0;
	}

	if (flags & MMC_RSP_PRESENT)
		cmd |= FTSDC010_CMD_NEED_RSP;

	if (flags & MMC_RSP_136)
		cmd |= FTSDC010_CMD_LONG_RSP;

	writel(FTSDC010_STATUS_RSP_MASK | FTSDC010_STATUS_CMD_SEND,
		&regs->clr);
	writel(arg, &regs->argu);
	writel(cmd, &regs->cmd);

	if (!(flags & (MMC_RSP_PRESENT | MMC_RSP_136))) {
		for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
			if (readl(&regs->status) & FTSDC010_STATUS_CMD_SEND) {
				writel(FTSDC010_STATUS_CMD_SEND, &regs->clr);
				ret = 0;
				break;
			}
		}
	} else {
		st = 0;
		for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
			st = readl(&regs->status);
			writel(st & FTSDC010_STATUS_RSP_MASK, &regs->clr);
			if (st & FTSDC010_STATUS_RSP_MASK)
				break;
		}
		if (st & FTSDC010_STATUS_RSP_CRC_OK) {
			if (flags & MMC_RSP_136) {
				mmc_cmd->response[0] = readl(&regs->rsp3);
				mmc_cmd->response[1] = readl(&regs->rsp2);
				mmc_cmd->response[2] = readl(&regs->rsp1);
				mmc_cmd->response[3] = readl(&regs->rsp0);
			} else {
				mmc_cmd->response[0] = readl(&regs->rsp0);
			}
			ret = 0;
		} else {
			debug("ftsdc010: rsp err (cmd=%d, st=0x%x)\n",
				mmc_cmd->cmdidx, st);
		}
	}

	if (ret) {
		debug("ftsdc010: cmd timeout (op code=%d)\n",
			mmc_cmd->cmdidx);
	} else if (mmc_cmd->cmdidx == MMC_CMD_APP_CMD) {
		chip->acmd = 1;
	}

	return ret;
}

static void ftsdc010_clkset(struct mmc *mmc, uint32_t rate)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	uint32_t div;

	for (div = 0; div < 0x7f; ++div) {
		if (rate >= chip->sclk / (2 * (div + 1)))
			break;
	}
	chip->rate = chip->sclk / (2 * (div + 1));

	writel(FTSDC010_CCR_CLK_DIV(div), &regs->ccr);

	if (IS_SD(mmc)) {
		setbits_le32(&regs->ccr, FTSDC010_CCR_CLK_SD);

		if (chip->rate > 25000000)
			setbits_le32(&regs->ccr, FTSDC010_CCR_CLK_HISPD);
		else
			clrbits_le32(&regs->ccr, FTSDC010_CCR_CLK_HISPD);
	}
}

static int ftsdc010_wait(struct ftsdc010_mmc __iomem *regs, uint32_t mask)
{
	int ret = -ETIMEDOUT;
	uint32_t st, ts;

	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		st = readl(&regs->status);
		if (!(st & mask))
			continue;
		writel(st & mask, &regs->clr);
		ret = 0;
		break;
	}

	if (ret)
		debug("ftsdc010: wait st(0x%x) timeout\n", mask);

	return ret;
}

/*
 * u-boot mmc api
 */

static int ftsdc010_request(struct mmc *mmc, struct mmc_cmd *cmd,
	struct mmc_data *data)
{
	int ret = -EOPNOTSUPP;
	uint32_t len = 0;
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;

	if (data && (data->flags & MMC_DATA_WRITE) && chip->wprot) {
		printf("ftsdc010: the card is write protected!\n");
		return ret;
	}

	if (data) {
		uint32_t dcr;

		len = data->blocksize * data->blocks;

		/* 1. data disable + fifo reset */
		dcr = 0;
#ifdef CONFIG_FTSDC010_SDIO
		dcr |= FTSDC010_DCR_FIFO_RST;
#endif
		writel(dcr, &regs->dcr);

		/* 2. clear status register */
		writel(FTSDC010_STATUS_DATA_MASK | FTSDC010_STATUS_FIFO_URUN
			| FTSDC010_STATUS_FIFO_ORUN, &regs->clr);

		/* 3. data timeout (1 sec) */
		writel(chip->rate, &regs->dtr);

		/* 4. data length (bytes) */
		writel(len, &regs->dlr);

		/* 5. data enable */
		dcr = (ffs(data->blocksize) - 1) | FTSDC010_DCR_DATA_EN;
		if (data->flags & MMC_DATA_WRITE)
			dcr |= FTSDC010_DCR_DATA_WRITE;
		writel(dcr, &regs->dcr);
	}

	ret = ftsdc010_send_cmd(mmc, cmd);
	if (ret) {
		printf("ftsdc010: CMD%d failed\n", cmd->cmdidx);
		return ret;
	}

	if (!data)
		return ret;

	if (data->flags & MMC_DATA_WRITE) {
		const uint8_t *buf = (const uint8_t *)data->src;

		while (len > 0) {
			int wlen;

			/* wait for tx ready */
			ret = ftsdc010_wait(regs, FTSDC010_STATUS_FIFO_URUN);
			if (ret)
				break;

			/* write bytes to ftsdc010 */
			for (wlen = 0; wlen < len && wlen < chip->fifo; ) {
				writel(*(uint32_t *)buf, &regs->dwr);
				buf  += 4;
				wlen += 4;
			}

			len -= wlen;
		}

	} else {
		uint8_t *buf = (uint8_t *)data->dest;

		while (len > 0) {
			int rlen;

			/* wait for rx ready */
			ret = ftsdc010_wait(regs, FTSDC010_STATUS_FIFO_ORUN);
			if (ret)
				break;

			/* fetch bytes from ftsdc010 */
			for (rlen = 0; rlen < len && rlen < chip->fifo; ) {
				*(uint32_t *)buf = readl(&regs->dwr);
				buf  += 4;
				rlen += 4;
			}

			len -= rlen;
		}

	}

	if (!ret) {
		ret = ftsdc010_wait(regs,
			FTSDC010_STATUS_DATA_END | FTSDC010_STATUS_DATA_ERROR);
	}

	return ret;
}

static int ftsdc010_set_ios(struct mmc *mmc)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;

	ftsdc010_clkset(mmc, mmc->clock);

	clrbits_le32(&regs->bwr, FTSDC010_BWR_MODE_MASK);
	switch (mmc->bus_width) {
	case 4:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_4BIT);
		break;
	case 8:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_8BIT);
		break;
	default:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_1BIT);
		break;
	}

	return 0;
}

static int ftsdc010_init(struct mmc *mmc)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	uint32_t ts;

	if (readl(&regs->status) & FTSDC010_STATUS_CARD_DETECT)
		return -ENOMEDIUM;

	if (readl(&regs->status) & FTSDC010_STATUS_WRITE_PROT) {
		printf("ftsdc010: write protected\n");
		chip->wprot = 1;
	}

	chip->fifo = (readl(&regs->feature) & 0xff) << 2;

	/* 1. chip reset */
	writel(FTSDC010_CMD_SDC_RST, &regs->cmd);
	for (ts = get_timer(0); get_timer(ts) < CFG_RST_TIMEOUT; ) {
		if (readl(&regs->cmd) & FTSDC010_CMD_SDC_RST)
			continue;
		break;
	}
	if (readl(&regs->cmd) & FTSDC010_CMD_SDC_RST) {
		printf("ftsdc010: reset failed\n");
		return -EOPNOTSUPP;
	}

	/* 2. enter low speed mode (400k card detection) */
	ftsdc010_clkset(mmc, 400000);

	/* 3. interrupt disabled */
	writel(0, &regs->int_mask);

	return 0;
}

static const struct mmc_ops ftsdc010_ops = {
	.send_cmd	= ftsdc010_request,
	.set_ios	= ftsdc010_set_ios,
	.init		= ftsdc010_init,
};

int ftsdc010_mmc_init(int devid)
{
	struct mmc *mmc;
	struct ftsdc010_chip *chip;
	struct ftsdc010_mmc __iomem *regs;
#ifdef CONFIG_FTSDC010_BASE_LIST
	uint32_t base_list[] = CONFIG_FTSDC010_BASE_LIST;

	if (devid < 0 || devid >= ARRAY_SIZE(base_list))
		return -1;
	regs = (void __iomem *)base_list[devid];
#else
	regs = (void __iomem *)(CONFIG_FTSDC010_BASE + (devid << 20));
#endif

	chip = malloc(sizeof(struct ftsdc010_chip));
	if (!chip)
		return -ENOMEM;
	memset(chip, 0, sizeof(struct ftsdc010_chip));

	chip->regs = regs;
#ifdef CONFIG_SYS_CLK_FREQ
	chip->sclk = CONFIG_SYS_CLK_FREQ;
#else
	chip->sclk = clk_get_rate("SDC");
#endif

	chip->cfg.name = "ftsdc010";
	chip->cfg.ops = &ftsdc010_ops;
	chip->cfg.host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz;
	switch (readl(&regs->bwr) & FTSDC010_BWR_CAPS_MASK) {
	case FTSDC010_BWR_CAPS_4BIT:
		chip->cfg.host_caps |= MMC_MODE_4BIT;
		break;
	case FTSDC010_BWR_CAPS_8BIT:
		chip->cfg.host_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;
		break;
	default:
		break;
	}

	chip->cfg.voltages  = MMC_VDD_32_33 | MMC_VDD_33_34;
	chip->cfg.f_max     = chip->sclk / 2;
	chip->cfg.f_min     = chip->sclk / 0x100;

	chip->cfg.part_type = PART_TYPE_DOS;
	chip->cfg.b_max	    = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	mmc = mmc_create(&chip->cfg, chip);
	if (mmc == NULL) {
		free(chip);
		return -ENOMEM;
	}

	return 0;
}
