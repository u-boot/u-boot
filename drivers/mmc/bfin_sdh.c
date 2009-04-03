/*
 * Driver for Blackfin on-chip SDH controller
 *
 * Copyright (c) 2008 Analog Devices Inc.
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

#include "bfin_sdh.h"

/* SD_CLK frequency must be less than 400k in identification mode */
#ifndef CONFIG_SYS_MMC_CLK_ID
#define CONFIG_SYS_MMC_CLK_ID		200000
#endif
/* SD_CLK for normal working */
#ifndef CONFIG_SYS_MMC_CLK_OP
#define CONFIG_SYS_MMC_CLK_OP		25000000
#endif
/* support 3.2-3.3V and 3.3-3.4V */
#define CONFIG_SYS_MMC_OP_COND		0x00300000
#define MMC_DEFAULT_RCA		1

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

static unsigned int mmc_rca;
static int mmc_card_is_sd;
static block_dev_desc_t mmc_blkdev;
struct mmc_cid cid;
static __u32 csd[4];

#define get_bits(resp, start, size)					\
	({								\
		const int __size = size;				\
		const uint32_t __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int32_t __off = 3 - ((start) / 32);			\
		const int32_t __shft = (start) & 31;			\
		uint32_t __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})


block_dev_desc_t *mmc_get_dev(int dev)
{
	return &mmc_blkdev;
}

static void mci_set_clk(unsigned long clk)
{
	unsigned long sys_clk;
	unsigned long clk_div;
	__u16 clk_ctl = 0;

	/* setting SD_CLK */
	sys_clk = get_sclk();
	bfin_write_SDH_CLK_CTL(0);
	if (sys_clk % (2 * clk) == 0)
		clk_div = sys_clk / (2 * clk) - 1;
	else
		clk_div = sys_clk / (2 * clk);

	if (clk_div > 0xff)
		clk_div = 0xff;
	clk_ctl |= (clk_div & 0xff);
	clk_ctl |= CLK_E;
	bfin_write_SDH_CLK_CTL(clk_ctl);
}

static int
mmc_cmd(unsigned long cmd, unsigned long arg, void *resp, unsigned long flags)
{
	unsigned int sdh_cmd;
	unsigned int status;
	int ret = 0;
	sdh_cmd = 0;
	unsigned long *response = resp;
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
		response[0] = bfin_read_SDH_RESPONSE0();
		if (flags & MMC_RSP_136) {
			response[1] = bfin_read_SDH_RESPONSE1();
			response[2] = bfin_read_SDH_RESPONSE2();
			response[3] = bfin_read_SDH_RESPONSE3();
		}
	}

	if (status & CMD_TIME_OUT) {
		printf("CMD%d timeout\n", (int)cmd);
		ret |= -ETIMEDOUT;
	} else if (status & CMD_CRC_FAIL && flags & MMC_RSP_CRC) {
		printf("CMD%d CRC failure\n", (int)cmd);
		ret |= -EILSEQ;
	}
	bfin_write_SDH_STATUS_CLR(CMD_SENT_STAT | CMD_RESP_END_STAT |
				CMD_TIMEOUT_STAT | CMD_CRC_FAIL_STAT);
	return ret;
}

static int
mmc_acmd(unsigned long cmd, unsigned long arg, void *resp, unsigned long flags)
{
	unsigned long aresp[4];
	int ret = 0;

	ret = mmc_cmd(MMC_CMD_APP_CMD, 0, aresp,
		      MMC_RSP_PRESENT);
	if (ret)
		return ret;

	if ((aresp[0] & (ILLEGAL_COMMAND | APP_CMD)) != APP_CMD)
		return -ENODEV;
	ret = mmc_cmd(cmd, arg, resp, flags);
	return ret;
}

