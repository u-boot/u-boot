/*
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
/*
 * Initialize PCI Devices, report devices found.
 */
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc85xxads_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_IDSEL_NUMBER, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ }
};
#endif

struct pci_controller local_hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_mpc85xxads_config_table,
#endif
};

void pci_init_board(void)
{
    struct pci_controller* hose = (struct pci_controller *)&local_hose;
    volatile immap_t    *immap = (immap_t *)CFG_CCSRBAR;
    volatile ccsr_pcix_t *pcix = &immap->im_pcix;

    u16 reg16;

    hose->first_busno = 0;
    hose->last_busno = 0xff;

    pci_set_region(hose->regions + 0,
	CFG_PCI_MEM_BASE,
	CFG_PCI_MEM_PHYS,
	(CFG_PCI_MEM_SIZE/2),
	PCI_REGION_MEM);

    pci_set_region(hose->regions + 1,
	(CFG_PCI_MEM_BASE+0x08000000),
	(CFG_PCI_MEM_PHYS+0x08000000),
	0x1000000, /* 16M */
	PCI_REGION_IO);

    hose->region_count = 2;

    pci_setup_indirect(hose,
	(CFG_IMMR+0x8000),
	(CFG_IMMR+0x8004));

    pci_register_hose(hose);

    hose->last_busno = pci_hose_scan(hose);

    pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
    reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
    pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg16);

    /* Clear non-reserved bits in status register */
    pci_write_config_word(PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
    pci_write_config_byte(PCI_BDF(0,0,0), PCI_LATENCY_TIMER,0x80);

    pcix->potar1   = (CFG_PCI_MEM_BASE >> 12) & 0x000fffff;
    pcix->potear1  = 0x00000000;
    pcix->powbar1  = (CFG_PCI_MEM_BASE >> 12) & 0x000fffff;
    pcix->powbear1 = 0x00000000;
    pcix->powar1   = 0x8004401a; /* 128M MEM space */
    pcix->potar2   = ((CFG_PCI_MEM_BASE + 0x08000000) >> 12)  & 0x000fffff;
    pcix->potear2  = 0x00000000;
    pcix->powbar2  = ((CFG_PCI_MEM_BASE + 0x08000000) >> 12) && 0x000fffff;
    pcix->powbear2 = 0x00000000;
    pcix->powar2   = 0x80088017; /* 16M IO  space */
    pcix->pitar1 = 0x00000000;
    pcix->piwbar1 = 0x00000000;
    pcix->piwar1 = 0xa0F5501f;

}
#endif /* CONFIG_PCI */
