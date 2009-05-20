/*
 * Copyright 2009 Freescale Semiconductor.
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
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <i2c.h>
#include <ioports.h>
#include <libfdt.h>
#include <fdt_support.h>

#include "bcsr.h"

phys_size_t fixed_sdram(void);

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* QE_MUX_MDC */
	{2,  31, 1, 0, 1}, /* QE_MUX_MDC               */

	/* QE_MUX_MDIO */
	{2,  30, 3, 0, 2}, /* QE_MUX_MDIO              */

#if defined(CONFIG_SYS_UCC_RGMII_MODE)
	/* UCC_1_RGMII */
	{2, 11, 2, 0, 1}, /* CLK12 */
	{0,  0, 1, 0, 3}, /* ENET1_TXD0_SER1_TXD0      */
	{0,  1, 1, 0, 3}, /* ENET1_TXD1_SER1_TXD1      */
	{0,  2, 1, 0, 1}, /* ENET1_TXD2_SER1_TXD2      */
	{0,  3, 1, 0, 2}, /* ENET1_TXD3_SER1_TXD3      */
	{0,  6, 2, 0, 3}, /* ENET1_RXD0_SER1_RXD0      */
	{0,  7, 2, 0, 1}, /* ENET1_RXD1_SER1_RXD1      */
	{0,  8, 2, 0, 2}, /* ENET1_RXD2_SER1_RXD2      */
	{0,  9, 2, 0, 2}, /* ENET1_RXD3_SER1_RXD3      */
	{0,  4, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B    */
	{0, 12, 2, 0, 3}, /* ENET1_RX_DV_SER1_CTS_B    */
	{2,  8, 2, 0, 1}, /* ENET1_GRXCLK              */
	{2, 20, 1, 0, 2}, /* ENET1_GTXCLK              */

	/* UCC_2_RGMII */
	{2, 16, 2, 0, 3}, /* CLK17 */
	{0, 14, 1, 0, 2}, /* ENET2_TXD0_SER2_TXD0      */
	{0, 15, 1, 0, 2}, /* ENET2_TXD1_SER2_TXD1      */
	{0, 16, 1, 0, 1}, /* ENET2_TXD2_SER2_TXD2      */
	{0, 17, 1, 0, 1}, /* ENET2_TXD3_SER2_TXD3      */
	{0, 20, 2, 0, 2}, /* ENET2_RXD0_SER2_RXD0      */
	{0, 21, 2, 0, 1}, /* ENET2_RXD1_SER2_RXD1      */
	{0, 22, 2, 0, 1}, /* ENET2_RXD2_SER2_RXD2      */
	{0, 23, 2, 0, 1}, /* ENET2_RXD3_SER2_RXD3      */
	{0, 18, 1, 0, 2}, /* ENET2_TX_EN_SER2_RTS_B    */
	{0, 26, 2, 0, 3}, /* ENET2_RX_DV_SER2_CTS_B    */
	{2,  3, 2, 0, 1}, /* ENET2_GRXCLK              */
	{2,  2, 1, 0, 2}, /* ENET2_GTXCLK              */

	/* UCC_3_RGMII */
	{2, 11, 2, 0, 1}, /* CLK12 */
	{0, 29, 1, 0, 2}, /* ENET3_TXD0_SER3_TXD0      */
	{0, 30, 1, 0, 3}, /* ENET3_TXD1_SER3_TXD1      */
	{0, 31, 1, 0, 2}, /* ENET3_TXD2_SER3_TXD2      */
	{1,  0, 1, 0, 3}, /* ENET3_TXD3_SER3_TXD3      */
	{1,  3, 2, 0, 3}, /* ENET3_RXD0_SER3_RXD0      */
	{1,  4, 2, 0, 1}, /* ENET3_RXD1_SER3_RXD1      */
	{1,  5, 2, 0, 2}, /* ENET3_RXD2_SER3_RXD2      */
	{1,  6, 2, 0, 3}, /* ENET3_RXD3_SER3_RXD3      */
	{1,  1, 1, 0, 1}, /* ENET3_TX_EN_SER3_RTS_B    */
	{1,  9, 2, 0, 3}, /* ENET3_RX_DV_SER3_CTS_B    */
	{2,  9, 2, 0, 2}, /* ENET3_GRXCLK              */
	{2, 25, 1, 0, 2}, /* ENET3_GTXCLK              */

	/* UCC_4_RGMII */
	{2, 16, 2, 0, 3}, /* CLK17 */
	{1, 12, 1, 0, 2}, /* ENET4_TXD0_SER4_TXD0      */
	{1, 13, 1, 0, 2}, /* ENET4_TXD1_SER4_TXD1      */
	{1, 14, 1, 0, 1}, /* ENET4_TXD2_SER4_TXD2      */
	{1, 15, 1, 0, 2}, /* ENET4_TXD3_SER4_TXD3      */
	{1, 18, 2, 0, 2}, /* ENET4_RXD0_SER4_RXD0      */
	{1, 19, 2, 0, 1}, /* ENET4_RXD1_SER4_RXD1      */
	{1, 20, 2, 0, 1}, /* ENET4_RXD2_SER4_RXD2      */
	{1, 21, 2, 0, 2}, /* ENET4_RXD3_SER4_RXD3      */
	{1, 16, 1, 0, 2}, /* ENET4_TX_EN_SER4_RTS_B    */
	{1, 24, 2, 0, 3}, /* ENET4_RX_DV_SER4_CTS_B    */
	{2, 17, 2, 0, 2}, /* ENET4_GRXCLK              */
	{2, 24, 1, 0, 2}, /* ENET4_GTXCLK              */

#elif defined(CONFIG_SYS_UCC_RMII_MODE)
	/* UCC_1_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0,  0, 1, 0, 3}, /* ENET1_TXD0_SER1_TXD0      */
	{0,  1, 1, 0, 3}, /* ENET1_TXD1_SER1_TXD1      */
	{0,  6, 2, 0, 3}, /* ENET1_RXD0_SER1_RXD0      */
	{0,  7, 2, 0, 1}, /* ENET1_RXD1_SER1_RXD1      */
	{0,  4, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B    */
	{0, 12, 2, 0, 3}, /* ENET1_RX_DV_SER1_CTS_B    */

	/* UCC_2_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0, 14, 1, 0, 2}, /* ENET2_TXD0_SER2_TXD0      */
	{0, 15, 1, 0, 2}, /* ENET2_TXD1_SER2_TXD1      */
	{0, 20, 2, 0, 2}, /* ENET2_RXD0_SER2_RXD0      */
	{0, 21, 2, 0, 1}, /* ENET2_RXD1_SER2_RXD1      */
	{0, 18, 1, 0, 2}, /* ENET2_TX_EN_SER2_RTS_B    */
	{0, 26, 2, 0, 3}, /* ENET2_RX_DV_SER2_CTS_B    */

	/* UCC_3_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0, 29, 1, 0, 2}, /* ENET3_TXD0_SER3_TXD0      */
	{0, 30, 1, 0, 3}, /* ENET3_TXD1_SER3_TXD1      */
	{1,  3, 2, 0, 3}, /* ENET3_RXD0_SER3_RXD0      */
	{1,  4, 2, 0, 1}, /* ENET3_RXD1_SER3_RXD1      */
	{1,  1, 1, 0, 1}, /* ENET3_TX_EN_SER3_RTS_B    */
	{1,  9, 2, 0, 3}, /* ENET3_RX_DV_SER3_CTS_B    */

	/* UCC_4_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{1, 12, 1, 0, 2}, /* ENET4_TXD0_SER4_TXD0      */
	{1, 13, 1, 0, 2}, /* ENET4_TXD1_SER4_TXD1      */
	{1, 18, 2, 0, 2}, /* ENET4_RXD0_SER4_RXD0      */
	{1, 19, 2, 0, 1}, /* ENET4_RXD1_SER4_RXD1      */
	{1, 16, 1, 0, 2}, /* ENET4_TX_EN_SER4_RTS_B    */
	{1, 24, 2, 0, 3}, /* ENET4_RX_DV_SER4_CTS_B    */
#endif

	/* UART1 is muxed with QE PortF bit [9-12].*/
	{5, 12, 2, 0, 3}, /* UART1_SIN */
	{5, 9,  1, 0, 3}, /* UART1_SOUT */
	{5, 10, 2, 0, 3}, /* UART1_CTS_B */
	{5, 11, 1, 0, 2}, /* UART1_RTS_B */

	{0,  0, 0, 0, QE_IOP_TAB_END} /* END of table */
};

void local_bus_init(void);

int board_early_init_f (void)
{
	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	enable_8569mds_flash_write();

#ifdef CONFIG_QE
	enable_8569mds_qe_uec();
#endif

#if CONFIG_SYS_I2C2_OFFSET
	/* Enable I2C2 signals instead of SD signals */
	volatile struct ccsr_gur *gur;
	gur = (struct ccsr_gur *)(CONFIG_SYS_IMMR + 0xe0000);
	gur->plppar1 &= ~PLPPAR1_I2C_BIT_MASK;
	gur->plppar1 |= PLPPAR1_I2C2_VAL;
	gur->plpdir1 &= ~PLPDIR1_I2C_BIT_MASK;
	gur->plpdir1 |= PLPDIR1_I2C2_VAL;

	disable_8569mds_brd_eeprom_write_protect();
#endif

	return 0;
}

int checkboard (void)
{
	printf ("Board: 8569 MDS\n");

	return 0;
}

phys_size_t
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

#if defined(CONFIG_DDR_DLL)
	/*
	 * Work around to stabilize DDR DLL MSYNC_IN.
	 * Errata DDR9 seems to have been fixed.
	 * This is now the workaround for Errata DDR11:
	 *    Override DLL = 1, Course Adj = 1, Tap Select = 0
	 */
	volatile ccsr_gur_t *gur =
			(void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	out_be32(&gur->ddrdllcr, 0x81000000);
	udelay(200);
#endif

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
phys_size_t fixed_sdram(void)
{
	volatile ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC85xx_DDR_ADDR;
	uint d_init;

	out_be32(&ddr->cs0_bnds, CONFIG_SYS_DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, CONFIG_SYS_DDR_CS0_CONFIG);
	out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);
	out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_SDRAM_CFG);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_SDRAM_CFG_2);
	out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_SDRAM_MODE_2);
	out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_SDRAM_INTERVAL);
	out_be32(&ddr->sdram_data_init, CONFIG_SYS_DDR_DATA_INIT);
	out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->timing_cfg_4, CONFIG_SYS_DDR_TIMING_4);
	out_be32(&ddr->timing_cfg_5, CONFIG_SYS_DDR_TIMING_5);
	out_be32(&ddr->ddr_zq_cntl, CONFIG_SYS_DDR_ZQ_CNTL);
	out_be32(&ddr->ddr_wrlvl_cntl, CONFIG_SYS_DDR_WRLVL_CNTL);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_SDRAM_CFG_2);
