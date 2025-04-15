// SPDX-License-Identifier: GPL-2.0
/*
 * GPIO based MDIO bitbang driver.
 *
 * Copyright 2024 Markus Gothe <markus.gothe@genexis.eu>
 *
 * This file is based on the Linux kernel drivers drivers/net/phy/mdio-gpio.c
 * and drivers/net/phy/mdio-bitbang.c which have the following copyrights:
 *
 * Copyright (c) 2008 CSE Semaphore Belgium.
 *  by Laurent Pinchart <laurentp@cse-semaphore.com>
 *
 * Copyright (C) 2008, Paulius Zaleckas <paulius.zaleckas@teltonika.lt>
 *
 * Author: Scott Wood <scottwood@freescale.com>
 * Copyright (c) 2007 Freescale Semiconductor
 *
 * Copyright (c) 2003 Intracom S.A.
 *  by Pantelis Antoniou <panto@intracom.gr>
 *
 * 2005 (c) MontaVista Software, Inc.
 * Vitaly Bordug <vbordug@ru.mvista.com>
 */

#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/mdio.h>

#define MDIO_READ 2
#define MDIO_WRITE 1

#define MDIO_C45 BIT(15)
#define MDIO_C45_ADDR (MDIO_C45 | 0)
#define MDIO_C45_READ (MDIO_C45 | 3)
#define MDIO_C45_WRITE (MDIO_C45 | 1)

/* Minimum MDC period is 400 ns, plus some margin for error.  MDIO_DELAY
 * is done twice per period.
 */
#define MDIO_DELAY 250

/* The PHY may take up to 300 ns to produce data, plus some margin
 * for error.
 */
#define MDIO_READ_DELAY 350

#define MDIO_GPIO_MDC	0
#define MDIO_GPIO_MDIO	1
#define MDIO_GPIO_MDO	2

struct mdio_gpio_priv {
	struct gpio_desc mdc, mdio, mdo;
};

