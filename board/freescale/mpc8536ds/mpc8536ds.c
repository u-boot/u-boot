/*
 * Copyright 2008 Freescale Semiconductor.
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
#include <spd.h>
#include <miiphy.h>
#include <libfdt.h>
#include <spd_sdram.h>
#include <fdt_support.h>
#include <tsec.h>
#include <netdev.h>
#include <sata.h>

#include "../common/pixis.h"
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

int first_free_busno=0;

void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	uint devdisr = gur->devdisr;
	uint sdrs2_io_sel =
		(gur->pordevsr & MPC85xx_PORDEVSR_SRDS2_IO_SEL) >> 27;
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	uint host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

	debug("   pci_init_board: devdisr=%x, sdrs2_io_sel=%x, io_sel=%x,\
		host_agent=%x\n", devdisr, sdrs2_io_sel, io_sel, host_agent);

	if (sdrs2_io_sel == 7)
		printf("    Serdes2 disalbed\n");
	else if (sdrs2_io_sel == 4) {
		printf("    eTSEC1 is in sgmii mode.\n");
		printf("    eTSEC3 is in sgmii mode.\n");
	} else if (sdrs2_io_sel == 6)
		printf("    eTSEC1 is in sgmii mode.\n");

#ifdef CONFIG_PCIE3
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE3_ADDR;
	struct pci_controller *hose = &pcie3_hose;
	int pcie_ep = (host_agent == 1);
	int pcie_configured  = (io_sel == 7);
	struct pci_region *r = hose->regions;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE3 connected to Slot3 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE3_MEM_BUS,
			       CONFIG_SYS_PCIE3_MEM_PHYS,
			       CONFIG_SYS_PCIE3_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE3_IO_BUS,
			       CONFIG_SYS_PCIE3_IO_PHYS,
			       CONFIG_SYS_PCIE3_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = r - hose->regions;

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
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE1_ADDR;
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep = (host_agent == 5);
	int pcie_configured  = (io_sel == 2 || io_sel == 3
				|| io_sel == 5 || io_sel == 7);
	struct pci_region *r = hose->regions;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE1 connected to Slot1 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE1_MEM_BUS,
			       CONFIG_SYS_PCIE1_MEM_PHYS,
			       CONFIG_SYS_PCIE1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE1_IO_BUS,
			       CONFIG_SYS_PCIE1_IO_PHYS,
			       CONFIG_SYS_PCIE1_IO_SIZE,
			       PCI_REGION_IO);

#ifdef CONFIG_SYS_PCIE1_MEM_BUS2
		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE1_MEM_BUS2,
			       CONFIG_SYS_PCIE1_MEM_PHYS2,
			       CONFIG_SYS_PCIE1_MEM_SIZE2,
			       PCI_REGION_MEM);
#endif
		hose->region_count = r - hose->regions;
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
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE2_ADDR;
	struct pci_controller *hose = &pcie2_hose;
	int pcie_ep = (host_agent == 3);
	int pcie_configured  = (io_sel == 5 || io_sel == 7);
	struct pci_region *r = hose->regions;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE2 connected to Slot 2 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		r += fsl_pci_setup_inbound_windows(r);

		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE2_MEM_BUS,
			       CONFIG_SYS_PCIE2_MEM_PHYS,
			       CONFIG_SYS_PCIE2_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE2_IO_BUS,
			       CONFIG_SYS_PCIE2_IO_PHYS,
			       CONFIG_SYS_PCIE2_IO_SIZE,
			       PCI_REGION_IO);

#ifdef CONFIG_SYS_PCIE2_MEM_BUS2
		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCIE2_MEM_BUS2,
			       CONFIG_SYS_PCIE2_MEM_PHYS2,
			       CONFIG_SYS_PCIE2_MEM_SIZE2,
			       PCI_REGION_MEM);
#endif
		hose->region_count = r - hose->regions;
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
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCI1_ADDR;
	struct pci_controller *hose = &pci1_hose;
	struct pci_region *r = hose->regions;

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

#ifdef CONFIG_SYS_PCI1_MEM_BUS2
		/* outbound memory */
		pci_set_region(r++,
			       CONFIG_SYS_PCI1_MEM_BUS2,
			       CONFIG_SYS_PCI1_MEM_PHYS2,
			       CONFIG_SYS_PCI1_MEM_SIZE2,
			       PCI_REGION_MEM);
