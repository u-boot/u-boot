/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
    u32 reg32;
    u16 reg16;

    hose->first_busno = 0;
    hose->last_busno = 0xff;

    pci_set_region(hose->regions + 0,
	CONFIG_SYS_PCI_MEMORY_BUS,
	CONFIG_SYS_PCI_MEMORY_PHYS,
    /*
    * Attention: pci_hose_phys_to_bus() failes in address compare,
    * so we need (CONFIG_SYS_PCI_MEMORY_SIZE-1)
    */
	CONFIG_SYS_PCI_MEMORY_SIZE-1,
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
    pci_read_config_dword (PCI_BDF(0,0,0), PCI_PICR2, &reg32);
    reg32 |= PICR2_CF_SNOOP_WS(3) |
	     PICR2_CF_FLUSH_L2 |
	     PICR2_CF_L2_HIT_DELAY(3) |
	     PICR2_CF_APHASE_WS(3);
    reg32 &= ~(PICR2_L2_EN | PICR2_L2_UPDATE_EN);
    pci_write_config_dword (PCI_BDF(0,0,0), PCI_PICR2, reg32);

    pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
    reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
    pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg16);

    /* Clear non-reserved bits in status register */
    pci_write_config_word(PCI_BDF(0,0,0), PCI_STATUS, 0xffff);

    pci_read_config_dword (PCI_BDF(0,0,0), PCI_PICR1, &reg32);
    reg32 |= PICR1_CF_CBA(63) |
	     PICR1_CF_BREAD_WS(2) |
	     PICR1_MCP_EN |
	     PICR1_CF_DPARK |
	     PICR1_PROC_TYPE_604 |
	     PICR1_CF_LOOP_SNOOP |
	     PICR1_CF_APARK;
    pci_write_config_dword (PCI_BDF(0,0,0), PCI_PICR1, reg32);
}

#endif /* CONFIG_PCI */
