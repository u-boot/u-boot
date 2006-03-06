/*
 * (C) Copyright 2006 DENX Software Engineering
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
 * hardware specific access to control-lines
 * function borrowed from Linux 2.6 (drivers/mtd/nand/ppchameleonevb.c)
 */
static void ppchameleonevb_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	struct nand_chip *this = mtdinfo->priv;
	ulong base = (ulong) this->IO_ADDR_W;

	switch(cmd) {
	case NAND_CTL_SETCLE:
		MACRO_NAND_CTL_SETCLE((unsigned long)base);
		break;
	case NAND_CTL_CLRCLE:
		MACRO_NAND_CTL_CLRCLE((unsigned long)base);
		break;
	case NAND_CTL_SETALE:
		MACRO_NAND_CTL_SETALE((unsigned long)base);
		break;
	case NAND_CTL_CLRALE:
		MACRO_NAND_CTL_CLRALE((unsigned long)base);
		break;
	case NAND_CTL_SETNCE:
		MACRO_NAND_ENABLE_CE((unsigned long)base);
		break;
	case NAND_CTL_CLRNCE:
		MACRO_NAND_DISABLE_CE((unsigned long)base);
		break;
	}
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
	case CFG_NAND0_BASE:
		rb_gpio_pin = CFG_NAND0_RDY;
		break;
	case CFG_NAND1_BASE:
		rb_gpio_pin = CFG_NAND1_RDY;
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
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - eccmode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
void board_nand_init(struct nand_chip *nand)
{

	nand->hwcontrol = ppchameleonevb_hwcontrol;
	nand->dev_ready = ppchameleonevb_device_ready;
	nand->eccmode = NAND_ECC_SOFT;
	nand->chip_delay = NAND_BIG_DELAY_US;
	nand->options = NAND_SAMSUNG_LP_OPTIONS;
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */
