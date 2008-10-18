/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
 *
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

#include <asm/processor.h>
#include <nand.h>

struct alpr_ndfc_regs {
	u8 cmd[4];
	u8 addr_wait;
	u8 term;
	u8 dummy;
	u8 dummy2;
	u8 data;
};

static u8 hwctl;
static struct alpr_ndfc_regs *alpr_ndfc = NULL;

#define readb(addr)	(u8)(*(volatile u8 *)(addr))
#define writeb(d,addr)	*(volatile u8 *)(addr) = ((u8)(d))

/*
 * The ALPR has a NAND Flash Controller (NDFC) that handles all accesses to
 * the NAND devices.  The NDFC has command, address and data registers that
 * when accessed will set up the NAND flash pins appropriately.  We'll use the
 * hwcontrol function to save the configuration in a global variable.
 * We can then use this information in the read and write functions to
 * determine which NDFC register to access.
 *
 * There are 2 NAND devices on the board, a Hynix HY27US08561A (1 GByte).
 */
static void alpr_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			hwctl |= 0x1;
		else
			hwctl &= ~0x1;
		if ( ctrl & NAND_ALE )
			hwctl |= 0x2;
		else
			hwctl &= ~0x2;
		if ( (ctrl & NAND_NCE) != NAND_NCE)
			writeb(0x00, &(alpr_ndfc->term));
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static u_char alpr_nand_read_byte(struct mtd_info *mtd)
{
	return readb(&(alpr_ndfc->data));
}

static void alpr_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	struct nand_chip *nand = mtd->priv;
	int i;

	for (i = 0; i < len; i++) {
		if (hwctl & 0x1)
			 /*
			  * IO_ADDR_W used as CMD[i] reg to support multiple NAND
			  * chips.
			  */
			writeb(buf[i], nand->IO_ADDR_W);
		else if (hwctl & 0x2)
			writeb(buf[i], &(alpr_ndfc->addr_wait));
		else
			writeb(buf[i], &(alpr_ndfc->data));
	}
}

static void alpr_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		buf[i] = readb(&(alpr_ndfc->data));
	}
}

static int alpr_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (buf[i] != readb(&(alpr_ndfc->data)))
			return i;

	return 0;
}

static int alpr_nand_dev_ready(struct mtd_info *mtd)
{
	volatile u_char val;

	/*
	 * Blocking read to wait for NAND to be ready
	 */
	val = readb(&(alpr_ndfc->addr_wait));

	/*
	 * Return always true
	 */
	return 1;
}

int board_nand_init(struct nand_chip *nand)
{
	alpr_ndfc = (struct alpr_ndfc_regs *)CONFIG_SYS_NAND_BASE;

	nand->ecc.mode = NAND_ECC_SOFT;

	/* Reference hardware control function */
	nand->cmd_ctrl  = alpr_nand_hwcontrol;
	nand->read_byte  = alpr_nand_read_byte;
	nand->write_buf  = alpr_nand_write_buf;
	nand->read_buf   = alpr_nand_read_buf;
	nand->verify_buf = alpr_nand_verify_buf;
	nand->dev_ready  = alpr_nand_dev_ready;

	return 0;
}
#endif
