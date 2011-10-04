/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/kirkwood.h>
#include <nand.h>

/* NAND Flash Soc registers */
struct kwnandf_registers {
	u32 rd_params;	/* 0x10418 */
	u32 wr_param;	/* 0x1041c */
	u8  pad[0x10470 - 0x1041c - 4];
	u32 ctrl;	/* 0x10470 */
};

static struct kwnandf_registers *nf_reg =
	(struct kwnandf_registers *)KW_NANDF_BASE;

/*
 * hardware specific access to control-lines/bits
 */
#define NAND_ACTCEBOOT_BIT		0x02

static void kw_nand_hwcontrol(struct mtd_info *mtd, int cmd,
			      unsigned int ctrl)
{
	struct nand_chip *nc = mtd->priv;
	u32 offs;

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		offs = (1 << 0);	/* Commands with A[1:0] == 01 */
	else if (ctrl & NAND_ALE)
		offs = (1 << 1);	/* Addresses with A[1:0] == 10 */
	else
		return;

	writeb(cmd, nc->IO_ADDR_W + offs);
}

void kw_nand_select_chip(struct mtd_info *mtd, int chip)
{
	u32 data;

	data = readl(&nf_reg->ctrl);
	data |= NAND_ACTCEBOOT_BIT;
	writel(data, &nf_reg->ctrl);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->options = NAND_COPYBACK | NAND_CACHEPRG | NAND_NO_PADDING;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->cmd_ctrl = kw_nand_hwcontrol;
	nand->chip_delay = 40;
	nand->select_chip = kw_nand_select_chip;
	return 0;
}
