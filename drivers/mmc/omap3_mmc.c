/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <fat.h>
#include <mmc.h>
#include <part.h>
#include <i2c.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc.h>

const unsigned short mmc_transspeed_val[15][4] = {
	{CLKD(10, 1), CLKD(10, 10), CLKD(10, 100), CLKD(10, 1000)},
	{CLKD(12, 1), CLKD(12, 10), CLKD(12, 100), CLKD(12, 1000)},
	{CLKD(13, 1), CLKD(13, 10), CLKD(13, 100), CLKD(13, 1000)},
	{CLKD(15, 1), CLKD(15, 10), CLKD(15, 100), CLKD(15, 1000)},
	{CLKD(20, 1), CLKD(20, 10), CLKD(20, 100), CLKD(20, 1000)},
	{CLKD(26, 1), CLKD(26, 10), CLKD(26, 100), CLKD(26, 1000)},
	{CLKD(30, 1), CLKD(30, 10), CLKD(30, 100), CLKD(30, 1000)},
	{CLKD(35, 1), CLKD(35, 10), CLKD(35, 100), CLKD(35, 1000)},
	{CLKD(40, 1), CLKD(40, 10), CLKD(40, 100), CLKD(40, 1000)},
	{CLKD(45, 1), CLKD(45, 10), CLKD(45, 100), CLKD(45, 1000)},
	{CLKD(52, 1), CLKD(52, 10), CLKD(52, 100), CLKD(52, 1000)},
	{CLKD(55, 1), CLKD(55, 10), CLKD(55, 100), CLKD(55, 1000)},
	{CLKD(60, 1), CLKD(60, 10), CLKD(60, 100), CLKD(60, 1000)},
	{CLKD(70, 1), CLKD(70, 10), CLKD(70, 100), CLKD(70, 1000)},
	{CLKD(80, 1), CLKD(80, 10), CLKD(80, 100), CLKD(80, 1000)}
};

mmc_card_data cur_card_data;
static block_dev_desc_t mmc_blk_dev;
static hsmmc_t *mmc_base = (hsmmc_t *)OMAP_HSMMC_BASE;

block_dev_desc_t *mmc_get_dev(int dev)
{
	return (block_dev_desc_t *) &mmc_blk_dev;
}

unsigned char mmc_board_init(void)
{
	t2_t *t2_base = (t2_t *)T2_BASE;

#if defined(CONFIG_TWL4030_POWER)
	twl4030_power_mmc_init();
#endif

	writel(readl(&t2_base->pbias_lite) | PBIASLITEPWRDNZ1 |
		PBIASSPEEDCTRL0 | PBIASLITEPWRDNZ0,
		&t2_base->pbias_lite);

	writel(readl(&t2_base->devconf0) | MMCSDIO1ADPCLKISEL,
		&t2_base->devconf0);

	return 1;
}

void mmc_init_stream(void)
{
	writel(readl(&mmc_base->con) | INIT_INITSTREAM, &mmc_base->con);

	writel(MMC_CMD0, &mmc_base->cmd);
	while (!(readl(&mmc_base->stat) & CC_MASK));

	writel(CC_MASK, &mmc_base->stat);

	writel(MMC_CMD0, &mmc_base->cmd);
	while (!(readl(&mmc_base->stat) & CC_MASK));

	writel(readl(&mmc_base->con) & ~INIT_INITSTREAM, &mmc_base->con);
}

unsigned char mmc_clock_config(unsigned int iclk, unsigned short clk_div)
{
	unsigned int val;

	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK | CEN_MASK),
			(ICE_STOP | DTO_15THDTO | CEN_DISABLE));

	switch (iclk) {
	case CLK_INITSEQ:
		val = MMC_INIT_SEQ_CLK / 2;
		break;
	case CLK_400KHZ:
		val = MMC_400kHz_CLK;
		break;
	case CLK_MISC:
		val = clk_div;
		break;
	default:
		return 0;
	}
	mmc_reg_out(&mmc_base->sysctl, ICE_MASK | CLKD_MASK,
			(val << CLKD_OFFSET) | ICE_OSCILLATE);

	while ((readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY);

	writel(readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);
	return 1;
}

