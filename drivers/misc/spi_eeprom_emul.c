// SPDX-License-Identifier: GPL-2.0-only
/*
 * Sandbox emulation of an AT25-style SPI EEPROM.
 *
 * Copyright (c) 2024 Koninklijke Philips N.V.
 * Copyright (c) 2026 João Loureiro <joaofl@gmail.com>
 */

#define LOG_CATEGORY UCLASS_SPI_EMUL

#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <linux/err.h>

#include "spi_eeprom_priv.h"

#define SANDBOX_SPI_EEPROM_SIZE		2048

/**
 * struct sandbox_spi_eeprom - state of the emulated EEPROM
 *
 * @data:	Backing store for the memory array
 * @size:	Size of the memory array in bytes
 * @cmd:	Opcode of the command currently being processed
 * @cmd_len:	Number of command/address bytes consumed so far
 * @addr:	Byte address decoded from the command
 */
struct sandbox_spi_eeprom {
	u8 *data;
	uint size;
	u8 cmd;
	uint cmd_len;
	uint addr;
};

static int sandbox_spi_eeprom_xfer(struct udevice *dev, uint bitlen,
				   const void *dout, void *din, ulong flags)
{
	struct sandbox_spi_eeprom *priv = dev_get_priv(dev);
	const u8 *tx = dout;
	u8 *rx = din;
	uint bytes = bitlen / 8;
	uint i;

	if (bitlen % 8)
		return -EINVAL;

	/* A new transaction starts with the command/address phase */
	if (flags & SPI_XFER_BEGIN) {
		priv->cmd = 0;
		priv->cmd_len = 0;
		priv->addr = 0;
	}

	for (i = 0; i < bytes; i++) {
		if (tx) {
			/* Command phase: first byte is the opcode */
			if (priv->cmd_len == 0)
				priv->cmd = tx[i];
			else if (priv->cmd == AT25_CMD_READ_DATA)
				priv->addr = (priv->addr << 8) | tx[i];
			priv->cmd_len++;
		} else if (rx) {
			/* Data phase: serve the requested register/array */
			switch (priv->cmd) {
			case AT25_CMD_READ_STATUS:
				/* Ready, write disabled, not protected */
				rx[i] = 0x00;
				break;
			case AT25_CMD_READ_DATA:
				rx[i] = priv->addr < priv->size ?
					priv->data[priv->addr] : 0xff;
				priv->addr++;
				break;
			default:
				rx[i] = 0xff;
				break;
			}
		}
	}

	return 0;
}

static int sandbox_spi_eeprom_probe(struct udevice *dev)
{
	struct sandbox_spi_eeprom *priv = dev_get_priv(dev);
	uint i;

	priv->size = dev_read_u32_default(dev, "sandbox,size",
					  SANDBOX_SPI_EEPROM_SIZE);
	priv->data = calloc(1, priv->size);
	if (!priv->data)
		return -ENOMEM;

	/* Fill with a known pattern so reads can be verified */
	for (i = 0; i < priv->size; i++)
		priv->data[i] = i & 0xff;

	return 0;
}

static int sandbox_spi_eeprom_remove(struct udevice *dev)
{
	struct sandbox_spi_eeprom *priv = dev_get_priv(dev);

	free(priv->data);

	return 0;
}

static const struct dm_spi_emul_ops sandbox_spi_eeprom_ops = {
	.xfer	= sandbox_spi_eeprom_xfer,
};

static const struct udevice_id sandbox_spi_eeprom_ids[] = {
	{ .compatible = "sandbox,spi-eeprom" },
	{ }
};

U_BOOT_DRIVER(sandbox_spi_eeprom) = {
	.name		= "sandbox_spi_eeprom",
	.id		= UCLASS_SPI_EMUL,
	.of_match	= sandbox_spi_eeprom_ids,
	.probe		= sandbox_spi_eeprom_probe,
	.remove		= sandbox_spi_eeprom_remove,
	.priv_auto	= sizeof(struct sandbox_spi_eeprom),
	.ops		= &sandbox_spi_eeprom_ops,
};
