/*
 * Driver for Blackfin on-chip SDH controller
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>
#include <asm/blackfin.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/sdh.h>
#include <asm/mach-common/bits/dma.h>

#if defined(__ADSPBF50x__) || defined(__ADSPBF51x__) || defined(__ADSPBF60x__)
# define bfin_read_SDH_CLK_CTL		bfin_read_RSI_CLK_CONTROL
# define bfin_write_SDH_CLK_CTL		bfin_write_RSI_CLK_CONTROL
# define bfin_write_SDH_ARGUMENT	bfin_write_RSI_ARGUMENT
# define bfin_write_SDH_COMMAND		bfin_write_RSI_COMMAND
# define bfin_read_SDH_RESPONSE0	bfin_read_RSI_RESPONSE0
# define bfin_read_SDH_RESPONSE1	bfin_read_RSI_RESPONSE1
# define bfin_read_SDH_RESPONSE2	bfin_read_RSI_RESPONSE2
# define bfin_read_SDH_RESPONSE3	bfin_read_RSI_RESPONSE3
# define bfin_write_SDH_DATA_TIMER	bfin_write_RSI_DATA_TIMER
# define bfin_write_SDH_DATA_LGTH	bfin_write_RSI_DATA_LGTH
# define bfin_read_SDH_DATA_CTL		bfin_read_RSI_DATA_CONTROL
# define bfin_write_SDH_DATA_CTL	bfin_write_RSI_DATA_CONTROL
# define bfin_read_SDH_STATUS		bfin_read_RSI_STATUS
# define bfin_write_SDH_STATUS_CLR 	bfin_write_RSI_STATUSCL
# define bfin_read_SDH_CFG		bfin_read_RSI_CONFIG
# define bfin_write_SDH_CFG		bfin_write_RSI_CONFIG
# if defined(__ADSPBF60x__)
# define bfin_read_SDH_BLK_SIZE		bfin_read_RSI_BLKSZ
# define bfin_write_SDH_BLK_SIZE	bfin_write_RSI_BLKSZ
# define bfin_write_DMA_START_ADDR	bfin_write_DMA10_START_ADDR
# define bfin_write_DMA_X_COUNT		bfin_write_DMA10_X_COUNT
# define bfin_write_DMA_X_MODIFY	bfin_write_DMA10_X_MODIFY
# define bfin_write_DMA_CONFIG		bfin_write_DMA10_CONFIG
# else
# define bfin_read_SDH_PWR_CTL		bfin_read_RSI_PWR_CONTROL
# define bfin_write_SDH_PWR_CTL		bfin_write_RSI_PWR_CONTROL
# define bfin_write_DMA_START_ADDR	bfin_write_DMA4_START_ADDR
# define bfin_write_DMA_X_COUNT		bfin_write_DMA4_X_COUNT
# define bfin_write_DMA_X_MODIFY	bfin_write_DMA4_X_MODIFY
# define bfin_write_DMA_CONFIG		bfin_write_DMA4_CONFIG
# endif
# define PORTMUX_PINS \
	{ P_RSI_DATA0, P_RSI_DATA1, P_RSI_DATA2, P_RSI_DATA3, P_RSI_CMD, P_RSI_CLK, 0 }
#elif defined(__ADSPBF54x__)
# define bfin_write_DMA_START_ADDR	bfin_write_DMA22_START_ADDR
# define bfin_write_DMA_X_COUNT		bfin_write_DMA22_X_COUNT
# define bfin_write_DMA_X_MODIFY	bfin_write_DMA22_X_MODIFY
# define bfin_write_DMA_CONFIG		bfin_write_DMA22_CONFIG
# define PORTMUX_PINS \
	{ P_SD_D0, P_SD_D1, P_SD_D2, P_SD_D3, P_SD_CLK, P_SD_CMD, 0 }
#else
# error no support for this proc yet
#endif

static int
sdh_send_cmd(struct mmc *mmc, struct mmc_cmd *mmc_cmd)
{
	unsigned int status, timeout;
	int cmd = mmc_cmd->cmdidx;
	int flags = mmc_cmd->resp_type;
	int arg = mmc_cmd->cmdarg;
	int ret;
	u16 sdh_cmd;

	sdh_cmd = cmd | CMD_E;
	if (flags & MMC_RSP_PRESENT)
		sdh_cmd |= CMD_RSP;
	if (flags & MMC_RSP_136)
		sdh_cmd |= CMD_L_RSP;
#ifdef RSI_BLKSZ
	sdh_cmd |= CMD_DATA0_BUSY;
#endif

	bfin_write_SDH_ARGUMENT(arg);
	bfin_write_SDH_COMMAND(sdh_cmd);

	/* wait for a while */
	timeout = 0;
	do {
		if (++timeout > 1000000) {
			status = CMD_TIME_OUT;
			break;
		}
		udelay(1);
		status = bfin_read_SDH_STATUS();
	} while (!(status & (CMD_SENT | CMD_RESP_END | CMD_TIME_OUT |
		CMD_CRC_FAIL)));

	if (flags & MMC_RSP_PRESENT) {
		mmc_cmd->response[0] = bfin_read_SDH_RESPONSE0();
		if (flags & MMC_RSP_136) {
			mmc_cmd->response[1] = bfin_read_SDH_RESPONSE1();
			mmc_cmd->response[2] = bfin_read_SDH_RESPONSE2();
			mmc_cmd->response[3] = bfin_read_SDH_RESPONSE3();
		}
	}

	if (status & CMD_TIME_OUT)
		ret = TIMEOUT;
	else if (status & CMD_CRC_FAIL && flags & MMC_RSP_CRC)
		ret = COMM_ERR;
	else
		ret = 0;

	bfin_write_SDH_STATUS_CLR(CMD_SENT_STAT | CMD_RESP_END_STAT |
				CMD_TIMEOUT_STAT | CMD_CRC_FAIL_STAT);