unsigned char mmc_init_setup(void)
{
	unsigned int reg_val;

	mmc_board_init();

	writel(readl(&mmc_base->sysconfig) | MMC_SOFTRESET,
		&mmc_base->sysconfig);
	while ((readl(&mmc_base->sysstatus) & RESETDONE) == 0);

	writel(readl(&mmc_base->sysctl) | SOFTRESETALL, &mmc_base->sysctl);
	while ((readl(&mmc_base->sysctl) & SOFTRESETALL) != 0x0);

	writel(DTW_1_BITMODE | SDBP_PWROFF | SDVS_3V0, &mmc_base->hctl);
	writel(readl(&mmc_base->capa) | VS30_3V0SUP | VS18_1V8SUP,
		&mmc_base->capa);

	reg_val = readl(&mmc_base->con) & RESERVED_MASK;

	writel(CTPL_MMC_SD | reg_val | WPP_ACTIVEHIGH | CDP_ACTIVEHIGH |
		MIT_CTO | DW8_1_4BITMODE | MODE_FUNC | STR_BLOCK |
		HR_NOHOSTRESP | INIT_NOINIT | NOOPENDRAIN, &mmc_base->con);

	mmc_clock_config(CLK_INITSEQ, 0);
	writel(readl(&mmc_base->hctl) | SDBP_PWRON, &mmc_base->hctl);

	writel(IE_BADA | IE_CERR | IE_DEB | IE_DCRC | IE_DTO | IE_CIE |
		IE_CEB | IE_CCRC | IE_CTO | IE_BRR | IE_BWR | IE_TC | IE_CC,
		&mmc_base->ie);

	mmc_init_stream();
	return 1;
}

