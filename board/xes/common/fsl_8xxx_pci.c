/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <libfdt.h>
#include <fdt_support.h>

int first_free_busno = 0;

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif
#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif
#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif
#ifdef CONFIG_PCIE3
static struct pci_controller pcie3_hose;
#endif

#ifdef CONFIG_MPC8572
/* Correlate host/agent POR bits to usable info. Table 4-14 */
struct host_agent_cfg_t {
	uchar pcie_root[3];
	uchar rio_host;
} host_agent_cfg[8] = {
	{{0, 0, 0}, 0},
	{{0, 1, 1}, 1},
	{{1, 0, 1}, 0},
	{{1, 1, 0}, 1},
	{{0, 0, 1}, 0},
	{{0, 1, 0}, 1},
	{{1, 0, 0}, 0},
	{{1, 1, 1}, 1}
};

/* Correlate port width POR bits to usable info. Table 4-15 */
struct io_port_cfg_t {
	uchar pcie_width[3];
	uchar rio_width;
} io_port_cfg[16] = {
	{{0, 0, 0}, 0},
	{{0, 0, 0}, 0},
	{{4, 0, 0}, 0},
	{{4, 4, 0}, 0},
	{{0, 0, 0}, 0},
	{{0, 0, 0}, 0},
	{{0, 0, 0}, 4},
	{{4, 2, 2}, 0},
	{{0, 0, 0}, 0},
	{{0, 0, 0}, 0},
	{{0, 0, 0}, 0},
	{{4, 0, 0}, 4},
	{{4, 0, 0}, 4},
	{{0, 0, 0}, 4},
	{{0, 0, 0}, 4},
	{{8, 0, 0}, 0},
};
#elif defined CONFIG_MPC8548
/* Correlate host/agent POR bits to usable info. Table 4-12 */
struct host_agent_cfg_t {
	uchar pci_host[2];
	uchar pcie_root[1];
	uchar rio_host;
} host_agent_cfg[8] = {
	{{1, 1}, {0}, 0},
	{{1, 1}, {1}, 0},
	{{1, 1}, {0}, 1},
	{{0, 0}, {0}, 0}, /* reserved */
	{{0, 1}, {1}, 0},
	{{1, 1}, {1}, 0},
	{{0, 1}, {1}, 1},
	{{1, 1}, {1}, 1}
};

/* Correlate port width POR bits to usable info. Table 4-13 */
struct io_port_cfg_t {
	uchar pcie_width[1];
	uchar rio_width;
} io_port_cfg[8] = {
	{{0}, 0},
	{{0}, 0},
	{{0}, 0},
	{{4}, 4},
	{{4}, 4},
	{{0}, 4},
	{{0}, 4},
	{{8}, 0},
};
#elif defined CONFIG_MPC86xx
/* Correlate host/agent POR bits to usable info. Table 4-17 */
struct host_agent_cfg_t {
	uchar pcie_root[2];
	uchar rio_host;
} host_agent_cfg[8] = {
	{{0, 0}, 0},
	{{1, 0}, 1},
	{{0, 1}, 0},
	{{1, 1}, 1}
};

/* Correlate port width POR bits to usable info. Table 4-16 */
struct io_port_cfg_t {
	uchar pcie_width[2];
	uchar rio_width;
} io_port_cfg[16] = {
	{{0, 0}, 0},
	{{0, 0}, 0},
	{{8, 0}, 0},
	{{8, 8}, 0},
	{{0, 0}, 0},
	{{8, 0}, 4},
	{{8, 0}, 4},
	{{8, 0}, 4},
	{{0, 0}, 0},
	{{0, 0}, 4},
	{{0, 0}, 4},
	{{0, 0}, 4},
	{{0, 0}, 0},
	{{0, 0}, 0},
	{{0, 8}, 0},
	{{8, 8}, 0},
};
#endif

/*
 * 85xx and 86xx share naming conventions, but different layout.
 * Correlate names to CPU-specific values to share common
 * PCI code.
 */
