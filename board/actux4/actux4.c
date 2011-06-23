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
#ifdef CONFIG_PCI
#include <pci.h>
#include <asm/arch/ixp425pci.h>
#endif

#include "actux4_hw.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	writel(0xbd113c42, IXP425_EXP_CS1);
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_ACTUX4;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_nPWRON);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_nPWRON);

	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_IORST);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_IORST);

	/* led not populated on board*/
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_LED3);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_LED3);

	/* middle LED */
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_LED2);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_LED2);

	/* right LED */
	/* weak pulldown = LED weak on */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_LED1);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_LED1);

	/* Setup GPIO's for Interrupt inputs */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_USBINTA);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_USBINTB);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_USBINTC);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_RTCINT);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTA);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTB);

	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_USBINTA);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_USBINTB);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_USBINTC);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_RTCINT);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTA);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTB);

	/* Setup GPIO's for 33MHz clock output */
	writel(0x011001FF, IXP425_GPIO_GPCLKR);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_EXTBUS_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_CLK);

	udelay(10000);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_IORST);
	udelay(10000);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_IORST);
	udelay(10000);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_IORST);

	return 0;
}

/* Check Board Identity */
int checkboard(void)
{
	puts("Board: AcTux-4\n");
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

/*
 * Hardcoded flash setup:
 * Flash 0 is a non-CFI SST 39VF020 flash, 8 bit flash / 8 bit bus.
 * Flash 1 is an Intel *16 flash using the CFI driver.
 */
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = 1;
		info->chipwidth = 1;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else
		return 0;
}
