// SPDX-License-Identifier: GPL-2.0
/*
 * AMD Versal2 DesignWare PCIe host controller driver
 *
 * Copyright (C) 2025 - 2026, Advanced Micro Devices, Inc.
 * Author: Pranav Sanwal <pranav.sanwal@amd.com>
 */

#include <dm.h>
#include <log.h>
#include <pci.h>
#include <wait_bit.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <linux/delay.h>

#include "pcie_dw_common.h"

/*
 * SLCR (System Level Control Register) Interrupt Register Offsets
 * These are relative to the SLCR base address from device tree
 */
#define AMD_DW_TLP_IR_STATUS_MISC	0x4c0
#define AMD_DW_TLP_IR_DISABLE_MISC	0x4cc

/* Interrupt bit definitions */
#define AMD_DW_PCIE_INTR_CMPL_TIMEOUT		15
#define AMD_DW_PCIE_INTR_PM_PME_RCVD		24
#define AMD_DW_PCIE_INTR_PME_TO_ACK_RCVD	25
#define AMD_DW_PCIE_INTR_MISC_CORRECTABLE	26
#define AMD_DW_PCIE_INTR_NONFATAL		27
#define AMD_DW_PCIE_INTR_FATAL			28

#define AMD_DW_PCIE_INTR_INTX_MASK		GENMASK(23, 16)

#define AMD_DW_PCIE_IMR_ALL_MASK	\
	(BIT(AMD_DW_PCIE_INTR_CMPL_TIMEOUT)	| \
	 BIT(AMD_DW_PCIE_INTR_PM_PME_RCVD)	| \
	 BIT(AMD_DW_PCIE_INTR_PME_TO_ACK_RCVD)	| \
	 BIT(AMD_DW_PCIE_INTR_MISC_CORRECTABLE)	| \
	 BIT(AMD_DW_PCIE_INTR_NONFATAL)		| \
	 BIT(AMD_DW_PCIE_INTR_FATAL)		| \
	 AMD_DW_PCIE_INTR_INTX_MASK)

/* DW PCIe Debug Registers (in DBI space) */
#define AMD_DW_PCIE_PORT_DEBUG1			0x72c
#define AMD_DW_PCIE_PORT_DEBUG1_LINK_UP		BIT(4)
#define AMD_DW_PCIE_PORT_DEBUG1_LINK_IN_TRAINING	BIT(29)
#define AMD_DW_PCIE_DBI_64BIT_MEM_DECODE		BIT(0)

/* Link training timeout */
#define LINK_WAIT_MSLEEP_MAX		1000

/* PCIe spec timing requirements */
#define PCIE_RESET_CONFIG_WAIT_MS	100
#define PCIE_T_PERST_WAIT_MS		1

/**
 * struct amd_dw_pcie - AMD DesignWare PCIe controller private data
 * @dw: DesignWare PCIe common structure
 * @slcr_base: System Level Control Register base (for interrupts)
 */
struct amd_dw_pcie {
	struct pcie_dw dw;
	void __iomem *slcr_base;
};

static void amd_dw_pcie_init_port(struct amd_dw_pcie *pcie)
{
	u32 val;

	if (!pcie->slcr_base)
		return;

	/* Disable all TLP interrupts */
	writel(AMD_DW_PCIE_IMR_ALL_MASK,
	       pcie->slcr_base + AMD_DW_TLP_IR_DISABLE_MISC);

	/* Clear any pending TLP interrupts */
	val = readl(pcie->slcr_base + AMD_DW_TLP_IR_STATUS_MISC);
	val &= AMD_DW_PCIE_IMR_ALL_MASK;
	writel(val, pcie->slcr_base + AMD_DW_TLP_IR_STATUS_MISC);
}

static void amd_dw_pcie_start_link(struct amd_dw_pcie *pcie)
{
	void __iomem *reg = pcie->dw.dbi_base + AMD_DW_PCIE_PORT_DEBUG1;
	struct udevice *dev = pcie->dw.dev;
	struct pcie_dw *pci = &pcie->dw;
	int ret;

	ret = wait_for_bit_le32(reg, AMD_DW_PCIE_PORT_DEBUG1_LINK_UP,
				true, LINK_WAIT_MSLEEP_MAX,
				false);
	if (!ret)
		ret = wait_for_bit_le32(reg,
					AMD_DW_PCIE_PORT_DEBUG1_LINK_IN_TRAINING,
					false, LINK_WAIT_MSLEEP_MAX, false);
	if (ret)
		dev_warn(dev, "PCIE-%d: Link down\n", dev_seq(dev));
	else
		dev_dbg(dev, "PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
			dev_seq(dev), pcie_dw_get_link_speed(pci),
			pcie_dw_get_link_width(pci), pci->first_busno);
}

