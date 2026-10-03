/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Koninklijke Philips N.V.
 * Copyright (c) 2026 João Loureiro <joaofl@gmail.com>
 */

#ifndef __SPI_EEPROM
#define __SPI_EEPROM

#include <linux/errno.h>
#include <linux/types.h>

struct udevice;

/**
 * struct spi_eeprom_ops - operations provided by an SPI EEPROM driver
 *
 * @read:	Read @size bytes from @offset into @buf
 * @write:	Write @size bytes from @buf at @offset
 * @size:	Return the capacity of the device in bytes
 */
struct spi_eeprom_ops {
	int (*read)(struct udevice *dev, int offset, u8 *buf, int size);
	int (*write)(struct udevice *dev, int offset, const u8 *buf,
		     int size);
	int (*size)(struct udevice *dev);
};

/**
 * struct spi_eeprom - geometry of an SPI EEPROM device
 *
 * @pagesize:	The EEPROM's page size in bytes
 * @size:	The EEPROM's capacity in bytes
 * @addr_len:	Number of address bytes sent after the command opcode
 */
struct spi_eeprom {
	unsigned long pagesize;
	unsigned long size;
	u8 addr_len;
};

#if CONFIG_IS_ENABLED(SPI_EEPROM)
/**
 * spi_eeprom_read() - read bytes from an SPI EEPROM chip
 *
 * @dev:	Chip to read from
 * @offset:	Offset within chip to start reading
 * @buf:	Place to put data
 * @size:	Number of bytes to read
 * Return: 0 on success, -ve on failure
 */
int spi_eeprom_read(struct udevice *dev, int offset, u8 *buf, int size);

/**
 * spi_eeprom_write() - write bytes to an SPI EEPROM chip
 *
 * @dev:	Chip to write to
 * @offset:	Offset within chip to start writing
 * @buf:	Buffer containing data to write
 * @size:	Number of bytes to write
 * Return: 0 on success, -ve on failure
 */
int spi_eeprom_write(struct udevice *dev, int offset, const u8 *buf,
		     int size);

/**
 * spi_eeprom_size() - get size of SPI EEPROM chip
 *
 * @dev:	Chip to query
 * Return: +ve size in bytes on success, -ve on failure
 */
int spi_eeprom_size(struct udevice *dev);

#else /* !SPI_EEPROM */

static inline int spi_eeprom_read(struct udevice *dev, int offset, u8 *buf,
				  int size)
{
	return -ENOSYS;
}

static inline int spi_eeprom_write(struct udevice *dev, int offset,
				   const u8 *buf, int size)
{
	return -ENOSYS;
}

static inline int spi_eeprom_size(struct udevice *dev)
{
	return -ENOSYS;
}

#endif /* SPI_EEPROM */

#endif /* __SPI_EEPROM */
