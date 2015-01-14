/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <spi_flash.h>
#include <dm/device-internal.h>
#include "sf_internal.h"

/*
 * TODO(sjg@chromium.org): This is an old-style function. We should remove
 * it when all SPI flash drivers use dm
 */
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct udevice *dev;

	if (spi_flash_probe_bus_cs(bus, cs, max_hz, spi_mode, &dev))
		return NULL;

	return dev->uclass_priv;
}

void spi_flash_free(struct spi_flash *flash)
{
	spi_flash_remove(flash->spi->dev);
}

int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
			   unsigned int max_hz, unsigned int spi_mode,
			   struct udevice **devp)
{
	struct spi_slave *slave;
	struct udevice *bus;
	char name[20], *str;
	int ret;

	snprintf(name, sizeof(name), "%d:%d", busnum, cs);
	str = strdup(name);
	ret = spi_get_bus_and_cs(busnum, cs, max_hz, spi_mode,
				  "spi_flash_std", str, &bus, &slave);
	if (ret)
		return ret;

	*devp = slave->dev;
	return 0;
}

int spi_flash_remove(struct udevice *dev)
{
	return device_remove(dev);
}

UCLASS_DRIVER(spi_flash) = {
	.id		= UCLASS_SPI_FLASH,
	.name		= "spi_flash",
	.per_device_auto_alloc_size = sizeof(struct spi_flash),
};