#if defined(CONFIG_MPC85xx)
#define MPC8xxx_DEVDISR_PCIE1		MPC85xx_DEVDISR_PCIE
#define MPC8xxx_DEVDISR_PCIE2		MPC85xx_DEVDISR_PCIE2
#define MPC8xxx_DEVDISR_PCIE3		MPC85xx_DEVDISR_PCIE3
#define MPC8xxx_PORDEVSR_IO_SEL		MPC85xx_PORDEVSR_IO_SEL
#define MPC8xxx_PORDEVSR_IO_SEL_SHIFT	MPC85xx_PORDEVSR_IO_SEL_SHIFT
#define MPC8xxx_PORBMSR_HA		MPC85xx_PORBMSR_HA
#define MPC8xxx_PORBMSR_HA_SHIFT	MPC85xx_PORBMSR_HA_SHIFT
#elif defined(CONFIG_MPC86xx)
#define MPC8xxx_DEVDISR_PCIE1		MPC86xx_DEVDISR_PCIEX1
#define MPC8xxx_DEVDISR_PCIE2		MPC86xx_DEVDISR_PCIEX2
#define MPC8xxx_DEVDISR_PCIE3	 	0	/* 8641 doesn't have PCIe3 */
#define MPC8xxx_PORDEVSR_IO_SEL		MPC8641_PORDEVSR_IO_SEL
#define MPC8xxx_PORDEVSR_IO_SEL_SHIFT	MPC8641_PORDEVSR_IO_SEL_SHIFT
#define MPC8xxx_PORBMSR_HA		MPC8641_PORBMSR_HA
#define MPC8xxx_PORBMSR_HA_SHIFT	MPC8641_PORBMSR_HA_SHIFT
#endif

void pci_init_board(void)
{
	struct pci_controller *hose;
	volatile ccsr_fsl_pci_t *pci;
	int width;
	int host;
#if defined(CONFIG_MPC85xx)
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#elif defined(CONFIG_MPC86xx)
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
#endif
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC8xxx_PORDEVSR_IO_SEL) >>
			MPC8xxx_PORDEVSR_IO_SEL_SHIFT;
	uint host_agent = (gur->porbmsr & MPC8xxx_PORBMSR_HA) >>
			MPC8xxx_PORBMSR_HA_SHIFT;
	struct pci_region *r;

#ifdef CONFIG_PCI1
	uint pci_spd_norm = (gur->pordevsr & MPC85xx_PORDEVSR_PCI1_SPD);
	uint pci_32 = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	uint pcix = gur->pordevsr & MPC85xx_PORDEVSR_PCI1;
	uint freq = CONFIG_SYS_CLK_FREQ / 1000 / 1000;

	width = 0; /* Silence compiler warning... */
	io_sel &= 0xf; /* Silence compiler warning... */
	pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCI1_ADDR;
	hose = &pci1_hose;
	host = host_agent_cfg[host_agent].pci_host[0];
	r = hose->regions;


	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf("\n    PCI1: %d bit %s, %s %d MHz, %s, %s\n",
			pci_32 ? 32 : 64,
			pcix ? "PCIX" : "PCI",
			pci_spd_norm ?  ">=" : "<=",
			pcix ? freq * 2 : freq,
			host ? "host" : "agent",
			pci_arb ? "arbiter" : "external-arbiter");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
				CONFIG_SYS_PCI1_MEM_BASE,
				CONFIG_SYS_PCI1_MEM_PHYS,
				CONFIG_SYS_PCI1_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
				CONFIG_SYS_PCI1_IO_BASE,
				CONFIG_SYS_PCI1_IO_PHYS,
				CONFIG_SYS_PCI1_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
				   (int)&pci->cfg_data);

		fsl_pci_init(hose);

		/* Unlock inbound PCI configuration cycles */
		if (!host)
			fsl_pci_config_unlock(hose);

		first_free_busno = hose->last_busno + 1;
		printf("    PCI1 on bus %02x - %02x\n",
			hose->first_busno, hose->last_busno);
	} else {
		printf("    PCI1: disabled\n");
	}
#elif defined CONFIG_MPC8548
	/* PCI1 not present on MPC8572 */
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif
#ifdef CONFIG_PCIE1
	pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE1_ADDR;
	hose = &pcie1_hose;
	host = host_agent_cfg[host_agent].pcie_root[0];
	width = io_port_cfg[io_sel].pcie_width[0];
	r = hose->regions;

	if (width && !(devdisr & MPC8xxx_DEVDISR_PCIE1)) {
		printf("\n    PCIE1 connected as %s (x%d)",
			host ? "Root Complex" : "End Point", width);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug(" with errors.  Clearing.  Now 0x%08x",
				pci->pme_msg_det);
		}
		printf("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
				CONFIG_SYS_PCIE1_MEM_BASE,
				CONFIG_SYS_PCIE1_MEM_PHYS,
				CONFIG_SYS_PCIE1_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
				CONFIG_SYS_PCIE1_IO_BASE,
				CONFIG_SYS_PCIE1_IO_PHYS,
				CONFIG_SYS_PCIE1_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
					(int) &pci->cfg_data);

		fsl_pci_init(hose);

		/* Unlock inbound PCI configuration cycles */
		if (!host)
			fsl_pci_config_unlock(hose);

		first_free_busno = hose->last_busno + 1;
		printf("    PCIE1 on bus %02x - %02x\n",
				hose->first_busno, hose->last_busno);
	}
