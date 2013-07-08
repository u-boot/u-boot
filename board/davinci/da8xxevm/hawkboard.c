/*
 * Modified for Hawkboard - Syed Mohammed Khasim <khasim@beagleboard.org>
 *
 * Copyright (C) 2008 Sekhar Nori, Texas Instruments, Inc.  <nsekhar@ti.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2004 Texas Instruments.
 * Copyright (C) 2012 Sughosh Ganu <urwithsughosh@gmail.com>.
 *
 * ----------------------------------------------------------------------------
 * SPDX-License-Identifier:	GPL-2.0+
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/arch/da8xx-usb.h>
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

int usb_phy_on(void)
{
	u32 timeout;
	u32 cfgchip2;

	cfgchip2 = readl(&davinci_syscfg_regs->cfgchip2);

	cfgchip2 &= ~(CFGCHIP2_RESET | CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN |
		      CFGCHIP2_OTGMODE | CFGCHIP2_REFFREQ |
		      CFGCHIP2_USB1PHYCLKMUX);
	cfgchip2 |= CFGCHIP2_SESENDEN | CFGCHIP2_VBDTCTEN | CFGCHIP2_PHY_PLLON |
		    CFGCHIP2_REFFREQ_24MHZ | CFGCHIP2_USB2PHYCLKMUX |
		    CFGCHIP2_USB1SUSPENDM;

	writel(cfgchip2, &davinci_syscfg_regs->cfgchip2);

	/* wait until the usb phy pll locks */
	timeout = DA8XX_USB_OTG_TIMEOUT;
	while (timeout--)
		if (readl(&davinci_syscfg_regs->cfgchip2) & CFGCHIP2_PHYCLKGD)
			return 1;

	/* USB phy was not turned on */
	return 0;
}

void usb_phy_off(void)
{
	u32 cfgchip2;

	/*
	 * Power down the on-chip PHY.
	 */
	cfgchip2 = readl(&davinci_syscfg_regs->cfgchip2);
	cfgchip2 &= ~(CFGCHIP2_PHY_PLLON | CFGCHIP2_USB1SUSPENDM);
	cfgchip2 |= CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN | CFGCHIP2_RESET;
	writel(cfgchip2, &davinci_syscfg_regs->cfgchip2);
}
