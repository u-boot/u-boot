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
#ifdef CONFIG_MX27
#include <asm/arch/imx-regs.h>
#endif

#define DRIVER_NAME "mxc_nand"

struct nfc_regs {
/* NFC RAM BUFFER Main area 0 */
	uint8_t main_area0[0x200];
	uint8_t main_area1[0x200];
	uint8_t main_area2[0x200];
	uint8_t main_area3[0x200];
/* SPARE BUFFER Spare area 0 */
	uint8_t spare_area0[0x10];
	uint8_t spare_area1[0x10];
	uint8_t spare_area2[0x10];
	uint8_t spare_area3[0x10];
	uint8_t pad[0x5c0];
/* NFC registers */
	uint16_t nfc_buf_size;
	uint16_t reserved;
	uint16_t nfc_buf_addr;
	uint16_t nfc_flash_addr;
	uint16_t nfc_flash_cmd;
	uint16_t nfc_config;
	uint16_t nfc_ecc_status_result;
	uint16_t nfc_rsltmain_area;
	uint16_t nfc_rsltspare_area;
	uint16_t nfc_wrprot;
	uint16_t nfc_unlockstart_blkaddr;
	uint16_t nfc_unlockend_blkaddr;
	uint16_t nfc_nf_wrprst;
	uint16_t nfc_config1;
	uint16_t nfc_config2;
};

/*
 * Set INT to 0, FCMD to 1, rest to 0 in NFC_CONFIG2 Register
 * for Command operation
 */
#define NFC_CMD            0x1

/*
 * Set INT to 0, FADD to 1, rest to 0 in NFC_CONFIG2 Register
 * for Address operation
 */
#define NFC_ADDR           0x2

/*
 * Set INT to 0, FDI to 1, rest to 0 in NFC_CONFIG2 Register
 * for Input operation
 */
#define NFC_INPUT          0x4

/*
 * Set INT to 0, FDO to 001, rest to 0 in NFC_CONFIG2 Register
 * for Data Output operation
 */
#define NFC_OUTPUT         0x8

/*
 * Set INT to 0, FD0 to 010, rest to 0 in NFC_CONFIG2 Register
 * for Read ID operation
 */
#define NFC_ID             0x10

/*
 * Set INT to 0, FDO to 100, rest to 0 in NFC_CONFIG2 Register
 * for Read Status operation
 */
#define NFC_STATUS         0x20

/*
 * Set INT to 1, rest to 0 in NFC_CONFIG2 Register for Read
 * Status operation
 */
#define NFC_INT            0x8000

#define NFC_SP_EN           (1 << 2)
#define NFC_ECC_EN          (1 << 3)
#define NFC_BIG             (1 << 5)
#define NFC_RST             (1 << 6)
#define NFC_CE              (1 << 7)
#define NFC_ONE_CYCLE       (1 << 8)

typedef enum {false, true} bool;

struct mxc_nand_host {
	struct mtd_info		mtd;
	struct nand_chip	*nand;

