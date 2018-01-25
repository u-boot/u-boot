/*
 * Copyright (c) 2018 Alexander Graf <agraf@suse.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <dm/platform_data/serial_pl01x.h>
#include "serial_pl01x_internal.h"

/*
 * Check if this serial device is muxed
 *
 * The serial device will only work properly if it has been muxed to the serial
 * pins by firmware. Check whether that happened here.
 *
 * @return true if serial device is muxed, false if not
 */
static bool bcm283x_is_serial_muxed(void)
{
	int serial_gpio = 15;
	struct udevice *dev;

	if (uclass_first_device(UCLASS_PINCTRL, &dev) || !dev)
		return false;

	if (pinctrl_get_gpio_mux(dev, 0, serial_gpio) != BCM2835_GPIO_ALT0)
		return false;

	return true;
}

static int bcm283x_pl011_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct pl01x_serial_platdata *plat = dev_get_platdata(dev);
	int ret;

	/* Don't spawn the device if it's not muxed */
	if (!bcm283x_is_serial_muxed())
		return -ENODEV;

	ret = pl01x_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;

	/*
	 * TODO: Reinitialization doesn't always work for now, just skip
	 *       init always - we know we're already initialized
	 */
	plat->skip_init = true;

	return 0;
}

static const struct udevice_id bcm283x_pl011_serial_id[] = {
	{.compatible = "brcm,bcm2835-pl011", .data = TYPE_PL011},
	{}
};

U_BOOT_DRIVER(bcm283x_pl011_uart) = {
	.name	= "bcm283x_pl011",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(bcm283x_pl011_serial_id),
	.ofdata_to_platdata = of_match_ptr(bcm283x_pl011_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct pl01x_serial_platdata),
	.probe	= pl01x_serial_probe,
	.ops	= &pl01x_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct pl01x_priv),
};
