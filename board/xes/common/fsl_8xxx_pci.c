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
#include <asm/io.h>
#include <linux/compiler.h>
#include <libfdt.h>
#include <fdt_support.h>


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
	struct fsl_pci_info pci_info[3];
	int first_free_busno = 0;
	int num = 0;
	int pcie_ep;
	__maybe_unused int pcie_configured;

#if defined(CONFIG_MPC85xx)
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#elif defined(CONFIG_MPC86xx)
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
#endif
	u32 devdisr = in_be32(&gur->devdisr);
	u32 pordevsr = in_be32(&gur->pordevsr);
	__maybe_unused uint io_sel = (pordevsr & MPC8xxx_PORDEVSR_IO_SEL) >>
			MPC8xxx_PORDEVSR_IO_SEL_SHIFT;

#ifdef CONFIG_PCI1
	uint pci_spd_norm = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_SPD;
	uint pci_32 = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_PCI32;
	uint pci_arb = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_ARB;
	uint pcix = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1;
	uint freq = CONFIG_SYS_CLK_FREQ / 1000 / 1000;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pci1_hose, pci_info[num].regs);
		printf("PCI1: %d bit %s, %s %d MHz, %s, %s\n",
			pci_32 ? 32 : 64,
			pcix ? "PCIX" : "PCI",
			pci_spd_norm ? ">=" : "<=",
			pcix ? freq * 2 : freq,
			pcie_ep ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter");

		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI1: disabled\n");
	}
#elif defined CONFIG_MPC8548
	/* PCI1 not present on MPC8572 */
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1);
#endif

#ifdef CONFIG_PCIE1
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	if (pcie_configured && !(devdisr & MPC8xxx_DEVDISR_PCIE1)) {
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
		printf("PCIE1: connected as %s\n",
			pcie_ep ? "Endpoint" : "Root Complex");
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie1_hose, first_free_busno);
	} else {
		printf("PCIE1: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC8xxx_DEVDISR_PCIE1);
#endif /* CONFIG_PCIE1 */

#ifdef CONFIG_PCIE2
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_2, io_sel);

	if (pcie_configured && !(devdisr & MPC8xxx_DEVDISR_PCIE2)) {
		SET_STD_PCIE_INFO(pci_info[num], 2);
		pcie_ep = fsl_setup_hose(&pcie2_hose, pci_info[num].regs);
		printf("PCIE2: connected as %s\n",
			pcie_ep ? "Endpoint" : "Root Complex");
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie2_hose, first_free_busno);
	} else {
		printf("PCIE2: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC8xxx_DEVDISR_PCIE2);
#endif /* CONFIG_PCIE2 */

#ifdef CONFIG_PCIE3
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_3, io_sel);

	if (pcie_configured && !(devdisr & MPC8xxx_DEVDISR_PCIE3)) {
		SET_STD_PCIE_INFO(pci_info[num], 3);
		pcie_ep = fsl_setup_hose(&pcie3_hose, pci_info[num].regs);
		printf("PCIE3: connected as %s\n",
			pcie_ep ? "Endpoint" : "Root Complex");
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie3_hose, first_free_busno);
	} else {
		printf("PCIE3: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC8xxx_DEVDISR_PCIE3);
#endif /* CONFIG_PCIE3 */
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_pci_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}
#endif /* CONFIG_OF_BOARD_SETUP */
