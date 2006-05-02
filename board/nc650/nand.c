/*
 * (C) Copyright 2006 Detlev Zundel, dzu@denx.de
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

#if defined(CONFIG_IDS852_REV1)
/*
 *	hardware specific access to control-lines
 */
static void nc650_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;

	switch(cmd) {
	case NAND_CTL_SETCLE:
		this->IO_ADDR_W += 2;
		break;
	case NAND_CTL_CLRCLE:
		this->IO_ADDR_W -= 2;
		break;
	case NAND_CTL_SETALE:
		this->IO_ADDR_W += 1;
		break;
	case NAND_CTL_CLRALE:
		this->IO_ADDR_W -= 1;
		break;
	case NAND_CTL_SETNCE:
	case NAND_CTL_CLRNCE:
		/* nop */
		break;
	}
}
#elif defined(CONFIG_IDS852_REV2)
/*
 *	hardware specific access to control-lines
 */
static void nc650_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;

	switch(cmd) {
	case NAND_CTL_SETCLE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0xa) = 0;
		break;
	case NAND_CTL_CLRCLE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0x8) = 0;
		break;
	case NAND_CTL_SETALE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0x9) = 0;
		break;
	case NAND_CTL_CLRALE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0x8) = 0;
		break;
	case NAND_CTL_SETNCE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0x8) = 0;
		break;
	case NAND_CTL_CLRNCE:
		*(((volatile __u8 *) this->IO_ADDR_W) + 0xc) = 0;
		break;
	}
}
#else
#error Unknown IDS852 module revision
#endif

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

	nand->hwcontrol = nc650_hwcontrol;
	nand->eccmode = NAND_ECC_SOFT;
	nand->chip_delay = 12;
/*	nand->options = NAND_SAMSUNG_LP_OPTIONS;*/
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */
