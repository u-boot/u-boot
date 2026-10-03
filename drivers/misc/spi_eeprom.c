// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 Koninklijke Philips N.V.
 * Copyright (c) 2026 João Loureiro <joaofl@gmail.com>
 */

#define LOG_CATEGORY UCLASS_SPI_EEPROM

#include <dm.h>
#include <spi.h>
#include <spi_eeprom.h>
#include <linux/err.h>

#include "spi_eeprom_priv.h"

/* Longest address accepted by spi_eeprom_std_read(), in bytes */
#define SPI_EEPROM_MAX_ADDR_LEN		3

/**
 * struct at25_chip - geometry defaults for an AT25-style EEPROM
 *
 * The atmel,at25 binding (dts/upstream/Bindings/eeprom/at25.yaml) requires
 * "size", "pagesize" and "address-width" to be present in the device tree.
 * These values are used when a device tree omits them, and are all zero for
 * the generic "atmel,at25" compatible, where nothing can be assumed.
 *
 * @size:	Total capacity in bytes
 * @pagesize:	Write page size in bytes
 * @addr_len:	Number of address bytes sent after the command opcode
 */
struct at25_chip {
	u32 size;
	u32 pagesize;
	u8 addr_len;
};

int spi_eeprom_read(struct udevice *dev, int offset, u8 *buf, int size)
{
	const struct spi_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, offset, buf, size);
}

int spi_eeprom_write(struct udevice *dev, int offset, const u8 *buf, int size)
{
	const struct spi_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, offset, buf, size);
}

int spi_eeprom_size(struct udevice *dev)
{
	const struct spi_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->size)
		return -ENOSYS;

	return ops->size(dev);
}

