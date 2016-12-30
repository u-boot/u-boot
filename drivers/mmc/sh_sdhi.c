/*
 * drivers/mmc/sh_sdhi.c
 *
 * SD/MMC driver for Renesas rmobile ARM SoCs.
 *
 * Copyright (C) 2011,2013-2014 Renesas Electronics Corporation
 * Copyright (C) 2014 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2008-2009 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/sh_sdhi.h>

#define DRIVER_NAME "sh-sdhi"

struct sh_sdhi_host {
	unsigned long addr;
	int ch;
	int bus_shift;
	unsigned long quirks;
	unsigned char wait_int;
	unsigned char sd_error;
	unsigned char detect_waiting;
};
static inline void sh_sdhi_writew(struct sh_sdhi_host *host, int reg, u16 val)
{
	writew(val, host->addr + (reg << host->bus_shift));
}

static inline u16 sh_sdhi_readw(struct sh_sdhi_host *host, int reg)
{
	return readw(host->addr + (reg << host->bus_shift));
}

static void *mmc_priv(struct mmc *mmc)
{
	return (void *)mmc->priv;
}

static void sh_sdhi_detect(struct sh_sdhi_host *host)
{
	sh_sdhi_writew(host, SDHI_OPTION,
		       OPT_BUS_WIDTH_1 | sh_sdhi_readw(host, SDHI_OPTION));

	host->detect_waiting = 0;
}

static int sh_sdhi_intr(void *dev_id)
{
	struct sh_sdhi_host *host = dev_id;
	int state1 = 0, state2 = 0;

	state1 = sh_sdhi_readw(host, SDHI_INFO1);
	state2 = sh_sdhi_readw(host, SDHI_INFO2);

	debug("%s: state1 = %x, state2 = %x\n", __func__, state1, state2);

	/* CARD Insert */
	if (state1 & INFO1_CARD_IN) {
		sh_sdhi_writew(host, SDHI_INFO1, ~INFO1_CARD_IN);
		if (!host->detect_waiting) {
			host->detect_waiting = 1;
			sh_sdhi_detect(host);
		}
		sh_sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END |
			       INFO1M_ACCESS_END | INFO1M_CARD_IN |
			       INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
		return -EAGAIN;
	}
	/* CARD Removal */
	if (state1 & INFO1_CARD_RE) {
		sh_sdhi_writew(host, SDHI_INFO1, ~INFO1_CARD_RE);
		if (!host->detect_waiting) {
			host->detect_waiting = 1;
			sh_sdhi_detect(host);
		}
		sh_sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END |
			       INFO1M_ACCESS_END | INFO1M_CARD_RE |
			       INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
		sh_sdhi_writew(host, SDHI_SDIO_INFO1_MASK, SDIO_INFO1M_ON);
		sh_sdhi_writew(host, SDHI_SDIO_MODE, SDIO_MODE_OFF);
		return -EAGAIN;
	}

	if (state2 & INFO2_ALL_ERR) {
		sh_sdhi_writew(host, SDHI_INFO2,
			       (unsigned short)~(INFO2_ALL_ERR));
		sh_sdhi_writew(host, SDHI_INFO2_MASK,
			       INFO2M_ALL_ERR |
			       sh_sdhi_readw(host, SDHI_INFO2_MASK));
		host->sd_error = 1;
		host->wait_int = 1;
		return 0;
	}
	/* Respons End */
	if (state1 & INFO1_RESP_END) {
		sh_sdhi_writew(host, SDHI_INFO1, ~INFO1_RESP_END);
		sh_sdhi_writew(host, SDHI_INFO1_MASK,
			       INFO1M_RESP_END |
			       sh_sdhi_readw(host, SDHI_INFO1_MASK));
		host->wait_int = 1;
		return 0;
	}
	/* SD_BUF Read Enable */
	if (state2 & INFO2_BRE_ENABLE) {
		sh_sdhi_writew(host, SDHI_INFO2, ~INFO2_BRE_ENABLE);
		sh_sdhi_writew(host, SDHI_INFO2_MASK,
			       INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ |
			       sh_sdhi_readw(host, SDHI_INFO2_MASK));
		host->wait_int = 1;
		return 0;
	}
	/* SD_BUF Write Enable */
	if (state2 & INFO2_BWE_ENABLE) {
		sh_sdhi_writew(host, SDHI_INFO2, ~INFO2_BWE_ENABLE);
		sh_sdhi_writew(host, SDHI_INFO2_MASK,
			       INFO2_BWE_ENABLE | INFO2M_BUF_ILL_WRITE |
			       sh_sdhi_readw(host, SDHI_INFO2_MASK));
		host->wait_int = 1;
		return 0;
	}
	/* Access End */
	if (state1 & INFO1_ACCESS_END) {
		sh_sdhi_writew(host, SDHI_INFO1, ~INFO1_ACCESS_END);
		sh_sdhi_writew(host, SDHI_INFO1_MASK,
			       INFO1_ACCESS_END |
			       sh_sdhi_readw(host, SDHI_INFO1_MASK));
		host->wait_int = 1;
		return 0;
	}
	return -EAGAIN;
}

