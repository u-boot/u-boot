/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/pch.h>

int intel_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	return -ENOSYS;
}

int intel_i2c_probe_chip(struct udevice *bus, uint chip_addr, uint chip_flags)
{
	return -ENOSYS;
}

int intel_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	return 0;
}

static int intel_i2c_probe(struct udevice *dev)
{
	/*
	 * So far this is just setup code for ivybridge SMbus. When we have
	 * a full I2C driver this may need to be moved, generalised or made
	 * dependant on a particular compatible string.
	 *
	 * Set SMBus I/O base
	 */
	dm_pci_write_config32(dev, SMB_BASE,
			      SMBUS_IO_BASE | PCI_BASE_ADDRESS_SPACE_IO);

	/* Set SMBus enable. */
	dm_pci_write_config8(dev, HOSTC, HST_EN);

	/* Set SMBus I/O space enable. */
	dm_pci_write_config16(dev, PCI_COMMAND, PCI_COMMAND_IO);

	/* Disable interrupt generation. */
	outb(0, SMBUS_IO_BASE + SMBHSTCTL);

	/* Clear any lingering errors, so transactions can run. */
	outb(inb(SMBUS_IO_BASE + SMBHSTSTAT), SMBUS_IO_BASE + SMBHSTSTAT);
	debug("SMBus controller enabled\n");

	return 0;
}

static const struct dm_i2c_ops intel_i2c_ops = {
	.xfer		= intel_i2c_xfer,
	.probe_chip	= intel_i2c_probe_chip,
	.set_bus_speed	= intel_i2c_set_bus_speed,
};

static const struct udevice_id intel_i2c_ids[] = {
	{ .compatible = "intel,ich-i2c" },
	{ }
};

U_BOOT_DRIVER(intel_i2c) = {
	.name	= "i2c_intel",
	.id	= UCLASS_I2C,
	.of_match = intel_i2c_ids,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),
	.ops	= &intel_i2c_ops,
	.probe	= intel_i2c_probe,
};