static int spi_eeprom_read_cmd(struct udevice *dev, u8 *buf, int size,
			       const u8 *cmd, int cmd_size)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int ret;

	ret = dm_spi_claim_bus(dev);
	if (ret) {
		log_err("Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	/* Send the command, keeping the transaction open */
	ret = spi_xfer(slave, cmd_size * 8, cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		log_err("Failed to send command: %d\n", ret);
		goto release;
	}

	/* Read the data back and close the transaction */
	ret = spi_xfer(slave, size * 8, NULL, buf, SPI_XFER_END);
	if (ret)
		log_err("Failed to read data: %d\n", ret);

release:
	dm_spi_release_bus(dev);
	return ret;
}

/*
 * Read the status register into @status. Returns 0 on success or a negative
 * error code on failure.
 *
 * Status register layout (AT25-style devices):
 *   Bit 0    Ready/Busy: 1 while an internal write cycle is in progress
 *   Bit 1    Write Enable Latch
 *   Bits 2-3 Block write protection
 *   Bits 4-6 Reserved
 *   Bit 7    Write-protect enable
 */
static int spi_eeprom_read_status(struct udevice *dev, u8 *status)
{
	u8 cmd = AT25_CMD_READ_STATUS;

	return spi_eeprom_read_cmd(dev, status, 1, &cmd, 1);
}

static int spi_eeprom_std_read(struct udevice *dev, int offset, u8 *buf,
			       int size)
{
	struct spi_eeprom *priv = dev_get_priv(dev);
	u8 cmd[1 + SPI_EEPROM_MAX_ADDR_LEN];
	int i;

	if (offset < 0 || size < 0 || offset >= priv->size ||
	    size > priv->size - offset) {
		log_err("Read out of bounds (offset %d, size %d, max %lu)\n",
			offset, size, priv->size);
		return -EINVAL;
	}

	cmd[0] = AT25_CMD_READ_DATA;
	for (i = 0; i < priv->addr_len; i++)
		cmd[1 + i] = offset >> (8 * (priv->addr_len - 1 - i));

	return spi_eeprom_read_cmd(dev, buf, size, cmd, 1 + priv->addr_len);
}

static int spi_eeprom_std_size(struct udevice *dev)
{
	struct spi_eeprom *priv = dev_get_priv(dev);

	return priv->size;
}

static const struct spi_eeprom_ops spi_eeprom_std_ops = {
	.read	= spi_eeprom_std_read,
	.size	= spi_eeprom_std_size,
};

/* Geometry is described by the device tree for the generic compatible */
static const struct at25_chip at25_generic = { };

static const struct at25_chip mchp_25aa010a = {
	.size		= 128,
	.pagesize	= 16,
	.addr_len	= 1,
};

static const struct at25_chip mchp_at25160bn = {
	.size		= 2048,
	.pagesize	= 32,
	.addr_len	= 2,
};

static const struct at25_chip atmel_at25256b = {
	.size		= 32768,
	.pagesize	= 64,
	.addr_len	= 2,
};

static const struct at25_chip st_m95640 = {
	.size		= 8192,
	.pagesize	= 32,
	.addr_len	= 2,
};

static const struct at25_chip st_m95256 = {
	.size		= 32768,
	.pagesize	= 64,
	.addr_len	= 2,
};

static const struct at25_chip st_m95m02 = {
	.size		= 262144,
	.pagesize	= 256,
	.addr_len	= 3,
};

static int spi_eeprom_std_probe(struct udevice *dev)
{
	const struct at25_chip *chip =
		(const struct at25_chip *)dev_get_driver_data(dev);
	struct spi_eeprom *priv = dev_get_priv(dev);
	u32 addr_bits;
	u8 status;

	if (!chip)
		chip = &at25_generic;

	priv->size = dev_read_u32_default(dev, "size", chip->size);
	priv->pagesize = dev_read_u32_default(dev, "pagesize", chip->pagesize);
	addr_bits = dev_read_u32_default(dev, "address-width",
					 chip->addr_len * 8);

	if (!priv->size || !priv->pagesize || !addr_bits) {
		log_err("Missing 'size'/'pagesize'/'address-width', required by the atmel,at25 binding\n");
		return -EINVAL;
	}

	/*
	 * The binding also allows 9-bit addressing, where the ninth address
	 * bit travels in the opcode. That is not implemented here.
	 */
	switch (addr_bits) {
	case 8:
	case 16:
	case 24:
		priv->addr_len = addr_bits / 8;
		break;
	default:
		log_err("Unsupported address width %u\n", addr_bits);
		return -EOPNOTSUPP;
	}

	if (priv->size > 1UL << addr_bits) {
		log_err("Size %lu exceeds the %u-bit address range\n",
			priv->size, addr_bits);
		return -EINVAL;
	}

	/*
	 * Diagnostic only. SPI has no acknowledge, so this cannot tell whether
	 * a chip is present: an empty socket simply reads back 0x00 or 0xff.
	 * Do not fail probe() if the read itself goes wrong.
	 */
	if (!spi_eeprom_read_status(dev, &status))
		log_debug("status register: 0x%02x\n", status);

	return 0;
}

static const struct udevice_id spi_eeprom_std_ids[] = {
	{ .compatible = "microchip,25aa010a",
	  .data = (ulong)&mchp_25aa010a },
	{ .compatible = "microchip,at25160bn",
	  .data = (ulong)&mchp_at25160bn },
	{ .compatible = "atmel,at25256B",
	  .data = (ulong)&atmel_at25256b },
	{ .compatible = "st,m95640", .data = (ulong)&st_m95640 },
	{ .compatible = "st,m95256", .data = (ulong)&st_m95256 },
	{ .compatible = "st,m95m02", .data = (ulong)&st_m95m02 },
	/* Fallback compatible required by the binding; must be last */
	{ .compatible = "atmel,at25", .data = (ulong)&at25_generic },
	{ }
};

U_BOOT_DRIVER(spi_eeprom_std) = {
	.name		= "spi_eeprom",
	.id		= UCLASS_SPI_EEPROM,
	.of_match	= spi_eeprom_std_ids,
	.probe		= spi_eeprom_std_probe,
	.priv_auto	= sizeof(struct spi_eeprom),
	.ops		= &spi_eeprom_std_ops,
};

UCLASS_DRIVER(spi_eeprom) = {
	.id	= UCLASS_SPI_EEPROM,
	.name	= "spi_eeprom",
};