#else
	gur->devdisr |= MPC8xxx_DEVDISR_PCIE1; /* disable */
#endif /* CONFIG_PCIE1 */

#ifdef CONFIG_PCIE2
	pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE2_ADDR;
	hose = &pcie2_hose;
	host = host_agent_cfg[host_agent].pcie_root[1];
	width = io_port_cfg[io_sel].pcie_width[1];
	r = hose->regions;

	if (width && !(devdisr & MPC8xxx_DEVDISR_PCIE2)) {
		printf("\n    PCIE2 connected as %s (x%d)",
			host ? "Root Complex" : "End Point", width);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug(" with errors.  Clearing.  Now 0x%08x",
				pci->pme_msg_det);
		}
		printf("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
				CONFIG_SYS_PCIE2_MEM_BASE,
				CONFIG_SYS_PCIE2_MEM_PHYS,
				CONFIG_SYS_PCIE2_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
				CONFIG_SYS_PCIE2_IO_BASE,
				CONFIG_SYS_PCIE2_IO_PHYS,
				CONFIG_SYS_PCIE2_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
					(int)&pci->cfg_data);

		fsl_pci_init(hose);

		/* Unlock inbound PCI configuration cycles */
		if (!host)
			fsl_pci_config_unlock(hose);

		first_free_busno = hose->last_busno + 1;
		printf("    PCIE2 on bus %02x - %02x\n",
				hose->first_busno, hose->last_busno);
	}
#else
	gur->devdisr |= MPC8xxx_DEVDISR_PCIE2; /* disable */
#endif /* CONFIG_PCIE2 */

#ifdef CONFIG_PCIE3
	pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE3_ADDR;
	hose = &pcie3_hose;
	host = host_agent_cfg[host_agent].pcie_root[2];
	width = io_port_cfg[io_sel].pcie_width[2];
	r = hose->regions;

	if (width && !(devdisr & MPC8xxx_DEVDISR_PCIE3)) {
		printf("\n    PCIE3 connected as %s (x%d)",
			host ? "Root Complex" : "End Point", width);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug(" with errors.  Clearing.  Now 0x%08x",
				pci->pme_msg_det);
		}
		printf("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
				CONFIG_SYS_PCIE3_MEM_BASE,
				CONFIG_SYS_PCIE3_MEM_PHYS,
				CONFIG_SYS_PCIE3_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
				CONFIG_SYS_PCIE3_IO_BASE,
				CONFIG_SYS_PCIE3_IO_PHYS,
				CONFIG_SYS_PCIE3_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
					(int)&pci->cfg_data);

		fsl_pci_init(hose);

		/* Unlock inbound PCI configuration cycles */
		if (!host)
			fsl_pci_config_unlock(hose);

		first_free_busno = hose->last_busno + 1;
		printf("    PCIE3 on bus %02x - %02x\n",
				hose->first_busno, hose->last_busno);
	}
#else
	gur->devdisr |= MPC8xxx_DEVDISR_PCIE3; /* disable */
#endif /* CONFIG_PCIE3 */
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_pci_setup(void *blob, bd_t *bd)
{
	/* TODO - make node name (eg pci0) dynamic */
#ifdef CONFIG_PCI1
	ft_fsl_pci_setup(blob, "pci0", &pci1_hose);
#endif
#ifdef CONFIG_PCIE1
	ft_fsl_pci_setup(blob, "pci2", &pcie1_hose);
#endif
#ifdef CONFIG_PCIE2
	ft_fsl_pci_setup(blob, "pci1", &pcie2_hose);
#endif
#ifdef CONFIG_PCIE3
	ft_fsl_pci_setup(blob, "pci0", &pcie3_hose);
#endif
}
#endif /* CONFIG_OF_BOARD_SETUP */
