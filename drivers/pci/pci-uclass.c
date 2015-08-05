/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <inttypes.h>
#include <pci.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

struct pci_controller *pci_bus_to_hose(int busnum)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PCI, busnum, &bus);
	if (ret) {
		debug("%s: Cannot get bus %d: ret=%d\n", __func__, busnum, ret);
		return NULL;
	}
	return dev_get_uclass_priv(bus);
}

/**
 * pci_get_bus_max() - returns the bus number of the last active bus
 *
 * @return last bus number, or -1 if no active buses
 */
static int pci_get_bus_max(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret = -1;

	ret = uclass_get(UCLASS_PCI, &uc);
	uclass_foreach_dev(bus, uc) {
		if (bus->seq > ret)
			ret = bus->seq;
	}

	debug("%s: ret=%d\n", __func__, ret);

	return ret;
}

int pci_last_busno(void)
{
	struct pci_controller *hose;
	struct udevice *bus;
	struct uclass *uc;
	int ret;

	debug("pci_last_busno\n");
	ret = uclass_get(UCLASS_PCI, &uc);
	if (ret || list_empty(&uc->dev_head))
		return -1;

	/* Probe the last bus */
	bus = list_entry(uc->dev_head.prev, struct udevice, uclass_node);
	debug("bus = %p, %s\n", bus, bus->name);
	assert(bus);
	ret = device_probe(bus);
	if (ret)
		return ret;

	/* If that bus has bridges, we may have new buses now. Get the last */
	bus = list_entry(uc->dev_head.prev, struct udevice, uclass_node);
	hose = dev_get_uclass_priv(bus);
	debug("bus = %s, hose = %p\n", bus->name, hose);

	return hose->last_busno;
}

int pci_get_ff(enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 0xff;
	case PCI_SIZE_16:
		return 0xffff;
	default:
		return 0xffffffff;
	}
}

int pci_bus_find_devfn(struct udevice *bus, pci_dev_t find_devfn,
		       struct udevice **devp)
{
	struct udevice *dev;

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct pci_child_platdata *pplat;

		pplat = dev_get_parent_platdata(dev);
		if (pplat && pplat->devfn == find_devfn) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int pci_bus_find_bdf(pci_dev_t bdf, struct udevice **devp)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PCI, PCI_BUS(bdf), &bus);
	if (ret)
		return ret;
	return pci_bus_find_devfn(bus, PCI_MASK_BUS(bdf), devp);
}

static int pci_device_matches_ids(struct udevice *dev,
				  struct pci_device_id *ids)
{
	struct pci_child_platdata *pplat;
	int i;

	pplat = dev_get_parent_platdata(dev);
	if (!pplat)
		return -EINVAL;
	for (i = 0; ids[i].vendor != 0; i++) {
		if (pplat->vendor == ids[i].vendor &&
		    pplat->device == ids[i].device)
			return i;
	}

	return -EINVAL;
}

int pci_bus_find_devices(struct udevice *bus, struct pci_device_id *ids,
			 int *indexp, struct udevice **devp)
{
	struct udevice *dev;

	/* Scan all devices on this bus */
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		if (pci_device_matches_ids(dev, ids) >= 0) {
			if ((*indexp)-- <= 0) {
				*devp = dev;
				return 0;
			}
		}
	}

	return -ENODEV;
}

int pci_find_device_id(struct pci_device_id *ids, int index,
		       struct udevice **devp)
{
	struct udevice *bus;

	/* Scan all known buses */
	for (uclass_first_device(UCLASS_PCI, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		if (!pci_bus_find_devices(bus, ids, &index, devp))
			return 0;
	}
	*devp = NULL;

	return -ENODEV;
}

int pci_bus_write_config(struct udevice *bus, pci_dev_t bdf, int offset,
			 unsigned long value, enum pci_size_t size)
{
	struct dm_pci_ops *ops;

	ops = pci_get_ops(bus);
	if (!ops->write_config)
		return -ENOSYS;
	return ops->write_config(bus, bdf, offset, value, size);
}

int pci_write_config(pci_dev_t bdf, int offset, unsigned long value,
		     enum pci_size_t size)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PCI, PCI_BUS(bdf), &bus);
	if (ret)
		return ret;

	return pci_bus_write_config(bus, PCI_MASK_BUS(bdf), offset, value,
				    size);
}

int pci_write_config32(pci_dev_t bdf, int offset, u32 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_32);
}

