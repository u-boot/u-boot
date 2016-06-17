/*
 * U-Boot - cache.c
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

#ifdef CONFIG_DCACHE_WB
static void flushinv_all_dcache(void)
{
	u32 way, bank, subbank, set;
	u32 status, addr;
	u32 dmem_ctl = bfin_read_DMEM_CONTROL();

	for (bank = 0; bank < 2; ++bank) {
		if (!(dmem_ctl & (1 << (DMC1_P - bank))))
			continue;

		for (way = 0; way < 2; ++way)
			for (subbank = 0; subbank < 4; ++subbank)
				for (set = 0; set < 64; ++set) {

					bfin_write_DTEST_COMMAND(
						way << 26 |
						bank << 23 |
						subbank << 16 |
						set << 5
					);
					CSYNC();
					status = bfin_read_DTEST_DATA0();

					/* only worry about valid/dirty entries */
					if ((status & 0x3) != 0x3)
						continue;

					/* construct the address using the tag */
					addr = (status & 0xFFFFC800) | (subbank << 12) | (set << 5);

					/* flush it */
					__asm__ __volatile__("FLUSHINV[%0];" : : "a"(addr));
				}
	}
}
#endif

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
#ifdef CONFIG_DCACHE_WB
	bfin_write_DMEM_CONTROL(bfin_read_DMEM_CONTROL() & ~(ENDCPLB));
	flushinv_all_dcache();
#endif
	bfin_write_DMEM_CONTROL(0);
	SSYNC();
}

int dcache_status(void)
{
	return bfin_read_DMEM_CONTROL() & ACACHE_BCACHE;
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	blackfin_dcache_flush_invalidate_range((const void *)start, (const void *)stop);
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	blackfin_dcache_flush_range((const void *)start, (const void *)stop);
}
