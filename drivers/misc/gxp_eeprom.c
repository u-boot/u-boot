// SPDX-License-Identifier: GPL-2.0+
/*
 * HPE GXP EEPROM driver
 *
 * Copyright (C) 2026 9elements GmbH
 */

#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/device_compat.h>

#define GXP_EEPROM_BLOCK_SIZE		128

struct gxp_eeprom_priv {
	void __iomem *base;
	size_t size;
};

static bool gxp_eeprom_validate_checksum(void __iomem *block)
{
	u8 version;
	u16 sum = 0;
	int i;

	version = readb(block);
	if (version != 2 && version != 3)
		return false;

	for (i = 0; i < GXP_EEPROM_BLOCK_SIZE / sizeof(u16); i++)
		sum += readw(block + i * sizeof(u16));

	return sum == 0;
}

static int gxp_eeprom_read(struct udevice *dev, int offset, void *buf,
			   int size)
{
	struct gxp_eeprom_priv *priv = dev_get_priv(dev);
	u8 *dst = buf;
	int i;

	if (offset < 0 || size <= 0 || offset + size > priv->size)
		return -EINVAL;

	for (i = 0; i < size; i++)
		dst[i] = readb(priv->base + offset + i);

	return size;
}

static int gxp_eeprom_probe(struct udevice *dev)
{
	struct gxp_eeprom_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;
	int i, num_blocks;

	addr = dev_read_addr_size(dev, &size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get address\n");
		return -EINVAL;
	}

	if (size == 0 || size % GXP_EEPROM_BLOCK_SIZE != 0) {
		dev_err(dev, "size must be a multiple of %d bytes\n",
			GXP_EEPROM_BLOCK_SIZE);
		return -EINVAL;
	}

	priv->base = map_physmem(addr, size, MAP_NOCACHE);
	if (!priv->base) {
		dev_err(dev, "failed to map memory\n");
		return -ENOMEM;
	}
	priv->size = size;

	num_blocks = size / GXP_EEPROM_BLOCK_SIZE;
	for (i = 0; i < num_blocks; i++) {
		if (!gxp_eeprom_validate_checksum(priv->base +
					       i * GXP_EEPROM_BLOCK_SIZE)) {
			dev_err(dev, "checksum failed for block %d\n", i);
			return -EBADMSG;
		}
	}

	return 0;
}

static const struct misc_ops gxp_eeprom_ops = {
	.read = gxp_eeprom_read,
};

static const struct udevice_id gxp_eeprom_ids[] = {
	{ .compatible = "hpe,gxp-eeprom" },
	{ }
};

U_BOOT_DRIVER(gxp_eeprom) = {
	.name		= "gxp_eeprom",
	.id		= UCLASS_MISC,
	.of_match	= gxp_eeprom_ids,
	.probe		= gxp_eeprom_probe,
	.ops		= &gxp_eeprom_ops,
	.priv_auto	= sizeof(struct gxp_eeprom_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
