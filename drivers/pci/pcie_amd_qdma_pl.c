// SPDX-License-Identifier: GPL-2.0-only
/*
 * PCIe host bridge driver for AMD QDMA PL PCIe bridge (no XDMA support)
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc.
 */

#define LOG_CATEGORY UCLASS_PCI

#include <dm.h>
#include <pci.h>

#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/io.h>

/* Register definitions */
#define AMD_QDMA_REG_IDR			0x138
#define AMD_QDMA_REG_IMR			0x13c
#define AMD_QDMA_REG_PSCR			0x144
#define AMD_QDMA_REG_RPSC			0x148

#define AMD_QDMA_IMR_ALL_MASK		0x0ff30fe9
#define AMD_QDMA_IDR_ALL_MASK		0xffffffff

/* Root Port Status/control Register definitions */
#define AMD_QDMA_REG_RPSC_BEN		BIT(0)

/* Phy Status/Control Register definitions */
#define AMD_QDMA_REG_PSCR_LNKUP		BIT(11)
#define QDMA_BRIDGE_BASE_OFF			0xcd8

/* Parameters for the waiting for link up routine */
#define LINK_WAIT_MAX_RETRIES			10
#define LINK_WAIT_DELAY_US			90000

/**
 * struct amd_qdma_pcie - AMD QDMA PL PCIe bridge private data
 * @dev: PCIe host bridge device
 * @breg_base: Bridge register base address
 * @ecam_base: ECAM configuration space base address
 */
struct amd_qdma_pcie {
	struct udevice *dev;
	void __iomem *breg_base;
	void __iomem *ecam_base;
};

static inline u32 amd_qdma_pcie_read(struct amd_qdma_pcie *pcie, u32 reg)
{
	u32 val = readl(pcie->breg_base + reg + QDMA_BRIDGE_BASE_OFF);

	dev_dbg(pcie->dev, "rd reg 0x%03x = 0x%08x\n", reg, val);

	return val;
}

static inline void amd_qdma_pcie_write(struct amd_qdma_pcie *pcie, u32 val,
				       u32 reg)
{
	dev_dbg(pcie->dev, "wr reg 0x%03x = 0x%08x\n", reg, val);
	writel(val, pcie->breg_base + reg + QDMA_BRIDGE_BASE_OFF);
}

static bool amd_qdma_pcie_link_up(struct amd_qdma_pcie *pcie)
{
	return !!(amd_qdma_pcie_read(pcie, AMD_QDMA_REG_PSCR) &
		  AMD_QDMA_REG_PSCR_LNKUP);
}

static int amd_qdma_pcie_config_address(const struct udevice *bus,
					pci_dev_t bdf, uint offset,
					void **paddress)
{
	struct amd_qdma_pcie *priv = dev_get_priv(bus);
	u32 busnum = PCI_BUS(bdf) - dev_seq(bus);
	void *addr;

	if (busnum == 0) {
		if (PCI_DEV(bdf) > 0 || PCI_FUNC(bdf) > 0)
			return -ENODEV;
	} else {
		if (!amd_qdma_pcie_link_up(priv))
			return -ENODEV;
	}

	addr = priv->ecam_base;
	addr += PCIE_ECAM_OFFSET(busnum, PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	*paddress = addr;

	return 0;
}

static int amd_qdma_pcie_read_config(const struct udevice *bus,
				     pci_dev_t bdf, uint offset,
				     ulong *valuep, enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, amd_qdma_pcie_config_address,
					     bdf, offset, valuep, size);
}

static int amd_qdma_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	/*
	 * The QDMA PL bridge's config space only decodes 32-bit accesses;
	 * narrower writes must be promoted to a 32-bit read-modify-write.
	 */
	return pci_generic_mmap_write_config32(bus, amd_qdma_pcie_config_address,
						bdf, offset, value, size);
}

static const struct dm_pci_ops amd_qdma_pcie_ops = {
	.read_config = amd_qdma_pcie_read_config,
	.write_config = amd_qdma_pcie_write_config,
};

static int amd_qdma_pcie_bridge_init(struct amd_qdma_pcie *pcie)
{
	int retries;
	u32 val;

	/* Check if the link is up or not */
	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		if (amd_qdma_pcie_link_up(pcie))
			break;
		udelay(LINK_WAIT_DELAY_US);
	}

	if (retries >= LINK_WAIT_MAX_RETRIES) {
		dev_warn(pcie->dev, "PHY link never came up\n");
		return -ETIMEDOUT;
	}

	dev_dbg(pcie->dev, "PCIe Link is UP\n");

	/* Disable all interrupts */
	amd_qdma_pcie_write(pcie, ~AMD_QDMA_IDR_ALL_MASK, AMD_QDMA_REG_IMR);

	/* Clear pending interrupts */
	val = amd_qdma_pcie_read(pcie, AMD_QDMA_REG_IDR) & AMD_QDMA_IMR_ALL_MASK;
	amd_qdma_pcie_write(pcie, val, AMD_QDMA_REG_IDR);

	/* Set the Bridge enable bit */
	val = amd_qdma_pcie_read(pcie, AMD_QDMA_REG_RPSC);
	amd_qdma_pcie_write(pcie, val | AMD_QDMA_REG_RPSC_BEN, AMD_QDMA_REG_RPSC);

	return 0;
}

static int amd_qdma_pcie_parse_dt(struct amd_qdma_pcie *priv)
{
	struct udevice *dev = priv->dev;

	priv->breg_base = dev_read_addr_name_ptr(dev, "breg");
	if (!priv->breg_base) {
		dev_err(dev, "failed to get breg address\n");
		return -EINVAL;
	}

	priv->ecam_base = dev_read_addr_name_ptr(dev, "cfg");
	if (!priv->ecam_base) {
		dev_err(dev, "failed to get cfg address\n");
		return -EINVAL;
	}

	return 0;
}

static int amd_qdma_pcie_probe(struct udevice *dev)
{
	struct amd_qdma_pcie *priv = dev_get_priv(dev);
	int err;

	priv->dev = dev;

	err = amd_qdma_pcie_parse_dt(priv);
	if (err) {
		dev_err(dev, "Parsing DT failed\n");
		return err;
	}

	err = amd_qdma_pcie_bridge_init(priv);
	if (err) {
		dev_err(dev, "HW Initialization failed\n");
		return err;
	}

	return 0;
}

static const struct udevice_id amd_qdma_pcie_of_match[] = {
	{ .compatible = "xlnx,qdma-host-3.00", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(amd_qdma_pcie) = {
	.name = "amd_qdma_pcie",
	.id = UCLASS_PCI,
	.of_match = amd_qdma_pcie_of_match,
	.probe = amd_qdma_pcie_probe,
	.priv_auto = sizeof(struct amd_qdma_pcie),
	.ops = &amd_qdma_pcie_ops,
};
