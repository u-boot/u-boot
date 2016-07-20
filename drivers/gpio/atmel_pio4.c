/*
 * Atmel PIO4 device driver
 *
 * Copyright (C) 2015 Atmel Corporation
 *		 Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <asm/arch/hardware.h>
#include <mach/gpio.h>
#include <mach/atmel_pio4.h>

static struct atmel_pio4_port *atmel_pio4_port_base(u32 port)
{
	struct atmel_pio4_port *base = NULL;

	switch (port) {
	case AT91_PIO_PORTA:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOA;
		break;
	case AT91_PIO_PORTB:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOB;
		break;
	case AT91_PIO_PORTC:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOC;
		break;
	case AT91_PIO_PORTD:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOD;
		break;
	default:
		printf("Error: Atmel PIO4: Failed to get PIO base of port#%d!\n",
		       port);
		break;
	}

	return base;
}

static int atmel_pio4_config_io_func(u32 port, u32 pin,
				     u32 func, u32 use_pullup)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -ENODEV;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -ENODEV;

	mask = 1 << pin;
	reg = func;
	reg |= use_pullup ? ATMEL_PIO_PUEN_MASK : 0;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	return 0;
}

int atmel_pio4_set_gpio(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_GPIO,
					 use_pullup);
}

int atmel_pio4_set_a_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_A,
					 use_pullup);
}

int atmel_pio4_set_b_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_B,
					 use_pullup);
}

int atmel_pio4_set_c_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_C,
					 use_pullup);
}

int atmel_pio4_set_d_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_D,
					 use_pullup);
}

int atmel_pio4_set_e_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_E,
					 use_pullup);
}

int atmel_pio4_set_f_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_F,
					 use_pullup);
}

int atmel_pio4_set_g_periph(u32 port, u32 pin, u32 use_pullup)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_G,
					 use_pullup);
}

int atmel_pio4_set_pio_output(u32 port, u32 pin, u32 value)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -ENODEV;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -ENODEV;

	mask = 0x01 << pin;
	reg = ATMEL_PIO_CFGR_FUNC_GPIO | ATMEL_PIO_DIR_MASK;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

int atmel_pio4_get_pio_input(u32 port, u32 pin)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -ENODEV;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -ENODEV;

	mask = 0x01 << pin;
	reg = ATMEL_PIO_CFGR_FUNC_GPIO;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	return (readl(&port_base->pdsr) & mask) ? 1 : 0;
}

#ifdef CONFIG_DM_GPIO
static int atmel_pio4_direction_input(struct udevice *dev, unsigned offset)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct atmel_pio4_port *port_base = (atmel_pio4_port *)plat->base_addr;
	u32 mask = 0x01 << offset;
	u32 reg = ATMEL_PIO_CFGR_FUNC_GPIO;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	return 0;
}

static int atmel_pio4_direction_output(struct udevice *dev,
				       unsigned offset, int value)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct atmel_pio4_port *port_base = (atmel_pio4_port *)plat->base_addr;
	u32 mask = 0x01 << offset;
	u32 reg = ATMEL_PIO_CFGR_FUNC_GPIO | ATMEL_PIO_DIR_MASK;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

static int atmel_pio4_get_value(struct udevice *dev, unsigned offset)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct atmel_pio4_port *port_base = (atmel_pio4_port *)plat->base_addr;
	u32 mask = 0x01 << offset;

	return (readl(&port_base->pdsr) & mask) ? 1 : 0;
}

static int atmel_pio4_set_value(struct udevice *dev,
				unsigned offset, int value)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct atmel_pio4_port *port_base = (atmel_pio4_port *)plat->base_addr;
	u32 mask = 0x01 << offset;

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

static int atmel_pio4_get_function(struct udevice *dev, unsigned offset)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct atmel_pio4_port *port_base = (atmel_pio4_port *)plat->base_addr;
	u32 mask = 0x01 << offset;

	writel(mask, &port_base->mskr);

	return (readl(&port_base->cfgr) &
		ATMEL_PIO_DIR_MASK) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops atmel_pio4_ops = {
	.direction_input	= atmel_pio4_direction_input,
	.direction_output	= atmel_pio4_direction_output,
	.get_value		= atmel_pio4_get_value,
	.set_value		= atmel_pio4_set_value,
	.get_function		= atmel_pio4_get_function,
};

static int atmel_pio4_probe(struct udevice *dev)
{
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = plat->bank_name;
	uc_priv->gpio_count = ATMEL_PIO_NPINS_PER_BANK;

	return 0;
}

U_BOOT_DRIVER(gpio_atmel_pio4) = {
	.name	= "gpio_atmel_pio4",
	.id	= UCLASS_GPIO,
	.ops	= &atmel_pio4_ops,
	.probe	= atmel_pio4_probe,
};
#endif
