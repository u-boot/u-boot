/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright (C) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
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
 * PCI Configuration space access support for MPC85xx PCI Bridge
 */
#include <common.h>
#include <asm/cpm_85xx.h>
#include <pci.h>


#if defined(CONFIG_PCI)

void
pci_mpc85xx_init(struct pci_controller *hose)
{
	volatile immap_t    *immap = (immap_t *)CFG_CCSRBAR;
	volatile ccsr_pcix_t *pcix = &immap->im_pcix;

	u16 reg16;

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_set_region(hose->regions + 0,
		       CFG_PCI1_MEM_BASE,
		       CFG_PCI1_MEM_PHYS,
		       CFG_PCI1_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose->regions + 1,
		       CFG_PCI1_IO_BASE,
		       CFG_PCI1_IO_PHYS,
		       CFG_PCI1_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 2;

	pci_setup_indirect(hose,
			   (CFG_IMMR+0x8000),
			   (CFG_IMMR+0x8004));

	pcix->potar1   = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pcix->potear1  = 0x00000000;
	pcix->powbar1  = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pcix->powbear1 = 0x00000000;
	pcix->powar1   = 0x8004401c;	/* 512M MEM space */

	pcix->potar2   = 0x00000000;
	pcix->potear2  = 0x00000000;
	pcix->powbar2  = (CFG_PCI1_IO_BASE >> 12) & 0x000fffff;
	pcix->powbear2 = 0x00000000;
	pcix->powar2   = 0x80088017;	/* 16M IO space */

	pcix->pitar1 = 0x00000000;
	pcix->piwbar1 = 0x00000000;
	pcix->piwar1 = 0xa0f5501e;	/* Enable, Prefetch, Local Mem,
					 * Snoop R/W, 2G */

	/*
	 * Hose scan.
	 */
	pci_register_hose(hose);

	pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_write_config_word(PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
	pci_write_config_byte(PCI_BDF(0,0,0), PCI_LATENCY_TIMER,0x80);

#if defined(CONFIG_MPC8555CDS) || defined(CONFIG_MPC8541CDS)
	/*
	 * This is a SW workaround for an apparent HW problem
	 * in the PCI controller on the MPC85555/41 CDS boards.
	 * The first config cycle must be to a valid, known
	 * device on the PCI bus in order to trick the PCI
	 * controller state machine into a known valid state.
	 * Without this, the first config cycle has the chance
	 * of hanging the controller permanently, just leaving
	 * it in a semi-working state, or leaving it working.
	 *
	 * Pick on the Tundra, Device 17, to get it right.
	 */
	{
		u8 header_type;

		pci_hose_read_config_byte(hose,
					  PCI_BDF(0,17,0),
					  PCI_HEADER_TYPE,
					  &header_type);
	}
#endif

	hose->last_busno = pci_hose_scan(hose);
}

#endif /* CONFIG_PCI */
