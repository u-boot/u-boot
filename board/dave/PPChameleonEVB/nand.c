/*
 * (C) Copyright 2006 DENX Software Engineering
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_CMD_NAND)

#include <nand.h>

/*
 * hardware specific access to control-lines
 * function borrowed from Linux 2.6 (drivers/mtd/nand/ppchameleonevb.c)
 */
static void ppchameleonevb_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	ulong base = (ulong) this->IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			MACRO_NAND_CTL_SETCLE((unsigned long)base);
		else
			MACRO_NAND_CTL_CLRCLE((unsigned long)base);
		if ( ctrl & NAND_ALE )
			MACRO_NAND_CTL_CLRCLE((unsigned long)base);
		else
			MACRO_NAND_CTL_CLRALE((unsigned long)base);
		if ( ctrl & NAND_NCE )
			MACRO_NAND_ENABLE_CE((unsigned long)base);
		else
			MACRO_NAND_DISABLE_CE((unsigned long)base);
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}


/*
 * read device ready pin
 * function +/- borrowed from Linux 2.6 (drivers/mtd/nand/ppchameleonevb.c)
 */
static int ppchameleonevb_device_ready(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong rb_gpio_pin;

	/* use the base addr to find out which chip are we dealing with */
	switch((ulong) this->IO_ADDR_W) {
	case CONFIG_SYS_NAND0_BASE:
		rb_gpio_pin = CONFIG_SYS_NAND0_RDY;
		break;
	case CONFIG_SYS_NAND1_BASE:
		rb_gpio_pin = CONFIG_SYS_NAND1_RDY;
		break;
	default: /* this should never happen */
		return 0;
		break;
	}

	if (in32(GPIO0_IR) & rb_gpio_pin)
		return 1;
	return 0;
}


/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand.h):
 * - IO_ADDR_R?: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W?: address to write the 8 I/O lines of the flash device
 * - cmd_ctrl: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - ecc.mode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
int board_nand_init(struct nand_chip *nand)
{

	nand->cmd_ctrl = ppchameleonevb_hwcontrol;
	nand->dev_ready = ppchameleonevb_device_ready;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_BIG_DELAY_US;
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
	return 0;
}
#endif
