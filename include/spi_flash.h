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

/* sf param flags */
#define SECT_4K		1 << 1
#define SECT_32K	1 << 2
#define E_FSR		1 << 3
#define WR_QPP		1 << 4

/* Enum list - Full read commands */
enum spi_read_cmds {
	ARRAY_SLOW = 1 << 0,
	DUAL_OUTPUT_FAST = 1 << 1,
	DUAL_IO_FAST = 1 << 2,
	QUAD_OUTPUT_FAST = 1 << 3,
	QUAD_IO_FAST = 1 << 4,
};
#define RD_EXTN		ARRAY_SLOW | DUAL_OUTPUT_FAST | DUAL_IO_FAST
#define RD_FULL		RD_EXTN | QUAD_OUTPUT_FAST | QUAD_IO_FAST

/* Dual SPI flash memories */
enum spi_dual_flash {
	SF_SINGLE_FLASH = 0,
	SF_DUAL_STACKED_FLASH = 1 << 0,
	SF_DUAL_PARALLEL_FLASH = 1 << 1,
};

/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:		Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:		Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:		Device ext_jedec ID
 * @sector_size:	Sector size of this device
 * @nr_sectors:		No.of sectors on this device
 * @e_rd_cmd:		Enum list for read commands
 * @flags:		Importent param, for flash specific behaviour
 */
struct spi_flash_params {
	const char *name;
	u32 jedec;
	u16 ext_jedec;
	u32 sector_size;
	u32 nr_sectors;
	u8 e_rd_cmd;
	u16 flags;
};

extern const struct spi_flash_params spi_flash_params_table[];

/**
 * struct spi_flash - SPI flash structure
 *
 * @spi:		SPI slave
 * @name:		Name of SPI flash
 * @dual_flash:		Indicates dual flash memories - dual stacked, parallel
 * @shift:		Flash shift useful in dual parallel
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:		Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @poll_cmd:		Poll cmd - for flash erase/program
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @read_cmd:		Read cmd - Array Fast, Extn read and quad read.
 * @write_cmd:		Write cmd - page and quad program.
 * @dummy_byte:		Dummy cycles for read operation.
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
	int (*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int (*erase)(struct spi_flash *flash, u32 offset, size_t len);
};

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

void spi_boot(void) __noreturn;

#endif /* _SPI_FLASH_H_ */
