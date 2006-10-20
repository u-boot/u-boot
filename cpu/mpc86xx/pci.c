/*
 * Copyright (C) Freescale Semiconductor,Inc.
 * 2005, 2006. All rights reserved.
 *
 * Ed Swarthout (ed.swarthout@freescale.com)
 * Jason Jin (Jason.jin@freescale.com)
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
 * PCIE Configuration space access support for PCIE Bridge
 */
#include <common.h>
#include <pci.h>

#if defined(CONFIG_PCI)
void
pci_mpc86xx_init(struct pci_controller *hose)
{
	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_pex_t *pcie1 = &immap->im_pex1;
	u16 temp16;
	u32 temp32;

	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint host1_agent = (gur->porbmsr & MPC86xx_PORBMSR_HA) >> 17;
	uint pcie1_host = (host1_agent == 2) || (host1_agent == 3);
	uint pcie1_agent = (host1_agent == 0) || (host1_agent == 1);
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC86xx_PORDEVSR_IO_SEL) >> 16;

	if ((io_sel == 2 || io_sel == 3 || io_sel == 5 || io_sel == 6 ||
	     io_sel == 7 || io_sel == 0xf)
	    && !(devdisr & MPC86xx_DEVDISR_PCIEX1)) {
		printf("PCI-EXPRESS 1: Configured as %s \n",
		       pcie1_agent ? "Agent" : "Host");
		if (pcie1_agent)
			return;	/*Don't scan bus when configured as agent */
		printf("               Scanning PCIE bus");
		debug("0x%08x=0x%08x ",
		      &pcie1->pme_msg_det,
		      pcie1->pme_msg_det);
		if (pcie1->pme_msg_det) {
			pcie1->pme_msg_det = 0xffffffff;
			debug(" with errors.  Clearing.  Now 0x%08x",
			      pcie1->pme_msg_det);
		}
		debug("\n");
	} else {
		printf("PCI-EXPRESS 1 disabled!\n");
		return;
	}

	/*
	 * Set first_bus=0 only skipped B0:D0:F0 which is
	 * a reserved device in M1575, but make it easy for
	 * most of the scan process.
	 */
	hose->first_busno = 0x00;
	hose->last_busno = 0xfe;

	pcie_setup_indirect(hose, (CFG_IMMR + 0x8000), (CFG_IMMR + 0x8004));

	pci_hose_read_config_word(hose,
				  PCI_BDF(0, 0, 0), PCI_COMMAND, &temp16);
	temp16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER |
	    PCI_COMMAND_MEMORY | PCI_COMMAND_IO;
	pci_hose_write_config_word(hose,
				   PCI_BDF(0, 0, 0), PCI_COMMAND, temp16);

	pci_hose_write_config_word(hose, PCI_BDF(0, 0, 0), PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(hose,
				   PCI_BDF(0, 0, 0), PCI_LATENCY_TIMER, 0x80);

	pci_hose_read_config_dword(hose, PCI_BDF(0, 0, 0), PCI_PRIMARY_BUS,
				   &temp32);
	temp32 = (temp32 & 0xff000000) | (0xff) | (0x0 << 8) | (0xfe << 16);
	pci_hose_write_config_dword(hose, PCI_BDF(0, 0, 0), PCI_PRIMARY_BUS,
				    temp32);

	pcie1->powar1 = 0;
	pcie1->powar2 = 0;
	pcie1->piwar1 = 0;
	pcie1->piwar1 = 0;

	pcie1->powbar1 = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pcie1->powar1 = 0x8004401c;	/* 512M MEM space */
	pcie1->potar1 = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pcie1->potear1 = 0x00000000;

	pcie1->powbar2 = (CFG_PCI1_IO_BASE >> 12) & 0x000fffff;
	pcie1->powar2 = 0x80088017;	/* 16M IO space */
	pcie1->potar2 = 0x00000000;
	pcie1->potear2 = 0x00000000;

	pcie1->pitar1 = 0x00000000;
	pcie1->piwbar1 = 0x00000000;
	/* Enable, Prefetch, Local Mem, * Snoop R/W, 2G */
	pcie1->piwar1 = 0xa0f5501e;

	pci_set_region(hose->regions + 0,
		       CFG_PCI_MEMORY_BUS,
		       CFG_PCI_MEMORY_PHYS,
		       CFG_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	pci_set_region(hose->regions + 1,
		       CFG_PCI1_MEM_BASE,
		       CFG_PCI1_MEM_PHYS,
		       CFG_PCI1_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose->regions + 2,
		       CFG_PCI1_IO_BASE,
		       CFG_PCI1_IO_PHYS,
		       CFG_PCI1_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 3;

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
	debug("pcie_mpc86xx_init: last_busno %x\n", hose->last_busno);
	debug("pcie_mpc86xx init: current_busno %x\n ", hose->current_busno);

	printf("....PCIE1 scan & enumeration done\n");
}
#endif				/* CONFIG_PCI */
