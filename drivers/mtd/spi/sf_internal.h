/*
 * SPI flash internal definitions
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SF_INTERNAL_H_
#define _SF_INTERNAL_H_

#include <linux/types.h>
#include <linux/compiler.h>

/* Dual SPI flash memories - see SPI_COMM_DUAL_... */
enum spi_dual_flash {
	SF_SINGLE_FLASH	= 0,
	SF_DUAL_STACKED_FLASH	= 1 << 0,
	SF_DUAL_PARALLEL_FLASH	= 1 << 1,
};

/* Enum list - Full read commands */
enum spi_read_cmds {
	ARRAY_SLOW		= 1 << 0,
	ARRAY_FAST		= 1 << 1,
	DUAL_OUTPUT_FAST	= 1 << 2,
	DUAL_IO_FAST		= 1 << 3,
	QUAD_OUTPUT_FAST	= 1 << 4,
	QUAD_IO_FAST		= 1 << 5,
};

/* Normal - Extended - Full command set */
#define RD_NORM	(ARRAY_SLOW | ARRAY_FAST)
#define RD_EXTN	(RD_NORM | DUAL_OUTPUT_FAST | DUAL_IO_FAST)
#define RD_FULL	(RD_EXTN | QUAD_OUTPUT_FAST | QUAD_IO_FAST)

/* sf param flags */
enum {
	SECT_4K		= 1 << 0,
	SECT_32K	= 1 << 1,
	E_FSR		= 1 << 2,
	SST_BP		= 1 << 3,
	SST_WP		= 1 << 4,
	WR_QPP		= 1 << 5,
};

#define SST_WR		(SST_BP | SST_WP)

#define SPI_FLASH_3B_ADDR_LEN		3
#define SPI_FLASH_CMD_LEN		(1 + SPI_FLASH_3B_ADDR_LEN)
#define SPI_FLASH_16MB_BOUN		0x1000000

/* CFI Manufacture ID's */
#define SPI_FLASH_CFI_MFR_SPANSION	0x01
#define SPI_FLASH_CFI_MFR_STMICRO	0x20
#define SPI_FLASH_CFI_MFR_MACRONIX	0xc2
#define SPI_FLASH_CFI_MFR_WINBOND	0xef

/* Erase commands */
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_CHIP			0xc7
#define CMD_ERASE_64K			0xd8

/* Write commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_STATUS		0x05
#define CMD_QUAD_PAGE_PROGRAM		0x32
#define CMD_READ_STATUS1		0x35
#define CMD_WRITE_ENABLE		0x06
#define CMD_READ_CONFIG		0x35
#define CMD_FLAG_STATUS		0x70

/* Read commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_DUAL_OUTPUT_FAST	0x3b
#define CMD_READ_DUAL_IO_FAST		0xbb
#define CMD_READ_QUAD_OUTPUT_FAST	0x6b
#define CMD_READ_QUAD_IO_FAST		0xeb
#define CMD_READ_ID			0x9f

/* Bank addr access commands */
#ifdef CONFIG_SPI_FLASH_BAR
# define CMD_BANKADDR_BRWR		0x17
# define CMD_BANKADDR_BRRD		0x16
# define CMD_EXTNADDR_WREAR		0xC5
# define CMD_EXTNADDR_RDEAR		0xC8
#endif

/* Common status */
#define STATUS_WIP			(1 << 0)
#define STATUS_QEB_WINSPAN		(1 << 1)
#define STATUS_QEB_MXIC		(1 << 6)
#define STATUS_PEC			(1 << 7)

#ifdef CONFIG_SYS_SPI_ST_ENABLE_WP_PIN
#define STATUS_SRWD			(1 << 7) /* SR write protect */
#endif

/* Flash timeout values */
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT		(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)

/* SST specific */
#ifdef CONFIG_SPI_FLASH_SST
# define CMD_SST_BP		0x02    /* Byte Program */
# define CMD_SST_AAI_WP	0xAD	/* Auto Address Incr Word Program */

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf);
int sst_write_bp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf);
#endif

/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:		Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:		Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:		Device ext_jedec ID
 * @sector_size:	Sector size of this device
 * @nr_sectors:	No.of sectors on this device
 * @e_rd_cmd:		Enum list for read commands
 * @flags:		Important param, for flash specific behaviour
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

/* Send a single-byte command to the device and read the response */
int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len);

/*
 * Send a multi-byte command to the device and read the response. Used
 * for flash array reads, etc.
 */
int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/*
 * Send a multi-byte command to the device followed by (optional)
 * data. Used for programming the flash array, etc.
 */
int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len);


/* Flash erase(sectors) operation, support all possible erase commands */
int spi_flash_cmd_erase_ops(struct spi_flash *flash, u32 offset, size_t len);

/* Read the status register */
int spi_flash_cmd_read_status(struct spi_flash *flash, u8 *rs);

/* Program the status register */
int spi_flash_cmd_write_status(struct spi_flash *flash, u8 ws);

/* Read the config register */
int spi_flash_cmd_read_config(struct spi_flash *flash, u8 *rc);

/* Program the config register */
int spi_flash_cmd_write_config(struct spi_flash *flash, u8 wc);

/* Enable writing on the SPI flash */
static inline int spi_flash_cmd_write_enable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_ENABLE, NULL, 0);
}

/* Disable writing on the SPI flash */
static inline int spi_flash_cmd_write_disable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_DISABLE, NULL, 0);
}

/*
 * Send the read status command to the device and wait for the wip
 * (write-in-progress) bit to clear itself.
 */
int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout);

/*
 * Used for spi_flash write operation
 * - SPI claim
 * - spi_flash_cmd_write_enable
 * - spi_flash_cmd_write
 * - spi_flash_cmd_wait_ready
 * - SPI release
 */
int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len);

/*
 * Flash write operation, support all possible write commands.
 * Write the requested data out breaking it up into multiple write
 * commands as needed per the write size.
 */
int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf);

/*
 * Same as spi_flash_cmd_read() except it also claims/releases the SPI
 * bus. Used as common part of the ->read() operation.
 */
int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* Flash read operation, support all possible read commands */
int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data);

#endif /* _SF_INTERNAL_H_ */
