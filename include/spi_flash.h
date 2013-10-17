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

#include <spi.h>
#include <linux/types.h>
#include <linux/compiler.h>

/* SPI connection modes */
enum spi_con_topology {
	MODE_UNKNOWN = -1,
	MODE_SINGLE,
	MODE_DUAL_STACKED,
	MODE_DUAL_PARALLEL,
};

/* Default read and write commands */
#define CMD_PAGE_PROGRAM		0x02
#define CMD_READ_ARRAY_FAST		0x0b

enum spi_write_cmds {
	PAGE_PROGRAM = 1 << 0,
	QUAD_PAGE_PROGRAM = 1 << 1,
};

enum spi_read_cmds {
	ARRAY_SLOW = 1 << 0,
	ARRAY_FAST = 1 << 1,
	DUAL_OUTPUT_FAST = 1 << 2,
	DUAL_IO_FAST = 1 << 3,
	QUAD_OUTPUT_FAST = 1 << 4,
};

#define READ_CMD_FULL	ARRAY_SLOW | ARRAY_FAST | DUAL_OUTPUT_FAST | \
			DUAL_IO_FAST | QUAD_OUTPUT_FAST

/**
 * struct spi_flash - SPI flash structure
 *
 * @spi:		SPI slave
 * @name:		Name of SPI flash
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:		Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @poll_cmd:		Poll cmd - for flash erase/program
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @memory_map:		Address of read-only SPI flash access
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
	/* Poll cmd - for flash erase/program */
	u8		poll_cmd;
	/* Read command */
	u8		read_cmd;
	/* Write command */
	u8		write_cmd;
	u8 erase_cmd;

	void *memory_map;
	int (*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int (*erase)(struct spi_flash *flash, u32 offset, size_t len);
};

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
