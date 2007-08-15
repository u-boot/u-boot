/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
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
#include <asm/io.h>
#include <nand.h>

/*
 * hardware specific access to control-lines
 */
static void esd405ep_nand_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	switch(cmd) {
	case NAND_CTL_SETCLE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CFG_NAND_CLE);
		break;
	case NAND_CTL_CLRCLE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CFG_NAND_CLE);
		break;
	case NAND_CTL_SETALE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CFG_NAND_ALE);
		break;
	case NAND_CTL_CLRALE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CFG_NAND_ALE);
		break;
	case NAND_CTL_SETNCE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CFG_NAND_CE);
		break;
	case NAND_CTL_CLRNCE:
		out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CFG_NAND_CE);
		break;
	}
}


/*
 * read device ready pin
 */
static int esd405ep_nand_device_ready(struct mtd_info *mtdinfo)
{
	if (in_be32((void *)GPIO0_IR) & CFG_NAND_RDY)
		return 1;
	return 0;
}


int board_nand_init(struct nand_chip *nand)
{
	/*
	 * Set NAND-FLASH GPIO signals to defaults
	 */
	out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~(CFG_NAND_CLE | CFG_NAND_ALE));
	out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CFG_NAND_CE);

	/*
	 * Initialize nand_chip structure
	 */
	nand->hwcontrol = esd405ep_nand_hwcontrol;
	nand->dev_ready = esd405ep_nand_device_ready;
	nand->eccmode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_BIG_DELAY_US;
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
	return 0;
}
#endif
