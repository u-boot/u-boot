/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2006 ATMEL Rousset, Lacressonniere Nicolas
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_pio.h>

#include <nand.h>

#ifdef CONFIG_ATMEL_NAND_HWECC

/* Register access macros */
#define ecc_readl(add, reg)				\
	readl(AT91_BASE_SYS + add + ATMEL_ECC_##reg)
#define ecc_writel(add, reg, value)			\
	writel((value), AT91_BASE_SYS + add + ATMEL_ECC_##reg)

#include "atmel_nand_ecc.h"	/* Hardware ECC registers */

/* oob layout for large page size
 * bad block info is on bytes 0 and 1
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_large = {
	.eccbytes = 4,
	.eccpos = {60, 61, 62, 63},
	.oobfree = {
		{2, 58}
	},
};

/* oob layout for small page size
 * bad block info is on bytes 4 and 5
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_small = {
	.eccbytes = 4,
	.eccpos = {0, 1, 2, 3},
	.oobfree = {
		{6, 10}
	},
};

/*
 * Calculate HW ECC
 *
 * function called after a write
 *
 * mtd:        MTD block structure
 * dat:        raw data (unused)
 * ecc_code:   buffer for ECC
 */
static int atmel_nand_calculate(struct mtd_info *mtd,
		const u_char *dat, unsigned char *ecc_code)
{
	struct nand_chip *nand_chip = mtd->priv;
	unsigned int ecc_value;

	/* get the first 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR);

	ecc_code[0] = ecc_value & 0xFF;
	ecc_code[1] = (ecc_value >> 8) & 0xFF;

	/* get the last 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, NPR) & ATMEL_ECC_NPARITY;

	ecc_code[2] = ecc_value & 0xFF;
	ecc_code[3] = (ecc_value >> 8) & 0xFF;

	return 0;
}

/*
 * HW ECC read page function
 *
 * mtd:        mtd info structure
 * chip:       nand chip info structure
 * buf:        buffer to store read data
 */
static int atmel_nand_read_page(struct mtd_info *mtd,
		struct nand_chip *chip, uint8_t *buf, int page)
{
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;
	uint8_t *ecc_pos;
	int stat;

	/* read the page */
	chip->read_buf(mtd, p, eccsize);

	/* move to ECC position if needed */
	if (eccpos[0] != 0) {
		/* This only works on large pages
		 * because the ECC controller waits for
		 * NAND_CMD_RNDOUTSTART after the
		 * NAND_CMD_RNDOUT.
		 * anyway, for small pages, the eccpos[0] == 0
		 */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT,
				mtd->writesize + eccpos[0], -1);
	}

	/* the ECC controller needs to read the ECC just after the data */
	ecc_pos = oob + eccpos[0];
	chip->read_buf(mtd, ecc_pos, eccbytes);

	/* check if there's an error */
	stat = chip->ecc.correct(mtd, p, oob, NULL);

	if (stat < 0)
		mtd->ecc_stats.failed++;
	else
		mtd->ecc_stats.corrected += stat;

	/* get back to oob start (end of page) */
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);

	/* read the oob */
	chip->read_buf(mtd, oob, mtd->oobsize);

	return 0;
}

/*
 * HW ECC Correction
 *
 * function called after a read
 *
 * mtd:        MTD block structure
 * dat:        raw data read from the chip
 * read_ecc:   ECC from the chip (unused)
 * isnull:     unused
 *
 * Detect and correct a 1 bit error for a page
 */
static int atmel_nand_correct(struct mtd_info *mtd, u_char *dat,
		u_char *read_ecc, u_char *isnull)
{
	struct nand_chip *nand_chip = mtd->priv;
	unsigned int ecc_status, ecc_parity, ecc_mode;
	unsigned int ecc_word, ecc_bit;

	/* get the status from the Status Register */
	ecc_status = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, SR);

	/* if there's no error */
	if (likely(!(ecc_status & ATMEL_ECC_RECERR)))
		return 0;

	/* get error bit offset (4 bits) */
	ecc_bit = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_BITADDR;
	/* get word address (12 bits) */
	ecc_word = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_WORDADDR;
	ecc_word >>= 4;

	/* if there are multiple errors */
	if (ecc_status & ATMEL_ECC_MULERR) {
		/* check if it is a freshly erased block
		 * (filled with 0xff) */
		if ((ecc_bit == ATMEL_ECC_BITADDR)
				&& (ecc_word == (ATMEL_ECC_WORDADDR >> 4))) {
			/* the block has just been erased, return OK */
			return 0;
		}
		/* it doesn't seems to be a freshly
		 * erased block.
		 * We can't correct so many errors */
		printk(KERN_WARNING "atmel_nand : multiple errors detected."
				" Unable to correct.\n");
		return -EIO;
	}

	/* if there's a single bit error : we can correct it */
	if (ecc_status & ATMEL_ECC_ECCERR) {
		/* there's nothing much to do here.
		 * the bit error is on the ECC itself.
		 */
		printk(KERN_WARNING "atmel_nand : one bit error on ECC code."
				" Nothing to correct\n");
		return 0;
	}

	printk(KERN_WARNING "atmel_nand : one bit error on data."
			" (word offset in the page :"
			" 0x%x bit offset : 0x%x)\n",
			ecc_word, ecc_bit);
	/* correct the error */
	if (nand_chip->options & NAND_BUSWIDTH_16) {
		/* 16 bits words */
		((unsigned short *) dat)[ecc_word] ^= (1 << ecc_bit);
	} else {
		/* 8 bits words */
		dat[ecc_word] ^= (1 << ecc_bit);
	}
	printk(KERN_WARNING "atmel_nand : error corrected\n");
	return 1;
}

/*
 * Enable HW ECC : unused on most chips
 */
static void atmel_nand_hwctl(struct mtd_info *mtd, int mode)
{
}
#endif

static void at91_nand_hwcontrol(struct mtd_info *mtd,
					 int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
		IO_ADDR_W &= ~(CONFIG_SYS_NAND_MASK_ALE
			     | CONFIG_SYS_NAND_MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_ALE;

#ifdef CONFIG_SYS_NAND_ENABLE_PIN
		at91_set_gpio_value(CONFIG_SYS_NAND_ENABLE_PIN,
				    !(ctrl & NAND_NCE));
#endif
		this->IO_ADDR_W = (void *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SYS_NAND_READY_PIN
static int at91_nand_ready(struct mtd_info *mtd)
{
	return at91_get_gpio_value(CONFIG_SYS_NAND_READY_PIN);
}
#endif

int board_nand_init(struct nand_chip *nand)
{
#ifdef CONFIG_ATMEL_NAND_HWECC
	static int chip_nr = 0;
	struct mtd_info *mtd;
#endif

	nand->ecc.mode = NAND_ECC_SOFT;
#ifdef CONFIG_SYS_NAND_DBW_16
	nand->options = NAND_BUSWIDTH_16;
#endif
	nand->cmd_ctrl = at91_nand_hwcontrol;
#ifdef CONFIG_SYS_NAND_READY_PIN
	nand->dev_ready = at91_nand_ready;
#endif
	nand->chip_delay = 20;

#ifdef CONFIG_ATMEL_NAND_HWECC
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.calculate = atmel_nand_calculate;
	nand->ecc.correct = atmel_nand_correct;
	nand->ecc.hwctl = atmel_nand_hwctl;
	nand->ecc.read_page = atmel_nand_read_page;
	nand->ecc.bytes = 4;
#endif

#ifdef CONFIG_ATMEL_NAND_HWECC
	mtd = &nand_info[chip_nr++];
	mtd->priv = nand;

	/* Detect NAND chips */
	if (nand_scan_ident(mtd, 1, NULL)) {
		printk(KERN_WARNING "NAND Flash not found !\n");
		return -ENXIO;
	}

	if (nand->ecc.mode == NAND_ECC_HW) {
		/* ECC is calculated for the whole page (1 step) */
		nand->ecc.size = mtd->writesize;

		/* set ECC page size and oob layout */
		switch (mtd->writesize) {
		case 512:
			nand->ecc.layout = &atmel_oobinfo_small;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_528);
			break;
		case 1024:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_1056);
			break;
		case 2048:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_2112);
			break;
		case 4096:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_4224);
			break;
		default:
			/* page size not handled by HW ECC */
			/* switching back to soft ECC */
			nand->ecc.mode = NAND_ECC_SOFT;
			nand->ecc.calculate = NULL;
			nand->ecc.correct = NULL;
			nand->ecc.hwctl = NULL;
			nand->ecc.read_page = NULL;
			nand->ecc.postpad = 0;
			nand->ecc.prepad = 0;
			nand->ecc.bytes = 0;
			break;
		}
	}
#endif

	return 0;
}
