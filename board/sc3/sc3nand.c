/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_CMD_NAND)

#include <nand.h>
#include <asm/processor.h>

#define readb(addr)	*(volatile u_char *)(addr)
#define readl(addr)	*(volatile u_long *)(addr)
#define writeb(d,addr)	*(volatile u_char *)(addr) = (d)

#define SC3_NAND_ALE 29 /* GPIO PIN 3 */
#define SC3_NAND_CLE 30	/* GPIO PIN 2 */
#define SC3_NAND_CE  27 /* GPIO PIN 5 */

static void *sc3_io_base;
static void *sc3_control_base = (void *)0xEF600700;

static void sc3_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			set_bit (SC3_NAND_CLE, sc3_control_base);
		else
			clear_bit (SC3_NAND_CLE, sc3_control_base);
		if ( ctrl & NAND_ALE )
			set_bit (SC3_NAND_ALE, sc3_control_base);
		else
			clear_bit (SC3_NAND_ALE, sc3_control_base);
		if ( ctrl & NAND_NCE )
			set_bit (SC3_NAND_CE, sc3_control_base);
		else
			clear_bit (SC3_NAND_CE, sc3_control_base);
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static int sc3_nand_dev_ready(struct mtd_info *mtd)
{
	if (!(readl(sc3_control_base + 0x1C) & 0x4000))
		return 0;
	return 1;
}

static void sc3_select_chip(struct mtd_info *mtd, int chip)
{
	clear_bit (SC3_NAND_CE, sc3_control_base);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->ecc.mode = NAND_ECC_SOFT;

	sc3_io_base = (void *) CONFIG_SYS_NAND_BASE;
	/* Set address of NAND IO lines (Using Linear Data Access Region) */
	nand->IO_ADDR_R = (void __iomem *) sc3_io_base;
	nand->IO_ADDR_W = (void __iomem *) sc3_io_base;
	/* Reference hardware control function */
	nand->cmd_ctrl  = sc3_nand_hwcontrol;
	nand->dev_ready  = sc3_nand_dev_ready;
	nand->select_chip = sc3_select_chip;
	return 0;
}
#endif
