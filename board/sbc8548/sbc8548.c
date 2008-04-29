/*
 * Copyright 2007 Wind River Systemes, Inc. <www.windriver.com>
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
#include <asm/immap_fsl_pci.h>
#include <spd_sdram.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

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
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);
	volatile u_char *rev= (void *)CFG_BD_REV;

	printf ("Board: Wind River SBC8548 Rev. 0x%01x\n",
			(*rev) >> 4);

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	/*
	 * Fix CPU2 errata: A core hang possible while executing a
	 * msync instruction and a snoopable transaction from an I/O
	 * master tagged to make quick forward progress is present.
	 */
	ecm->eebpcr |= (1 << 16);

	/*
	 * Hack TSEC 3 and 4 IO voltages.
	 */
	gur->tsec34ioovcr = 0xe7e0;	/*  1110 0111 1110 0xxx */

	ecm->eedr = 0xffffffff;		/* clear ecm errors */
	ecm->eeer = 0xffffffff;		/* enable ecm errors */
	return 0;
}

long int
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

		volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);

		gur->ddrdllcr = 0x81000000;
		asm("sync;isync;msync");
		udelay(200);
	}
#endif

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
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
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);

	uint clkdiv;
	uint lbc_hz;
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);
	clkdiv = (lbc->lcrr & 0x0f) * 2;
	lbc_hz = sysinfo.freqSystemBus / 1000000 / clkdiv;

	gur->lbiuiplldcr1 = 0x00078080;
	if (clkdiv == 16) {
		gur->lbiuiplldcr0 = 0x7c0f1bf0;
	} else if (clkdiv == 8) {
		gur->lbiuiplldcr0 = 0x6c0f1bf0;
	} else if (clkdiv == 4) {
		gur->lbiuiplldcr0 = 0x5c0f1bf0;
	}

	lbc->lcrr |= 0x00030000;

	asm("sync;isync;msync");

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
}

/*
 * Initialize SDRAM memory on the Local Bus.
 */
void
sdram_init(void)
{
#if defined(CFG_OR3_PRELIM) && defined(CFG_BR3_PRELIM)

	uint idx;
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	uint *sdram_addr = (uint *)CFG_LBC_SDRAM_BASE;
	uint lsdmr_common;

	puts("    SDRAM: ");

	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	lbc->or3 = CFG_OR3_PRELIM;
	asm("msync");

	lbc->br3 = CFG_BR3_PRELIM;
	asm("msync");

	lbc->lbcr = CFG_LBC_LBCR;
	asm("msync");


	lbc->lsrt = CFG_LBC_LSRT;
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm("msync");

	/*
	 * MPC8548 uses "new" 15-16 style addressing.
	 */
	lsdmr_common = CFG_LBC_LSDMR_COMMON;
	lsdmr_common |= CFG_LBC_LSDMR_BSMA1516;

	/*
	 * Issue PRECHARGE ALL command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_PCHALL;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue 8 AUTO REFRESH commands.
	 */
	for (idx = 0; idx < 8; idx++) {
		lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_ARFRSH;
		asm("sync;msync");
		*sdram_addr = 0xff;
		ppcDcbf((unsigned long) sdram_addr);
		udelay(100);
	}

	/*
	 * Issue 8 MODE-set command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_MRW;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue NORMAL OP command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_NORMAL;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(200);    /* Overkill. Must wait > 200 bus cycles */

#endif	/* enable SDRAM init */
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

#if	!defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed_sdram init -- doesn't use serial presence detect.
 *  assumes 256MB DDR2 SDRAM SODIMM, without ECC, running at DDR400 speed.
 ************************************************************************/
long int fixed_sdram (void)
{
    #define CFG_DDR_CONTROL 0xc300c000

	volatile ccsr_ddr_t *ddr = (void *)(CFG_MPC85xx_DDR_ADDR);

	ddr->cs0_bnds		= 0x0000007f;
	ddr->cs1_bnds		= 0x008000ff;
	ddr->cs2_bnds		= 0x00000000;
	ddr->cs3_bnds		= 0x00000000;
	ddr->cs0_config		= 0x80010101;
	ddr->cs1_config		= 0x80010101;
	ddr->cs2_config		= 0x00000000;
	ddr->cs3_config		= 0x00000000;
	ddr->timing_cfg_3		= 0x00000000;
	ddr->timing_cfg_0	= 0x00220802;
	ddr->timing_cfg_1	= 0x38377322;
	ddr->timing_cfg_2	= 0x0fa044C7;
	ddr->sdram_cfg		= 0x4300C000;
	ddr->sdram_cfg_2	= 0x24401000;
	ddr->sdram_mode		= 0x23C00542;
	ddr->sdram_mode_2	= 0x00000000;
	ddr->sdram_interval	= 0x05080100;
	ddr->sdram_md_cntl	= 0x00000000;
	ddr->sdram_data_init	= 0x00000000;
	ddr->sdram_clk_cntl 	= 0x03800000;
	asm("sync;isync;msync");
	udelay(500);

	#if defined (CONFIG_DDR_ECC)
	  /* Enable ECC checking */
	  ddr->sdram_cfg = (CFG_DDR_CONTROL | 0x20000000);
	#else
	  ddr->sdram_cfg = CFG_DDR_CONTROL;
	#endif

	return CFG_SDRAM_SIZE * 1024 * 1024;
}
#endif

#if defined(CONFIG_PCI) || defined(CONFIG_PCI1)
/* For some reason the Tundra PCI bridge shows up on itself as a
 * different device.  Work around that by refusing to configure it.
 */
void dummy_func(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab) { }

static struct pci_config_table pci_sbc8548_config_table[] = {
	{0x10e3, 0x0513, PCI_ANY_ID, 1, 3, PCI_ANY_ID, dummy_func, {0,0,0}},
	{0x1106, 0x0686, PCI_ANY_ID, 1, VIA_ID, 0, mpc85xx_config_via, {0,0,0}},
	{0x1106, 0x0571, PCI_ANY_ID, 1, VIA_ID, 1,
		mpc85xx_config_via_usbide, {0,0,0}},
	{0x1105, 0x3038, PCI_ANY_ID, 1, VIA_ID, 2,
		mpc85xx_config_via_usb, {0,0,0}},
	{0x1106, 0x3038, PCI_ANY_ID, 1, VIA_ID, 3,
		mpc85xx_config_via_usb2, {0,0,0}},
	{0x1106, 0x3058, PCI_ANY_ID, 1, VIA_ID, 5,
		mpc85xx_config_via_power, {0,0,0}},
	{0x1106, 0x3068, PCI_ANY_ID, 1, VIA_ID, 6,
		mpc85xx_config_via_ac97, {0,0,0}},
	{},
};

static struct pci_controller pci1_hose = {
	config_table: pci_sbc8548_config_table};
#endif	/* CONFIG_PCI */

#ifdef CONFIG_PCI2
static struct pci_controller pci2_hose;
#endif	/* CONFIG_PCI2 */

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif	/* CONFIG_PCIE1 */

int first_free_busno=0;

void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);

