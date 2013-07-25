/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_PCI)

#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <mpc5xxx.h>

/* System RAM mapped over PCI */
#define CONFIG_PCI_MEMORY_BUS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_MEMORY_PHYS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_MEMORY_SIZE	(1024 * 1024 * 1024)

/* PCIIWCR bit fields */
#define IWCR_MEM	(0 << 3)
#define IWCR_IO		(1 << 3)
#define IWCR_READ	(0 << 1)
#define IWCR_READLINE	(1 << 1)
#define IWCR_READMULT	(2 << 1)
#define IWCR_EN		(1 << 0)

static int mpc5200_read_config_dword(struct pci_controller *hose,
			      pci_dev_t dev, int offset, u32* value)
{
	*(volatile u32 *)MPC5XXX_PCI_CAR = (1 << 31) | dev | offset;
	eieio();
	udelay(10);
#if (defined CONFIG_PF5200 || defined CONFIG_CPCI5200)
	if (dev & 0x00ff0000) {
		u32 val;
		val  = in_le16((volatile u16 *)(CONFIG_PCI_IO_PHYS+2));
		udelay(10);
		val = val << 16;
		val |= in_le16((volatile u16 *)(CONFIG_PCI_IO_PHYS+0));
		*value = val;
	} else {
		*value = in_le32((volatile u32 *)CONFIG_PCI_IO_PHYS);
	}
	udelay(10);
#else
	*value = in_le32((volatile u32 *)CONFIG_PCI_IO_PHYS);
#endif
	eieio();
	*(volatile u32 *)MPC5XXX_PCI_CAR = 0;
	udelay(10);
	return 0;
}

static int mpc5200_write_config_dword(struct pci_controller *hose,
			      pci_dev_t dev, int offset, u32 value)
{
	*(volatile u32 *)MPC5XXX_PCI_CAR = (1 << 31) | dev | offset;
	eieio();
	udelay(10);
	out_le32((volatile u32 *)CONFIG_PCI_IO_PHYS, value);
	eieio();
	*(volatile u32 *)MPC5XXX_PCI_CAR = 0;
	udelay(10);
	return 0;
}

void pci_mpc5xxx_init (struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System space */
	pci_set_region(hose->regions + 0,
		       CONFIG_PCI_MEMORY_BUS,
		       CONFIG_PCI_MEMORY_PHYS,
		       CONFIG_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       CONFIG_PCI_MEM_BUS,
		       CONFIG_PCI_MEM_PHYS,
		       CONFIG_PCI_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI IO space */
	pci_set_region(hose->regions + 2,
		       CONFIG_PCI_IO_BUS,
		       CONFIG_PCI_IO_PHYS,
		       CONFIG_PCI_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 3;

	pci_register_hose(hose);

	/* GPIO Multiplexing - enable PCI */
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG &= ~(1 << 15);

	/* Set host bridge as pci master and enable memory decoding */
	*(vu_long *)MPC5XXX_PCI_CMD |=
		PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;

	/* Set maximum latency timer */
	*(vu_long *)MPC5XXX_PCI_CFG |= (0xf800);

	/* Set cache line size */
	*(vu_long *)MPC5XXX_PCI_CFG = (*(vu_long *)MPC5XXX_PCI_CFG & ~0xff) |
		(CONFIG_SYS_CACHELINE_SIZE / 4);

	/* Map MBAR to PCI space */
	*(vu_long *)MPC5XXX_PCI_BAR0 = CONFIG_SYS_MBAR;
	*(vu_long *)MPC5XXX_PCI_TBATR0 = CONFIG_SYS_MBAR | 1;

	/* Map RAM to PCI space */
	*(vu_long *)MPC5XXX_PCI_BAR1 = CONFIG_PCI_MEMORY_BUS | (1 << 3);
	*(vu_long *)MPC5XXX_PCI_TBATR1 = CONFIG_PCI_MEMORY_PHYS | 1;

	/* Park XLB on PCI */
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) &= ~((7 << 8) | (3 << 5));
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) |= (3 << 8) | (3 << 5);

	/* Disable interrupts from PCI controller */
	*(vu_long *)MPC5XXX_PCI_GSCR &= ~(7 << 12);
	*(vu_long *)MPC5XXX_PCI_ICR  &= ~(7 << 24);

	/* Set PCI retry counter to 0 = infinite retry. */
	/* The default of 255 is too short for slow devices. */
	*(vu_long *)MPC5XXX_PCI_ICR &= 0xFFFFFF00;

	/* Disable initiator windows */
	*(vu_long *)MPC5XXX_PCI_IWCR = 0;

	/* Map PCI memory to physical space */
	*(vu_long *)MPC5XXX_PCI_IW0BTAR = CONFIG_PCI_MEM_PHYS |
		(((CONFIG_PCI_MEM_SIZE - 1) >> 8) & 0x00ff0000) |
		(CONFIG_PCI_MEM_BUS >> 16);
	*(vu_long *)MPC5XXX_PCI_IWCR |= (IWCR_MEM | IWCR_READ | IWCR_EN) << 24;

	/* Map PCI I/O to physical space */
	*(vu_long *)MPC5XXX_PCI_IW1BTAR = CONFIG_PCI_IO_PHYS |
		(((CONFIG_PCI_IO_SIZE - 1) >> 8) & 0x00ff0000) |
		(CONFIG_PCI_IO_BUS >> 16);
	*(vu_long *)MPC5XXX_PCI_IWCR |= (IWCR_IO | IWCR_READ | IWCR_EN) << 16;

	/* Reset the PCI bus */
	*(vu_long *)MPC5XXX_PCI_GSCR |= 1;
	udelay(1000);
	*(vu_long *)MPC5XXX_PCI_GSCR &= ~1;
	udelay(1000);

	pci_set_ops(hose,
		pci_hose_read_config_byte_via_dword,
		pci_hose_read_config_word_via_dword,
		mpc5200_read_config_dword,
		pci_hose_write_config_byte_via_dword,
		pci_hose_write_config_word_via_dword,
		mpc5200_write_config_dword);

	udelay(1000);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif

	hose->last_busno = pci_hose_scan(hose);
}
#endif /* CONFIG_PCI */
