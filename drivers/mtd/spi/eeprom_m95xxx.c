/*
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>

#define SPI_EEPROM_WREN		0x06
#define SPI_EEPROM_RDSR		0x05
#define SPI_EEPROM_READ		0x03
#define SPI_EEPROM_WRITE	0x02

#ifndef CONFIG_DEFAULT_SPI_BUS
#define CONFIG_DEFAULT_SPI_BUS 0
#endif

#ifndef CONFIG_DEFAULT_SPI_MODE
#define CONFIG_DEFAULT_SPI_MODE SPI_MODE_0
#endif

#ifndef CONFIG_SYS_SPI_WRITE_TOUT
#define CONFIG_SYS_SPI_WRITE_TOUT (5 * CONFIG_SYS_HZ)
#endif

ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	struct spi_slave *slave;
	u8 cmd = SPI_EEPROM_READ;

	slave = spi_setup_slave(CONFIG_DEFAULT_SPI_BUS, 1, 1000000,
			CONFIG_DEFAULT_SPI_MODE);
	if (!slave)
		return 0;

	spi_claim_bus(slave);

	/* command */
	if (spi_xfer(slave, 8, &cmd, NULL, SPI_XFER_BEGIN))
		return -1;

	/*
	 * if alen == 3, addr[0] is the block number, we never use it here.
	 * All we need are the lower 16 bits.
	 */
	if (alen == 3)
		addr++;

	/* address, and data */
	if (spi_xfer(slave, 16, addr, NULL, 0))
		return -1;
	if (spi_xfer(slave, 8 * len, NULL, buffer, SPI_XFER_END))
		return -1;

	spi_release_bus(slave);
	spi_free_slave(slave);
	return len;
}

ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	struct spi_slave *slave;
	char buf[3];
	ulong start;

	slave = spi_setup_slave(CONFIG_DEFAULT_SPI_BUS, 1, 1000000,
			CONFIG_DEFAULT_SPI_MODE);
	if (!slave)
		return 0;

	spi_claim_bus(slave);

	buf[0] = SPI_EEPROM_WREN;
	if (spi_xfer(slave, 8, buf, NULL, SPI_XFER_BEGIN | SPI_XFER_END))
		return -1;

	buf[0] = SPI_EEPROM_WRITE;

	/* As for reading, drop addr[0] if alen is 3 */
	if (alen == 3) {
		alen--;
		addr++;
	}

	memcpy(buf + 1, addr, alen);
	/* command + addr, then data */
	if (spi_xfer(slave, 24, buf, NULL, SPI_XFER_BEGIN))
		return -1;
	if (spi_xfer(slave, len * 8, buffer, NULL, SPI_XFER_END))
		return -1;

	start = get_timer(0);
	do {
		buf[0] = SPI_EEPROM_RDSR;
		buf[1] = 0;
		spi_xfer(slave, 16, buf, buf, SPI_XFER_BEGIN | SPI_XFER_END);

		if (!(buf[1] & 1))
			break;

	} while (get_timer(start) < CONFIG_SYS_SPI_WRITE_TOUT);

	if (buf[1] & 1)
		printf("*** spi_write: Timeout while writing!\n");

	spi_release_bus(slave);
	spi_free_slave(slave);
	return len;
}