int pci_write_config16(pci_dev_t bdf, int offset, u16 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_16);
}

int pci_write_config8(pci_dev_t bdf, int offset, u8 value)
{
	return pci_write_config(bdf, offset, value, PCI_SIZE_8);
}

int pci_bus_read_config(struct udevice *bus, pci_dev_t bdf, int offset,
			unsigned long *valuep, enum pci_size_t size)
{
	struct dm_pci_ops *ops;

	ops = pci_get_ops(bus);
	if (!ops->read_config)
		return -ENOSYS;
	return ops->read_config(bus, bdf, offset, valuep, size);
}

int pci_read_config(pci_dev_t bdf, int offset, unsigned long *valuep,
		    enum pci_size_t size)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PCI, PCI_BUS(bdf), &bus);
	if (ret)
		return ret;

	return pci_bus_read_config(bus, PCI_MASK_BUS(bdf), offset, valuep,
				   size);
}

int pci_read_config32(pci_dev_t bdf, int offset, u32 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_32);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int pci_read_config16(pci_dev_t bdf, int offset, u16 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_16);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int pci_read_config8(pci_dev_t bdf, int offset, u8 *valuep)
{
	unsigned long value;
	int ret;

	ret = pci_read_config(bdf, offset, &value, PCI_SIZE_8);
	if (ret)
		return ret;
	*valuep = value;

	return 0;
}

int pci_auto_config_devices(struct udevice *bus)
{
	struct pci_controller *hose = bus->uclass_priv;
	unsigned int sub_bus;
	struct udevice *dev;
	int ret;

	sub_bus = bus->seq;
	debug("%s: start\n", __func__);
	pciauto_config_init(hose);
	for (ret = device_find_first_child(bus, &dev);
	     !ret && dev;
	     ret = device_find_next_child(&dev)) {
		struct pci_child_platdata *pplat;

		pplat = dev_get_parent_platdata(dev);
		unsigned int max_bus;
		pci_dev_t bdf;

		bdf = PCI_ADD_BUS(bus->seq, pplat->devfn);
		debug("%s: device %s\n", __func__, dev->name);
		max_bus = pciauto_config_device(hose, bdf);
		sub_bus = max(sub_bus, max_bus);
	}
	debug("%s: done\n", __func__);

	return sub_bus;
}

int dm_pci_hose_probe_bus(struct pci_controller *hose, pci_dev_t bdf)
{
	struct udevice *parent, *bus;
	int sub_bus;
	int ret;

	debug("%s\n", __func__);
	parent = hose->bus;

	/* Find the bus within the parent */
	ret = pci_bus_find_devfn(parent, bdf, &bus);
	if (ret) {
		debug("%s: Cannot find device %x on bus %s: %d\n", __func__,
		      bdf, parent->name, ret);
		return ret;
	}

	sub_bus = pci_get_bus_max() + 1;
	debug("%s: bus = %d/%s\n", __func__, sub_bus, bus->name);
	pciauto_prescan_setup_bridge(hose, bdf, bus->seq);

	ret = device_probe(bus);
	if (ret) {
		debug("%s: Cannot probe bus bus %s: %d\n", __func__, bus->name,
		      ret);
		return ret;
	}
	if (sub_bus != bus->seq) {
		printf("%s: Internal error, bus '%s' got seq %d, expected %d\n",
		       __func__, bus->name, bus->seq, sub_bus);
		return -EPIPE;
	}
	sub_bus = pci_get_bus_max();
	pciauto_postscan_setup_bridge(hose, bdf, sub_bus);

	return sub_bus;
}

