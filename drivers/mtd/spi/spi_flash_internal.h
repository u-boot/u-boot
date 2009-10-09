/*
 * SPI flash internal definitions
 *
 * Copyright (C) 2008 Atmel Corporation
 */

/* Common parameters -- kind of high, but they should only occur when there
 * is a problem (and well your system already is broken), so err on the side
 * of caution in case we're dealing with slower SPI buses and/or processors.
 */
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)

/* Common commands */
#define CMD_READ_ID			0x9f

#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_ARRAY_LEGACY		0xe8

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

/*
 * Same as spi_flash_cmd_read() except it also claims/releases the SPI
 * bus. Used as common part of the ->read() operation.
 */
int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* Manufacturer-specific probe functions */
struct spi_flash *spi_flash_probe_spansion(struct spi_slave *spi, u8 *idcode);
struct spi_flash *spi_flash_probe_atmel(struct spi_slave *spi, u8 *idcode);
struct spi_flash *spi_flash_probe_macronix(struct spi_slave *spi, u8 *idcode);
struct spi_flash *spi_flash_probe_sst(struct spi_slave *spi, u8 *idcode);
struct spi_flash *spi_flash_probe_stmicro(struct spi_slave *spi, u8 *idcode);
struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode);
