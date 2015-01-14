/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This is a GPIO driver for Intel ICH6 and later. The x86 GPIOs are accessed
 * through the PCI bus. Each PCI device has 256 bytes of configuration space,
 * consisting of a standard header and a device-specific set of registers. PCI
 * bus 0, device 31, function 0 gives us access to the chipset GPIOs (among
 * other things). Within the PCI configuration space, the GPIOBASE register
 * tells us where in the device's I/O region we can find more registers to
 * actually access the GPIOs.
 *
 * PCI bus/device/function 0:1f:0  => PCI config registers
 *   PCI config register "GPIOBASE"
 *     PCI I/O space + [GPIOBASE]  => start of GPIO registers
 *       GPIO registers => gpio pin function, direction, value
 *
 *
 * Danger Will Robinson! Bank 0 (GPIOs 0-31) seems to be fairly stable. Most
 * ICH versions have more, but the decoding the matrix that describes them is
 * absurdly complex and constantly changing. We'll provide Bank 1 and Bank 2,
 * but they will ONLY work for certain unspecified chipsets because the offset
 * from GPIOBASE changes randomly. Even then, many GPIOs are unimplemented or
 * reserved or subject to arcane restrictions.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pci.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/pci.h>

#define GPIO_PER_BANK	32

struct ich6_bank_priv {
	/* These are I/O addresses */
	uint16_t use_sel;
	uint16_t io_sel;
	uint16_t lvl;
};

/* TODO: Move this to device tree, or platform data */
void ich_gpio_set_gpio_map(const struct pch_gpio_map *map)
{
	gd->arch.gpio_map = map;
}

static int gpio_ich6_ofdata_to_platdata(struct udevice *dev)
{
	struct ich6_bank_platdata *plat = dev_get_platdata(dev);
	pci_dev_t pci_dev;			/* handle for 0:1f:0 */
	u8 tmpbyte;
	u16 tmpword;
	u32 tmplong;
	u16 gpiobase;
	int offset;

	/* Where should it be? */
	pci_dev = PCI_BDF(0, 0x1f, 0);

	/* Is the device present? */
	tmpword = pci_read_config16(pci_dev, PCI_VENDOR_ID);
	if (tmpword != PCI_VENDOR_ID_INTEL) {
		debug("%s: wrong VendorID\n", __func__);
		return -ENODEV;
	}

	tmpword = pci_read_config16(pci_dev, PCI_DEVICE_ID);
	debug("Found %04x:%04x\n", PCI_VENDOR_ID_INTEL, tmpword);
	/*
	 * We'd like to validate the Device ID too, but pretty much any
	 * value is either a) correct with slight differences, or b)
	 * correct but undocumented. We'll have to check a bunch of other
	 * things instead...
	 */

	/* I/O should already be enabled (it's a RO bit). */
	tmpword = pci_read_config16(pci_dev, PCI_COMMAND);
	if (!(tmpword & PCI_COMMAND_IO)) {
		debug("%s: device IO not enabled\n", __func__);
		return -ENODEV;
	}

	/* Header Type must be normal (bits 6-0 only; see spec.) */
	tmpbyte = pci_read_config8(pci_dev, PCI_HEADER_TYPE);
	if ((tmpbyte & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
		debug("%s: invalid Header type\n", __func__);
		return -ENODEV;
	}

	/* Base Class must be a bridge device */
	tmpbyte = pci_read_config8(pci_dev, PCI_CLASS_CODE);
	if (tmpbyte != PCI_CLASS_CODE_BRIDGE) {
		debug("%s: invalid class\n", __func__);
		return -ENODEV;
	}
	/* Sub Class must be ISA */
	tmpbyte = pci_read_config8(pci_dev, PCI_CLASS_SUB_CODE);
	if (tmpbyte != PCI_CLASS_SUB_CODE_BRIDGE_ISA) {
		debug("%s: invalid subclass\n", __func__);
		return -ENODEV;
	}

	/* Programming Interface must be 0x00 (no others exist) */
	tmpbyte = pci_read_config8(pci_dev, PCI_CLASS_PROG);
	if (tmpbyte != 0x00) {
		debug("%s: invalid interface type\n", __func__);
		return -ENODEV;
	}

	/*
	 * GPIOBASE moved to its current offset with ICH6, but prior to
	 * that it was unused (or undocumented). Check that it looks
	 * okay: not all ones or zeros.
	 *
	 * Note we don't need check bit0 here, because the Tunnel Creek
	 * GPIO base address register bit0 is reserved (read returns 0),
	 * while on the Ivybridge the bit0 is used to indicate it is an
	 * I/O space.
	 */
	tmplong = pci_read_config32(pci_dev, PCI_CFG_GPIOBASE);
	if (tmplong == 0x00000000 || tmplong == 0xffffffff) {
		debug("%s: unexpected GPIOBASE value\n", __func__);
		return -ENODEV;
	}

	/*
	 * Okay, I guess we're looking at the right device. The actual
	 * GPIO registers are in the PCI device's I/O space, starting
	 * at the offset that we just read. Bit 0 indicates that it's
	 * an I/O address, not a memory address, so mask that off.
	 */
	gpiobase = tmplong & 0xfffe;
	offset = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "reg", -1);
	if (offset == -1) {
		debug("%s: Invalid register offset %d\n", __func__, offset);
		return -EINVAL;
	}
	plat->base_addr = gpiobase + offset;
	plat->bank_name = fdt_getprop(gd->fdt_blob, dev->of_offset,
				      "bank-name", NULL);

	return 0;
}

