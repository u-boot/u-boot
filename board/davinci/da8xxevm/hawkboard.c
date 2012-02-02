/*
 * Modified for Hawkboard - Syed Mohammed Khasim <khasim@beagleboard.org>
 *
 * Copyright (C) 2008 Sekhar Nori, Texas Instruments, Inc.  <nsekhar@ti.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/pinmux_defs.h>
#include <ns16550.h>

DECLARE_GLOBAL_DATA_PTR;

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(emac_pins_mii),
	PINMUX_ITEM(emac_pins_mdio),
	PINMUX_ITEM(emifa_pins_cs3),
	PINMUX_ITEM(emifa_pins_cs4),
	PINMUX_ITEM(emifa_pins_nand),
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(uart2_pins_rtscts),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_OMAPL138_HAWKBOARD;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	return 0;
}

int board_early_init_f(void)
{
	/*
	 * Kick Registers need to be set to allow access to Pin Mux registers
	 */
	writel(DV_SYSCFG_KICK0_UNLOCK, &davinci_syscfg_regs->kick0);
	writel(DV_SYSCFG_KICK1_UNLOCK, &davinci_syscfg_regs->kick1);

	/* set cfgchip3 to select mii */
	writel(readl(&davinci_syscfg_regs->cfgchip3) &
	       ~(1 << 8), &davinci_syscfg_regs->cfgchip3);

	return 0;
}

int misc_init_r(void)
{
	char buf[32];

	printf("ARM Clock : %s MHz\n",
	       strmhz(buf, clk_get(DAVINCI_ARM_CLKID)));

	return 0;
}