	struct nfc_regs __iomem	*regs;
	int			spare_only;
	int			status_request;
	int			pagesize_2k;
	int			clk_act;
	uint16_t		col_addr;
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
#ifdef CONFIG_MXC_NAND_HWECC
static struct nand_ecclayout nand_hw_eccoob = {
	.eccbytes = 5,
	.eccpos = {6, 7, 8, 9, 10},
	.oobfree = {{0, 5}, {11, 5}, }
};
#else
static struct nand_ecclayout nand_soft_eccoob = {
	.eccbytes = 6,
	.eccpos = {6, 7, 8, 9, 10, 11},
	.oobfree = {{0, 5}, {12, 4}, }
};
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
		if (readw(&host->regs->nfc_config2) & NFC_INT) {
			tmp = readw(&host->regs->nfc_config2);
			tmp  &= ~NFC_INT;
			writew(tmp, &host->regs->nfc_config2);
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

	writew(cmd, &host->regs->nfc_flash_cmd);
	writew(NFC_CMD, &host->regs->nfc_config2);

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

	writew(addr, &host->regs->nfc_flash_addr);
	writew(NFC_ADDR, &host->regs->nfc_config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, addr);
}

/*
 * This function requests the NANDFC to initate the transfer
 * of data currently in the NANDFC RAM buffer to the NAND device.
 */
static void send_prog_page(struct mxc_nand_host *host, uint8_t buf_id,
			int spare_only)
{
	MTDDEBUG(MTD_DEBUG_LEVEL3, "send_prog_page (%d)\n", spare_only);

	writew(buf_id, &host->regs->nfc_buf_addr);

	/* Configure spare or page+spare access */
	if (!host->pagesize_2k) {
		uint16_t config1 = readw(&host->regs->nfc_config1);
		if (spare_only)
			config1 |= NFC_SP_EN;
		else
			config1 &= ~(NFC_SP_EN);
		writew(config1, &host->regs->nfc_config1);
	}

	writew(NFC_INPUT, &host->regs->nfc_config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, spare_only);
}

/*
 * Requests NANDFC to initated the transfer of data from the
 * NAND device into in the NANDFC ram buffer.
 */
static void send_read_page(struct mxc_nand_host *host, uint8_t buf_id,
		int spare_only)
{
	MTDDEBUG(MTD_DEBUG_LEVEL3, "send_read_page (%d)\n", spare_only);

	writew(buf_id, &host->regs->nfc_buf_addr);

	/* Configure spare or page+spare access */
	if (!host->pagesize_2k) {
		uint32_t config1 = readw(&host->regs->nfc_config1);
		if (spare_only)
			config1 |= NFC_SP_EN;
		else
			config1 &= ~NFC_SP_EN;
		writew(config1, &host->regs->nfc_config1);
	}

	writew(NFC_OUTPUT, &host->regs->nfc_config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, spare_only);
}

/* Request the NANDFC to perform a read of the NAND device ID. */
static void send_read_id(struct mxc_nand_host *host)
{
	uint16_t tmp;

	/* NANDFC buffer 0 is used for device ID output */
	writew(0x0, &host->regs->nfc_buf_addr);

	/* Read ID into main buffer */
	tmp = readw(&host->regs->nfc_config1);
	tmp &= ~NFC_SP_EN;
	writew(tmp, &host->regs->nfc_config1);

	writew(NFC_ID, &host->regs->nfc_config2);

	/* Wait for operation to complete */
	wait_op_done(host, TROP_US_DELAY, 0);
}

/*
 * This function requests the NANDFC to perform a read of the
 * NAND device status and returns the current status.
 */
static uint16_t get_dev_status(struct mxc_nand_host *host)
{
	void __iomem *main_buf = host->regs->main_area1;
	uint32_t store;
	uint16_t ret, tmp;
	/* Issue status request to NAND device */

	/* store the main area1 first word, later do recovery */
	store = readl(main_buf);
	/* NANDFC buffer 1 is used for device status */
	writew(1, &host->regs->nfc_buf_addr);

	/* Read status into main buffer */
	tmp = readw(&host->regs->nfc_config1);
	tmp &= ~NFC_SP_EN;
	writew(tmp, &host->regs->nfc_config1);

	writew(NFC_STATUS, &host->regs->nfc_config2);

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

#ifdef CONFIG_MXC_NAND_HWECC
static void mxc_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	/*
	 * If HW ECC is enabled, we turn it on during init. There is
	 * no need to enable again here.
	 */
}

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
	uint16_t ecc_status = readw(&host->regs->nfc_ecc_status_result);

	if (((ecc_status & 0x3) == 2) || ((ecc_status >> 2) == 2)) {
		MTDDEBUG(MTD_DEBUG_LEVEL0,
		      "MXC_NAND: HWECC uncorrectable 2-bit ECC error\n");
		return -1;
	}