#endif
		hose->region_count = r - hose->regions;
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


int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = 1;

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

#ifdef CONFIG_GET_CLK_FROM_ICS307
/* decode S[0-2] to Output Divider (OD) */
static unsigned char
ics307_S_to_OD[] = {
	10, 2, 8, 4, 5, 7, 3, 6
};

/* Calculate frequency being generated by ICS307-02 clock chip based upon
 * the control bytes being programmed into it. */
/* XXX: This function should probably go into a common library */
static unsigned long
ics307_clk_freq (unsigned char cw0, unsigned char cw1, unsigned char cw2)
{
	const unsigned long long InputFrequency = CONFIG_ICS307_REFCLK_HZ;
	unsigned long VDW = ((cw1 << 1) & 0x1FE) + ((cw2 >> 7) & 1);
	unsigned long RDW = cw2 & 0x7F;
	unsigned long OD = ics307_S_to_OD[cw0 & 0x7];
	unsigned long freq;

	/* CLK1Frequency = InputFrequency * 2 * (VDW + 8) / ((RDW + 2) * OD) */

	/* cw0:  C1 C0 TTL F1 F0 S2 S1 S0
	 * cw1:  V8 V7 V6 V5 V4 V3 V2 V1
	 * cw2:  V0 R6 R5 R4 R3 R2 R1 R0
	 *
	 * R6:R0 = Reference Divider Word (RDW)
	 * V8:V0 = VCO Divider Word (VDW)
	 * S2:S0 = Output Divider Select (OD)
	 * F1:F0 = Function of CLK2 Output
	 * TTL = duty cycle
	 * C1:C0 = internal load capacitance for cyrstal
	 */

	/* Adding 1 to get a "nicely" rounded number, but this needs
	 * more tweaking to get a "properly" rounded number. */

	freq = 1 + (InputFrequency * 2 * (VDW + 8) / ((RDW + 2) * OD));

	debug("ICS307: CW[0-2]: %02X %02X %02X => %u Hz\n", cw0, cw1, cw2,
		freq);
	return freq;
}

unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	return ics307_clk_freq (
	    in_8(pixis_base + PIXIS_VSYSCLK0),
	    in_8(pixis_base + PIXIS_VSYSCLK1),
	    in_8(pixis_base + PIXIS_VSYSCLK2)
	);
}

unsigned long
get_board_ddr_clk(ulong dummy)
{
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	return ics307_clk_freq (
	    in_8(pixis_base + PIXIS_VDDRCLK0),
	    in_8(pixis_base + PIXIS_VDDRCLK1),
	    in_8(pixis_base + PIXIS_VDDRCLK2)
	);
}
#else
unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 i;
	ulong val = 0;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	i = in_8(pixis_base + PIXIS_SPD);
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
		val = 83333333;
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

unsigned long
get_board_ddr_clk(ulong dummy)
{
	u8 i;
	ulong val = 0;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	i = in_8(pixis_base + PIXIS_SPD);
	i &= 0x38;
	i >>= 3;

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
		val = 83333333;
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
#endif

int sata_initialize(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	uint sdrs2_io_sel =
		(gur->pordevsr & MPC85xx_PORDEVSR_SRDS2_IO_SEL) >> 27;
	if (sdrs2_io_sel & 0x04)
		return 1;

	return __sata_initialize();
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

#ifdef CONFIG_PCI1
	ft_fsl_pci_setup(blob, "pci0", &pci1_hose);
#endif
#ifdef CONFIG_PCIE2
	ft_fsl_pci_setup(blob, "pci1", &pcie2_hose);
#endif
#ifdef CONFIG_PCIE2
	ft_fsl_pci_setup(blob, "pci2", &pcie1_hose);
#endif
#ifdef CONFIG_PCIE1
	ft_fsl_pci_setup(blob, "pci3", &pcie3_hose);
#endif
#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif
}
#endif