#if defined (CONFIG_DDR_ECC)
	out_be32(&ddr->err_int_en, CONFIG_SYS_DDR_ERR_INT_EN);
	out_be32(&ddr->err_disable, CONFIG_SYS_DDR_ERR_DIS);
	out_be32(&ddr->err_sbe, CONFIG_SYS_DDR_SBE);
#endif
	udelay(500);

	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
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
	udelay(500);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif

/*
 * Initialize Local Bus
 */
void
local_bus_init(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile ccsr_lbc_t *lbc = (void *)(CONFIG_SYS_MPC85xx_LBC_ADDR);

	uint clkdiv;
	uint lbc_hz;
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);
	clkdiv = (lbc->lcrr & LCRR_CLKDIV) * 2;
	lbc_hz = sysinfo.freqSystemBus / 1000000 / clkdiv;

	out_be32(&gur->lbiuiplldcr1, 0x00078080);
	if (clkdiv == 16)
		out_be32(&gur->lbiuiplldcr0, 0x7c0f1bf0);
	else if (clkdiv == 8)
		out_be32(&gur->lbiuiplldcr0, 0x6c0f1bf0);
	else if (clkdiv == 4)
		out_be32(&gur->lbiuiplldcr0, 0x5c0f1bf0);

	out_be32(&lbc->lcrr, (u32)in_be32(&lbc->lcrr)| 0x00030000);
}

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif  /* CONFIG_PCIE1 */