static unsigned long
mmc_bread(int dev, unsigned long start, lbaint_t blkcnt, void *buffer)
{
	int ret, i;
	unsigned long resp[4];
	unsigned long card_status;
	__u8 *buf = buffer;
	__u32 status;
	__u16 data_ctl = 0;
	__u16 dma_cfg = 0;

	if (blkcnt == 0)
		return 0;
	debug("mmc_bread: dev %d, start %d, blkcnt %d\n", dev, start, blkcnt);
	/* Force to use 512-byte block,because a lot of code depends on this */
	data_ctl |= 9 << 4;
	data_ctl |= DTX_DIR;
	bfin_write_SDH_DATA_CTL(data_ctl);
	dma_cfg |= WDSIZE_32 | RESTART | WNR | DMAEN;

	/* FIXME later */
	bfin_write_SDH_DATA_TIMER(0xFFFFFFFF);
	for (i = 0; i < blkcnt; ++i, ++start) {
		blackfin_dcache_flush_invalidate_range(buf + i * mmc_blkdev.blksz,
			buf + (i + 1) * mmc_blkdev.blksz);
		bfin_write_DMA_START_ADDR(buf + i * mmc_blkdev.blksz);
		bfin_write_DMA_X_COUNT(mmc_blkdev.blksz / 4);
		bfin_write_DMA_X_MODIFY(4);
		bfin_write_DMA_CONFIG(dma_cfg);
		bfin_write_SDH_DATA_LGTH(mmc_blkdev.blksz);
		/* Put the device into Transfer state */
		ret = mmc_cmd(MMC_CMD_SELECT_CARD, mmc_rca << 16, resp, MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_SELECT_CARD failed\n");
			goto out;
		}
		/* Set block length */
		ret = mmc_cmd(MMC_CMD_SET_BLOCKLEN, mmc_blkdev.blksz, resp, MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_SET_BLOCKLEN failed\n");
			goto out;
		}
		ret = mmc_cmd(MMC_CMD_READ_SINGLE_BLOCK,
			      start * mmc_blkdev.blksz, resp,
			      MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_READ_SINGLE_BLOCK failed\n");
			goto out;
		}
		bfin_write_SDH_DATA_CTL(bfin_read_SDH_DATA_CTL() | DTX_DMA_E | DTX_E);

		do {
			udelay(1);
			status = bfin_read_SDH_STATUS();
		} while (!(status & (DAT_BLK_END | DAT_END | DAT_TIME_OUT | DAT_CRC_FAIL | RX_OVERRUN)));

		if (status & (DAT_TIME_OUT | DAT_CRC_FAIL | RX_OVERRUN)) {
			bfin_write_SDH_STATUS_CLR(DAT_TIMEOUT_STAT | \
				DAT_CRC_FAIL_STAT | RX_OVERRUN_STAT);
			goto read_error;
		} else {
			bfin_write_SDH_STATUS_CLR(DAT_BLK_END_STAT | DAT_END_STAT);
			mmc_cmd(MMC_CMD_SELECT_CARD, 0, resp, 0);
		}
	}
 out:

	return i;

 read_error:
	mmc_cmd(MMC_CMD_SEND_STATUS, mmc_rca << 16, &card_status, MMC_RSP_R1);
	printf("mmc: bread failed, status = %08x, card status = %08lx\n",
	       status, card_status);
	goto out;
}

static unsigned long
mmc_bwrite(int dev, unsigned long start, lbaint_t blkcnt, const void *buffer)
{
	int ret, i = 0;
	unsigned long resp[4];
	unsigned long card_status;
	const __u8 *buf = buffer;
	__u32 status;
	__u16 data_ctl = 0;
	__u16 dma_cfg = 0;

	if (blkcnt == 0)
		return 0;

	debug("mmc_bwrite: dev %d, start %lx, blkcnt %lx\n",
		 dev, start, blkcnt);
	/* Force to use 512-byte block,because a lot of code depends on this */
	data_ctl |= 9 << 4;
	data_ctl &= ~DTX_DIR;
	bfin_write_SDH_DATA_CTL(data_ctl);
	dma_cfg |= WDSIZE_32 | RESTART | DMAEN;
	/* FIXME later */
	bfin_write_SDH_DATA_TIMER(0xFFFFFFFF);
	for (i = 0; i < blkcnt; ++i, ++start) {
		bfin_write_DMA_START_ADDR(buf + i * mmc_blkdev.blksz);
		bfin_write_DMA_X_COUNT(mmc_blkdev.blksz / 4);
		bfin_write_DMA_X_MODIFY(4);
		bfin_write_DMA_CONFIG(dma_cfg);
		bfin_write_SDH_DATA_LGTH(mmc_blkdev.blksz);

		/* Put the device into Transfer state */
		ret = mmc_cmd(MMC_CMD_SELECT_CARD, mmc_rca << 16, resp, MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_SELECT_CARD failed\n");
			goto out;
		}
		/* Set block length */
		ret = mmc_cmd(MMC_CMD_SET_BLOCKLEN, mmc_blkdev.blksz, resp, MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_SET_BLOCKLEN failed\n");
			goto out;
		}
		ret = mmc_cmd(MMC_CMD_WRITE_SINGLE_BLOCK,
			      start * mmc_blkdev.blksz, resp,
			      MMC_RSP_R1);
		if (ret) {
			printf("MMC_CMD_WRITE_SINGLE_BLOCK failed\n");
			goto out;
		}
		bfin_write_SDH_DATA_CTL(bfin_read_SDH_DATA_CTL() | DTX_DMA_E | DTX_E);

		do {
			udelay(1);
			status = bfin_read_SDH_STATUS();
		} while (!(status & (DAT_BLK_END | DAT_END | DAT_TIME_OUT | DAT_CRC_FAIL | TX_UNDERRUN)));

		if (status & (DAT_TIME_OUT | DAT_CRC_FAIL | TX_UNDERRUN)) {
			bfin_write_SDH_STATUS_CLR(DAT_TIMEOUT_STAT |
				DAT_CRC_FAIL_STAT | TX_UNDERRUN_STAT);
			goto write_error;
		} else {
			bfin_write_SDH_STATUS_CLR(DAT_BLK_END_STAT | DAT_END_STAT);
			mmc_cmd(MMC_CMD_SELECT_CARD, 0, resp, 0);
		}
	}
 out:
	return i;

 write_error:
	mmc_cmd(MMC_CMD_SEND_STATUS, mmc_rca << 16, &card_status, MMC_RSP_R1);
	printf("mmc: bwrite failed, status = %08x, card status = %08lx\n",
	       status, card_status);
	goto out;
}

