/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * Based on da850evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/timer_defs.h>

DECLARE_GLOBAL_DATA_PTR;

#define CALIMAIN_HWVERSION_MASK    0x7f000000
#define CALIMAIN_HWVERSION_SHIFT   24

/* Hardware version pinmux settings */
const struct pinmux_config hwversion_pins[] = {
	{ pinmux(16), 8, 2 }, /* GP7[15] */
	{ pinmux(16), 8, 3 }, /* GP7[14] */
	{ pinmux(16), 8, 4 }, /* GP7[13] */
	{ pinmux(16), 8, 5 }, /* GP7[12] */
	{ pinmux(16), 8, 6 }, /* GP7[11] */
	{ pinmux(16), 8, 7 }, /* GP7[10] */
	{ pinmux(17), 8, 0 }, /* GP7[9] */
	{ pinmux(17), 8, 1 }  /* GP7[8] */
};

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(emac_pins_mii),
	PINMUX_ITEM(emac_pins_mdio),
	PINMUX_ITEM(emifa_pins_nor),
	PINMUX_ITEM(emifa_pins_cs2),
	PINMUX_ITEM(emifa_pins_cs3),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

/* read board revision from GPIO7[8..14] */
u32 get_board_rev(void)
{
	lpsc_on(DAVINCI_LPSC_GPIO);
	if (davinci_configure_pin_mux(hwversion_pins,
				      ARRAY_SIZE(hwversion_pins)) != 0)
		return 0xffffffff;

	return (davinci_gpio_bank67->in_data & CALIMAIN_HWVERSION_MASK)
		>> CALIMAIN_HWVERSION_SHIFT;
}

/*
 * determine the oscillator frequency depending on the board revision
 *
 * rev 0x00  ... 25 MHz oscillator
 * rev 0x01  ... 24 MHz oscillator
 */
int calimain_get_osc_freq(void)
{
	u32 rev;
	int freq;

	rev = get_board_rev();
	switch (rev) {
	case 0x00:
		freq = 25000000;
		break;
	default:
		freq = 24000000;
		break;
	}
	return freq;
}

int board_init(void)
{
	int val;

#ifndef CONFIG_USE_IRQ
	irq_init();
#endif

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

#ifdef CONFIG_DRIVER_TI_EMAC
	/* select emac MII mode */
	val = readl(&davinci_syscfg_regs->cfgchip3);
	val &= ~(1 << 8);
	writel(val, &davinci_syscfg_regs->cfgchip3);
#endif /* CONFIG_DRIVER_TI_EMAC */

#ifdef CONFIG_HW_WATCHDOG
	davinci_hw_watchdog_enable();
#endif

	printf("Input clock frequency: %d Hz\n", calimain_get_osc_freq());
	printf("Board revision:        %d\n", get_board_rev());

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	davinci_hw_watchdog_reset();
}
#endif
