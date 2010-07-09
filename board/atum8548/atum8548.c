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
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <spd_sdram.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

long int fixed_sdram(void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);

	if ((uint)&gur->porpllsr != 0xe00e0000) {
		printf("immap size error %lx\n",(ulong)&gur->porpllsr);
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
	volatile ccsr_ddr_t *ddr= (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
    #if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000000D;
	ddr->err_sbe = 0x00ff0000;
    #endif
	asm("sync;isync;msync");
	udelay(500);
    #if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg = (CONFIG_SYS_DDR_CONTROL | 0x20000000);
    #else
	ddr->sdram_cfg = CONFIG_SYS_DDR_CONTROL;
    #endif
	asm("sync; isync; msync");
	udelay(500);
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

phys_size_t
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

#if defined(CONFIG_SPD_EEPROM)
	puts("fsl_ddr_sdram\n");
	dram_size = fsl_ddr_sdram();
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;
#else
	puts("fixed_sdram\n");
	dram_size = fixed_sdram ();
#endif

	puts("    DDR: ");
	return dram_size;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int
testdram(void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
	       CONFIG_SYS_MEMTEST_START,
	       CONFIG_SYS_MEMTEST_END);

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

void pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info[3];
	u32 devdisr, pordevsr, io_sel;
	u32 porpllsr, pci_agent, pci_speed, pci_32, pci_arb, pci_clk_sel;
	int first_free_busno = 0;
	int num = 0;

	int pcie_ep, pcie_configured;

	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	porpllsr = in_be32(&gur->porpllsr);
	io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x\n", devdisr, io_sel);

	/* explicitly set 'Clock out select register' to echo SYSCLK input to our CPLD */
	setbits_be32(&gur->clkocr, MPC85xx_ATUM_CLKOCR);

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
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
#ifdef CONFIG_SYS_PCIE1_MEM_BUS2
		/* outbound memory */
		pci_set_region(&pcie1_hose.regions[0],
			       CONFIG_SYS_PCIE1_MEM_BUS2,
			       CONFIG_SYS_PCIE1_MEM_PHYS2,
			       CONFIG_SYS_PCIE1_MEM_SIZE2,
			       PCI_REGION_MEM);

		pcie1_hose.region_count = 1;
#endif
		printf ("    PCIE1 connected to Slot as %s (base addr %lx)\n",
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

#ifdef CONFIG_PCI1
	pci_speed = 33333000; /*get_clock_freq (); PCI PSPEED in [4:5] */
	pci_32 = pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;	/* PORDEVSR[15] */
	pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info[num], 1);
		pci_agent = fsl_setup_hose(&pci1_hose, pci_info[num].regs);
		printf ("\n    PCI1: %d bit, %s MHz, %s, %s, %s (base address %lx)\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter",
			pci_info[num].regs);

		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pci1_hose, first_free_busno);
	} else {
		printf ("    PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1); /* disable */
#endif

#ifdef CONFIG_PCI2
	if (!(devdisr & MPC85xx_DEVDISR_PCI2)) {
		SET_STD_PCI_INFO(pci_info[num], 2);
		pci_agent = fsl_setup_hose(&pci2_hose, pci_info[num].regs);

		puts ("    PCI2\n");
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pci1_hose, first_free_busno);
	} else {
		printf ("    PCI2: disabled\n");
	}
	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI2); /* disable */
#endif
}


int last_stage_init(void)
{
	int ic = icache_status ();
	printf ("icache_status: %d\n", ic);
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;
}
#endif
