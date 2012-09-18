/*
 * Copyright 2004-2007 Freescale Semiconductor, Inc.
 * Copyright 2008 Sascha Hauer, kernel@pengutronix.de
 * Copyright 2009 Ilya Yanok, <yanok@emcraft.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <common.h>
#include <nand.h>
#include <linux/err.h>
#include <asm/io.h>
#if defined(CONFIG_MX25) || defined(CONFIG_MX27) || defined(CONFIG_MX35)
#include <asm/arch/imx-regs.h>
#endif
#include <fsl_nfc.h>

#define DRIVER_NAME "mxc_nand"

typedef enum {false, true} bool;

struct mxc_nand_host {
	struct mtd_info			mtd;
	struct nand_chip		*nand;

	struct fsl_nfc_regs __iomem	*regs;
	int				spare_only;
	int				status_request;
	int				pagesize_2k;
	int				clk_act;
	uint16_t			col_addr;
	unsigned int			page_addr;
};

static struct mxc_nand_host mxc_host;
static struct mxc_nand_host *host = &mxc_host;

/* Define delays in microsec for NAND device operations */
#define TROP_US_DELAY   2000
/* Macros to get byte and bit positions of ECC */
#define COLPOS(x)  ((x) >> 3)
#define BITPOS(x) ((x) & 0xf)

/* Define single bit Error positions in Main & Spare area */
#define MAIN_SINGLEBIT_ERROR 0x4
#define SPARE_SINGLEBIT_ERROR 0x1

