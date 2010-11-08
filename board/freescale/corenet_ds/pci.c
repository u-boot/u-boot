/*
 * Copyright 2007-2010 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/fsl_serdes.h>

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif

#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif

#ifdef CONFIG_PCIE3
static struct pci_controller pcie3_hose;
#endif

void pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info[3];
	u32 devdisr;
	int first_free_busno = 0;
	int num = 0;

	int pcie_ep, pcie_configured;

	devdisr = in_be32(&gur->devdisr);

	debug ("   pci_init_board: devdisr=%x\n", devdisr);

#ifdef CONFIG_PCIE1
	pcie_configured = is_serdes_configured(PCIE1);

	if (pcie_configured && !(devdisr & FSL_CORENET_DEVDISR_PCIE1)) {
		set_next_law(CONFIG_SYS_PCIE1_MEM_PHYS, LAW_SIZE_512M,
				LAW_TRGT_IF_PCIE_1);
		set_next_law(CONFIG_SYS_PCIE1_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_1);
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
		printf("    PCIE1 connected to Slot 1 as %s (base addr %lx)\n",
				pcie_ep ? "End Point" : "Root Complex",
				pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
				&pcie1_hose, first_free_busno);
	} else {
		printf ("    PCIE1: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, FSL_CORENET_DEVDISR_PCIE1); /* disable */
#endif

#ifdef CONFIG_PCIE2
	pcie_configured = is_serdes_configured(PCIE2);

	if (pcie_configured && !(devdisr & FSL_CORENET_DEVDISR_PCIE2)) {
		set_next_law(CONFIG_SYS_PCIE2_MEM_PHYS, LAW_SIZE_512M,
				LAW_TRGT_IF_PCIE_2);
		set_next_law(CONFIG_SYS_PCIE2_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_2);
		SET_STD_PCIE_INFO(pci_info[num], 2);
		pcie_ep = fsl_setup_hose(&pcie2_hose, pci_info[num].regs);
		printf("    PCIE2 connected to Slot 3 as %s (base addr %lx)\n",
				pcie_ep ? "End Point" : "Root Complex",
				pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
				&pcie2_hose, first_free_busno);
	} else {
		printf ("    PCIE2: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, FSL_CORENET_DEVDISR_PCIE2); /* disable */
#endif

#ifdef CONFIG_PCIE3
	pcie_configured = is_serdes_configured(PCIE3);

	if (pcie_configured && !(devdisr & FSL_CORENET_DEVDISR_PCIE3)) {
		set_next_law(CONFIG_SYS_PCIE3_MEM_PHYS, LAW_SIZE_512M,
				LAW_TRGT_IF_PCIE_3);
		set_next_law(CONFIG_SYS_PCIE3_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_3);
		SET_STD_PCIE_INFO(pci_info[num], 3);
		pcie_ep = fsl_setup_hose(&pcie3_hose, pci_info[num].regs);
		printf("    PCIE3 connected to Slot 2 as %s (base addr %lx)\n",
				pcie_ep ? "End Point" : "Root Complex",
				pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
				&pcie3_hose, first_free_busno);
	} else {
		printf ("    PCIE3: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, FSL_CORENET_DEVDISR_PCIE3); /* disable */
#endif
}

void pci_of_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}
