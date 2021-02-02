// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>

#include <asm/io.h>

#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This driver supports multiple types of operations / host bridges / busses:
 *
 * OTX_ECAM: Octeon TX & TX2 ECAM (Enhanced Configuration Access Mechanism)
 *	     Used to access the internal on-chip devices which are connected
 *	     to internal buses
 * OTX_PEM:  Octeon TX PEM (PCI Express MAC)
 *	     Used to access the external (off-chip) PCI devices
 * OTX2_PEM: Octeon TX2 PEM (PCI Express MAC)
 *	     Used to access the external (off-chip) PCI devices
 */
enum {
	OTX_ECAM,
	OTX_PEM,
	OTX2_PEM,
};

/**
 * struct octeontx_pci - Driver private data
 * @type:	Device type matched via compatible (e.g. OTX_ECAM etc)
 * @cfg:	Config resource
 * @bus:	Bus resource
 */
struct octeontx_pci {
	unsigned int type;

	struct resource cfg;
	struct resource bus;
};

static uintptr_t octeontx_cfg_addr(struct octeontx_pci *pcie,
				   int bus_offs, int shift_offs,
				   pci_dev_t bdf, uint offset)
{
	u32 bus, dev, func;
	uintptr_t address;

	bus = PCI_BUS(bdf) + bus_offs;
	dev = PCI_DEV(bdf);
	func = PCI_FUNC(bdf);

	address = (bus << (20 + shift_offs)) |
		(dev << (15 + shift_offs)) |
		(func << (12 + shift_offs)) | offset;
	address += pcie->cfg.start;

	return address;
}

static ulong readl_size(uintptr_t addr, enum pci_size_t size)
{
	ulong val;

	switch (size) {
	case PCI_SIZE_8:
		val = readb(addr);
		break;
	case PCI_SIZE_16:
		val = readw(addr);
		break;
	case PCI_SIZE_32:
		val = readl(addr);
		break;
	default:
		printf("Invalid size\n");
		return -EINVAL;
	};

	return val;
}

static void writel_size(uintptr_t addr, enum pci_size_t size, ulong valuep)
{
	switch (size) {
	case PCI_SIZE_8:
		writeb(valuep, addr);
		break;
	case PCI_SIZE_16:
		writew(valuep, addr);
		break;
	case PCI_SIZE_32:
		writel(valuep, addr);
		break;
	default:
		printf("Invalid size\n");
	};
}

static bool octeontx_bdf_invalid(pci_dev_t bdf)
{
	if (PCI_BUS(bdf) == 1 && PCI_DEV(bdf) > 0)
		return true;

	return false;
}

