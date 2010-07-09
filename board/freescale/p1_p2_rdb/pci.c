/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
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
#include <command.h>
#include <pci.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/fsl_pci.h>
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif

#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif

void pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info[2];
	u32 devdisr, pordevsr, io_sel;
	int first_free_busno = 0;
	int num = 0;

	int pcie_ep, pcie_configured;

	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x\n", devdisr, io_sel);

	if (!(pordevsr & MPC85xx_PORDEVSR_SGMII2_DIS))
		printf ("    eTSEC2 is in sgmii mode.\n");

	puts("\n");
#ifdef CONFIG_PCIE2
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_2, io_sel);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		SET_STD_PCIE_INFO(pci_info[num], 2);
		pcie_ep = fsl_setup_hose(&pcie2_hose, pci_info[num].regs);
		printf("    PCIE2 connected to Slot 1 as %s (base addr %lx)\n",
				pcie_ep ? "Endpoint" : "Root Complex",
				pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie2_hose, first_free_busno);
	} else {
		printf ("    PCIE2: disabled\n");
	}
	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE2); /* disable */
#endif

#ifdef CONFIG_PCIE1
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
		printf("    PCIE1 connected to Slot 2 as %s (base addr %lx)\n",
				pcie_ep ? "Endpoint" : "Root Complex",
				pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie1_hose, first_free_busno);
	} else {
		printf ("    PCIE1: disabled\n");
	}
	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE); /* disable */
#endif
}

void ft_pci_board_setup(void *blob)
{
	FT_FSL_PCI_SETUP;
}
