/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * PCI initialisation for the MPC10x.
 */

#include <common.h>
#include <pci.h>
#include <mpc106.h>

#ifdef CONFIG_PCI

struct pci_controller local_hose;

void pci_init_board(void)
{
    struct pci_controller* hose = (struct pci_controller *)&local_hose;
    u16 reg16;

    hose->first_busno = 0;
    hose->last_busno = 0xff;

    pci_set_region(hose->regions + 0,
	CONFIG_SYS_PCI_MEMORY_BUS,
	CONFIG_SYS_PCI_MEMORY_PHYS,
	CONFIG_SYS_PCI_MEMORY_SIZE,
	PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

    /* PCI memory space */
    pci_set_region(hose->regions + 1,
	CONFIG_SYS_PCI_MEM_BUS,
	CONFIG_SYS_PCI_MEM_PHYS,
	CONFIG_SYS_PCI_MEM_SIZE,
	PCI_REGION_MEM);

    /* ISA/PCI memory space */
    pci_set_region(hose->regions + 2,
	CONFIG_SYS_ISA_MEM_BUS,
	CONFIG_SYS_ISA_MEM_PHYS,
	CONFIG_SYS_ISA_MEM_SIZE,
	PCI_REGION_MEM);

    /* PCI I/O space */
    pci_set_region(hose->regions + 3,
	CONFIG_SYS_PCI_IO_BUS,
	CONFIG_SYS_PCI_IO_PHYS,
	CONFIG_SYS_PCI_IO_SIZE,
	PCI_REGION_IO);

    /* ISA/PCI I/O space */
    pci_set_region(hose->regions + 4,
	CONFIG_SYS_ISA_IO_BUS,
	CONFIG_SYS_ISA_IO_PHYS,
	CONFIG_SYS_ISA_IO_SIZE,
	PCI_REGION_IO);

    hose->region_count = 5;

    pci_setup_indirect(hose,
	MPC106_REG_ADDR,
	MPC106_REG_DATA);

    pci_register_hose(hose);

    hose->last_busno = pci_hose_scan(hose);

    /* Initialises the MPC10x PCI Configuration regs. */
    pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
    reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
    pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg16);

    /* Clear non-reserved bits in status register */
    pci_write_config_word(PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
}

#endif /* CONFIG_PCI */
