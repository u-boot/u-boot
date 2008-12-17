/*
 * U-boot - cache.c
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/mpu.h>

void flush_cache(unsigned long addr, unsigned long size)
{
	/* no need to flush stuff in on chip memory (L1/L2/etc...) */
	if (addr >= 0xE0000000)
		return;

	if (icache_status())
		blackfin_icache_flush_range((void *)addr, (void *)(addr + size));

	if (dcache_status())
		blackfin_dcache_flush_range((void *)addr, (void *)(addr + size));
}

void icache_enable(void)
{
	bfin_write_IMEM_CONTROL(IMC | ENICPLB);
	SSYNC();
}

void icache_disable(void)
{
	bfin_write_IMEM_CONTROL(0);
	SSYNC();
}

int icache_status(void)
{
	return bfin_read_IMEM_CONTROL() & IMC;
}

void dcache_enable(void)
{
	bfin_write_DMEM_CONTROL(ACACHE_BCACHE | ENDCPLB | PORT_PREF0);
	SSYNC();
}

void dcache_disable(void)
{
	bfin_write_DMEM_CONTROL(0);
	SSYNC();
}

int dcache_status(void)
{
	return bfin_read_DMEM_CONTROL() & ACACHE_BCACHE;
}
