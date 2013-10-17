/*
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * Copyright (C) 2008 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>

#define STBCR4      0xFFFE040C
#define cmt_clock_enable() do {\
		writeb(readb(STBCR4) & ~0x04, STBCR4);\
	} while (0)
#define scif0_enable() do {\
		writeb(readb(STBCR4) & ~0x80, STBCR4);\
	} while (0)
#define scif3_enable() do {\
		writeb(readb(STBCR4) & ~0x10, STBCR4);\
	} while (0)

int checkcpu(void)
{
#if defined(CONFIG_SH2A)
	puts("CPU: SH2A\n");
#else
	puts("CPU: SH2\n");
#endif
	return 0;
}

int cpu_init(void)
{
	/* SCIF enable */
#if defined(CONFIG_CONS_SCIF3)
	scif3_enable();
#else
	scif0_enable();
#endif
	/* CMT clock enable */
	cmt_clock_enable() ;
	return 0;
}

int cleanup_before_linux(void)
{
	disable_interrupts();
	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	reset_cpu(0);
	return 0;
}

void flush_cache(unsigned long addr, unsigned long size)
{

}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int dcache_status(void)
{
	return 0;
}
