/*
 * (C) Copyright 2004, Freescale Inc.
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com
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
#include <mpc8220.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <pci.h>

long int initdram (int board_type)
{
	ulong size;

	size = dramSetup ();

	return get_ram_size(CFG_SDRAM_BASE, size);
}

int checkboard (void)
{
	puts ("Board: Sorcery-C MPC8220\n");

	return 0;
}

#if defined(CONFIG_PCI)
/*
 * Initialize PCI devices, report devices found.
 */
static struct pci_controller hose;

#endif /* CONFIG_PCI */

void pci_init_board (void)
{
#ifdef CONFIG_PCI
	extern void pci_mpc8220_init (struct pci_controller *hose);
	pci_mpc8220_init (&hose);
#endif /* CONFIG_PCI */
}
