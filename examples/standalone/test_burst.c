/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * The test exercises SDRAM accesses in burst mode
 */

#include <common.h>
#include <exports.h>

#include <commproc.h>
#include <asm/mmu.h>
#include <asm/processor.h>

#include <serial.h>
#include <watchdog.h>

#include "test_burst.h"

/* 8 MB test region of physical RAM */
#define TEST_PADDR	0x00800000
/* The uncached virtual region */
#define TEST_VADDR_NC	0x00800000
/* The cached virtual region */
#define TEST_VADDR_C	0x01000000
/* When an error is detected, the address where the error has been found,
   and also the current and the expected data will be written to
   the following flash address
*/
#define TEST_FLASH_ADDR	0x40100000

/* Define GPIO ports to signal start of burst transfers and errors */
#ifdef CONFIG_LWMON
/* Use PD.8 to signal start of burst transfers */
#define GPIO1_DAT	(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddat)
#define GPIO1_BIT	0x0080
/* Configure PD.8 as general purpose output */
#define GPIO1_INIT \
	((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pdpar &= ~GPIO1_BIT; \
	((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddir |=  GPIO1_BIT;
/* Use PD.9 to signal error */
#define GPIO2_DAT	(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddat)
#define GPIO2_BIT	0x0040
/* Configure PD.9 as general purpose output */
#define GPIO2_INIT \
	((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pdpar &= ~GPIO2_BIT; \
	((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddir |=  GPIO2_BIT;
#endif /* CONFIG_LWMON */


static void test_prepare (void);
static int test_burst_start (unsigned long size, unsigned long pattern);
static void test_map_8M (unsigned long paddr, unsigned long vaddr, int cached);
static int test_mmu_is_on(void);
static void test_desc(unsigned long size);
static void test_error(char * step, volatile void * addr, unsigned long val, unsigned long pattern);
static void signal_init(void);
static void signal_start(void);
static void signal_error(void);
static void test_usage(void);

static unsigned long test_pattern [] = {
	0x00000000,
	0xffffffff,
	0x55555555,
	0xaaaaaaaa,
};


int test_burst (int argc, char * const argv[])
{
	unsigned long size = CACHE_LINE_SIZE;
	unsigned int pass = 0;
	int res = 0;
	int i, j;

	if (argc == 3) {
		char * d;
		for (size = 0, d = argv[1]; *d >= '0' && *d <= '9'; d++) {
			size *= 10;
			size += *d - '0';
		}
		if (size == 0 || *d) {
			test_usage();
			return 1;
		}
		for (d = argv[2]; *d >= '0' && *d <= '9'; d++) {
			pass *= 10;
			pass += *d - '0';
		}
		if (*d) {
			test_usage();
			return 1;
		}
	} else if (argc > 3) {
		test_usage();
		return 1;
	}

	size +=  (CACHE_LINE_SIZE - 1);
	size &= ~(CACHE_LINE_SIZE - 1);

	if (!test_mmu_is_on()) {
		test_prepare();
	}

	test_desc(size);

	for (j = 0; !pass || j < pass; j++) {
		for (i = 0; i < sizeof(test_pattern) / sizeof(test_pattern[0]);
		     i++) {
			res = test_burst_start(size, test_pattern[i]);
			if (res != 0) {
				goto Done;
			}
		}

		printf ("Iteration #%d passed\n", j + 1);

		if (tstc() && 0x03 == getc())
			break;
	}
Done:
	return res;
}

static void test_prepare (void)
{
	printf ("\n");

	caches_init();
	disable_interrupts();
	mmu_init();

	printf ("Interrupts are disabled\n");
	printf ("I-Cache is ON\n");
	printf ("D-Cache is ON\n");
	printf ("MMU is ON\n");

	printf ("\n");

	test_map_8M (TEST_PADDR, TEST_VADDR_NC, 0);
	test_map_8M (TEST_PADDR, TEST_VADDR_C,  1);

	test_map_8M (TEST_FLASH_ADDR & 0xFF800000, TEST_FLASH_ADDR & 0xFF800000, 0);

	/* Configure GPIO ports */
	signal_init();
}

static int test_burst_start (unsigned long size, unsigned long pattern)
{
	volatile unsigned long * vaddr_c = (unsigned long *)TEST_VADDR_C;
	volatile unsigned long * vaddr_nc = (unsigned long *)TEST_VADDR_NC;
	int i, n;
	int res = 1;

	printf ("Test pattern %08lx ...", pattern);

	n = size / 4;

	for (i = 0; i < n; i ++) {
		vaddr_c [i] = pattern;
	}
	signal_start();
	flush_dcache_range((unsigned long)vaddr_c, (unsigned long)(vaddr_c + n) - 1);

	for (i = 0; i < n; i ++) {
		register unsigned long tmp = vaddr_nc [i];
		if (tmp != pattern) {
			test_error("2a", vaddr_nc + i, tmp, pattern);
			goto Done;
		}
	}

	for (i = 0; i < n; i ++) {
		register unsigned long tmp = vaddr_c [i];
		if (tmp != pattern) {
			test_error("2b", vaddr_c + i, tmp, pattern);
			goto Done;
		}
	}

	for (i = 0; i < n; i ++) {
		vaddr_nc [i] = pattern;
	}

	for (i = 0; i < n; i ++) {
		register unsigned long tmp = vaddr_nc [i];
		if (tmp != pattern) {
			test_error("3a", vaddr_nc + i, tmp, pattern);
			goto Done;
		}
	}

	signal_start();
	for (i = 0; i < n; i ++) {
		register unsigned long tmp = vaddr_c [i];
		if (tmp != pattern) {
			test_error("3b", vaddr_c + i, tmp, pattern);
			goto Done;
		}
	}

	res = 0;
Done:
	printf(" %s\n", res == 0 ? "OK" : "");

	return res;
}

static void test_map_8M (unsigned long paddr, unsigned long vaddr, int cached)
{
	mtspr (MD_EPN, (vaddr & 0xFFFFFC00) | MI_EVALID);
	mtspr (MD_TWC, MI_PS8MEG | MI_SVALID);
	mtspr (MD_RPN, (paddr & 0xFFFFF000) | MI_BOOTINIT | (cached ? 0 : 2));
	mtspr (MD_AP, MI_Kp);
}

static int test_mmu_is_on(void)
{
	unsigned long msr;

	asm volatile("mfmsr %0" : "=r" (msr) :);

	return msr & MSR_DR;
}

static void test_desc(unsigned long size)
{
	printf(
	"The following tests will be conducted:\n"
	"1)  Map %ld-byte region of physical RAM at 0x%08x\n"
	"    into two virtual regions:\n"
	"    one cached at 0x%08x and\n"
	"    the the other uncached at 0x%08x.\n",
	size, TEST_PADDR, TEST_VADDR_NC, TEST_VADDR_C);

	puts(
	"2)  Fill the cached region with a pattern, and flush the cache\n"
	"2a) Check the uncached region to match the pattern\n"
	"2b) Check the cached region to match the pattern\n"
	"3)  Fill the uncached region with a pattern\n"
	"3a) Check the cached region to match the pattern\n"
	"3b) Check the uncached region to match the pattern\n"
	"2b) Change the patterns and go to step 2\n"
	"\n"
	);
}

static void test_error(
	char * step, volatile void * addr, unsigned long val, unsigned long pattern)
{
	volatile unsigned long * p = (void *)TEST_FLASH_ADDR;

	signal_error();

	p[0] = (unsigned long)addr;
	p[1] = val;
	p[2] = pattern;

	printf ("\nError at step %s, addr %08lx: read %08lx, pattern %08lx",
		step, (unsigned long)addr, val, pattern);
}

static void signal_init(void)
{
#if defined(GPIO1_INIT)
	GPIO1_INIT;
#endif
#if defined(GPIO2_INIT)
	GPIO2_INIT;
#endif
}

static void signal_start(void)
{
#if defined(GPIO1_INIT)
	if (GPIO1_DAT & GPIO1_BIT) {
		GPIO1_DAT &= ~GPIO1_BIT;
	} else {
		GPIO1_DAT |= GPIO1_BIT;
	}
#endif
}

static void signal_error(void)
{
#if defined(GPIO2_INIT)
	if (GPIO2_DAT & GPIO2_BIT) {
		GPIO2_DAT &= ~GPIO2_BIT;
	} else {
		GPIO2_DAT |= GPIO2_BIT;
	}
#endif
}

static void test_usage(void)
{
	printf("Usage: go 0x40004 [size] [count]\n");
}
