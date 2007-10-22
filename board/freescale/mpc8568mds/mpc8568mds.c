/*
 * Copyright 2007 Freescale Semiconductor.
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
#include <spd.h>
#include <i2c.h>
#include <ioports.h>
#include <libfdt.h>
#include <fdt_support.h>

#include "bcsr.h"

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* GETH1 */
	{4, 10, 1, 0, 2}, /* TxD0 */
	{4,  9, 1, 0, 2}, /* TxD1 */
	{4,  8, 1, 0, 2}, /* TxD2 */
	{4,  7, 1, 0, 2}, /* TxD3 */
	{4, 23, 1, 0, 2}, /* TxD4 */
	{4, 22, 1, 0, 2}, /* TxD5 */
	{4, 21, 1, 0, 2}, /* TxD6 */
	{4, 20, 1, 0, 2}, /* TxD7 */
	{4, 15, 2, 0, 2}, /* RxD0 */
	{4, 14, 2, 0, 2}, /* RxD1 */
	{4, 13, 2, 0, 2}, /* RxD2 */
	{4, 12, 2, 0, 2}, /* RxD3 */
	{4, 29, 2, 0, 2}, /* RxD4 */
	{4, 28, 2, 0, 2}, /* RxD5 */
	{4, 27, 2, 0, 2}, /* RxD6 */
	{4, 26, 2, 0, 2}, /* RxD7 */
	{4, 11, 1, 0, 2}, /* TX_EN */
	{4, 24, 1, 0, 2}, /* TX_ER */
	{4, 16, 2, 0, 2}, /* RX_DV */
	{4, 30, 2, 0, 2}, /* RX_ER */
	{4, 17, 2, 0, 2}, /* RX_CLK */
	{4, 19, 1, 0, 2}, /* GTX_CLK */
	{1, 31, 2, 0, 3}, /* GTX125 */

	/* GETH2 */
	{5, 10, 1, 0, 2}, /* TxD0 */
	{5,  9, 1, 0, 2}, /* TxD1 */
	{5,  8, 1, 0, 2}, /* TxD2 */
	{5,  7, 1, 0, 2}, /* TxD3 */
	{5, 23, 1, 0, 2}, /* TxD4 */
	{5, 22, 1, 0, 2}, /* TxD5 */
	{5, 21, 1, 0, 2}, /* TxD6 */
	{5, 20, 1, 0, 2}, /* TxD7 */
	{5, 15, 2, 0, 2}, /* RxD0 */
	{5, 14, 2, 0, 2}, /* RxD1 */
	{5, 13, 2, 0, 2}, /* RxD2 */
	{5, 12, 2, 0, 2}, /* RxD3 */
	{5, 29, 2, 0, 2}, /* RxD4 */
	{5, 28, 2, 0, 2}, /* RxD5 */
	{5, 27, 2, 0, 3}, /* RxD6 */
	{5, 26, 2, 0, 2}, /* RxD7 */
	{5, 11, 1, 0, 2}, /* TX_EN */
	{5, 24, 1, 0, 2}, /* TX_ER */
	{5, 16, 2, 0, 2}, /* RX_DV */
	{5, 30, 2, 0, 2}, /* RX_ER */
	{5, 17, 2, 0, 2}, /* RX_CLK */
	{5, 19, 1, 0, 2}, /* GTX_CLK */
	{1, 31, 2, 0, 3}, /* GTX125 */
	{4,  6, 3, 0, 2}, /* MDIO */
	{4,  5, 1, 0, 2}, /* MDC */
	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};


#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

extern long int spd_sdram(void);

void local_bus_init(void);
void sdram_init(void);

int board_early_init_f (void)
{
	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	enable_8568mds_duart();
	enable_8568mds_flash_write();
#if defined(CONFIG_UEC_ETH1) || defined(CONFIG_UEC_ETH2)
	reset_8568mds_uccs();
#endif
#if defined(CONFIG_QE) && !defined(CONFIG_eTSEC_MDIO_BUS)
	enable_8568mds_qe_mdio();
#endif

#ifdef CFG_I2C2_OFFSET
	/* Enable I2C2_SCL and I2C2_SDA */
	volatile struct par_io *port_c;
	port_c = (struct par_io*)(CFG_IMMR + 0xe0140);
	port_c->cpdir2 |= 0x0f000000;
	port_c->cppar2 &= ~0x0f000000;
	port_c->cppar2 |= 0x0a000000;
#endif

	return 0;
}

