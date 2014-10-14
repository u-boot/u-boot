/*
 * Copyright (c) 2014 Google, Inc
 *
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * Influenced by code from:
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

struct soft_spi_platdata {
	struct fdt_gpio_state cs;
	struct fdt_gpio_state sclk;
	struct fdt_gpio_state mosi;
	struct fdt_gpio_state miso;
	int spi_delay_us;
};

struct soft_spi_priv {
	unsigned int mode;
};

static int soft_spi_scl(struct udevice *dev, int bit)
{
	struct soft_spi_platdata *plat = dev->platdata;
	struct soft_spi_priv *priv = dev_get_priv(dev);

	gpio_set_value(plat->sclk.gpio, priv->mode & SPI_CPOL ? bit : !bit);

	return 0;
}

static int soft_spi_sda(struct udevice *dev, int bit)
{
	struct soft_spi_platdata *plat = dev->platdata;

	gpio_set_value(plat->mosi.gpio, bit);

	return 0;
}

static int soft_spi_cs_activate(struct udevice *dev)
{
	struct soft_spi_platdata *plat = dev->platdata;
	struct soft_spi_priv *priv = dev_get_priv(dev);

	gpio_set_value(plat->cs.gpio, !(priv->mode & SPI_CS_HIGH));
	gpio_set_value(plat->sclk.gpio, priv->mode & SPI_CPOL);
	gpio_set_value(plat->cs.gpio, priv->mode & SPI_CS_HIGH);

	return 0;
}

static int soft_spi_cs_deactivate(struct udevice *dev)
{
	struct soft_spi_platdata *plat = dev->platdata;
	struct soft_spi_priv *priv = dev_get_priv(dev);

	gpio_set_value(plat->cs.gpio, !(priv->mode & SPI_CS_HIGH));

	return 0;
}

static int soft_spi_claim_bus(struct udevice *dev)
{
	/*
	 * Make sure the SPI clock is in idle state as defined for
	 * this slave.
	 */
	return soft_spi_scl(dev, 0);
}

static int soft_spi_release_bus(struct udevice *dev)
{
	/* Nothing to do */
	return 0;
}

/*-----------------------------------------------------------------------
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 */
static int soft_spi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct soft_spi_priv *priv = dev_get_priv(dev);
	struct soft_spi_platdata *plat = dev->platdata;
	uchar		tmpdin  = 0;
	uchar		tmpdout = 0;
	const u8	*txd = dout;
	u8		*rxd = din;
	int		cpol = priv->mode & SPI_CPOL;
	int		cpha = priv->mode & SPI_CPHA;
	unsigned int	j;

	debug("spi_xfer: slave %s:%s dout %08X din %08X bitlen %u\n",
	      dev->parent->name, dev->name, *(uint *)txd, *(uint *)rxd,
	      bitlen);

	if (flags & SPI_XFER_BEGIN)
		soft_spi_cs_activate(dev);

	for (j = 0; j < bitlen; j++) {
		/*
		 * Check if it is time to work on a new byte.
		 */
		if ((j % 8) == 0) {
			if (txd)
				tmpdout = *txd++;
			else
				tmpdout = 0;
			if (j != 0) {
				if (rxd)
					*rxd++ = tmpdin;
			}
			tmpdin  = 0;
		}

		if (!cpha)
			soft_spi_scl(dev, !cpol);
		soft_spi_sda(dev, tmpdout & 0x80);
		udelay(plat->spi_delay_us);
		if (cpha)
			soft_spi_scl(dev, !cpol);
		else
			soft_spi_scl(dev, cpol);
		tmpdin	<<= 1;
		tmpdin	|= gpio_get_value(plat->miso.gpio);
		tmpdout	<<= 1;
		udelay(plat->spi_delay_us);
		if (cpha)
			soft_spi_scl(dev, cpol);
	}
	/*
	 * If the number of bits isn't a multiple of 8, shift the last
	 * bits over to left-justify them.  Then store the last byte
	 * read in.
	 */
	if (rxd) {
		if ((bitlen % 8) != 0)
			tmpdin <<= 8 - (bitlen % 8);
		*rxd++ = tmpdin;
	}

	if (flags & SPI_XFER_END)
		soft_spi_cs_deactivate(dev);

	return 0;
}

static int soft_spi_set_speed(struct udevice *dev, unsigned int speed)
{
	/* Accept any speed */
	return 0;
}

static int soft_spi_set_mode(struct udevice *dev, unsigned int mode)
{
	struct soft_spi_priv *priv = dev_get_priv(dev);

	priv->mode = mode;

	return 0;
}

static int soft_spi_child_pre_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parentdata(dev);

	slave->dev = dev;
	return spi_ofdata_to_platdata(gd->fdt_blob, dev->of_offset, slave);
}

static const struct dm_spi_ops soft_spi_ops = {
	.claim_bus	= soft_spi_claim_bus,
	.release_bus	= soft_spi_release_bus,
	.xfer		= soft_spi_xfer,
	.set_speed	= soft_spi_set_speed,
	.set_mode	= soft_spi_set_mode,
};

static int soft_spi_ofdata_to_platdata(struct udevice *dev)
{
	struct soft_spi_platdata *plat = dev->platdata;
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;

	if (fdtdec_decode_gpio(blob, node, "cs-gpio", &plat->cs) ||
	    fdtdec_decode_gpio(blob, node, "sclk-gpio", &plat->sclk) ||
	    fdtdec_decode_gpio(blob, node, "mosi-gpio", &plat->mosi) ||
	    fdtdec_decode_gpio(blob, node, "miso-gpio", &plat->miso))
		return -EINVAL;
	plat->spi_delay_us = fdtdec_get_int(blob, node, "spi-delay-us", 0);

	return 0;
}

static int soft_spi_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parentdata(dev);
	struct soft_spi_platdata *plat = dev->platdata;

	gpio_request(plat->cs.gpio, "soft_spi_cs");
	gpio_request(plat->sclk.gpio, "soft_spi_sclk");
	gpio_request(plat->mosi.gpio, "soft_spi_mosi");
	gpio_request(plat->miso.gpio, "soft_spi_miso");

	gpio_direction_output(plat->sclk.gpio, slave->mode & SPI_CPOL);
	gpio_direction_output(plat->mosi.gpio, 1);
	gpio_direction_input(plat->miso.gpio);
	gpio_direction_output(plat->cs.gpio, !(slave->mode & SPI_CS_HIGH));

	return 0;
}

static const struct udevice_id soft_spi_ids[] = {
	{ .compatible = "u-boot,soft-spi" },
	{ }
};

U_BOOT_DRIVER(soft_spi) = {
	.name	= "soft_spi",
	.id	= UCLASS_SPI,
	.of_match = soft_spi_ids,
	.ops	= &soft_spi_ops,
	.ofdata_to_platdata = soft_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct soft_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct soft_spi_priv),
	.per_child_auto_alloc_size	= sizeof(struct spi_slave),
	.probe	= soft_spi_probe,
	.child_pre_probe	= soft_spi_child_pre_probe,
};
