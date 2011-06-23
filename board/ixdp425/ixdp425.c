/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <command.h>
#include <malloc.h>
#include <netdev.h>
#include <asm/arch/ixp425.h>
#include <asm/io.h>
#ifdef CONFIG_PCI
#include <pci.h>
#include <asm/arch/ixp425pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define IXDP425_LED_PORT 0x52000000 /* 4-digit hex display */

int board_early_init_f(void)
{
	/* CS2: LED port */
	writel(0xbcff0002, IXP425_EXP_CS2);
	writew(0x0001, IXDP425_LED_PORT); /* output postcode to LEDs */

	return 0;
}

#ifdef CONFIG_PCI
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_ixpdp425_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, PCI_ANY_ID,
	  pci_cfgfunc_config_device,
	  { 0x400,
	    0x40000000,
	    PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER } },

	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x01, PCI_ANY_ID,
	  pci_cfgfunc_config_device,
	  { 0x800,
	    0x40010000,
	    PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER } },

	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x02, PCI_ANY_ID,
	  pci_cfgfunc_config_device,
	  { 0xc00,
	    0x40020000,
	    PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER } },

	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x03, PCI_ANY_ID,
	  pci_cfgfunc_config_device,
	  { 0x1000,
	    0x40030000,
	    PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER } },
	{ }
};
#endif

struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_ixpdp425_config_table,
#endif
};
#endif /* CONFIG_PCI */


/*
 * Miscelaneous platform dependent initialisations
 */
int board_init(void)
{
	writew(0x0002, IXDP425_LED_PORT); /* output postcode to LEDs */

#ifdef CONFIG_IXDPG425
	/* arch number of IXDP */
	gd->bd->bi_arch_number = MACH_TYPE_IXDPG425;
#else
	/* arch number of IXDP */
	gd->bd->bi_arch_number = MACH_TYPE_IXDP425;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

#ifdef CONFIG_IXDPG425
	/*
	 * Get realtek RTL8305 switch and SLIC out of reset
	 */
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_SWITCH_RESET_N);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_SWITCH_RESET_N);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_SLIC_RESET_N);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_SLIC_RESET_N);

	/*
	 * Setup GPIOs for PCI INTA & INTB
	 */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTA_N);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTA_N);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTB_N);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTB_N);

	/* Setup GPIOs for 33MHz clock output */
	writel(0x01FF01FF, IXP425_GPIO_GPCLKR);

	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_EXTBUS_CLK);

	/* set GPIO8..11 interrupt type to active low */
	writel((0x1 << 9) | (0x1 << 6) | (0x1 << 3) | 0x1, IXP425_GPIO_GPIT2R);

	/* clear pending interrupts */
	writel(-1, IXP425_GPIO_GPISR);

	/* assert PCI reset */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_SLIC_RESET_N);

	udelay(533);

	/* deassert PCI reset */
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_SLIC_RESET_N);

	udelay(533);

#else /* IXDP425 */
	/* Setup GPIOs for 33MHz ExpBus and PCI clock output */
	writel(0x01FF01FF, IXP425_GPIO_GPCLKR);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_EXTBUS_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_RESET_N);

	/* set GPIO8..11 interrupt type to active low */
	writel((0x1 << 9) | (0x1 << 6) | (0x1 << 3) | 0x1, IXP425_GPIO_GPIT2R);
	/* clear pending interrupts */
	writel(-1, IXP425_GPIO_GPISR);

	/* assert PCI reset */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_PCI_RESET_N);

	udelay(533);

	/* deassert PCI reset */
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_PCI_RESET_N);

	udelay(533);
#endif

	return 0;
}

/*
 * Check Board Identity
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

#ifdef CONFIG_IXDPG425
	puts("Board: IXDPG425 - Intel Network Gateway Reference Platform");
#else
	puts("Board: IXDP425 - Intel Development Platform");
#endif

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

int dram_init(void)
{
	/* we can only map 64MB via PCI, so we limit memory
	   until a better solution is implemented. */
#ifdef CONFIG_PCI
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, 64<<20);
#else
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, 256<<20);
#endif
	return 0;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	pci_ixp_init(&hose);
}

/*
 * dev 0 on the PCI bus is not the host bridge, so we have to override
 * these functions in order to not skip PCI slot 0 during configuration.
*/
int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 0;
}
int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 1;
}

#endif

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_PCI
	pci_eth_init(bis);
#endif
	return cpu_eth_init(bis);
}
