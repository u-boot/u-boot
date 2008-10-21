/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de
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
#include <config.h>
#if defined(CONFIG_CMD_NAND)
#include <asm/gpio.h>
#include <asm/io.h>
#include <nand.h>

/*
 *	hardware specific access to control-lines
 */
static void quad100hd_hwcontrol(struct mtd_info *mtd,
				int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		gpio_write_bit(CONFIG_SYS_NAND_CLE, !!(ctrl & NAND_CLE));
		gpio_write_bit(CONFIG_SYS_NAND_ALE, !!(ctrl & NAND_ALE));
		gpio_write_bit(CONFIG_SYS_NAND_CE, !(ctrl & NAND_NCE));
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static int quad100hd_nand_ready(struct mtd_info *mtd)
{
	return gpio_read_in_bit(CONFIG_SYS_NAND_RDY);
}

/*
 * Main initialization routine
 */
int board_nand_init(struct nand_chip *nand)
{
	/* Set address of hardware control function */
	nand->cmd_ctrl = quad100hd_hwcontrol;
	nand->dev_ready = quad100hd_nand_ready;
	nand->ecc.mode = NAND_ECC_SOFT;
	/* 15 us command delay time */
	nand->chip_delay =  20;

	/* Return happy */
	return 0;
}
#endif /* CONFIG_CMD_NAND */
