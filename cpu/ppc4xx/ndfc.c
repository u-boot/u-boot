/*
 * Overview:
 *   Platform independend driver for NDFC (NanD Flash Controller)
 *   integrated into EP440 cores
 *
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Based on original work by
 *	Thomas Gleixner
 *	Copyright 2006 IBM
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && !defined(CFG_NAND_LEGACY) && \
	(defined(CONFIG_440EP) || defined(CONFIG_440GR) ||	     \
	 defined(CONFIG_440EPX) || defined(CONFIG_440GRX))

#include <nand.h>
#include <linux/mtd/ndfc.h>
#include <asm/processor.h>
#include <ppc440.h>

static u8 hwctl = 0;

static void ndfc_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	switch (cmd) {
	case NAND_CTL_SETCLE:
		hwctl |= 0x1;
		break;

	case NAND_CTL_CLRCLE:
		hwctl &= ~0x1;
		break;

	case NAND_CTL_SETALE:
		hwctl |= 0x2;
		break;

	case NAND_CTL_CLRALE:
		hwctl &= ~0x2;
		break;
	}
}

static void ndfc_write_byte(struct mtd_info *mtdinfo, u_char byte)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;

	if (hwctl & 0x1)
		out8(base + NDFC_CMD, byte);
	else if (hwctl & 0x2)
		out8(base + NDFC_ALE, byte);
	else
		out8(base + NDFC_DATA, byte);
}

static u_char ndfc_read_byte(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;

	return (in8(base + NDFC_DATA));
}

static int ndfc_dev_ready(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;

	while (!(in32(base + NDFC_STAT) & NDFC_STAT_IS_READY))
		;

	return 1;
}

#ifndef CONFIG_NAND_SPL
/*
 * Don't use these speedup functions in NAND boot image, since the image
 * has to fit into 4kByte.
 */

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
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;
	uint32_t *p = (uint32_t *) buf;

	for (;len > 0; len -= 4)
		*p++ = in32(base + NDFC_DATA);
}

static void ndfc_write_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;
	uint32_t *p = (uint32_t *) buf;

	for (; len > 0; len -= 4)
		out32(base + NDFC_DATA, *p++);
}

static int ndfc_verify_buf(struct mtd_info *mtdinfo, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W & 0xfffffffc;
	uint32_t *p = (uint32_t *) buf;

	for (; len > 0; len -= 4)
		if (*p++ != in32(base + NDFC_DATA))
			return -1;

	return 0;
}
#endif /* #ifndef CONFIG_NAND_SPL */

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	/*
	 * Don't use "chip" to address the NAND device,
	 * generate the cs from the address where it is encoded.
	 */
	int cs = (ulong)nand->IO_ADDR_W & 0x00000003;
	ulong base = (ulong)nand->IO_ADDR_W & 0xfffffffc;

	/* Set NandFlash Core Configuration Register */
	/* 1col x 2 rows */
	out32(base + NDFC_CCR, 0x00000000 | (cs << 24));
}

void board_nand_init(struct nand_chip *nand)
{
	int cs = (ulong)nand->IO_ADDR_W & 0x00000003;
	ulong base = (ulong)nand->IO_ADDR_W & 0xfffffffc;

	nand->eccmode = NAND_ECC_SOFT;

	nand->hwcontrol  = ndfc_hwcontrol;
	nand->read_byte  = ndfc_read_byte;
	nand->write_byte = ndfc_write_byte;
	nand->dev_ready  = ndfc_dev_ready;

#ifndef CONFIG_NAND_SPL
	nand->write_buf  = ndfc_write_buf;
	nand->read_buf   = ndfc_read_buf;
	nand->verify_buf = ndfc_verify_buf;
#else
	/*
	 * Setup EBC (CS0 only right now)
	 */
	mtdcr(ebccfga, xbcfg);
	mtdcr(ebccfgd, 0xb8400000);

	mtebc(pb0cr, CFG_EBC_PB0CR);
	mtebc(pb0ap, CFG_EBC_PB0AP);
#endif

	/*
	 * Select required NAND chip in NDFC
	 */
	board_nand_select_device(nand, cs);
	out32(base + NDFC_BCFG0 + (cs << 2), 0x80002222);
}

#endif
