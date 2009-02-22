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
#include <command.h>
#include <asm/blackfin.h>

void cf_outb(unsigned char val, volatile unsigned char *addr)
{
	*(addr) = val;
	SSYNC();
}

unsigned char cf_inb(volatile unsigned char *addr)
{
	volatile unsigned char c;

	c = *(addr);
	SSYNC();

	return c;
}

void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words)
{
	int i;

	for (i = 0; i < words; i++)
		*(sect_buf + i) = *(addr);
	SSYNC();
}

void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words)
{
	int i;

	for (i = 0; i < words; i++)
		*(addr) = *(sect_buf + i);
	SSYNC();
}

void cf_ide_init(void)
{
#if defined(CONFIG_BFIN_TRUE_IDE)
	/* Enable ATASEL when in True IDE mode */
	printf("Using CF True IDE Mode\n");
	cf_outb(0, (unsigned char *)CONFIG_CF_ATASEL_ENA);
	udelay(1000);
#elif defined(CONFIG_BFIN_CF_IDE)
	/* Disable ATASEL when we're in Common Memory Mode */
	printf("Using CF Common Memory Mode\n");
	cf_outb(0, (unsigned char *)CONFIG_CF_ATASEL_DIS);
	udelay(1000);
#elif defined(CONFIG_BFIN_HDD_IDE)
	printf("Using HDD IDE Mode\n");
#endif
	ide_init();
}
