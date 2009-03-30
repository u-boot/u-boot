/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#include <common.h>

#if 0
#define DEBUGN	printf
#else
#define DEBUGN(x, args ...) {}
#endif

#include <nand.h>
#include <s3c2410.h>
#include <asm/io.h>

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))

#define	NF_BASE		0x4e000000
#define	NFCONF		__REGi(NF_BASE + 0x0)
#define	NFCMD		__REGb(NF_BASE + 0x4)
#define	NFADDR		__REGb(NF_BASE + 0x8)
#define	NFDATA		__REGb(NF_BASE + 0xc)
#define	NFSTAT		__REGb(NF_BASE + 0x10)
#define NFECC0		__REGb(NF_BASE + 0x14)
#define NFECC1		__REGb(NF_BASE + 0x15)
#define NFECC2		__REGb(NF_BASE + 0x16)

#define S3C2410_NFCONF_EN          (1<<15)
#define S3C2410_NFCONF_512BYTE     (1<<14)
#define S3C2410_NFCONF_4STEP       (1<<13)
#define S3C2410_NFCONF_INITECC     (1<<12)
#define S3C2410_NFCONF_nFCE        (1<<11)
#define S3C2410_NFCONF_TACLS(x)    ((x)<<8)
#define S3C2410_NFCONF_TWRPH0(x)   ((x)<<4)
#define S3C2410_NFCONF_TWRPH1(x)   ((x)<<0)

#define S3C2410_ADDR_NALE 4
#define S3C2410_ADDR_NCLE 8

static void s3c2410_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

	DEBUGN("hwcontrol(): 0x%02x 0x%02x\n", cmd, ctrl);

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = NF_BASE;

		if (!(ctrl & NAND_CLE))
			IO_ADDR_W |= S3C2410_ADDR_NCLE;
		if (!(ctrl & NAND_ALE))
			IO_ADDR_W |= S3C2410_ADDR_NALE;

		chip->IO_ADDR_W = (void *)IO_ADDR_W;

		if (ctrl & NAND_NCE)
			NFCONF &= ~S3C2410_NFCONF_nFCE;
		else
			NFCONF |= S3C2410_NFCONF_nFCE;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
}

static int s3c2410_dev_ready(struct mtd_info *mtd)
{
	DEBUGN("dev_ready\n");
	return (NFSTAT & 0x01);
}

#ifdef CONFIG_S3C2410_NAND_HWECC
void s3c2410_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	DEBUGN("s3c2410_nand_enable_hwecc(%p, %d)\n", mtd, mode);
	NFCONF |= S3C2410_NFCONF_INITECC;
}

static int s3c2410_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				      u_char *ecc_code)
{
	ecc_code[0] = NFECC0;
	ecc_code[1] = NFECC1;
	ecc_code[2] = NFECC2;
	DEBUGN("s3c2410_nand_calculate_hwecc(%p,): 0x%02x 0x%02x 0x%02x\n",
		mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

static int s3c2410_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	if (read_ecc[0] == calc_ecc[0] &&
	    read_ecc[1] == calc_ecc[1] &&
	    read_ecc[2] == calc_ecc[2])
		return 0;

	printf("s3c2410_nand_correct_data: not implemented\n");
	return -1;
}
#endif

int board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg;
	u_int8_t tacls, twrph0, twrph1;
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

	DEBUGN("board_nand_init()\n");

	clk_power->CLKCON |= (1 << 4);

	/* initialize hardware */
	twrph0 = 3; twrph1 = 0; tacls = 0;

	cfg = S3C2410_NFCONF_EN;
	cfg |= S3C2410_NFCONF_TACLS(tacls - 1);
	cfg |= S3C2410_NFCONF_TWRPH0(twrph0 - 1);
	cfg |= S3C2410_NFCONF_TWRPH1(twrph1 - 1);

	NFCONF = cfg;

	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = nand->IO_ADDR_W = (void *)0x4e00000c;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */

	/* hwcontrol always must be implemented */
	nand->cmd_ctrl = s3c2410_hwcontrol;

	nand->dev_ready = s3c2410_dev_ready;

#ifdef CONFIG_S3C2410_NAND_HWECC
	nand->ecc.hwctl = s3c2410_nand_enable_hwecc;
	nand->ecc.calculate = s3c2410_nand_calculate_ecc;
	nand->ecc.correct = s3c2410_nand_correct_data;
	nand->ecc.mode = NAND_ECC_HW3_512;
#else
	nand->ecc.mode = NAND_ECC_SOFT;
#endif

#ifdef CONFIG_S3C2410_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT;
#else
	nand->options = 0;
#endif

	DEBUGN("end of nand_init\n");

	return 0;
}
