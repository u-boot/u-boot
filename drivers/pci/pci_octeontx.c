// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <pci.h>

#include <asm/io.h>

#if defined(CONFIG_ARCH_OCTEONTX2)
#include <asm/arch/soc.h>

#define PEM_CFG_WR 0x18
#define PEM_CFG_RD 0x20

#define PCIERC_RASDP_DE_ME		0x440
#define PCIERC_RASDP_EP_CTL		0x420
#define PCIERC_RAS_EINJ_EN		0x348
#define PCIERC_RAS_EINJ_CTL6PE		0x3A4
#define PCIERC_RAS_EINJ_CTL6_CMPP0	0x364
#define PCIERC_RAS_EINJ_CTL6_CMPV0	0x374
#define PCIERC_RAS_EINJ_CTL6_CHGP1	0x388
#define PCIERC_RAS_EINJ_CTL6_CHGV1	0x398

#endif

DECLARE_GLOBAL_DATA_PTR;

struct octeontx_pci {
	unsigned int type;

	struct fdt_resource cfg;
	struct fdt_resource bus;
#if defined(CONFIG_ARCH_OCTEONTX2)
	struct fdt_resource pem;
#endif
};

static int pci_octeontx_ecam_read_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong *valuep,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) + pcie->bus.start - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12) | offset;

	address += pcie->cfg.start;

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address);
		break;
	case PCI_SIZE_16:
		*valuep = readw(address);
		break;
	case PCI_SIZE_32:
		*valuep = readl(address);
		break;
	};

	debug("%02x.%02x.%02x: u%d %x -> %lx\n",
	      b, d, f, size, offset, *valuep);
	return 0;
}

static int pci_octeontx_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
					  uint offset, ulong valuep,
					  enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) + pcie->bus.start - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12) | offset;

	address += pcie->cfg.start;

	switch (size) {
	case PCI_SIZE_8:
		writeb(valuep, address);
		break;
	case PCI_SIZE_16:
		writew(valuep, address);
		break;
	case PCI_SIZE_32:
		writel(valuep, address);
		break;
	};

	debug("%02x.%02x.%02x: u%d %x <- %lx\n",
	      b, d, f, size, offset, valuep);
	return 0;
}

static int pci_octeontx_pem_read_config(struct udevice *bus, pci_dev_t bdf,
					uint offset, ulong *valuep,
					enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;
	u8  hdrtype;
	u8  pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 24) | (d << 19) | (f << 16);

	address += pcie->cfg.start;

	*valuep = pci_conv_32_to_size(~0UL, offset, size);

	if (b == 1 && d > 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address + offset);
		break;
	case PCI_SIZE_16:
		*valuep = readw(address + offset);
		break;
	case PCI_SIZE_32:
		*valuep = readl(address + offset);
		break;
	default:
		printf("Invalid size\n");
	}

	hdrtype = readb(address + PCI_HEADER_TYPE);

	if (hdrtype == PCI_HEADER_TYPE_BRIDGE &&
	    offset >= PCI_PRIMARY_BUS &&
	    offset <= PCI_SUBORDINATE_BUS &&
	    *valuep != pci_conv_32_to_size(~0UL, offset, size)) {
		*valuep -= pci_conv_32_to_size(bus_offs, offset, size);
	}
	debug("%02x.%02x.%02x: u%d %x (%lx) -> %lx\n",
	      b, d, f, size, offset, address, *valuep);
	return 0;
}

static int pci_octeontx_pem_write_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong value,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;
	u8  hdrtype;
	u8  pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 24) | (d << 19) | (f << 16);

	address += pcie->cfg.start;

	hdrtype = readb(address + PCI_HEADER_TYPE);

	if (hdrtype == PCI_HEADER_TYPE_BRIDGE &&
	    offset >= PCI_PRIMARY_BUS &&
	    offset <= PCI_SUBORDINATE_BUS &&
	    value != pci_conv_32_to_size(~0UL, offset, size)) {
		value +=  pci_conv_32_to_size(bus_offs, offset, size);
	}

	if (b == 1 && d > 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		writeb(value, address + offset);
		break;
	case PCI_SIZE_16:
		writew(value, address + offset);
		break;
	case PCI_SIZE_32:
		writel(value, address + offset);
		break;
	default:
		printf("Invalid size\n");
	}
	debug("%02x.%02x.%02x: u%d %x (%lx) <- %lx\n",
	      b, d, f, size, offset, address, value);
	return 0;
}

#if defined(CONFIG_ARCH_OCTEONTX2)
static inline bool use_workaround(void)
{
	u8 var = read_partvar();

	/* HW issue workaround should be applied only to older silicons */
	if (otx_is_soc(CN96XX) && var <= 1)
		return true;

	return false;
}

