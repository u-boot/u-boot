/*
 * Copyright 2009-2014 Freescale Semiconductor, Inc. and others
 *
 * Description: MPC5125, VF610, MCF54418 and Kinetis K70 Nand driver.
 * Ported to U-Boot by Stefan Agner
 * Based on RFC driver posted on Kernel Mailing list by Bill Pringlemeir
 * Jason ported to M54418TWR and MVFA5.
 * Authors: Stefan Agner <stefan.agner@toradex.com>
 *          Bill Pringlemeir <bpringlemeir@nbsps.com>
 *          Shaohui Xie <b21989@freescale.com>
 *          Jason Jin <Jason.jin@freescale.com>
 *
 * Based on original driver mpc5121_nfc.c.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Limitations:
 * - Untested on MPC5125 and M54418.
 * - DMA not used.
 * - 2K pages or less.
 * - Only 2K page w. 64+OOB and hardware ECC.
 */

#include <common.h>
#include <malloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <nand.h>
#include <errno.h>
#include <asm/io.h>

/* Register Offsets */
#define NFC_FLASH_CMD1			0x3F00
#define NFC_FLASH_CMD2			0x3F04
#define NFC_COL_ADDR			0x3F08
#define NFC_ROW_ADDR			0x3F0c
#define NFC_ROW_ADDR_INC		0x3F14
#define NFC_FLASH_STATUS1		0x3F18
#define NFC_FLASH_STATUS2		0x3F1c
#define NFC_CACHE_SWAP			0x3F28
#define NFC_SECTOR_SIZE			0x3F2c
#define NFC_FLASH_CONFIG		0x3F30
#define NFC_IRQ_STATUS			0x3F38

/* Addresses for NFC MAIN RAM BUFFER areas */
#define NFC_MAIN_AREA(n)		((n) *  0x1000)

#define PAGE_2K				0x0800
#define OOB_64				0x0040

/*
 * NFC_CMD2[CODE] values. See section:
 *  - 31.4.7 Flash Command Code Description, Vybrid manual
 *  - 23.8.6 Flash Command Sequencer, MPC5125 manual
 *
 * Briefly these are bitmasks of controller cycles.
 */
#define READ_PAGE_CMD_CODE		0x7EE0
#define PROGRAM_PAGE_CMD_CODE		0x7FC0
#define ERASE_CMD_CODE			0x4EC0
#define READ_ID_CMD_CODE		0x4804
#define RESET_CMD_CODE			0x4040
#define STATUS_READ_CMD_CODE		0x4068

/* NFC ECC mode define */
#define ECC_BYPASS			0
#define ECC_45_BYTE			6

/*** Register Mask and bit definitions */

/* NFC_FLASH_CMD1 Field */
#define CMD_BYTE2_MASK				0xFF000000
#define CMD_BYTE2_SHIFT				24

/* NFC_FLASH_CM2 Field */
#define CMD_BYTE1_MASK				0xFF000000
#define CMD_BYTE1_SHIFT				24
#define CMD_CODE_MASK				0x00FFFF00
#define CMD_CODE_SHIFT				8
#define BUFNO_MASK				0x00000006
#define BUFNO_SHIFT				1
#define START_BIT				(1<<0)

/* NFC_COL_ADDR Field */
#define COL_ADDR_MASK				0x0000FFFF
#define COL_ADDR_SHIFT				0

/* NFC_ROW_ADDR Field */
#define ROW_ADDR_MASK				0x00FFFFFF
#define ROW_ADDR_SHIFT				0
#define ROW_ADDR_CHIP_SEL_RB_MASK		0xF0000000
#define ROW_ADDR_CHIP_SEL_RB_SHIFT		28
#define ROW_ADDR_CHIP_SEL_MASK			0x0F000000
#define ROW_ADDR_CHIP_SEL_SHIFT			24

/* NFC_FLASH_STATUS2 Field */
#define STATUS_BYTE1_MASK			0x000000FF

