/*
 * Copyright (C) 2009-2010 DENX Software Engineering <wd@denx.de>
 * Copyright (C) Freescale Semiconductor, Inc. 2006, 2007.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/global_data.h>
#include <pci.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CONFIG_SYS_SDRAM_BASE

static struct pci_controller pci_hose;


/**************************************************************************
 * pci_init_board()
 *
 */
void
pci_init_board(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile law512x_t *pci_law;
	volatile pot512x_t *pci_pot;
	volatile pcictrl512x_t *pci_ctrl;
	u16 reg16;
	u32 reg32;
	u32 dev;
	int i;
	struct pci_controller *hose;

	/* Set PCI divider for 33MHz */
	reg32 = in_be32(&im->clk.scfr[0]);
	reg32 &= ~(SCFR1_PCI_DIV_MASK);
	reg32 |= SCFR1_PCI_DIV << SCFR1_PCI_DIV_SHIFT;
	out_be32(&im->clk.scfr[0], reg32);

	clrsetbits_be32(&im->clk.scfr[0],
			SCFR1_PCI_DIV_MASK,
			SCFR1_PCI_DIV << SCFR1_PCI_DIV_SHIFT
	);

	pci_law = im->sysconf.pcilaw;
	pci_pot = im->ios.pot;
	pci_ctrl = &im->pci_ctrl;

	hose = &pci_hose;

	/*
	 * Release PCI RST Output signal
	 */
	out_be32(&pci_ctrl->gcr, 0);
	udelay(2000);
	out_be32(&pci_ctrl->gcr, 1);

	/* We need to wait at least a 1sec based on PCI specs */
	for (i = 0; i < 1000; i++)
		udelay(1000);

	/*
	 * Configure PCI Local Access Windows
	 */
	out_be32(&pci_law[0].bar, CONFIG_SYS_PCI_MEM_PHYS & LAWBAR_BAR);
	out_be32(&pci_law[0].ar, LAWAR_EN | LAWAR_SIZE_512M);

	out_be32(&pci_law[1].bar, CONFIG_SYS_PCI_IO_PHYS & LAWBAR_BAR);
	out_be32(&pci_law[1].ar, LAWAR_EN | LAWAR_SIZE_16M);

	/*
	 * Configure PCI Outbound Translation Windows
	 */

	/* PCI mem space - prefetch */
	out_be32(&pci_pot[0].potar,
		(CONFIG_SYS_PCI_MEM_BASE >> 12) & POTAR_TA_MASK);
	out_be32(&pci_pot[0].pobar,
		(CONFIG_SYS_PCI_MEM_PHYS >> 12) & POBAR_BA_MASK);
	out_be32(&pci_pot[0].pocmr,
		POCMR_EN | POCMR_PRE | POCMR_CM_256M);

	/* PCI IO space */
	out_be32(&pci_pot[1].potar,
		(CONFIG_SYS_PCI_IO_BASE >> 12) & POTAR_TA_MASK);
	out_be32(&pci_pot[1].pobar,
		(CONFIG_SYS_PCI_IO_PHYS >> 12) & POBAR_BA_MASK);
	out_be32(&pci_pot[1].pocmr,
		POCMR_EN | POCMR_IO | POCMR_CM_16M);

	/* PCI mmio - non-prefetch mem space */
	out_be32(&pci_pot[2].potar,
		(CONFIG_SYS_PCI_MMIO_BASE >> 12) & POTAR_TA_MASK);
	out_be32(&pci_pot[2].pobar,
		(CONFIG_SYS_PCI_MMIO_PHYS >> 12) & POBAR_BA_MASK);
	out_be32(&pci_pot[2].pocmr,
		POCMR_EN | POCMR_CM_256M);

	/*
	 * Configure PCI Inbound Translation Windows
	 */

	/* we need RAM mapped to PCI space for the devices to
	 * access main memory */
	out_be32(&pci_ctrl[0].pitar1, 0x0);
	out_be32(&pci_ctrl[0].pibar1, 0x0);
	out_be32(&pci_ctrl[0].piebar1, 0x0);
	out_be32(&pci_ctrl[0].piwar1,
		PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP |
		PIWAR_WTT_SNOOP | (__ilog2(gd->ram_size) - 1));

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* PCI memory prefetch space */
	pci_set_region(hose->regions + 0,
		       CONFIG_SYS_PCI_MEM_BASE,
		       CONFIG_SYS_PCI_MEM_PHYS,
		       CONFIG_SYS_PCI_MEM_SIZE,
		       PCI_REGION_MEM|PCI_REGION_PREFETCH);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       CONFIG_SYS_PCI_MMIO_BASE,
		       CONFIG_SYS_PCI_MMIO_PHYS,
		       CONFIG_SYS_PCI_MMIO_SIZE,
		       PCI_REGION_MEM);

	/* PCI IO space */
	pci_set_region(hose->regions + 2,
		       CONFIG_SYS_PCI_IO_BASE,
		       CONFIG_SYS_PCI_IO_PHYS,
		       CONFIG_SYS_PCI_IO_SIZE,
		       PCI_REGION_IO);

	/* System memory space */
	pci_set_region(hose->regions + 3,
		       CONFIG_PCI_SYS_MEM_BUS,
		       CONFIG_PCI_SYS_MEM_PHYS,
		       gd->ram_size,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	hose->region_count = 4;

	pci_setup_indirect(hose,
			   (CONFIG_SYS_IMMR + 0x8300),
			   (CONFIG_SYS_IMMR + 0x8304));

	pci_register_hose(hose);

	/*
	 * Write to Command register
	 */
	reg16 = 0xff;
	dev = PCI_BDF(hose->first_busno, 0, 0);
	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(hose, dev, PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
	/*
	 * Hose scan.
	 */
	hose->last_busno = pci_hose_scan(hose);
}

#if defined(CONFIG_OF_LIBFDT)
void ft_pci_setup(void *blob, bd_t *bd)
{
	int nodeoffset;
	int tmp[2];
	const char *path;

	nodeoffset = fdt_path_offset(blob, "/aliases");
	if (nodeoffset >= 0) {
		path = fdt_getprop(blob, nodeoffset, "pci", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(pci_hose.first_busno);
			tmp[1] = cpu_to_be32(pci_hose.last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);

			tmp[0] = cpu_to_be32(gd->pci_clk);
			do_fixup_by_path(blob, path, "clock-frequency",
				&tmp, sizeof(tmp[0]), 1);
		}
	}
}
#endif /* CONFIG_OF_LIBFDT */
