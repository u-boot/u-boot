/*
 * Common SPI flash Interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
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
 * @name:		Name of SPI flash
 * @dual_flash:	Indicates dual flash memories - dual stacked, parallel
 * @shift:		Flash shift useful in dual parallel
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:	Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @poll_cmd:		Poll cmd - for flash erase/program
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @read_cmd:		Read cmd - Array Fast, Extn read and quad read.
 * @write_cmd:		Write cmd - page and quad program.
 * @dummy_byte:	Dummy cycles for read operation.
 * @memory_map:	Address of read-only SPI flash access
 * @read:		Flash read ops: Read len bytes at offset into buf
 *			Supported cmds: Fast Array Read
 * @write:		Flash write ops: Write len bytes from buf into offset
 *			Supported cmds: Page Program
 * @erase:		Flash erase ops: Erase len bytes from offset
 *			Supported cmds: Sector erase 4K, 32K, 64K
 * return 0 - Success, 1 - Failure
 */
struct spi_flash {
#ifdef CONFIG_DM_SPI_FLASH
	struct spi_slave *spi;
	struct udevice *dev;
#else
	struct spi_slave *spi;
#endif
	const char *name;
	u8 dual_flash;
	u8 shift;

	u32 size;
	u32 page_size;
	u32 sector_size;
	u32 erase_size;
#ifdef CONFIG_SPI_FLASH_BAR
	u8 bank_read_cmd;
	u8 bank_write_cmd;
	u8 bank_curr;
#endif
	u8 poll_cmd;
	u8 erase_cmd;
	u8 read_cmd;
	u8 write_cmd;
	u8 dummy_byte;

	void *memory_map;
#ifndef CONFIG_DM_SPI_FLASH
	/*
	 * These are not strictly needed for driver model, but keep them here
	 * whilt the transition is in progress.
	 *
	 * Normally each driver would provide its own operations, but for
	 * SPI flash most chips use the same algorithms. One approach is
	 * to create a 'common' SPI flash device which knows how to talk
	 * to most devices, and then allow other drivers to be used instead
	 * if requird, perhaps with a way of scanning through the list to
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
int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
			   unsigned int max_hz, unsigned int spi_mode,
			   struct udevice **devp);

/* Compatibility function - this is the old U-Boot API */
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode);

/* Compatibility function - this is the old U-Boot API */
void spi_flash_free(struct spi_flash *flash);

int spi_flash_remove(struct udevice *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
	return sf_get_ops(flash->dev)->read(flash->dev, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	return sf_get_ops(flash->dev)->write(flash->dev, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
	return sf_get_ops(flash->dev)->erase(flash->dev, offset, len);
}

struct sandbox_state;

int sandbox_sf_bind_emul(struct sandbox_state *state, int busnum, int cs,
			 struct udevice *bus, int of_offset, const char *spec);

void sandbox_sf_unbind_emul(struct sandbox_state *state, int busnum, int cs);

#else
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

/**
 * Set up a new SPI flash from an fdt node
 *
 * @param blob		Device tree blob
 * @param slave_node	Pointer to this SPI slave node in the device tree
 * @param spi_node	Cached pointer to the SPI interface this node belongs
 *			to
 * @return 0 if ok, -1 on error
 */
struct spi_flash *spi_flash_probe_fdt(const void *blob, int slave_node,
				      int spi_node);

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

void spi_boot(void) __noreturn;
void spi_spl_load_image(uint32_t offs, unsigned int size, void *vdst);

#endif /* _SPI_FLASH_H_ */