/* NFC_FLASH_CONFIG Field */
#define CONFIG_ECC_SRAM_ADDR_MASK		0x7FC00000
#define CONFIG_ECC_SRAM_ADDR_SHIFT		22
#define CONFIG_ECC_SRAM_REQ_BIT			(1<<21)
#define CONFIG_DMA_REQ_BIT			(1<<20)
#define CONFIG_ECC_MODE_MASK			0x000E0000
#define CONFIG_ECC_MODE_SHIFT			17
#define CONFIG_FAST_FLASH_BIT			(1<<16)
#define CONFIG_16BIT				(1<<7)
#define CONFIG_BOOT_MODE_BIT			(1<<6)
#define CONFIG_ADDR_AUTO_INCR_BIT		(1<<5)
#define CONFIG_BUFNO_AUTO_INCR_BIT		(1<<4)
#define CONFIG_PAGE_CNT_MASK			0xF
#define CONFIG_PAGE_CNT_SHIFT			0

/* NFC_IRQ_STATUS Field */
#define IDLE_IRQ_BIT				(1<<29)
#define IDLE_EN_BIT				(1<<20)
#define CMD_DONE_CLEAR_BIT			(1<<18)
#define IDLE_CLEAR_BIT				(1<<17)

#define NFC_TIMEOUT	(1000)

/* ECC status placed at end of buffers. */
#define ECC_SRAM_ADDR	((PAGE_2K+256-8) >> 3)
#define ECC_STATUS_MASK	0x80
#define ECC_ERR_COUNT	0x3F

/*
 * ECC status is stored at NFC_CFG[ECCADD] +4 for little-endian
 * and +7 for big-endian SOC.
 */
#ifdef CONFIG_VF610
#define ECC_OFFSET	4
#else
#define ECC_OFFSET	7
#endif

struct vf610_nfc {
	struct mtd_info	  *mtd;
	struct nand_chip   chip;
	void __iomem	  *regs;
	uint               column;
	int                spareonly;
	int		   page_sz;
	int                page;
	/* Status and ID are in alternate locations. */
	int                alt_buf;
#define ALT_BUF_ID   1
#define ALT_BUF_STAT 2
	struct clk        *clk;
};

#define mtd_to_nfc(_mtd) \
	(struct vf610_nfc *)((struct nand_chip *)_mtd->priv)->priv

static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

static struct nand_ecclayout vf610_nfc_ecc45 = {
	.eccbytes = 45,
	.eccpos = {19, 20, 21, 22, 23,
		   24, 25, 26, 27, 28, 29, 30, 31,
		   32, 33, 34, 35, 36, 37, 38, 39,
		   40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55,
		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 8,
		 .length = 11} }
};

static inline u32 vf610_nfc_read(struct mtd_info *mtd, uint reg)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);

	return readl(nfc->regs + reg);
}

static inline void vf610_nfc_write(struct mtd_info *mtd, uint reg, u32 val)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);

	writel(val, nfc->regs + reg);
}

static inline void vf610_nfc_set(struct mtd_info *mtd, uint reg, u32 bits)
{
	vf610_nfc_write(mtd, reg, vf610_nfc_read(mtd, reg) | bits);
}

static inline void vf610_nfc_clear(struct mtd_info *mtd, uint reg, u32 bits)
{
	vf610_nfc_write(mtd, reg, vf610_nfc_read(mtd, reg) & ~bits);
}

static inline void vf610_nfc_set_field(struct mtd_info *mtd, u32 reg,
				       u32 mask, u32 shift, u32 val)
{
	vf610_nfc_write(mtd, reg,
			(vf610_nfc_read(mtd, reg) & (~mask)) | val << shift);
}

static inline void vf610_nfc_memcpy(void *dst, const void *src, size_t n)
{
	/*
	 * Use this accessor for the interal SRAM buffers. On ARM we can
	 * treat the SRAM buffer as if its memory, hence use memcpy
	 */
	memcpy(dst, src, n);
}

/* Clear flags for upcoming command */
static inline void vf610_nfc_clear_status(void __iomem *regbase)
{
	void __iomem *reg = regbase + NFC_IRQ_STATUS;
	u32 tmp = __raw_readl(reg);
	tmp |= CMD_DONE_CLEAR_BIT | IDLE_CLEAR_BIT;
	__raw_writel(tmp, reg);
}

