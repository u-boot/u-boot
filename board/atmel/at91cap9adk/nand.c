/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop <at> leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2006 ATMEL Rousset, Lacressonniere Nicolas
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/at91cap9.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_pio.h>

#include <nand.h>

/*
 *	hardware specific access to control-lines
 */
#define	MASK_ALE	(1 << 21)	/* our ALE is AD21 */
#define	MASK_CLE	(1 << 22)	/* our CLE is AD22 */

static void at91cap9adk_nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;

	IO_ADDR_W &= ~(MASK_ALE|MASK_CLE);
	switch (cmd) {
	case NAND_CTL_SETCLE:
		IO_ADDR_W |= MASK_CLE;
		break;
	case NAND_CTL_SETALE:
		IO_ADDR_W |= MASK_ALE;
		break;
	case NAND_CTL_CLRNCE:
		at91_set_gpio_value(AT91_PIN_PD15, 1);
		break;
	case NAND_CTL_SETNCE:
		at91_set_gpio_value(AT91_PIN_PD15, 0);
		break;
	}
	this->IO_ADDR_W = (void *) IO_ADDR_W;
}

int board_nand_init(struct nand_chip *nand)
{
	nand->eccmode = NAND_ECC_SOFT;
	nand->hwcontrol = at91cap9adk_nand_hwcontrol;
	nand->chip_delay = 20;

	return 0;
}
