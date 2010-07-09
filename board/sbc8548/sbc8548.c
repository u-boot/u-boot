/*
 * Copyright 2007,2009 Wind River Systems, Inc. <www.windriver.com>
 *
 * Copyright 2007 Embedded Specialties, Inc.
 *
 * Copyright 2004, 2007 Freescale Semiconductor.
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
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <spd_sdram.h>
#include <netdev.h>
#include <tsec.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

void local_bus_init(void);
void sdram_init(void);
long int fixed_sdram (void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	volatile u_char *rev= (void *)CONFIG_SYS_BD_REV;

	printf ("Board: Wind River SBC8548 Rev. 0x%01x\n",
			in_8(rev) >> 4);

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	out_be32(&ecm->eedr, 0xffffffff);	/* clear ecm errors */
	out_be32(&ecm->eeer, 0xffffffff);	/* enable ecm errors */
	return 0;
}

phys_size_t
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

#if defined(CONFIG_DDR_DLL)
	{
		/*
		 * Work around to stabilize DDR DLL MSYNC_IN.
		 * Errata DDR9 seems to have been fixed.
		 * This is now the workaround for Errata DDR11:
		 *    Override DLL = 1, Course Adj = 1, Tap Select = 0
		 */

		volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

		out_be32(&gur->ddrdllcr, 0x81000000);
		asm("sync;isync;msync");
		udelay(200);
	}
#endif

#if defined(CONFIG_SPD_EEPROM)
	dram_size = fsl_ddr_sdram();
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;
#else
	dram_size = fixed_sdram ();
#endif

	/*
	 * SDRAM Initialization
	 */
	sdram_init();

	puts("    DDR: ");
	return dram_size;
}

/*
 * Initialize Local Bus
 */
void
local_bus_init(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;

	uint clkdiv;
	uint lbc_hz;
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);
	clkdiv = (in_be32(&lbc->lcrr) & LCRR_CLKDIV) * 2;
	lbc_hz = sysinfo.freqSystemBus / 1000000 / clkdiv;

	out_be32(&gur->lbiuiplldcr1, 0x00078080);
	if (clkdiv == 16) {
		out_be32(&gur->lbiuiplldcr0, 0x7c0f1bf0);
	} else if (clkdiv == 8) {
		out_be32(&gur->lbiuiplldcr0, 0x6c0f1bf0);
	} else if (clkdiv == 4) {
		out_be32(&gur->lbiuiplldcr0, 0x5c0f1bf0);
	}

	setbits_be32(&lbc->lcrr, 0x00030000);

	asm("sync;isync;msync");

	out_be32(&lbc->ltesr, 0xffffffff);	/* Clear LBC error IRQs */
	out_be32(&lbc->lteir, 0xffffffff);	/* Enable LBC error IRQs */
}

/*
 * Initialize SDRAM memory on the Local Bus.
 */