#ifdef RSI_BLKSZ
	/* wait till card ready */
	while (!(bfin_read_RSI_ESTAT() & SD_CARD_READY))
		continue;
	bfin_write_RSI_ESTAT(SD_CARD_READY);
#endif

	return ret;
}

/* set data for single block transfer */
static int sdh_setup_data(struct mmc *mmc, struct mmc_data *data)
{
	u16 data_ctl = 0;
	u16 dma_cfg = 0;
	unsigned long data_size = data->blocksize * data->blocks;

	/* Don't support write yet. */
	if (data->flags & MMC_DATA_WRITE)
		return UNUSABLE_ERR;
#ifndef RSI_BLKSZ
	data_ctl |= ((ffs(data_size) - 1) << 4);
#else
	bfin_write_SDH_BLK_SIZE(data_size);
#endif
	data_ctl |= DTX_DIR;
	bfin_write_SDH_DATA_CTL(data_ctl);
	dma_cfg = WDSIZE_32 | PSIZE_32 | RESTART | WNR | DMAEN;

	bfin_write_SDH_DATA_TIMER(-1);

	blackfin_dcache_flush_invalidate_range(data->dest,
			data->dest + data_size);
	/* configure DMA */
	bfin_write_DMA_START_ADDR(data->dest);
	bfin_write_DMA_X_COUNT(data_size / 4);
	bfin_write_DMA_X_MODIFY(4);
	bfin_write_DMA_CONFIG(dma_cfg);
	bfin_write_SDH_DATA_LGTH(data_size);
	/* kick off transfer */
	bfin_write_SDH_DATA_CTL(bfin_read_SDH_DATA_CTL() | DTX_DMA_E | DTX_E);

	return 0;
}


static int bfin_sdh_request(struct mmc *mmc, struct mmc_cmd *cmd,
		struct mmc_data *data)
{
	u32 status;
	int ret = 0;

	if (data) {
		ret = sdh_setup_data(mmc, data);
		if (ret)
			return ret;
	}

	ret = sdh_send_cmd(mmc, cmd);
	if (ret) {
		bfin_write_SDH_COMMAND(0);
		bfin_write_DMA_CONFIG(0);
		bfin_write_SDH_DATA_CTL(0);
		SSYNC();
		printf("sending CMD%d failed\n", cmd->cmdidx);
		return ret;
	}

