/*
 * Copyright 2007
 * Robert Lazarski, Instituto Atlantico, robertlazarski@gmail.com
 *
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
#include <asm/io.h>
#include <spd_sdram.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

long int fixed_sdram(void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);

	if ((uint)&gur->porpllsr != 0xe00e0000) {
		printf("immap size error %x\n",&gur->porpllsr);
	}
	printf ("Board: ATUM8548\n");

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
	ecm->eedr = 0xffffffff;		/* Clear ecm errors */
	ecm->eeer = 0xffffffff;		/* Enable ecm errors */

	return 0;
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
long int fixed_sdram (void)
{
	volatile ccsr_ddr_t *ddr= (void *)(CFG_MPC85xx_DDR_ADDR);

	ddr->cs0_bnds = CFG_DDR_CS0_BNDS;
	ddr->cs0_config = CFG_DDR_CS0_CONFIG;
	ddr->timing_cfg_0 = CFG_DDR_TIMING_0;
	ddr->timing_cfg_1 = CFG_DDR_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR_TIMING_2;
	ddr->sdram_mode = CFG_DDR_MODE;
	ddr->sdram_interval = CFG_DDR_INTERVAL;
    #if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000000D;
	ddr->err_sbe = 0x00ff0000;
    #endif
	asm("sync;isync;msync");
	udelay(500);
    #if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg = (CFG_DDR_CONTROL | 0x20000000);
    #else
	ddr->sdram_cfg = CFG_DDR_CONTROL;
    #endif
	asm("sync; isync; msync");
	udelay(500);
	return CFG_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

long int
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

#if defined(CONFIG_SPD_EEPROM)
	puts("spd_sdram\n");
	dram_size = spd_sdram ();
#else
	puts("fixed_sdram\n");
	dram_size = fixed_sdram ();
#endif

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
	for (p = pstart; p < pend; p++) {
		printf ("DRAM test attempting to write 0xaaaaaaaa at: %08x\n", (uint) p);
		*p = 0xaaaaaaaa;
	}

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

#ifdef CONFIG_PCI2
static struct pci_controller pci2_hose;
#endif

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif

int first_free_busno=0;

void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);

	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	uint host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x, host_agent=%x\n",
		devdisr, io_sel, host_agent);

	/* explicitly set 'Clock out select register' to echo SYSCLK input to our CPLD */
	gur->clkocr  |= MPC85xx_ATUM_CLKOCR;

	if (io_sel & 1) {
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII1_DIS))
			printf ("    eTSEC1 is in sgmii mode.\n");
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII2_DIS))
			printf ("    eTSEC2 is in sgmii mode.\n");
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII3_DIS))
			printf ("    eTSEC3 is in sgmii mode.\n");
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII4_DIS))
			printf ("    eTSEC4 is in sgmii mode.\n");
	}

#ifdef CONFIG_PCIE1
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep = (host_agent == 5);
	int pcie_configured  = io_sel & 6;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE1 connected to slot as %s (base address %x)",
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

#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;

	uint pci_agent = (host_agent == 6);
	uint pci_speed = 33333000; /*get_clock_freq (); PCI PSPEED in [4:5] */
	uint pci_32 = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;	/* PORDEVSR[15] */
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;	/* PORDEVSR[14] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;	/* PORPLLSR[16] */

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf ("\n    PCI1: %d bit, %s MHz, %s, %s, %s (base address %x)\n",
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
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("PCI1 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);
	} else {
		printf ("    PCI1: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif

#ifdef CONFIG_PCI2
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI2_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci2_hose;

	if (!(devdisr & MPC85xx_DEVDISR_PCI2)) {
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		pci_set_region(hose->regions + 1,
			       CFG_PCI2_MEM_BASE,
			       CFG_PCI2_MEM_PHYS,
			       CFG_PCI2_MEM_SIZE,
			       PCI_REGION_MEM);

		pci_set_region(hose->regions + 2,
			       CFG_PCI2_IO_BASE,
			       CFG_PCI2_IO_PHYS,
			       CFG_PCI2_IO_SIZE,
			       PCI_REGION_IO);
		hose->region_count = 3;
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("PCI2 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);
	} else {
		printf ("    PCI2: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI2;
#endif
}


int last_stage_init(void)
{
	int ic = icache_status ();
	printf ("icache_status: %d\n", ic);
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)

void
ft_board_setup(void *blob, bd_t *bd)
{
	int node, tmp[2];
	const char *path;

	ft_cpu_setup(blob, bd);

	node = fdt_path_offset(blob, "/aliases");
	tmp[0] = 0;
	if (node >= 0) {
#ifdef CONFIG_PCI1
		path = fdt_getprop(blob, node, "pci0", NULL);
		if (path) {
			tmp[1] = pci1_hose.last_busno - pci1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
#ifdef CONFIG_PCI2
		path = fdt_getprop(blob, node, "pci1", NULL);
		if (path) {
			tmp[1] = pci2_hose.last_busno - pci2_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
#ifdef CONFIG_PCIE1
		path = fdt_getprop(blob, node, "pci2", NULL);
		if (path) {
			tmp[1] = pcie1_hose.last_busno - pcie1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
	}
}
#endif
