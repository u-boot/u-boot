/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 * (C) Copyright 2011
 * Xiangfu Liu <xiangfu@openmobilefree.net>
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
#include <command.h>
#include <netdev.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/jz4740.h>

#define cache_op(op, addr)		\
	__asm__ __volatile__(		\
		".set	push\n"		\
		".set	noreorder\n"	\
		".set	mips3\n"	\
		"cache	%0, %1\n"	\
		".set	pop\n"		\
		:			\
		: "i" (op), "R" (*(unsigned char *)(addr)))

void __attribute__((weak)) _machine_restart(void)
{
	struct jz4740_wdt *wdt = (struct jz4740_wdt *)JZ4740_WDT_BASE;
	struct jz4740_tcu *tcu = (struct jz4740_tcu *)JZ4740_TCU_BASE;
	u16 tmp;

	/* wdt_select_extalclk() */
	tmp = readw(&wdt->tcsr);
	tmp &= ~(WDT_TCSR_EXT_EN | WDT_TCSR_RTC_EN | WDT_TCSR_PCK_EN);
	tmp |= WDT_TCSR_EXT_EN;
	writew(tmp, &wdt->tcsr);

	/* wdt_select_clk_div64() */
	tmp = readw(&wdt->tcsr);
	tmp &= ~WDT_TCSR_PRESCALE_MASK;
	tmp |= WDT_TCSR_PRESCALE64,
	writew(tmp, &wdt->tcsr);

	writew(100, &wdt->tdr); /* wdt_set_data(100) */
	writew(0, &wdt->tcnt); /* wdt_set_count(0); */
	writel(TCU_TSSR_WDTSC, &tcu->tscr); /* tcu_start_wdt_clock */
	writeb(readb(&wdt->tcer) | WDT_TCER_TCEN, &wdt->tcer); /* wdt start */

	while (1)
		;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	_machine_restart();

	fprintf(stderr, "*** reset failed ***\n");
	return 0;
}

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize) {
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
	}
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize)
		cache_op(Hit_Writeback_Inv_D, addr);
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize)
		cache_op(Hit_Invalidate_D, addr);
}

void flush_icache_all(void)
{
	u32 addr, t = 0;

	__asm__ __volatile__("mtc0 $0, $28"); /* Clear Taglo */
	__asm__ __volatile__("mtc0 $0, $29"); /* Clear TagHi */

	for (addr = CKSEG0; addr < CKSEG0 + CONFIG_SYS_ICACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		cache_op(Index_Store_Tag_I, addr);
	}

	/* invalidate btb */
	__asm__ __volatile__(
		".set mips32\n\t"
		"mfc0 %0, $16, 7\n\t"
		"nop\n\t"
		"ori %0,2\n\t"
		"mtc0 %0, $16, 7\n\t"
		".set mips2\n\t"
		:
		: "r" (t));
}

void flush_dcache_all(void)
{
	u32 addr;

	for (addr = CKSEG0; addr < CKSEG0 + CONFIG_SYS_DCACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		cache_op(Index_Writeback_Inv_D, addr);
	}

	__asm__ __volatile__("sync");
}

void flush_cache_all(void)
{
	flush_dcache_all();
	flush_icache_all();
}
