/*
 * Copyright 2006, 2007 Freescale Semiconductor.
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
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_86xx.h>
#include <asm/immap_fsl_pci.h>
#include <spd.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdt_support.h>

#include "../common/pixis.h"

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

#if defined(CONFIG_SPD_EEPROM)
#include "spd_sdram.h"
#endif

void sdram_init(void);
long int fixed_sdram(void);


int board_early_init_f(void)
{
	return 0;
}

int checkboard(void)
{
	puts("Board: MPC8641HPCN\n");

	return 0;
}


long int
initdram(int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram();
#else
	dram_size = fixed_sdram();
#endif

#if defined(CFG_RAMBOOT)
	puts("    DDR: ");
	return dram_size;
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

	puts("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts("SDRAM test passed.\n");
	return 0;
}
#endif


#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
long int
fixed_sdram(void)
{
#if !defined(CFG_RAMBOOT)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr1;

	ddr->cs0_bnds = CFG_DDR_CS0_BNDS;
	ddr->cs0_config = CFG_DDR_CS0_CONFIG;
	ddr->ext_refrec = CFG_DDR_EXT_REFRESH;
	ddr->timing_cfg_0 = CFG_DDR_TIMING_0;
	ddr->timing_cfg_1 = CFG_DDR_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR_TIMING_2;
	ddr->sdram_mode_1 = CFG_DDR_MODE_1;
	ddr->sdram_mode_2 = CFG_DDR_MODE_2;
	ddr->sdram_interval = CFG_DDR_INTERVAL;
	ddr->sdram_data_init = CFG_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CFG_DDR_CLK_CTRL;
	ddr->sdram_ocd_cntl = CFG_DDR_OCD_CTRL;
	ddr->sdram_ocd_status = CFG_DDR_OCD_STATUS;

#if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000008D;
	ddr->err_sbe = 0x00ff0000;
#endif
	asm("sync;isync");

	udelay(500);

#if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg_1 = (CFG_DDR_CONTROL | 0x20000000);
#else
	ddr->sdram_cfg_1 = CFG_DDR_CONTROL;
	ddr->sdram_cfg_2 = CFG_DDR_CONTROL2;
#endif
	asm("sync; isync");

	udelay(500);
#endif
	return CFG_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */


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
	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC86xx_PORDEVSR_IO_SEL) >> 16;

#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;
#ifdef DEBUG
	uint host1_agent = (gur->porbmsr & MPC86xx_PORBMSR_HA) >> 17;
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
		printf ("    PCI-EXPRESS 1 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

		/*
		 * Activate ULI1575 legacy chip by performing a fake
		 * memory access.  Needed to make ULI RTC work.
		 */
		in_be32((unsigned *) ((char *)(CFG_PCI1_MEM_BASE
				       + CFG_PCI1_MEM_SIZE - 0x1000000)));

	} else {
		puts("PCI-EXPRESS 1: Disabled\n");
	}
}
#else
	puts("PCI-EXPRESS1: Disabled\n");
#endif /* CONFIG_PCI1 */

#ifdef CONFIG_PCI2
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI2_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci2_hose;


	/* inbound */
	pci_set_region(hose->regions + 0,
		       CFG_PCI_MEMORY_BUS,
		       CFG_PCI_MEMORY_PHYS,
		       CFG_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	/* outbound memory */
	pci_set_region(hose->regions + 1,
		       CFG_PCI2_MEM_BASE,
		       CFG_PCI2_MEM_PHYS,
		       CFG_PCI2_MEM_SIZE,
		       PCI_REGION_MEM);

	/* outbound io */
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
	printf ("    PCI-EXPRESS 2 on bus %02x - %02x\n",
		hose->first_busno,hose->last_busno);
}
#else
	puts("PCI-EXPRESS 2: Disabled\n");
#endif /* CONFIG_PCI2 */

}

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	int node, tmp[2];
	const char *path;

	fdt_fixup_ethernet(blob, bd);

	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", bd->bi_busfreq / 4, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "soc", 4,
			     "bus-frequency", bd->bi_busfreq, 1);

	do_fixup_by_compat_u32(blob, "ns16550",
			       "clock-frequency", bd->bi_busfreq, 1);

	fdt_fixup_memory(blob, bd->bi_memstart, bd->bi_memsize);

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
	}
}
#endif


/*
 * get_board_sys_clk
 *      Reads the FPGA on board for CONFIG_SYS_CLK_FREQ
 */

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
