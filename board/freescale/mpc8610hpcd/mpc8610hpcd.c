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
#include <asm/immap_fsl_pci.h>
#include <i2c.h>
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
void mpc8610hpcd_diu_init(void);


/* called before any console output */
int board_early_init_f(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;

	gur->gpiocr |= 0x88aa5500; /* DIU16, IR1, UART0, UART2 */

	return 0;
}

int misc_init_r(void)
{
	u8 tmp_val, version;

	/*Do not use 8259PIC*/
	tmp_val = in8(PIXIS_BASE + PIXIS_BRDCFG0);
	out8(PIXIS_BASE + PIXIS_BRDCFG0, tmp_val | 0x80);

	/*For FPGA V7 or higher, set the IRQMAPSEL to 0 to use MAP0 interrupt*/
	version = in8(PIXIS_BASE + PIXIS_PVER);
	if(version >= 0x07) {
		tmp_val = in8(PIXIS_BASE + PIXIS_BRDCFG0);
		out8(PIXIS_BASE + PIXIS_BRDCFG0, tmp_val & 0xbf);
	}

	/* Using this for DIU init before the driver in linux takes over
	 *  Enable the TFP410 Encoder (I2C address 0x38)
	 */

	tmp_val = 0xBF;
	i2c_write(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02lx\n",tmp_val);

	tmp_val = 0x10;
	i2c_write(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02lx\n",tmp_val);

#ifdef CONFIG_FSL_DIU_FB
	mpc8610hpcd_diu_init();
#endif

	return 0;
}

int checkboard(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_local_mcm_t *mcm = &immap->im_local_mcm;

	puts("Board: MPC8610HPCD\n");

	mcm->abcr |= 0x00010000; /* 0 */
	mcm->hpmr3 = 0x80000008; /* 4c */
	mcm->hpmr0 = 0;
	mcm->hpmr1 = 0;
	mcm->hpmr2 = 0;
	mcm->hpmr4 = 0;
	mcm->hpmr5 = 0;

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
	puts(" DDR: ");
	return dram_size;
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif

	puts(" DDR: ");
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

long int fixed_sdram(void)
{
#if !defined(CFG_RAMBOOT)
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr1;
	uint d_init;

	ddr->cs0_bnds = 0x0000001f;
	ddr->cs0_config = 0x80010202;

	ddr->ext_refrec = 0x00000000;
	ddr->timing_cfg_0 = 0x00260802;
	ddr->timing_cfg_1 = 0x3935d322;
	ddr->timing_cfg_2 = 0x14904cc8;
	ddr->sdram_mode_1 = 0x00480432;
	ddr->sdram_mode_2 = 0x00000000;
	ddr->sdram_interval = 0x06180fff; /* 0x06180100; */
	ddr->sdram_data_init = 0xDEADBEEF;
	ddr->sdram_clk_cntl = 0x03800000;
	ddr->sdram_cfg_2 = 0x04400010;

#if defined(CONFIG_DDR_ECC)
	ddr->err_int_en = 0x0000000d;
	ddr->err_disable = 0x00000000;
	ddr->err_sbe = 0x00010000;
#endif
	asm("sync;isync");

	udelay(500);

	ddr->sdram_cfg_1 = 0xc3000000; /* 0xe3008000;*/


#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	d_init = 1;
	debug("DDR - 1st controller: memory initializing\n");
	/*
	 * Poll until memory is initialized.
	 * 512 Meg at 400 might hit this 200 times or so.
	 */
	while ((ddr->sdram_cfg_2 & (d_init << 4)) != 0)
		udelay(1000);

	debug("DDR: memory initialized\n\n");
	asm("sync; isync");
	udelay(500);
#endif

	return 512 * 1024 * 1024;
#endif
	return CFG_SDRAM_SIZE * 1024 * 1024;
}

#endif

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
				 PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER} },
	{}
};
#endif


static struct pci_controller pci1_hose = {
#ifndef CONFIG_PCI_PNP
config_table:pci_mpc86xxcts_config_table
#endif
};
#endif /* CONFIG_PCI */

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif

#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif

int first_free_busno = 0;

void pci_init_board(void)
{
	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC8610_PORDEVSR_IO_SEL)
		>> MPC8610_PORDEVSR_IO_SEL_SHIFT;
	uint host_agent = (gur->porbmsr & MPC8610_PORBMSR_HA)
		>> MPC8610_PORBMSR_HA_SHIFT;

	printf( " pci_init_board: devdisr=%x, io_sel=%x, host_agent=%x\n",
		devdisr, io_sel, host_agent);

#ifdef CONFIG_PCIE1
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie1_hose;
	int pcie_configured = (io_sel == 1) || (io_sel == 4);
	int pcie_ep = (host_agent == 0) || (host_agent == 2) ||
		(host_agent == 5);

	if (pcie_configured && !(devdisr & MPC86xx_DEVDISR_PCIE1)) {
		printf(" PCIe 1 connected to Uli as %s (base address %x)\n",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det)
			pci->pme_msg_det = 0xffffffff;

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

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
				 (int)&pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno = hose->last_busno + 1;
		printf(" PCI-Express 1 on bus %02x - %02x\n",
			hose->first_busno, hose->last_busno);

	} else
		puts(" PCI-Express 1: Disabled\n");
 }
#else
	puts("PCI-Express 1: Disabled\n");
#endif /* CONFIG_PCIE1 */


#ifdef CONFIG_PCIE2
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE2_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie2_hose;

	int pcie_configured = (io_sel == 0) || (io_sel == 4);
	int pcie_ep = (host_agent == 0) || (host_agent == 1) ||
		(host_agent == 4);

	if (pcie_configured && !(devdisr & MPC86xx_DEVDISR_PCIE2)) {
		printf(" PCI-Express 2 connected to slot as %s" \
			" (base address %x)\n",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det)
			pci->pme_msg_det = 0xffffffff;

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

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int)&pci->cfg_addr,
				 (int)&pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno = hose->last_busno + 1;
		printf(" PCI-Express 2 on bus %02x - %02x\n",
			hose->first_busno, hose->last_busno);
	} else
		puts(" PCI-Express 2: Disabled\n");
 }
#else
	puts("PCI-Express 2: Disabled\n");
#endif /* CONFIG_PCIE2 */


#ifdef CONFIG_PCI1
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;
	int pci_agent = (host_agent >= 4) && (host_agent <= 6);

	if ( !(devdisr & MPC86xx_DEVDISR_PCI1)) {
		printf(" PCI connected to PCI slots as %s" \
			" (base address %x)\n",
			pci_agent ? "Agent" : "Host",
			(uint)pci);

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

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr,
				 (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno = hose->last_busno + 1;
		printf(" PCI on bus %02x - %02x\n",
			hose->first_busno, hose->last_busno);


	} else
		puts(" PCI: Disabled\n");
 }
#endif /* CONFIG_PCI1 */
}

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	int node, tmp[2];
	const char *path;

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
#ifdef CONFIG_PCIE1
		path = fdt_getprop(blob, node, "pci1", NULL);
		if (path) {
			tmp[1] = pcie1_hose.last_busno
				- pcie1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
	}
#endif
#ifdef CONFIG_PCIE2
		path = fdt_getprop(blob, node, "pci2", NULL);
		if (path) {
			tmp[1] = pcie2_hose.last_busno
				- pcie2_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
	}
}
#endif

/*
 * get_board_sys_clk
 * Reads the FPGA on board for CONFIG_SYS_CLK_FREQ
 */

unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 i;
	ulong val = 0;
	ulong a;

	a = PIXIS_BASE + PIXIS_SPD;
	i = in8(a);
	i &= 0x07;

	switch (i) {
	case 0:
		val = 33333000;
		break;
	case 1:
		val = 39999600;
		break;
	case 2:
		val = 49999500;
		break;
	case 3:
		val = 66666000;
		break;
	case 4:
		val = 83332500;
		break;
	case 5:
		val = 99999000;
		break;
	case 6:
		val = 133332000;
		break;
	case 7:
		val = 166665000;
		break;
	}

	return val;
}