/* Wait for complete operation */
static inline void vf610_nfc_done(struct mtd_info *mtd)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);
	uint start;

	/*
	 * Barrier is needed after this write. This write need
	 * to be done before reading the next register the first
	 * time.
	 * vf610_nfc_set implicates such a barrier by using writel
	 * to write to the register.
	 */
	vf610_nfc_set(mtd, NFC_FLASH_CMD2, START_BIT);

	start = get_timer(0);

	while (!(vf610_nfc_read(mtd, NFC_IRQ_STATUS) & IDLE_IRQ_BIT)) {
		if (get_timer(start) > NFC_TIMEOUT) {
			printf("Timeout while waiting for !BUSY.\n");
			return;
		}
	}
	vf610_nfc_clear_status(nfc->regs);
}

static u8 vf610_nfc_get_id(struct mtd_info *mtd, int col)
{
	u32 flash_id;

	if (col < 4) {
		flash_id = vf610_nfc_read(mtd, NFC_FLASH_STATUS1);
		return (flash_id >> (3-col)*8) & 0xff;
	} else {
		flash_id = vf610_nfc_read(mtd, NFC_FLASH_STATUS2);
		return flash_id >> 24;
	}
}

static u8 vf610_nfc_get_status(struct mtd_info *mtd)
{
	return vf610_nfc_read(mtd, NFC_FLASH_STATUS2) & STATUS_BYTE1_MASK;
}

/* Single command */
static void vf610_nfc_send_command(void __iomem *regbase, u32 cmd_byte1,
				   u32 cmd_code)
{
	void __iomem *reg = regbase + NFC_FLASH_CMD2;
	u32 tmp;
	vf610_nfc_clear_status(regbase);

	tmp = __raw_readl(reg);
	tmp &= ~(CMD_BYTE1_MASK | CMD_CODE_MASK | BUFNO_MASK);
	tmp |= cmd_byte1 << CMD_BYTE1_SHIFT;
	tmp |= cmd_code << CMD_CODE_SHIFT;
	__raw_writel(tmp, reg);
}

/* Two commands */
static void vf610_nfc_send_commands(void __iomem *regbase, u32 cmd_byte1,
			      u32 cmd_byte2, u32 cmd_code)
{
	void __iomem *reg = regbase + NFC_FLASH_CMD1;
	u32 tmp;
	vf610_nfc_send_command(regbase, cmd_byte1, cmd_code);

	tmp = __raw_readl(reg);
	tmp &= ~CMD_BYTE2_MASK;
	tmp |= cmd_byte2 << CMD_BYTE2_SHIFT;
	__raw_writel(tmp, reg);
}

static void vf610_nfc_addr_cycle(struct mtd_info *mtd, int column, int page)
{
	if (column != -1) {
		struct vf610_nfc *nfc = mtd_to_nfc(mtd);
		if (nfc->chip.options | NAND_BUSWIDTH_16)
			column = column/2;
		vf610_nfc_set_field(mtd, NFC_COL_ADDR, COL_ADDR_MASK,
				    COL_ADDR_SHIFT, column);
	}
	if (page != -1)
		vf610_nfc_set_field(mtd, NFC_ROW_ADDR, ROW_ADDR_MASK,
				    ROW_ADDR_SHIFT, page);
}

static inline void vf610_nfc_transfer_size(void __iomem *regbase, int size)
{
	__raw_writel(size, regbase + NFC_SECTOR_SIZE);
}

