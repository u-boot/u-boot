/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
