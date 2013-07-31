/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>

static void s3c44b0_flush_cache(void)
{
	volatile int i;
	/* flush cycle */
	for(i=0x10002000;i<0x10004800;i+=16)
	{
		*((int *)i)=0x0;
	}
}

void icache_enable (void)
{
	ulong reg;

	s3c44b0_flush_cache();

	/*
		Init cache
		Non-cacheable area (everything outside RAM)
		0x0000:0000 - 0x0C00:0000
	 */
	NCACHBE0 = 0xC0000000;
	NCACHBE1 = 0x00000000;

	/*
		Enable chache
	*/
	reg = SYSCFG;
	reg |= 0x00000006; /* 8kB */
	SYSCFG = reg;
}

void icache_disable (void)
{
	ulong reg;

	reg = SYSCFG;
	reg &= ~0x00000006; /* 8kB */
	SYSCFG = reg;
}

int icache_status (void)
{
	return 0;
}

void dcache_enable (void)
{
	icache_enable();
}

void dcache_disable (void)
{
	icache_disable();
}

int dcache_status (void)
{
	return dcache_status();
}
