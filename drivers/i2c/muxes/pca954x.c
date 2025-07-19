// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 - 2016 Xilinx, Inc.
 * Copyright (C) 2017 National Instruments Corp
 * Written by Michal Simek
 */

#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>

#include <asm-generic/gpio.h>

enum pca_type {
	MAX7356,
	MAX7357,
	MAX7358,
	MAX7367,
	MAX7368,
	MAX7369,
	PCA9543,
	PCA9544,
	PCA9545,
	PCA9546,
	PCA9547,
	PCA9548,
	PCA9646,
	PCA9847,
};

struct chip_desc {
	u8 enable; /* Enable mask in ctl register (used for muxes only) */
	enum muxtype {
		pca954x_ismux = 0,
		pca954x_isswi,
	} muxtype;
	u32 width;
};

struct pca954x_priv {
	u32 addr; /* I2C mux address */
	u32 width; /* I2C mux width - number of busses */
	struct gpio_desc gpio_mux_reset;
};

static const struct chip_desc chips[] = {
	[MAX7356] = {
		.muxtype = pca954x_isswi,
		.width = 8,
	},
	[MAX7357] = {
		.muxtype = pca954x_isswi,
		.width = 8,
	},
	[MAX7358] = {
		.muxtype = pca954x_isswi,
		.width = 8,
	},
	[MAX7367] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
	[MAX7368] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
	[MAX7369] = {
		.enable = 0x4,
		.muxtype = pca954x_ismux,
		.width = 4,
	},
	[PCA9543] = {
		.muxtype = pca954x_isswi,
		.width = 2,
	},
	[PCA9544] = {
		.enable = 0x4,
		.muxtype = pca954x_ismux,
		.width = 4,
	},
	[PCA9545] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
	[PCA9546] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
	[PCA9547] = {
		.enable = 0x8,
		.muxtype = pca954x_ismux,
		.width = 8,
	},
	[PCA9548] = {
		.muxtype = pca954x_isswi,
		.width = 8,
	},
	[PCA9646] = {
		.muxtype = pca954x_isswi,
		.width = 4,
	},
	[PCA9847] = {
		.enable = 0x8,
		.muxtype = pca954x_ismux,
		.width = 8,
	},
};

static int pca954x_deselect(struct udevice *mux, struct udevice *bus,
			    uint channel)
{
	struct pca954x_priv *priv = dev_get_priv(mux);
	uchar byte = 0;

	return dm_i2c_write(mux, priv->addr, &byte, 1);
}

static int pca954x_select(struct udevice *mux, struct udevice *bus,
			  uint channel)
{
	struct pca954x_priv *priv = dev_get_priv(mux);
	const struct chip_desc *chip = &chips[dev_get_driver_data(mux)];
	uchar byte;

	if (chip->muxtype == pca954x_ismux)
		byte = channel | chip->enable;
	else
		byte = 1 << channel;

	return dm_i2c_write(mux, priv->addr, &byte, 1);
}

static const struct i2c_mux_ops pca954x_ops = {
	.select = pca954x_select,
	.deselect = pca954x_deselect,
};

static const struct udevice_id pca954x_ids[] = {
	{ .compatible = "maxim,max7356", .data = MAX7356 },
	{ .compatible = "maxim,max7357", .data = MAX7357 },
	{ .compatible = "maxim,max7358", .data = MAX7358 },
	{ .compatible = "maxim,max7367", .data = MAX7367 },
	{ .compatible = "maxim,max7368", .data = MAX7368 },
	{ .compatible = "maxim,max7369", .data = MAX7369 },
	{ .compatible = "nxp,pca9543", .data = PCA9543 },
	{ .compatible = "nxp,pca9544", .data = PCA9544 },
	{ .compatible = "nxp,pca9545", .data = PCA9545 },
	{ .compatible = "nxp,pca9546", .data = PCA9546 },
	{ .compatible = "nxp,pca9547", .data = PCA9547 },
	{ .compatible = "nxp,pca9548", .data = PCA9548 },
	{ .compatible = "nxp,pca9646", .data = PCA9646 },
	{ .compatible = "nxp,pca9847", .data = PCA9847 },
	{ }
};

static int pca954x_of_to_plat(struct udevice *dev)
{
	struct pca954x_priv *priv = dev_get_priv(dev);
	const struct chip_desc *chip = &chips[dev_get_driver_data(dev)];

	priv->addr = dev_read_u32_default(dev, "reg", 0);
	if (!priv->addr) {
		debug("MUX not found\n");
		return -ENODEV;
	}
	priv->width = chip->width;

	if (!priv->width) {
		debug("No I2C MUX width specified\n");
		return -EINVAL;
	}

	debug("Device %s at 0x%x with width %d\n",
	      dev->name, priv->addr, priv->width);

	return 0;
}

static int pca954x_probe(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(DM_GPIO)) {
		struct pca954x_priv *priv = dev_get_priv(dev);
		int err;

		err = gpio_request_by_name(dev, "reset-gpios", 0,
				&priv->gpio_mux_reset, GPIOD_IS_OUT);

		/* it's optional so only bail if we get a real error */
		if (err && (err != -ENOENT))
			return err;

		/* dm will take care of polarity */
		if (dm_gpio_is_valid(&priv->gpio_mux_reset))
			dm_gpio_set_value(&priv->gpio_mux_reset, 0);
	}

	return 0;
}

static int pca954x_remove(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(DM_GPIO)) {
		struct pca954x_priv *priv = dev_get_priv(dev);

		if (dm_gpio_is_valid(&priv->gpio_mux_reset))
			dm_gpio_free(dev, &priv->gpio_mux_reset);
	}

	return 0;
}

U_BOOT_DRIVER(pca954x) = {
	.name = "pca954x",
	.id = UCLASS_I2C_MUX,
	.of_match = pca954x_ids,
	.probe = pca954x_probe,
	.remove = pca954x_remove,
	.ops = &pca954x_ops,
	.of_to_plat = pca954x_of_to_plat,
	.priv_auto	= sizeof(struct pca954x_priv),
};
