/*
 * Board initialize code for TANBAC Evaluation board TB0229.
 *
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <asm/addrspace.h>
#include <asm/inca-ip.h>
#include <pci.h>

unsigned long mips_io_port_base = 0;

#if defined(CONFIG_PCI)
static struct pci_controller hose;

void pci_init_board (void)
{
	init_vr4131_pci(&hose);
}
#endif


long int initdram(int board_type)
{
	return get_ram_size (CFG_SDRAM_BASE, 0x8000000);
}


int checkboard (void)
{
	printf("Board: TANBAC TB0229 ");
	printf("(CPU Speed %d MHz)\n", (int)CPU_CLOCK_RATE/1000000);

	return 0;
}
