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

#if defined(CONFIG_CMD_NAND)
#if !defined(CFG_NAND_LEGACY)

#include <nand.h>
#include <s3c2410.h>

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

static void s3c2410_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *chip = mtd->priv;

	DEBUGN("hwcontrol(): 0x%02x: ", cmd);

	switch (cmd) {
	case NAND_CTL_SETNCE:
		NFCONF &= ~S3C2410_NFCONF_nFCE;
		DEBUGN("NFCONF=0x%08x\n", NFCONF);
		break;
	case NAND_CTL_CLRNCE:
		NFCONF |= S3C2410_NFCONF_nFCE;
		DEBUGN("NFCONF=0x%08x\n", NFCONF);
		break;
	case NAND_CTL_SETALE:
		chip->IO_ADDR_W = NF_BASE + 0x8;
		DEBUGN("SETALE\n");
		break;
	case NAND_CTL_SETCLE:
		chip->IO_ADDR_W = NF_BASE + 0x4;
		DEBUGN("SETCLE\n");
		break;
	default:
		chip->IO_ADDR_W = NF_BASE + 0xc;
		break;
	}
	return;
}

static int s3c2410_dev_ready(struct mtd_info *mtd)
{
	DEBUGN("dev_ready\n");
	return (NFSTAT & 0x01);
}

#ifdef CONFIG_S3C2410_NAND_HWECC
void s3c2410_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	DEBUGN("s3c2410_nand_enable_hwecc(%p, %d)\n", mtd ,mode);
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
	nand->IO_ADDR_R = nand->IO_ADDR_W = 0x4e00000c;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */

	/* hwcontrol always must be implemented */
	nand->hwcontrol = s3c2410_hwcontrol;

	nand->dev_ready = s3c2410_dev_ready;

#ifdef CONFIG_S3C2410_NAND_HWECC
	nand->enable_hwecc = s3c2410_nand_enable_hwecc;
	nand->calculate_ecc = s3c2410_nand_calculate_ecc;
	nand->correct_data = s3c2410_nand_correct_data;
	nand->eccmode = NAND_ECC_HW3_512;
#else
	nand->eccmode = NAND_ECC_SOFT;
#endif

#ifdef CONFIG_S3C2410_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT;
#else
	nand->options = 0;
#endif

	DEBUGN("end of nand_init\n");

	return 0;
}

#else
 #error "U-Boot legacy NAND support not available for S3C2410"
#endif
#endif
