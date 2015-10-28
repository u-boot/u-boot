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

DECLARE_GLOBAL_DATA_PTR;

int spi_flash_read_dm(struct udevice *dev, u32 offset, size_t len, void *buf)
{
	return sf_get_ops(dev)->read(dev, offset, len, buf);
}

int spi_flash_write_dm(struct udevice *dev, u32 offset, size_t len,
		       const void *buf)
{
	return sf_get_ops(dev)->write(dev, offset, len, buf);
}

int spi_flash_erase_dm(struct udevice *dev, u32 offset, size_t len)
{
	return sf_get_ops(dev)->erase(dev, offset, len);
}

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

	return dev_get_uclass_priv(dev);
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
	char name[30], *str;
	int ret;

	snprintf(name, sizeof(name), "spi_flash@%d:%d", busnum, cs);
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

static int spi_flash_post_bind(struct udevice *dev)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct dm_spi_flash_ops *ops = sf_get_ops(dev);
	static int reloc_done = 0;

	if (!reloc_done) {
		if (ops->read)
			ops->read += gd->reloc_off;
		if (ops->write)
			ops->write += gd->reloc_off;
		if (ops->erase)
			ops->erase += gd->reloc_off;

		reloc_done++;
	}
#endif
	return 0;
}

UCLASS_DRIVER(spi_flash) = {
	.id		= UCLASS_SPI_FLASH,
	.name		= "spi_flash",
	.post_bind	= spi_flash_post_bind,
	.per_device_auto_alloc_size = sizeof(struct spi_flash),
};
