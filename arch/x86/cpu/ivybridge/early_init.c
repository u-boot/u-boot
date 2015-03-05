/*
 * From Coreboot
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>
#include <asm/arch/sandybridge.h>

static void sandybridge_setup_bars(pci_dev_t pch_dev, pci_dev_t lpc_dev)
{
	/* Setting up Southbridge. In the northbridge code. */
	debug("Setting up static southbridge registers\n");
	x86_pci_write_config32(lpc_dev, PCH_RCBA_BASE, DEFAULT_RCBA | 1);

	x86_pci_write_config32(lpc_dev, PMBASE, DEFAULT_PMBASE | 1);
	x86_pci_write_config8(lpc_dev, ACPI_CNTL, 0x80); /* Enable ACPI BAR */

	debug("Disabling watchdog reboot\n");
	setbits_le32(RCB_REG(GCS), 1 >> 5);	/* No reset */
	outw(1 << 11, DEFAULT_PMBASE | 0x60 | 0x08);	/* halt timer */

	/* Set up all hardcoded northbridge BARs */
	debug("Setting up static registers\n");
	x86_pci_write_config32(pch_dev, EPBAR, DEFAULT_EPBAR | 1);
	x86_pci_write_config32(pch_dev, EPBAR + 4, (0LL + DEFAULT_EPBAR) >> 32);
	x86_pci_write_config32(pch_dev, MCHBAR, DEFAULT_MCHBAR | 1);
	x86_pci_write_config32(pch_dev, MCHBAR + 4,
			       (0LL + DEFAULT_MCHBAR) >> 32);
	/* 64MB - busses 0-63 */
	x86_pci_write_config32(pch_dev, PCIEXBAR, DEFAULT_PCIEXBAR | 5);
	x86_pci_write_config32(pch_dev, PCIEXBAR + 4,
			       (0LL + DEFAULT_PCIEXBAR) >> 32);
	x86_pci_write_config32(pch_dev, DMIBAR, DEFAULT_DMIBAR | 1);
	x86_pci_write_config32(pch_dev, DMIBAR + 4,
			       (0LL + DEFAULT_DMIBAR) >> 32);

	/* Set C0000-FFFFF to access RAM on both reads and writes */
	x86_pci_write_config8(pch_dev, PAM0, 0x30);
	x86_pci_write_config8(pch_dev, PAM1, 0x33);
	x86_pci_write_config8(pch_dev, PAM2, 0x33);
	x86_pci_write_config8(pch_dev, PAM3, 0x33);
	x86_pci_write_config8(pch_dev, PAM4, 0x33);
	x86_pci_write_config8(pch_dev, PAM5, 0x33);
	x86_pci_write_config8(pch_dev, PAM6, 0x33);
}

static void sandybridge_setup_graphics(pci_dev_t pch_dev, pci_dev_t video_dev)
{
	u32 reg32;
	u16 reg16;
	u8 reg8;

	reg16 = x86_pci_read_config16(video_dev, PCI_DEVICE_ID);
	switch (reg16) {
	case 0x0102: /* GT1 Desktop */
	case 0x0106: /* GT1 Mobile */
	case 0x010a: /* GT1 Server */
	case 0x0112: /* GT2 Desktop */
	case 0x0116: /* GT2 Mobile */
	case 0x0122: /* GT2 Desktop >=1.3GHz */
	case 0x0126: /* GT2 Mobile >=1.3GHz */
	case 0x0156: /* IvyBridge */
	case 0x0166: /* IvyBridge */
		break;
	default:
		debug("Graphics not supported by this CPU/chipset\n");
		return;
	}

	debug("Initialising Graphics\n");

	/* Setup IGD memory by setting GGC[7:3] = 1 for 32MB */
	reg16 = x86_pci_read_config16(pch_dev, GGC);
	reg16 &= ~0x00f8;
	reg16 |= 1 << 3;
	/* Program GTT memory by setting GGC[9:8] = 2MB */
	reg16 &= ~0x0300;
	reg16 |= 2 << 8;
	/* Enable VGA decode */
	reg16 &= ~0x0002;
	x86_pci_write_config16(pch_dev, GGC, reg16);

	/* Enable 256MB aperture */
	reg8 = x86_pci_read_config8(video_dev, MSAC);
	reg8 &= ~0x06;
	reg8 |= 0x02;
	x86_pci_write_config8(video_dev, MSAC, reg8);

	/* Erratum workarounds */
	reg32 = readl(MCHBAR_REG(0x5f00));
	reg32 |= (1 << 9) | (1 << 10);
	writel(reg32, MCHBAR_REG(0x5f00));

	/* Enable SA Clock Gating */
	reg32 = readl(MCHBAR_REG(0x5f00));
	writel(reg32 | 1, MCHBAR_REG(0x5f00));

	/* GPU RC6 workaround for sighting 366252 */
	reg32 = readl(MCHBAR_REG(0x5d14));
	reg32 |= (1 << 31);
	writel(reg32, MCHBAR_REG(0x5d14));

	/* VLW */
	reg32 = readl(MCHBAR_REG(0x6120));
	reg32 &= ~(1 << 0);
	writel(reg32, MCHBAR_REG(0x6120));

	reg32 = readl(MCHBAR_REG(0x5418));
	reg32 |= (1 << 4) | (1 << 5);
	writel(reg32, MCHBAR_REG(0x5418));
}

void sandybridge_early_init(int chipset_type)
{
	pci_dev_t pch_dev = PCH_DEV;
	pci_dev_t video_dev = PCH_VIDEO_DEV;
	pci_dev_t lpc_dev = PCH_LPC_DEV;
	u32 capid0_a;
	u8 reg8;

	/* Device ID Override Enable should be done very early */
	capid0_a = x86_pci_read_config32(pch_dev, 0xe4);
	if (capid0_a & (1 << 10)) {
		reg8 = x86_pci_read_config8(pch_dev, 0xf3);
		reg8 &= ~7; /* Clear 2:0 */

		if (chipset_type == SANDYBRIDGE_MOBILE)
			reg8 |= 1; /* Set bit 0 */

		x86_pci_write_config8(pch_dev, 0xf3, reg8);
	}

	/* Setup all BARs required for early PCIe and raminit */
	sandybridge_setup_bars(pch_dev, lpc_dev);

	/* Device Enable */
	x86_pci_write_config32(pch_dev, DEVEN, DEVEN_HOST | DEVEN_IGD);

	sandybridge_setup_graphics(pch_dev, video_dev);
}
