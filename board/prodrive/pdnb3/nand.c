/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
static void pdnb3_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
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
			writeb(0x00, &(pdnb3_ndfc->term));
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
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

	for (i = 0; i < len; i++)
		buf[i] = readb(&(pdnb3_ndfc->data));
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
	/*
	 * Blocking read to wait for NAND to be ready
	 */
	readb(&(pdnb3_ndfc->wait));

	/*
	 * Return always true
	 */
	return 1;
}

int board_nand_init(struct nand_chip *nand)
{
	pdnb3_ndfc = (struct pdnb3_ndfc_regs *)CONFIG_SYS_NAND_BASE;

	nand->ecc.mode = NAND_ECC_SOFT;

	/* Set address of NAND IO lines (Using Linear Data Access Region) */
	nand->IO_ADDR_R = (void __iomem *) ((ulong) pdnb3_ndfc + 0x4);
	nand->IO_ADDR_W = (void __iomem *) ((ulong) pdnb3_ndfc + 0x4);
	/* Reference hardware control function */
	nand->cmd_ctrl   = pdnb3_nand_hwcontrol;
	nand->read_byte  = pdnb3_nand_read_byte;
	nand->write_buf  = pdnb3_nand_write_buf;
	nand->read_buf   = pdnb3_nand_read_buf;
	nand->verify_buf = pdnb3_nand_verify_buf;
	nand->dev_ready  = pdnb3_nand_dev_ready;
	return 0;
}
#endif
