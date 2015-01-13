/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2005 MontaVista Software, Inc.
 * Vitaly Bordug <vbordug@ru.mvista.com>
 * Added support for PCI bridge on MPC8272ADS
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <pci.h>
#include <mpc8260.h>
#include <asm/m8260_pci.h>
#include <asm/io.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#include <fdt_support.h>
#endif

/*
 *   Local->PCI map (from CPU)				   controlled by
 *   MPC826x master window
 *
 *   0x80000000 - 0xBFFFFFFF	CPU2PCI space		   PCIBR0
 *   0xF4000000 - 0xF7FFFFFF	CPU2PCI space		   PCIBR1
 *
 *   0x80000000 - 0x9FFFFFFF	0x80000000 - 0x9FFFFFFF	  (Outbound ATU #1)
 *				PCI Mem with prefetch
 *
 *   0xA0000000 - 0xBFFFFFFF	0xA0000000 - 0xBFFFFFFF	  (Outbound ATU #2)
 *				PCI Mem w/o  prefetch
 *
 *   0xF4000000 - 0xF7FFFFFF	0x00000000 - 0x03FFFFFF	  (Outbound ATU #3)
 *				32-bit PCI IO
 *
 *   PCI->Local map (from PCI)
 *   MPC826x slave window				   controlled by
 *
 *   0x00000000 - 0x1FFFFFFF	0x00000000 - 0x1FFFFFFF	  (Inbound ATU #1)
 *				MPC826x local memory
 */

/*
 * Slave window that allows PCI masters to access MPC826x local memory.
 * This window is set up using the first set of Inbound ATU registers
 */

#ifndef CONFIG_SYS_PCI_SLV_MEM_LOCAL
#define PCI_SLV_MEM_LOCAL CONFIG_SYS_SDRAM_BASE	/* Local base */
#else
#define PCI_SLV_MEM_LOCAL CONFIG_SYS_PCI_SLV_MEM_LOCAL
#endif

#ifndef CONFIG_SYS_PCI_SLV_MEM_BUS
#define PCI_SLV_MEM_BUS 0x00000000	/* PCI base */
#else
#define PCI_SLV_MEM_BUS CONFIG_SYS_PCI_SLV_MEM_BUS
#endif

#ifndef CONFIG_SYS_PICMR0_MASK_ATTRIB
#define PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
				 PICMR_PREFETCH_EN)
#else
#define PICMR0_MASK_ATTRIB CONFIG_SYS_PICMR0_MASK_ATTRIB
#endif

/*
 * These are the windows that allow the CPU to access PCI address space.
 * All three PCI master windows, which allow the CPU to access PCI
 * prefetch, non prefetch, and IO space (see below), must all fit within
 * these windows.
 */

/* PCIBR0 */
#ifndef CONFIG_SYS_PCI_MSTR0_LOCAL
#define PCI_MSTR0_LOCAL		0x80000000	/* Local base */
#else
#define PCI_MSTR0_LOCAL CONFIG_SYS_PCI_MSTR0_LOCAL
#endif

#ifndef CONFIG_SYS_PCIMSK0_MASK
#define PCIMSK0_MASK		PCIMSK_1GB	/* Size of window */
#else
#define PCIMSK0_MASK	CONFIG_SYS_PCIMSK0_MASK
#endif

/* PCIBR1 */
#ifndef CONFIG_SYS_PCI_MSTR1_LOCAL
#define PCI_MSTR1_LOCAL		0xF4000000	/* Local base */
#else
#define PCI_MSTR1_LOCAL		CONFIG_SYS_PCI_MSTR1_LOCAL
#endif

#ifndef CONFIG_SYS_PCIMSK1_MASK
#define	 PCIMSK1_MASK		PCIMSK_64MB	/* Size of window */
#else
#define	 PCIMSK1_MASK		CONFIG_SYS_PCIMSK1_MASK
#endif