static void mdio_dir(struct udevice *mdio_dev, int dir)
{
	struct mdio_gpio_priv *priv = dev_get_priv(mdio_dev);

	if (dm_gpio_is_valid(&priv->mdo)) {
		/* Separate output pin. Always set its value to high
		 * when changing direction. If direction is input,
		 * assume the pin serves as pull-up. If direction is
		 * output, the default value is high.
		 */
		dm_gpio_set_value(&priv->mdo, 1);
		return;
	}

	if (dir)
		dm_gpio_set_dir_flags(&priv->mdio, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	else
		dm_gpio_set_dir_flags(&priv->mdio, GPIOD_IS_IN);
}

static int mdio_get(struct udevice *mdio_dev)
{
	struct mdio_gpio_priv *priv = dev_get_priv(mdio_dev);

	return dm_gpio_get_value(&priv->mdio);
}

static void mdio_set(struct udevice *mdio_dev, int what)
{
	struct mdio_gpio_priv *priv = dev_get_priv(mdio_dev);

	if (dm_gpio_is_valid(&priv->mdo))
		dm_gpio_set_value(&priv->mdo, what);
	else
		dm_gpio_set_value(&priv->mdio, what);
}

static void mdc_set(struct udevice *mdio_dev, int what)
{
	struct mdio_gpio_priv *priv = dev_get_priv(mdio_dev);

	dm_gpio_set_value(&priv->mdc, what);
}

/* MDIO must already be configured as output. */
static void mdio_gpio_send_bit(struct udevice *mdio_dev, int val)
{
	mdio_set(mdio_dev, val);
	ndelay(MDIO_DELAY);
	mdc_set(mdio_dev, 1);
	ndelay(MDIO_DELAY);
	mdc_set(mdio_dev, 0);
}

/* MDIO must already be configured as input. */
static int mdio_gpio_get_bit(struct udevice *mdio_dev)
{
	ndelay(MDIO_DELAY);
	mdc_set(mdio_dev, 1);
	ndelay(MDIO_READ_DELAY);
	mdc_set(mdio_dev, 0);

	return mdio_get(mdio_dev);
}

/* MDIO must already be configured as output. */
static void mdio_gpio_send_num(struct udevice *mdio_dev, u16 val, int bits)
{
	int i;

	for (i = bits - 1; i >= 0; i--)
		mdio_gpio_send_bit(mdio_dev, (val >> i) & 1);
}

/* MDIO must already be configured as input. */
static u16 mdio_gpio_get_num(struct udevice *mdio_dev, int bits)
{
	int i;
	u16 ret = 0;

	for (i = bits - 1; i >= 0; i--) {
		ret <<= 1;
		ret |= mdio_gpio_get_bit(mdio_dev);
	}

	return ret;
}

/* Utility to send the preamble, address, and
 * register (common to read and write).
 */
static void mdio_gpio_cmd(struct udevice *mdio_dev, int op, u8 phy, u8 reg)
{
	int i;

	mdio_dir(mdio_dev, 1);

	/*
	 * Send a 32 bit preamble ('1's) with an extra '1' bit for good
	 * measure.  The IEEE spec says this is a PHY optional
	 * requirement.  The AMD 79C874 requires one after power up and
	 * one after a MII communications error.  This means that we are
	 * doing more preambles than we need, but it is safer and will be
	 * much more robust.
	 */
	for (i = 0; i < 32; i++)
		mdio_gpio_send_bit(mdio_dev, 1);

	/*
	 * Send the start bit (01) and the read opcode (10) or write (01).
	 * Clause 45 operation uses 00 for the start and 11, 10 for
	 * read/write.
	 */
	mdio_gpio_send_bit(mdio_dev, 0);
	if (op & MDIO_C45)
		mdio_gpio_send_bit(mdio_dev, 0);
	else
		mdio_gpio_send_bit(mdio_dev, 1);
	mdio_gpio_send_bit(mdio_dev, (op >> 1) & 1);
	mdio_gpio_send_bit(mdio_dev, (op >> 0) & 1);

	mdio_gpio_send_num(mdio_dev, phy, 5);
	mdio_gpio_send_num(mdio_dev, reg, 5);
}

/*
 * In clause 45 mode all commands are prefixed by MDIO_ADDR to specify the
 * lower 16 bits of the 21 bit address. This transfer is done identically to a
 * MDIO_WRITE except for a different code. To enable clause 45 mode or
 * MII_ADDR_C45 into the address. Theoretically clause 45 and normal devices
 * can exist on the same bus. Normal devices should ignore the MDIO_ADDR
 * phase.
 */
static int mdio_gpio_cmd_addr(struct udevice *mdio_dev, int phy, u32 dev_addr, u32 reg)
{
	mdio_gpio_cmd(mdio_dev, MDIO_C45_ADDR, phy, dev_addr);

	/* send the turnaround (10) */
	mdio_gpio_send_bit(mdio_dev, 1);
	mdio_gpio_send_bit(mdio_dev, 0);

	mdio_gpio_send_num(mdio_dev, reg, 16);

	mdio_dir(mdio_dev, 0);
	mdio_gpio_get_bit(mdio_dev);

	return dev_addr;
}

static int mdio_gpio_read(struct udevice *mdio_dev, int addr, int devad, int reg)
{
	int ret, i;

	if (devad != MDIO_DEVAD_NONE) {
		reg = mdio_gpio_cmd_addr(mdio_dev, addr, devad, reg);
		mdio_gpio_cmd(mdio_dev, MDIO_C45_READ, addr, reg);
	} else {
		mdio_gpio_cmd(mdio_dev, MDIO_READ, addr, reg);
	}

	mdio_dir(mdio_dev, 0);

	/* check the turnaround bit: the PHY should be driving it to zero.
	 */
	if (mdio_gpio_get_bit(mdio_dev) != 0) {
		/* PHY didn't drive TA low -- flush any bits it
		 * may be trying to send.
		 */
		for (i = 0; i < 32; i++)
			mdio_gpio_get_bit(mdio_dev);

		return 0xffff;
	}

	ret = mdio_gpio_get_num(mdio_dev, 16);
	mdio_gpio_get_bit(mdio_dev);

	return ret;
}

static int mdio_gpio_write(struct udevice *mdio_dev, int addr, int devad, int reg, u16 val)
{
	if (devad != MDIO_DEVAD_NONE) {
		reg = mdio_gpio_cmd_addr(mdio_dev, addr, devad, reg);
		mdio_gpio_cmd(mdio_dev, MDIO_C45_WRITE, addr, reg);
	} else {
		mdio_gpio_cmd(mdio_dev, MDIO_WRITE, addr, reg);
	}

	/* send the turnaround (10) */
	mdio_gpio_send_bit(mdio_dev, 1);
	mdio_gpio_send_bit(mdio_dev, 0);

	mdio_gpio_send_num(mdio_dev, val, 16);

	mdio_dir(mdio_dev, 0);
	mdio_gpio_get_bit(mdio_dev);

	return 0;
}

static const struct mdio_ops mdio_gpio_ops = {
	.read = mdio_gpio_read,
	.write = mdio_gpio_write,
	.reset = NULL,
};

/*
 * Name the device, we use the device tree node name.
 * This can be overwritten by MDIO class code if device-name property is
 * present.
 */
static int mdio_gpio_bind(struct udevice *mdio_dev)
{
	if (ofnode_valid(dev_ofnode(mdio_dev)))
		device_set_name(mdio_dev, ofnode_get_name(dev_ofnode(mdio_dev)));

	return 0;
}

static int mdio_gpio_probe(struct udevice *mdio_dev)
{
	struct mdio_gpio_priv *priv = dev_get_priv(mdio_dev);
	int ret = 0;

	ret = gpio_request_by_name(mdio_dev, "gpios", MDIO_GPIO_MDC, &priv->mdc, GPIOD_IS_OUT);
	if (ret)
		return ret;

	ret = gpio_request_by_name(mdio_dev, "gpios", MDIO_GPIO_MDIO, &priv->mdio, GPIOD_IS_IN);
	if (ret)
		return ret;

	ret = gpio_request_by_name(mdio_dev, "gpios", MDIO_GPIO_MDO, &priv->mdo, GPIOD_IS_OUT);
	if (ret && ret != -ENOENT)
		return ret;

	return 0;
}

static const struct udevice_id mdio_gpio_ids[] = {
	{ .compatible = "virtual,mdio-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_mdio) = {
	.name = "gpio_mdio",
	.id = UCLASS_MDIO,
	.of_match = mdio_gpio_ids,
	.bind = mdio_gpio_bind,
	.probe = mdio_gpio_probe,
	.ops = &mdio_gpio_ops,
	.plat_auto = sizeof(struct mdio_perdev_priv),
	.priv_auto = sizeof(struct mdio_gpio_priv),
};