	return 0;
}

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
		(uint16_t __iomem *)host->regs->main_area0;
	uint16_t __iomem *spare_buf =
		(uint16_t __iomem *)host->regs->spare_area0;
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
		p = (uint16_t __iomem *)(host->regs->main_area0 + (col >> 1));
	} else {
		p = (uint16_t __iomem *)(host->regs->spare_area0 +
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
			p = host->regs->main_area0 + (col & ~3);
		} else {
			p = host->regs->spare_area0 -
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
			p = host->regs->main_area0 + (col & ~3);
		} else {
			p = host->regs->spare_area0 -
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
static void mxc_nand_command(struct mtd_info *mtd, unsigned command,
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
			 * partial write. It alway send out 512+ecc+512+ecc ...
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

		if (host->pagesize_2k) {
			/* data in 4 areas datas */
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
		 * spare-only read/write.  When the upper layers
		 * layers perform a read/write buf operation,
		 * we will used the saved column adress to index into
		 * the full page.
		 */
		send_addr(host, 0);
		if (host->pagesize_2k)
			/* another col addr cycle for 2k page */
			send_addr(host, 0);
	}

	/* Write out page address, if necessary */
	if (page_addr != -1) {
		/* paddr_0 - p_addr_7 */
		send_addr(host, (page_addr & 0xff));

		if (host->pagesize_2k) {
			send_addr(host, (page_addr >> 8) & 0xFF);
			if (mtd->size >= 0x10000000) {
				/* paddr_8 - paddr_15 */
				send_addr(host, (page_addr >> 8) & 0xff);
				send_addr(host, (page_addr >> 16) & 0xff);
			} else {
				/* paddr_8 - paddr_15 */
				send_addr(host, (page_addr >> 8) & 0xff);
			}
		} else {
			/* One more address cycle for higher density devices */
			if (mtd->size >= 0x4000000) {
				/* paddr_8 - paddr_15 */
				send_addr(host, (page_addr >> 8) & 0xff);
				send_addr(host, (page_addr >> 16) & 0xff);
			} else {
				/* paddr_8 - paddr_15 */
				send_addr(host, (page_addr >> 8) & 0xff);
			}
		}
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
			send_read_page(host, 1, host->spare_only);
			send_read_page(host, 2, host->spare_only);
			send_read_page(host, 3, host->spare_only);
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

int board_nand_init(struct nand_chip *this)
{
	struct system_control_regs *sc_regs =
		(struct system_control_regs *)IMX_SYSTEM_CTL_BASE;
	struct mtd_info *mtd;
	uint16_t tmp;
	int err = 0;

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

	host->regs = (struct nfc_regs __iomem *)CONFIG_MXC_NAND_REGS_BASE;
	host->clk_act = 1;

#ifdef CONFIG_MXC_NAND_HWECC
	this->ecc.calculate = mxc_nand_calculate_ecc;
	this->ecc.hwctl = mxc_nand_enable_hwecc;
	this->ecc.correct = mxc_nand_correct_data;
	this->ecc.mode = NAND_ECC_HW;
	this->ecc.size = 512;
	this->ecc.bytes = 3;
	this->ecc.layout = &nand_hw_eccoob;
	tmp = readw(&host->regs->nfc_config1);
	tmp |= NFC_ECC_EN;
	writew(tmp, &host->regs->nfc_config1);
#else
	this->ecc.layout = &nand_soft_eccoob;
	this->ecc.mode = NAND_ECC_SOFT;
	tmp = readw(&host->regs->nfc_config1);
	tmp &= ~NFC_ECC_EN;
	writew(tmp, &host->regs->nfc_config1);
#endif

	/* Reset NAND */
	this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/*
	 * preset operation
	 * Unlock the internal RAM Buffer
	 */
	writew(0x2, &host->regs->nfc_config);

	/* Blocks to be unlocked */
	writew(0x0, &host->regs->nfc_unlockstart_blkaddr);
	writew(0x4000, &host->regs->nfc_unlockend_blkaddr);

	/* Unlock Block Command for given address range */
	writew(0x4, &host->regs->nfc_wrprot);

	/* NAND bus width determines access funtions used by upper layer */
	if (readl(&sc_regs->fmcr) & NF_16BIT_SEL)
		this->options |= NAND_BUSWIDTH_16;

	host->pagesize_2k = 0;

	return err;
}
