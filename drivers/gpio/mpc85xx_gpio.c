/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, six@gdsys.de
 *
 * based on arch/powerpc/include/asm/mpc85xx_gpio.h, which is
 *
 * Copyright 2010 eXMeritus, A Boeing Company
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

struct ccsr_gpio {
	u32	gpdir;
	u32	gpodr;
	u32	gpdat;
	u32	gpier;
	u32	gpimr;
	u32	gpicr;
};

struct mpc85xx_gpio_data {
	/* The bank's register base in memory */
	struct ccsr_gpio __iomem *base;
	/* The address of the registers; used to identify the bank */
	ulong addr;
	/* The GPIO count of the bank */
	uint gpio_count;
	/* The GPDAT register cannot be used to determine the value of output
	 * pins on MPC8572/MPC8536, so we shadow it and use the shadowed value
	 * for output pins */
	u32 dat_shadow;
};

inline u32 gpio_mask(unsigned gpio) {
	return (1U << (31 - (gpio)));
}

static inline u32 mpc85xx_gpio_get_val(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpdat) & mask;
}

static inline u32 mpc85xx_gpio_get_dir(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpdir) & mask;
}

static inline void mpc85xx_gpio_set_in(struct ccsr_gpio *base, u32 gpios)
{
	clrbits_be32(&base->gpdat, gpios);
	/* GPDIR register 0 -> input */
	clrbits_be32(&base->gpdir, gpios);
}

static inline void mpc85xx_gpio_set_low(struct ccsr_gpio *base, u32 gpios)
{
	clrbits_be32(&base->gpdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be32(&base->gpdir, gpios);
}

static inline void mpc85xx_gpio_set_high(struct ccsr_gpio *base, u32 gpios)
{
	setbits_be32(&base->gpdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be32(&base->gpdir, gpios);
}

static inline int mpc85xx_gpio_open_drain_val(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpodr) & mask;
}

static inline void mpc85xx_gpio_open_drain_on(struct ccsr_gpio *base, u32
					      gpios)
{
	/* GPODR register 1 -> open drain on */
	setbits_be32(&base->gpodr, gpios);
}

static inline void mpc85xx_gpio_open_drain_off(struct ccsr_gpio *base,
					       u32 gpios)
{
	/* GPODR register 0 -> open drain off (actively driven) */
	clrbits_be32(&base->gpodr, gpios);
}

static int mpc85xx_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);

	mpc85xx_gpio_set_in(data->base, gpio_mask(gpio));
	return 0;
}

static int mpc85xx_gpio_set_value(struct udevice *dev, unsigned gpio,
				  int value)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);

	if (value) {
		data->dat_shadow |= gpio_mask(gpio);
		mpc85xx_gpio_set_high(data->base, gpio_mask(gpio));
	} else {
		data->dat_shadow &= ~gpio_mask(gpio);
		mpc85xx_gpio_set_low(data->base, gpio_mask(gpio));
	}
	return 0;
}

static int mpc85xx_gpio_direction_output(struct udevice *dev, unsigned gpio,
					 int value)
{
	return mpc85xx_gpio_set_value(dev, gpio, value);
}

static int mpc85xx_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);

	if (!!mpc85xx_gpio_get_dir(data->base, gpio_mask(gpio))) {
		/* Output -> use shadowed value */
		return !!(data->dat_shadow & gpio_mask(gpio));
	} else {
		/* Input -> read value from GPDAT register */
		return !!mpc85xx_gpio_get_val(data->base, gpio_mask(gpio));
	}
}

static int mpc85xx_gpio_get_open_drain(struct udevice *dev, unsigned gpio)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);

	return !!mpc85xx_gpio_open_drain_val(data->base, gpio_mask(gpio));
}

static int mpc85xx_gpio_set_open_drain(struct udevice *dev, unsigned gpio,
				       int value)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);

	if (value) {
		mpc85xx_gpio_open_drain_on(data->base, gpio_mask(gpio));
	} else {
		mpc85xx_gpio_open_drain_off(data->base, gpio_mask(gpio));
	}
	return 0;
}

static int mpc85xx_gpio_get_function(struct udevice *dev, unsigned gpio)
{
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);
	int dir;

	dir = !!mpc85xx_gpio_get_dir(data->base, gpio_mask(gpio));
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int mpc85xx_gpio_ofdata_to_platdata(struct udevice *dev) {
	struct mpc85xx_gpio_plat *plat = dev_get_platdata(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = fdtdec_get_addr_size_auto_noparent(gd->fdt_blob,
			dev_of_offset(dev), "reg", 0, &size, false);

	plat->addr = addr;
	plat->size = size;
	plat->ngpios = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				      "ngpios", 32);

	return 0;
}
#endif

static int mpc85xx_gpio_platdata_to_priv(struct udevice *dev)
{
	struct mpc85xx_gpio_data *priv = dev_get_priv(dev);
	struct mpc85xx_gpio_plat *plat = dev_get_platdata(dev);
	unsigned long size = plat->size;

	if (size == 0)
		size = 0x100;

	priv->addr = plat->addr;
	priv->base = map_sysmem(CONFIG_SYS_IMMR + plat->addr, size);

	if (!priv->base)
		return -ENOMEM;

	priv->gpio_count = plat->ngpios;
	priv->dat_shadow = 0;

	return 0;
}

static int mpc85xx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mpc85xx_gpio_data *data = dev_get_priv(dev);
	char name[32], *str;

	mpc85xx_gpio_platdata_to_priv(dev);

	snprintf(name, sizeof(name), "MPC@%lx_", data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = data->gpio_count;

	return 0;
}

static const struct dm_gpio_ops gpio_mpc85xx_ops = {
	.direction_input	= mpc85xx_gpio_direction_input,
	.direction_output	= mpc85xx_gpio_direction_output,
	.get_value		= mpc85xx_gpio_get_value,
	.set_value		= mpc85xx_gpio_set_value,
	.get_open_drain		= mpc85xx_gpio_get_open_drain,
	.set_open_drain		= mpc85xx_gpio_set_open_drain,
	.get_function 		= mpc85xx_gpio_get_function,
};

static const struct udevice_id mpc85xx_gpio_ids[] = {
	{ .compatible = "fsl,pq3-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_mpc85xx) = {
	.name	= "gpio_mpc85xx",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mpc85xx_ops,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.ofdata_to_platdata = mpc85xx_gpio_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mpc85xx_gpio_plat),
	.of_match = mpc85xx_gpio_ids,
#endif
	.probe	= mpc85xx_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct mpc85xx_gpio_data),
};
