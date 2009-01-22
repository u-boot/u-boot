/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * This driver support NAND devices which have address lines
 * connected as ALE and CLE inputs.
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
#include <nand.h>
#include <asm/io.h>

/*
 * Hardware specific access to control-lines
 */
static void nand_addr_hwcontrol(struct mtd_info *mtd, int cmd, uint ctrl)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		IO_ADDR_W = (ulong)this->IO_ADDR_W;

		IO_ADDR_W &= ~(CONFIG_SYS_NAND_ACTL_CLE |
				CONFIG_SYS_NAND_ACTL_ALE |
				CONFIG_SYS_NAND_ACTL_NCE);
		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_ACTL_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_ACTL_ALE;
		if (ctrl & NAND_NCE)
			IO_ADDR_W |= CONFIG_SYS_NAND_ACTL_NCE;

		this->IO_ADDR_W = (void *)IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->cmd_ctrl = nand_addr_hwcontrol;
	nand->chip_delay = CONFIG_SYS_NAND_ACTL_DELAY;

	return 0;
}
