// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Alexander Graf <agraf@suse.de>
 *
 * Based on drivers/pinctrl/mvebu/pinctrl-mvebu.c and
 *          drivers/gpio/bcm2835_gpio.c
 *
 * This driver gets instantiated by the GPIO driver, because both devices
 * share the same device node.
 * https://spdx.org/licenses
 */

#include <config.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/pinctrl/bcm2835.h>
#include <linux/delay.h>

struct bcm283x_pinctrl_priv {
	u32 *base_reg;
};

#define MAX_PINS_PER_BANK 16

static void bcm2835_gpio_set_func_id(struct udevice *dev, unsigned int gpio,
				     int func)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	int reg_offset;
	int field_offset;

	reg_offset = BCM2835_GPIO_FSEL_BANK(gpio);
	field_offset = BCM2835_GPIO_FSEL_SHIFT(gpio);

	clrsetbits_le32(&priv->base_reg[reg_offset],
			BCM2835_GPIO_FSEL_MASK << field_offset,
			(func & BCM2835_GPIO_FSEL_MASK) << field_offset);
}

static int bcm2835_gpio_get_func_id(struct udevice *dev, unsigned int gpio)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(&priv->base_reg[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return (val >> BCM2835_GPIO_FSEL_SHIFT(gpio) & BCM2835_GPIO_FSEL_MASK);
}

/*
 * bcm2835_gpio_set_pull: Set GPIO pull-up/down resistor for BCM2835
 * @dev: the pinctrl device
 * @gpio: the GPIO pin number
 * @pull: pull setting (BCM2835_PUD_OFF, BCM2835_PUD_DOWN, BCM2835_PUD_UP)
 */
static void bcm2835_gpio_set_pull(struct udevice *dev, unsigned int gpio, int pull)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	u32 bank = BCM2835_GPPUDCLK0 + BCM2835_GPIO_COMMON_BANK(gpio);
	u32 bit = BCM2835_GPIO_COMMON_SHIFT(gpio);

	/* Set required control signal */
	writel(pull & 0x3, &priv->base_reg[BCM2835_GPPUD]);
	udelay(1);

	/* Clock the control signal into the GPIO pads */
	writel(1 << bit, &priv->base_reg[bank]);
	udelay(1);

	/* Remove the control signal and clock */
	writel(0, &priv->base_reg[BCM2835_GPPUD]);
	writel(0, &priv->base_reg[bank]);
}

/*
 * bcm2711_gpio_set_pull: Set GPIO pull-up/down resistor for BCM2711
 * @dev: the pinctrl device
 * @gpio: the GPIO pin number
 * @pull: pull setting (BCM2835_PUD_OFF, BCM2835_PUD_DOWN, BCM2835_PUD_UP)
 */
static void bcm2711_gpio_set_pull(struct udevice *dev, unsigned int gpio, int pull)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	u32 reg_offset;
	u32 bit_shift;
	u32 pull_bits;

	/* Findout which GPIO_PUP_PDN_CNTRL register to use */
	reg_offset = BCM2711_GPPUD_CNTRL_REG0 + BCM2711_PUD_REG_OFFSET(gpio);

	/* Findout the bit position */
	bit_shift = BCM2711_PUD_REG_SHIFT(gpio);

	/* Update the 2-bit field for this GPIO */
	pull_bits = pull & BCM2711_PUD_2711_MASK;
	clrsetbits_le32(&priv->base_reg[reg_offset],
			BCM2711_PUD_2711_MASK << bit_shift,
			pull_bits << bit_shift);
}

static void bcm283x_gpio_set_pull(struct udevice *dev, unsigned int gpio, int pull)
{
	if (device_is_compatible(dev, "brcm,bcm2835-gpio"))
		bcm2835_gpio_set_pull(dev, gpio, pull);
	else
		bcm2711_gpio_set_pull(dev, gpio, pull);
}

