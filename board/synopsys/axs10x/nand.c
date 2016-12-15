/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bouncebuf.h>
#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <asm/io.h>
#include "axs10x.h"

DECLARE_GLOBAL_DATA_PTR;

#define BUS_WIDTH	8		/* AXI data bus width in bytes	*/

/* DMA buffer descriptor bits & masks */
#define BD_STAT_OWN			(1 << 31)
#define BD_STAT_BD_FIRST		(1 << 3)
#define BD_STAT_BD_LAST			(1 << 2)
#define BD_SIZES_BUFFER1_MASK		0xfff

#define BD_STAT_BD_COMPLETE	(BD_STAT_BD_FIRST | BD_STAT_BD_LAST)

/* Controller command flags */
#define B_WFR		(1 << 19)	/* 1b - Wait for ready		*/
#define B_LC		(1 << 18)	/* 1b - Last cycle		*/
#define B_IWC		(1 << 13)	/* 1b - Interrupt when complete	*/

/* NAND cycle types */
#define B_CT_ADDRESS	(0x0 << 16)	/* Address operation		*/
#define B_CT_COMMAND	(0x1 << 16)	/* Command operation		*/
#define B_CT_WRITE	(0x2 << 16)	/* Write operation		*/
#define B_CT_READ	(0x3 << 16)	/* Write operation		*/

enum nand_isr_t {
	NAND_ISR_DATAREQUIRED = 0,
	NAND_ISR_TXUNDERFLOW,
	NAND_ISR_TXOVERFLOW,
	NAND_ISR_DATAAVAILABLE,
	NAND_ISR_RXUNDERFLOW,
	NAND_ISR_RXOVERFLOW,
	NAND_ISR_TXDMACOMPLETE,
	NAND_ISR_RXDMACOMPLETE,
	NAND_ISR_DESCRIPTORUNAVAILABLE,
	NAND_ISR_CMDDONE,
	NAND_ISR_CMDAVAILABLE,
	NAND_ISR_CMDERROR,
	NAND_ISR_DATATRANSFEROVER,
	NAND_ISR_NONE
};

enum nand_regs_t {
	AC_FIFO = 0,		/* address and command fifo */
	IDMAC_BDADDR = 0x18,	/* idmac descriptor list base address */
	INT_STATUS = 0x118,	/* interrupt status register */
	INT_CLR_STATUS = 0x120,	/* interrupt clear status register */
};

struct nand_bd {
	uint32_t status;	/* DES0 */
	uint32_t sizes;		/* DES1 */
	uint32_t buffer_ptr0;	/* DES2 */
	uint32_t buffer_ptr1;	/* DES3 */
};

#define NAND_REG_WRITE(r, v)	\
	writel(v, (volatile void __iomem *)(CONFIG_SYS_NAND_BASE + r))
#define NAND_REG_READ(r)		\
	readl((const volatile void __iomem *)(CONFIG_SYS_NAND_BASE + r))

static struct nand_bd *bd;	/* DMA buffer descriptors	*/

/**
 * axs101_nand_write_buf -  write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 */
static uint32_t nand_flag_is_set(uint32_t flag)
{
	uint32_t reg = NAND_REG_READ(INT_STATUS);

	if (reg & (1 << NAND_ISR_CMDERROR))
		return 0;

	if (reg & (1 << flag)) {
		NAND_REG_WRITE(INT_CLR_STATUS, 1 << flag);
		return 1;
	}

	return 0;
}

/**
 * axs101_nand_write_buf -  write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 */
static void axs101_nand_write_buf(struct mtd_info *mtd, const u_char *buf,
				   int len)
{
	struct bounce_buffer bbstate;

	bounce_buffer_start(&bbstate, (void *)buf, len, GEN_BB_READ);

	/* Setup buffer descriptor */
	writel(BD_STAT_OWN | BD_STAT_BD_COMPLETE, &bd->status);
	writel(ALIGN(len, BUS_WIDTH) & BD_SIZES_BUFFER1_MASK, &bd->sizes);
	writel(bbstate.bounce_buffer, &bd->buffer_ptr0);
	writel(0, &bd->buffer_ptr1);

	/* Flush modified buffer descriptor */
	flush_dcache_range((unsigned long)bd,
			   (unsigned long)bd + sizeof(struct nand_bd));

	/* Issue "write" command */
	NAND_REG_WRITE(AC_FIFO, B_CT_WRITE | B_WFR | B_IWC | B_LC | (len-1));

	/* Wait for NAND command and DMA to complete */
	while (!nand_flag_is_set(NAND_ISR_CMDDONE))
		;
	while (!nand_flag_is_set(NAND_ISR_TXDMACOMPLETE))
		;

	bounce_buffer_stop(&bbstate);
}

