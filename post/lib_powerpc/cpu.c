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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

/*
 * CPU test
 *
 * This test checks the arithmetic logic unit (ALU) of CPU.
 * It tests independently various groups of instructions using
 * run-time modification of the code to reduce the memory footprint.
 * For more details refer to post/cpu/ *.c files.
 */

#include <watchdog.h>
#include <post.h>
#include <asm/mmu.h>

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern int cpu_post_test_cmp (void);
extern int cpu_post_test_cmpi (void);
extern int cpu_post_test_two (void);
extern int cpu_post_test_twox (void);
extern int cpu_post_test_three (void);
extern int cpu_post_test_threex (void);
extern int cpu_post_test_threei (void);
extern int cpu_post_test_andi (void);
extern int cpu_post_test_srawi (void);
extern int cpu_post_test_rlwnm (void);
extern int cpu_post_test_rlwinm (void);
extern int cpu_post_test_rlwimi (void);
extern int cpu_post_test_store (void);
extern int cpu_post_test_load (void);
extern int cpu_post_test_cr (void);
extern int cpu_post_test_b (void);
extern int cpu_post_test_multi (void);
extern int cpu_post_test_string (void);
extern int cpu_post_test_complex (void);

DECLARE_GLOBAL_DATA_PTR;

ulong cpu_post_makecr (long v)
{
	ulong cr = 0;

	if (v < 0)
		cr |= 0x80000000;
	if (v > 0)
		cr |= 0x40000000;
	if (v == 0)
		cr |= 0x20000000;

	return cr;
}

int cpu_post_test (int flags)
{
	int ic = icache_status ();
	int ret = 0;

	WATCHDOG_RESET();
	if (ic)
		icache_disable ();
#ifdef CONFIG_4xx_DCACHE
	/* disable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, TLB_WORD2_I_ENABLE);
#endif

	if (ret == 0)
		ret = cpu_post_test_cmp ();
	if (ret == 0)
		ret = cpu_post_test_cmpi ();
	if (ret == 0)
		ret = cpu_post_test_two ();
	if (ret == 0)
		ret = cpu_post_test_twox ();
	WATCHDOG_RESET();
	if (ret == 0)
		ret = cpu_post_test_three ();
	if (ret == 0)
		ret = cpu_post_test_threex ();
	if (ret == 0)
		ret = cpu_post_test_threei ();
	if (ret == 0)
		ret = cpu_post_test_andi ();
	WATCHDOG_RESET();
	if (ret == 0)
		ret = cpu_post_test_srawi ();
	if (ret == 0)
		ret = cpu_post_test_rlwnm ();
	if (ret == 0)
		ret = cpu_post_test_rlwinm ();
	if (ret == 0)
		ret = cpu_post_test_rlwimi ();
	WATCHDOG_RESET();
	if (ret == 0)
		ret = cpu_post_test_store ();
	if (ret == 0)
		ret = cpu_post_test_load ();
	if (ret == 0)
		ret = cpu_post_test_cr ();
	if (ret == 0)
		ret = cpu_post_test_b ();
	WATCHDOG_RESET();
	if (ret == 0)
		ret = cpu_post_test_multi ();
	WATCHDOG_RESET();
	if (ret == 0)
		ret = cpu_post_test_string ();
	if (ret == 0)
		ret = cpu_post_test_complex ();
	WATCHDOG_RESET();

	if (ic)
		icache_enable ();
#ifdef CONFIG_4xx_DCACHE
	/* enable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, 0);
#endif

	WATCHDOG_RESET();

	return ret;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_CPU */