int pci_bind_bus_devices(struct udevice *bus)
{
	ulong vendor, device;
	ulong header_type;
	pci_dev_t devfn, end;
	bool found_multi;
	int ret;

	found_multi = false;
	end = PCI_DEVFN(PCI_MAX_PCI_DEVICES - 1, PCI_MAX_PCI_FUNCTIONS - 1);
	for (devfn = PCI_DEVFN(0, 0); devfn < end; devfn += PCI_DEVFN(0, 1)) {
		struct pci_child_platdata *pplat;
		struct udevice *dev;
		ulong class;

		if (PCI_FUNC(devfn) && !found_multi)
			continue;
		/* Check only the first access, we don't expect problems */
		ret = pci_bus_read_config(bus, devfn, PCI_HEADER_TYPE,
					  &header_type, PCI_SIZE_8);
		if (ret)
			goto error;
		pci_bus_read_config(bus, devfn, PCI_VENDOR_ID, &vendor,
				    PCI_SIZE_16);
		if (vendor == 0xffff || vendor == 0x0000)
			continue;

		if (!PCI_FUNC(devfn))
			found_multi = header_type & 0x80;

		debug("%s: bus %d/%s: found device %x, function %d\n", __func__,
		      bus->seq, bus->name, PCI_DEV(devfn), PCI_FUNC(devfn));
		pci_bus_read_config(bus, devfn, PCI_DEVICE_ID, &device,
				    PCI_SIZE_16);
		pci_bus_read_config(bus, devfn, PCI_CLASS_DEVICE, &class,
				    PCI_SIZE_16);

		/* Find this device in the device tree */
		ret = pci_bus_find_devfn(bus, devfn, &dev);

		/* If nothing in the device tree, bind a generic device */
		if (ret == -ENODEV) {
			char name[30], *str;
			const char *drv;

			sprintf(name, "pci_%x:%x.%x", bus->seq,
				PCI_DEV(devfn), PCI_FUNC(devfn));
			str = strdup(name);
			if (!str)
				return -ENOMEM;
			drv = class == PCI_CLASS_BRIDGE_PCI ?
				"pci_bridge_drv" : "pci_generic_drv";
			ret = device_bind_driver(bus, drv, str, &dev);
		}
		if (ret)
			return ret;

		/* Update the platform data */
		pplat = dev_get_parent_platdata(dev);
		pplat->devfn = devfn;
		pplat->vendor = vendor;
		pplat->device = device;
		pplat->class = class;
	}

	return 0;
error:
	printf("Cannot read bus configuration: %d\n", ret);

	return ret;
}

static int pci_uclass_post_bind(struct udevice *bus)
{
	/*
	 * Scan the device tree for devices. This does not probe the PCI bus,
	 * as this is not permitted while binding. It just finds devices
	 * mentioned in the device tree.
	 *
	 * Before relocation, only bind devices marked for pre-relocation
	 * use.
	 */
	return dm_scan_fdt_node(bus, gd->fdt_blob, bus->of_offset,
				gd->flags & GD_FLG_RELOC ? false : true);
}

