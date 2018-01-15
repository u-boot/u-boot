/*
 * Common SPI flash Interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include <dm.h>	/* Because we dereference struct udevice here */
#include <linux/types.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS		0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS		0
#endif

struct spi_slave;

/**
 * struct spi_flash - SPI flash structure
 *
 * @spi:		SPI slave
 * @dev:		SPI flash device
 * @name:		Name of SPI flash
 * @dual_flash:		Indicates dual flash memories - dual stacked, parallel
 * @shift:		Flash shift useful in dual parallel
 * @flags:		Indication of spi flash flags
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:		Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @read_cmd:		Read cmd - Array Fast, Extn read and quad read.
 * @write_cmd:		Write cmd - page and quad program.
 * @dummy_byte:		Dummy cycles for read operation.
 * @memory_map:		Address of read-only SPI flash access
 * @flash_lock:		lock a region of the SPI Flash
 * @flash_unlock:	unlock a region of the SPI Flash
 * @flash_is_locked:	check if a region of the SPI Flash is completely locked
 * @read:		Flash read ops: Read len bytes at offset into buf
 *			Supported cmds: Fast Array Read
 * @write:		Flash write ops: Write len bytes from buf into offset
 *			Supported cmds: Page Program
 * @erase:		Flash erase ops: Erase len bytes from offset
 *			Supported cmds: Sector erase 4K, 32K, 64K
 * return 0 - Success, 1 - Failure
 */
struct spi_flash {
	struct spi_slave *spi;
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *dev;
#endif
	const char *name;
	u8 dual_flash;
	u8 shift;
	u16 flags;

	u32 size;
	u32 page_size;
	u32 sector_size;
	u32 erase_size;
#ifdef CONFIG_SPI_FLASH_BAR
	u8 bank_read_cmd;
	u8 bank_write_cmd;
	u8 bank_curr;
#endif
	u8 erase_cmd;
	u8 read_cmd;
	u8 write_cmd;
	u8 dummy_byte;

	void *memory_map;

	int (*flash_lock)(struct spi_flash *flash, u32 ofs, size_t len);
	int (*flash_unlock)(struct spi_flash *flash, u32 ofs, size_t len);
	int (*flash_is_locked)(struct spi_flash *flash, u32 ofs, size_t len);
#ifndef CONFIG_DM_SPI_FLASH
	/*
	 * These are not strictly needed for driver model, but keep them here
	 * while the transition is in progress.
	 *
	 * Normally each driver would provide its own operations, but for
	 * SPI flash most chips use the same algorithms. One approach is
	 * to create a 'common' SPI flash device which knows how to talk
	 * to most devices, and then allow other drivers to be used instead
	 * if required, perhaps with a way of scanning through the list to
	 * find the driver that matches the device.
	 */
	int (*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int (*erase)(struct spi_flash *flash, u32 offset, size_t len);
#endif
};

struct dm_spi_flash_ops {
	int (*read)(struct udevice *dev, u32 offset, size_t len, void *buf);
	int (*write)(struct udevice *dev, u32 offset, size_t len,
		     const void *buf);
	int (*erase)(struct udevice *dev, u32 offset, size_t len);
};

/* Access the serial operations for a device */
#define sf_get_ops(dev) ((struct dm_spi_flash_ops *)(dev)->driver->ops)

#ifdef CONFIG_DM_SPI_FLASH
/**
 * spi_flash_read_dm() - Read data from SPI flash
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to read from
 * @len:	Number of bytes to read
 * @buf:	Buffer to put the data that is read
 * @return 0 if OK, -ve on error
 */
int spi_flash_read_dm(struct udevice *dev, u32 offset, size_t len, void *buf);

/**
 * spi_flash_write_dm() - Write data to SPI flash
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to write to
 * @len:	Number of bytes to write
 * @buf:	Buffer containing bytes to write
 * @return 0 if OK, -ve on error
 */
int spi_flash_write_dm(struct udevice *dev, u32 offset, size_t len,
		       const void *buf);

/**
 * spi_flash_erase_dm() - Erase blocks of the SPI flash
 *
 * Note that @len must be a muiltiple of the flash sector size.
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to start erasing
 * @len:	Number of bytes to erase
 * @return 0 if OK, -ve on error
 */
int spi_flash_erase_dm(struct udevice *dev, u32 offset, size_t len);

int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
			   unsigned int max_hz, unsigned int spi_mode,
			   struct udevice **devp);

/* Compatibility function - this is the old U-Boot API */
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode);

/* Compatibility function - this is the old U-Boot API */
void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
				 size_t len, void *buf)
{
	return spi_flash_read_dm(flash->dev, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
				  size_t len, const void *buf)
{
	return spi_flash_write_dm(flash->dev, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
				  size_t len)
{
	return spi_flash_erase_dm(flash->dev, offset, len);
}

struct sandbox_state;

int sandbox_sf_bind_emul(struct sandbox_state *state, int busnum, int cs,
			 struct udevice *bus, int of_offset, const char *spec);

void sandbox_sf_unbind_emul(struct sandbox_state *state, int busnum, int cs);

#else
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
	return flash->read(flash, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	return flash->write(flash, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
	return flash->erase(flash, offset, len);
}
#endif

static inline int spi_flash_protect(struct spi_flash *flash, u32 ofs, u32 len,
					bool prot)
{
	if (!flash->flash_lock || !flash->flash_unlock)
		return -EOPNOTSUPP;

	if (prot)
		return flash->flash_lock(flash, ofs, len);
	else
		return flash->flash_unlock(flash, ofs, len);
}

#endif /* _SPI_FLASH_H_ */
