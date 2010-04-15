/*
 * arch/powerpc/kernel/mpc10x_common.c
 *
 * Common routines for the Motorola SPS MPC106, MPC107 and MPC8240 Host bridge,
 * Mem ctlr, EPIC, etc.
 *
 * Author: Mark A. Greer
 *         mgreer@mvista.com
 *
 * Copyright 2001 MontaVista Software Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <mpc824x.h>

void pci_mpc824x_init (struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region(hose->regions + 0,
		       CHRP_PCI_MEMORY_BUS,
		       CHRP_PCI_MEMORY_PHYS,
		       CHRP_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       CHRP_PCI_MEM_BUS,
		       CHRP_PCI_MEM_PHYS,
		       CHRP_PCI_MEM_SIZE,
		       PCI_REGION_MEM);

	/* ISA/PCI memory space */
	pci_set_region(hose->regions + 2,
		       CHRP_ISA_MEM_BUS,
		       CHRP_ISA_MEM_PHYS,
		       CHRP_ISA_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region(hose->regions + 3,
		       CHRP_PCI_IO_BUS,
		       CHRP_PCI_IO_PHYS,
		       CHRP_PCI_IO_SIZE,
		       PCI_REGION_IO);

	/* ISA/PCI I/O space */
	pci_set_region(hose->regions + 4,
		       CHRP_ISA_IO_BUS,
		       CHRP_ISA_IO_PHYS,
		       CHRP_ISA_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 5;

	pci_setup_indirect(hose,
			   CHRP_REG_ADDR,
			   CHRP_REG_DATA);

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
}

#endif