static int ich6_gpio_probe(struct udevice *dev)
{
	struct ich6_bank_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct ich6_bank_priv *bank = dev_get_priv(dev);

	if (gd->arch.gpio_map) {
		setup_pch_gpios(plat->base_addr, gd->arch.gpio_map);
		gd->arch.gpio_map = NULL;
	}

	uc_priv->gpio_count = GPIO_PER_BANK;
	uc_priv->bank_name = plat->bank_name;
	bank->use_sel = plat->base_addr;
	bank->io_sel = plat->base_addr + 4;
	bank->lvl = plat->base_addr + 8;

	return 0;
}

static int ich6_gpio_request(struct udevice *dev, unsigned offset,
			     const char *label)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;

	/*
	 * Make sure that the GPIO pin we want isn't already in use for some
	 * built-in hardware function. We have to check this for every
	 * requested pin.
	 */
	tmplong = inl(bank->use_sel);
	if (!(tmplong & (1UL << offset))) {
		debug("%s: gpio %d is reserved for internal use\n", __func__,
		      offset);
		return -EPERM;
	}

	return 0;
}

static int ich6_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;

	tmplong = inl(bank->io_sel);
	tmplong |= (1UL << offset);
	outl(bank->io_sel, tmplong);
	return 0;
}

static int ich6_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;

	gpio_set_value(offset, value);

	tmplong = inl(bank->io_sel);
	tmplong &= ~(1UL << offset);
	outl(bank->io_sel, tmplong);
	return 0;
}

static int ich6_gpio_get_value(struct udevice *dev, unsigned offset)

{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;
	int r;

	tmplong = inl(bank->lvl);
	r = (tmplong & (1UL << offset)) ? 1 : 0;
	return r;
}

static int ich6_gpio_set_value(struct udevice *dev, unsigned offset,
			       int value)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 tmplong;

	tmplong = inl(bank->lvl);
	if (value)
		tmplong |= (1UL << offset);
	else
		tmplong &= ~(1UL << offset);
	outl(bank->lvl, tmplong);
	return 0;
}

static int ich6_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct ich6_bank_priv *bank = dev_get_priv(dev);
	u32 mask = 1UL << offset;

	if (!(inl(bank->use_sel) & mask))
		return GPIOF_FUNC;
	if (inl(bank->io_sel) & mask)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops gpio_ich6_ops = {
	.request		= ich6_gpio_request,
	.direction_input	= ich6_gpio_direction_input,
	.direction_output	= ich6_gpio_direction_output,
	.get_value		= ich6_gpio_get_value,
	.set_value		= ich6_gpio_set_value,
	.get_function		= ich6_gpio_get_function,
};

static const struct udevice_id intel_ich6_gpio_ids[] = {
	{ .compatible = "intel,ich6-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_ich6) = {
	.name	= "gpio_ich6",
	.id	= UCLASS_GPIO,
	.of_match = intel_ich6_gpio_ids,
	.ops	= &gpio_ich6_ops,
	.ofdata_to_platdata	= gpio_ich6_ofdata_to_platdata,
	.probe	= ich6_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct ich6_bank_priv),
	.platdata_auto_alloc_size = sizeof(struct ich6_bank_platdata),
};