static int pci_octeontx2_pem_read_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong *valuep,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12);

	debug("bdf %x %02x.%02x.%02x: u%d %x (%lx)\n",
	      bdf, b, d, f, size, offset, address);
	address += pcie->cfg.start;

	debug("%02x.%02x.%02x: u%d %x (%lx) %lx\n",
	      b, d, f, size, offset, address, *valuep);
	*valuep = pci_conv_32_to_size(~0UL, offset, size);

	if (b == 1 && d > 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		debug("byte %lx\n", address + offset);
		*valuep = readb(address + offset);
		break;
	case PCI_SIZE_16:
		debug("word %lx\n", address + offset);
		*valuep = readw(address + offset);
		break;
	case PCI_SIZE_32:
		debug("long %lx\n", address + offset);
		*valuep = readl(address + offset);
		break;
	default:
		printf("Invalid size\n");
	}

	debug("%02x.%02x.%02x: u%d %x (%lx) -> %lx\n",
	      b, d, f, size, offset, address, *valuep);

	return 0;
}

static void pci_octeontx2_pem_workaround(struct udevice *bus, uint offset,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	u64 rval, wval;
	u32 cfg_off, data;
	u64 raddr, waddr;
	u8 shift;

	raddr = pcie->pem.start + PEM_CFG_RD;
	waddr = pcie->pem.start + PEM_CFG_WR;

	debug("%s raddr %llx waddr %llx\n", __func__, raddr, waddr);
		cfg_off = PCIERC_RASDP_DE_ME;
		wval = cfg_off;
	debug("%s DE_ME raddr %llx wval %llx\n", __func__, raddr, wval);
		writeq(wval, raddr);
		rval = readq(raddr);
	debug("%s DE_ME raddr %llx rval %llx\n", __func__, raddr, rval);
		data = rval >> 32;
		if (data & 0x1) {
			data = (data & (~0x1));
			wval |= ((u64)data << 32);
	debug("%s DE_ME waddr %llx wval %llx\n", __func__, waddr, wval);
			writeq(wval, waddr);
		}

		cfg_off = PCIERC_RAS_EINJ_CTL6_CMPP0;
		wval = cfg_off;
		data = 0xFE000000;
		wval |= ((u64)data << 32);
	debug("%s CMPP0 waddr %llx wval %llx\n", __func__, waddr, wval);
		writeq(wval, waddr);

		cfg_off = PCIERC_RAS_EINJ_CTL6_CMPV0;
		wval = cfg_off;
		data = 0x44000000;
		wval |= ((u64)data << 32);
	debug("%s CMPV0 waddr %llx wval %llx\n", __func__, waddr, wval);
		writeq(wval, waddr);

		cfg_off = PCIERC_RAS_EINJ_CTL6_CHGP1;
		wval = cfg_off;
		data = 0xFF;
		wval |= ((u64)data << 32);
	debug("%s CHGP1 waddr %llx wval %llx\n", __func__, waddr, wval);
		writeq(wval, waddr);

	cfg_off = PCIERC_RAS_EINJ_EN;
	wval = cfg_off;
	data = 0x40;
	wval |= ((u64)data << 32);
	debug("%s EINJ_EN waddr %llx wval %llx\n", __func__, waddr, wval);
	writeq(wval, waddr);

	cfg_off = PCIERC_RAS_EINJ_CTL6PE;
	wval = cfg_off;
	data = 0x1;
	wval |= ((u64)data << 32);
	debug("%s EINJ_CTL6PE waddr %llx wval %llx\n", __func__, waddr, wval);
	writeq(wval, waddr);

	switch (size) {
	case PCI_SIZE_8:
		shift = offset % 4;
		data = (0x1 << shift);
		break;
	case PCI_SIZE_16:
		shift = (offset % 4) ? 2 : 0;
		data = (0x3 << shift);
		break;
	default:
	case PCI_SIZE_32:
		data = 0xF;
		break;
	}

	cfg_off = PCIERC_RAS_EINJ_CTL6_CHGV1;
	wval = cfg_off;
	wval |= ((u64)data << 32);
	debug("%s EINJ_CHGV1 waddr %llx <= wval %llx\n", __func__, waddr, wval);
	writeq(wval, waddr);

	cfg_off = PCIERC_RASDP_EP_CTL;
	wval = cfg_off;
	wval |= ((u64)0x1 << 32);
	debug("%s EP_CTL waddr %llx <= wval %llx\n", __func__, waddr, wval);
	writeq(wval, waddr);

	wval = readq(waddr);
	debug("%s EP_CTL waddr %llx => wval %llx\n", __func__, waddr, wval);
}