unsigned char mmc_send_cmd(unsigned int cmd, unsigned int arg,
				unsigned int *response)
{
	unsigned int mmc_stat;

	while ((readl(&mmc_base->pstate) & DATI_MASK) == DATI_CMDDIS);

	writel(BLEN_512BYTESLEN | NBLK_STPCNT, &mmc_base->blk);
	writel(0xFFFFFFFF, &mmc_base->stat);
	writel(arg, &mmc_base->arg);
	writel(cmd | CMD_TYPE_NORMAL | CICE_NOCHECK | CCCE_NOCHECK |
		MSBS_SGLEBLK | ACEN_DISABLE | BCE_DISABLE | DE_DISABLE,
		&mmc_base->cmd);

	while (1) {
		do {
			mmc_stat = readl(&mmc_base->stat);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return (unsigned char) mmc_stat;

		if (mmc_stat & CC_MASK) {
			writel(CC_MASK, &mmc_base->stat);
			response[0] = readl(&mmc_base->rsp10);
			if ((cmd & RSP_TYPE_MASK) == RSP_TYPE_LGHT136) {
				response[1] = readl(&mmc_base->rsp32);
				response[2] = readl(&mmc_base->rsp54);
				response[3] = readl(&mmc_base->rsp76);
			}
			break;
		}
	}
	return 1;
}

unsigned char mmc_read_data(unsigned int *output_buf)
{
	unsigned int mmc_stat;
	unsigned int read_count = 0;

	/*
	 * Start Polled Read
	 */
	while (1) {
		do {
			mmc_stat = readl(&mmc_base->stat);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return (unsigned char) mmc_stat;

		if (mmc_stat & BRR_MASK) {
			unsigned int k;

			writel(readl(&mmc_base->stat) | BRR_MASK,
				&mmc_base->stat);
			for (k = 0; k < MMCSD_SECTOR_SIZE / 4; k++) {
				*output_buf = readl(&mmc_base->data);
				output_buf++;
				read_count += 4;
			}
		}

		if (mmc_stat & BWR_MASK)
			writel(readl(&mmc_base->stat) | BWR_MASK,
				&mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			writel(readl(&mmc_base->stat) | TC_MASK,
				&mmc_base->stat);
			break;
		}
	}
	return 1;
}

unsigned char mmc_detect_card(mmc_card_data *mmc_card_cur)
{
	unsigned char err;
	unsigned int argument = 0;
	unsigned int ocr_value, ocr_recvd, ret_cmd41, hcs_val;
	unsigned short retry_cnt = 2000;
	mmc_resp_t mmc_resp;

	/* Set to Initialization Clock */
	err = mmc_clock_config(CLK_400KHZ, 0);
	if (err != 1)
		return err;

	mmc_card_cur->RCA = MMC_RELATIVE_CARD_ADDRESS;
	argument = 0x00000000;

	ocr_value = (0x1FF << 15);
	err = mmc_send_cmd(MMC_CMD0, argument, mmc_resp.resp);
	if (err != 1)
		return err;

	argument = SD_CMD8_CHECK_PATTERN | SD_CMD8_2_7_3_6_V_RANGE;
	err = mmc_send_cmd(MMC_SDCMD8, argument, mmc_resp.resp);
	hcs_val = (err == 1) ?
		MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR :
		MMC_OCR_REG_HOST_CAPACITY_SUPPORT_BYTE;

	argument = 0x0000 << 16;
	err = mmc_send_cmd(MMC_CMD55, argument, mmc_resp.resp);
	if (err == 1) {
		mmc_card_cur->card_type = SD_CARD;
		ocr_value |= hcs_val;
		ret_cmd41 = MMC_ACMD41;
	} else {
		mmc_card_cur->card_type = MMC_CARD;
		ocr_value |= MMC_OCR_REG_ACCESS_MODE_SECTOR;
		ret_cmd41 = MMC_CMD1;
		writel(readl(&mmc_base->con) & ~OD, &mmc_base->con);
		writel(readl(&mmc_base->con) | OPENDRAIN, &mmc_base->con);
	}

	argument = ocr_value;
	err = mmc_send_cmd(ret_cmd41, argument, mmc_resp.resp);
	if (err != 1)
		return err;

	ocr_recvd = mmc_resp.r3.ocr;

	while (!(ocr_recvd & (0x1 << 31)) && (retry_cnt > 0)) {
		retry_cnt--;
		if (mmc_card_cur->card_type == SD_CARD) {
			argument = 0x0000 << 16;
			err = mmc_send_cmd(MMC_CMD55, argument, mmc_resp.resp);
		}

		argument = ocr_value;
		err = mmc_send_cmd(ret_cmd41, argument, mmc_resp.resp);
		if (err != 1)
			return err;
		ocr_recvd = mmc_resp.r3.ocr;
	}

	if (!(ocr_recvd & (0x1 << 31)))
		return 0;

	if (mmc_card_cur->card_type == MMC_CARD) {
		if ((ocr_recvd & MMC_OCR_REG_ACCESS_MODE_MASK) ==
			MMC_OCR_REG_ACCESS_MODE_SECTOR) {
			mmc_card_cur->mode = SECTOR_MODE;
		} else {
			mmc_card_cur->mode = BYTE_MODE;
		}

		ocr_recvd &= ~MMC_OCR_REG_ACCESS_MODE_MASK;
	} else {
		if ((ocr_recvd & MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK)
			== MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR) {
			mmc_card_cur->mode = SECTOR_MODE;
		} else {
			mmc_card_cur->mode = BYTE_MODE;
		}
		ocr_recvd &= ~MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK;
	}

	ocr_recvd &= ~(0x1 << 31);
	if (!(ocr_recvd & ocr_value))
		return 0;

	err = mmc_send_cmd(MMC_CMD2, argument, mmc_resp.resp);
	if (err != 1)
		return err;

	if (mmc_card_cur->card_type == MMC_CARD) {
		argument = mmc_card_cur->RCA << 16;
		err = mmc_send_cmd(MMC_CMD3, argument, mmc_resp.resp);
		if (err != 1)
			return err;
	} else {
		argument = 0x00000000;
		err = mmc_send_cmd(MMC_SDCMD3, argument, mmc_resp.resp);
		if (err != 1)
			return err;

		mmc_card_cur->RCA = mmc_resp.r6.newpublishedrca;
	}

	writel(readl(&mmc_base->con) & ~OD, &mmc_base->con);
	writel(readl(&mmc_base->con) | NOOPENDRAIN, &mmc_base->con);
	return 1;
}

unsigned char mmc_read_cardsize(mmc_card_data *mmc_dev_data,
				mmc_csd_reg_t *cur_csd)
{
	mmc_extended_csd_reg_t ext_csd;
	unsigned int size, count, blk_len, blk_no, card_size, argument;
	unsigned char err;
	unsigned int resp[4];

	if (mmc_dev_data->mode == SECTOR_MODE) {
		if (mmc_dev_data->card_type == SD_CARD) {
			card_size =
				(((mmc_sd2_csd_reg_t *) cur_csd)->
				c_size_lsb & MMC_SD2_CSD_C_SIZE_LSB_MASK) |
				((((mmc_sd2_csd_reg_t *) cur_csd)->
				c_size_msb & MMC_SD2_CSD_C_SIZE_MSB_MASK)
				<< MMC_SD2_CSD_C_SIZE_MSB_OFFSET);
			mmc_dev_data->size = card_size * 1024;
			if (mmc_dev_data->size == 0)
				return 0;
		} else {
			argument = 0x00000000;
			err = mmc_send_cmd(MMC_CMD8, argument, resp);
			if (err != 1)
				return err;
			err = mmc_read_data((unsigned int *) &ext_csd);
			if (err != 1)
				return err;
			mmc_dev_data->size = ext_csd.sectorcount;

			if (mmc_dev_data->size == 0)
				mmc_dev_data->size = 8388608;
		}
	} else {
		if (cur_csd->c_size_mult >= 8)
			return 0;

		if (cur_csd->read_bl_len >= 12)
			return 0;

		/* Compute size */
		count = 1 << (cur_csd->c_size_mult + 2);
		card_size = (cur_csd->c_size_lsb & MMC_CSD_C_SIZE_LSB_MASK) |
			((cur_csd->c_size_msb & MMC_CSD_C_SIZE_MSB_MASK)
			<< MMC_CSD_C_SIZE_MSB_OFFSET);
		blk_no = (card_size + 1) * count;
		blk_len = 1 << cur_csd->read_bl_len;
		size = blk_no * blk_len;
		mmc_dev_data->size = size / MMCSD_SECTOR_SIZE;
		if (mmc_dev_data->size == 0)
			return 0;
	}
	return 1;
}

unsigned char omap_mmc_read_sect(unsigned int start_sec, unsigned int num_bytes,
				 mmc_card_data *mmc_c,
				 unsigned long *output_buf)
{
	unsigned char err;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int num_sec_val =
		(num_bytes + (MMCSD_SECTOR_SIZE - 1)) / MMCSD_SECTOR_SIZE;
	unsigned int sec_inc_val;

	if (num_sec_val == 0)
		return 1;

	if (mmc_c->mode == SECTOR_MODE) {
		argument = start_sec;
		sec_inc_val = 1;
	} else {
		argument = start_sec * MMCSD_SECTOR_SIZE;
		sec_inc_val = MMCSD_SECTOR_SIZE;
	}

	while (num_sec_val) {
		err = mmc_send_cmd(MMC_CMD17, argument, resp);
		if (err != 1)
			return err;

		err = mmc_read_data((unsigned int *) output_buf);
		if (err != 1)
			return err;

		output_buf += (MMCSD_SECTOR_SIZE / 4);
		argument += sec_inc_val;
		num_sec_val--;
	}
	return 1;
}

unsigned char configure_mmc(mmc_card_data *mmc_card_cur)
{
	unsigned char ret_val;
	unsigned int argument;
	unsigned int trans_clk, trans_fact, trans_unit, retries = 2;
	unsigned char trans_speed;
	mmc_resp_t mmc_resp;

	ret_val = mmc_init_setup();

	if (ret_val != 1)
		return ret_val;

	do {
		ret_val = mmc_detect_card(mmc_card_cur);
		retries--;
	} while ((retries > 0) && (ret_val != 1));

	argument = mmc_card_cur->RCA << 16;
	ret_val = mmc_send_cmd(MMC_CMD9, argument, mmc_resp.resp);
	if (ret_val != 1)
		return ret_val;

	if (mmc_card_cur->card_type == MMC_CARD)
		mmc_card_cur->version = mmc_resp.Card_CSD.spec_vers;

	trans_speed = mmc_resp.Card_CSD.tran_speed;

	ret_val = mmc_send_cmd(MMC_CMD4, MMC_DSR_DEFAULT << 16, mmc_resp.resp);
	if (ret_val != 1)
		return ret_val;

	trans_unit = trans_speed & MMC_CSD_TRAN_SPEED_UNIT_MASK;
	trans_fact = trans_speed & MMC_CSD_TRAN_SPEED_FACTOR_MASK;

	if (trans_unit > MMC_CSD_TRAN_SPEED_UNIT_100MHZ)
		return 0;

	if ((trans_fact < MMC_CSD_TRAN_SPEED_FACTOR_1_0) ||
		(trans_fact > MMC_CSD_TRAN_SPEED_FACTOR_8_0))
		return 0;

	trans_unit >>= 0;
	trans_fact >>= 3;

	trans_clk = mmc_transspeed_val[trans_fact - 1][trans_unit] * 2;
	ret_val = mmc_clock_config(CLK_MISC, trans_clk);

	if (ret_val != 1)
		return ret_val;

	argument = mmc_card_cur->RCA << 16;
	ret_val = mmc_send_cmd(MMC_CMD7_SELECT, argument, mmc_resp.resp);
	if (ret_val != 1)
		return ret_val;

	/* Configure the block length to 512 bytes */
	argument = MMCSD_SECTOR_SIZE;
	ret_val = mmc_send_cmd(MMC_CMD16, argument, mmc_resp.resp);
	if (ret_val != 1)
		return ret_val;

	/* get the card size in sectors */
	ret_val = mmc_read_cardsize(mmc_card_cur, &mmc_resp.Card_CSD);
	if (ret_val != 1)
		return ret_val;

	return 1;
}
unsigned long mmc_bread(int dev_num, unsigned long blknr, lbaint_t blkcnt,
			void *dst)
{
	omap_mmc_read_sect(blknr, (blkcnt * MMCSD_SECTOR_SIZE), &cur_card_data,
				(unsigned long *) dst);
	return 1;
}

int mmc_legacy_init(int verbose)
{
	if (configure_mmc(&cur_card_data) != 1)
		return 1;

	mmc_blk_dev.if_type = IF_TYPE_MMC;
	mmc_blk_dev.part_type = PART_TYPE_DOS;
	mmc_blk_dev.dev = 0;
	mmc_blk_dev.lun = 0;
	mmc_blk_dev.type = 0;

	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_blk_dev.blksz = MMCSD_SECTOR_SIZE;
	mmc_blk_dev.lba = 0x10000;
	mmc_blk_dev.removable = 0;
	mmc_blk_dev.block_read = mmc_bread;

	fat_register_device(&mmc_blk_dev, 1);
	return 0;
}
