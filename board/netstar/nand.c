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

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>

/*
 *	hardware specific access to control-lines
 */
#define	MASK_CLE	0x02
#define	MASK_ALE	0x04

static void netstar_nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;

	IO_ADDR_W &= ~(MASK_ALE|MASK_CLE);
	switch (cmd) {
		case NAND_CTL_SETCLE: IO_ADDR_W |= MASK_CLE; break;
		case NAND_CTL_SETALE: IO_ADDR_W |= MASK_ALE; break;
	}
	this->IO_ADDR_W = (void *) IO_ADDR_W;
}

/*
 *	chip R/B detection
 */
/***
static int netstar_nand_ready(struct mtd_info *mtd)
{
	return (*(volatile ushort *)GPIO_DATA_INPUT_REG) & 0x02;
}
***/

void board_nand_init(struct nand_chip *nand)
{
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
	nand->eccmode = NAND_ECC_SOFT;
	nand->hwcontrol = netstar_nand_hwcontrol;
/*	nand->dev_ready = netstar_nand_ready; */
	nand->chip_delay = 18;
}
#endif
