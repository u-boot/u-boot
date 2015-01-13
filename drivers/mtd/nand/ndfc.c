/*
 * Overview:
 *   Platform independend driver for NDFC (NanD Flash Controller)
 *   integrated into IBM/AMCC PPC4xx cores
 *
 * (C) Copyright 2006-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Based on original work by
 *	Thomas Gleixner
 *	Copyright 2006 IBM
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <linux/mtd/ndfc.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx.h>

#ifndef CONFIG_SYS_NAND_BCR
#define CONFIG_SYS_NAND_BCR 0x80002222
#endif
#ifndef CONFIG_SYS_NDFC_EBC0_CFG
#define CONFIG_SYS_NDFC_EBC0_CFG 0xb8400000
#endif

/*
 * We need to store the info, which chip-select (CS) is used for the
 * chip number. For example on Sequoia NAND chip #0 uses
 * CS #3.
 */
static int ndfc_cs[NDFC_MAX_BANKS];

static void ndfc_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		out_8((u8 *)(base + NDFC_CMD), cmd & 0xFF);
	else
		out_8((u8 *)(base + NDFC_ALE), cmd & 0xFF);
}

static int ndfc_dev_ready(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;

	return (in_be32((u32 *)(base + NDFC_STAT)) & NDFC_STAT_IS_READY);
}

static void ndfc_enable_hwecc(struct mtd_info *mtdinfo, int mode)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;
	u32 ccr;

	ccr = in_be32((u32 *)(base + NDFC_CCR));
	ccr |= NDFC_CCR_RESET_ECC;
	out_be32((u32 *)(base + NDFC_CCR), ccr);
}

static int ndfc_calculate_ecc(struct mtd_info *mtdinfo,
			      const u_char *dat, u_char *ecc_code)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;
	u32 ecc;
	u8 *p = (u8 *)&ecc;

	ecc = in_be32((u32 *)(base + NDFC_ECC));

	/* The NDFC uses Smart Media (SMC) bytes order
	 */
	ecc_code[0] = p[1];
	ecc_code[1] = p[2];
	ecc_code[2] = p[3];

	return 0;
}

/*
 * Speedups for buffer read/write/verify
 *
 * NDFC allows 32bit read/write of data. So we can speed up the buffer
 * functions. No further checking, as nand_base will always read/write
 * page aligned.
 */
static void ndfc_read_buf(struct mtd_info *mtdinfo, uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;
	uint32_t *p = (uint32_t *) buf;

	for (;len > 0; len -= 4)
		*p++ = in_be32((u32 *)(base + NDFC_DATA));
}

/*
 * Don't use these speedup functions in NAND boot image, since the image
 * has to fit into 4kByte.
 */
static void ndfc_write_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;
	uint32_t *p = (uint32_t *) buf;

	for (; len > 0; len -= 4)
		out_be32((u32 *)(base + NDFC_DATA), *p++);
}

#if defined(CONFIG_MTD_NAND_VERIFY_WRITE)
static int ndfc_verify_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xffffff00;
	uint32_t *p = (uint32_t *) buf;

	for (; len > 0; len -= 4)
		if (*p++ != in_be32((u32 *)(base + NDFC_DATA)))
			return -1;

	return 0;
}
#endif

/*
 * Read a byte from the NDFC.
 */
static uint8_t ndfc_read_byte(struct mtd_info *mtd)
{

	struct nand_chip *chip = mtd->priv;

#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
	return (uint8_t) readw(chip->IO_ADDR_R);
#else
	return readb(chip->IO_ADDR_R);
#endif

}

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	/*
	 * Don't use "chip" to address the NAND device,
	 * generate the cs from the address where it is encoded.
	 */
	ulong base = (ulong)nand->IO_ADDR_W & 0xffffff00;
	int cs = ndfc_cs[chip];

	/* Set NandFlash Core Configuration Register */
	/* 1 col x 2 rows */
	out_be32((u32 *)(base + NDFC_CCR), 0x00000000 | (cs << 24));
	out_be32((u32 *)(base + NDFC_BCFG0 + (cs << 2)), CONFIG_SYS_NAND_BCR);
}

static void ndfc_select_chip(struct mtd_info *mtd, int chip)
{
	/*
	 * Nothing to do here!
	 */
}

int board_nand_init(struct nand_chip *nand)
{
	int cs = (ulong)nand->IO_ADDR_W & 0x00000003;
	ulong base = (ulong)nand->IO_ADDR_W & 0xffffff00;
	static int chip = 0;

	/*
	 * Save chip-select for this chip #
	 */
	ndfc_cs[chip] = cs;

	/*
	 * Select required NAND chip in NDFC
	 */
	board_nand_select_device(nand, chip);

	nand->IO_ADDR_R = (void __iomem *)(base + NDFC_DATA);
	nand->IO_ADDR_W = (void __iomem *)(base + NDFC_DATA);
	nand->cmd_ctrl = ndfc_hwcontrol;
	nand->chip_delay = 50;
	nand->read_buf = ndfc_read_buf;
	nand->dev_ready = ndfc_dev_ready;
	nand->ecc.correct = nand_correct_data;
	nand->ecc.hwctl = ndfc_enable_hwecc;
	nand->ecc.calculate = ndfc_calculate_ecc;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = 256;
	nand->ecc.bytes = 3;
	nand->ecc.strength = 1;
	nand->select_chip = ndfc_select_chip;

#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
	nand->options |= NAND_BUSWIDTH_16;
#endif

	nand->write_buf  = ndfc_write_buf;
#if defined(CONFIG_MTD_NAND_VERIFY_WRITE)
	nand->verify_buf = ndfc_verify_buf;
#endif
	nand->read_byte = ndfc_read_byte;

	chip++;

	return 0;
}
