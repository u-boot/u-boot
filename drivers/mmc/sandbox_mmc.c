// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <mmc.h>
#include <asm/test.h>

struct sandbox_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

#define MMC_CSIZE 0
#define MMC_CMULT 8 /* 8 because the card is high-capacity */
#define MMC_BL_LEN_SHIFT 10
#define MMC_BL_LEN BIT(MMC_BL_LEN_SHIFT)
#define MMC_CAPACITY (((MMC_CSIZE + 1) << (MMC_CMULT + 2)) \
		      * MMC_BL_LEN) /* 1 MiB */

struct sandbox_mmc_priv {
	u8 buf[MMC_CAPACITY];
};

/**
 * sandbox_mmc_send_cmd() - Emulate SD commands
 *
 * This emulate an SD card version 2. Single-block reads result in zero data.
 * Multiple-block reads return a test string.
 */
static int sandbox_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct sandbox_mmc_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	static ulong erase_start, erase_end;

	switch (cmd->cmdidx) {
	case MMC_CMD_ALL_SEND_CID:
		memset(cmd->response, '\0', sizeof(cmd->response));
		break;
	case SD_CMD_SEND_RELATIVE_ADDR:
		cmd->response[0] = 0 << 16; /* mmc->rca */
	case MMC_CMD_GO_IDLE_STATE:
		break;
	case SD_CMD_SEND_IF_COND:
		cmd->response[0] = 0xaa;
		break;
	case MMC_CMD_SEND_STATUS:
		cmd->response[0] = MMC_STATUS_RDY_FOR_DATA;
		break;
	case MMC_CMD_SELECT_CARD:
		break;
	case MMC_CMD_SEND_CSD:
		cmd->response[0] = 0;
		cmd->response[1] = (MMC_BL_LEN_SHIFT << 16) |
				   ((MMC_CSIZE >> 16) & 0x3f);
		cmd->response[2] = (MMC_CSIZE & 0xffff) << 16;
		cmd->response[3] = 0;
		break;
	case SD_CMD_SWITCH_FUNC: {
		if (!data)
			break;
		u32 *resp = (u32 *)data->dest;
		resp[3] = 0;
		resp[7] = cpu_to_be32(SD_HIGHSPEED_BUSY);
		if ((cmd->cmdarg & 0xF) == UHS_SDR12_BUS_SPEED)
			resp[4] = (cmd->cmdarg & 0xF) << 24;
		break;
	}
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		memcpy(data->dest, &priv->buf[cmd->cmdarg * data->blocksize],
		       data->blocks * data->blocksize);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		memcpy(&priv->buf[cmd->cmdarg * data->blocksize], data->src,
		       data->blocks * data->blocksize);
		break;
	case MMC_CMD_STOP_TRANSMISSION:
		break;
	case SD_CMD_ERASE_WR_BLK_START:
		erase_start = cmd->cmdarg;
		break;
	case SD_CMD_ERASE_WR_BLK_END:
		erase_end = cmd->cmdarg;
		break;
	case MMC_CMD_ERASE:
		memset(&priv->buf[erase_start * mmc->write_bl_len], '\0',
		       (erase_end - erase_start + 1) * mmc->write_bl_len);
		break;
	case SD_CMD_APP_SEND_OP_COND:
		cmd->response[0] = OCR_BUSY | OCR_HCS;
		cmd->response[1] = 0;
		cmd->response[2] = 0;
		break;
	case MMC_CMD_APP_CMD:
		break;
	case MMC_CMD_SET_BLOCKLEN:
		debug("block len %d\n", cmd->cmdarg);
		break;
	case SD_CMD_APP_SEND_SCR: {
		u32 *scr = (u32 *)data->dest;

		scr[0] = cpu_to_be32(2 << 24 | 1 << 15);  /* SD version 3 */
		break;
	}
	default:
		debug("%s: Unknown command %d\n", __func__, cmd->cmdidx);
		break;
	}

	return 0;
}

static int sandbox_mmc_set_ios(struct udevice *dev)
{
	return 0;
}

static int sandbox_mmc_get_cd(struct udevice *dev)
{
	return 1;
}

static const struct dm_mmc_ops sandbox_mmc_ops = {
	.send_cmd = sandbox_mmc_send_cmd,
	.set_ios = sandbox_mmc_set_ios,
	.get_cd = sandbox_mmc_get_cd,
};

static int sandbox_mmc_of_to_plat(struct udevice *dev)
{
	struct sandbox_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct blk_desc *blk;
	int ret;

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;
	blk = mmc_get_blk_desc(&plat->mmc);
	if (blk)
		blk->removable = !(cfg->host_caps & MMC_CAP_NONREMOVABLE);

	return 0;
}

static int sandbox_mmc_probe(struct udevice *dev)
{
	struct sandbox_mmc_plat *plat = dev_get_plat(dev);

	return mmc_init(&plat->mmc);
}

static int sandbox_mmc_bind(struct udevice *dev)
{
	struct sandbox_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;

	cfg->name = dev->name;
	cfg->host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_8BIT;
	cfg->voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->f_min = 1000000;
	cfg->f_max = 52000000;
	cfg->b_max = U32_MAX;

	return mmc_bind(dev, &plat->mmc, cfg);
}

static int sandbox_mmc_unbind(struct udevice *dev)
{
	mmc_unbind(dev);

	return 0;
}

static const struct udevice_id sandbox_mmc_ids[] = {
	{ .compatible = "sandbox,mmc" },
	{ }
};

U_BOOT_DRIVER(mmc_sandbox) = {
	.name		= "mmc_sandbox",
	.id		= UCLASS_MMC,
	.of_match	= sandbox_mmc_ids,
	.ops		= &sandbox_mmc_ops,
	.bind		= sandbox_mmc_bind,
	.unbind		= sandbox_mmc_unbind,
	.of_to_plat	= sandbox_mmc_of_to_plat,
	.probe		= sandbox_mmc_probe,
	.priv_auto = sizeof(struct sandbox_mmc_priv),
	.plat_auto = sizeof(struct sandbox_mmc_plat),
};
