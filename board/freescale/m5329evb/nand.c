/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <nand.h>
#include <linux/mtd/mtd.h>

#define SET_CLE		0x10
#define CLR_CLE		~SET_CLE
#define SET_ALE		0x08
#define CLR_ALE		~SET_ALE

static void nand_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	struct nand_chip *this = mtdinfo->priv;
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
	u32 nand_baseaddr = (u32) this->IO_ADDR_W;

	switch (cmd) {
	case NAND_CTL_SETNCE:
	case NAND_CTL_CLRNCE:
		break;
	case NAND_CTL_SETCLE:
		nand_baseaddr |= SET_CLE;
		break;
	case NAND_CTL_CLRCLE:
		nand_baseaddr &= CLR_CLE;
		break;
	case NAND_CTL_SETALE:
		nand_baseaddr |= SET_ALE;
		break;
	case NAND_CTL_CLRALE:
		nand_baseaddr |= CLR_ALE;
		break;
	case NAND_CTL_SETWP:
		fbcs->csmr2 |= CSMR_WP;
		break;
	case NAND_CTL_CLRWP:
		fbcs->csmr2 &= ~CSMR_WP;
		break;
	}
	this->IO_ADDR_W = (void __iomem *)(nand_baseaddr);
}

static void nand_write_byte(struct mtd_info *mtdinfo, u_char byte)
{
	struct nand_chip *this = mtdinfo->priv;
	*((volatile u8 *)(this->IO_ADDR_W)) = byte;
}

static u8 nand_read_byte(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtdinfo->priv;
	return (u8) (*((volatile u8 *)this->IO_ADDR_R));
}

static int nand_dev_ready(struct mtd_info *mtdinfo)
{
	return 1;
}

int board_nand_init(struct nand_chip *nand)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	*((volatile u16 *)CFG_LATCH_ADDR) |= 0x0004;

	/* set up pin configuration */
	gpio->par_timer &= ~GPIO_PAR_TIN3_TIN3;
	gpio->pddr_timer |= 0x08;
	gpio->ppd_timer |= 0x08;
	gpio->pclrr_timer = 0;
	gpio->podr_timer = 0;

	nand->chip_delay = 50;
	nand->eccmode = NAND_ECC_SOFT;
	nand->hwcontrol = nand_hwcontrol;
	nand->read_byte = nand_read_byte;
	nand->write_byte = nand_write_byte;
	nand->dev_ready = nand_dev_ready;

	return 0;
}
#endif
