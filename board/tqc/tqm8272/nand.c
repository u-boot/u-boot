/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ioports.h>
#include <mpc8260.h>

#include "tqm8272.h"

/* UPM pattern for bus clock = 66.7 MHz */
static const uint upmTable67[] =
{
    /* Offset	UPM Read Single RAM array entry -> NAND Read Data */
    /* 0x00 */	0x0fa3f100, 0x0fa3b000, 0x0fa33100, 0x0fa33000,
    /* 0x04 */	0x0fa33000, 0x0fa33004, 0xfffffc01, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x08 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x0C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x10 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x14 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Write Single RAM array entry -> NAND Write Data, ADDR and CMD */
    /* 0x18 */	0x00a3fc00, 0x00a3fc00, 0x00a3fc00, 0x00a3fc00,
    /* 0x1C */	0x0fa3fc00, 0x0fa3fc04, 0xfffffc01, 0xfffffc00,

		/* UPM Write Burst RAM array entry -> unused */
    /* 0x20 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x24 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x28 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x2C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Refresh Timer RAM array entry -> unused */
    /* 0x30 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x34 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x38 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Exception RAM array entry -> unsused */
    /* 0x3C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 100 MHz */
static const uint upmTable100[] =
{
    /* Offset	UPM Read Single RAM array entry -> NAND Read Data */
    /* 0x00 */	0x0fa3f200, 0x0fa3b000, 0x0fa33300, 0x0fa33000,
    /* 0x04 */	0x0fa33000, 0x0fa33004, 0xfffffc01, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x08 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x0C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x10 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x14 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Write Single RAM array entry -> NAND Write Data, ADDR and CMD */
    /* 0x18 */	0x00a3ff00, 0x00a3fc00, 0x00a3fc00, 0x0fa3fc00,
    /* 0x1C */	0x0fa3fc00, 0x0fa3fc04, 0xfffffc01, 0xfffffc00,

		/* UPM Write Burst RAM array entry -> unused */
    /* 0x20 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x24 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x28 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x2C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Refresh Timer RAM array entry -> unused */
    /* 0x30 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x34 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x38 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Exception RAM array entry -> unsused */
    /* 0x3C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 133.3 MHz */
static const uint upmTable133[] =
{
    /* Offset	UPM Read Single RAM array entry -> NAND Read Data */
    /* 0x00 */	0x0fa3f300, 0x0fa3b000, 0x0fa33300, 0x0fa33000,
    /* 0x04 */	0x0fa33200, 0x0fa33004, 0xfffffc01, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x08 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x0C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Read Burst RAM array entry -> unused */
    /* 0x10 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x14 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

		/* UPM Write Single RAM array entry -> NAND Write Data, ADDR and CMD */
    /* 0x18 */	0x00a3ff00, 0x00a3fc00, 0x00a3fd00, 0x0fa3fc00,
    /* 0x1C */	0x0fa3fd00, 0x0fa3fc04, 0xfffffc01, 0xfffffc00,

		/* UPM Write Burst RAM array entry -> unused */
    /* 0x20 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x24 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x28 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x2C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Refresh Timer RAM array entry -> unused */
    /* 0x30 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x34 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
    /* 0x38 */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

		/* UPM Exception RAM array entry -> unsused */
    /* 0x3C */	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

static int	chipsel = 0;

#if defined(CONFIG_CMD_NAND)

#include <nand.h>
#include <linux/mtd/mtd.h>

static u8 hwctl = 0;

static void upmnand_write_byte(struct mtd_info *mtdinfo, u_char byte)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) (this->IO_ADDR_W + chipsel * CONFIG_SYS_NAND_CS_DIST);

	if (hwctl & 0x1) {
		WRITE_NAND_UPM(byte, base, CONFIG_SYS_NAND_UPM_WRITE_CMD_OFS);
	} else if (hwctl & 0x2) {
		WRITE_NAND_UPM(byte, base, CONFIG_SYS_NAND_UPM_WRITE_ADDR_OFS);
	} else {
		WRITE_NAND(byte, base);
	}
}

static void upmnand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			hwctl |= 0x1;
		else
			hwctl &= ~0x1;
		if ( ctrl & NAND_ALE )
			hwctl |= 0x2;
		else
			hwctl &= ~0x2;
	}
	if (cmd != NAND_CMD_NONE)
		upmnand_write_byte (mtd, cmd);
}

static u_char upmnand_read_byte(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) (this->IO_ADDR_W + chipsel * CONFIG_SYS_NAND_CS_DIST);

	return READ_NAND(base);
}

static int tqm8272_dev_ready(struct mtd_info *mtdinfo)
{
	/* constant delay (see also tR in the datasheet) */
	udelay(12); \
	return 1;
}

#ifndef CONFIG_NAND_SPL
static void tqm8272_read_buf(struct mtd_info *mtdinfo, uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	unsigned char *base = (unsigned char *) (this->IO_ADDR_W + chipsel * CONFIG_SYS_NAND_CS_DIST);
	int	i;

	for (i = 0; i< len; i++)
		buf[i] = *base;
}

static void tqm8272_write_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	unsigned char *base = (unsigned char *) (this->IO_ADDR_W + chipsel * CONFIG_SYS_NAND_CS_DIST);
	int	i;

	for (i = 0; i< len; i++)
		*base = buf[i];
}

static int tqm8272_verify_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	unsigned char *base = (unsigned char *) (this->IO_ADDR_W + chipsel * CONFIG_SYS_NAND_CS_DIST);
	int	i;

	for (i = 0; i < len; i++)
		if (buf[i] != *base)
			return -1;
	return 0;
}
#endif /* #ifndef CONFIG_NAND_SPL */

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	chipsel = chip;
}

int board_nand_init(struct nand_chip *nand)
{
	static	int	UpmInit = 0;
	volatile immap_t * immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immr->im_memctl;

	if (hwinf.nand == 0) return -1;

	/* Setup the UPM */
	if (UpmInit == 0) {
		switch (hwinf.busclk_real) {
		case 100000000:
			upmconfig (UPMB, (uint *) upmTable100,
			   sizeof (upmTable100) / sizeof (uint));
			break;
		case 133333333:
			upmconfig (UPMB, (uint *) upmTable133,
			   sizeof (upmTable133) / sizeof (uint));
			break;
		default:
			upmconfig (UPMB, (uint *) upmTable67,
			   sizeof (upmTable67) / sizeof (uint));
			break;
		}
		UpmInit = 1;
	}

	/* Setup the memctrl */
	memctl->memc_or3 = CONFIG_SYS_NAND_OR;
	memctl->memc_br3 = CONFIG_SYS_NAND_BR;
	memctl->memc_mbmr = (MxMR_OP_NORM);

	nand->ecc.mode = NAND_ECC_SOFT;

	nand->cmd_ctrl	 = upmnand_hwcontrol;
	nand->read_byte	 = upmnand_read_byte;
	nand->dev_ready	 = tqm8272_dev_ready;

#ifndef CONFIG_NAND_SPL
	nand->write_buf	 = tqm8272_write_buf;
	nand->read_buf	 = tqm8272_read_buf;
	nand->verify_buf = tqm8272_verify_buf;
#endif

	/*
	 * Select required NAND chip
	 */
	board_nand_select_device(nand, 0);
	return 0;
}

#endif