/**
 * axs101_nand_read_buf -  read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store data
 * @len:	number of bytes to read
 */
static void axs101_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct bounce_buffer bbstate;

	bounce_buffer_start(&bbstate, buf, len, GEN_BB_WRITE);

	/* Setup buffer descriptor */
	writel(BD_STAT_OWN | BD_STAT_BD_COMPLETE, &bd->status);
	writel(ALIGN(len, BUS_WIDTH) & BD_SIZES_BUFFER1_MASK, &bd->sizes);
	writel(bbstate.bounce_buffer, &bd->buffer_ptr0);
	writel(0, &bd->buffer_ptr1);

	/* Flush modified buffer descriptor */
	flush_dcache_range((unsigned long)bd,
			   (unsigned long)bd + sizeof(struct nand_bd));

	/* Issue "read" command */
	NAND_REG_WRITE(AC_FIFO, B_CT_READ | B_WFR | B_IWC | B_LC | (len - 1));

	/* Wait for NAND command and DMA to complete */
	while (!nand_flag_is_set(NAND_ISR_CMDDONE))
		;
	while (!nand_flag_is_set(NAND_ISR_RXDMACOMPLETE))
		;

	bounce_buffer_stop(&bbstate);
}

/**
 * axs101_nand_read_byte -  read one byte from the chip
 * @mtd:	MTD device structure
 */
static u_char axs101_nand_read_byte(struct mtd_info *mtd)
{
	u8 byte;

	axs101_nand_read_buf(mtd, (uchar *)&byte, sizeof(byte));
	return byte;
}

/**
 * axs101_nand_read_word -  read one word from the chip
 * @mtd:	MTD device structure
 */
static u16 axs101_nand_read_word(struct mtd_info *mtd)
{
	u16 word;

	axs101_nand_read_buf(mtd, (uchar *)&word, sizeof(word));
	return word;
}

/**
 * axs101_nand_hwcontrol - NAND control functions wrapper.
 * @mtd:	MTD device structure
 * @cmd:	Command
 */
static void axs101_nand_hwcontrol(struct mtd_info *mtdinfo, int cmd,
				   unsigned int ctrl)
{
	if (cmd == NAND_CMD_NONE)
		return;

	cmd = cmd & 0xff;

	switch (ctrl & (NAND_ALE | NAND_CLE)) {
	/* Address */
	case NAND_ALE:
		cmd |= B_CT_ADDRESS;
		break;

	/* Command */
	case NAND_CLE:
		cmd |= B_CT_COMMAND | B_WFR;

		break;

	default:
		debug("%s: unknown ctrl %#x\n", __func__, ctrl);
	}

	NAND_REG_WRITE(AC_FIFO, cmd | B_LC);
	while (!nand_flag_is_set(NAND_ISR_CMDDONE))
		;
}

int board_nand_init(struct nand_chip *nand)
{
	bd = (struct nand_bd *)memalign(ARCH_DMA_MINALIGN,
					sizeof(struct nand_bd));

	/* Set buffer descriptor address in IDMAC */
	NAND_REG_WRITE(IDMAC_BDADDR, bd);

	nand->ecc.mode = NAND_ECC_SOFT;
	nand->cmd_ctrl = axs101_nand_hwcontrol;
	nand->read_byte = axs101_nand_read_byte;
	nand->read_word = axs101_nand_read_word;
	nand->write_buf = axs101_nand_write_buf;
	nand->read_buf = axs101_nand_read_buf;

	/* MBv3 has NAND IC with 16-bit data bus */
	if (gd->board_type == AXS_MB_V3)
		nand->options |= NAND_BUSWIDTH_16;

	return 0;
}