static int pci_octeontx2_pem_write_config(struct udevice *bus, pci_dev_t bdf,
					  uint offset, ulong value,
					  enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address, addr;
	u32 b, d, f;
	u32 data;
	int tmp;

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12);

	debug("bdf %x %02x.%02x.%02x: u%d %x (%lx)\n",
	      bdf, b, d, f, size, offset, address);
	address += pcie->cfg.start;

	debug("%02x.%02x.%02x: u%d %x (%lx) %lx\n",
	      b, d, f, size, offset, address, value);

	if (b == 1 && d > 0)
		return 0;

	addr = (address + offset) & ~0x3UL;
	switch (size) {
	case PCI_SIZE_8:
		tmp = (address + offset) & 0x3;
		size = PCI_SIZE_32;
		data = readl(addr);
		debug("tmp 8 long %lx %x\n", addr, data);
		tmp *= 8;
		value = (data & ~(0xFFUL << tmp)) | ((value & 0xFF) << tmp);
	break;
	case PCI_SIZE_16:
		tmp = (address + offset) & 0x3;
		size = PCI_SIZE_32;
		data = readl(addr);
		debug("tmp 16 long %lx %x\n", addr, data);
		tmp *= 8;
		value = (data & 0xFFFF) | (value << tmp);
	break;
	case PCI_SIZE_32:
	break;
	}
	debug("tmp long %lx %lx\n", addr, value);

	if (use_workaround())
		pci_octeontx2_pem_workaround(bus, offset, size);

	switch (size) {
	case PCI_SIZE_8:
		debug("byte %lx %lx\n", address + offset, value);
		writeb(value, address + offset);
		break;
	case PCI_SIZE_16:
		debug("word %lx %lx\n", address + offset, value);
		writew(value, address + offset);
		break;
	case PCI_SIZE_32:
		debug("long %lx %lx\n", addr, value);
		writel(value, addr);
		break;
	default:
		printf("Invalid size\n");
	}

	debug("%02x.%02x.%02x: u%d %x (%lx) <- %lx\n",
	      b, d, f, size, offset, addr, value);

	return 0;
}
#endif

static int pci_octeontx_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int pci_octeontx_ecam_probe(struct udevice *dev)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(dev);
	int err;

	err = fdt_get_resource(gd->fdt_blob, dev->node.of_offset, "reg", 0,
			       &pcie->cfg);

	if (err) {
		printf("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

#if defined(CONFIG_ARCH_OCTEONTX2)
	err = fdt_node_check_compatible(gd->fdt_blob, dev->node.of_offset,
					"marvell,pci-host-octeontx2-pem");
	if (!err) {
		err = fdt_get_resource(gd->fdt_blob, dev->node.of_offset,
				       "reg", 1, &pcie->pem);

		if (err) {
			printf("Error reading resource: %s\n",
			       fdt_strerror(err));
			return err;
		}
	}
#endif
	err = fdtdec_get_pci_bus_range(gd->fdt_blob, dev->node.of_offset,
				       &pcie->bus);

	if (err) {
		printf("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pci_octeontx_ecam_ops = {
	.read_config	= pci_octeontx_ecam_read_config,
	.write_config	= pci_octeontx_ecam_write_config,
};

static const struct udevice_id pci_octeontx_ecam_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-ecam" },
	{ .compatible = "cavium,pci-host-octeontx-ecam" },
	{ .compatible = "pci-host-ecam-generic" },
	{ }
};

U_BOOT_DRIVER(pci_octeontx_ecam) = {
	.name	= "pci_octeontx_ecam",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx_ecam_ids,
	.ops	= &pci_octeontx_ecam_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct dm_pci_ops pci_octeontx_pem_ops = {
	.read_config	= pci_octeontx_pem_read_config,
	.write_config	= pci_octeontx_pem_write_config,
};

static const struct udevice_id pci_octeontx_pem_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-pem" },
	{ }
};

U_BOOT_DRIVER(pci_octeontx_pcie) = {
	.name	= "pci_octeontx_pem",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx_pem_ids,
	.ops	= &pci_octeontx_pem_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
};

#if defined(CONFIG_ARCH_OCTEONTX2)
static const struct dm_pci_ops pci_octeontx2_pem_ops = {
	.read_config	= pci_octeontx2_pem_read_config,
	.write_config	= pci_octeontx2_pem_write_config,
};

static const struct udevice_id pci_octeontx2_pem_ids[] = {
	{ .compatible = "marvell,pci-host-octeontx2-pem" },
	{ }
};

U_BOOT_DRIVER(pci_octeontx2_pcie) = {
	.name	= "pci_octeontx2_pem",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx2_pem_ids,
	.ops	= &pci_octeontx2_pem_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
};
#endif
