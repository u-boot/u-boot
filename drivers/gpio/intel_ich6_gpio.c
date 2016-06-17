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
#include <pch.h>
#include <pci.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/pci.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_PER_BANK	32

struct ich6_bank_priv {
	/* These are I/O addresses */
	uint16_t use_sel;
	uint16_t io_sel;
	uint16_t lvl;
};

#define GPIO_USESEL_OFFSET(x)	(x)
#define GPIO_IOSEL_OFFSET(x)	(x + 4)
#define GPIO_LVL_OFFSET(x)	(x + 8)

#define IOPAD_MODE_MASK				0x7
#define IOPAD_PULL_ASSIGN_SHIFT		7
#define IOPAD_PULL_ASSIGN_MASK		(0x3 << IOPAD_PULL_ASSIGN_SHIFT)
#define IOPAD_PULL_STRENGTH_SHIFT	9
#define IOPAD_PULL_STRENGTH_MASK	(0x3 << IOPAD_PULL_STRENGTH_SHIFT)

/* TODO: Move this to device tree, or platform data */
void ich_gpio_set_gpio_map(const struct pch_gpio_map *map)
{
	gd->arch.gpio_map = map;
}

static int _ich6_gpio_set_value(uint16_t base, unsigned offset, int value)
{
	u32 val;

	val = inl(base);
	if (value)
		val |= (1UL << offset);
	else
		val &= ~(1UL << offset);
	outl(val, base);

	return 0;
}

static int _ich6_gpio_set_function(uint16_t base, unsigned offset, int func)
{
	u32 val;

	if (func) {
		val = inl(base);
		val |= (1UL << offset);
		outl(val, base);
	} else {
		val = inl(base);
		val &= ~(1UL << offset);
		outl(val, base);
	}

	return 0;
}

static int _ich6_gpio_set_direction(uint16_t base, unsigned offset, int dir)
{
	u32 val;

	if (!dir) {
		val = inl(base);
		val |= (1UL << offset);
		outl(val, base);
	} else {
		val = inl(base);
		val &= ~(1UL << offset);
		outl(val, base);
	}

	return 0;
}

static int _gpio_ich6_pinctrl_cfg_pin(s32 gpiobase, s32 iobase, int pin_node)
{
	u32 gpio_offset[2];
	int pad_offset;
	int val;
	int ret;
	const void *prop;

	/*
	 * GPIO node is not mandatory, so we only do the
	 * pinmuxing if the node exist.
	 */
	ret = fdtdec_get_int_array(gd->fdt_blob, pin_node, "gpio-offset",
			     gpio_offset, 2);
	if (!ret) {
		/* Do we want to force the GPIO mode? */
		prop = fdt_getprop(gd->fdt_blob, pin_node, "mode-gpio",
				      NULL);
		if (prop)
			_ich6_gpio_set_function(GPIO_USESEL_OFFSET
						(gpiobase) +
						gpio_offset[0],
						gpio_offset[1], 1);

		val =
		    fdtdec_get_int(gd->fdt_blob, pin_node, "direction", -1);
		if (val != -1)
			_ich6_gpio_set_direction(GPIO_IOSEL_OFFSET
						 (gpiobase) +
						 gpio_offset[0],
						 gpio_offset[1], val);

		val =
		    fdtdec_get_int(gd->fdt_blob, pin_node, "output-value", -1);
		if (val != -1)
			_ich6_gpio_set_value(GPIO_LVL_OFFSET(gpiobase)
					     + gpio_offset[0],
					     gpio_offset[1], val);
	}

	/* if iobase is present, let's configure the pad */
	if (iobase != -1) {
		int iobase_addr;

		/*
		 * The offset for the same pin for the IOBASE and GPIOBASE are
		 * different, so instead of maintaining a lookup table,
		 * the device tree should provide directly the correct
		 * value for both mapping.
		 */
		pad_offset =
		    fdtdec_get_int(gd->fdt_blob, pin_node, "pad-offset", -1);
		if (pad_offset == -1) {
			debug("%s: Invalid register io offset %d\n",
			      __func__, pad_offset);
			return -EINVAL;
		}

		/* compute the absolute pad address */
		iobase_addr = iobase + pad_offset;

		/*
		 * Do we need to set a specific function mode?
		 * If someone put also 'mode-gpio', this option will
		 * be just ignored by the controller
		 */
		val = fdtdec_get_int(gd->fdt_blob, pin_node, "mode-func", -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr, IOPAD_MODE_MASK, val);

		/* Configure the pull-up/down if needed */
		val = fdtdec_get_int(gd->fdt_blob, pin_node, "pull-assign", -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr,
					IOPAD_PULL_ASSIGN_MASK,
					val << IOPAD_PULL_ASSIGN_SHIFT);

		val =
		    fdtdec_get_int(gd->fdt_blob, pin_node, "pull-strength", -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr,
					IOPAD_PULL_STRENGTH_MASK,
					val << IOPAD_PULL_STRENGTH_SHIFT);

		debug("%s: pad cfg [0x%x]: %08x\n", __func__, pad_offset,
		      readl(iobase_addr));
	}

	return 0;
}

int gpio_ich6_pinctrl_init(void)
{
	struct udevice *pch;
	int pin_node;
	int node;
	int ret;
	u32 gpiobase;
	u32 iobase = -1;

	ret = uclass_first_device(UCLASS_PCH, &pch);
	if (ret)
		return ret;
	if (!pch)
		return -ENODEV;

	/*
	 * Get the memory/io base address to configure every pins.
	 * IOBASE is used to configure the mode/pads
	 * GPIOBASE is used to configure the direction and default value
	 */
	ret = pch_get_gpio_base(pch, &gpiobase);
	if (ret) {
		debug("%s: invalid GPIOBASE address (%08x)\n", __func__,
		      gpiobase);
		return -EINVAL;
	}

	/* This is not an error to not have a pinctrl node */
	node =
	    fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_INTEL_X86_PINCTRL);
	if (node <= 0) {
		debug("%s: no pinctrl node\n", __func__);
		return 0;
	}

	/*
	 * Get the IOBASE, this is not mandatory as this is not
	 * supported by all the CPU
	 */
	ret = pch_get_io_base(pch, &iobase);
	if (ret && ret != -ENOSYS) {
		debug("%s: invalid IOBASE address (%08x)\n", __func__,
		      iobase);
		return -EINVAL;
	}

	for (pin_node = fdt_first_subnode(gd->fdt_blob, node);
	     pin_node > 0;
	     pin_node = fdt_next_subnode(gd->fdt_blob, pin_node)) {
		/* Configure the pin */
		ret = _gpio_ich6_pinctrl_cfg_pin(gpiobase, iobase, pin_node);
		if (ret != 0) {
			debug("%s: invalid configuration for the pin %d\n",
			      __func__, pin_node);
			return ret;
		}
	}

	return 0;
}

static int gpio_ich6_ofdata_to_platdata(struct udevice *dev)
{
	struct ich6_bank_platdata *plat = dev_get_platdata(dev);
	u32 gpiobase;
	int offset;
	int ret;

	ret = pch_get_gpio_base(dev->parent, &gpiobase);
	if (ret)
		return ret;

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
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
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

	return _ich6_gpio_set_direction(bank->io_sel, offset, 0);
}

static int ich6_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	int ret;
	struct ich6_bank_priv *bank = dev_get_priv(dev);

	ret = _ich6_gpio_set_direction(bank->io_sel, offset, 1);
	if (ret)
		return ret;

	return _ich6_gpio_set_value(bank->lvl, offset, value);
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
	return _ich6_gpio_set_value(bank->lvl, offset, value);
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
