/*
 * Copyright 2007 Freescale Semiconductor, Inc.
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
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/immap_fsl_pci.h>
#include <spd.h>
#include <miiphy.h>

#include "../common/pixis.h"

#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
extern void ft_cpu_setup(void *blob, bd_t *bd);
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

extern long int spd_sdram(void);

void sdram_init(void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile ccsr_lbc_t *lbc = &immap->im_lbc;
	volatile ccsr_local_ecm_t *ecm = &immap->im_local_ecm;

	if ((uint)&gur->porpllsr != 0xe00e0000) {
		printf("immap size error %x\n",&gur->porpllsr);
	}
	printf ("Board: MPC8544DS\n");

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
	ecm->eedr = 0xffffffff;		/* Clear ecm errors */
	ecm->eeer = 0xffffffff;		/* Enable ecm errors */

	return 0;
}

long int
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

	dram_size = spd_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif
	puts("    DDR: ");
	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int
testdram(void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
	       CFG_MEMTEST_START,
	       CFG_MEMTEST_END);

	printf("DRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test passed.\n");
	return 0;
}
#endif

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

int first_free_busno=0;

void
pci_init_board(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	uint host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x, host_agent=%x\n",
		devdisr, io_sel, host_agent);

	if (io_sel & 1) {
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII1_DIS))
			printf ("    eTSEC1 is in sgmii mode.\n");
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII3_DIS))
			printf ("    eTSEC3 is in sgmii mode.\n");
	}

#ifdef CONFIG_PCIE3
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE3_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie3_hose;
	int pcie_ep = (host_agent == 3);
	int pcie_configured  = io_sel >= 1;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE3 connected to ULI as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE3_MEM_BASE,
			       CFG_PCIE3_MEM_PHYS,
			       CFG_PCIE3_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE3_IO_BASE,
			       CFG_PCIE3_IO_PHYS,
			       CFG_PCIE3_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE3_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE3_MEM_BASE2,
			       CFG_PCIE3_MEM_PHYS2,
			       CFG_PCIE3_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno=hose->last_busno+1;
		printf ("    PCIE3 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

	} else {
		printf ("    PCIE3: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE3; /* disable */
#endif

#ifdef CONFIG_PCIE1
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep = (host_agent == 5);
	int pcie_configured  = io_sel & 6;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE1 connected to Slot2 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE1_MEM_BASE,
			       CFG_PCIE1_MEM_PHYS,
			       CFG_PCIE1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE1_IO_BASE,
			       CFG_PCIE1_IO_PHYS,
			       CFG_PCIE1_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE1_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE1_MEM_BASE2,
			       CFG_PCIE1_MEM_PHYS2,
			       CFG_PCIE1_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;

		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno=hose->last_busno+1;
		printf("    PCIE1 on bus %02x - %02x\n",
		       hose->first_busno,hose->last_busno);

	} else {
		printf ("    PCIE1: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE; /* disable */
#endif

#ifdef CONFIG_PCIE2
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE2_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie2_hose;
	int pcie_ep = (host_agent == 3);
	int pcie_configured  = io_sel & 4;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE2 connected to Slot 1 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE2_MEM_BASE,
			       CFG_PCIE2_MEM_PHYS,
			       CFG_PCIE2_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE2_IO_BASE,
			       CFG_PCIE2_IO_PHYS,
			       CFG_PCIE2_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE2_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE2_MEM_BASE2,
			       CFG_PCIE2_MEM_PHYS2,
			       CFG_PCIE2_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("    PCIE2 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

	} else {
		printf ("    PCIE2: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE2; /* disable */
#endif


#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;

	uint pci_agent = (host_agent == 6);
	uint pci_speed = 66666000; /*get_clock_freq (); PCI PSPEED in [4:5] */
	uint pci_32 = 1;
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;	/* PORDEVSR[14] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;	/* PORPLLSR[16] */


	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf ("\n    PCI: %d bit, %s MHz, %s, %s, %s (base address %x)\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter",
			(uint)pci
			);

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCI1_MEM_BASE,
			       CFG_PCI1_MEM_PHYS,
			       CFG_PCI1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCI1_IO_BASE,
			       CFG_PCI1_IO_PHYS,
			       CFG_PCI1_IO_SIZE,
			       PCI_REGION_IO);
		hose->region_count = 3;
#ifdef CFG_PCIE3_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE3_MEM_BASE2,
			       CFG_PCIE3_MEM_PHYS2,
			       CFG_PCIE3_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("PCI on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);
	} else {
		printf ("    PCI: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif
}


int last_stage_init(void)
{
	return 0;
}


unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 i, go_bit, rd_clks;
	ulong val = 0;

	go_bit = in8(PIXIS_BASE + PIXIS_VCTL);
	go_bit &= 0x01;

	rd_clks = in8(PIXIS_BASE + PIXIS_VCFGEN0);
	rd_clks &= 0x1C;

	/*
	 * Only if both go bit and the SCLK bit in VCFGEN0 are set
	 * should we be using the AUX register. Remember, we also set the
	 * GO bit to boot from the alternate bank on the on-board flash
	 */

	if (go_bit) {
		if (rd_clks == 0x1c)
			i = in8(PIXIS_BASE + PIXIS_AUX);
		else
			i = in8(PIXIS_BASE + PIXIS_SPD);
	} else {
		i = in8(PIXIS_BASE + PIXIS_SPD);
	}

	i &= 0x07;

	switch (i) {
	case 0:
		val = 33333333;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66666666;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 133333333;
		break;
	case 7:
		val = 166666666;
		break;
	}

	return val;
}

#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;

	ft_cpu_setup(blob, bd);

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
#ifdef CONFIG_PCIE1
	p = (u32 *)ft_get_prop(blob, "/" OF_SOC "/pci@a000/bus-range", &len);
	if (p != NULL) {
		p[0] = 0;
		p[1] = pcie1_hose.last_busno - pcie1_hose.first_busno;
		debug("PCI@a000 first_busno=%d last_busno=%d\n",p[0],p[1]);
	}
#endif
#ifdef CONFIG_PCIE2
	p = (u32 *)ft_get_prop(blob, "/" OF_SOC "/pci@9000/bus-range", &len);
	if (p != NULL) {
		p[0] = 0;
		p[1] = pcie2_hose.last_busno - pcie2_hose.first_busno;
		debug("PCI@9000 first_busno=%d last_busno=%d\n",p[0],p[1]);
	}
#endif
#ifdef CONFIG_PCIE3
	p = (u32 *)ft_get_prop(blob, "/" OF_SOC "/pci@b000/bus-range", &len);
	if (p != NULL) {
		p[0] = 0;
		p[1] = pcie3_hose.last_busno - pcie3_hose.first_busno;;
		debug("PCI@b000 first_busno=%d last_busno=%d\n",p[0],p[1]);
	}
#endif
	ft_cpu_setup(blob, bd);

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
}
#endif
