/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <asm/io.h>
#include <miiphy.h>

#include "actux1_hw.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init (void)
{
	gd->bd->bi_arch_number = MACH_TYPE_ACTUX1;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	GPIO_OUTPUT_CLEAR (CFG_GPIO_IORST);
	GPIO_OUTPUT_ENABLE (CFG_GPIO_IORST);

	/* Setup GPIO's for PCI INTA */
	GPIO_OUTPUT_DISABLE (CFG_GPIO_PCI1_INTA);
	GPIO_INT_ACT_LOW_SET (CFG_GPIO_PCI1_INTA);

	/* Setup GPIO's for 33MHz clock output */
	GPIO_OUTPUT_ENABLE (CFG_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE (CFG_GPIO_EXTBUS_CLK);
	*IXP425_GPIO_GPCLKR = 0x011001FF;

	/* CS5: Debug port */
	*IXP425_EXP_CS5 = 0x9d520003;
	/* CS6: HwRel */
	*IXP425_EXP_CS6 = 0x81860001;
	/* CS7: LEDs */
	*IXP425_EXP_CS7 = 0x80900003;

	udelay (533);
	GPIO_OUTPUT_SET (CFG_GPIO_IORST);

	ACTUX1_LED1 (2);
	ACTUX1_LED2 (2);
	ACTUX1_LED3 (0);
	ACTUX1_LED4 (0);
	ACTUX1_LED5 (0);
	ACTUX1_LED6 (0);
	ACTUX1_LED7 (0);

	ACTUX1_HS (ACTUX1_HS_DCD);

	return 0;
}

/*
 * Check Board Identity
 */
int checkboard (void)
{
	char *s = getenv ("serial#");

	puts ("Board: AcTux-1 rev.");
	putc (ACTUX1_BOARDREL + 'A' - 1);

	if (s != NULL) {
		puts (", serial# ");
		puts (s);
	}
	putc ('\n');

	return (0);
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * 0 = reserved
 * 1 = Rev. A
 * 2 = Rev. B
 *************************************************************************/
u32 get_board_rev (void)
{
	return ACTUX1_BOARDREL;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return (0);
}

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
extern struct pci_controller hose;
extern void pci_ixp_init (struct pci_controller *hose);

void pci_init_board (void)
{
	extern void pci_ixp_init (struct pci_controller *hose);
	pci_ixp_init (&hose);
}
#endif

void reset_phy (void)
{
	u16 id1, id2;

	/* initialize the PHY */
	miiphy_reset ("NPE0", CONFIG_PHY_ADDR);

	miiphy_read ("NPE0", CONFIG_PHY_ADDR, PHY_PHYIDR1, &id1);
	miiphy_read ("NPE0", CONFIG_PHY_ADDR, PHY_PHYIDR2, &id2);

	id2 &= 0xFFF0;		/* mask out revision bits */

	if (id1 == 0x13 && id2 == 0x78e0) {
		/*
		 * LXT971/LXT972 PHY: set LED outputs:
		 * LED1(green) = Link/ACT,
		 * LED2 (unused) = LINK,
		 * LED3(red) = Coll
		 */
		miiphy_write ("NPE0", CONFIG_PHY_ADDR, 20, 0xD432);
	} else if (id1 == 0x143 && id2 == 0xbc30) {
		/* BCM5241: default values are OK */
	} else
		printf ("unknown ethernet PHY ID: %x %x\n", id1, id2);
}
