/*
 * Xilinx Zynq GPIO device driver
 *
 * Copyright (C) 2015 DAVE Embedded Systems <devel@dave.eu>
 *
 * Most of code taken from linux kernel driver (linux/drivers/gpio/gpio-zynq.c)
 * Copyright (C) 2009 - 2014 Xilinx, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <dm.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct zynq_gpio_privdata {
	phys_addr_t base;
};

/**
 * zynq_gpio_get_bank_pin - Get the bank number and pin number within that bank
 * for a given pin in the GPIO device
 * @pin_num:	gpio pin number within the device
 * @bank_num:	an output parameter used to return the bank number of the gpio
 *		pin
 * @bank_pin_num: an output parameter used to return pin number within a bank
 *		  for the given gpio pin
 *
 * Returns the bank number and pin offset within the bank.
 */
static inline void zynq_gpio_get_bank_pin(unsigned int pin_num,
					  unsigned int *bank_num,
					  unsigned int *bank_pin_num)
{
	switch (pin_num) {
	case ZYNQ_GPIO_BANK0_PIN_MIN ... ZYNQ_GPIO_BANK0_PIN_MAX:
		*bank_num = 0;
		*bank_pin_num = pin_num;
		break;
	case ZYNQ_GPIO_BANK1_PIN_MIN ... ZYNQ_GPIO_BANK1_PIN_MAX:
		*bank_num = 1;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK1_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK2_PIN_MIN ... ZYNQ_GPIO_BANK2_PIN_MAX:
		*bank_num = 2;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK2_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK3_PIN_MIN ... ZYNQ_GPIO_BANK3_PIN_MAX:
		*bank_num = 3;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK3_PIN_MIN;
		break;
	default:
		printf("invalid GPIO pin number: %u\n", pin_num);
		*bank_num = 0;
		*bank_pin_num = 0;
		break;
	}
}

static int gpio_is_valid(unsigned gpio)
{
	return (gpio >= 0) && (gpio < ZYNQ_GPIO_NR_GPIOS);
}

static int check_gpio(unsigned gpio)
{
	if (!gpio_is_valid(gpio)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

static int zynq_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	u32 data;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_privdata *priv = dev_get_priv(dev);

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	data = readl(priv->base +
			     ZYNQ_GPIO_DATA_RO_OFFSET(bank_num));

	return (data >> bank_pin_num) & 1;
}

static int zynq_gpio_set_value(struct udevice *dev, unsigned gpio, int value)
{
	unsigned int reg_offset, bank_num, bank_pin_num;
	struct zynq_gpio_privdata *priv = dev_get_priv(dev);

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	if (bank_pin_num >= ZYNQ_GPIO_MID_PIN_NUM) {
		/* only 16 data bits in bit maskable reg */
		bank_pin_num -= ZYNQ_GPIO_MID_PIN_NUM;
		reg_offset = ZYNQ_GPIO_DATA_MSW_OFFSET(bank_num);
	} else {
		reg_offset = ZYNQ_GPIO_DATA_LSW_OFFSET(bank_num);
	}

	/*
	 * get the 32 bit value to be written to the mask/data register where
	 * the upper 16 bits is the mask and lower 16 bits is the data
	 */
	value = !!value;
	value = ~(1 << (bank_pin_num + ZYNQ_GPIO_MID_PIN_NUM)) &
		((value << bank_pin_num) | ZYNQ_GPIO_UPPER_MASK);

	writel(value, priv->base + reg_offset);

	return 0;
}

static int zynq_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_privdata *priv = dev_get_priv(dev);

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	/* bank 0 pins 7 and 8 are special and cannot be used as inputs */
	if (bank_num == 0 && (bank_pin_num == 7 || bank_pin_num == 8))
		return -1;

	/* clear the bit in direction mode reg to set the pin as input */
	reg = readl(priv->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg &= ~BIT(bank_pin_num);
	writel(reg, priv->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	return 0;
}

static int zynq_gpio_direction_output(struct udevice *dev, unsigned gpio,
				      int value)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_privdata *priv = dev_get_priv(dev);

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	/* set the GPIO pin as output */
	reg = readl(priv->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, priv->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	/* configure the output enable reg for the pin */
	reg = readl(priv->base + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, priv->base + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));

	/* set the state of the pin */
	gpio_set_value(gpio, value);
	return 0;
}

static const struct dm_gpio_ops gpio_zynq_ops = {
	.direction_input	= zynq_gpio_direction_input,
	.direction_output	= zynq_gpio_direction_output,
	.get_value		= zynq_gpio_get_value,
	.set_value		= zynq_gpio_set_value,
};

static int zynq_gpio_probe(struct udevice *dev)
{
	struct zynq_gpio_privdata *priv = dev_get_priv(dev);

	priv->base = dev_get_addr(dev);

	return 0;
}

static int zynq_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = ZYNQ_GPIO_NR_GPIOS;

	return 0;
}

static const struct udevice_id zynq_gpio_ids[] = {
	{ .compatible = "xlnx,zynq-gpio-1.0" },
	{ }
};

U_BOOT_DRIVER(gpio_zynq) = {
	.name	= "gpio_zynq",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_zynq_ops,
	.of_match = zynq_gpio_ids,
	.ofdata_to_platdata = zynq_gpio_ofdata_to_platdata,
	.probe	= zynq_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct zynq_gpio_privdata),
};