/* OOB placement block for use with hardware ecc generation */
#if defined(MXC_NFC_V1)
#ifndef CONFIG_SYS_NAND_LARGEPAGE
static struct nand_ecclayout nand_hw_eccoob = {
	.eccbytes = 5,
	.eccpos = {6, 7, 8, 9, 10},
	.oobfree = { {0, 5}, {11, 5}, }
};
#else
static struct nand_ecclayout nand_hw_eccoob2k = {
	.eccbytes = 20,
	.eccpos = {
		6, 7, 8, 9, 10,
		22, 23, 24, 25, 26,
		38, 39, 40, 41, 42,
		54, 55, 56, 57, 58,
	},
	.oobfree = { {2, 4}, {11, 11}, {27, 11}, {43, 11}, {59, 5} },
};
#endif
#elif defined(MXC_NFC_V2_1)
#ifndef CONFIG_SYS_NAND_LARGEPAGE
static struct nand_ecclayout nand_hw_eccoob = {
	.eccbytes = 9,
	.eccpos = {7, 8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = { {2, 5} }
};
#else
static struct nand_ecclayout nand_hw_eccoob2k = {
	.eccbytes = 36,
	.eccpos = {
		7, 8, 9, 10, 11, 12, 13, 14, 15,
		23, 24, 25, 26, 27, 28, 29, 30, 31,
		39, 40, 41, 42, 43, 44, 45, 46, 47,
		55, 56, 57, 58, 59, 60, 61, 62, 63,
	},
	.oobfree = { {2, 5}, {16, 7}, {32, 7}, {48, 7} },
};
#endif
#endif

#ifdef CONFIG_MX27
static int is_16bit_nand(void)
{
	struct system_control_regs *sc_regs =
		(struct system_control_regs *)IMX_SYSTEM_CTL_BASE;

	if (readl(&sc_regs->fmcr) & NF_16BIT_SEL)
		return 1;
	else
		return 0;
}
#elif defined(CONFIG_MX31)
static int is_16bit_nand(void)
{
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;

	if (readl(&sc_regs->rcsr) & CCM_RCSR_NF16B)
		return 1;
	else
		return 0;
}
#elif defined(CONFIG_MX25) || defined(CONFIG_MX35)
static int is_16bit_nand(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;

	if (readl(&ccm->rcsr) & CCM_RCSR_NF_16BIT_SEL)
		return 1;
	else
		return 0;
}
#else
#warning "8/16 bit NAND autodetection not supported"
static int is_16bit_nand(void)
{
	return 0;
}
#endif

static uint32_t *mxc_nand_memcpy32(uint32_t *dest, uint32_t *source, size_t size)
{
	uint32_t *d = dest;

	size >>= 2;
	while (size--)
		__raw_writel(__raw_readl(source++), d++);
	return dest;
}

/*
 * This function polls the NANDFC to wait for the basic operation to
 * complete by checking the INT bit of config2 register.
 */
static void wait_op_done(struct mxc_nand_host *host, int max_retries,
				uint16_t param)
{
	uint32_t tmp;

	while (max_retries-- > 0) {
		if (readw(&host->regs->config2) & NFC_INT) {
			tmp = readw(&host->regs->config2);
			tmp  &= ~NFC_INT;
			writew(tmp, &host->regs->config2);
			break;
		}
		udelay(1);
	}
	if (max_retries < 0) {
		MTDDEBUG(MTD_DEBUG_LEVEL0, "%s(%d): INT not set\n",
				__func__, param);
	}
}

/*
 * This function issues the specified command to the NAND device and
 * waits for completion.
 */
static void send_cmd(struct mxc_nand_host *host, uint16_t cmd)
{
	MTDDEBUG(MTD_DEBUG_LEVEL3, "send_cmd(host, 0x%x)\n", cmd);

	writew(cmd, &host->regs->flash_cmd);
	writew(NFC_CMD, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, cmd);
}

/*
 * This function sends an address (or partial address) to the
 * NAND device. The address is used to select the source/destination for
 * a NAND command.
 */
static void send_addr(struct mxc_nand_host *host, uint16_t addr)
{
	MTDDEBUG(MTD_DEBUG_LEVEL3, "send_addr(host, 0x%x)\n", addr);

	writew(addr, &host->regs->flash_addr);
	writew(NFC_ADDR, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, addr);
}

/*
 * This function requests the NANDFC to initiate the transfer
 * of data currently in the NANDFC RAM buffer to the NAND device.
 */
static void send_prog_page(struct mxc_nand_host *host, uint8_t buf_id,
			int spare_only)
{
	if (spare_only)
		MTDDEBUG(MTD_DEBUG_LEVEL1, "send_prog_page (%d)\n", spare_only);

	if (is_mxc_nfc_21()) {
		int i;
		/*
		 *  The controller copies the 64 bytes of spare data from
		 *  the first 16 bytes of each of the 4 64 byte spare buffers.
		 *  Copy the contiguous data starting in spare_area[0] to
		 *  the four spare area buffers.
		 */
		for (i = 1; i < 4; i++) {
			void __iomem *src = &host->regs->spare_area[0][i * 16];
			void __iomem *dst = &host->regs->spare_area[i][0];

			mxc_nand_memcpy32(dst, src, 16);
		}
	}

	writew(buf_id, &host->regs->buf_addr);

	/* Configure spare or page+spare access */
	if (!host->pagesize_2k) {
		uint16_t config1 = readw(&host->regs->config1);
		if (spare_only)
			config1 |= NFC_SP_EN;
		else
			config1 &= ~NFC_SP_EN;
		writew(config1, &host->regs->config1);
	}

	writew(NFC_INPUT, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, spare_only);
}

/*
 * Requests NANDFC to initiate the transfer of data from the
 * NAND device into in the NANDFC ram buffer.
 */
static void send_read_page(struct mxc_nand_host *host, uint8_t buf_id,
		int spare_only)
{
	MTDDEBUG(MTD_DEBUG_LEVEL3, "send_read_page (%d)\n", spare_only);

	writew(buf_id, &host->regs->buf_addr);

	/* Configure spare or page+spare access */
	if (!host->pagesize_2k) {
		uint32_t config1 = readw(&host->regs->config1);
		if (spare_only)
			config1 |= NFC_SP_EN;
		else
			config1 &= ~NFC_SP_EN;
		writew(config1, &host->regs->config1);
	}

	writew(NFC_OUTPUT, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, spare_only);

	if (is_mxc_nfc_21()) {
		int i;

		/*
		 *  The controller copies the 64 bytes of spare data to
		 *  the first 16 bytes of each of the 4 spare buffers.
		 *  Make the data contiguous starting in spare_area[0].
		 */
		for (i = 1; i < 4; i++) {
			void __iomem *src = &host->regs->spare_area[i][0];
			void __iomem *dst = &host->regs->spare_area[0][i * 16];

			mxc_nand_memcpy32(dst, src, 16);
		}
	}
}

/* Request the NANDFC to perform a read of the NAND device ID. */
static void send_read_id(struct mxc_nand_host *host)
{
	uint16_t tmp;

	/* NANDFC buffer 0 is used for device ID output */
	writew(0x0, &host->regs->buf_addr);

	/* Read ID into main buffer */
	tmp = readw(&host->regs->config1);
	tmp &= ~NFC_SP_EN;
	writew(tmp, &host->regs->config1);

	writew(NFC_ID, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, 0);
}

/*
 * This function requests the NANDFC to perform a read of the
 * NAND device status and returns the current status.
 */
static uint16_t get_dev_status(struct mxc_nand_host *host)
{
	void __iomem *main_buf = host->regs->main_area[1];
	uint32_t store;
	uint16_t ret, tmp;
	/* Issue status request to NAND device */

	/* store the main area1 first word, later do recovery */
	store = readl(main_buf);
	/* NANDFC buffer 1 is used for device status */
	writew(1, &host->regs->buf_addr);

	/* Read status into main buffer */
	tmp = readw(&host->regs->config1);
	tmp &= ~NFC_SP_EN;
	writew(tmp, &host->regs->config1);

	writew(NFC_STATUS, &host->regs->config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, 0);

	/*
	 *  Status is placed in first word of main buffer
	 * get status, then recovery area 1 data
	 */
	ret = readw(main_buf);
	writel(store, main_buf);

	return ret;
}

/* This function is used by upper layer to checks if device is ready */
static int mxc_nand_dev_ready(struct mtd_info *mtd)
{
	/*
	 * NFC handles R/B internally. Therefore, this function
	 * always returns status as ready.
	 */
	return 1;
}

static void _mxc_nand_enable_hwecc(struct mtd_info *mtd, int on)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	uint16_t tmp = readw(&host->regs->config1);

