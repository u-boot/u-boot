/*
 * Xilinx AXI Bridge for PCI Express Driver
 *
 * Copyright (C) 2016 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <pci.h>

#include <asm/io.h>

/**
 * struct xilinx_pcie - Xilinx PCIe controller state
 * @hose: The parent classes PCI controller state
 * @cfg_base: The base address of memory mapped configuration space
 */
struct xilinx_pcie {
	struct pci_controller hose;
	void *cfg_base;
};

/* Register definitions */
#define XILINX_PCIE_REG_PSCR		0x144
#define XILINX_PCIE_REG_PSCR_LNKUP	BIT(11)

/**
 * pcie_xilinx_link_up() - Check whether the PCIe link is up
 * @pcie: Pointer to the PCI controller state
 *
 * Checks whether the PCIe link for the given device is up or down.
 *
 * Return: true if the link is up, else false
 */
static bool pcie_xilinx_link_up(struct xilinx_pcie *pcie)
{
	uint32_t pscr = __raw_readl(pcie->cfg_base + XILINX_PCIE_REG_PSCR);

	return pscr & XILINX_PCIE_REG_PSCR_LNKUP;
}

/**
 * pcie_xilinx_config_address() - Calculate the address of a config access
 * @pcie: Pointer to the PCI controller state
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @paddress: Pointer to the pointer to write the calculates address to
 *
 * Calculates the address that should be accessed to perform a PCIe
 * configuration space access for a given device identified by the PCIe
 * controller device @pcie and the bus, device & function numbers in @bdf. If
 * access to the device is not valid then the function will return an error
 * code. Otherwise the address to access will be written to the pointer pointed
 * to by @paddress.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_xilinx_config_address(struct xilinx_pcie *pcie, pci_dev_t bdf,
				      uint offset, void **paddress)
{
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int func = PCI_FUNC(bdf);
	void *addr;

	if ((bus > 0) && !pcie_xilinx_link_up(pcie))
		return -ENODEV;

	/*
	 * Busses 0 (host-PCIe bridge) & 1 (its immediate child) are
	 * limited to a single device each.
	 */
	if ((bus < 2) && (dev > 0))
		return -ENODEV;

	addr = pcie->cfg_base;
	addr += bus << 20;
	addr += dev << 15;
	addr += func << 12;
	addr += offset;
	*paddress = addr;

	return 0;
}

/**
 * pcie_xilinx_read_config() - Read from configuration space
 * @pcie: Pointer to the PCI controller state
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success, else -ENODEV or -EINVAL
 */
static int pcie_xilinx_read_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	struct xilinx_pcie *pcie = dev_get_priv(bus);
	void *address;
	int err;

	err = pcie_xilinx_config_address(pcie, bdf, offset, &address);
	if (err < 0) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	switch (size) {
	case PCI_SIZE_8:
		*valuep = __raw_readb(address);
		return 0;
	case PCI_SIZE_16:
		*valuep = __raw_readw(address);
		return 0;
	case PCI_SIZE_32:
		*valuep = __raw_readl(address);
		return 0;
	default:
		return -EINVAL;
	}
}

/**
 * pcie_xilinx_write_config() - Write to configuration space
 * @pcie: Pointer to the PCI controller state
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success, else -ENODEV or -EINVAL
 */
static int pcie_xilinx_write_config(struct udevice *bus, pci_dev_t bdf,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct xilinx_pcie *pcie = dev_get_priv(bus);
	void *address;
	int err;

	err = pcie_xilinx_config_address(pcie, bdf, offset, &address);
	if (err < 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		__raw_writeb(value, address);
		return 0;
	case PCI_SIZE_16:
		__raw_writew(value, address);
		return 0;
	case PCI_SIZE_32:
		__raw_writel(value, address);
		return 0;
	default:
		return -EINVAL;
	}
}

/**
 * pcie_xilinx_ofdata_to_platdata() - Translate from DT to device state
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_xilinx_ofdata_to_platdata(struct udevice *dev)
{
	struct xilinx_pcie *pcie = dev_get_priv(dev);
	struct fdt_resource reg_res;
	DECLARE_GLOBAL_DATA_PTR;
	int err;

	err = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev), "reg",
			       0, &reg_res);
	if (err < 0) {
		error("\"reg\" resource not found\n");
		return err;
	}

	pcie->cfg_base = map_physmem(reg_res.start,
				     fdt_resource_size(&reg_res),
				     MAP_NOCACHE);

	return 0;
}

static const struct dm_pci_ops pcie_xilinx_ops = {
	.read_config	= pcie_xilinx_read_config,
	.write_config	= pcie_xilinx_write_config,
};

static const struct udevice_id pcie_xilinx_ids[] = {
	{ .compatible = "xlnx,axi-pcie-host-1.00.a" },
	{ }
};

U_BOOT_DRIVER(pcie_xilinx) = {
	.name			= "pcie_xilinx",
	.id			= UCLASS_PCI,
	.of_match		= pcie_xilinx_ids,
	.ops			= &pcie_xilinx_ops,
	.ofdata_to_platdata	= pcie_xilinx_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct xilinx_pcie),
};
