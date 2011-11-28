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
#include <nand.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(emac_pins_mii),
	PINMUX_ITEM(emac_pins_mdio),
	PINMUX_ITEM(emifa_pins_cs3),
	PINMUX_ITEM(emifa_pins_cs4),
	PINMUX_ITEM(emifa_pins_nand),
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(uart2_pins_rtscts),
};

static const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

void board_init_f(ulong bootflag)
{
	/*
	 * Kick Registers need to be set to allow access to Pin Mux registers
	 */
	writel(DV_SYSCFG_KICK0_UNLOCK, &davinci_syscfg_regs->kick0);
	writel(DV_SYSCFG_KICK1_UNLOCK, &davinci_syscfg_regs->kick1);

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2), &davinci_syscfg_regs->suspsrc);

	/* Power on required peripherals
	 * ARM does not have acess by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc));

	/* configure pinmux settings */
	davinci_configure_pin_mux_items(pinmuxes,
					ARRAY_SIZE(pinmuxes));

	writel(readl(&davinci_uart2_ctrl_regs->pwremu_mgmt) |
	       (DAVINCI_UART_PWREMU_MGMT_FREE) |
	       (DAVINCI_UART_PWREMU_MGMT_URRST) |
	       (DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	NS16550_init((NS16550_t)(DAVINCI_UART2_BASE),
			CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);

	puts("Nand boot...\n");

	nand_boot();
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

void putc(char c)
{
	if (gd->flags & GD_FLG_SILENT)
		return;

	if (c == '\n')
		NS16550_putc((NS16550_t)(DAVINCI_UART2_BASE), '\r');

	NS16550_putc((NS16550_t)(DAVINCI_UART2_BASE), c);
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}