int first_free_busno = 0;

#ifdef CONFIG_PCI
void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur;
	uint io_sel;
	uint host_agent;

	gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

#ifdef CONFIG_PCIE1
{
	volatile ccsr_fsl_pci_t *pci;
	struct pci_controller *hose;
	int pcie_ep;
	struct pci_region *r;
	int pcie_configured;

	pci = (ccsr_fsl_pci_t *) CONFIG_SYS_PCIE1_ADDR;
	hose = &pcie1_hose;
	pcie_ep =  (host_agent == 0) || (host_agent == 2 ) || (host_agent == 3);
	r = hose->regions;
	pcie_configured  = io_sel >= 1;

	if (pcie_configured && !(gur->devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE connected to slot as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);

		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing. Now 0x%08x",
				pci->pme_msg_det);
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

		hose->region_count = r - hose->regions;

		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr,
					(int) &pci->cfg_data);

		fsl_pci_init(hose);
		printf ("PCIE on bus %02x - %02x\n",
				hose->first_busno,hose->last_busno);

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
void ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_SYS_UCC_RMII_MODE)
	int nodeoff, off, err;
	unsigned int val;
	const u32 *ph;
	const u32 *index;

	/* fixup device tree for supporting rmii mode */
	nodeoff = -1;
	while ((nodeoff = fdt_node_offset_by_compatible(blob, nodeoff,
				"ucc_geth")) >= 0) {
		err = fdt_setprop_string(blob, nodeoff, "tx-clock-name",
						"clk16");
		if (err < 0) {
			printf("WARNING: could not set tx-clock-name %s.\n",
				fdt_strerror(err));
			break;
		}

		err = fdt_setprop_string(blob, nodeoff, "phy-connection-type",
					"rmii");
		if (err < 0) {
			printf("WARNING: could not set phy-connection-type "
				"%s.\n", fdt_strerror(err));
			break;
		}

		index = fdt_getprop(blob, nodeoff, "cell-index", 0);
		if (index == NULL) {
			printf("WARNING: could not get cell-index of ucc\n");
			break;
		}

		ph = fdt_getprop(blob, nodeoff, "phy-handle", 0);
		if (ph == NULL) {
			printf("WARNING: could not get phy-handle of ucc\n");
			break;
		}

		off = fdt_node_offset_by_phandle(blob, *ph);
		if (off < 0) {
			printf("WARNING: could not get phy node	%s.\n",
				fdt_strerror(err));
			break;
		}

		val = 0x7 + *index; /* RMII phy address starts from 0x8 */

		err = fdt_setprop(blob, off, "reg", &val, sizeof(u32));
		if (err < 0) {
			printf("WARNING: could not set reg for phy-handle "
				"%s.\n", fdt_strerror(err));
			break;
		}
	}
#endif
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCIE1
	ft_fsl_pci_setup(blob, "pci1", &pcie1_hose);
#endif
}
#endif
