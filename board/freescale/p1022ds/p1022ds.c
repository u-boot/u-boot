/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
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
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <tsec.h>
#include <asm/fsl_law.h>
#include <asm/mp.h>
#include <netdev.h>
#include <i2c.h>

#include "../common/ngpixis.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Set pmuxcr to allow both i2c1 and i2c2 */
	setbits_be32(&gur->pmuxcr, 0x1000);

	/* Read back the register to synchronize the write. */
	in_be32(&gur->pmuxcr);

	/* Set the pin muxing to enable ETSEC2. */
	clrbits_be32(&gur->pmuxcr2, 0x001F8000);

	return 0;
}

int checkboard(void)
{
	u8 sw;

	puts("Board: P1022DS ");

	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(&pixis->id), in_8(&pixis->arch), in_8(&pixis->scver));

	sw = in_8(&PIXIS_SW(PIXIS_LBMAP_SWITCH));

	switch ((sw & PIXIS_LBMAP_MASK) >> 6) {
	case 0:
		printf ("vBank: %u\n", ((sw & 0x30) >> 4));
		break;
	case 1:
		printf ("NAND\n");
		break;
	case 2:
	case 3:
		puts ("Promjet\n");
		break;
	}

	return 0;
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = 0;

	puts("Initializing....\n");

	dram_size = fsl_ddr_sdram();
	dram_size = setup_ddr_tlbs(dram_size / 0x100000) * 0x100000;

	puts("    DDR: ");
	return dram_size;
}

#define CONFIG_TFP410_I2C_ADDR	0x38

int misc_init_r(void)
{
	u8 temp;

	/*  Enable the TFP410 Encoder */

	temp = 0xBF;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;

	/* Verify if enabled */
	temp = 0;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;

	debug("DVI Encoder Read: 0x%02x\n", temp);

	temp = 0x10;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;

	/* Verify if enabled */
	temp = 0;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;

	debug("DVI Encoder Read: 0x%02x\n",temp);

	return 0;
}

/*
 * A list of PCI and SATA slots
 */
enum slot_id {
	SLOT_PCIE1 = 1,
	SLOT_PCIE2,
	SLOT_PCIE3,
	SLOT_PCIE4,
	SLOT_PCIE5,
	SLOT_SATA1,
	SLOT_SATA2
};

/*
 * This array maps the slot identifiers to their names on the P1022DS board.
 */
static const char *slot_names[] = {
	[SLOT_PCIE1] = "Slot 1",
	[SLOT_PCIE2] = "Slot 2",
	[SLOT_PCIE3] = "Slot 3",
	[SLOT_PCIE4] = "Slot 4",
	[SLOT_PCIE5] = "Mini-PCIe",
	[SLOT_SATA1] = "SATA 1",
	[SLOT_SATA2] = "SATA 2",
};

/*
 * This array maps a given SERDES configuration and SERDES device to the PCI or
 * SATA slot that it connects to.  This mapping is hard-coded in the FPGA.
 */
static u8 serdes_dev_slot[][SATA2 + 1] = {
	[0x01] = { [PCIE3] = SLOT_PCIE4, [PCIE2] = SLOT_PCIE5 },
	[0x02] = { [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x09] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE4,
		   [PCIE2] = SLOT_PCIE5 },
	[0x16] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x17] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3 },
	[0x1a] = { [PCIE1] = SLOT_PCIE1, [PCIE2] = SLOT_PCIE3,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1c] = { [PCIE1] = SLOT_PCIE1,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1e] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE3 },
	[0x1f] = { [PCIE1] = SLOT_PCIE1 },
};


/*
 * Returns the name of the slot to which the PCIe or SATA controller is
 * connected
 */
const char *serdes_slot_name(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	unsigned int srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	enum slot_id slot = serdes_dev_slot[srds_cfg][device];
	const char *name = slot_names[slot];

	if (name)
		return name;
	else
		return "Nothing";
}

static void configure_pcie(struct fsl_pci_info *info,
			   struct pci_controller *hose,
			   const char *connected)
{
	static int bus_number = 0;
	int is_endpoint;

	set_next_law(info->mem_phys, law_size_bits(info->mem_size), info->law);
	set_next_law(info->io_phys, law_size_bits(info->io_size), info->law);
	is_endpoint = fsl_setup_hose(hose, info->regs);
	printf("    PCIE%u connected to %s as %s (base addr %lx)\n",
	       info->pci_num, connected,
	       is_endpoint ? "Endpoint" : "Root Complex", info->regs);
	bus_number = fsl_pci_init_port(info, hose, bus_number);
}

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
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	struct fsl_pci_info pci_info;
	u32 devdisr = in_be32(&gur->devdisr);

#ifdef CONFIG_PCIE1
	if (is_serdes_configured(PCIE1) && !(devdisr & MPC85xx_DEVDISR_PCIE)) {
		SET_STD_PCIE_INFO(pci_info, 1);
		configure_pcie(&pci_info, &pcie1_hose, serdes_slot_name(PCIE1));
	} else {
		printf("    PCIE1: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE); /* disable */
#endif

#ifdef CONFIG_PCIE2
	if (is_serdes_configured(PCIE2) && !(devdisr & MPC85xx_DEVDISR_PCIE2)) {
		SET_STD_PCIE_INFO(pci_info, 2);
		configure_pcie(&pci_info, &pcie2_hose, serdes_slot_name(PCIE2));
	} else {
		printf("    PCIE2: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE2); /* disable */
#endif

#ifdef CONFIG_PCIE3
	if (is_serdes_configured(PCIE3) && !(devdisr & MPC85xx_DEVDISR_PCIE3)) {
		SET_STD_PCIE_INFO(pci_info, 3);
		configure_pcie(&pci_info, &pcie3_hose, serdes_slot_name(PCIE3));
	} else {
		printf("    PCIE3: disabled\n");
	}
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE3); /* disable */
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

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	return 0;
}

/*
 * Initialize on-board and/or PCI Ethernet devices
 *
 * Returns:
 *      <0, error
 *       0, no ethernet devices found
 *      >0, number of ethernet devices initialized
 */
int board_eth_init(bd_t *bis)
{
	struct tsec_info_struct tsec_info[2];
	unsigned int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif

	return tsec_eth_init(bis, tsec_info, num) + pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif
}
#endif

#ifdef CONFIG_MP
void board_lmb_reserve(struct lmb *lmb)
{
	cpu_mp_lmb_reserve(lmb);
}
#endif
