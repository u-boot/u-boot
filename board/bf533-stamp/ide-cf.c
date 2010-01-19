/*
 * CF IDE addon card code
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <asm/blackfin.h>
#include "bf533-stamp.h"

void cf_outb(unsigned char val, volatile unsigned char *addr)
{
	/* "ETHERNET" means the expansion memory banks */
	swap_to(ETHERNET);

	*addr = val;
	SSYNC();

	swap_to(FLASH);
}

unsigned char cf_inb(volatile unsigned char *addr)
{
	unsigned char c;

	swap_to(ETHERNET);

	c = *addr;
	SSYNC();

	swap_to(FLASH);

	return c;
}

void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words)
{
	int i;

	swap_to(ETHERNET);

	for (i = 0; i < words; i++) {
		*(sect_buf + i) = *addr;
		SSYNC();
	}

	swap_to(FLASH);
}

void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words)
{
	int i;

	swap_to(ETHERNET);

	for (i = 0; i < words; i++) {
		*addr = *(sect_buf + i);
		SSYNC();
	}

	swap_to(FLASH);
}

void cf_ide_init(void)
{
	int i, cf_stat;

	/* Check whether CF card is inserted */
	bfin_write_FIO_EDGE(FIO_EDGE_CF_BITS);
	bfin_write_FIO_POLAR(FIO_POLAR_CF_BITS);
	for (i = 0; i < 0x300; i++)
		asm volatile("nop;");

	cf_stat = bfin_read_FIO_FLAG_S() & CF_STAT_BITS;

	bfin_write_FIO_EDGE(FIO_EDGE_BITS);
	bfin_write_FIO_POLAR(FIO_POLAR_BITS);

	if (!cf_stat) {
		for (i = 0; i < 0x3000; i++)
			asm volatile("nop;");

		ide_init();
	}
}
