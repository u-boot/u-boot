/*
 * From Coreboot northbridge/intel/sandybridge/northbridge.c
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The Chromium Authors
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/msr.h>
#include <asm/acpi.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/processor.h>
#include <asm/arch/pch.h>
#include <asm/arch/model_206ax.h>
#include <asm/arch/sandybridge.h>

static int bridge_revision_id = -1;

int bridge_silicon_revision(void)
{
	if (bridge_revision_id < 0) {
		struct cpuid_result result;
		uint8_t stepping, bridge_id;
		pci_dev_t dev;

		result = cpuid(1);
		stepping = result.eax & 0xf;
		dev = PCI_BDF(0, 0, 0);
		bridge_id = x86_pci_read_config16(dev, PCI_DEVICE_ID) & 0xf0;
		bridge_revision_id = bridge_id | stepping;
	}

	return bridge_revision_id;
}

/*
 * Reserve everything between A segment and 1MB:
 *
 * 0xa0000 - 0xbffff: legacy VGA
 * 0xc0000 - 0xcffff: VGA OPROM (needed by kernel)
 * 0xe0000 - 0xfffff: SeaBIOS, if used, otherwise DMI
 */
static const int legacy_hole_base_k = 0xa0000 / 1024;
static const int legacy_hole_size_k = 384;

static int get_pcie_bar(u32 *base, u32 *len)
{
	pci_dev_t dev = PCI_BDF(0, 0, 0);
	u32 pciexbar_reg;

	*base = 0;
	*len = 0;

	pciexbar_reg = x86_pci_read_config32(dev, PCIEXBAR);

	if (!(pciexbar_reg & (1 << 0)))
		return 0;

	switch ((pciexbar_reg >> 1) & 3) {
	case 0: /* 256MB */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28));
		*len = 256 * 1024 * 1024;
		return 1;
	case 1: /* 128M */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28) | (1 << 27));
		*len = 128 * 1024 * 1024;
		return 1;
	case 2: /* 64M */
		*base = pciexbar_reg & ((1 << 31) | (1 << 30) | (1 << 29) |
				(1 << 28) | (1 << 27) | (1 << 26));
		*len = 64 * 1024 * 1024;
		return 1;
	}

	return 0;
}

static void add_fixed_resources(pci_dev_t dev, int index)
{
	u32 pcie_config_base, pcie_config_size;

	if (get_pcie_bar(&pcie_config_base, &pcie_config_size)) {
		debug("Adding PCIe config bar base=0x%08x size=0x%x\n",
		      pcie_config_base, pcie_config_size);
	}
}

static void northbridge_dmi_init(pci_dev_t dev)
{
	/* Clear error status bits */
	writel(0xffffffff, DMIBAR_REG(0x1c4));
	writel(0xffffffff, DMIBAR_REG(0x1d0));

	/* Steps prior to DMI ASPM */
	if ((bridge_silicon_revision() & BASE_REV_MASK) == BASE_REV_SNB) {
		clrsetbits_le32(DMIBAR_REG(0x250), (1 << 22) | (1 << 20),
				1 << 21);
	}

	setbits_le32(DMIBAR_REG(0x238), 1 << 29);

	if (bridge_silicon_revision() >= SNB_STEP_D0) {
		setbits_le32(DMIBAR_REG(0x1f8), 1 << 16);
	} else if (bridge_silicon_revision() >= SNB_STEP_D1) {
		clrsetbits_le32(DMIBAR_REG(0x1f8), 1 << 26, 1 << 16);
		setbits_le32(DMIBAR_REG(0x1fc), (1 << 12) | (1 << 23));
	}

	/* Enable ASPM on SNB link, should happen before PCH link */
	if ((bridge_silicon_revision() & BASE_REV_MASK) == BASE_REV_SNB)
		setbits_le32(DMIBAR_REG(0xd04), 1 << 4);

	setbits_le32(DMIBAR_REG(0x88), (1 << 1) | (1 << 0));
}

void northbridge_init(pci_dev_t dev)
{
	u32 bridge_type;

	add_fixed_resources(dev, 6);
	northbridge_dmi_init(dev);

	bridge_type = readl(MCHBAR_REG(0x5f10));
	bridge_type &= ~0xff;

	if ((bridge_silicon_revision() & BASE_REV_MASK) == BASE_REV_IVB) {
		/* Enable Power Aware Interrupt Routing - fixed priority */
		clrsetbits_8(MCHBAR_REG(0x5418), 0xf, 0x4);

		/* 30h for IvyBridge */
		bridge_type |= 0x30;
	} else {
		/* 20h for Sandybridge */
		bridge_type |= 0x20;
	}
	writel(bridge_type, MCHBAR_REG(0x5f10));

	/*
	 * Set bit 0 of BIOS_RESET_CPL to indicate to the CPU
	 * that BIOS has initialized memory and power management
	 */
	setbits_8(MCHBAR_REG(BIOS_RESET_CPL), 1);
	debug("Set BIOS_RESET_CPL\n");

	/* Configure turbo power limits 1ms after reset complete bit */
	mdelay(1);
	set_power_limits(28);

	/*
	 * CPUs with configurable TDP also need power limits set
	 * in MCHBAR.  Use same values from MSR_PKG_POWER_LIMIT.
	 */
	if (cpu_config_tdp_levels()) {
		msr_t msr = msr_read(MSR_PKG_POWER_LIMIT);

		writel(msr.lo, MCHBAR_REG(0x59A0));
		writel(msr.hi, MCHBAR_REG(0x59A4));
	}

	/* Set here before graphics PM init */
	writel(0x00100001, MCHBAR_REG(0x5500));
}

void northbridge_enable(pci_dev_t dev)
{
}
