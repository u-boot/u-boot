/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * SPI Read/Write Utilities
 */

#include <common.h>
#include <command.h>
#include <spi.h>

/*-----------------------------------------------------------------------
 * Definitions
 */

#ifndef MAX_SPI_BYTES
#   define MAX_SPI_BYTES 32	/* Maximum number of bytes we can handle */
#endif

#ifndef CONFIG_DEFAULT_SPI_BUS
#   define CONFIG_DEFAULT_SPI_BUS	0
#endif
#ifndef CONFIG_DEFAULT_SPI_MODE
#   define CONFIG_DEFAULT_SPI_MODE	SPI_MODE_0
#endif

/*
 * Values from last command.
 */
static unsigned int	device;
static int   		bitlen;
static uchar 		dout[MAX_SPI_BYTES];
static uchar 		din[MAX_SPI_BYTES];

/*
 * SPI read/write
 *
 * Syntax:
 *   spi {dev} {num_bits} {dout}
 *     {dev} is the device number for controlling chip select (see TBD)
 *     {num_bits} is the number of bits to send & receive (base 10)
 *     {dout} is a hexadecimal string of data to send
 * The command prints out the hexadecimal string received via SPI.
 */

int do_spi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct spi_slave *slave;
	char  *cp = 0;
	uchar tmp;
	int   j;
	int   rcode = 0;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc >= 2)
			device = simple_strtoul(argv[1], NULL, 10);
		if (argc >= 3)
			bitlen = simple_strtoul(argv[2], NULL, 10);
		if (argc >= 4) {
			cp = argv[3];
			for(j = 0; *cp; j++, cp++) {
				tmp = *cp - '0';
				if(tmp > 9)
					tmp -= ('A' - '0') - 10;
				if(tmp > 15)
					tmp -= ('a' - 'A');
				if(tmp > 15) {
					printf("Hex conversion error on %c, giving up.\n", *cp);
					return 1;
				}
				if((j % 2) == 0)
					dout[j / 2] = (tmp << 4);
				else
					dout[j / 2] |= tmp;
			}
		}
	}

	if ((bitlen < 0) || (bitlen >  (MAX_SPI_BYTES * 8))) {
		printf("Invalid bitlen %d, giving up.\n", bitlen);
		return 1;
	}

	/* FIXME: Make these parameters run-time configurable */
	slave = spi_setup_slave(CONFIG_DEFAULT_SPI_BUS, device, 1000000,
			CONFIG_DEFAULT_SPI_MODE);
	if (!slave) {
		printf("Invalid device %d, giving up.\n", device);
		return 1;
	}

	debug ("spi chipsel = %08X\n", device);

	spi_claim_bus(slave);
	if(spi_xfer(slave, bitlen, dout, din,
				SPI_XFER_BEGIN | SPI_XFER_END) != 0) {
		printf("Error with the SPI transaction.\n");
		rcode = 1;
	} else {
		for(j = 0; j < ((bitlen + 7) / 8); j++) {
			printf("%02X", din[j]);
		}
		printf("\n");
	}
	spi_release_bus(slave);
	spi_free_slave(slave);

	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	sspi,	5,	1,	do_spi,
	"SPI utility commands",
	"<device> <bit_len> <dout> - Send <bit_len> bits from <dout> out the SPI\n"
	"<device>  - Identifies the chip select of the device\n"
	"<bit_len> - Number of bits to send (base 10)\n"
	"<dout>    - Hexadecimal string that gets sent"
);