static int octeontx_ecam_read_config(const struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong *valuep,
				     enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;

	address = octeontx_cfg_addr(pcie, pcie->bus.start - hose->first_busno,
				    0, bdf, offset);
	*valuep = readl_size(address, size);

	debug("%02x.%02x.%02x: u%d %x -> %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset, *valuep);

	return 0;
}

static int octeontx_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;

	address = octeontx_cfg_addr(pcie, pcie->bus.start - hose->first_busno,
				    0, bdf, offset);
	writel_size(address, size, value);

	debug("%02x.%02x.%02x: u%d %x <- %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset, value);

	return 0;
}

static int octeontx_pem_read_config(const struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong *valuep,
				    enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u8 hdrtype;
	u8 pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	address = octeontx_cfg_addr(pcie, 1 - hose->first_busno, 4,
				    bdf, 0);

	*valuep = pci_conv_32_to_size(~0UL, offset, size);

	if (octeontx_bdf_invalid(bdf))
		return -EPERM;

	*valuep = readl_size(address + offset, size);

	hdrtype = readb(address + PCI_HEADER_TYPE);
	if (hdrtype == PCI_HEADER_TYPE_BRIDGE &&
	    offset >= PCI_PRIMARY_BUS &&
	    offset <= PCI_SUBORDINATE_BUS &&
	    *valuep != pci_conv_32_to_size(~0UL, offset, size))
		*valuep -= pci_conv_32_to_size(bus_offs, offset, size);

	return 0;
}

static int octeontx_pem_write_config(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong value,
				     enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u8 hdrtype;
	u8 pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	address = octeontx_cfg_addr(pcie, 1 - hose->first_busno, 4, bdf, 0);

	hdrtype = readb(address + PCI_HEADER_TYPE);
	if (hdrtype == PCI_HEADER_TYPE_BRIDGE &&
	    offset >= PCI_PRIMARY_BUS &&
	    offset <= PCI_SUBORDINATE_BUS &&
	    value != pci_conv_32_to_size(~0UL, offset, size))
		value +=  pci_conv_32_to_size(bus_offs, offset, size);

	if (octeontx_bdf_invalid(bdf))
		return -EPERM;

	writel_size(address + offset, size, value);

	debug("%02x.%02x.%02x: u%d %x (%lx) <- %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset,
	      address, value);

	return 0;
}

static int octeontx2_pem_read_config(const struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong *valuep,
				     enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;

	address = octeontx_cfg_addr(pcie, 1 - hose->first_busno, 0,
				    bdf, 0);

	*valuep = pci_conv_32_to_size(~0UL, offset, size);

	if (octeontx_bdf_invalid(bdf))
		return -EPERM;

	*valuep = readl_size(address + offset, size);

	debug("%02x.%02x.%02x: u%d %x (%lx) -> %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset,
	      address, *valuep);

	return 0;
}

static int octeontx2_pem_write_config(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;

	address = octeontx_cfg_addr(pcie, 1 - hose->first_busno, 0,
				    bdf, 0);

	if (octeontx_bdf_invalid(bdf))
		return -EPERM;

	writel_size(address + offset, size, value);

	debug("%02x.%02x.%02x: u%d %x (%lx) <- %lx\n",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), size, offset,
	      address, value);

	return 0;
}

int pci_octeontx_read_config(const struct udevice *bus, pci_dev_t bdf,
			     uint offset, ulong *valuep,
			     enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	int ret = -EIO;

	switch (pcie->type) {
	case OTX_ECAM:
		ret = octeontx_ecam_read_config(bus, bdf, offset, valuep,
						size);
		break;
	case OTX_PEM:
		ret = octeontx_pem_read_config(bus, bdf, offset, valuep,
					       size);
		break;
	case OTX2_PEM:
		ret = octeontx2_pem_read_config(bus, bdf, offset, valuep,
						size);
		break;
	}

	return ret;
}

int pci_octeontx_write_config(struct udevice *bus, pci_dev_t bdf,
			      uint offset, ulong value,
			      enum pci_size_t size)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(bus);
	int ret = -EIO;

	switch (pcie->type) {
	case OTX_ECAM:
		ret = octeontx_ecam_write_config(bus, bdf, offset, value,
						 size);
		break;
	case OTX_PEM:
		ret = octeontx_pem_write_config(bus, bdf, offset, value,
						size);
		break;
	case OTX2_PEM:
		ret = octeontx2_pem_write_config(bus, bdf, offset, value,
						 size);
		break;
	}

	return ret;
}

static int pci_octeontx_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int pci_octeontx_probe(struct udevice *dev)
{
	struct octeontx_pci *pcie = (struct octeontx_pci *)dev_get_priv(dev);
	int err;

	pcie->type = dev_get_driver_data(dev);

	err = dev_read_resource(dev, 0, &pcie->cfg);
	if (err) {
		debug("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	err = dev_read_pci_bus_range(dev, &pcie->bus);
	if (err) {
		debug("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pci_octeontx_ops = {
	.read_config	= pci_octeontx_read_config,
	.write_config	= pci_octeontx_write_config,
};

static const struct udevice_id pci_octeontx_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-ecam", .data = OTX_ECAM },
	{ .compatible = "cavium,pci-host-octeontx-ecam", .data = OTX_ECAM },
	{ .compatible = "pci-host-ecam-generic", .data = OTX_ECAM },
	{ .compatible = "cavium,pci-host-thunder-pem", .data = OTX_PEM },
	{ .compatible = "marvell,pci-host-octeontx2-pem", .data = OTX2_PEM },
	{ }
};

U_BOOT_DRIVER(pci_octeontx) = {
	.name	= "pci_octeontx",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx_ids,
	.ops	= &pci_octeontx_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
	.flags = DM_FLAG_PRE_RELOC,
};