#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;
	struct pci_config_table *table;

	uint pci_32 = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;	/* PORDEVSR[15] */
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;	/* PORDEVSR[14] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;	/* PORPLLSR[16] */

	uint pci_agent = (host_agent == 3) || (host_agent == 4 ) || (host_agent == 6);

	uint pci_speed = get_clock_freq ();	/* PCI PSPEED in [4:5] */

	if (!(gur->devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf ("    PCI: %d bit, %s MHz, %s, %s, %s\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter"
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

		/* relocate config table pointers */
		hose->config_table = \
			(struct pci_config_table *)((uint)hose->config_table + gd->reloc_off);
		for (table = hose->config_table; table && table->vendor; table++)
			table->config_device += gd->reloc_off;

		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("PCI on bus %02x - %02x\n",hose->first_busno,hose->last_busno);
#ifdef CONFIG_PCIX_CHECK
		if (!(gur->pordevsr & PORDEVSR_PCI)) {
			/* PCI-X init */
			if (CONFIG_SYS_CLK_FREQ < 66000000)
				printf("PCI-X will only work at 66 MHz\n");

			reg16 = PCI_X_CMD_MAX_SPLIT | PCI_X_CMD_MAX_READ
				| PCI_X_CMD_ERO | PCI_X_CMD_DPERR_E;
			pci_hose_write_config_word(hose, bus, PCIX_COMMAND, reg16);
		}
#endif
	} else {
		printf ("    PCI: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif

#ifdef CONFIG_PCI2
{
	uint pci2_clk_sel = gur->porpllsr & 0x4000;	/* PORPLLSR[17] */
	uint pci_dual = get_pci_dual ();	/* PCI DUAL in CM_PCI[3] */
	if (pci_dual) {
		printf ("    PCI2: 32 bit, 66 MHz, %s\n",
			pci2_clk_sel ? "sync" : "async");
	} else {
		printf ("    PCI2: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI2; /* disable */
#endif /* CONFIG_PCI2 */

#ifdef CONFIG_PCIE1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep =  (host_agent == 0) || (host_agent == 2 ) || (host_agent == 3);

	int pcie_configured  = io_sel >= 1;

	if (pcie_configured && !(gur->devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE connected to slot as %s (base address %x)",
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

		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		printf ("PCIE on bus %d - %d\n",hose->first_busno,hose->last_busno);

		first_free_busno=hose->last_busno+1;

	} else {
		printf ("    PCIE: disabled\n");
	}
 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE; /* disable */
#endif

}

int last_stage_init(void)
{
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_pci_setup(void *blob, bd_t *bd)
{
	int node, tmp[2];

	node = fdt_path_offset(blob, "/aliases");
	tmp[0] = 0;
	if (node >= 0) {
#ifdef CONFIG_PCI1
		const char *path;
		path = fdt_getprop(blob, node, "pci0", NULL);
		if (path) {
			tmp[1] = pci1_hose.last_busno - pci1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
#ifdef CONFIG_PCIE1
		const char *path;
		path = fdt_getprop(blob, node, "pci1", NULL);
		if (path) {
			tmp[1] = pcie1_hose.last_busno - pcie1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
	}
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif
