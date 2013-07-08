/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2007
 * Copyright (C) Sheldon Instruments, Inc. 2008
 *
 * Author: Ron Madrid <info@sheldoninst.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <pci.h>
#include <mpc83xx.h>
#include <ns16550.h>
#include <nand.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_NAND_SPL
int checkboard(void)
{
	puts("Board: Sheldon Instruments SIMPC8313\n");
	return 0;
}

static struct pci_region pci_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI1_MEM_BASE,
		phys_start: CONFIG_SYS_PCI1_MEM_PHYS,
		size: CONFIG_SYS_PCI1_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI1_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI1_MMIO_PHYS,
		size: CONFIG_SYS_PCI1_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CONFIG_SYS_PCI1_IO_BASE,
		phys_start: CONFIG_SYS_PCI1_IO_PHYS,
		size: CONFIG_SYS_PCI1_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci_regions };

	/* Enable all 3 PCI_CLK_OUTPUTs. */
	clk->occr |= 0xe0000000;

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg);
}

/*
 * Miscellaneous late-boot configurations
 */
int misc_init_r(void)
{
	int rc = 0;
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	fsl_lbc_t *lbus = &immap->im_lbc;
	u32 *mxmr = &lbus->mamr;	/* Pointer to mamr */

	/* UPM Table Configuration Code */
	static uint UPMATable[] = {
		/* Read Single-Beat (RSS) */
		0x0fff0c00, 0x0fffdc00, 0x0fff0c05, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		/* Read Burst (RBS) */
		0x0fff0c00, 0x0ffcdc00, 0x0ffc0c00, 0x0ffc0f0c,
		0x0ffccf0c, 0x0ffc0f0c, 0x0ffcce0c, 0x3ffc0c05,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		/* Write Single-Beat (WSS) */
		0x0ffc0c00, 0x0ffcdc00, 0x0ffc0c05, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		/* Write Burst (WBS) */
		0x0ffc0c00, 0x0fffcc0c, 0x0fff0c00, 0x0fffcc00,
		0x0fff1c00, 0x0fffcf0c, 0x0fff0f0c, 0x0fffcf0c,
		0x0fff0c0c, 0x0fffcc0c, 0x0fff0c05, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		/* Refresh Timer (RTS) */
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		/* Exception Condition (EXS) */
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01
	};

	upmconfig(UPMA, UPMATable, sizeof(UPMATable) / sizeof(UPMATable[0]));

	/* Set LUPWAIT to be active low and enabled */
	out_be32(mxmr, MxMR_UWPL | MxMR_GPL_x4DIS);

	return rc;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif
#else /* CONFIG_NAND_SPL */
void board_init_f(ulong bootflag)
{
	NS16550_init((NS16550_t)(CONFIG_SYS_IMMR + 0x4500),
				CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);
	puts("NAND boot... ");
	init_timebase();
	initdram(0);
	relocate_code(CONFIG_SYS_NAND_U_BOOT_RELOC_SP, (gd_t *)gd,
				  CONFIG_SYS_NAND_U_BOOT_RELOC);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	nand_boot();
}

void putc(char c)
{
	if (gd->flags & GD_FLG_SILENT)
		return;

	if (c == '\n')
		NS16550_putc((NS16550_t)(CONFIG_SYS_IMMR + 0x4500), '\r');

	NS16550_putc((NS16550_t)(CONFIG_SYS_IMMR + 0x4500), c);
}
#endif
