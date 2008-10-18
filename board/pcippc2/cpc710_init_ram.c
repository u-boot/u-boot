/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>

#include "pcippc2.h"
#include "i2c.h"

typedef struct cpc710_mem_org_s {
	u8 rows;
	u8 cols;
	u8 banks2;
	u8 org;
} cpc710_mem_org_t;

static int cpc710_compute_mcer (u32 * mcer,
				unsigned long *size, unsigned int sdram);
static int cpc710_eeprom_checksum (unsigned int sdram);
static u8 cpc710_eeprom_read (unsigned int sdram, unsigned int offset);

static u32 cpc710_mcer_mem[] = {
	0x000003f3,		/* 18 lines,    4 Mb */
	0x000003e3,		/* 19 lines,    8 Mb */
	0x000003c3,		/* 20 lines,   16 Mb */
	0x00000383,		/* 21 lines,   32 Mb */
	0x00000303,		/* 22 lines,   64 Mb */
	0x00000203,		/* 23 lines,  128 Mb */
	0x00000003,		/* 24 lines,  256 Mb */
	0x00000002,		/* 25 lines,  512 Mb */
	0x00000001		/* 26 lines, 1024 Mb */
};
static cpc710_mem_org_t cpc710_mem_org[] = {
	{0x0c, 0x09, 0x02, 0x00},	/* 0000: 12/ 9/2 */
	{0x0d, 0x09, 0x02, 0x00},	/* 0000: 13/ 9/2 */
	{0x0d, 0x0a, 0x02, 0x00},	/* 0000: 13/10/2 */
	{0x0d, 0x0b, 0x02, 0x00},	/* 0000: 13/11/2 */
	{0x0d, 0x0c, 0x02, 0x00},	/* 0000: 13/12/2 */
	{0x0e, 0x0c, 0x02, 0x00},	/* 0000: 14/12/2 */
	{0x0b, 0x08, 0x02, 0x01},	/* 0001: 11/ 8/2 */
	{0x0b, 0x09, 0x01, 0x02},	/* 0010: 11/ 9/1 */
	{0x0b, 0x0a, 0x01, 0x03},	/* 0011: 11/10/1 */
	{0x0c, 0x08, 0x02, 0x04},	/* 0100: 12/ 8/2 */
	{0x0c, 0x0a, 0x02, 0x05},	/* 0101: 12/10/2 */
	{0x0d, 0x08, 0x01, 0x06},	/* 0110: 13/ 8/1 */
	{0x0d, 0x08, 0x02, 0x07},	/* 0111: 13/ 8/2 */
	{0x0d, 0x09, 0x01, 0x08},	/* 1000: 13/ 9/1 */
	{0x0d, 0x0a, 0x01, 0x09},	/* 1001: 13/10/1 */
	{0x0b, 0x08, 0x01, 0x0a},	/* 1010: 11/ 8/1 */
	{0x0c, 0x08, 0x01, 0x0b},	/* 1011: 12/ 8/1 */
	{0x0c, 0x09, 0x01, 0x0c},	/* 1100: 12/ 9/1 */
	{0x0e, 0x09, 0x02, 0x0d},	/* 1101: 14/ 9/2 */
	{0x0e, 0x0a, 0x02, 0x0e},	/* 1110: 14/10/2 */
	{0x0e, 0x0b, 0x02, 0x0f}	/* 1111: 14/11/2 */
};

unsigned long cpc710_ram_init (void)
{
	unsigned long memsize = 0;
	unsigned long bank_size;
	u32 mcer;

#ifndef CONFIG_SYS_RAMBOOT
	/* Clear memory banks
	 */
	out32 (REG (SDRAM0, MCER0), 0);
	out32 (REG (SDRAM0, MCER1), 0);
	out32 (REG (SDRAM0, MCER2), 0);
	out32 (REG (SDRAM0, MCER3), 0);
	out32 (REG (SDRAM0, MCER4), 0);
	out32 (REG (SDRAM0, MCER5), 0);
	out32 (REG (SDRAM0, MCER6), 0);
	out32 (REG (SDRAM0, MCER7), 0);
	iobarrier_rw ();

	/* Disable memory
	 */
	out32 (REG (SDRAM0, MCCR), 0x13b06000);
	iobarrier_rw ();
#endif

	/* Only the first memory bank is initialised now
	 */
	if (!cpc710_compute_mcer (&mcer, &bank_size, 0)) {
		puts ("Unsupported SDRAM type !\n");
		hang ();
	}
	memsize += bank_size;
#ifndef CONFIG_SYS_RAMBOOT
	/* Enable bank, zero start
	 */
	out32 (REG (SDRAM0, MCER0), mcer | 0x80000000);
	iobarrier_rw ();
#endif

#ifndef CONFIG_SYS_RAMBOOT
	/* Enable memory
	 */
	out32 (REG (SDRAM0, MCCR), in32 (REG (SDRAM0, MCCR)) | 0x80000000);

	/* Wait until initialisation finished
	 */
	while (!(in32 (REG (SDRAM0, MCCR)) & 0x20000000)) {
		iobarrier_rw ();
	}

	/* Clear Memory Error Status and Address registers
	 */
	out32 (REG (SDRAM0, MESR), 0);
	out32 (REG (SDRAM0, MEAR), 0);
	iobarrier_rw ();

	/* ECC is not configured now
	 */
#endif

	/* Memory size counter
	 */
	out32 (REG (CPC0, RGBAN1), memsize);

	return memsize;
}

static int cpc710_compute_mcer (u32 * mcer, unsigned long *size, unsigned int sdram)
{
	u8 rows;
	u8 cols;
	u8 banks2;
	unsigned int lines;
	u32 mc = 0;
	unsigned int i;
	cpc710_mem_org_t *org = 0;

	if (!i2c_reset ()) {
		puts ("Can't reset I2C!\n");
		hang ();
	}

	if (!cpc710_eeprom_checksum (sdram)) {
		puts ("Invalid EEPROM checksum !\n");
		hang ();
	}

	rows = cpc710_eeprom_read (sdram, 3);
	cols = cpc710_eeprom_read (sdram, 4);
	/* Can be 2 or 4 banks; divide by 2
	 */
	banks2 = cpc710_eeprom_read (sdram, 17) / 2;

	lines = rows + cols + banks2;

	if (lines < 18 || lines > 26) {
		/* Unsupported configuration
		 */
		return 0;
	}

	mc |= cpc710_mcer_mem[lines - 18] << 6;

	for (i = 0; i < sizeof (cpc710_mem_org) / sizeof (cpc710_mem_org_t);
	     i++) {
		cpc710_mem_org_t *corg = cpc710_mem_org + i;

		if (corg->rows == rows && corg->cols == cols
		    && corg->banks2 == banks2) {
			org = corg;

			break;
		}
	}

	if (!org) {
		/* Unsupported configuration
		 */
		return 0;
	}

	mc |= (u32) org->org << 2;

	/* Supported configuration
	 */
	*mcer = mc;
	*size = 1l << (lines + 4);

	return 1;
}

static int cpc710_eeprom_checksum (unsigned int sdram)
{
	u8 sum = 0;
	unsigned int i;

	for (i = 0; i < 63; i++) {
		sum += cpc710_eeprom_read (sdram, i);
	}

	return sum == cpc710_eeprom_read (sdram, 63);
}

static u8 cpc710_eeprom_read (unsigned int sdram, unsigned int offset)
{
	u8 dev = (sdram << 1) | 0xa0;
	u8 data;

	if (!i2c_read_byte (&data, dev, offset)) {
		puts ("I2C error !\n");
		hang ();
	}

	return data;
}
