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
	void *start_addr, *end_addr;
	int istatus, dstatus;

	/* no need to flush stuff in on chip memory (L1/L2/etc...) */
	if (addr >= 0xE0000000)
		return;

	start_addr = (void *)addr;
	end_addr = (void *)(addr + size);
	istatus = icache_status();
	dstatus = dcache_status();

	if (istatus) {
		if (dstatus)
			blackfin_icache_dcache_flush_range(start_addr, end_addr);
		else
			blackfin_icache_flush_range(start_addr, end_addr);
	} else if (dstatus)
		blackfin_dcache_flush_range(start_addr, end_addr);
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