/* Send command to NAND chip */
static void vf610_nfc_command(struct mtd_info *mtd, unsigned command,
			      int column, int page)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);

	nfc->column     = max(column, 0);
	nfc->spareonly	= 0;
	nfc->alt_buf	= 0;

	switch (command) {
	case NAND_CMD_PAGEPROG:
		nfc->page = -1;
		vf610_nfc_transfer_size(nfc->regs, nfc->page_sz);
		vf610_nfc_send_commands(nfc->regs, NAND_CMD_SEQIN,
					command, PROGRAM_PAGE_CMD_CODE);
		vf610_nfc_addr_cycle(mtd, column, page);
		break;

	case NAND_CMD_RESET:
		vf610_nfc_transfer_size(nfc->regs, 0);
		vf610_nfc_send_command(nfc->regs, command, RESET_CMD_CODE);
		break;
	/*
	 * NFC does not support sub-page reads and writes,
	 * so emulate them using full page transfers.
	 */
	case NAND_CMD_READOOB:
		nfc->spareonly = 1;
	case NAND_CMD_SEQIN: /* Pre-read for partial writes. */
	case NAND_CMD_READ0:
		column = 0;
		/* Already read? */
		if (nfc->page == page)
			return;
		nfc->page = page;
		vf610_nfc_transfer_size(nfc->regs, nfc->page_sz);
		vf610_nfc_send_commands(nfc->regs, NAND_CMD_READ0,
					NAND_CMD_READSTART, READ_PAGE_CMD_CODE);
		vf610_nfc_addr_cycle(mtd, column, page);
		break;

	case NAND_CMD_ERASE1:
		nfc->page = -1;
		vf610_nfc_transfer_size(nfc->regs, 0);
		vf610_nfc_send_commands(nfc->regs, command,
					NAND_CMD_ERASE2, ERASE_CMD_CODE);
		vf610_nfc_addr_cycle(mtd, column, page);
		break;

	case NAND_CMD_READID:
		nfc->alt_buf = ALT_BUF_ID;
		vf610_nfc_transfer_size(nfc->regs, 0);
		vf610_nfc_send_command(nfc->regs, command, READ_ID_CMD_CODE);
		break;

	case NAND_CMD_STATUS:
		nfc->alt_buf = ALT_BUF_STAT;
		vf610_nfc_transfer_size(nfc->regs, 0);
		vf610_nfc_send_command(nfc->regs, command,
				       STATUS_READ_CMD_CODE);
		break;
	default:
		return;
	}

	vf610_nfc_done(mtd);
}

static inline void vf610_nfc_read_spare(struct mtd_info *mtd, void *buf,
					int len)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);

	len = min(mtd->oobsize, (uint)len);
	if (len > 0)
		vf610_nfc_memcpy(buf, nfc->regs + mtd->writesize, len);
}

/* Read data from NFC buffers */
static void vf610_nfc_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);
	uint c = nfc->column;
	uint l;

	/* Handle main area */
	if (!nfc->spareonly) {
		l = min((uint)len, mtd->writesize - c);
		nfc->column += l;

		if (!nfc->alt_buf)
			vf610_nfc_memcpy(buf, nfc->regs + NFC_MAIN_AREA(0) + c,
					 l);
		else
			if (nfc->alt_buf & ALT_BUF_ID)
				*buf = vf610_nfc_get_id(mtd, c);
			else
				*buf = vf610_nfc_get_status(mtd);

		buf += l;
		len -= l;
	}

	/* Handle spare area access */
	if (len) {
		nfc->column += len;
		vf610_nfc_read_spare(mtd, buf, len);
	}
}

/* Write data to NFC buffers */
static void vf610_nfc_write_buf(struct mtd_info *mtd, const u_char *buf,
				int len)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);
	uint c = nfc->column;
	uint l;

	l = min((uint)len, mtd->writesize + mtd->oobsize - c);
	nfc->column += l;
	vf610_nfc_memcpy(nfc->regs + NFC_MAIN_AREA(0) + c, buf, l);
}

/* Read byte from NFC buffers */
static u8 vf610_nfc_read_byte(struct mtd_info *mtd)
{
	u8 tmp;
	vf610_nfc_read_buf(mtd, &tmp, sizeof(tmp));
	return tmp;
}

/* Read word from NFC buffers */
static u16 vf610_nfc_read_word(struct mtd_info *mtd)
{
	u16 tmp;
	vf610_nfc_read_buf(mtd, (u_char *)&tmp, sizeof(tmp));
	return tmp;
}

/* If not provided, upper layers apply a fixed delay. */
static int vf610_nfc_dev_ready(struct mtd_info *mtd)
{
	/* NFC handles R/B internally; always ready.  */
	return 1;
}

/*
 * This function supports Vybrid only (MPC5125 would have full RB and four CS)
 */
