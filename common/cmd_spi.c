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
#include <cmd_spi.h>

#if (CONFIG_COMMANDS & CFG_CMD_SPI)

#define MAX_SPI_BYTES	32	/* max number of bytes we can handle */

/*
 * External table of chip select functions (see the appropriate board
 * support for the actual definition of the table).
 */
extern spi_chipsel_type spi_chipsel[];


/*
 * Values from last command.
 */
static int   device;
static int   bitlen;
static uchar dout[MAX_SPI_BYTES];
static uchar din[MAX_SPI_BYTES];

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

int do_spi (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
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
		if (argc >= 4)
		cp = argv[3];
		for(j = 0; *cp; j++, cp++) {
			tmp = *cp - '0';
			if(tmp > 9)
				tmp -= ('A' - '0') - 10;
			if(tmp > 15)
				tmp -= ('a' - 'A');
			if(tmp > 15) {
				printf("Conversion error on %c, bailing out.\n", *cp);
				break;
			}
			if((j % 2) == 0)
				dout[j / 2] = (tmp << 4);
			else
				dout[j / 2] |= tmp;
		}
	}

printf("spi_chipsel[%d] = %08X\n", device, (uint)spi_chipsel[device]);
	if(spi_xfer(spi_chipsel[device], bitlen, dout, din) != 0) {
		printf("Error with the SPI transaction.\n");
		rcode = 1;
	} else {
		cp = din;
		for(j = 0; j < ((bitlen + 7) / 8); j++) {
			printf("%02X", *cp++);
		}
		printf("\n");
	}

	return rcode;
}

#endif	/* CFG_CMD_SPI */

