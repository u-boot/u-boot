/*
 * (C) Copyright 2005 2N TELEKOMUNIKACE, Ladislav Michl
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
#include <asm/io.h>

#if defined(CONFIG_CMD_NAND)

#include <nand.h>

/*
 *	hardware specific access to control-lines
 */
#define	MASK_CLE	0x02
#define	MASK_ALE	0x04

static void netstar_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;

	IO_ADDR_W &= ~(MASK_ALE|MASK_CLE);
	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			IO_ADDR_W |= MASK_CLE;
		if ( ctrl & NAND_ALE )
			IO_ADDR_W |= MASK_ALE;
	}
	this->IO_ADDR_W = (void __iomem *) IO_ADDR_W;

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->cmd_ctrl = netstar_nand_hwcontrol;
	nand->chip_delay = 400;
	return 0;
}
#endif