int checkboard (void)
{
	printf ("Board: 8568 MDS\n");

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
	dram_size = spd_sdram();

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
}

/*
 * Initialize SDRAM memory on the Local Bus.
 */
void
sdram_init(void)
{
#if defined(CFG_OR2_PRELIM) && defined(CFG_BR2_PRELIM)

	uint idx;
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	uint *sdram_addr = (uint *)CFG_LBC_SDRAM_BASE;
	uint lsdmr_common;

	puts("    SDRAM: ");

	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	lbc->or2 = CFG_OR2_PRELIM;
	asm("msync");

	lbc->br2 = CFG_BR2_PRELIM;
	asm("msync");

	lbc->lbcr = CFG_LBC_LBCR;
	asm("msync");


	lbc->lsrt = CFG_LBC_LSRT;
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm("msync");

	/*
	 * MPC8568 uses "new" 15-16 style addressing.
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

#if defined(CONFIG_PCI)
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc8568mds_config_table[] = {
	{
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 pci_cfgfunc_config_device,
	 {PCI_ENET0_IOADDR,
	  PCI_ENET0_MEMADDR,
	  PCI_COMMON_MEMORY | PCI_COMMAND_MASTER}
	 },
	{}
};
#endif

static struct pci_controller pci1_hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_mpc8568mds_config_table,
#endif
};
#endif	/* CONFIG_PCI */

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif  /* CONFIG_PCIE1 */

int first_free_busno = 0;

/*
 * pib_init() -- Initialize the PCA9555 IO expander on the PIB board
 */
void
pib_init(void)
{
	u8 val8, orig_i2c_bus;
	/*
	 * Assign PIB PMC2/3 to PCI bus
	 */

	/*switch temporarily to I2C bus #2 */
	orig_i2c_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	val8 = 0x00;
	i2c_write(0x23, 0x6, 1, &val8, 1);
	i2c_write(0x23, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x23, 0x2, 1, &val8, 1);
	i2c_write(0x23, 0x3, 1, &val8, 1);

	val8 = 0x00;
	i2c_write(0x26, 0x6, 1, &val8, 1);
	val8 = 0x34;
	i2c_write(0x26, 0x7, 1, &val8, 1);
	val8 = 0xf9;
	i2c_write(0x26, 0x2, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x26, 0x3, 1, &val8, 1);

	val8 = 0x00;
	i2c_write(0x27, 0x6, 1, &val8, 1);
	i2c_write(0x27, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x27, 0x2, 1, &val8, 1);
	val8 = 0xef;
	i2c_write(0x27, 0x3, 1, &val8, 1);

	asm("eieio");
}

#ifdef CONFIG_PCI
void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	uint host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

#ifdef CONFIG_PCI1
{
	pib_init();

	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;

	uint pci_32 = 1;      /* PORDEVSR[15] */
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;       /* PORDEVSR[14] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;   /* PORPLLSR[16] */

	uint pci_agent = (host_agent == 3) || (host_agent == 4 ) || (host_agent == 6);

	uint pci_speed = 66666000;

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

		hose->first_busno = first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno = hose->last_busno+1;
		printf ("PCI on bus %02x - %02x\n",hose->first_busno,hose->last_busno);
	} else {
	printf ("    PCI: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif

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
		printf ("PCIE on bus %02x - %02x\n",hose->first_busno,hose->last_busno);

		first_free_busno=hose->last_busno+1;

	} else {
		printf ("    PCIE: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE; /* disable */
#endif
}
#endif /* CONFIG_PCI */

#if defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	int node, tmp[2];
	const char *path;

	ft_cpu_setup(blob, bd);

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
			tmp[1] = pcie1_hose.last_busno - pcie1_hose.first_busno;
			do_fixup_by_path(blob, path, "bus-range", &tmp, 8, 1);
		}
#endif
	}
}
#endif