static int sh_sdhi_wait_interrupt_flag(struct sh_sdhi_host *host)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			debug(DRIVER_NAME": %s timeout\n", __func__);
			return 0;
		}

		if (!sh_sdhi_intr(host))
			break;

		udelay(1);	/* 1 usec */
	}

	return 1; /* Return value: NOT 0 = complete waiting */
}

static int sh_sdhi_clock_control(struct sh_sdhi_host *host, unsigned long clk)
{
	u32 clkdiv, i, timeout;

	if (sh_sdhi_readw(host, SDHI_INFO2) & (1 << 14)) {
		printf(DRIVER_NAME": Busy state ! Cannot change the clock\n");
		return -EBUSY;
	}

	sh_sdhi_writew(host, SDHI_CLK_CTRL,
		       ~CLK_ENABLE & sh_sdhi_readw(host, SDHI_CLK_CTRL));

	if (clk == 0)
		return -EIO;

	clkdiv = 0x80;
	i = CONFIG_SH_SDHI_FREQ >> (0x8 + 1);
	for (; clkdiv && clk >= (i << 1); (clkdiv >>= 1))
		i <<= 1;

	sh_sdhi_writew(host, SDHI_CLK_CTRL, clkdiv);

	timeout = 100000;
	/* Waiting for SD Bus busy to be cleared */
	while (timeout--) {
		if ((sh_sdhi_readw(host, SDHI_INFO2) & 0x2000))
			break;
	}

	if (timeout)
		sh_sdhi_writew(host, SDHI_CLK_CTRL,
			       CLK_ENABLE | sh_sdhi_readw(host, SDHI_CLK_CTRL));
	else
		return -EBUSY;

	return 0;
}

static int sh_sdhi_sync_reset(struct sh_sdhi_host *host)
{
	u32 timeout;
	sh_sdhi_writew(host, SDHI_SOFT_RST, SOFT_RST_ON);
	sh_sdhi_writew(host, SDHI_SOFT_RST, SOFT_RST_OFF);
	sh_sdhi_writew(host, SDHI_CLK_CTRL,
		       CLK_ENABLE | sh_sdhi_readw(host, SDHI_CLK_CTRL));

	timeout = 100000;
	while (timeout--) {
		if (!(sh_sdhi_readw(host, SDHI_INFO2) & INFO2_CBUSY))
			break;
		udelay(100);
	}

	if (!timeout)
		return -EBUSY;

	if (host->quirks & SH_SDHI_QUIRK_16BIT_BUF)
		sh_sdhi_writew(host, SDHI_HOST_MODE, 1);

	return 0;
}

static int sh_sdhi_error_manage(struct sh_sdhi_host *host)
{
	unsigned short e_state1, e_state2;
	int ret;

	host->sd_error = 0;
	host->wait_int = 0;

	e_state1 = sh_sdhi_readw(host, SDHI_ERR_STS1);
	e_state2 = sh_sdhi_readw(host, SDHI_ERR_STS2);
	if (e_state2 & ERR_STS2_SYS_ERROR) {
		if (e_state2 & ERR_STS2_RES_STOP_TIMEOUT)
			ret = -ETIMEDOUT;
		else
			ret = -EILSEQ;
		debug("%s: ERR_STS2 = %04x\n",
		      DRIVER_NAME, sh_sdhi_readw(host, SDHI_ERR_STS2));
		sh_sdhi_sync_reset(host);

		sh_sdhi_writew(host, SDHI_INFO1_MASK,
			       INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
		return ret;
	}
	if (e_state1 & ERR_STS1_CRC_ERROR || e_state1 & ERR_STS1_CMD_ERROR)
		ret = -EILSEQ;
	else
		ret = -ETIMEDOUT;

	debug("%s: ERR_STS1 = %04x\n",
	      DRIVER_NAME, sh_sdhi_readw(host, SDHI_ERR_STS1));
	sh_sdhi_sync_reset(host);
	sh_sdhi_writew(host, SDHI_INFO1_MASK,
		       INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
	return ret;
}

static int sh_sdhi_single_read(struct sh_sdhi_host *host, struct mmc_data *data)
{
	long time;
	unsigned short blocksize, i;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		debug(DRIVER_NAME": %s: The data pointer is unaligned.",
		      __func__);
		return -EIO;
	}

	host->wait_int = 0;
	sh_sdhi_writew(host, SDHI_INFO2_MASK,
		       ~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
		       sh_sdhi_readw(host, SDHI_INFO2_MASK));
	sh_sdhi_writew(host, SDHI_INFO1_MASK,
		       ~INFO1M_ACCESS_END &
		       sh_sdhi_readw(host, SDHI_INFO1_MASK));
	time = sh_sdhi_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_sdhi_error_manage(host);

	host->wait_int = 0;
	blocksize = sh_sdhi_readw(host, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++)
		*p++ = sh_sdhi_readw(host, SDHI_BUF0);

	time = sh_sdhi_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_sdhi_error_manage(host);

	host->wait_int = 0;
	return 0;
}

static int sh_sdhi_multi_read(struct sh_sdhi_host *host, struct mmc_data *data)
{
	long time;
	unsigned short blocksize, i, sec;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		debug(DRIVER_NAME": %s: The data pointer is unaligned.",
		      __func__);
		return -EIO;
	}

	debug("%s: blocks = %d, blocksize = %d\n",
	      __func__, data->blocks, data->blocksize);

	host->wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sh_sdhi_writew(host, SDHI_INFO2_MASK,
			       ~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
			       sh_sdhi_readw(host, SDHI_INFO2_MASK));

		time = sh_sdhi_wait_interrupt_flag(host);
		if (time == 0 || host->sd_error != 0)
			return sh_sdhi_error_manage(host);

		host->wait_int = 0;
		blocksize = sh_sdhi_readw(host, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++)
			*p++ = sh_sdhi_readw(host, SDHI_BUF0);
	}

	return 0;
}

static int sh_sdhi_single_write(struct sh_sdhi_host *host,
		struct mmc_data *data)
{
	long time;
	unsigned short blocksize, i;
	const unsigned short *p = (const unsigned short *)data->src;

	if ((unsigned long)p & 0x00000001) {
		debug(DRIVER_NAME": %s: The data pointer is unaligned.",
		      __func__);
		return -EIO;
	}

	debug("%s: blocks = %d, blocksize = %d\n",
	      __func__, data->blocks, data->blocksize);

	host->wait_int = 0;
	sh_sdhi_writew(host, SDHI_INFO2_MASK,
		       ~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
		       sh_sdhi_readw(host, SDHI_INFO2_MASK));
	sh_sdhi_writew(host, SDHI_INFO1_MASK,
		       ~INFO1M_ACCESS_END &
		       sh_sdhi_readw(host, SDHI_INFO1_MASK));

