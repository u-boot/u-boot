/*
 * Copyright 2007 Wind River Systemes, Inc. <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Joe Hamman joe.hamman@embeddedspecialties.com
 *
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
#include <asm/processor.h>
#include <asm/immap_86xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <libfdt.h>
#include <fdt_support.h>

long int fixed_sdram (void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	puts ("Board: Wind River SBC8641D\n");

	return 0;
}

phys_size_t initdram (int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = fsl_ddr_sdram();
#else
	dram_size = fixed_sdram ();
#endif

#if defined(CONFIG_SYS_RAMBOOT)
	puts ("    DDR: ");
	return dram_size;
#endif

	puts ("    DDR: ");
	return dram_size;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	puts ("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts ("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts ("SDRAM test passed.\n");
	return 0;
}
#endif

#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
long int fixed_sdram (void)
{
#if !defined(CONFIG_SYS_RAMBOOT)
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr1;

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs1_bnds = CONFIG_SYS_DDR_CS1_BNDS;
	ddr->cs2_bnds = CONFIG_SYS_DDR_CS2_BNDS;
	ddr->cs3_bnds = CONFIG_SYS_DDR_CS3_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;
	ddr->cs1_config = CONFIG_SYS_DDR_CS1_CONFIG;
	ddr->cs2_config = CONFIG_SYS_DDR_CS2_CONFIG;
	ddr->cs3_config = CONFIG_SYS_DDR_CS3_CONFIG;
	ddr->timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_cfg = CONFIG_SYS_DDR_CFG_1A;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR_CFG_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR_MODE_2;
	ddr->sdram_mode_cntl = CONFIG_SYS_DDR_MODE_CTL;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg = CONFIG_SYS_DDR_CFG_1B;
	asm ("sync; isync");

	udelay (500);
	ddr = &immap->im_ddr2;

	ddr->cs0_bnds = CONFIG_SYS_DDR2_CS0_BNDS;
	ddr->cs1_bnds = CONFIG_SYS_DDR2_CS1_BNDS;
	ddr->cs2_bnds = CONFIG_SYS_DDR2_CS2_BNDS;
	ddr->cs3_bnds = CONFIG_SYS_DDR2_CS3_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR2_CS0_CONFIG;
	ddr->cs1_config = CONFIG_SYS_DDR2_CS1_CONFIG;
	ddr->cs2_config = CONFIG_SYS_DDR2_CS2_CONFIG;
	ddr->cs3_config = CONFIG_SYS_DDR2_CS3_CONFIG;
	ddr->timing_cfg_3 = CONFIG_SYS_DDR2_EXT_REFRESH;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR2_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR2_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR2_TIMING_2;
	ddr->sdram_cfg = CONFIG_SYS_DDR2_CFG_1A;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR2_CFG_2;
	ddr->sdram_mode = CONFIG_SYS_DDR2_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR2_MODE_2;
	ddr->sdram_mode_cntl = CONFIG_SYS_DDR2_MODE_CTL;
	ddr->sdram_interval = CONFIG_SYS_DDR2_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR2_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR2_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg = CONFIG_SYS_DDR2_CFG_1B;
	asm ("sync; isync");

	udelay (500);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif				/* !defined(CONFIG_SPD_EEPROM) */

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_fsl86xxads_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER}},
	{}
};
#endif

static struct pci_controller pci1_hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_mpc86xxcts_config_table
#endif
};
#endif /* CONFIG_PCI */

#ifdef CONFIG_PCI2
static struct pci_controller pci2_hose;
#endif	/* CONFIG_PCI2 */

int first_free_busno = 0;

void pci_init_board(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC8641_PORDEVSR_IO_SEL)
		>> MPC8641_PORDEVSR_IO_SEL_SHIFT;

#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCI1_ADDR;
	struct pci_controller *hose = &pci1_hose;
	struct pci_region *r = hose->regions;
#ifdef DEBUG
	uint host1_agent = (gur->porbmsr & MPC8641_PORBMSR_HA)
		>> MPC8641_PORBMSR_HA_SHIFT;
	uint pex1_agent = (host1_agent == 0) || (host1_agent == 1);
