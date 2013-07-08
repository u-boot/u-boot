/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_CMD_NAND)
#include <asm/io.h>
#include <nand.h>

/*
 * hardware specific access to control-lines
 */
static void esd405ep_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CONFIG_SYS_NAND_CLE);
		else
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_NAND_CLE);
		if ( ctrl & NAND_ALE )
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CONFIG_SYS_NAND_ALE);
		else
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_NAND_ALE);
		if ( ctrl & NAND_NCE )
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_NAND_CE);
		else
			out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CONFIG_SYS_NAND_CE);
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}


/*
 * read device ready pin
 */
static int esd405ep_nand_device_ready(struct mtd_info *mtdinfo)
{
	if (in_be32((void *)GPIO0_IR) & CONFIG_SYS_NAND_RDY)
		return 1;
	return 0;
}


int board_nand_init(struct nand_chip *nand)
{
	/*
	 * Set NAND-FLASH GPIO signals to defaults
	 */
	out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) & ~(CONFIG_SYS_NAND_CLE | CONFIG_SYS_NAND_ALE));
	out_be32((void *)GPIO0_OR, in_be32((void *)GPIO0_OR) | CONFIG_SYS_NAND_CE);

	/*
	 * Initialize nand_chip structure
	 */
	nand->cmd_ctrl = esd405ep_nand_hwcontrol;
	nand->dev_ready = esd405ep_nand_device_ready;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_BIG_DELAY_US;
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
	return 0;
}
#endif
