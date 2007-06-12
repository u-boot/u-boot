/*
 * (C) Copyright 2006 Aubrey.Li, aubrey.li@analog.com
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

#if (CONFIG_COMMANDS & CFG_CMD_NAND) || defined(CONFIG_CMD_NAND)

#include <nand.h>

#define CONCAT(a,b,c,d) a ## b ## c ## d
#define PORT(a,b)  CONCAT(pPORT,a,b,)

#ifndef CONFIG_NAND_GPIO_PORT
#define CONFIG_NAND_GPIO_PORT F
#endif

/*
 * hardware specific access to control-lines
 */
static void bfin_hwcontrol(struct mtd_info *mtd, int cmd)
{
	register struct nand_chip *this = mtd->priv;

	switch (cmd) {

	case NAND_CTL_SETCLE:
		this->IO_ADDR_W = CFG_NAND_BASE + BFIN_NAND_CLE;
		break;
	case NAND_CTL_CLRCLE:
		this->IO_ADDR_W = CFG_NAND_BASE;
		break;

	case NAND_CTL_SETALE:
		this->IO_ADDR_W = CFG_NAND_BASE + BFIN_NAND_ALE;
		break;
	case NAND_CTL_CLRALE:
		this->IO_ADDR_W = CFG_NAND_BASE;
		break;
	case NAND_CTL_SETNCE:
	case NAND_CTL_CLRNCE:
		break;
	}

	this->IO_ADDR_R = this->IO_ADDR_W;

	/* Drain the writebuffer */
	sync();
}

int bfin_device_ready(struct mtd_info *mtd)
{
	int ret = (*PORT(CONFIG_NAND_GPIO_PORT, IO) & BFIN_NAND_READY) ? 1 : 0;
	sync();
	return ret;
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
	*PORT(CONFIG_NAND_GPIO_PORT, _FER) &= ~BFIN_NAND_READY;
	*PORT(CONFIG_NAND_GPIO_PORT, IO_DIR) &= ~BFIN_NAND_READY;
	*PORT(CONFIG_NAND_GPIO_PORT, IO_INEN) |= BFIN_NAND_READY;

	nand->hwcontrol = bfin_hwcontrol;
	nand->eccmode = NAND_ECC_SOFT;
	nand->dev_ready = bfin_device_ready;
	nand->chip_delay = 30;
}
#endif				/* (CONFIG_COMMANDS & CFG_CMD_NAND) */
