/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_SDRAM_H
#define __ASM_AVR32_SDRAM_H

struct sdram_config {
	/* Number of data bits. */
	enum {
		SDRAM_DATA_16BIT = 16,
		SDRAM_DATA_32BIT = 32,
	} data_bits;

	/* Number of address bits */
	uint8_t row_bits, col_bits, bank_bits;

	/* SDRAM timings in cycles */
	uint8_t cas, twr, trc, trp, trcd, tras, txsr;

	/* SDRAM refresh period in cycles */
	unsigned long refresh_period;
};

/*
 * Attempt to initialize the SDRAM controller using the specified
 * parameters. Return the expected size of the memory area based on
 * the number of address and data bits.
 *
 * The caller should verify that the configuration is correct by
 * running a memory test, e.g. get_ram_size().
 */
extern unsigned long sdram_init(void *sdram_base,
			const struct sdram_config *config);

#endif /* __ASM_AVR32_SDRAM_H */
