/*
 * Copyright 2005 Freescale Semiconductor.
 * Ed Swarthout (ed.swarthout@freescale.com)
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
 * PEX Configuration space access support for MPC85xx PEX Bridge
 */
#include <common.h>
#include <pci.h>


#if defined(CONFIG_PCI)

void
pci_mpc86xx_init(struct pci_controller *hose)
{
	volatile immap_t    *immap = (immap_t *)CFG_CCSRBAR;
	volatile ccsr_pex_t *pex1 = &immap->im_pex1;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint host1_agent = (gur->porbmsr & MPC86xx_PORBMSR_HA) >> 17;
	uint pex1_host =   (host1_agent == 2) || (host1_agent == 3);

	u16 reg16, reg16_1, reg16_2, reg16_3;
	u32 reg32, i;

        ulong addr, data;

        
        uint pex1_agent =  (host1_agent == 0) || (host1_agent == 1);
        uint devdisr = gur->devdisr;
        uint io_sel = (gur->pordevsr & MPC86xx_PORDEVSR_IO_SEL) >> 16;
        
        if ((io_sel==2 || io_sel==3 || io_sel==5 || io_sel==6 || io_sel==7 || io_sel==0xF ) && !(devdisr & MPC86xx_DEVDISR_PCIEX1)){
                printf ("PCI-EXPRESS 1: Configured as %s \n",
                        pex1_agent ? "Agent" : "Host");
                printf ("               Scanning PCI bus");
                debug("0x%08x=0x%08x ", &pex1->pme_msg_det,pex1->pme_msg_det);
                if (pex1->pme_msg_det) {
                        pex1->pme_msg_det = 0xffffffff;
                        debug (" with errors.  Clearing.  Now 0x%08x",pex1->pme_msg_det);
                }
                debug ("\n");
        }
        
        
	hose->first_busno = 0;
	hose->last_busno = 0x7f;

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

	/*
	 * Hose scan.
	 */
	pci_register_hose(hose);

        //#define MPC8548_REV1_PEX12_ERRATA
#ifdef MPC8548_REV1_PEX12_ERRATA
	/* can only read/write 4 bytes */
	pci_read_config_dword (PCI_BDF(0,0,0), PCI_VENDOR_ID, &reg32);
	printf("pex_mpc85xx_init: pex cr %2x %8x\n",PCI_VENDOR_ID, reg32);

	pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg32);
	reg32 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg32);
#else
	pci_read_config_word (PCI_BDF(0,0,0), PCI_VENDOR_ID, &reg16);
	debug("pex_mpc86xx_init: read %2x %4x\n",PCI_VENDOR_ID, reg16);
	pci_read_config_word (PCI_BDF(0,0,0), PCI_DEVICE_ID, &reg16);
	debug("pex_mpc86xx_init: read %2x %4x\n",PCI_DEVICE_ID, reg16);

	pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR |  PCI_COMMAND_PARITY | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(PCI_BDF(0,0,0), PCI_COMMAND, reg16);

        pci_read_config_word (PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
	debug("pex_mpc86xx_init: read %2x %4x\n",PCI_COMMAND, reg16);

        
#endif

	/*
	 * Clear non-reserved bits in status register.
	 */
	//	pci_write_config_word(PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
	//	pci_write_config_byte(PCI_BDF(0,0,0), PCI_LATENCY_TIMER,0x80);

	pex1->powbar1  = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pex1->powar1   = 0x8004401c;	/* 512M MEM space */
	pex1->potar1   = (CFG_PCI1_MEM_BASE >> 12) & 0x000fffff;
	pex1->potear1  = 0x00000000;

        pex1->powbar2  = (CFG_PCI1_IO_BASE >> 12) & 0x000fffff;
	pex1->powar2   = 0x80088017;	/* 16M IO space */
	pex1->potar2   = 0x00000000;
	pex1->potear2  = 0x00000000;


	if (!pex1->piwar1) {
		pex1->pitar1 = 0x00000000;
		pex1->piwbar1 = (0x80000000 >> 12 ) & 0x000fffff;
		pex1->piwar1 = 0xa0f5501e;	/* Enable, Prefetch, Local Mem,
						 * Snoop R/W, 2G */
	}

	pex1->pitar2 = 0x00000000;
        pex1->piwbar2 = (0xe2000000 >> 12 ) & 0x000fffff;
        pex1->piwar2 = 0xa0f5501e;	/* Enable, Prefetch, Local Mem,
                                           

        
/* 	if (pex1_host) { */
/* #ifdef MPC8548_REV1_PEX12_ERRATA */
/* 		pci_write_config_dword (PCI_BDF(0,0,0), 0x18, 0x00ff0100); */
/* #else */



                *(u32 *)(0xf8008000)= 0x80000000;
                debug("Received data for addr 0x%08lx is 0x%08lx\n", *(u32*)(0xf8008000), *(u32*)(0xf8008004));


                pci_write_config_byte(PCI_BDF(0,0,0), PCI_PRIMARY_BUS,0x20);
                pci_write_config_byte(PCI_BDF(0,0,0), PCI_SECONDARY_BUS,0x00);
		pci_write_config_byte(PCI_BDF(0,0,0), PCI_SUBORDINATE_BUS,0x1F);
/* #endif */


                *(u32 *)(0xf8008000)= 0x80200000;
                debug("Received data for addr 0x%08lx is 0x%08lx\n", *(u32*)(0xf8008000), *(u32*)(0xf8008004));

                *(u32 *)(0xf8008000)= 0x80200000;
                debug("Received data for addr 0x%08lx is 0x%08lx\n", *(u32*)(0xf8008000), *(u32*)(0xf8008004));

                *(u32 *)(0xf8008000)= 0x80200000;
                debug("Received data for addr 0x%08lx is 0x%08lx\n", *(u32*)(0xf8008000), *(u32*)(0xf8008004));


                
		hose->last_busno = pci_hose_scan(hose);
                hose->last_busno = 0x21;
		debug("pex_mpc86xx_init: last_busno %x\n",hose->last_busno);
                debug("pex_mpc86xx init: current_busno %x\n ",hose->current_busno);


                printf("....PCI scan & enumeration done\n");

/*                 *(u32 *)(0xf8008000)= 0x80000000 | (0x12 << 11); */
/*                 printf("Received data for addr 0x%08lx is 0x%08lx\n", *(u32*)(0xf8008000), *(u32*)(0xf8008004)); */
                
/* 		if (hose->last_busno < 1) { */
/* 			hose->last_busno=1;  /\*Hack*\/ */
/*                 } else { */
/*                    hose->last_busno = 0; */
/*                 } */
/*}*/
/*         pci_read_config_dword (PCI_BDF(1,0,0), 0x18, &reg32); */
/*         printf("pex_mpc86xx_init: pex cr %2x %8x\n",0x18, reg32); */


}
#endif /* CONFIG_PCI */