	time = sh_sdhi_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_sdhi_error_manage(host);

	host->wait_int = 0;
	blocksize = sh_sdhi_readw(host, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++)
		sh_sdhi_writew(host, SDHI_BUF0, *p++);

	time = sh_sdhi_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_sdhi_error_manage(host);

	host->wait_int = 0;
	return 0;
}

static int sh_sdhi_multi_write(struct sh_sdhi_host *host, struct mmc_data *data)
{
	long time;
	unsigned short i, sec, blocksize;
	const unsigned short *p = (const unsigned short *)data->src;

	debug("%s: blocks = %d, blocksize = %d\n",
	      __func__, data->blocks, data->blocksize);

	host->wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sh_sdhi_writew(host, SDHI_INFO2_MASK,
			       ~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
			       sh_sdhi_readw(host, SDHI_INFO2_MASK));

		time = sh_sdhi_wait_interrupt_flag(host);
		if (time == 0 || host->sd_error != 0)
			return sh_sdhi_error_manage(host);

		host->wait_int = 0;
		blocksize = sh_sdhi_readw(host, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++)
			sh_sdhi_writew(host, SDHI_BUF0, *p++);
	}

	return 0;
}

static void sh_sdhi_get_response(struct sh_sdhi_host *host, struct mmc_cmd *cmd)
{
	unsigned short i, j, cnt = 1;
	unsigned short resp[8];

	if (cmd->resp_type & MMC_RSP_136) {
		cnt = 4;
		resp[0] = sh_sdhi_readw(host, SDHI_RSP00);
		resp[1] = sh_sdhi_readw(host, SDHI_RSP01);
		resp[2] = sh_sdhi_readw(host, SDHI_RSP02);
		resp[3] = sh_sdhi_readw(host, SDHI_RSP03);
		resp[4] = sh_sdhi_readw(host, SDHI_RSP04);
		resp[5] = sh_sdhi_readw(host, SDHI_RSP05);
		resp[6] = sh_sdhi_readw(host, SDHI_RSP06);
		resp[7] = sh_sdhi_readw(host, SDHI_RSP07);

		/* SDHI REGISTER SPECIFICATION */
		for (i = 7, j = 6; i > 0; i--) {
			resp[i] = (resp[i] << 8) & 0xff00;
			resp[i] |= (resp[j--] >> 8) & 0x00ff;
		}
		resp[0] = (resp[0] << 8) & 0xff00;
	} else {
		resp[0] = sh_sdhi_readw(host, SDHI_RSP00);
		resp[1] = sh_sdhi_readw(host, SDHI_RSP01);
	}

#if defined(__BIG_ENDIAN_BITFIELD)
	if (cnt == 4) {
		cmd->response[0] = (resp[6] << 16) | resp[7];
		cmd->response[1] = (resp[4] << 16) | resp[5];
		cmd->response[2] = (resp[2] << 16) | resp[3];
		cmd->response[3] = (resp[0] << 16) | resp[1];
	} else {
		cmd->response[0] = (resp[0] << 16) | resp[1];
	}
#else
	if (cnt == 4) {
		cmd->response[0] = (resp[7] << 16) | resp[6];
		cmd->response[1] = (resp[5] << 16) | resp[4];
		cmd->response[2] = (resp[3] << 16) | resp[2];
		cmd->response[3] = (resp[1] << 16) | resp[0];
	} else {
		cmd->response[0] = (resp[1] << 16) | resp[0];
	}
#endif /* __BIG_ENDIAN_BITFIELD */
}

static unsigned short sh_sdhi_set_cmd(struct sh_sdhi_host *host,
			struct mmc_data *data, unsigned short opc)
{
	switch (opc) {
	case SD_CMD_APP_SEND_OP_COND:
	case SD_CMD_APP_SEND_SCR:
		opc |= SDHI_APP;
		break;
	case SD_CMD_APP_SET_BUS_WIDTH:
		 /* SD_APP_SET_BUS_WIDTH*/
		if (!data)
			opc |= SDHI_APP;
		else /* SD_SWITCH */
			opc = SDHI_SD_SWITCH;
		break;
	default:
		break;
	}
	return opc;
}

