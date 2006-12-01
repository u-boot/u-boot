/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

/*
 * PCI Configuration space access support for MPC83xx PCI Bridge
 */
#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <pci.h>
#include <i2c.h>

#include <asm/fsl_i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PCI)
#define PCI_FUNCTION_CONFIG   0x44
#define PCI_FUNCTION_CFG_LOCK 0x20

/*
 * Initialize PCI Devices, report devices found
 */
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc83xxemds_config_table[] = {
	{
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 pci_cfgfunc_config_device,
	 {PCI_ENET0_IOADDR,
	  PCI_ENET0_MEMADDR,
	  PCI_COMMON_MEMORY | PCI_COMMAND_MASTER}
	 },
	{}
}
#endif
static struct pci_controller hose[] = {
	{
#ifndef CONFIG_PCI_PNP
	      config_table:pci_mpc83xxemds_config_table,
#endif
	 },
};

/**********************************************************************
 * pci_init_board()
 *********************************************************************/
void pci_init_board(void)
#ifdef CONFIG_PCISLAVE
{
	u16 reg16;
	volatile immap_t *immr;
	volatile law83xx_t *pci_law;
	volatile pot83xx_t *pci_pot;
	volatile pcictrl83xx_t *pci_ctrl;
	volatile pciconf83xx_t *pci_conf;

	immr = (immap_t *) CFG_IMMR;
	pci_law = immr->sysconf.pcilaw;
	pci_pot = immr->ios.pot;
	pci_ctrl = immr->pci_ctrl;
	pci_conf = immr->pci_conf;
	/*
	 * Configure PCI Inbound Translation Windows
	 */
	pci_ctrl[0].pitar0 = 0x0;
	pci_ctrl[0].pibar0 = 0x0;
	pci_ctrl[0].piwar0 = PIWAR_EN | PIWAR_RTT_SNOOP |
	    PIWAR_WTT_SNOOP | PIWAR_IWS_4K;

	pci_ctrl[0].pitar1 = 0x0;
	pci_ctrl[0].pibar1 = 0x0;
	pci_ctrl[0].piebar1 = 0x0;
	pci_ctrl[0].piwar1 &= ~PIWAR_EN;

	pci_ctrl[0].pitar2 = 0x0;
	pci_ctrl[0].pibar2 = 0x0;
	pci_ctrl[0].piebar2 = 0x0;
	pci_ctrl[0].piwar2 &= ~PIWAR_EN;

	hose[0].first_busno = 0;
	hose[0].last_busno = 0xff;
	pci_setup_indirect(&hose[0],
			   (CFG_IMMR + 0x8300), (CFG_IMMR + 0x8304));
	reg16 = 0xff;

	pci_hose_read_config_word(&hose[0], PCI_BDF(0, 0, 0),
				  PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(&hose[0], PCI_BDF(0, 0, 0),
				   PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(&hose[0], PCI_BDF(0, 0, 0),
				   PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(&hose[0], PCI_BDF(0, 0, 0),
				   PCI_LATENCY_TIMER, 0x80);

	/*
	 * Unlock configuration lock in PCI function configuration register.
	 */
	pci_hose_read_config_word(&hose[0], PCI_BDF(0, 0, 0),
				  PCI_FUNCTION_CONFIG, &reg16);
	reg16 &= ~(PCI_FUNCTION_CFG_LOCK);
	pci_hose_write_config_word(&hose[0], PCI_BDF(0, 0, 0),
				   PCI_FUNCTION_CONFIG, reg16);

	printf("Enabled PCI 32bit Agent Mode\n");
}
#else
{
	volatile immap_t *immr;
	volatile clk83xx_t *clk;
	volatile law83xx_t *pci_law;
	volatile pot83xx_t *pci_pot;
	volatile pcictrl83xx_t *pci_ctrl;
	volatile pciconf83xx_t *pci_conf;

	u8 val8, orig_i2c_bus;
	u16 reg16;
	u32 val32;
	u32 dev;

	immr = (immap_t *) CFG_IMMR;
	clk = (clk83xx_t *) & immr->clk;
	pci_law = immr->sysconf.pcilaw;
	pci_pot = immr->ios.pot;
	pci_ctrl = immr->pci_ctrl;
	pci_conf = immr->pci_conf;
	/*
	 * Configure PCI controller and PCI_CLK_OUTPUT both in 66M mode
	 */
	val32 = clk->occr;
	udelay(2000);
#if defined(PCI_66M)
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2;
	printf("PCI clock is 66MHz\n");
#elif defined(PCI_33M)
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2 |
	    OCCR_PCICD0 | OCCR_PCICD1 | OCCR_PCICD2 | OCCR_PCICR;
	printf("PCI clock is 33MHz\n");
#else
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2;
	printf("PCI clock is 66MHz\n");
#endif
	udelay(2000);

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CFG_PCI_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_512M;

	pci_law[1].bar = CFG_PCI_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_1M;

	/*
	 * Configure PCI Outbound Translation Windows
	 */

	/* PCI mem space - prefetch */
	pci_pot[0].potar = (CFG_PCI_MEM_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[0].pobar = (CFG_PCI_MEM_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[0].pocmr =
	    POCMR_EN | POCMR_SE | (POCMR_CM_256M & POCMR_CM_MASK);

	/* PCI mmio - non-prefetch mem space */
	pci_pot[1].potar = (CFG_PCI_MMIO_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[1].pobar = (CFG_PCI_MMIO_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[1].pocmr = POCMR_EN | (POCMR_CM_256M & POCMR_CM_MASK);

	/* PCI IO space */
	pci_pot[2].potar = (CFG_PCI_IO_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[2].pobar = (CFG_PCI_IO_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[2].pocmr = POCMR_EN | POCMR_IO | (POCMR_CM_1M & POCMR_CM_MASK);

	/*
	 * Configure PCI Inbound Translation Windows
	 */
	pci_ctrl[0].pitar1 = (CFG_PCI_SLV_MEM_LOCAL >> 12) & PITAR_TA_MASK;
	pci_ctrl[0].pibar1 = (CFG_PCI_SLV_MEM_BUS >> 12) & PIBAR_MASK;
	pci_ctrl[0].piebar1 = 0x0;
	pci_ctrl[0].piwar1 =
	    PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP | PIWAR_WTT_SNOOP |
	    PIWAR_IWS_2G;

	/*
	 * Assign PIB PMC slot to desired PCI bus
	 */

	/* Switch temporarily to I2C bus #2 */
	orig_i2c_bus = i2c_get_bus_num();
 	i2c_set_bus_num(1);

	val8 = 0;
	i2c_write(0x23, 0x6, 1, &val8, 1);
	i2c_write(0x23, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x23, 0x2, 1, &val8, 1);
	i2c_write(0x23, 0x3, 1, &val8, 1);

	val8 = 0;
	i2c_write(0x26, 0x6, 1, &val8, 1);
	val8 = 0x34;
	i2c_write(0x26, 0x7, 1, &val8, 1);

	val8 = 0xf3;		/*PMC1, PMC2, PMC3 slot to PCI bus */
	i2c_write(0x26, 0x2, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x26, 0x3, 1, &val8, 1);

	val8 = 0;
	i2c_write(0x27, 0x6, 1, &val8, 1);
	i2c_write(0x27, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x27, 0x2, 1, &val8, 1);
	val8 = 0xef;
	i2c_write(0x27, 0x3, 1, &val8, 1);
	asm("eieio");

	/* Reset to original I2C bus */
	i2c_set_bus_num(orig_i2c_bus);

	/*
	 * Release PCI RST Output signal
	 */
	udelay(2000);
	pci_ctrl[0].gcr = 1;
	udelay(2000);

	hose[0].first_busno = 0;
	hose[0].last_busno = 0xff;

	/* PCI memory prefetch space */
	pci_set_region(hose[0].regions + 0,
		       CFG_PCI_MEM_BASE,
		       CFG_PCI_MEM_PHYS,
		       CFG_PCI_MEM_SIZE, PCI_REGION_MEM | PCI_REGION_PREFETCH);

	/* PCI memory space */
	pci_set_region(hose[0].regions + 1,
		       CFG_PCI_MMIO_BASE,
		       CFG_PCI_MMIO_PHYS, CFG_PCI_MMIO_SIZE, PCI_REGION_MEM);

	/* PCI IO space */
	pci_set_region(hose[0].regions + 2,
		       CFG_PCI_IO_BASE,
		       CFG_PCI_IO_PHYS, CFG_PCI_IO_SIZE, PCI_REGION_IO);

	/* System memory space */
	pci_set_region(hose[0].regions + 3,
		       CFG_PCI_SLV_MEM_LOCAL,
		       CFG_PCI_SLV_MEM_BUS,
		       CFG_PCI_SLV_MEM_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	hose[0].region_count = 4;

	pci_setup_indirect(&hose[0],
			   (CFG_IMMR + 0x8300), (CFG_IMMR + 0x8304));

	pci_register_hose(hose);

	/*
	 * Write command register
	 */
	reg16 = 0xff;
	dev = PCI_BDF(0, 0, 0);
	pci_hose_read_config_word(&hose[0], dev, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(&hose[0], dev, PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(&hose[0], dev, PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(&hose[0], dev, PCI_LATENCY_TIMER, 0x80);
	pci_hose_write_config_byte(&hose[0], dev, PCI_CACHE_LINE_SIZE, 0x08);

	printf("PCI 32bit bus on PMC1 & PMC2 & PMC3\n");

	/*
	 * Hose scan.
	 */
	hose->last_busno = pci_hose_scan(hose);
}
#endif				/* CONFIG_PCISLAVE */

#ifdef CONFIG_OF_FLAT_TREE
void
ft_pci_setup(void *blob, bd_t *bd)
{
       	u32 *p;
       	int len;

       	p = (u32 *)ft_get_prop(blob, "/" OF_SOC "/pci@8500/bus-range", &len);
       	if (p != NULL) {
		p[0] = hose[0].first_busno;
		p[1] = hose[0].last_busno;
       	}
}
#endif				/* CONFIG_OF_FLAT_TREE */
#endif				/* CONFIG_PCI */
