/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <pci.h>

#include "hardware.h"
#include "pcippc2.h"

struct pci_controller local_hose, cpci_hose;

static u32	cpc710_mapped_ram;

  /* Enable PCI retry timeouts
   */
void cpc710_pci_enable_timeout (void)
{
  out32(BRIDGE(LOCAL, CFGADDR), 0x50000080);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, CFGDATA), 0x32000000);
  iobarrier_rw();

  out32(BRIDGE(CPCI, CFGADDR), 0x50000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), 0x32000000);
  iobarrier_rw();
}

void cpc710_pci_init (void)
{
  u32			sdram_size = pcippc2_sdram_size();

  cpc710_mapped_ram = sdram_size < PCI_MEMORY_MAXSIZE ?
		      sdram_size : PCI_MEMORY_MAXSIZE;

    /* Select the local PCI
     */
  out32(REG(CPC0, PCICNFR), 0x80000002);
  iobarrier_rw();

  out32(REG(CPC0, PCIBAR), BRIDGE_LOCAL_PHYS);
  iobarrier_rw();

    /* Enable PCI bridge address decoding
     */
  out32(REG(CPC0, PCIENB), 0x80000000);
  iobarrier_rw();

    /* Select the CPCI bridge
     */
  out32(REG(CPC0, PCICNFR), 0x80000003);
  iobarrier_rw();

  out32(REG(CPC0, PCIBAR), BRIDGE_CPCI_PHYS);
  iobarrier_rw();

    /* Enable PCI bridge address decoding
     */
  out32(REG(CPC0, PCIENB), 0x80000000);
  iobarrier_rw();

    /* Disable configuration accesses
     */
  out32(REG(CPC0, PCICNFR), 0x80000000);
  iobarrier_rw();

    /* Initialise the local PCI
     */
  out32(BRIDGE(LOCAL, CRR), 0x7c000000);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, PCIDG), 0x40000000);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, PIBAR), BRIDGE_LOCAL_IO_BUS);
  out32(BRIDGE(LOCAL, SIBAR), BRIDGE_LOCAL_IO_PHYS);
  out32(BRIDGE(LOCAL, IOSIZE), -BRIDGE_LOCAL_IO_SIZE);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, PMBAR), BRIDGE_LOCAL_MEM_BUS);
  out32(BRIDGE(LOCAL, SMBAR), BRIDGE_LOCAL_MEM_PHYS);
  out32(BRIDGE(LOCAL, MSIZE), -BRIDGE_LOCAL_MEM_SIZE);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, PR), 0x00ffe000);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, ACR), 0xfe000000);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, PSBAR), PCI_MEMORY_BUS >> 24);
  out32(BRIDGE(LOCAL, BARPS), PCI_MEMORY_PHYS >> 24);
  out32(BRIDGE(LOCAL, PSSIZE), 256 - (cpc710_mapped_ram >> 24));
  iobarrier_rw();

    /* Initialise the CPCI bridge
     */
  out32(BRIDGE(CPCI, CRR), 0x7c000000);
  iobarrier_rw();
  out32(BRIDGE(CPCI, PCIDG), 0xC0000000);
  iobarrier_rw();
  out32(BRIDGE(CPCI, PIBAR), BRIDGE_CPCI_IO_BUS);
  out32(BRIDGE(CPCI, SIBAR), BRIDGE_CPCI_IO_PHYS);
  out32(BRIDGE(CPCI, IOSIZE), -BRIDGE_CPCI_IO_SIZE);
  iobarrier_rw();
  out32(BRIDGE(CPCI, PMBAR), BRIDGE_CPCI_MEM_BUS);
  out32(BRIDGE(CPCI, SMBAR), BRIDGE_CPCI_MEM_PHYS);
  out32(BRIDGE(CPCI, MSIZE), -BRIDGE_CPCI_MEM_SIZE);
  iobarrier_rw();
  out32(BRIDGE(CPCI, PR), 0x80ffe000);
  iobarrier_rw();
  out32(BRIDGE(CPCI, ACR), 0xdf000000);
  iobarrier_rw();
  out32(BRIDGE(CPCI, PSBAR), PCI_MEMORY_BUS >> 24);
  out32(BRIDGE(CPCI, BARPS), PCI_MEMORY_PHYS >> 24);
  out32(BRIDGE(CPCI, PSSIZE), 256 - (cpc710_mapped_ram >> 24));
  iobarrier_rw();

    /* Local PCI
     */

  out32(BRIDGE(LOCAL, CFGADDR), 0x04000080);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, CFGDATA), 0x56010000);
  iobarrier_rw();

  out32(BRIDGE(LOCAL, CFGADDR), 0x0c000080);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, CFGDATA), PCI_LATENCY_TIMER_VAL << 16);
  iobarrier_rw();

    /* Set bus and subbus numbers
     */
  out32(BRIDGE(LOCAL, CFGADDR), 0x40000080);
  iobarrier_rw();
  out32(BRIDGE(LOCAL, CFGDATA), 0x00000000);
  iobarrier_rw();

  out32(BRIDGE(LOCAL, CFGADDR), 0x50000080);
  iobarrier_rw();
    /* PCI retry timeouts will be enabled later
     */
  out32(BRIDGE(LOCAL, CFGDATA), 0x00000000);
  iobarrier_rw();

    /* CPCI
     */

    /* Set bus and subbus numbers
     */
  out32(BRIDGE(CPCI, CFGADDR), 0x40000080);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), 0x01010000);
  iobarrier_rw();

  out32(BRIDGE(CPCI, CFGADDR), 0x04000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), 0x56010000);
  iobarrier_rw();

  out32(BRIDGE(CPCI, CFGADDR), 0x0c000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), PCI_LATENCY_TIMER_VAL << 16);
  iobarrier_rw();

    /* Write to the PSBAR */
  out32(BRIDGE(CPCI, CFGADDR), 0x10000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), cpu_to_le32(PCI_MEMORY_BUS));
  iobarrier_rw();

    /* Set bus and subbus numbers
     */
  out32(BRIDGE(CPCI, CFGADDR), 0x40000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), 0x01ff0000);
  iobarrier_rw();

  out32(BRIDGE(CPCI, CFGADDR), 0x50000180);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CFGDATA), 0x32000000);
    /* PCI retry timeouts will be enabled later
     */
  out32(BRIDGE(CPCI, CFGDATA), 0x00000000);
  iobarrier_rw();

    /* Remove reset on the PCI buses
     */
  out32(BRIDGE(LOCAL, CRR), 0xfc000000);
  iobarrier_rw();
  out32(BRIDGE(CPCI, CRR), 0xfc000000);
  iobarrier_rw();

  local_hose.first_busno = 0;
  local_hose.last_busno = 0xff;

  /* System memory space */
  pci_set_region(local_hose.regions + 0,
		 PCI_MEMORY_BUS,
		 PCI_MEMORY_PHYS,
		 PCI_MEMORY_MAXSIZE,
		 PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

  /* PCI memory space */
  pci_set_region(local_hose.regions + 1,
		 BRIDGE_LOCAL_MEM_BUS,
		 BRIDGE_LOCAL_MEM_PHYS,
		 BRIDGE_LOCAL_MEM_SIZE,
		 PCI_REGION_MEM);

  /* PCI I/O space */
  pci_set_region(local_hose.regions + 2,
		 BRIDGE_LOCAL_IO_BUS,
		 BRIDGE_LOCAL_IO_PHYS,
		 BRIDGE_LOCAL_IO_SIZE,
		 PCI_REGION_IO);

  local_hose.region_count = 3;

  pci_setup_indirect(&local_hose,
		     BRIDGE_LOCAL_PHYS + HW_BRIDGE_CFGADDR,
		     BRIDGE_LOCAL_PHYS + HW_BRIDGE_CFGDATA);

  pci_register_hose(&local_hose);

  /* Initialize PCI32 bus registers */
  pci_hose_write_config_byte(&local_hose,
			  PCI_BDF(local_hose.first_busno,0,0),
			  CPC710_BUS_NUMBER,
			  local_hose.first_busno);
  pci_hose_write_config_byte(&local_hose,
			  PCI_BDF(local_hose.first_busno,0,0),
			  CPC710_SUB_BUS_NUMBER,
			  local_hose.last_busno);

  local_hose.last_busno = pci_hose_scan(&local_hose);

  /* Write out correct max subordinate bus number for local hose */
  pci_hose_write_config_byte(&local_hose,
			  PCI_BDF(local_hose.first_busno,0,0),
			  CPC710_SUB_BUS_NUMBER,
			  local_hose.last_busno);

  cpci_hose.first_busno = local_hose.last_busno + 1;
  cpci_hose.last_busno = 0xff;

  /* System memory space */
  pci_set_region(cpci_hose.regions + 0,
		 PCI_MEMORY_BUS,
		 PCI_MEMORY_PHYS,
		 PCI_MEMORY_MAXSIZE,
		 PCI_REGION_SYS_MEMORY);

  /* PCI memory space */
  pci_set_region(cpci_hose.regions + 1,
		 BRIDGE_CPCI_MEM_BUS,
		 BRIDGE_CPCI_MEM_PHYS,
		 BRIDGE_CPCI_MEM_SIZE,
		 PCI_REGION_MEM);

  /* PCI I/O space */
  pci_set_region(cpci_hose.regions + 2,
		 BRIDGE_CPCI_IO_BUS,
		 BRIDGE_CPCI_IO_PHYS,
		 BRIDGE_CPCI_IO_SIZE,
		 PCI_REGION_IO);

  cpci_hose.region_count = 3;

  pci_setup_indirect(&cpci_hose,
		     BRIDGE_CPCI_PHYS + HW_BRIDGE_CFGADDR,
		     BRIDGE_CPCI_PHYS + HW_BRIDGE_CFGDATA);

  pci_register_hose(&cpci_hose);

  /* Initialize PCI64 bus registers */
  pci_hose_write_config_byte(&cpci_hose,
			  PCI_BDF(cpci_hose.first_busno,0,0),
			  CPC710_BUS_NUMBER,
			  cpci_hose.first_busno);
  pci_hose_write_config_byte(&cpci_hose,
			  PCI_BDF(cpci_hose.first_busno,0,0),
			  CPC710_SUB_BUS_NUMBER,
			  cpci_hose.last_busno);

  cpci_hose.last_busno = pci_hose_scan(&cpci_hose);

  /* Write out correct max subordinate bus number for cpci hose */
  pci_hose_write_config_byte(&cpci_hose,
			  PCI_BDF(cpci_hose.first_busno,0,0),
			  CPC710_SUB_BUS_NUMBER,
			  cpci_hose.last_busno);
}