	if (on)
		tmp |= NFC_ECC_EN;
	else
		tmp &= ~NFC_ECC_EN;
	writew(tmp, &host->regs->config1);
}

#ifdef CONFIG_MXC_NAND_HWECC
static void mxc_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	/*
	 * If HW ECC is enabled, we turn it on during init. There is
	 * no need to enable again here.
	 */
}

#ifdef MXC_NFC_V2_1
static int mxc_nand_read_oob_syndrome(struct mtd_info *mtd,
				      struct nand_chip *chip,
				      int page, int sndcmd)
{
	struct mxc_nand_host *host = chip->priv;
	uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;
	int eccpitch = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;
	uint8_t *bufpoi = buf;
	int i, toread;

	MTDDEBUG(MTD_DEBUG_LEVEL0,
			"%s: Reading OOB area of page %u to oob %p\n",
			 __FUNCTION__, host->page_addr, buf);

	chip->cmdfunc(mtd, NAND_CMD_READOOB, mtd->writesize, page);
	for (i = 0; i < chip->ecc.steps; i++) {
		toread = min_t(int, length, chip->ecc.prepad);
		if (toread) {
			chip->read_buf(mtd, bufpoi, toread);
			bufpoi += toread;
			length -= toread;
		}
		bufpoi += chip->ecc.bytes;
		host->col_addr += chip->ecc.bytes;
		length -= chip->ecc.bytes;

		toread = min_t(int, length, chip->ecc.postpad);
		if (toread) {
			chip->read_buf(mtd, bufpoi, toread);
			bufpoi += toread;
			length -= toread;
		}
	}
	if (length > 0)
		chip->read_buf(mtd, bufpoi, length);

	_mxc_nand_enable_hwecc(mtd, 0);
	chip->cmdfunc(mtd, NAND_CMD_READOOB,
			mtd->writesize + chip->ecc.prepad, page);
	bufpoi = buf + chip->ecc.prepad;
	length = mtd->oobsize - chip->ecc.prepad;
	for (i = 0; i < chip->ecc.steps; i++) {
		toread = min_t(int, length, chip->ecc.bytes);
		chip->read_buf(mtd, bufpoi, toread);
		bufpoi += eccpitch;
		length -= eccpitch;
		host->col_addr += chip->ecc.postpad + chip->ecc.prepad;
	}
	_mxc_nand_enable_hwecc(mtd, 1);
	return 1;
}

