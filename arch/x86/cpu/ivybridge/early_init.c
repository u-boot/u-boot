/*
 * From Coreboot
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>
#include <asm/arch/sandybridge.h>

static void sandybridge_setup_lpc_bars(pci_dev_t lpc_dev)
{
	/* Setting up Southbridge. In the northbridge code. */
	debug("Setting up static southbridge registers\n");
	x86_pci_write_config32(lpc_dev, PCH_RCBA_BASE, DEFAULT_RCBA | 1);

	x86_pci_write_config32(lpc_dev, PMBASE, DEFAULT_PMBASE | 1);
	x86_pci_write_config8(lpc_dev, ACPI_CNTL, 0x80); /* Enable ACPI BAR */

	debug("Disabling watchdog reboot\n");
	setbits_le32(RCB_REG(GCS), 1 >> 5);	/* No reset */
	outw(1 << 11, DEFAULT_PMBASE | 0x60 | 0x08);	/* halt timer */
}

static void sandybridge_setup_northbridge_bars(struct udevice *dev)
{
	/* Set up all hardcoded northbridge BARs */
	debug("Setting up static registers\n");
	dm_pci_write_config32(dev, EPBAR, DEFAULT_EPBAR | 1);
	dm_pci_write_config32(dev, EPBAR + 4, (0LL + DEFAULT_EPBAR) >> 32);
	dm_pci_write_config32(dev, MCHBAR, DEFAULT_MCHBAR | 1);
	dm_pci_write_config32(dev, MCHBAR + 4, (0LL + DEFAULT_MCHBAR) >> 32);
	/* 64MB - busses 0-63 */
	dm_pci_write_config32(dev, PCIEXBAR, DEFAULT_PCIEXBAR | 5);
	dm_pci_write_config32(dev, PCIEXBAR + 4,
			      (0LL + DEFAULT_PCIEXBAR) >> 32);
	dm_pci_write_config32(dev, DMIBAR, DEFAULT_DMIBAR | 1);
	dm_pci_write_config32(dev, DMIBAR + 4, (0LL + DEFAULT_DMIBAR) >> 32);

	/* Set C0000-FFFFF to access RAM on both reads and writes */
	dm_pci_write_config8(dev, PAM0, 0x30);
	dm_pci_write_config8(dev, PAM1, 0x33);
	dm_pci_write_config8(dev, PAM2, 0x33);
	dm_pci_write_config8(dev, PAM3, 0x33);
	dm_pci_write_config8(dev, PAM4, 0x33);
	dm_pci_write_config8(dev, PAM5, 0x33);
	dm_pci_write_config8(dev, PAM6, 0x33);
}

static int bd82x6x_northbridge_probe(struct udevice *dev)
{
	const int chipset_type = SANDYBRIDGE_MOBILE;
	u32 capid0_a;
	u8 reg8;

	if (gd->flags & GD_FLG_RELOC)
		return 0;

	/* Device ID Override Enable should be done very early */
	dm_pci_read_config32(dev, 0xe4, &capid0_a);
	if (capid0_a & (1 << 10)) {
		dm_pci_read_config8(dev, 0xf3, &reg8);
		reg8 &= ~7; /* Clear 2:0 */

		if (chipset_type == SANDYBRIDGE_MOBILE)
			reg8 |= 1; /* Set bit 0 */

		dm_pci_write_config8(dev, 0xf3, reg8);
	}

	sandybridge_setup_lpc_bars(PCH_LPC_DEV);

	sandybridge_setup_northbridge_bars(dev);

	/* Device Enable */
	dm_pci_write_config32(dev, DEVEN, DEVEN_HOST | DEVEN_IGD);

	return 0;
}

static const struct udevice_id bd82x6x_northbridge_ids[] = {
	{ .compatible = "intel,bd82x6x-northbridge" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_northbridge_drv) = {
	.name		= "bd82x6x_northbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= bd82x6x_northbridge_ids,
	.probe		= bd82x6x_northbridge_probe,
};