#endif
	if ((io_sel == 2 || io_sel == 3 || io_sel == 5
	     || io_sel == 6 || io_sel == 7 || io_sel == 0xF)
	    && !(devdisr & MPC86xx_DEVDISR_PCIEX1)) {
		debug("PCI-EXPRESS 1: %s \n", pex1_agent ? "Agent" : "Host");
		debug("0x%08x=0x%08x ", &pci->pme_msg_det, pci->pme_msg_det);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug(" with errors.  Clearing.  Now 0x%08x",
			      pci->pme_msg_det);
		}
		debug("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCI1_MEM_BUS,
			       CONFIG_SYS_PCI1_MEM_PHYS,
			       CONFIG_SYS_PCI1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
			       CONFIG_SYS_PCI1_IO_BUS,
			       CONFIG_SYS_PCI1_IO_PHYS,
			       CONFIG_SYS_PCI1_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno=hose->last_busno+1;
		printf ("    PCI-EXPRESS 1 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

	} else {
		puts("PCI-EXPRESS 1: Disabled\n");
	}
}
#else
	puts("PCI-EXPRESS1: Disabled\n");
#endif /* CONFIG_PCI1 */

#ifdef CONFIG_PCI2
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCI2_ADDR;
	struct pci_controller *hose = &pci2_hose;
	struct pci_region *r = hose->regions;


	/* inbound */
	r += fsl_pci_setup_inbound_windows(r);

	/* outbound memory */
	pci_set_region(r++,
		       CONFIG_SYS_PCI2_MEM_BUS,
		       CONFIG_SYS_PCI2_MEM_PHYS,
		       CONFIG_SYS_PCI2_MEM_SIZE,
		       PCI_REGION_MEM);

	/* outbound io */
	pci_set_region(r++,
		       CONFIG_SYS_PCI2_IO_BUS,
		       CONFIG_SYS_PCI2_IO_PHYS,
		       CONFIG_SYS_PCI2_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = r - hose->regions;

	hose->first_busno=first_free_busno;
	pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

	fsl_pci_init(hose);

	first_free_busno=hose->last_busno+1;
	printf ("    PCI-EXPRESS 2 on bus %02x - %02x\n",
		hose->first_busno,hose->last_busno);
}
#else
	puts("PCI-EXPRESS 2: Disabled\n");
#endif /* CONFIG_PCI2 */

}


#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup (void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI1
	ft_fsl_pci_setup(blob, "pci0", &pci1_hose);
#endif
#ifdef CONFIG_PCI2
	ft_fsl_pci_setup(blob, "pci1", &pci2_hose);
#endif
}
#endif

void sbc8641d_reset_board (void)
{
	puts ("Resetting board....\n");
}

/*
 * get_board_sys_clk
 *      Clock is fixed at 1GHz on this board. Used for CONFIG_SYS_CLK_FREQ
 */

unsigned long get_board_sys_clk (ulong dummy)
{
	int i;
	ulong val = 0;

	i = 5;
	i &= 0x07;

	switch (i) {
	case 0:
		val = 33000000;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66000000;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 134000000;
		break;
	case 7:
		val = 166000000;
		break;
	}

	return val;
}

void board_reset(void)
{
#ifdef CONFIG_SYS_RESET_ADDRESS
	ulong addr = CONFIG_SYS_RESET_ADDRESS;

	/* flush and disable I/D cache */
	__asm__ __volatile__ ("mfspr	3, 1008"	::: "r3");
	__asm__ __volatile__ ("ori	5, 5, 0xcc00"	::: "r5");
	__asm__ __volatile__ ("ori	4, 3, 0xc00"	::: "r4");
	__asm__ __volatile__ ("andc	5, 3, 5"	::: "r5");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 4");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 5");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");

	/*
	 * SRR0 has system reset vector, SRR1 has default MSR value
	 * rfi restores MSR from SRR1 and sets the PC to the SRR0 value
	 */
	__asm__ __volatile__ ("mtspr	26, %0"		:: "r" (addr));
	__asm__ __volatile__ ("li	4, (1 << 6)"	::: "r4");
	__asm__ __volatile__ ("mtspr	27, 4");
	__asm__ __volatile__ ("rfi");
#endif
}

#ifdef CONFIG_MP
extern void cpu_mp_lmb_reserve(struct lmb *lmb);

void board_lmb_reserve(struct lmb *lmb)
{
	cpu_mp_lmb_reserve(lmb);
}
#endif
