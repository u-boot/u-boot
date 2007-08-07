/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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
#define DEBUG
#include <common.h>

#ifdef CONFIG_FSL_PCI_INIT

/*
 * PCI/PCIE Controller initialization for mpc85xx/mpc86xx soc's
 *
 * Initialize controller and call the common driver/pci pci_hose_scan to
 * scan for bridges and devices.
 *
 * Hose fields which need to be pre-initialized by board specific code:
 *   regions[]
 *   first_busno
 *
 * Fields updated:
 *   last_busno
 */

#include <pci.h>
#include <asm/immap_fsl_pci.h>

void pciauto_prescan_setup_bridge(struct pci_controller *hose,
				pci_dev_t dev, int sub_bus);
void pciauto_postscan_setup_bridge(struct pci_controller *hose,
				pci_dev_t dev, int sub_bus);

void pciauto_config_init(struct pci_controller *hose);
void
fsl_pci_init(struct pci_controller *hose)
{
	u16 temp16;
	u32 temp32;
	int busno = hose->first_busno;
	int enabled;
	u16 ltssm;
	u8 temp8;
	int r;
	int bridge;
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) hose->cfg_addr;
	pci_dev_t dev = PCI_BDF(busno,0,0);

	/* Initialize ATMU registers based on hose regions and flags */
	volatile pot_t *po=&pci->pot[1];	/* skip 0 */
	volatile pit_t *pi=&pci->pit[0];	/* ranges from: 3 to 1 */

#ifdef DEBUG
	int neg_link_w;
#endif

	for (r=0; r<hose->region_count; r++) {
		if (hose->regions[r].flags & PCI_REGION_MEMORY) { /* inbound */
			pi->pitar = (hose->regions[r].bus_start >> 12) & 0x000fffff;
			pi->piwbar = (hose->regions[r].phys_start >> 12) & 0x000fffff;
			pi->piwbear = 0;
			pi->piwar = PIWAR_EN | PIWAR_PF | PIWAR_LOCAL |
				PIWAR_READ_SNOOP | PIWAR_WRITE_SNOOP |
				(__ilog2(hose->regions[r].size) - 1);
			pi++;
		} else { /* Outbound */
			po->powbar = (hose->regions[r].phys_start >> 12) & 0x000fffff;
			po->potar = (hose->regions[r].bus_start >> 12) & 0x000fffff;
			po->potear = 0;
			if (hose->regions[r].flags & PCI_REGION_IO)
				po->powar = POWAR_EN | POWAR_IO_READ | POWAR_IO_WRITE |
					(__ilog2(hose->regions[r].size) - 1);
			else
				po->powar = POWAR_EN | POWAR_MEM_READ | POWAR_MEM_WRITE |
					(__ilog2(hose->regions[r].size) - 1);
			po++;
		}
	}

	pci_register_hose(hose);
	pciauto_config_init(hose);	/* grab pci_{mem,prefetch,io} */
	hose->current_busno = hose->first_busno;

	pci->pedr = 0xffffffff;		/* Clear any errors */
	pci->peer = 0xffffffff;		/* Enable Error Interupts */
	pci_hose_read_config_dword (hose, dev, PCI_DCR, &temp32);
	temp32 |= 0xf000e;		/* set URR, FER, NFER (but not CER) */
	pci_hose_write_config_dword(hose, dev, PCI_DCR, temp32);

	pci_hose_read_config_byte (hose, dev, PCI_HEADER_TYPE, &temp8);
	bridge = temp8 & PCI_HEADER_TYPE_BRIDGE; /* Bridge, such as pcie */

	if ( bridge ) {

		pci_hose_read_config_word(hose, dev, PCI_LTSSM, &ltssm);
		enabled = ltssm >= PCI_LTSSM_L0;

		if (!enabled) {
			debug("....PCIE link error.  Skipping scan."
			      "LTSSM=0x%02x\n", temp16);
			hose->last_busno = hose->first_busno;
			return;
		}

		pci->pme_msg_det = 0xffffffff;
		pci->pme_msg_int_en = 0xffffffff;
#ifdef DEBUG
		pci_hose_read_config_word(hose, dev, PCI_LSR, &temp16);
		neg_link_w = (temp16 & 0x3f0 ) >> 4;
		debug("...PCIE LTSSM=0x%x, Negotiated link width=%d\n",
		      ltssm, neg_link_w);
#endif
		hose->current_busno++; /* Start scan with secondary */
		pciauto_prescan_setup_bridge(hose, dev, hose->current_busno);

	} else {
#if 0
/* done in pci_hose_config_device() */
		pci_hose_read_config_word(hose, dev, PCI_COMMAND, &temp16);
		temp16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER |
			PCI_COMMAND_MEMORY | PCI_COMMAND_IO;
		pci_hose_write_config_word(hose, dev, PCI_COMMAND, temp16);
		pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);
		pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
#endif
	}

	/* Call setup to allocate PCSRBAR window */
	pciauto_setup_device(hose, dev, 1, hose->pci_mem,
			     hose->pci_prefetch, hose->pci_io);

	printf ("               Scanning PCI bus %02x\n", hose->current_busno);
	hose->last_busno = pci_hose_scan_bus(hose,hose->current_busno);

	if ( bridge ) { /* update limit regs and subordinate busno */
		pciauto_postscan_setup_bridge(hose, dev, hose->last_busno);
	}

	/* Clear all error indications */

	if (pci->pme_msg_det && pci->pme_msg_det != 0xffffffff) {
		debug("pci_fsl_init: pme_msg_det@%x=%x.  Clearing\n",
			&pci->pme_msg_det, pci->pme_msg_det);
		pci->pme_msg_det = 0xffffffff;
	}

	if (pci->pedr) {
		debug("pci_fsl_init: pedr@%x=%x.  Clearing\n",
			&pci->pedr, pci->pedr);
		pci->pedr = 0xffffffff;
	}

	pci_hose_read_config_word (hose, dev, PCI_DSR, &temp16);
	if (temp16) {
		debug("pci_fsl_init: PCI_DSR@%x=%x.  Clearing\n",
			PCI_DSR, temp16);
		pci_hose_write_config_word(hose, dev,
					   PCI_DSR, 0xffff);
	}

	pci_hose_read_config_word (hose, dev, PCI_SEC_STATUS, &temp16);
	if (temp16) {
		debug("pci_fsl_init: PCI_SEC_STATUS@%x=%x.  Clearing\n",
			PCI_SEC_STATUS, temp16);
		pci_hose_write_config_word(hose, dev, PCI_SEC_STATUS, 0xffff);
	}
}

#endif /* CONFIG_FSL_PCI */
