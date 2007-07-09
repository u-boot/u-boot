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
#include <asm/arch/ixp425.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */
int board_post_init (void)
{
	return (0);
}

int board_init (void)
{
	/* arch number of IXDP */
	gd->bd->bi_arch_number = MACH_TYPE_IXDP425;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

#ifdef CONFIG_IXDPG425
	/* arch number of IXDP */
	gd->bd->bi_arch_number = MACH_TYPE_IXDPG425;

	/*
	 * Get realtek RTL8305 switch and SLIC out of reset
	 */
	GPIO_OUTPUT_SET(CFG_GPIO_SWITCH_RESET_N);
	GPIO_OUTPUT_ENABLE(CFG_GPIO_SWITCH_RESET_N);
	GPIO_OUTPUT_SET(CFG_GPIO_SLIC_RESET_N);
	GPIO_OUTPUT_ENABLE(CFG_GPIO_SLIC_RESET_N);

	/*
	 * Setup GPIO's for PCI INTA & INTB
	 */
	GPIO_OUTPUT_DISABLE(CFG_GPIO_PCI_INTA_N);
	GPIO_INT_ACT_LOW_SET(CFG_GPIO_PCI_INTA_N);
	GPIO_OUTPUT_DISABLE(CFG_GPIO_PCI_INTB_N);
	GPIO_INT_ACT_LOW_SET(CFG_GPIO_PCI_INTB_N);

	/*
	 * Setup GPIO's for 33MHz clock output
	 */
	*IXP425_GPIO_GPCLKR = 0x01FF01FF;
	GPIO_OUTPUT_ENABLE(CFG_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE(CFG_GPIO_EXTBUS_CLK);
#endif

	return 0;
}

/*
 * Check Board Identity
 */
int checkboard(void)
{
	char *s = getenv("serial#");

#ifdef CONFIG_IXDPG425
	puts("Board: IXDPG425 - Intel Network Gateway Reference Platform");
#else
	puts("Board: IXDP425 - Intel Development Platform");
#endif

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

	return (0);
}

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
extern struct pci_controller hose;
extern void pci_ixp_init(struct pci_controller * hose);

void pci_init_board(void)
{
	extern void pci_ixp_init (struct pci_controller *hose);

	pci_ixp_init(&hose);
}
#endif
