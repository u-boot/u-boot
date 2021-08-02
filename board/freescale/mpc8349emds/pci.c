// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 */

#include <init.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <mpc83xx.h>
#include <pci.h>
#include <i2c.h>
#include <asm/fsl_i2c.h>
#include <linux/delay.h>

static struct pci_region pci1_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI1_MEM_BASE,
		phys_start: CONFIG_SYS_PCI1_MEM_PHYS,
		size: CONFIG_SYS_PCI1_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI1_IO_BASE,
		phys_start: CONFIG_SYS_PCI1_IO_PHYS,
		size: CONFIG_SYS_PCI1_IO_SIZE,
		flags: PCI_REGION_IO
	},
	{
		bus_start: CONFIG_SYS_PCI1_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI1_MMIO_PHYS,
		size: CONFIG_SYS_PCI1_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
};

#ifdef CONFIG_MPC83XX_PCI2
static struct pci_region pci2_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI2_MEM_BASE,
		phys_start: CONFIG_SYS_PCI2_MEM_PHYS,
		size: CONFIG_SYS_PCI2_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI2_IO_BASE,
		phys_start: CONFIG_SYS_PCI2_IO_PHYS,
		size: CONFIG_SYS_PCI2_IO_SIZE,
		flags: PCI_REGION_IO
	},
	{
		bus_start: CONFIG_SYS_PCI2_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI2_MMIO_PHYS,
		size: CONFIG_SYS_PCI2_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
};
#endif

#ifndef CONFIG_PCISLAVE
void pib_init(void)
{
	u8 val8, orig_i2c_bus;
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
#if defined(CONFIG_PCI_64BIT)
	val8 = 0xf4;	/* PMC2:PCI1/64-bit */
#elif defined(CONFIG_PCI_ALL_PCI1)
	val8 = 0xf3;	/* PMC1:PCI1 PMC2:PCI1 PMC3:PCI1 */
#elif defined(CONFIG_PCI_ONE_PCI1)
	val8 = 0xf9;	/* PMC1:PCI1 PMC2:PCI2 PMC3:PCI2 */
#else
	val8 = 0xf5;	/* PMC1:PCI1 PMC2:PCI1 PMC3:PCI2 */
#endif
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

#if defined(CONFIG_PCI_64BIT)
	printf("PCI1: 64-bit on PMC2\n");
#elif defined(CONFIG_PCI_ALL_PCI1)
	printf("PCI1: 32-bit on PMC1, PMC2, PMC3\n");
#elif defined(CONFIG_PCI_ONE_PCI1)
	printf("PCI1: 32-bit on PMC1\n");
	printf("PCI2: 32-bit on PMC2, PMC3\n");
#else
	printf("PCI1: 32-bit on PMC1, PMC2\n");
	printf("PCI2: 32-bit on PMC3\n");
#endif
	/* Reset to original I2C bus */
	i2c_set_bus_num(orig_i2c_bus);
}

#endif /* CONFIG_PCISLAVE */
