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
 * SPI Functions
 */
#ifndef	_CMD_SPI_H
#define _CMD_SPI_H

#if (CONFIG_COMMANDS & CFG_CMD_SPI)
#define CMD_TBL_SPI	MK_CMD_TBL_ENTRY(				\
	"sspi",		3,	5,	1,	do_spi,			\
	"sspi     - SPI utility commands\n",				\
	"\
<device> <bit_len> <dout> - Send <bit_len> bits from <dout> out the SPI\n\
  <device>  - Identifies the chip select of the device\n\
  <bit_len> - Number of bits to send (base 10)\n\
  <dout>    - Hexadecimal string that gets sent\n" \
),

int do_spi       (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_SPI
#endif	/* CFG_CMD_SPI */

#endif	/* _CMD_SPI_H */
