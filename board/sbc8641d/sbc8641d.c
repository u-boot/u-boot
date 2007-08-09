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
#include <spd.h>

#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
extern void ft_cpu_setup (void *blob, bd_t * bd);
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc (unsigned int dram_size);
#endif

#if defined(CONFIG_SPD_EEPROM)
#include "spd_sdram.h"
#endif

void sdram_init (void);
long int fixed_sdram (void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	puts ("Board: Wind River SBC8641D\n");

#ifdef CONFIG_PCI

	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile ccsr_pex_t *pex1 = &immap->im_pex1;

	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC86xx_PORDEVSR_IO_SEL) >> 16;
	uint host1_agent = (gur->porbmsr & MPC86xx_PORBMSR_HA) >> 17;
	uint pex1_agent = (host1_agent == 0) || (host1_agent == 1);

	if ((io_sel == 2 || io_sel == 3 || io_sel == 5
	     || io_sel == 6 || io_sel == 7 || io_sel == 0xF)
	    && !(devdisr & MPC86xx_DEVDISR_PCIEX1)) {
		debug ("PCI-EXPRESS 1: %s \n", pex1_agent ? "Agent" : "Host");
		debug ("0x%08x=0x%08x ", &pex1->pme_msg_det, pex1->pme_msg_det);
		if (pex1->pme_msg_det) {
			pex1->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",
			       pex1->pme_msg_det);
		}
		debug ("\n");
	} else {
		puts ("PCI-EXPRESS 1: Disabled in hardware\n");
	}

#else
	puts ("PCI-EXPRESS1: Disabled in configuration\n");
#endif

	return 0;
}

long int initdram (int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif

#if defined(CFG_RAMBOOT)
	puts ("    DDR: ");
	return dram_size;
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc (dram_size);
#endif

	puts ("    DDR: ");
	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
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
#if !defined(CFG_RAMBOOT)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr1;

	ddr->cs0_bnds = CFG_DDR_CS0_BNDS;
	ddr->cs1_bnds = CFG_DDR_CS1_BNDS;
	ddr->cs2_bnds = CFG_DDR_CS2_BNDS;
	ddr->cs3_bnds = CFG_DDR_CS3_BNDS;
	ddr->cs0_config = CFG_DDR_CS0_CONFIG;
	ddr->cs1_config = CFG_DDR_CS1_CONFIG;
	ddr->cs2_config = CFG_DDR_CS2_CONFIG;
	ddr->cs3_config = CFG_DDR_CS3_CONFIG;
	ddr->ext_refrec = CFG_DDR_EXT_REFRESH;
	ddr->timing_cfg_0 = CFG_DDR_TIMING_0;
	ddr->timing_cfg_1 = CFG_DDR_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR_TIMING_2;
	ddr->sdram_cfg_1 = CFG_DDR_CFG_1A;
	ddr->sdram_cfg_2 = CFG_DDR_CFG_2;
	ddr->sdram_mode_1 = CFG_DDR_MODE_1;
	ddr->sdram_mode_2 = CFG_DDR_MODE_2;
	ddr->sdram_mode_cntl = CFG_DDR_MODE_CTL;
	ddr->sdram_interval = CFG_DDR_INTERVAL;
	ddr->sdram_data_init = CFG_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CFG_DDR_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg_1 = CFG_DDR_CFG_1B;
	asm ("sync; isync");

	udelay (500);
	ddr = &immap->im_ddr2;

	ddr->cs0_bnds = CFG_DDR2_CS0_BNDS;
	ddr->cs1_bnds = CFG_DDR2_CS1_BNDS;
	ddr->cs2_bnds = CFG_DDR2_CS2_BNDS;
	ddr->cs3_bnds = CFG_DDR2_CS3_BNDS;
	ddr->cs0_config = CFG_DDR2_CS0_CONFIG;
	ddr->cs1_config = CFG_DDR2_CS1_CONFIG;
	ddr->cs2_config = CFG_DDR2_CS2_CONFIG;
	ddr->cs3_config = CFG_DDR2_CS3_CONFIG;
	ddr->ext_refrec = CFG_DDR2_EXT_REFRESH;
	ddr->timing_cfg_0 = CFG_DDR2_TIMING_0;
	ddr->timing_cfg_1 = CFG_DDR2_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR2_TIMING_2;
	ddr->sdram_cfg_1 = CFG_DDR2_CFG_1A;
	ddr->sdram_cfg_2 = CFG_DDR2_CFG_2;
	ddr->sdram_mode_1 = CFG_DDR2_MODE_1;
	ddr->sdram_mode_2 = CFG_DDR2_MODE_2;
	ddr->sdram_mode_cntl = CFG_DDR2_MODE_CTL;
	ddr->sdram_interval = CFG_DDR2_INTERVAL;
	ddr->sdram_data_init = CFG_DDR2_DATA_INIT;
	ddr->sdram_clk_cntl = CFG_DDR2_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg_1 = CFG_DDR2_CFG_1B;
	asm ("sync; isync");

	udelay (500);
#endif
	return CFG_SDRAM_SIZE * 1024 * 1024;
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

static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
      config_table:pci_mpc86xxcts_config_table,
#endif
};

#endif				/* CONFIG_PCI */

void pci_init_board (void)
{
#ifdef CONFIG_PCI
	extern void pci_mpc86xx_init (struct pci_controller *hose);

	pci_mpc86xx_init (&hose);
#endif				/* CONFIG_PCI */
}

#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup (void *blob, bd_t * bd)
{
	u32 *p;
	int len;

	ft_cpu_setup (blob, bd);

	p = ft_get_prop (blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32 (bd->bi_memstart);
		*p = cpu_to_be32 (bd->bi_memsize);
	}
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
