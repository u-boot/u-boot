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

static struct pci_controller pcie1_hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_mpc86xxcts_config_table
#endif
};
#endif /* CONFIG_PCI */

#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif	/* CONFIG_PCIE2 */

int first_free_busno = 0;

void pci_init_board(void)
{
	struct fsl_pci_info pci_info[2];
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint devdisr = in_be32(&gur->devdisr);
	uint io_sel = (gur->pordevsr & MPC8641_PORDEVSR_IO_SEL)
		>> MPC8641_PORDEVSR_IO_SEL_SHIFT;
	int pcie_ep;
	int num = 0;

#ifdef CONFIG_PCIE1
	int pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	if (pcie_configured && !(devdisr & MPC86xx_DEVDISR_PCIEX1)) {
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
		printf("PCIE1: connected as %s (base addr %lx)\n",
			pcie_ep ? "Endpoint" : "Root Complex",
			pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie1_hose, first_free_busno);
	} else {
		puts("PCIE1: disabled\n");
	}
#else
	puts("PCIE1: disabled\n");
#endif /* CONFIG_PCIE1 */

#ifdef CONFIG_PCIE2

	SET_STD_PCIE_INFO(pci_info[num], 2);
	pcie_ep = fsl_setup_hose(&pcie2_hose, pci_info[num].regs);
	printf("PCIE2: connected as %s (base addr %lx)\n",
		pcie_ep ? "Endpoint" : "Root Complex",
		pci_info[num].regs);
	first_free_busno = fsl_pci_init_port(&pci_info[num++],
				&pcie2_hose, first_free_busno);
#else
	puts("PCIE2: disabled\n");
#endif /* CONFIG_PCIE2 */
}


#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup (void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;
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