	if (data) {
		do {
			udelay(1);
			status = bfin_read_SDH_STATUS();
		} while (!(status & (DAT_BLK_END | DAT_END | DAT_TIME_OUT | DAT_CRC_FAIL | RX_OVERRUN)));

		if (status & DAT_TIME_OUT) {
			bfin_write_SDH_STATUS_CLR(DAT_TIMEOUT_STAT);
			ret |= TIMEOUT;
		} else if (status & (DAT_CRC_FAIL | RX_OVERRUN)) {
			bfin_write_SDH_STATUS_CLR(DAT_CRC_FAIL_STAT | RX_OVERRUN_STAT);
			ret |= COMM_ERR;
		} else
			bfin_write_SDH_STATUS_CLR(DAT_BLK_END_STAT | DAT_END_STAT);

		if (ret) {
			printf("tranfering data failed\n");
			return ret;
		}
	}
	return 0;
}

static void sdh_set_clk(unsigned long clk)
{
	unsigned long sys_clk;
	unsigned long clk_div;
	u16 clk_ctl = 0;

	clk_ctl = bfin_read_SDH_CLK_CTL();
	if (clk) {
		/* setting SD_CLK */
		sys_clk = get_sclk();
		bfin_write_SDH_CLK_CTL(clk_ctl & ~CLK_E);
		if (sys_clk % (2 * clk) == 0)
			clk_div = sys_clk / (2 * clk) - 1;
		else
			clk_div = sys_clk / (2 * clk);

		if (clk_div > 0xff)
			clk_div = 0xff;
		clk_ctl |= (clk_div & 0xff);
		clk_ctl |= CLK_E;
		bfin_write_SDH_CLK_CTL(clk_ctl);
	} else
		bfin_write_SDH_CLK_CTL(clk_ctl & ~CLK_E);
}

static void bfin_sdh_set_ios(struct mmc *mmc)
{
	u16 cfg = 0;
	u16 clk_ctl = 0;

	if (mmc->bus_width == 4) {
		cfg = bfin_read_SDH_CFG();
#ifndef RSI_BLKSZ
		cfg &= ~PD_SDDAT3;
#endif
		cfg |= PUP_SDDAT3;
		bfin_write_SDH_CFG(cfg);
		clk_ctl |= WIDE_BUS_4;
	}
	bfin_write_SDH_CLK_CTL(clk_ctl);
	sdh_set_clk(mmc->clock);
}

static int bfin_sdh_init(struct mmc *mmc)
{
	const unsigned short pins[] = PORTMUX_PINS;
	int ret;

	/* Initialize sdh controller */
	ret = peripheral_request_list(pins, "bfin_sdh");
	if (ret < 0)
		return ret;
#if defined(__ADSPBF54x__)
	bfin_write_DMAC1_PERIMUX(bfin_read_DMAC1_PERIMUX() | 0x1);
#endif
	bfin_write_SDH_CFG(bfin_read_SDH_CFG() | CLKS_EN);
	/* Disable card detect pin */
	bfin_write_SDH_CFG((bfin_read_SDH_CFG() & 0x1F) | 0x60);
#ifndef RSI_BLKSZ
	bfin_write_SDH_PWR_CTL(PWR_ON | ROD_CTL);
#else
	bfin_write_SDH_CFG(bfin_read_SDH_CFG() | PWR_ON);
#endif
	return 0;
}


int bfin_mmc_init(bd_t *bis)
{
	struct mmc *mmc = NULL;

	mmc = malloc(sizeof(struct mmc));

	if (!mmc)
		return -ENOMEM;
	sprintf(mmc->name, "Blackfin SDH");
	mmc->send_cmd = bfin_sdh_request;
	mmc->set_ios = bfin_sdh_set_ios;
	mmc->init = bfin_sdh_init;
	mmc->getcd = NULL;
	mmc->getwp = NULL;
	mmc->host_caps = MMC_MODE_4BIT;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max = get_sclk();
	mmc->f_min = mmc->f_max >> 9;

	mmc->b_max = 0;

	mmc_register(mmc);

	return 0;
}
