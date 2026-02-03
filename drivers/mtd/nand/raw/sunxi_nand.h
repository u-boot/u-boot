/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Bootlin, Richard GENOUD
 *
 * merged defines from sunxi_nand{,_spl}.c
 * Containing the following copyrights:
 * Copyright (C) 2013 Boris BREZILLON <b.brezillon.dev@gmail.com>
 * Copyright (C) 2015 Roy Spliet <r.spliet@ultimaker.com>
 * Copyright (c) 2014-2015, Antmicro Ltd <www.antmicro.com>
 * Copyright (c) 2015, AW-SOM Technologies <www.aw-som.com>
 * Derived from:
 *	https://github.com/yuq/sunxi-nfc-mtd
 *	Copyright (C) 2013 Qiang Yu <yuq825@gmail.com>
 *
 *	https://github.com/hno/Allwinner-Info
 *	Copyright (C) 2013 Henrik Nordström <Henrik Nordström>
 *
 *	Copyright (C) 2013 Dmitriy B. <rzk333@gmail.com>
 *	Copyright (C) 2013 Sergey Lapin <slapin@ossfans.org>
 *
 */

#ifndef SUNXI_NAND_H
#define SUNXI_NAND_H

#include <linux/bitops.h>

/* non compile-time field get/prep */
#define field_get(_mask, _reg) (((_reg) & (_mask)) >> (ffs(_mask) - 1))
#define field_prep(_mask, _val) (((_val) << (ffs(_mask) - 1)) & (_mask))

#define NFC_REG_CTL		0x0000
#define NFC_REG_ST		0x0004
#define NFC_REG_INT		0x0008
#define NFC_REG_TIMING_CTL	0x000C
#define NFC_REG_TIMING_CFG	0x0010
#define NFC_REG_ADDR_LOW	0x0014
#define NFC_REG_ADDR_HIGH	0x0018
#define NFC_REG_SECTOR_NUM	0x001C
#define NFC_REG_CNT		0x0020
#define NFC_REG_CMD		0x0024
#define NFC_REG_RCMD_SET	0x0028
#define NFC_REG_WCMD_SET	0x002C
#define NFC_REG_IO_DATA		0x0030
#define NFC_REG_ECC_CTL		0x0034
#define NFC_REG_ECC_ST		0x0038
#define NFC_REG_H6_PAT_FOUND	0x003C
#define NFC_REG_A10_ECC_ERR_CNT	0x0040
#define NFC_REG_H6_ECC_ERR_CNT	0x0050
#define NFC_REG_ECC_ERR_CNT(nfc, x)	(((nfc)->caps->reg_ecc_err_cnt + (x)) & ~0x3)
#define NFC_REG_A10_USER_DATA	0x0050
#define NFC_REG_H6_USER_DATA	0x0080
#define NFC_REG_H6_USER_DATA_LEN 0x0070
#define NFC_REG_USER_DATA(nfc, x)	((nfc)->caps->reg_user_data + ((x) * 4))

/* A USER_DATA_LEN register can hold the length of 8 USER_DATA registers */
#define NFC_REG_USER_DATA_LEN_CAPACITY 8
#define NFC_REG_USER_DATA_LEN(nfc, step) \
	((nfc)->caps->reg_user_data_len + \
	 ((step) / NFC_REG_USER_DATA_LEN_CAPACITY) * 4)
#define NFC_REG_SPARE_AREA(nfc) ((nfc)->caps->reg_spare_area)
#define NFC_REG_A10_SPARE_AREA	0x00A0
#define NFC_REG_H6_SPARE_AREA	0x0114
#define NFC_REG_PAT_ID(nfc)	((nfc)->caps->reg_pat_id)
#define NFC_REG_A10_PAT_ID	0x00A4
#define NFC_REG_H6_PAT_ID	0x0118
#define NFC_RAM0_BASE		0x0400
#define NFC_RAM1_BASE		0x0800

