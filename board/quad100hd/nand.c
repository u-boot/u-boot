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
#include <nand.h>

/*
 *	hardware specific access to control-lines
 */
static void quad100hd_hwcontrol(struct mtd_info *mtd, int cmd)
{
	switch(cmd) {
	case NAND_CTL_SETCLE:
		gpio_write_bit(CFG_NAND_CLE, 1);
		break;
	case NAND_CTL_CLRCLE:
		gpio_write_bit(CFG_NAND_CLE, 0);
		break;

	case NAND_CTL_SETALE:
		gpio_write_bit(CFG_NAND_ALE, 1);
		break;
	case NAND_CTL_CLRALE:
		gpio_write_bit(CFG_NAND_ALE, 0);
		break;

	case NAND_CTL_SETNCE:
		gpio_write_bit(CFG_NAND_CE, 0);
		break;
	case NAND_CTL_CLRNCE:
		gpio_write_bit(CFG_NAND_CE, 1);
		break;
	}
}

static int quad100hd_nand_ready(struct mtd_info *mtd)
{
	return gpio_read_in_bit(CFG_NAND_RDY);
}

/*
 * Main initialization routine
 */
int board_nand_init(struct nand_chip *nand)
{
	/* Set address of hardware control function */
	nand->hwcontrol = quad100hd_hwcontrol;
	nand->dev_ready = quad100hd_nand_ready;
	nand->eccmode = NAND_ECC_SOFT;
	/* 15 us command delay time */
	nand->chip_delay =  20;

	/* Return happy */
	return 0;
}
#endif /* CONFIG_CMD_NAND */