static void vf610_nfc_select_chip(struct mtd_info *mtd, int chip)
{
#ifdef CONFIG_VF610
	u32 tmp = vf610_nfc_read(mtd, NFC_ROW_ADDR);
	tmp &= ~(ROW_ADDR_CHIP_SEL_RB_MASK | ROW_ADDR_CHIP_SEL_MASK);
	tmp |= 1 << ROW_ADDR_CHIP_SEL_RB_SHIFT;

	if (chip == 0)
		tmp |= 1 << ROW_ADDR_CHIP_SEL_SHIFT;
	else if (chip == 1)
		tmp |= 2 << ROW_ADDR_CHIP_SEL_SHIFT;

	vf610_nfc_write(mtd, NFC_ROW_ADDR, tmp);
#endif
}

/* Count the number of 0's in buff upto max_bits */
static inline int count_written_bits(uint8_t *buff, int size, int max_bits)
{
	uint32_t *buff32 = (uint32_t *)buff;
	int k, written_bits = 0;

	for (k = 0; k < (size / 4); k++) {
		written_bits += hweight32(~buff32[k]);
		if (written_bits > max_bits)
			break;
	}

	return written_bits;
}

static inline int vf610_nfc_correct_data(struct mtd_info *mtd, u_char *dat)
{
	struct vf610_nfc *nfc = mtd_to_nfc(mtd);
	u8 ecc_status;
	u8 ecc_count;
	int flip;

	ecc_status = __raw_readb(nfc->regs + ECC_SRAM_ADDR * 8 + ECC_OFFSET);
	ecc_count = ecc_status & ECC_ERR_COUNT;
	if (!(ecc_status & ECC_STATUS_MASK))
		return ecc_count;

	/* If 'ecc_count' zero or less then buffer is all 0xff or erased. */
	flip = count_written_bits(dat, nfc->chip.ecc.size, ecc_count);

	/* ECC failed. */
	if (flip > ecc_count) {
		nfc->page = -1;
		return -1;
	}

	/* Erased page. */
	memset(dat, 0xff, nfc->chip.ecc.size);
	return 0;
}


static int vf610_nfc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int eccsize = chip->ecc.size;
	int stat;
	uint8_t *p = buf;


	vf610_nfc_read_buf(mtd, p, eccsize);

	if (oob_required)
		vf610_nfc_read_buf(mtd, chip->oob_poi, mtd->oobsize);

	stat = vf610_nfc_correct_data(mtd, p);

	if (stat < 0)
		mtd->ecc_stats.failed++;
	else
		mtd->ecc_stats.corrected += stat;

	return 0;
}

/*
 * ECC will be calculated automatically
 */
