/*
 * Interface to SPI flash
 *
 * Copyright (C) 2008 Atmel Corporation
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

#include <spi.h>
#include <linux/types.h>
#include <linux/compiler.h>

/* SECT flags */
#define SECT_4K		(1 << 1)
#define SECT_32K		(1 << 2)
#define E_FSR			(1 << 3)

/* SST specific macros */
#ifdef CONFIG_SPI_FLASH_SST
# define SST_WP			0x01	/* Supports AAI word program */
# define CMD_SST_BP			0x02    /* Byte Program */
# define CMD_SST_AAI_WP		0xAD	/* Auto Address Incr Word Program */
#endif

/**
 * struct spi_flash - SPI flash structure
 *
 * @spi:		SPI slave
 * @name:		Name of SPI flash
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:	Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @poll_cmd:		Poll cmd - for flash erase/program
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @memory_map:	Address of read-only SPI flash access
 * @read:		Flash read ops: Read len bytes at offset into buf
 *			Supported cmds: Fast Array Read
 * @write:		Flash write ops: Write len bytes from buf into offeset
 *			Supported cmds: Page Program
 * @erase:		Flash erase ops: Erase len bytes from offset
 *			Supported cmds: Sector erase 4K, 32K, 64K
 * return 0 - Sucess, 1 - Failure
 */
struct spi_flash {
	struct spi_slave *spi;
	const char *name;

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

	void *memory_map;
	int (*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int (*erase)(struct spi_flash *flash, u32 offset, size_t len);
};

/**
 * spi_flash_do_alloc - Allocate a new spi flash structure
 *
 * The structure is allocated and cleared with default values for
 * read, write and erase, which the caller can modify. The caller must set
 * up size, page_size and sector_size.
 *
 * Use the helper macro spi_flash_alloc() to call this.
 *
 * @offset: Offset of struct spi_slave within slave structure
 * @size: Size of slave structure
 * @spi: SPI slave
 * @name: Name of SPI flash device
 */
void *spi_flash_do_alloc(int offset, int size, struct spi_slave *spi,
			 const char *name);

/**
 * spi_flash_alloc - Allocate a new SPI flash structure
 *
 * @_struct: Name of structure to allocate (e.g. struct ramtron_spi_fram). This
 *	structure must contain a member 'struct spi_flash *flash'.
 * @spi: SPI slave
 * @name: Name of SPI flash device
 */
#define spi_flash_alloc(_struct, spi, name) \
	spi_flash_do_alloc(offsetof(_struct, flash), sizeof(_struct), \
				spi, name)

/**
 * spi_flash_alloc_base - Allocate a new SPI flash structure with no private data
 *
 * @spi: SPI slave
 * @name: Name of SPI flash device
 */
#define spi_flash_alloc_base(spi, name) \
	spi_flash_do_alloc(0, sizeof(struct spi_flash), spi, name)

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

void spi_boot(void) __noreturn;

#endif /* _SPI_FLASH_H_ */