/*
 * Master window that allows the CPU to access PCI Memory (prefetch).
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#ifndef CONFIG_SYS_PCI_MSTR_MEM_LOCAL
#define PCI_MSTR_MEM_LOCAL 0x80000000	/* Local base */
#else
#define PCI_MSTR_MEM_LOCAL CONFIG_SYS_PCI_MSTR_MEM_LOCAL
#endif

#ifndef CONFIG_SYS_PCI_MSTR_MEM_BUS
#define PCI_MSTR_MEM_BUS 0x80000000	/* PCI base   */
#else
#define PCI_MSTR_MEM_BUS CONFIG_SYS_PCI_MSTR_MEM_BUS
#endif

#ifndef CONFIG_SYS_CPU_PCI_MEM_START
#define CPU_PCI_MEM_START PCI_MSTR_MEM_LOCAL
#else
#define CPU_PCI_MEM_START CONFIG_SYS_CPU_PCI_MEM_START
#endif

#ifndef CONFIG_SYS_PCI_MSTR_MEM_SIZE
#define PCI_MSTR_MEM_SIZE 0x10000000	/* 256MB */
#else
#define PCI_MSTR_MEM_SIZE CONFIG_SYS_PCI_MSTR_MEM_SIZE
#endif

#ifndef CONFIG_SYS_POCMR0_MASK_ATTRIB
#define POCMR0_MASK_ATTRIB	(POCMR_MASK_256MB | POCMR_ENABLE | POCMR_PREFETCH_EN)
#else
#define POCMR0_MASK_ATTRIB CONFIG_SYS_POCMR0_MASK_ATTRIB
#endif

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#ifndef CONFIG_SYS_PCI_MSTR_MEMIO_LOCAL
#define PCI_MSTR_MEMIO_LOCAL 0x90000000 /* Local base */
#else
#define PCI_MSTR_MEMIO_LOCAL CONFIG_SYS_PCI_MSTR_MEMIO_LOCAL
#endif

#ifndef CONFIG_SYS_PCI_MSTR_MEMIO_BUS
#define PCI_MSTR_MEMIO_BUS 0x90000000	/* PCI base   */
#else
#define PCI_MSTR_MEMIO_BUS CONFIG_SYS_PCI_MSTR_MEMIO_BUS
#endif

#ifndef CONFIG_SYS_CPU_PCI_MEMIO_START
#define CPU_PCI_MEMIO_START PCI_MSTR_MEMIO_LOCAL
#else
#define CPU_PCI_MEMIO_START CONFIG_SYS_CPU_PCI_MEMIO_START
#endif

#ifndef CONFIG_SYS_PCI_MSTR_MEMIO_SIZE
#define PCI_MSTR_MEMIO_SIZE 0x10000000	/* 256 MB */
#else
#define PCI_MSTR_MEMIO_SIZE CONFIG_SYS_PCI_MSTR_MEMIO_SIZE
#endif

#ifndef CONFIG_SYS_POCMR1_MASK_ATTRIB
#define POCMR1_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE)
#else
#define POCMR1_MASK_ATTRIB CONFIG_SYS_POCMR1_MASK_ATTRIB
#endif

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the third set of Outbound ATU registers
 * in the bridge.
 */

#ifndef CONFIG_SYS_PCI_MSTR_IO_LOCAL
#define PCI_MSTR_IO_LOCAL 0xA0000000	/* Local base */
#else
#define PCI_MSTR_IO_LOCAL CONFIG_SYS_PCI_MSTR_IO_LOCAL
#endif

#ifndef CONFIG_SYS_PCI_MSTR_IO_BUS
#define PCI_MSTR_IO_BUS 0xA0000000	/* PCI base   */
#else
#define PCI_MSTR_IO_BUS CONFIG_SYS_PCI_MSTR_IO_BUS
#endif

#ifndef CONFIG_SYS_CPU_PCI_IO_START
#define CPU_PCI_IO_START PCI_MSTR_IO_LOCAL
#else
#define CPU_PCI_IO_START CONFIG_SYS_CPU_PCI_IO_START
#endif