void
sdram_init(void)
{
#if defined(CONFIG_SYS_LBC_SDRAM_SIZE)

	uint idx;
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	uint *sdram_addr = (uint *)CONFIG_SYS_LBC_SDRAM_BASE;
	uint lsdmr_common;

	puts("    SDRAM: ");

	print_size (CONFIG_SYS_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	set_lbc_or(3, CONFIG_SYS_OR3_PRELIM);
	set_lbc_br(3, CONFIG_SYS_BR3_PRELIM);
	set_lbc_or(4, CONFIG_SYS_OR4_PRELIM);
	set_lbc_br(4, CONFIG_SYS_BR4_PRELIM);

	out_be32(&lbc->lbcr, CONFIG_SYS_LBC_LBCR);
	asm("msync");

	out_be32(&lbc->lsrt,  CONFIG_SYS_LBC_LSRT);
	out_be32(&lbc->mrtpr, CONFIG_SYS_LBC_MRTPR);
	asm("msync");

	/*
	 * MPC8548 uses "new" 15-16 style addressing.
	 */
	lsdmr_common = CONFIG_SYS_LBC_LSDMR_COMMON;
	lsdmr_common |= LSDMR_BSMA1516;

	/*
	 * Issue PRECHARGE ALL command.
	 */
	out_be32(&lbc->lsdmr, lsdmr_common | LSDMR_OP_PCHALL);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue 8 AUTO REFRESH commands.
	 */
	for (idx = 0; idx < 8; idx++) {
		out_be32(&lbc->lsdmr, lsdmr_common | LSDMR_OP_ARFRSH);
		asm("sync;msync");
		*sdram_addr = 0xff;
		ppcDcbf((unsigned long) sdram_addr);
		udelay(100);
	}

	/*
	 * Issue 8 MODE-set command.
	 */
	out_be32(&lbc->lsdmr, lsdmr_common | LSDMR_OP_MRW);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue NORMAL OP command.
	 */
	out_be32(&lbc->lsdmr, lsdmr_common | LSDMR_OP_NORMAL);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(200);    /* Overkill. Must wait > 200 bus cycles */

#endif	/* enable SDRAM init */
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

#if !defined(CONFIG_SPD_EEPROM)
#define CONFIG_SYS_DDR_CONTROL 0xc300c000
/*************************************************************************
 *  fixed_sdram init -- doesn't use serial presence detect.
 *  assumes 256MB DDR2 SDRAM SODIMM, without ECC, running at DDR400 speed.
 ************************************************************************/
long int fixed_sdram (void)
{
	volatile ccsr_ddr_t *ddr = (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);

	out_be32(&ddr->cs0_bnds, 0x0000007f);
	out_be32(&ddr->cs1_bnds, 0x008000ff);
	out_be32(&ddr->cs2_bnds, 0x00000000);
	out_be32(&ddr->cs3_bnds, 0x00000000);
	out_be32(&ddr->cs0_config, 0x80010101);
	out_be32(&ddr->cs1_config, 0x80010101);
	out_be32(&ddr->cs2_config, 0x00000000);
	out_be32(&ddr->cs3_config, 0x00000000);
	out_be32(&ddr->timing_cfg_3, 0x00000000);
	out_be32(&ddr->timing_cfg_0, 0x00220802);
	out_be32(&ddr->timing_cfg_1, 0x38377322);
	out_be32(&ddr->timing_cfg_2, 0x0fa044C7);
	out_be32(&ddr->sdram_cfg, 0x4300C000);
	out_be32(&ddr->sdram_cfg_2, 0x24401000);
	out_be32(&ddr->sdram_mode, 0x23C00542);
	out_be32(&ddr->sdram_mode_2, 0x00000000);
	out_be32(&ddr->sdram_interval, 0x05080100);
	out_be32(&ddr->sdram_md_cntl, 0x00000000);
	out_be32(&ddr->sdram_data_init, 0x00000000);
	out_be32(&ddr->sdram_clk_cntl, 0x03800000);
	asm("sync;isync;msync");
	udelay(500);

	#if defined (CONFIG_DDR_ECC)
	  /* Enable ECC checking */
	  out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL | 0x20000000);
	#else
	  out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
	#endif

	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif	/* CONFIG_PCI1 */

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif	/* CONFIG_PCIE1 */


#ifdef CONFIG_PCI
void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info[2];
	u32 devdisr, pordevsr, porpllsr, io_sel;
	int first_free_busno = 0;
	int num = 0;

#ifdef CONFIG_PCIE1
	int pcie_configured;
#endif

	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	porpllsr = in_be32(&gur->porpllsr);
	io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;

	debug("   pci_init_board: devdisr=%x, io_sel=%x\n", devdisr, io_sel);

#ifdef CONFIG_PCI1
	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		uint pci_32 = pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;
		uint pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
		uint pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;
		uint pci_speed = CONFIG_SYS_CLK_FREQ;	/* get_clock_freq() */

		printf ("    PCI host: %d bit, %s MHz, %s, %s\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33000000) ? "33" :
			(pci_speed == 66000000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_arb ? "arbiter" : "external-arbiter");

		SET_STD_PCI_INFO(pci_info[num], 1);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pci1_hose, first_free_busno);
	} else {
		printf ("    PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1); /* disable */
#endif

	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI2); /* disable PCI2 */

#ifdef CONFIG_PCIE1
	pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		SET_STD_PCIE_INFO(pci_info[num], 1);
		printf ("    PCIE at base address %lx\n", pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie1_hose, first_free_busno);
	} else {
		printf ("    PCIE: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE); /* disable */
#endif
}
#endif

int board_eth_init(bd_t *bis)
{
	tsec_standard_init(bis);
	pci_eth_init(bis);
	return 0;	/* otherwise cpu_eth_init gets run */
}

int last_stage_init(void)
{
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_FSL_PCI_INIT
	FT_FSL_PCI_SETUP;
#endif
}
#endif