static unsigned short sh_sdhi_data_trans(struct sh_sdhi_host *host,
			struct mmc_data *data, unsigned short opc)
{
	unsigned short ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = sh_sdhi_multi_read(host, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = sh_sdhi_multi_write(host, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = sh_sdhi_single_write(host, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case SDHI_SD_APP_SEND_SCR:
	case SDHI_SD_SWITCH: /* SD_SWITCH */
		ret = sh_sdhi_single_read(host, data);
		break;
	default:
		printf(DRIVER_NAME": SD: NOT SUPPORT CMD = d'%04d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int sh_sdhi_start_cmd(struct sh_sdhi_host *host,
			struct mmc_data *data, struct mmc_cmd *cmd)
{
	long time;
	unsigned short opc = cmd->cmdidx;
	int ret = 0;
	unsigned long timeout;

	debug("opc = %d, arg = %x, resp_type = %x\n",
	      opc, cmd->cmdarg, cmd->resp_type);

	if (opc == MMC_CMD_STOP_TRANSMISSION) {
		/* SDHI sends the STOP command automatically by STOP reg */
		sh_sdhi_writew(host, SDHI_INFO1_MASK, ~INFO1M_ACCESS_END &
			       sh_sdhi_readw(host, SDHI_INFO1_MASK));

		time = sh_sdhi_wait_interrupt_flag(host);
		if (time == 0 || host->sd_error != 0)
			return sh_sdhi_error_manage(host);

		sh_sdhi_get_response(host, cmd);
		return 0;
	}

	if (data) {
		if ((opc == MMC_CMD_READ_MULTIPLE_BLOCK) ||
		    opc == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
			sh_sdhi_writew(host, SDHI_STOP, STOP_SEC_ENABLE);
			sh_sdhi_writew(host, SDHI_SECCNT, data->blocks);
		}
		sh_sdhi_writew(host, SDHI_SIZE, data->blocksize);
	}
	opc = sh_sdhi_set_cmd(host, data, opc);

	/*
	 *  U-Boot cannot use interrupt.
	 *  So this flag may not be clear by timing
	 */
	sh_sdhi_writew(host, SDHI_INFO1, ~INFO1_RESP_END);

	sh_sdhi_writew(host, SDHI_INFO1_MASK,
		       INFO1M_RESP_END | sh_sdhi_readw(host, SDHI_INFO1_MASK));
	sh_sdhi_writew(host, SDHI_ARG0,
		       (unsigned short)(cmd->cmdarg & ARG0_MASK));
	sh_sdhi_writew(host, SDHI_ARG1,
		       (unsigned short)((cmd->cmdarg >> 16) & ARG1_MASK));

	timeout = 100000;
	/* Waiting for SD Bus busy to be cleared */
	while (timeout--) {
		if ((sh_sdhi_readw(host, SDHI_INFO2) & 0x2000))
			break;
	}

	sh_sdhi_writew(host, SDHI_CMD, (unsigned short)(opc & CMD_MASK));

	host->wait_int = 0;
	sh_sdhi_writew(host, SDHI_INFO1_MASK,
		       ~INFO1M_RESP_END & sh_sdhi_readw(host, SDHI_INFO1_MASK));
	sh_sdhi_writew(host, SDHI_INFO2_MASK,
		       ~(INFO2M_CMD_ERROR | INFO2M_CRC_ERROR |
		       INFO2M_END_ERROR | INFO2M_TIMEOUT |
		       INFO2M_RESP_TIMEOUT | INFO2M_ILA) &
		       sh_sdhi_readw(host, SDHI_INFO2_MASK));

	time = sh_sdhi_wait_interrupt_flag(host);
	if (!time)
		return sh_sdhi_error_manage(host);

	if (host->sd_error) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case SD_CMD_SEND_IF_COND:
		case MMC_CMD_APP_CMD:
			ret = -ETIMEDOUT;
			break;
		default:
			debug(DRIVER_NAME": Cmd(d'%d) err\n", opc);
			debug(DRIVER_NAME": cmdidx = %d\n", cmd->cmdidx);
			ret = sh_sdhi_error_manage(host);
			break;
		}
		host->sd_error = 0;
		host->wait_int = 0;
		return ret;
	}
	if (sh_sdhi_readw(host, SDHI_INFO1) & INFO1_RESP_END)
		return -EINVAL;

	if (host->wait_int) {
		sh_sdhi_get_response(host, cmd);
		host->wait_int = 0;
	}
	if (data)
		ret = sh_sdhi_data_trans(host, data, opc);

	debug("ret = %d, resp = %08x, %08x, %08x, %08x\n",
	      ret, cmd->response[0], cmd->response[1],
	      cmd->response[2], cmd->response[3]);
	return ret;
}

static int sh_sdhi_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sh_sdhi_host *host = mmc_priv(mmc);
	int ret;

	host->sd_error = 0;

	ret = sh_sdhi_start_cmd(host, data, cmd);

	return ret;
}

static int sh_sdhi_set_ios(struct mmc *mmc)
{
	int ret;
	struct sh_sdhi_host *host = mmc_priv(mmc);

	ret = sh_sdhi_clock_control(host, mmc->clock);
	if (ret)
		return -EINVAL;

	if (mmc->bus_width == 4)
		sh_sdhi_writew(host, SDHI_OPTION, ~OPT_BUS_WIDTH_1 &
			       sh_sdhi_readw(host, SDHI_OPTION));
	else
		sh_sdhi_writew(host, SDHI_OPTION, OPT_BUS_WIDTH_1 |
			       sh_sdhi_readw(host, SDHI_OPTION));

	debug("clock = %d, buswidth = %d\n", mmc->clock, mmc->bus_width);

	return 0;
}

static int sh_sdhi_initialize(struct mmc *mmc)
{
	struct sh_sdhi_host *host = mmc_priv(mmc);
	int ret = sh_sdhi_sync_reset(host);

	sh_sdhi_writew(host, SDHI_PORTSEL, USE_1PORT);

#if defined(__BIG_ENDIAN_BITFIELD)
	sh_sdhi_writew(host, SDHI_EXT_SWAP, SET_SWAP);
#endif

	sh_sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END |
		       INFO1M_ACCESS_END | INFO1M_CARD_RE |
		       INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);