static void mmc_parse_cid(struct mmc_cid *cid, unsigned long *resp)
{
	cid->mid = resp[0] >> 24;
	cid->oid = (resp[0] >> 8) & 0xffff;
	cid->pnm[0] = resp[0];
	cid->pnm[1] = resp[1] >> 24;
	cid->pnm[2] = resp[1] >> 16;
	cid->pnm[3] = resp[1] >> 8;
	cid->pnm[4] = resp[1];
	cid->pnm[5] = resp[2] >> 24;
	cid->pnm[6] = 0;
	cid->prv = resp[2] >> 16;
	cid->psn = (resp[2] << 16) | (resp[3] >> 16);
	cid->mdt = resp[3] >> 8;
}

static void sd_parse_cid(struct mmc_cid *cid, unsigned long *resp)
{
	cid->mid = resp[0] >> 24;
	cid->oid = (resp[0] >> 8) & 0xffff;
	cid->pnm[0] = resp[0];
	cid->pnm[1] = resp[1] >> 24;
	cid->pnm[2] = resp[1] >> 16;
	cid->pnm[3] = resp[1] >> 8;
	cid->pnm[4] = resp[1];
	cid->pnm[5] = 0;
	cid->pnm[6] = 0;
	cid->prv = resp[2] >> 24;
	cid->psn = (resp[2] << 8) | (resp[3] >> 24);
	cid->mdt = (resp[3] >> 8) & 0x0fff;
}

static void mmc_dump_cid(const struct mmc_cid *cid)
{
	printf("CID information:\n");
	printf("Manufacturer ID:       %02X\n", cid->mid);
	printf("OEM/Application ID:    %04X\n", cid->oid);
	printf("Product name:          %s\n", cid->pnm);
	printf("Product Revision:      %u.%u\n",
	       cid->prv >> 4, cid->prv & 0x0f);
	printf("Product Serial Number: %lu\n", cid->psn);
	printf("Manufacturing Date:    %02u/%02u\n",
	       cid->mdt >> 4, cid->mdt & 0x0f);
}

static void mmc_dump_csd(__u32 *csd)
{
	printf("CSD information:\n");
	printf("CSD structure version:   1.%u\n", get_bits(csd, 126, 2));
	printf("Card command classes:    %03x\n", get_bits(csd, 84, 12));
	printf("Max trans speed: %s\n", (get_bits(csd, 96, 8) == 0x32) ? "25MHz" : "50MHz");
	printf("Read block length:       %d\n", 1 << get_bits(csd, 80, 4));
	printf("Write block length:      %u\n", 1 << get_bits(csd, 22, 4));
	printf("Card capacity:		%u bytes\n",
	       (get_bits(csd, 62, 12) + 1) * (1 << (get_bits(csd, 47, 3) + 2)) *
	       (1 << get_bits(csd, 80, 4)));
	putc('\n');
}

static int mmc_idle_cards(void)
{
	int ret = 0;

	/* Reset all cards */
	ret = mmc_cmd(MMC_CMD_GO_IDLE_STATE, 0, NULL, 0);
	if (ret)
		return ret;
	udelay(500);
	return mmc_cmd(MMC_CMD_GO_IDLE_STATE, 0, NULL, 0);
}