#ifndef CONFIG_SYS_PCI_MSTR_IO_SIZE
#define PCI_MSTR_IO_SIZE 0x10000000	/* 256MB */
#else
#define PCI_MSTR_IO_SIZE CONFIG_SYS_PCI_MSTR_IO_SIZE
#endif

#ifndef CONFIG_SYS_POCMR2_MASK_ATTRIB
#define POCMR2_MASK_ATTRIB	(POCMR_MASK_256MB | POCMR_ENABLE | POCMR_PCI_IO)
#else
#define POCMR2_MASK_ATTRIB CONFIG_SYS_POCMR2_MASK_ATTRIB
#endif

/* PCI bus configuration registers.
 */

#define PCI_CLASS_BRIDGE_CTLR	0x06


static inline void pci_outl (u32 addr, u32 data)
{
	*(volatile u32 *) addr = cpu_to_le32 (data);
}

void pci_mpc8250_init (struct pci_controller *hose)
{
	u16 tempShort;

	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	pci_dev_t host_devno = PCI_BDF (0, 0, 0);

	pci_setup_indirect (hose, CONFIG_SYS_IMMR + PCI_CFG_ADDR_REG,
			    CONFIG_SYS_IMMR + PCI_CFG_DATA_REG);

	/*
	 * Setting required to enable IRQ1-IRQ7 (SIUMCR [DPPC]),
	 * and local bus for PCI (SIUMCR [LBPC]).
	 */
	immap->im_siu_conf.sc_siumcr = (immap->im_siu_conf.sc_siumcr &
						~SIUMCR_LBPC11 &
						~SIUMCR_CS10PC11 &
						~SIUMCR_LBPC11) |
					SIUMCR_LBPC01 |
					SIUMCR_CS10PC01 |
					SIUMCR_APPC10;

	/* Make PCI lowest priority */
	/* Each 4 bits is a device bus request	and the MS 4bits
	   is highest priority */
	/* Bus		     4bit value
	   ---		     ----------
	   CPM high	     0b0000
	   CPM middle	     0b0001
	   CPM low	     0b0010
	   PCI reguest	     0b0011
	   Reserved	     0b0100
	   Reserved	     0b0101
	   Internal Core     0b0110
	   External Master 1 0b0111
	   External Master 2 0b1000
	   External Master 3 0b1001
	   The rest are reserved */
	immap->im_siu_conf.sc_ppc_alrh = 0x61207893;

	/* Park bus on core while modifying PCI Bus accesses */
	immap->im_siu_conf.sc_ppc_acr = 0x6;

	/*
	 * Set up master windows that allow the CPU to access PCI space. These
	 * windows are set up using the two SIU PCIBR registers.
	 */
	immap->im_memctl.memc_pcimsk0 = PCIMSK0_MASK;
	immap->im_memctl.memc_pcibr0 = PCI_MSTR0_LOCAL | PCIBR_ENABLE;

	/* Release PCI RST (by default the PCI RST signal is held low)	*/
	immap->im_pci.pci_gcr = cpu_to_le32 (PCIGCR_PCI_BUS_EN);

	/* give it some time */
	{
			udelay (1000);
	}

	/*
	 * Set up master window that allows the CPU to access PCI Memory (prefetch)
	 * space. This window is set up using the first set of Outbound ATU registers.
	 */
	immap->im_pci.pci_potar0 = cpu_to_le32 (PCI_MSTR_MEM_BUS >> 12);	/* PCI base */
	immap->im_pci.pci_pobar0 = cpu_to_le32 (PCI_MSTR_MEM_LOCAL >> 12);	/* Local base */
	immap->im_pci.pci_pocmr0 = cpu_to_le32 (POCMR0_MASK_ATTRIB);	/* Size & attribute */

	/*
	 * Set up master window that allows the CPU to access PCI Memory (non-prefetch)
	 * space. This window is set up using the second set of Outbound ATU registers.
	 */
	immap->im_pci.pci_potar1 = cpu_to_le32 (PCI_MSTR_MEMIO_BUS >> 12);	/* PCI base */
	immap->im_pci.pci_pobar1 = cpu_to_le32 (PCI_MSTR_MEMIO_LOCAL >> 12);	/* Local base */
	immap->im_pci.pci_pocmr1 = cpu_to_le32 (POCMR1_MASK_ATTRIB);	/* Size & attribute */

	/*
	 * Set up master window that allows the CPU to access PCI IO space. This window
	 * is set up using the third set of Outbound ATU registers.
	 */
	immap->im_pci.pci_potar2 = cpu_to_le32 (PCI_MSTR_IO_BUS >> 12); /* PCI base */
	immap->im_pci.pci_pobar2 = cpu_to_le32 (PCI_MSTR_IO_LOCAL >> 12);	/* Local base */
	immap->im_pci.pci_pocmr2 = cpu_to_le32 (POCMR2_MASK_ATTRIB);	/* Size & attribute */

	/*
	 * Set up slave window that allows PCI masters to access MPC826x local memory.
	 * This window is set up using the first set of Inbound ATU registers
	 */
	immap->im_pci.pci_pitar0 = cpu_to_le32 (PCI_SLV_MEM_LOCAL >> 12);	/* PCI base */
	immap->im_pci.pci_pibar0 = cpu_to_le32 (PCI_SLV_MEM_BUS >> 12); /* Local base */
	immap->im_pci.pci_picmr0 = cpu_to_le32 (PICMR0_MASK_ATTRIB);	/* Size & attribute */

	/* See above for description - puts PCI request as highest priority */
	immap->im_siu_conf.sc_ppc_alrh = 0x03124567;

	/* Park the bus on the PCI */
	immap->im_siu_conf.sc_ppc_acr = PPC_ACR_BUS_PARK_PCI;

	/* Host mode - specify the bridge as a host-PCI bridge */

	pci_hose_write_config_byte (hose, host_devno, PCI_CLASS_CODE,
				    PCI_CLASS_BRIDGE_CTLR);

	/* Enable the host bridge to be a master on the PCI bus, and to act as a PCI memory target */
	pci_hose_read_config_word (hose, host_devno, PCI_COMMAND, &tempShort);
	pci_hose_write_config_word (hose, host_devno, PCI_COMMAND,
				    tempShort | PCI_COMMAND_MASTER |
				    PCI_COMMAND_MEMORY);

	/* do some bridge init, should be done on all 8260 based bridges */
	pci_hose_write_config_byte (hose, host_devno, PCI_CACHE_LINE_SIZE,
				    0x08);
	pci_hose_write_config_byte (hose, host_devno, PCI_LATENCY_TIMER,
				    0xF8);

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region (hose->regions + 0,
			CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_BASE,
			0x4000000, PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region (hose->regions + 1,
			PCI_MSTR_MEM_BUS,
			PCI_MSTR_MEM_LOCAL,
			PCI_MSTR_MEM_SIZE, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (hose->regions + 2,
			PCI_MSTR_IO_BUS,
			PCI_MSTR_IO_LOCAL, PCI_MSTR_IO_SIZE, PCI_REGION_IO);

	hose->region_count = 3;

	pci_register_hose (hose);
	/* Mask off master abort machine checks */
	immap->im_pci.pci_emr &= cpu_to_le32 (~PCI_ERROR_PCI_NO_RSP);
	eieio ();

	hose->last_busno = pci_hose_scan (hose);


	/* clear the error in the error status register */
	immap->im_pci.pci_esr = cpu_to_le32 (PCI_ERROR_PCI_NO_RSP);

	/* unmask master abort machine checks */
	immap->im_pci.pci_emr |= cpu_to_le32 (PCI_ERROR_PCI_NO_RSP);
}

#if defined(CONFIG_OF_LIBFDT)
void ft_pci_setup(void *blob, bd_t *bd)
{
	do_fixup_by_prop_u32(blob, "device_type", "pci", 4,
		"clock-frequency", gd->pci_clk, 1);
}
#endif

#endif /* CONFIG_PCI */