static int mxc_nand_read_page_raw_syndrome(struct mtd_info *mtd,
					   struct nand_chip *chip,
					   uint8_t *buf,
					   int page)
{
	struct mxc_nand_host *host = chip->priv;
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccpitch = eccbytes + chip->ecc.prepad + chip->ecc.postpad;
	uint8_t *oob = chip->oob_poi;
	int steps, size;
	int n;

	_mxc_nand_enable_hwecc(mtd, 0);
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, host->page_addr);

	for (n = 0, steps = chip->ecc.steps; steps > 0; n++, steps--) {
		host->col_addr = n * eccsize;
		chip->read_buf(mtd, buf, eccsize);
		buf += eccsize;

		host->col_addr = mtd->writesize + n * eccpitch;
		if (chip->ecc.prepad) {
			chip->read_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->read_buf(mtd, oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->read_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	size = mtd->oobsize - (oob - chip->oob_poi);
	if (size)
		chip->read_buf(mtd, oob, size);
	_mxc_nand_enable_hwecc(mtd, 1);

	return 0;
}

static int mxc_nand_read_page_syndrome(struct mtd_info *mtd,
				       struct nand_chip *chip,
				       uint8_t *buf,
				       int page)
{
	struct mxc_nand_host *host = chip->priv;
	int n, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccpitch = eccbytes + chip->ecc.prepad + chip->ecc.postpad;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	MTDDEBUG(MTD_DEBUG_LEVEL1, "Reading page %u to buf %p oob %p\n",
	      host->page_addr, buf, oob);

	/* first read the data area and the available portion of OOB */
	for (n = 0; eccsteps; n++, eccsteps--, p += eccsize) {
		int stat;

		host->col_addr = n * eccsize;

		chip->read_buf(mtd, p, eccsize);

		host->col_addr = mtd->writesize + n * eccpitch;

		if (chip->ecc.prepad) {
			chip->read_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		stat = chip->ecc.correct(mtd, p, oob, NULL);

		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->read_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	/* Calculate remaining oob bytes */
	n = mtd->oobsize - (oob - chip->oob_poi);
	if (n)
		chip->read_buf(mtd, oob, n);

	/* Then switch ECC off and read the OOB area to get the ECC code */
	_mxc_nand_enable_hwecc(mtd, 0);
	chip->cmdfunc(mtd, NAND_CMD_READOOB, mtd->writesize, host->page_addr);
	eccsteps = chip->ecc.steps;
	oob = chip->oob_poi + chip->ecc.prepad;
	for (n = 0; eccsteps; n++, eccsteps--, p += eccsize) {
		host->col_addr = mtd->writesize +
				 n * eccpitch +
				 chip->ecc.prepad;
		chip->read_buf(mtd, oob, eccbytes);
		oob += eccbytes + chip->ecc.postpad;
	}
	_mxc_nand_enable_hwecc(mtd, 1);
	return 0;
}

static int mxc_nand_write_oob_syndrome(struct mtd_info *mtd,
				       struct nand_chip *chip, int page)
{
	struct mxc_nand_host *host = chip->priv;
	int eccpitch = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;
	int length = mtd->oobsize;
	int i, len, status, steps = chip->ecc.steps;
	const uint8_t *bufpoi = chip->oob_poi;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	for (i = 0; i < steps; i++) {
		len = min_t(int, length, eccpitch);

		chip->write_buf(mtd, bufpoi, len);
		bufpoi += len;
		length -= len;
		host->col_addr += chip->ecc.prepad + chip->ecc.postpad;
	}
	if (length > 0)
		chip->write_buf(mtd, bufpoi, length);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);
	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

static void mxc_nand_write_page_raw_syndrome(struct mtd_info *mtd,
					     struct nand_chip *chip,
					     const uint8_t *buf)
{
	struct mxc_nand_host *host = chip->priv;
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccpitch = eccbytes + chip->ecc.prepad + chip->ecc.postpad;
	uint8_t *oob = chip->oob_poi;
	int steps, size;
	int n;

	for (n = 0, steps = chip->ecc.steps; steps > 0; n++, steps--) {
		host->col_addr = n * eccsize;
		chip->write_buf(mtd, buf, eccsize);
		buf += eccsize;

		host->col_addr = mtd->writesize + n * eccpitch;

		if (chip->ecc.prepad) {
			chip->write_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		host->col_addr += eccbytes;
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->write_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	size = mtd->oobsize - (oob - chip->oob_poi);
	if (size)
		chip->write_buf(mtd, oob, size);
}

static void mxc_nand_write_page_syndrome(struct mtd_info *mtd,
					 struct nand_chip *chip,
					 const uint8_t *buf)
{
	struct mxc_nand_host *host = chip->priv;
	int i, n, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccpitch = eccbytes + chip->ecc.prepad + chip->ecc.postpad;
	int eccsteps = chip->ecc.steps;
	const uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);

	for (i = n = 0;
	     eccsteps;
	     n++, eccsteps--, i += eccbytes, p += eccsize) {
		host->col_addr = n * eccsize;

		chip->write_buf(mtd, p, eccsize);

		host->col_addr = mtd->writesize + n * eccpitch;

		if (chip->ecc.prepad) {
			chip->write_buf(mtd, oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		chip->write_buf(mtd, oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			chip->write_buf(mtd, oob, chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	/* Calculate remaining oob bytes */
	i = mtd->oobsize - (oob - chip->oob_poi);
	if (i)
		chip->write_buf(mtd, oob, i);
}

static int mxc_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				 u_char *read_ecc, u_char *calc_ecc)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	uint32_t ecc_status = readl(&host->regs->ecc_status_result);
	int subpages = mtd->writesize / nand_chip->subpagesize;
	int pg2blk_shift = nand_chip->phys_erase_shift -
			   nand_chip->page_shift;

	do {
		if ((ecc_status & 0xf) > 4) {
			static int last_bad = -1;

			if (last_bad != host->page_addr >> pg2blk_shift) {
				last_bad = host->page_addr >> pg2blk_shift;
				printk(KERN_DEBUG
				       "MXC_NAND: HWECC uncorrectable ECC error"
				       " in block %u page %u subpage %d\n",
				       last_bad, host->page_addr,
				       mtd->writesize / nand_chip->subpagesize
					    - subpages);
			}
			return -1;
		}
		ecc_status >>= 4;
		subpages--;
	} while (subpages > 0);

	return 0;
}
#else
#define mxc_nand_read_page_syndrome NULL
#define mxc_nand_read_page_raw_syndrome NULL
#define mxc_nand_read_oob_syndrome NULL
#define mxc_nand_write_page_syndrome NULL
#define mxc_nand_write_page_raw_syndrome NULL
#define mxc_nand_write_oob_syndrome NULL

static int mxc_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				 u_char *read_ecc, u_char *calc_ecc)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;

	/*
	 * 1-Bit errors are automatically corrected in HW.  No need for
	 * additional correction.  2-Bit errors cannot be corrected by
	 * HW ECC, so we need to return failure
	 */
	uint16_t ecc_status = readw(&host->regs->ecc_status_result);

	if (((ecc_status & 0x3) == 2) || ((ecc_status >> 2) == 2)) {
		MTDDEBUG(MTD_DEBUG_LEVEL0,
		      "MXC_NAND: HWECC uncorrectable 2-bit ECC error\n");
		return -1;
	}

	return 0;
}
#endif

static int mxc_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				  u_char *ecc_code)
{
	return 0;
}
#endif

static u_char mxc_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	uint8_t ret = 0;
	uint16_t col;
	uint16_t __iomem *main_buf =
		(uint16_t __iomem *)host->regs->main_area[0];
	uint16_t __iomem *spare_buf =
		(uint16_t __iomem *)host->regs->spare_area[0];
	union {
		uint16_t word;
		uint8_t bytes[2];
	} nfc_word;

	/* Check for status request */
	if (host->status_request)
		return get_dev_status(host) & 0xFF;

	/* Get column for 16-bit access */
	col = host->col_addr >> 1;

	/* If we are accessing the spare region */
	if (host->spare_only)
		nfc_word.word = readw(&spare_buf[col]);
	else
		nfc_word.word = readw(&main_buf[col]);

	/* Pick upper/lower byte of word from RAM buffer */
	ret = nfc_word.bytes[host->col_addr & 0x1];

	/* Update saved column address */
	if (nand_chip->options & NAND_BUSWIDTH_16)
		host->col_addr += 2;
	else
		host->col_addr++;

	return ret;
}

static uint16_t mxc_nand_read_word(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	uint16_t col, ret;
	uint16_t __iomem *p;

	MTDDEBUG(MTD_DEBUG_LEVEL3,
	      "mxc_nand_read_word(col = %d)\n", host->col_addr);

	col = host->col_addr;
	/* Adjust saved column address */
	if (col < mtd->writesize && host->spare_only)
		col += mtd->writesize;

	if (col < mtd->writesize) {
		p = (uint16_t __iomem *)(host->regs->main_area[0] +
				(col >> 1));
	} else {
		p = (uint16_t __iomem *)(host->regs->spare_area[0] +
				((col - mtd->writesize) >> 1));
	}

	if (col & 1) {
		union {
			uint16_t word;
			uint8_t bytes[2];
		} nfc_word[3];

		nfc_word[0].word = readw(p);
		nfc_word[1].word = readw(p + 1);

		nfc_word[2].bytes[0] = nfc_word[0].bytes[1];
		nfc_word[2].bytes[1] = nfc_word[1].bytes[0];

		ret = nfc_word[2].word;
	} else {
		ret = readw(p);
	}

	/* Update saved column address */
	host->col_addr = col + 2;

	return ret;
}

/*
 * Write data of length len to buffer buf. The data to be
 * written on NAND Flash is first copied to RAMbuffer. After the Data Input
 * Operation by the NFC, the data is written to NAND Flash
 */
static void mxc_nand_write_buf(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	int n, col, i = 0;

	MTDDEBUG(MTD_DEBUG_LEVEL3,
	      "mxc_nand_write_buf(col = %d, len = %d)\n", host->col_addr,
	      len);

	col = host->col_addr;

	/* Adjust saved column address */
	if (col < mtd->writesize && host->spare_only)
		col += mtd->writesize;

	n = mtd->writesize + mtd->oobsize - col;
	n = min(len, n);

	MTDDEBUG(MTD_DEBUG_LEVEL3,
	      "%s:%d: col = %d, n = %d\n", __func__, __LINE__, col, n);

	while (n > 0) {
		void __iomem *p;

		if (col < mtd->writesize) {
			p = host->regs->main_area[0] + (col & ~3);
		} else {
			p = host->regs->spare_area[0] -
						mtd->writesize + (col & ~3);
		}

		MTDDEBUG(MTD_DEBUG_LEVEL3, "%s:%d: p = %p\n", __func__,
		      __LINE__, p);

		if (((col | (unsigned long)&buf[i]) & 3) || n < 4) {
			union {
				uint32_t word;
				uint8_t bytes[4];
			} nfc_word;

			nfc_word.word = readl(p);
			nfc_word.bytes[col & 3] = buf[i++];
			n--;
			col++;

			writel(nfc_word.word, p);
		} else {
			int m = mtd->writesize - col;

			if (col >= mtd->writesize)
				m += mtd->oobsize;

			m = min(n, m) & ~3;

			MTDDEBUG(MTD_DEBUG_LEVEL3,
			      "%s:%d: n = %d, m = %d, i = %d, col = %d\n",
			      __func__,  __LINE__, n, m, i, col);

			mxc_nand_memcpy32(p, (uint32_t *)&buf[i], m);
			col += m;
			i += m;
			n -= m;
		}
	}
	/* Update saved column address */
	host->col_addr = col;
}

/*
 * Read the data buffer from the NAND Flash. To read the data from NAND
 * Flash first the data output cycle is initiated by the NFC, which copies
 * the data to RAMbuffer. This data of length len is then copied to buffer buf.
 */
static void mxc_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;
	int n, col, i = 0;

	MTDDEBUG(MTD_DEBUG_LEVEL3,
	      "mxc_nand_read_buf(col = %d, len = %d)\n", host->col_addr, len);

	col = host->col_addr;

	/* Adjust saved column address */
	if (col < mtd->writesize && host->spare_only)
		col += mtd->writesize;

	n = mtd->writesize + mtd->oobsize - col;
	n = min(len, n);

	while (n > 0) {
		void __iomem *p;

		if (col < mtd->writesize) {
			p = host->regs->main_area[0] + (col & ~3);
		} else {
			p = host->regs->spare_area[0] -
					mtd->writesize + (col & ~3);
		}

		if (((col | (int)&buf[i]) & 3) || n < 4) {
			union {
				uint32_t word;
				uint8_t bytes[4];
			} nfc_word;

			nfc_word.word = readl(p);
			buf[i++] = nfc_word.bytes[col & 3];
			n--;
			col++;
		} else {
			int m = mtd->writesize - col;

			if (col >= mtd->writesize)
				m += mtd->oobsize;

			m = min(n, m) & ~3;
			mxc_nand_memcpy32((uint32_t *)&buf[i], p, m);

			col += m;
			i += m;
			n -= m;
		}
	}
	/* Update saved column address */
	host->col_addr = col;
}

/*
 * Used by the upper layer to verify the data in NAND Flash
 * with the data in the buf.
 */
static int mxc_nand_verify_buf(struct mtd_info *mtd,
				const u_char *buf, int len)
{
	u_char tmp[256];
	uint bsize;

	while (len) {
		bsize = min(len, 256);
		mxc_nand_read_buf(mtd, tmp, bsize);

		if (memcmp(buf, tmp, bsize))
			return 1;

		buf += bsize;
		len -= bsize;
	}

	return 0;
}

/*
 * This function is used by upper layer for select and
 * deselect of the NAND chip
 */
static void mxc_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;

	switch (chip) {
	case -1:
		/* TODO: Disable the NFC clock */
		if (host->clk_act)
			host->clk_act = 0;
		break;
	case 0:
		/* TODO: Enable the NFC clock */
		if (!host->clk_act)
			host->clk_act = 1;
		break;

	default:
		break;
	}
}

/*
 * Used by the upper layer to write command to NAND Flash for
 * different operations to be carried out on NAND Flash
 */
void mxc_nand_command(struct mtd_info *mtd, unsigned command,
				int column, int page_addr)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct mxc_nand_host *host = nand_chip->priv;

	MTDDEBUG(MTD_DEBUG_LEVEL3,
	      "mxc_nand_command (cmd = 0x%x, col = 0x%x, page = 0x%x)\n",
	      command, column, page_addr);

	/* Reset command state information */
	host->status_request = false;

	/* Command pre-processing step */
	switch (command) {

	case NAND_CMD_STATUS:
		host->col_addr = 0;
		host->status_request = true;
		break;

	case NAND_CMD_READ0:
		host->page_addr = page_addr;
		host->col_addr = column;
		host->spare_only = false;
		break;

	case NAND_CMD_READOOB:
		host->col_addr = column;
		host->spare_only = true;
		if (host->pagesize_2k)
			command = NAND_CMD_READ0; /* only READ0 is valid */
		break;

	case NAND_CMD_SEQIN:
		if (column >= mtd->writesize) {
			/*
			 * before sending SEQIN command for partial write,
			 * we need read one page out. FSL NFC does not support
			 * partial write. It always sends out 512+ecc+512+ecc
			 * for large page nand flash. But for small page nand
			 * flash, it does support SPARE ONLY operation.
			 */
			if (host->pagesize_2k) {
				/* call ourself to read a page */
				mxc_nand_command(mtd, NAND_CMD_READ0, 0,
						page_addr);
			}

			host->col_addr = column - mtd->writesize;
			host->spare_only = true;

			/* Set program pointer to spare region */
			if (!host->pagesize_2k)
				send_cmd(host, NAND_CMD_READOOB);
		} else {
			host->spare_only = false;
			host->col_addr = column;

			/* Set program pointer to page start */
			if (!host->pagesize_2k)
				send_cmd(host, NAND_CMD_READ0);
		}
		break;

	case NAND_CMD_PAGEPROG:
		send_prog_page(host, 0, host->spare_only);

		if (host->pagesize_2k && is_mxc_nfc_1()) {
			/* data in 4 areas */
			send_prog_page(host, 1, host->spare_only);
			send_prog_page(host, 2, host->spare_only);
			send_prog_page(host, 3, host->spare_only);
		}

		break;
	}

	/* Write out the command to the device. */
	send_cmd(host, command);

	/* Write out column address, if necessary */
	if (column != -1) {
		/*
		 * MXC NANDFC can only perform full page+spare or
		 * spare-only read/write. When the upper layers perform
		 * a read/write buffer operation, we will use the saved
		 * column address to index into the full page.
		 */
		send_addr(host, 0);
		if (host->pagesize_2k)
			/* another col addr cycle for 2k page */
			send_addr(host, 0);
	}

	/* Write out page address, if necessary */
	if (page_addr != -1) {
		u32 page_mask = nand_chip->pagemask;
		do {
			send_addr(host, page_addr & 0xFF);
			page_addr >>= 8;
			page_mask >>= 8;
		} while (page_mask);
	}

	/* Command post-processing step */
	switch (command) {

	case NAND_CMD_RESET:
		break;

	case NAND_CMD_READOOB:
	case NAND_CMD_READ0:
		if (host->pagesize_2k) {
			/* send read confirm command */
			send_cmd(host, NAND_CMD_READSTART);
			/* read for each AREA */
			send_read_page(host, 0, host->spare_only);
			if (is_mxc_nfc_1()) {
				send_read_page(host, 1, host->spare_only);
				send_read_page(host, 2, host->spare_only);
				send_read_page(host, 3, host->spare_only);
			}
		} else {
			send_read_page(host, 0, host->spare_only);
		}
		break;

	case NAND_CMD_READID:
		host->col_addr = 0;
		send_read_id(host);
		break;

	case NAND_CMD_PAGEPROG:
		break;

	case NAND_CMD_STATUS:
		break;

	case NAND_CMD_ERASE2:
		break;
	}
}

#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT

static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

#endif

int board_nand_init(struct nand_chip *this)
{
	struct mtd_info *mtd;
#ifdef MXC_NFC_V2_1
	uint16_t tmp;
#endif

#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT
	this->options |= NAND_USE_FLASH_BBT;
	this->bbt_td = &bbt_main_descr;
	this->bbt_md = &bbt_mirror_descr;
#endif

	/* structures must be linked */
	mtd = &host->mtd;
	mtd->priv = this;
	host->nand = this;

	/* 5 us command delay time */
	this->chip_delay = 5;

	this->priv = host;
	this->dev_ready = mxc_nand_dev_ready;
	this->cmdfunc = mxc_nand_command;
	this->select_chip = mxc_nand_select_chip;
	this->read_byte = mxc_nand_read_byte;
	this->read_word = mxc_nand_read_word;
	this->write_buf = mxc_nand_write_buf;
	this->read_buf = mxc_nand_read_buf;
	this->verify_buf = mxc_nand_verify_buf;

	host->regs = (struct fsl_nfc_regs __iomem *)CONFIG_MXC_NAND_REGS_BASE;
	host->clk_act = 1;

#ifdef CONFIG_MXC_NAND_HWECC
	this->ecc.calculate = mxc_nand_calculate_ecc;
	this->ecc.hwctl = mxc_nand_enable_hwecc;
	this->ecc.correct = mxc_nand_correct_data;
	if (is_mxc_nfc_21()) {
		this->ecc.mode = NAND_ECC_HW_SYNDROME;
		this->ecc.read_page = mxc_nand_read_page_syndrome;
		this->ecc.read_page_raw = mxc_nand_read_page_raw_syndrome;
		this->ecc.read_oob = mxc_nand_read_oob_syndrome;
		this->ecc.write_page = mxc_nand_write_page_syndrome;
		this->ecc.write_page_raw = mxc_nand_write_page_raw_syndrome;
		this->ecc.write_oob = mxc_nand_write_oob_syndrome;
		this->ecc.bytes = 9;
		this->ecc.prepad = 7;
	} else {
		this->ecc.mode = NAND_ECC_HW;
	}

	host->pagesize_2k = 0;

	this->ecc.size = 512;
	_mxc_nand_enable_hwecc(mtd, 1);
#else
	this->ecc.layout = &nand_soft_eccoob;
	this->ecc.mode = NAND_ECC_SOFT;
	_mxc_nand_enable_hwecc(mtd, 0);
#endif
	/* Reset NAND */
	this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/* NAND bus width determines access functions used by upper layer */
	if (is_16bit_nand())
		this->options |= NAND_BUSWIDTH_16;

#ifdef CONFIG_SYS_NAND_LARGEPAGE
	host->pagesize_2k = 1;
	this->ecc.layout = &nand_hw_eccoob2k;
#else
	host->pagesize_2k = 0;
	this->ecc.layout = &nand_hw_eccoob;
#endif

#ifdef MXC_NFC_V2_1
	tmp = readw(&host->regs->config1);
	tmp |= NFC_ONE_CYCLE;
	tmp |= NFC_4_8N_ECC;
	writew(tmp, &host->regs->config1);
	if (host->pagesize_2k)
		writew(64/2, &host->regs->spare_area_size);
	else
		writew(16/2, &host->regs->spare_area_size);
#endif

	/*
	 * preset operation
	 * Unlock the internal RAM Buffer
	 */
	writew(0x2, &host->regs->config);

	/* Blocks to be unlocked */
	writew(0x0, &host->regs->unlockstart_blkaddr);
	/* Originally (Freescale LTIB 2.6.21) 0x4000 was written to the
	 * unlockend_blkaddr, but the magic 0x4000 does not always work
	 * when writing more than some 32 megabytes (on 2k page nands)
	 * However 0xFFFF doesn't seem to have this kind
	 * of limitation (tried it back and forth several times).
	 * The linux kernel driver sets this to 0xFFFF for the v2 controller
	 * only, but probably this was not tested there for v1.
	 * The very same limitation seems to apply to this kernel driver.
	 * This might be NAND chip specific and the i.MX31 datasheet is
	 * extremely vague about the semantics of this register.
	 */
	writew(0xFFFF, &host->regs->unlockend_blkaddr);

	/* Unlock Block Command for given address range */
	writew(0x4, &host->regs->wrprot);

	return 0;
}