static int vf610_nfc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			       const uint8_t *buf, int oob_required)
{
	vf610_nfc_write_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		vf610_nfc_write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

struct vf610_nfc_config {
	int hardware_ecc;
	int width;
	int flash_bbt;
};

static int vf610_nfc_nand_init(int devnum, void __iomem *addr)
{
	struct mtd_info *mtd = &nand_info[devnum];
	struct nand_chip *chip;
	struct vf610_nfc *nfc;
	int err = 0;
	struct vf610_nfc_config cfg = {
		.hardware_ecc = 1,
#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
		.width = 16,
#else
		.width = 8,
#endif
		.flash_bbt = 1,
	};

	nfc = malloc(sizeof(*nfc));
	if (!nfc) {
		printf(KERN_ERR "%s: Memory exhausted!\n", __func__);
		return -ENOMEM;
	}

	chip = &nfc->chip;
	nfc->regs = addr;

	mtd->priv = chip;
	chip->priv = nfc;

	if (cfg.width == 16) {
		chip->options |= NAND_BUSWIDTH_16;
		vf610_nfc_set(mtd, NFC_FLASH_CONFIG, CONFIG_16BIT);
	} else {
		chip->options &= ~NAND_BUSWIDTH_16;
		vf610_nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_16BIT);
	}

	/* Disable subpage writes as we do not provide ecc->hwctl */
	chip->options |= NAND_NO_SUBPAGE_WRITE;

	chip->dev_ready = vf610_nfc_dev_ready;
	chip->cmdfunc = vf610_nfc_command;
	chip->read_byte = vf610_nfc_read_byte;
	chip->read_word = vf610_nfc_read_word;
	chip->read_buf = vf610_nfc_read_buf;
	chip->write_buf = vf610_nfc_write_buf;
	chip->select_chip = vf610_nfc_select_chip;

	/* Bad block options. */
	if (cfg.flash_bbt)
		chip->bbt_options = NAND_BBT_USE_FLASH | NAND_BBT_CREATE;

	/* Default to software ECC until flash ID. */
	vf610_nfc_set_field(mtd, NFC_FLASH_CONFIG,
			    CONFIG_ECC_MODE_MASK,
			    CONFIG_ECC_MODE_SHIFT, ECC_BYPASS);

	chip->bbt_td = &bbt_main_descr;
	chip->bbt_md = &bbt_mirror_descr;

	nfc->page_sz = PAGE_2K + OOB_64;
	nfc->page_sz += cfg.width == 16 ? 1 : 0;

	/* Set configuration register. */
	vf610_nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_ADDR_AUTO_INCR_BIT);
	vf610_nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_BUFNO_AUTO_INCR_BIT);
	vf610_nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_BOOT_MODE_BIT);
	vf610_nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_DMA_REQ_BIT);
	vf610_nfc_set(mtd, NFC_FLASH_CONFIG, CONFIG_FAST_FLASH_BIT);

	/* Enable Idle IRQ */
	vf610_nfc_set(mtd, NFC_IRQ_STATUS, IDLE_EN_BIT);

	/* PAGE_CNT = 1 */
	vf610_nfc_set_field(mtd, NFC_FLASH_CONFIG, CONFIG_PAGE_CNT_MASK,
			    CONFIG_PAGE_CNT_SHIFT, 1);

	/* Set ECC_STATUS offset */
	vf610_nfc_set_field(mtd, NFC_FLASH_CONFIG,
			    CONFIG_ECC_SRAM_ADDR_MASK,
			    CONFIG_ECC_SRAM_ADDR_SHIFT, ECC_SRAM_ADDR);

	/* first scan to find the device and get the page size */
	if (nand_scan_ident(mtd, CONFIG_SYS_MAX_NAND_DEVICE, NULL)) {
		err = -ENXIO;
		goto error;
	}

	chip->ecc.mode = NAND_ECC_SOFT; /* default */

	nfc->page_sz = mtd->writesize + mtd->oobsize;

	/* Single buffer only, max 256 OOB minus ECC status */
	if (nfc->page_sz > PAGE_2K + 256 - 8) {
		dev_err(nfc->dev, "Unsupported flash size\n");
		err = -ENXIO;
		goto error;
	}
	nfc->page_sz += cfg.width == 16 ? 1 : 0;

	if (cfg.hardware_ecc) {
		if (mtd->writesize != PAGE_2K && mtd->oobsize < 64) {
			dev_err(nfc->dev, "Unsupported flash with hwecc\n");
			err = -ENXIO;
			goto error;
		}

		chip->ecc.layout = &vf610_nfc_ecc45;

		/* propagate ecc.layout to mtd_info */
		mtd->ecclayout = chip->ecc.layout;
		chip->ecc.read_page = vf610_nfc_read_page;
		chip->ecc.write_page = vf610_nfc_write_page;
		chip->ecc.mode = NAND_ECC_HW;

		chip->ecc.bytes = 45;
		chip->ecc.size = PAGE_2K;
		chip->ecc.strength = 24;

		/* set ECC mode to 45 bytes OOB with 24 bits correction */
		vf610_nfc_set_field(mtd, NFC_FLASH_CONFIG,
				    CONFIG_ECC_MODE_MASK,
				    CONFIG_ECC_MODE_SHIFT, ECC_45_BYTE);

		/* Enable ECC_STATUS */
		vf610_nfc_set(mtd, NFC_FLASH_CONFIG, CONFIG_ECC_SRAM_REQ_BIT);
	}

	/* second phase scan */
	err = nand_scan_tail(mtd);
	if (err)
		return err;

	err = nand_register(devnum);
	if (err)
		return err;

	return 0;

error:
	return err;
}

void board_nand_init(void)
{
	int err = vf610_nfc_nand_init(0, (void __iomem *)CONFIG_SYS_NAND_BASE);
	if (err)
		printf("VF610 NAND init failed (err %d)\n", err);
}
