/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>
#include <i2c.h>

void sdram_init(void)
{
	return;
}

long int initdram(int board_type)
{
	return (CFG_MBYTES_SDRAM << 20);
}

#if defined(CFG_DRAM_TEST)
int testdram (void)
{
    printf ("testdram\n");
#if defined (CONFIG_NAND_U_BOOT)
    return 0;
#endif
	uint *pstart = (uint *) 0x00000000;
	uint *pend = (uint *) 0x00001000;
	uint *p;

	for (p = pstart; p < pend; p++) {
		*p = 0xaaaaaaaa;
	}

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
#if !defined (CONFIG_NAND_SPL)
			printf ("SDRAM test fails at: %08x\n", (uint) p);
#endif
			return 1;
		}
	}

	for (p = pstart; p < pend; p++) {
		*p = 0x55555555;
	}

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
#if !defined (CONFIG_NAND_SPL)
			printf ("SDRAM test fails at: %08x\n", (uint) p);
#endif
			return 1;
		}
	}
#if !defined (CONFIG_NAND_SPL)
	printf ("SDRAM test passed!!!\n");
#endif
	return 0;
}
#endif