static int sd_init_card(struct mmc_cid *cid, int verbose)
{
	unsigned long resp[4];
	int i, ret = 0;

	mmc_idle_cards();
	for (i = 0; i < 1000; ++i) {
		ret = mmc_acmd(SD_CMD_APP_SEND_OP_COND, CONFIG_SYS_MMC_OP_COND,
			       resp, MMC_RSP_R3);
		if (ret || (resp[0] & 0x80000000))
			break;
		ret = -ETIMEDOUT;
	}
	if (ret)
		return ret;

	ret = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, resp, MMC_RSP_R2);
	if (ret)
		return ret;
	sd_parse_cid(cid, resp);
	if (verbose)
		mmc_dump_cid(cid);

	/* Get RCA of the card that responded */
	ret = mmc_cmd(SD_CMD_SEND_RELATIVE_ADDR, 0, resp, MMC_RSP_R6);
	if (ret)
		return ret;

	mmc_rca = (resp[0] >> 16) & 0xffff;
	if (verbose)
		printf("SD Card detected (RCA %u)\n", mmc_rca);
	mmc_card_is_sd = 1;
	return 0;
}

static int mmc_init_card(struct mmc_cid *cid, int verbose)
{
	unsigned long resp[4];
	int i, ret = 0;

	mmc_idle_cards();
	for (i = 0; i < 1000; ++i) {
		ret = mmc_cmd(MMC_CMD_SEND_OP_COND, CONFIG_SYS_MMC_OP_COND, resp,
			      MMC_RSP_R3);
		if (ret || (resp[0] & 0x80000000))
			break;
		ret = -ETIMEDOUT;
	}
	if (ret)
		return ret;

	/* Get CID of all cards. FIXME: Support more than one card */
	ret = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, resp, MMC_RSP_R2);
	if (ret)
		return ret;
	mmc_parse_cid(cid, resp);
	if (verbose)
		mmc_dump_cid(cid);

	/* Set Relative Address of the card that responded */
	ret = mmc_cmd(MMC_CMD_SET_RELATIVE_ADDR, mmc_rca << 16, resp,
		      MMC_RSP_R1);
	return ret;
}

int mmc_legacy_init(int verbose)
{
	__u16 pwr_ctl = 0;
	int ret;
	unsigned int max_blksz;
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
	mci_set_clk(CONFIG_SYS_MMC_CLK_ID);
	/* setting power control */
	pwr_ctl |= ROD_CTL;
	pwr_ctl |= PWR_ON;
	bfin_write_SDH_PWR_CTL(pwr_ctl);
	mmc_card_is_sd = 0;
	ret = sd_init_card(&cid, verbose);
	if (ret) {
		mmc_rca = MMC_DEFAULT_RCA;
		ret = mmc_init_card(&cid, verbose);
	}
	if (ret)
		return ret;
	/* Get CSD from the card */
	ret = mmc_cmd(MMC_CMD_SEND_CSD, mmc_rca << 16, csd, MMC_RSP_R2);
	if (ret)
		return ret;
	if (verbose)
		mmc_dump_csd(csd);
	/* Initialize the blockdev structure */
	mmc_blkdev.if_type = IF_TYPE_MMC;
	mmc_blkdev.part_type = PART_TYPE_DOS;
	mmc_blkdev.block_read = mmc_bread;
	mmc_blkdev.block_write = mmc_bwrite;
	sprintf(mmc_blkdev.vendor,
		"Man %02x%04x Snr %08lx",
		cid.mid, cid.oid, cid.psn);
	strncpy(mmc_blkdev.product, cid.pnm,
		sizeof(mmc_blkdev.product));
	sprintf(mmc_blkdev.revision, "%x %x",
		cid.prv >> 4, cid.prv & 0x0f);

	max_blksz = 1 << get_bits(csd, 80, 4);
	/*
	 * If we can't use 512 byte blocks, refuse to deal with the
	 * card. Tons of code elsewhere seems to depend on this.
	 */
	if (max_blksz < 512 || (max_blksz > 512 && !get_bits(csd, 79, 1))) {
		printf("Card does not support 512 byte reads, aborting.\n");
		return -ENODEV;
	}

	mmc_blkdev.blksz = 512;
	mmc_blkdev.lba = (get_bits(csd, 62, 12) + 1) * (1 << (get_bits(csd, 47, 3) + 2));
	mci_set_clk(CONFIG_SYS_MMC_CLK_OP);
	init_part(&mmc_blkdev);
	return 0;
}

int mmc2info(ulong addr)
{
	return 0;
}