	return ret;
}

static const struct mmc_ops sh_sdhi_ops = {
	.send_cmd       = sh_sdhi_send_cmd,
	.set_ios        = sh_sdhi_set_ios,
	.init           = sh_sdhi_initialize,
};

static struct mmc_config sh_sdhi_cfg = {
	.name           = DRIVER_NAME,
	.ops            = &sh_sdhi_ops,
	.f_min          = CLKDEV_INIT,
	.f_max          = CLKDEV_HS_DATA,
	.voltages       = MMC_VDD_32_33 | MMC_VDD_33_34,
	.host_caps      = MMC_MODE_4BIT | MMC_MODE_HS,
	.part_type      = PART_TYPE_DOS,
	.b_max          = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

int sh_sdhi_init(unsigned long addr, int ch, unsigned long quirks)
{
	int ret = 0;
	struct mmc *mmc;
	struct sh_sdhi_host *host = NULL;

	if (ch >= CONFIG_SYS_SH_SDHI_NR_CHANNEL)
		return -ENODEV;

	host = malloc(sizeof(struct sh_sdhi_host));
	if (!host)
		return -ENOMEM;

	mmc = mmc_create(&sh_sdhi_cfg, host);
	if (!mmc) {
		ret = -1;
		goto error;
	}

	host->ch = ch;
	host->addr = addr;
	host->quirks = quirks;

	if (host->quirks & SH_SDHI_QUIRK_16BIT_BUF)
		host->bus_shift = 1;

	return ret;
error:
	if (host)
		free(host);
	return ret;
}
