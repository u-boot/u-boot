/*
 * (C) Copyright 2009
 * Michael Schwingen, michael@schwingen.org
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <malloc.h>
#include <asm/arch/ixp425.h>
#include <asm/io.h>
#include <miiphy.h>
#ifdef CONFIG_PCI
#include <pci.h>
#include <asm/arch/ixp425pci.h>
#endif

#include "dvlhost_hw.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* CS1: LED Latch */
	writel(0xBFFF0002, IXP425_EXP_CS1);
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	/* Setup GPIOs used as output */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_WDGTRIGGER);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_DLAN_PAIRING);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_PCIRST);

	/*
	 * LED latch enable and watchdog enable are tied to the same GPIO,
	 * so we need to trigger the watchdog if we want to enable the LEDs.
	*/
#ifdef CONFIG_HW_WATCHDOG
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_WDG_LED_EN);
#else
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_WDG_LED_EN);
#endif

	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_WDGTRIGGER);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_DLAN_PAIRING);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_WDG_LED_EN);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCIRST);

	/* Setup GPIOs for Interrupt inputs */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_BTN_WLAN);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_BTN_PAIRING);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_BTN_RESET);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_IRQA);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_IRQB);

	/* Setup GPIO's for 33MHz clock output */
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_EXTBUS_CLK);
	writel(0x01FF01FF, IXP425_GPIO_GPCLKR);

	/* turn off all LEDs */
	writew(0x0000, DVLHOST_LED_LATCH);

	udelay(533);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_PCIRST);

	return 0;
}

/* Check Board Identity */
int checkboard(void)
{
	char *s = getenv("serial#");

	puts("Board: dLAN 200AV (dvlhost)");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, 128<<20);
	return 0;
}

#ifdef CONFIG_PCI
struct pci_controller hose;

void pci_init_board(void)
{
	pci_ixp_init(&hose);
}
#endif

void reset_phy(void)
{
	/* init IcPlus IP175C ethernet switch to native IP175C mode */
	miiphy_write("NPE1", 29, 31, 0x175C);
}
