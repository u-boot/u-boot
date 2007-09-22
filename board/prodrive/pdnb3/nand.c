/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#if defined(CONFIG_CMD_NAND)

#include <nand.h>

struct pdnb3_ndfc_regs {
	uchar cmd;
	uchar wait;
	uchar addr;
	uchar term;
	uchar data;
};

static u8 hwctl;
static struct pdnb3_ndfc_regs *pdnb3_ndfc;

#define readb(addr)	*(volatile u_char *)(addr)
#define readl(addr)	*(volatile u_long *)(addr)
#define writeb(d,addr)	*(volatile u_char *)(addr) = (d)

/*
 * The PDNB3 has a NAND Flash Controller (NDFC) that handles all accesses to
 * the NAND devices.  The NDFC has command, address and data registers that
 * when accessed will set up the NAND flash pins appropriately.  We'll use the
 * hwcontrol function to save the configuration in a global variable.
 * We can then use this information in the read and write functions to
 * determine which NDFC register to access.
 *
 * There is one NAND devices on the board, a Hynix HY27US08561A (32 MByte).
 */
static void pdnb3_nand_hwcontrol(struct mtd_info *mtd, int cmd)
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

	case NAND_CTL_SETNCE:
		break;
	case NAND_CTL_CLRNCE:
		writeb(0x00, &(pdnb3_ndfc->term));
		break;
	}
}

static void pdnb3_nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	if (hwctl & 0x1)
		writeb(byte, &(pdnb3_ndfc->cmd));
	else if (hwctl & 0x2)
		writeb(byte, &(pdnb3_ndfc->addr));
	else
		writeb(byte, &(pdnb3_ndfc->data));
}

static u_char pdnb3_nand_read_byte(struct mtd_info *mtd)
{
	return readb(&(pdnb3_ndfc->data));
}

static void pdnb3_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (hwctl & 0x1)
			writeb(buf[i], &(pdnb3_ndfc->cmd));
		else if (hwctl & 0x2)
			writeb(buf[i], &(pdnb3_ndfc->addr));
		else
			writeb(buf[i], &(pdnb3_ndfc->data));
	}
}

static void pdnb3_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;

	if (len % 4) {
		for (i = 0; i < len; i++)
			buf[i] = readb(&(pdnb3_ndfc->data));
	} else {
		ulong *ptr = (ulong *)buf;
		int count = len >> 2;

		for (i = 0; i < count; i++)
			*ptr++ = readl(&(pdnb3_ndfc->data));
	}
}

static int pdnb3_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (buf[i] != readb(&(pdnb3_ndfc->data)))
			return i;

	return 0;
}

static int pdnb3_nand_dev_ready(struct mtd_info *mtd)
{
	volatile u_char val;

	/*
	 * Blocking read to wait for NAND to be ready
	 */
	val = readb(&(pdnb3_ndfc->wait));

	/*
	 * Return always true
	 */
	return 1;
}

int board_nand_init(struct nand_chip *nand)
{
	pdnb3_ndfc = (struct pdnb3_ndfc_regs *)CFG_NAND_BASE;

	nand->eccmode = NAND_ECC_SOFT;

	/* Set address of NAND IO lines (Using Linear Data Access Region) */
	nand->IO_ADDR_R = (void __iomem *) ((ulong) pdnb3_ndfc + 0x4);
	nand->IO_ADDR_W = (void __iomem *) ((ulong) pdnb3_ndfc + 0x4);
	/* Reference hardware control function */
	nand->hwcontrol  = pdnb3_nand_hwcontrol;
	/* Set command delay time */
	nand->hwcontrol  = pdnb3_nand_hwcontrol;
	nand->write_byte = pdnb3_nand_write_byte;
	nand->read_byte  = pdnb3_nand_read_byte;
	nand->write_buf  = pdnb3_nand_write_buf;
	nand->read_buf   = pdnb3_nand_read_buf;
	nand->verify_buf = pdnb3_nand_verify_buf;
	nand->dev_ready  = pdnb3_nand_dev_ready;
	return 0;
}
#endif