/* define bit use in NFC_CTL */
#define NFC_EN			BIT(0)
#define NFC_RESET		BIT(1)
#define NFC_BUS_WIDTH_MSK	BIT(2)
#define NFC_BUS_WIDTH_8		(0 << 2)
#define NFC_BUS_WIDTH_16	(1 << 2)
#define NFC_RB_SEL_MSK		BIT(3)
#define NFC_RB_SEL(x)		((x) << 3)
#define NFC_CE_SEL_MSK		(0x7 << 24)
#define NFC_CE_SEL(x)		((x) << 24)
#define NFC_CE_CTL		BIT(6)
#define NFC_PAGE_SHIFT_MSK	(0xf << 8)
#define NFC_PAGE_SHIFT(x)	(((x) < 10 ? 0 : (x) - 10) << 8)
#define NFC_PAGE_SIZE(a)	((fls(a) - 11) << 8)
#define NFC_SAM			BIT(12)
#define NFC_RAM_METHOD		BIT(14)
#define NFC_DEBUG_CTL		BIT(31)

/* define bit use in NFC_ST */
#define NFC_RB_B2R		BIT(0)
#define NFC_CMD_INT_FLAG	BIT(1)
#define NFC_DMA_INT_FLAG	BIT(2)
#define NFC_CMD_FIFO_STATUS	BIT(3)
#define NFC_STA			BIT(4)
#define NFC_NATCH_INT_FLAG	BIT(5)
#define NFC_RB_STATE(x)		BIT((x) + 8)

/* define bit use in NFC_INT */
#define NFC_B2R_INT_ENABLE	BIT(0)
#define NFC_CMD_INT_ENABLE	BIT(1)
#define NFC_DMA_INT_ENABLE	BIT(2)
#define NFC_INT_MASK		(NFC_B2R_INT_ENABLE | \
				 NFC_CMD_INT_ENABLE | \
				 NFC_DMA_INT_ENABLE)

/* define bit use in NFC_TIMING_CTL */
#define NFC_TIMING_CTL_EDO	BIT(8)

/* define NFC_TIMING_CFG register layout */
#define NFC_TIMING_CFG(tWB, tADL, tWHR, tRHW, tCAD)		\
	(((tWB) & 0x3) | (((tADL) & 0x3) << 2) |		\
	(((tWHR) & 0x3) << 4) | (((tRHW) & 0x3) << 6) |		\
	(((tCAD) & 0x7) << 8))

/* define bit use in NFC_CMD */
#define NFC_CMD_LOW_BYTE_MSK	0xff
#define NFC_CMD_HIGH_BYTE_MSK	(0xff << 8)
#define NFC_CMD(x)		(x)
#define NFC_ADR_NUM_OFFSET	16
#define NFC_ADR_NUM_MSK		(0x7 << NFC_ADR_NUM_OFFSET)
#define NFC_ADR_NUM(x)		(((x) - 1) << NFC_ADR_NUM_OFFSET)
#define NFC_SEND_ADR		BIT(19)
#define NFC_ACCESS_DIR		BIT(20)
#define NFC_DATA_TRANS		BIT(21)
#define NFC_SEND_CMD1		BIT(22)
#define NFC_WAIT_FLAG		BIT(23)
#define NFC_SEND_CMD2		BIT(24)
#define NFC_SEQ			BIT(25)
#define NFC_DATA_SWAP_METHOD	BIT(26)
#define NFC_ROW_AUTO_INC	BIT(27)
#define NFC_SEND_CMD3		BIT(28)
#define NFC_SEND_CMD4		BIT(29)
#define NFC_CMD_TYPE_MSK	(0x3 << 30)
#define NFC_NORMAL_OP		(0 << 30)
#define NFC_ECC_OP		(1 << 30)
#define NFC_PAGE_OP		(2 << 30)

/* define bit use in NFC_RCMD_SET */
#define NFC_READ_CMD_OFFSET	0
#define NFC_READ_CMD_MSK	(0xff << NFC_READ_CMD_OFFSET)
#define NFC_RND_READ_CMD0_OFFSET 8
#define NFC_RND_READ_CMD0_MSK	(0xff << NFC_RND_READ_CMD0_OFFSET)
#define NFC_RND_READ_CMD1_OFFSET 16
#define NFC_RND_READ_CMD1_MSK	(0xff << NFC_RND_READ_CMD1_OFFSET)