static int decode_regions(struct pci_controller *hose, const void *blob,
			  int parent_node, int node)
{
	int pci_addr_cells, addr_cells, size_cells;
	int cells_per_record;
	const u32 *prop;
	int len;
	int i;

	prop = fdt_getprop(blob, node, "ranges", &len);
	if (!prop)
		return -EINVAL;
	pci_addr_cells = fdt_address_cells(blob, node);
	addr_cells = fdt_address_cells(blob, parent_node);
	size_cells = fdt_size_cells(blob, node);

	/* PCI addresses are always 3-cells */
	len /= sizeof(u32);
	cells_per_record = pci_addr_cells + addr_cells + size_cells;
	hose->region_count = 0;
	debug("%s: len=%d, cells_per_record=%d\n", __func__, len,
	      cells_per_record);
	for (i = 0; i < MAX_PCI_REGIONS; i++, len -= cells_per_record) {
		u64 pci_addr, addr, size;
		int space_code;
		u32 flags;
		int type;

		if (len < cells_per_record)
			break;
		flags = fdt32_to_cpu(prop[0]);
		space_code = (flags >> 24) & 3;
		pci_addr = fdtdec_get_number(prop + 1, 2);
		prop += pci_addr_cells;
		addr = fdtdec_get_number(prop, addr_cells);
		prop += addr_cells;
		size = fdtdec_get_number(prop, size_cells);
		prop += size_cells;
		debug("%s: region %d, pci_addr=%" PRIx64 ", addr=%" PRIx64
		      ", size=%" PRIx64 ", space_code=%d\n", __func__,
		      hose->region_count, pci_addr, addr, size, space_code);
		if (space_code & 2) {
			type = flags & (1U << 30) ? PCI_REGION_PREFETCH :
					PCI_REGION_MEM;
		} else if (space_code & 1) {
			type = PCI_REGION_IO;
		} else {
			continue;
		}
		debug(" - type=%d\n", type);
		pci_set_region(hose->regions + hose->region_count++, pci_addr,
			       addr, size, type);
	}

	/* Add a region for our local memory */
	pci_set_region(hose->regions + hose->region_count++, 0, 0,
		       gd->ram_size, PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	return 0;
}

static int pci_uclass_pre_probe(struct udevice *bus)
{
	struct pci_controller *hose;
	int ret;

	debug("%s, bus=%d/%s, parent=%s\n", __func__, bus->seq, bus->name,
	      bus->parent->name);
	hose = bus->uclass_priv;

	/* For bridges, use the top-level PCI controller */
	if (device_get_uclass_id(bus->parent) == UCLASS_ROOT) {
		hose->ctlr = bus;
		ret = decode_regions(hose, gd->fdt_blob, bus->parent->of_offset,
				bus->of_offset);
		if (ret) {
			debug("%s: Cannot decode regions\n", __func__);
			return ret;
		}
	} else {
		struct pci_controller *parent_hose;

		parent_hose = dev_get_uclass_priv(bus->parent);
		hose->ctlr = parent_hose->bus;
	}
	hose->bus = bus;
	hose->first_busno = bus->seq;
	hose->last_busno = bus->seq;

	return 0;
}

static int pci_uclass_post_probe(struct udevice *bus)
{
	int ret;

	/* Don't scan buses before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	debug("%s: probing bus %d\n", __func__, bus->seq);
	ret = pci_bind_bus_devices(bus);
	if (ret)
		return ret;

#ifdef CONFIG_PCI_PNP
	ret = pci_auto_config_devices(bus);
#endif

	return ret < 0 ? ret : 0;
}

static int pci_uclass_child_post_bind(struct udevice *dev)
{
	struct pci_child_platdata *pplat;
	struct fdt_pci_addr addr;
	int ret;

	if (dev->of_offset == -1)
		return 0;

	/*
	 * We could read vendor, device, class if available. But for now we
	 * just check the address.
	 */
	pplat = dev_get_parent_platdata(dev);
	ret = fdtdec_get_pci_addr(gd->fdt_blob, dev->of_offset,
				  FDT_PCI_SPACE_CONFIG, "reg", &addr);

	if (ret) {
		if (ret != -ENOENT)
			return -EINVAL;
	} else {
		/* extract the bdf from fdt_pci_addr */
		pplat->devfn = addr.phys_hi & 0xffff00;
	}

	return 0;
}

int pci_bridge_read_config(struct udevice *bus, pci_dev_t devfn, uint offset,
			   ulong *valuep, enum pci_size_t size)
{
	struct pci_controller *hose = bus->uclass_priv;
	pci_dev_t bdf = PCI_ADD_BUS(bus->seq, devfn);

	return pci_bus_read_config(hose->ctlr, bdf, offset, valuep, size);
}

int pci_bridge_write_config(struct udevice *bus, pci_dev_t devfn, uint offset,
			    ulong value, enum pci_size_t size)
{
	struct pci_controller *hose = bus->uclass_priv;
	pci_dev_t bdf = PCI_ADD_BUS(bus->seq, devfn);

	return pci_bus_write_config(hose->ctlr, bdf, offset, value, size);
}

UCLASS_DRIVER(pci) = {
	.id		= UCLASS_PCI,
	.name		= "pci",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= pci_uclass_post_bind,
	.pre_probe	= pci_uclass_pre_probe,
	.post_probe	= pci_uclass_post_probe,
	.child_post_bind = pci_uclass_child_post_bind,
	.per_device_auto_alloc_size = sizeof(struct pci_controller),
	.per_child_platdata_auto_alloc_size =
			sizeof(struct pci_child_platdata),
};

static const struct dm_pci_ops pci_bridge_ops = {
	.read_config	= pci_bridge_read_config,
	.write_config	= pci_bridge_write_config,
};

static const struct udevice_id pci_bridge_ids[] = {
	{ .compatible = "pci-bridge" },
	{ }
};

U_BOOT_DRIVER(pci_bridge_drv) = {
	.name		= "pci_bridge_drv",
	.id		= UCLASS_PCI,
	.of_match	= pci_bridge_ids,
	.ops		= &pci_bridge_ops,
};

UCLASS_DRIVER(pci_generic) = {
	.id		= UCLASS_PCI_GENERIC,
	.name		= "pci_generic",
};

static const struct udevice_id pci_generic_ids[] = {
	{ .compatible = "pci-generic" },
	{ }
};

U_BOOT_DRIVER(pci_generic_drv) = {
	.name		= "pci_generic_drv",
	.id		= UCLASS_PCI_GENERIC,
	.of_match	= pci_generic_ids,
};
