/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
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
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <spd.h>
#include <miiphy.h>
#include <libfdt.h>
#include <spd_sdram.h>
#include <fdt_support.h>
#include <tsec.h>
#include <netdev.h>
#include <sata.h>

#include "../common/sgmii_riser.h"

phys_size_t fixed_sdram(void);

int board_early_init_f (void)
{
#ifdef CONFIG_MMC
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SD_DATA |
			 MPC85xx_PMUXCR_SDHC_CD |
			 MPC85xx_PMUXCR_SDHC_WP));

#endif
	return 0;
}

int checkboard (void)
{
	u8 vboot;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	puts("Board: MPC8536DS ");
#ifdef CONFIG_PHYS_64BIT
	puts("(36-bit addrmap) ");
#endif

	printf ("Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	vboot = in_8(pixis_base + PIXIS_VBOOT);
	switch ((vboot & PIXIS_VBOOT_LBMAP) >> 5) {
		case PIXIS_VBOOT_LBMAP_NOR0:
			puts ("vBank: 0\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR1:
			puts ("vBank: 1\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR2:
			puts ("vBank: 2\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR3:
			puts ("vBank: 3\n");
			break;
		case PIXIS_VBOOT_LBMAP_PJET:
			puts ("Promjet\n");
			break;
		case PIXIS_VBOOT_LBMAP_NAND:
			puts ("NAND\n");
			break;
	}

	return 0;
}

phys_size_t
initdram(int board_type)
{
	phys_size_t dram_size = 0;

	puts("Initializing....");

#ifdef CONFIG_SPD_EEPROM
	dram_size = fsl_ddr_sdram();
#else
	dram_size = fixed_sdram();
#endif
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	puts("    DDR: ");
	return dram_size;
}

#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */

phys_size_t fixed_sdram (void)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_ddr_t *ddr= &immap->im_ddr;
	uint d_init;

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;

	ddr->timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR_MODE_2;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL2;

#if defined (CONFIG_DDR_ECC)
	ddr->err_int_en = CONFIG_SYS_DDR_ERR_INT_EN;
	ddr->err_disable = CONFIG_SYS_DDR_ERR_DIS;
	ddr->err_sbe = CONFIG_SYS_DDR_SBE;
#endif
	asm("sync;isync");

	udelay(500);

	ddr->sdram_cfg = CONFIG_SYS_DDR_CONTROL;

#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	d_init = 1;
	debug("DDR - 1st controller: memory initializing\n");
	/*
	 * Poll until memory is initialized.
	 * 512 Meg at 400 might hit this 200 times or so.
	 */
	while ((ddr->sdram_cfg_2 & (d_init << 4)) != 0) {
		udelay(1000);
	}
	debug("DDR: memory initialized\n\n");
	asm("sync; isync");
	udelay(500);
#endif

	return 512 * 1024 * 1024;
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

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info[4];
	u32 devdisr, pordevsr, io_sel, sdrs2_io_sel;
	u32 porpllsr, pci_agent, pci_speed, pci_32, pci_arb, pci_clk_sel;
	int first_free_busno = 0;
	int num = 0;

	int pcie_ep, pcie_configured;

	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	porpllsr = in_be32(&gur->porpllsr);
	io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	sdrs2_io_sel = (pordevsr & MPC85xx_PORDEVSR_SRDS2_IO_SEL) >> 27;

	debug("   pci_init_board: devdisr=%x, sdrs2_io_sel=%x, io_sel=%x\n",
		devdisr, sdrs2_io_sel, io_sel);

	if (sdrs2_io_sel == 7)
		printf("    Serdes2 disalbed\n");
	else if (sdrs2_io_sel == 4) {
		printf("    eTSEC1 is in sgmii mode.\n");
		printf("    eTSEC3 is in sgmii mode.\n");
	} else if (sdrs2_io_sel == 6)
		printf("    eTSEC1 is in sgmii mode.\n");

	puts("\n");
#ifdef CONFIG_PCIE3
	pcie_configured = is_serdes_configured(PCIE3);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE3)){
		set_next_law(CONFIG_SYS_PCIE3_MEM_PHYS, LAW_SIZE_512M,
				LAW_TRGT_IF_PCIE_3);
		set_next_law(CONFIG_SYS_PCIE3_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_3);
		SET_STD_PCIE_INFO(pci_info[num], 3);
		pcie_ep = fsl_setup_hose(&pcie3_hose, pci_info[num].regs);
		printf ("    PCIE3 connected to Slot3 as %s (base address %lx)\n",
			pcie_ep ? "Endpoint" : "Root Complex",
			pci_info[num].regs);
		first_free_busno = fsl_pci_init_port(&pci_info[num++],
					&pcie3_hose, first_free_busno);
	} else {
		printf ("    PCIE3: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE3); /* disable */
#endif

#ifdef CONFIG_PCIE1
	pcie_configured = is_serdes_configured(PCIE1);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		set_next_law(CONFIG_SYS_PCIE1_MEM_PHYS, LAW_SIZE_128M,
				LAW_TRGT_IF_PCIE_1);
		set_next_law(CONFIG_SYS_PCIE1_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_1);
		SET_STD_PCIE_INFO(pci_info[num], 1);
		pcie_ep = fsl_setup_hose(&pcie1_hose, pci_info[num].regs);
		printf ("    PCIE1 connected to Slot1 as %s (base address %lx)\n",
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

#ifdef CONFIG_PCIE2
	pcie_configured = is_serdes_configured(PCIE2);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE2)){
		set_next_law(CONFIG_SYS_PCIE2_MEM_PHYS, LAW_SIZE_128M,
				LAW_TRGT_IF_PCIE_2);
		set_next_law(CONFIG_SYS_PCIE2_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCIE_2);
		SET_STD_PCIE_INFO(pci_info[num], 2);
		pcie_ep = fsl_setup_hose(&pcie2_hose, pci_info[num].regs);
		printf ("    PCIE2 connected to Slot 2 as %s (base address %lx)\n",
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

#ifdef CONFIG_PCI1
	pci_speed = 66666000;
	pci_32 = 1;
	pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		set_next_law(CONFIG_SYS_PCI1_MEM_PHYS, LAW_SIZE_256M,
				LAW_TRGT_IF_PCI);
		set_next_law(CONFIG_SYS_PCI1_IO_PHYS, LAW_SIZE_64K,
				LAW_TRGT_IF_PCI);
		SET_STD_PCI_INFO(pci_info[num], 1);
		pci_agent = fsl_setup_hose(&pci1_hose, pci_info[num].regs);
		printf ("\n    PCI: %d bit, %s MHz, %s, %s, %s (base address %lx)\n",
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
}
#endif

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash + promjet */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,	/* tlb, epn, rpn */
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,	/* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_256M, 1);	/* ts, esel, tsize, iprot */

	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	struct tsec_info_struct tsec_info[2];
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int num = 0;
	uint sdrs2_io_sel =
		(gur->pordevsr & MPC85xx_PORDEVSR_SRDS2_IO_SEL) >> 27;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if ((sdrs2_io_sel == 4) || (sdrs2_io_sel == 6)) {
		tsec_info[num].phyaddr = 0;
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	if (sdrs2_io_sel == 4) {
		tsec_info[num].phyaddr = 1;
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

#ifdef CONFIG_FSL_SGMII_RISER
	if ((sdrs2_io_sel == 4) || (sdrs2_io_sel == 6))
		fsl_sgmii_riser_init(tsec_info, num);
#endif

	tsec_eth_init(bis, tsec_info, num);
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif
}
#endif