/* define bit use in NFC_WCMD_SET */
#define NFC_PROGRAM_CMD_MSK	0xff
#define NFC_RND_WRITE_CMD_MSK	(0xff << 8)
#define NFC_READ_CMD0_MSK	(0xff << 16)
#define NFC_READ_CMD1_MSK	(0xff << 24)

/* define bit use in NFC_ECC_CTL */
#define NFC_ECC_EN		BIT(0)
#define NFC_ECC_PIPELINE	BIT(3)
#define NFC_ECC_EXCEPTION	BIT(4)
#define NFC_ECC_BLOCK_512	BIT(5)
#define NFC_RANDOM_EN(nfc)	((nfc)->caps->random_en_mask)
#define NFC_ECC_MODE_MSK(nfc)	((nfc)->caps->ecc_mode_mask)
#define NFC_ECC_MODE(nfc, x)	field_prep(NFC_ECC_MODE_MSK(nfc), (x))
#define NFC_RANDOM_SEED_MSK	(0x7fff << 16)
#define NFC_RANDOM_SEED(x)	((x) << 16)

/* define bit use in NFC_ECC_ST */
#define NFC_ECC_ERR(x)		BIT(x)
#define NFC_ECC_ERR_MSK(nfc)	((nfc)->caps->ecc_err_mask)

/*
 * define bit use in NFC_REG_PAT_FOUND
 * For A10/A23, NFC_REG_PAT_FOUND == NFC_ECC_ST register
 */
#define NFC_ECC_PAT_FOUND(x)	BIT(x)
#define NFC_ECC_PAT_FOUND_MSK(nfc) ((nfc)->caps->pat_found_mask)

#define NFC_ECC_ERR_CNT(b, x)	(((x) >> ((b) * 8)) & 0xff)

#define NFC_USER_DATA_LEN_MSK(step) \
	(0xf << (((step) % NFC_REG_USER_DATA_LEN_CAPACITY) * 4))

#define NFC_DEFAULT_TIMEOUT_MS	1000

#define NFC_SRAM_SIZE		1024

#define NFC_MAX_CS		7

/*
 * NAND Controller capabilities structure: stores NAND controller capabilities
 * for distinction between compatible strings.
 *
 * @has_ecc_block_512:	If the ECC can handle 512B or only 1024B chuncks
 * @nstrengths:		Number of element of ECC strengths array
 * @ecc_strengths:	available ECC strengths array
 * @reg_ecc_err_cnt:	ECC error counter register
 * @reg_user_data:	User data register
 * @reg_user_data_len:	User data length register
 * @reg_spare_area:	Spare Area Register
 * @reg_pat_id:		Pattern ID Register
 * @reg_pat_found:	Data Pattern Status Register
 * @ecc_err_mask:	ERR_ERR mask in NFC_ECC_ST register
 * @pat_found_mask:	ECC_PAT_FOUND mask in NFC_REG_PAT_FOUND register
 * @ecc_mode_mask:	ECC_MODE mask in NFC_ECC_CTL register
 * @random_en_mask:	RANDOM_EN mask in NFC_ECC_CTL register
 * @user_data_len_tab:  Table of lenghts supported by USER_DATA_LEN register
 *			The table index is the value to set in NFC_USER_DATA_LEN
 *			registers, and the corresponding value is the number of
 *			bytes to write
 * @nuser_data_tab:	Size of @user_data_len_tab
 * @max_ecc_steps:	Maximum supported steps for ECC, this is also the
 *			number of user data registers
 */
struct sunxi_nfc_caps {
	bool has_ecc_block_512;
	unsigned int nstrengths;
	const u8 *ecc_strengths;
	unsigned int reg_ecc_err_cnt;
	unsigned int reg_user_data;
	unsigned int reg_user_data_len;
	unsigned int reg_spare_area;
	unsigned int reg_pat_id;
	unsigned int reg_pat_found;
	unsigned int pat_found_mask;
	unsigned int ecc_err_mask;
	unsigned int ecc_mode_mask;
	unsigned int random_en_mask;
	const u8 *user_data_len_tab;
	unsigned int nuser_data_tab;
	unsigned int max_ecc_steps;
};

#endif