/*
 * bcm283x_pinctrl_set_state: configure pin functions and pull states.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
int bcm283x_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	u32 pin_arr[MAX_PINS_PER_BANK];
	u32 pull_arr[MAX_PINS_PER_BANK];
	int function;
	int i, len, pin_count = 0, pull_len = 0, pull_count = 0;
	int pull_value;

	if (!dev_read_prop(config, "brcm,pins", &len) || !len ||
	    len & 0x3 || dev_read_u32_array(config, "brcm,pins", pin_arr,
						  len / sizeof(u32))) {
		debug("Failed reading pins array for pinconfig %s (%d)\n",
		      config->name, len);
		return -EINVAL;
	}

	pin_count = len / sizeof(u32);

	function = dev_read_u32_default(config, "brcm,function", -1);
	if (function < 0) {
		debug("Failed reading function for pinconfig %s (%d)\n",
		      config->name, function);
		return -EINVAL;
	}

	/* Check if brcm,pull property exists */
	if (dev_read_prop(config, "brcm,pull", &pull_len) && pull_len > 0) {
		if (pull_len & 0x3) {
			debug("Invalid pull array length for pinconfig %s (%d)\n",
			      config->name, pull_len);
			return -EINVAL;
		}

		pull_count = pull_len / sizeof(u32);

		if (pull_count != 1 && pull_count != pin_count) {
			debug("Pull array count (%d) must be 1 or match pin count (%d) for pinconfig %s\n",
			      pull_count, pin_count, config->name);
			return -EINVAL;
		}

		if (dev_read_u32_array(config, "brcm,pull", pull_arr, pull_count)) {
			debug("Failed reading pull array for pinconfig %s\n", config->name);
			return -EINVAL;
		}

		/* Validate pull values */
		for (i = 0; i < pull_count; i++) {
			if (pull_arr[i] > 2) {
				debug("Invalid pull value %d for pin %d in pinconfig %s\n",
				      pull_arr[i], pin_arr[i], config->name);
				return -EINVAL;
			}
		}
	}

	for (i = 0; i < pin_count; i++) {
		bcm2835_gpio_set_func_id(dev, pin_arr[i], function);
		if (pull_count > 0) {
			pull_value = (pull_count == 1) ? pull_arr[0] : pull_arr[i];
			bcm283x_gpio_set_pull(dev, pin_arr[i], pull_value);
		}
	}

	return 0;
}

static int bcm283x_pinctrl_get_gpio_mux(struct udevice *dev, int banknum,
					int index)
{
	if (banknum != 0)
		return -EINVAL;

	return bcm2835_gpio_get_func_id(dev, index);
}

static const struct udevice_id bcm2835_pinctrl_id[] = {
	{.compatible = "brcm,bcm2835-gpio"},
	{.compatible = "brcm,bcm2711-gpio"},
	{}
};

int bcm283x_pinctl_of_to_plat(struct udevice *dev)
{
	struct bcm283x_pinctrl_priv *priv;

	priv = dev_get_priv(dev);

	priv->base_reg = dev_read_addr_ptr(dev);
	if (!priv->base_reg) {
		debug("%s: Failed to get base address\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int bcm283x_pinctl_probe(struct udevice *dev)
{
	int ret;
	struct udevice *pdev;

	/* Create GPIO device as well */
	ret = device_bind(dev, lists_driver_lookup_name("gpio_bcm2835"),
			  "gpio_bcm2835", NULL, dev_ofnode(dev), &pdev);
	if (ret) {
		/*
		 * While we really want the pinctrl driver to work to make
		 * devices go where they should go, the GPIO controller is
		 * not quite as crucial as it's only rarely used, so don't
		 * fail here.
		 */
		printf("Failed to bind GPIO driver\n");
	}

	return 0;
}

static struct pinctrl_ops bcm283x_pinctrl_ops = {
	.set_state	= bcm283x_pinctrl_set_state,
	.get_gpio_mux	= bcm283x_pinctrl_get_gpio_mux,
};

U_BOOT_DRIVER(pinctrl_bcm283x) = {
	.name		= "bcm283x_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= of_match_ptr(bcm2835_pinctrl_id),
	.of_to_plat = bcm283x_pinctl_of_to_plat,
	.priv_auto	= sizeof(struct bcm283x_pinctrl_priv),
	.ops		= &bcm283x_pinctrl_ops,
	.probe		= bcm283x_pinctl_probe,
#if IS_ENABLED(CONFIG_OF_BOARD)
	.flags		= DM_FLAG_PRE_RELOC,
#endif
};
