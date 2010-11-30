/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Based on da850evm.c, original Copyrights follow:
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
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
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>

DECLARE_GLOBAL_DATA_PTR;

#define pinmux(x)	(&davinci_syscfg_regs->pinmux[x])

/* SPI0 pin muxer settings */
static const struct pinmux_config spi1_pins[] = {
	{ pinmux(5), 1, 1 },
	{ pinmux(5), 1, 2 },
	{ pinmux(5), 1, 4 },
	{ pinmux(5), 1, 5 }
};

/* UART pin muxer settings */
static const struct pinmux_config uart_pins[] = {
	{ pinmux(0), 4, 6 },
	{ pinmux(0), 4, 7 },
	{ pinmux(4), 2, 4 },
	{ pinmux(4), 2, 5 }
};

#ifdef CONFIG_DRIVER_TI_EMAC
#define HAS_RMII 1
static const struct pinmux_config emac_pins[] = {
	{ pinmux(14), 8, 2 },
	{ pinmux(14), 8, 3 },
	{ pinmux(14), 8, 4 },
	{ pinmux(14), 8, 5 },
	{ pinmux(14), 8, 6 },
	{ pinmux(14), 8, 7 },
	{ pinmux(15), 8, 1 },
	{ pinmux(4), 8, 0 },
	{ pinmux(4), 8, 1 }
};
#endif

#ifdef CONFIG_NAND_DAVINCI
const struct pinmux_config nand_pins[] = {
	{ pinmux(7), 1, 1 },
	{ pinmux(7), 1, 2 },
	{ pinmux(7), 1, 4 },
	{ pinmux(7), 1, 5 },
	{ pinmux(9), 1, 0 },
	{ pinmux(9), 1, 1 },
	{ pinmux(9), 1, 2 },
	{ pinmux(9), 1, 3 },
	{ pinmux(9), 1, 4 },
	{ pinmux(9), 1, 5 },
	{ pinmux(9), 1, 6 },
	{ pinmux(9), 1, 7 },
	{ pinmux(12), 1, 5 },
	{ pinmux(12), 1, 6 }
};
#endif

static const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi1_pins),
#endif
	PINMUX_ITEM(uart_pins),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(nand_pins),
#endif
};

static const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
};

int board_init(void)
{
#ifndef CONFIG_USE_IRQ
	irq_init();
#endif


#ifdef CONFIG_NAND_DAVINCI
	/*
	 * NAND CS setup - cycle counts based on da850evm NAND timings in the
	 * Linux kernel @ 25MHz EMIFA
	 */
	writel((DAVINCI_ABCR_WSETUP(0) |
		DAVINCI_ABCR_WSTROBE(0) |
		DAVINCI_ABCR_WHOLD(0) |
		DAVINCI_ABCR_RSETUP(0) |
		DAVINCI_ABCR_RSTROBE(1) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(0) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab2cr); /* CS3 */
#endif

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_EA20;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_DRIVER_TI_EMAC
	if (davinci_configure_pin_mux(emac_pins, ARRAY_SIZE(emac_pins)) != 0)
		return 1;

	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

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

	/*
	 * This board has a RMII PHY. However, the MDC line on the SOM
	 * must not be disabled (there is no MII PHY on the
	 * baseboard) via the GPIO2[6], because this pin
	 * disables at the same time the SPI flash.
	 */

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
