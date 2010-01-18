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
#include <asm/mach-common/bits/sdh.h>
#include <asm/mach-common/bits/dma.h>

#if defined(__ADSPBF51x__)
# define bfin_read_SDH_PWR_CTL		bfin_read_RSI_PWR_CONTROL
# define bfin_write_SDH_PWR_CTL		bfin_write_RSI_PWR_CONTROL
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
# define bfin_write_DMA_START_ADDR	bfin_write_DMA4_START_ADDR
# define bfin_write_DMA_X_COUNT		bfin_write_DMA4_X_COUNT
# define bfin_write_DMA_X_MODIFY	bfin_write_DMA4_X_MODIFY
# define bfin_write_DMA_CONFIG		bfin_write_DMA4_CONFIG
#elif defined(__ADSPBF54x__)
# define bfin_write_DMA_START_ADDR	bfin_write_DMA22_START_ADDR
# define bfin_write_DMA_X_COUNT		bfin_write_DMA22_X_COUNT
# define bfin_write_DMA_X_MODIFY	bfin_write_DMA22_X_MODIFY
# define bfin_write_DMA_CONFIG		bfin_write_DMA22_CONFIG
#else
# error no support for this proc yet
#endif

static int
sdh_send_cmd(struct mmc *mmc, struct mmc_cmd *mmc_cmd)
{
	unsigned int sdh_cmd;
	unsigned int status;
	int cmd = mmc_cmd->cmdidx;
	int flags = mmc_cmd->resp_type;
	int arg = mmc_cmd->cmdarg;
	int ret = 0;
	sdh_cmd = 0;

	sdh_cmd |= cmd;

	if (flags & MMC_RSP_PRESENT)
		sdh_cmd |= CMD_RSP;

	if (flags & MMC_RSP_136)
		sdh_cmd |= CMD_L_RSP;

	bfin_write_SDH_ARGUMENT(arg);
	bfin_write_SDH_COMMAND(sdh_cmd | CMD_E);

	/* wait for a while */
	do {
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
		ret |= TIMEOUT;
	else if (status & CMD_CRC_FAIL && flags & MMC_RSP_CRC)
		ret |= COMM_ERR;

	bfin_write_SDH_STATUS_CLR(CMD_SENT_STAT | CMD_RESP_END_STAT |
				CMD_TIMEOUT_STAT | CMD_CRC_FAIL_STAT);
	return ret;
}

/* set data for single block transfer */
static int sdh_setup_data(struct mmc *mmc, struct mmc_data *data)
{
	u16 data_ctl = 0;
	u16 dma_cfg = 0;
	int ret = 0;

	/* Don't support write yet. */
	if (data->flags & MMC_DATA_WRITE)
		return UNUSABLE_ERR;
	data_ctl |= ((ffs(data->blocksize) - 1) << 4);
	data_ctl |= DTX_DIR;
	bfin_write_SDH_DATA_CTL(data_ctl);
	dma_cfg = WDSIZE_32 | RESTART | WNR | DMAEN;

	bfin_write_SDH_DATA_TIMER(0xFFFF);

	blackfin_dcache_flush_invalidate_range(data->dest,
			data->dest + data->blocksize);
	/* configure DMA */
	bfin_write_DMA_START_ADDR(data->dest);
	bfin_write_DMA_X_COUNT(data->blocksize / 4);
	bfin_write_DMA_X_MODIFY(4);
	bfin_write_DMA_CONFIG(dma_cfg);
	bfin_write_SDH_DATA_LGTH(data->blocksize);
	/* kick off transfer */
	bfin_write_SDH_DATA_CTL(bfin_read_SDH_DATA_CTL() | DTX_DMA_E | DTX_E);

	return ret;
}


static int bfin_sdh_request(struct mmc *mmc, struct mmc_cmd *cmd,
		struct mmc_data *data)
{
	u32 status;
	int ret = 0;

	ret = sdh_send_cmd(mmc, cmd);
	if (ret) {
		printf("sending CMD%d failed\n", cmd->cmdidx);
		return ret;
	}
	if (data) {
		ret = sdh_setup_data(mmc, data);
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
		cfg &= ~0x80;
		cfg |= 0x40;
		bfin_write_SDH_CFG(cfg);
		clk_ctl |= WIDE_BUS;
	}
	bfin_write_SDH_CLK_CTL(clk_ctl);
	sdh_set_clk(mmc->clock);
}

static int bfin_sdh_init(struct mmc *mmc)
{

	u16 pwr_ctl = 0;
/* Initialize sdh controller */
#if defined(__ADSPBF54x__)
	bfin_write_DMAC1_PERIMUX(bfin_read_DMAC1_PERIMUX() | 0x1);
	bfin_write_PORTC_FER(bfin_read_PORTC_FER() | 0x3F00);
	bfin_write_PORTC_MUX(bfin_read_PORTC_MUX() & ~0xFFF0000);
#elif defined(__ADSPBF51x__)
	bfin_write_PORTG_FER(bfin_read_PORTG_FER() | 0x01F8);
	bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() & ~0x3FC) | 0x154);
#else
# error no portmux for this proc yet
#endif
	bfin_write_SDH_CFG(bfin_read_SDH_CFG() | CLKS_EN);
	/* Disable card detect pin */
	bfin_write_SDH_CFG((bfin_read_SDH_CFG() & 0x1F) | 0x60);

	pwr_ctl |= ROD_CTL;
	pwr_ctl |= PWR_ON;
	bfin_write_SDH_PWR_CTL(pwr_ctl);
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
	mmc->host_caps = MMC_MODE_4BIT;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max = get_sclk();
	mmc->f_min = mmc->f_max >> 9;
	mmc->block_dev.part_type = PART_TYPE_DOS;

	mmc_register(mmc);

	return 0;
}
