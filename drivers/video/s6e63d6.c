/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
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
#include <common.h>
#include <spi.h>
#include <s6e63d6.h>

/*
 * Each transfer is performed as:
 * 1. chip-select active
 * 2. send 8-bit start code
 * 3. send 16-bit data
 * 4. chip-select inactive
 */
static int send_word(struct s6e63d6 *data, u8 rs, u16 word)
{
	/*
	 * The start byte looks like (binary):
	 * 01110<ID><RS><R/W>
	 * RS is 0 for index or 1 for data, and R/W is 0 for write.
	 */
	u32 buf8 = 0x70 | data->id | (rs & 2);
	u32 buf16 = cpu_to_le16(word);
	u32 buf_in;
	int err;

	err = spi_xfer(data->slave, 8, &buf8, &buf_in, SPI_XFER_BEGIN);
	if (err)
		return err;

	return spi_xfer(data->slave, 16, &buf16, &buf_in, SPI_XFER_END);
}

/* Index and param differ in Register Select bit */
int s6e63d6_index(struct s6e63d6 *data, u8 idx)
{
	return send_word(data, 0, idx);
}

int s6e63d6_param(struct s6e63d6 *data, u16 param)
{
	return send_word(data, 2, param);
}

int s6e63d6_init(struct s6e63d6 *data)
{
	if (data->id != 0 && data->id != 4) {
		printf("s6e63d6: invalid ID %u\n", data->id);
		return 1;
	}

	data->slave = spi_setup_slave(data->bus, data->cs, 100000, SPI_MODE_3);
	if (!data->slave)
		return 1;

	return 0;
}