static void amd_dw_pcie_host_init(struct amd_dw_pcie *pcie)
{
	struct pcie_dw *pci = &pcie->dw;

	/*
	 * Set 64-bit prefetchable memory decode capability. U-Boot's pci_auto.c
	 * reads this bit before assigning prefetchable BARs. If cleared, it skips
	 * PCI_PREF_BASE_UPPER32 programming, causing 64-bit BAR assignment to fail.
	 */
	dw_pcie_dbi_write_enable(pci, true);
	setbits_le32(pci->dbi_base + PCI_PREF_MEMORY_BASE,
		     AMD_DW_PCIE_DBI_64BIT_MEM_DECODE);
	dw_pcie_dbi_write_enable(pci, false);

	amd_dw_pcie_init_port(pcie);
	pcie_dw_setup_host(pci);
}

static void amd_dw_pcie_request_gpio(struct udevice *dev)
{
	struct gpio_desc perst_gpio;
	ofnode child_node;
	int ret;

	/*
	 * PERST# reset GPIO is optional. Child PCI endpoint nodes may carry a
	 * 'reset-gpios' property to toggle the endpoint reset signal during
	 * initialization. If absent, the endpoint is assumed to be already
	 * released from reset.
	 */
	ofnode_for_each_subnode(child_node, dev_ofnode(dev)) {
		ret = gpio_request_by_name_nodev(child_node, "reset-gpios", 0,
						 &perst_gpio, GPIOD_IS_OUT);
		if (!ret) {
			dev_dbg(dev, "Found reset-gpios in child node %s\n",
				ofnode_get_name(child_node));
			dm_gpio_set_value(&perst_gpio, 1);
			mdelay(PCIE_T_PERST_WAIT_MS);
			dm_gpio_set_value(&perst_gpio, 0);
			mdelay(PCIE_RESET_CONFIG_WAIT_MS);
			dm_gpio_free(dev, &perst_gpio);
		}
	}
}

static int amd_dw_pcie_of_to_plat(struct udevice *dev)
{
	struct pci_region *io_region, *mem_region, *pref_region;
	struct amd_dw_pcie *pcie = dev_get_priv(dev);
	struct pcie_dw *pci = &pcie->dw;
	int ret;

	pci->dev = dev;

	pci->dbi_base = dev_read_addr_name_ptr(dev, "dbi");
	if (!pci->dbi_base) {
		dev_err(dev, "Missing 'dbi' register region\n");
		return -EINVAL;
	}

	pci->cfg_base = dev_read_addr_size_name_ptr(dev, "config", &pci->cfg_size);
	if (!pci->cfg_base) {
		dev_err(dev, "Missing 'config' register region\n");
		return -EINVAL;
	}

	pci->atu_base = dev_read_addr_name_ptr(dev, "atu");
	if (!pci->atu_base) {
		dev_dbg(dev, "No 'atu' region, using default offset from DBI\n");
		pci->atu_base = pci->dbi_base + DEFAULT_DBI_ATU_OFFSET;
	}

	pcie->slcr_base = dev_read_addr_name_ptr(dev, "slcr");
	if (!pcie->slcr_base)
		dev_dbg(dev, "No 'slcr' region, interrupt features disabled\n");

	ret = pci_get_regions(dev, &io_region, &mem_region, &pref_region);
	if (ret < 0) {
		dev_err(dev, "Failed to get PCI regions: %d\n", ret);
		return ret;
	}

	if (mem_region)
		pci->mem = *mem_region;

	return 0;
}

static int amd_dw_pcie_probe(struct udevice *dev)
{
	struct amd_dw_pcie *pcie = dev_get_priv(dev);
	struct pcie_dw *pci = &pcie->dw;

	/* Set first bus number */
	pci->first_busno = dev_seq(dev);

	amd_dw_pcie_request_gpio(dev);
	amd_dw_pcie_host_init(pcie);
	amd_dw_pcie_start_link(pcie);

	if (pci->mem.size) {
		dev_dbg(dev, "Programming ATU region 0 for MEM: phys=0x%llx bus=0x%llx size=0x%llx\n",
			(unsigned long long)pci->mem.phys_start,
			(unsigned long long)pci->mem.bus_start,
			(unsigned long long)pci->mem.size);
		pcie_dw_prog_outbound_atu_unroll(pci,
						 PCIE_ATU_REGION_INDEX0,
						 PCIE_ATU_TYPE_MEM,
						 pci->mem.phys_start,
						 pci->mem.bus_start,
						 pci->mem.size);
	} else {
		dev_warn(dev, "No MEM region configured!\n");
	}

	dev_dbg(dev, "dbi: 0x%lx | config: 0x%lx | atu: 0x%lx | slcr: 0x%lx\n",
		(long)pci->dbi_base, (long)pci->cfg_base,
		(long)pci->atu_base, (long)pcie->slcr_base);

	return 0;
}

static const struct dm_pci_ops amd_dw_pcie_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id amd_dw_pcie_ids[] = {
	{ .compatible = "amd,versal2-mdb-host" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_amd) = {
	.name		= "pcie_dw_amd",
	.id		= UCLASS_PCI,
	.of_match	= amd_dw_pcie_ids,
	.ops		= &amd_dw_pcie_ops,
	.of_to_plat	= amd_dw_pcie_of_to_plat,
	.probe		= amd_dw_pcie_probe,
	.priv_auto	= sizeof(struct amd_dw_pcie),
};
