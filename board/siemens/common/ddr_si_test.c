// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright Siemens AG 2023
 *
 * DDR signal integrity test
 *   Check signals on DDR lines
 *   - signals must be as fast as possible and generate long burst
 *   - signals must be unidirectional (to DDR or from DDR only)
 *
 *   Set pattern: define 2^n 32-bit patterns (up to 4)
 *   Addresses: must be multiple of 16 to avoid checks in loops
 *   Test functions
 *   - write: write pattern to memory area for iteration times
 *   - read: write pattern once to memory area, read for iteration times
 */

#include <command.h>
#include <exports.h>
#include <time.h>
#if CONFIG_IS_ENABLED(AM33XX)
#include <asm/arch-am33xx/hardware_am33xx.h>
#include <asm/arch-am33xx/cpu.h>
#include <asm/io.h>
#endif

/* enable some print for debugging */
#ifdef PR_DEBUG
	#define PDEBUG(fmt, args...) printf(fmt, ## args)
#else
	#define PDEBUG(fmt, args...)
#endif

/* define 4 32-bit patterns */
#define MAX_PTN_SIZE (128)
#define PTN_ARRAY_SIZE (MAX_PTN_SIZE / (8 * sizeof(u32)))

/* define test direction */
#define DIR_READ	0
#define DIR_WRITE	1

static union {
	u64 l[2];
	u32 s[4];
	} test_pattern;
static int num_ptn32;

#if CONFIG_IS_ENABLED(AM33XX)
static inline void wdt_disable(void)
{
	struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
}

static inline void wdt_enable(void)
{
	struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

	writel(0xBBBB, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x4444, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
}
#else /* ! */
static inline void wdt_disable(void) {}

static inline void wdt_enable(void) {}
#endif /* CONFIG_IS_ENABLED(AM33XX) */

static int do_ddr_set_ptn(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int i, n;

	if (argc < 1)
		return CMD_RET_USAGE;

	/* number of patterns: 2 exponent */
	n = argc - 1;
	if (n > PTN_ARRAY_SIZE || (n & (n - 1)))
		return CMD_RET_USAGE;
	num_ptn32 = n;

	/* get patterns */
	for (i = 0; i < n; i++)
		test_pattern.s[i] = simple_strtoul(argv[i + 1], NULL, 0);

	printf("Test pattern set\n");

	return CMD_RET_SUCCESS;
}

static int do_ddr_show_ptn(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	if (!num_ptn32) {
		printf("No pattern available\n");
	} else {
		u32 *buf = test_pattern.s;
		int len = num_ptn32;
		int i;

		printf("Pattern: ");
		for (i = 0 ; i < len; i++)
			printf("0x%08X ", *buf++);

		printf("\n");
	}

	return CMD_RET_SUCCESS;
}

static void ddr_read32(u64 start_addr, u64 n_word, unsigned long iter)
{
	while (iter--) {
		register volatile u32 *addr = (u32 *)start_addr;
		register u64 count = n_word;

		while (count) {
			(void)*addr++;
			PDEBUG("Read 0x%08X from 0x%p\n", val, addr - 1);
			count--;
		}
	}
}

static void ddr_read64(u64 start_addr, u64 n_word, unsigned long iter)
{
	while (iter--) {
		register volatile u64 *addr = (u64 *)start_addr;
		register u64 count = n_word;

		if (num_ptn32 == 4)
			count *= 2;

		/*
		 * 64 & 128 bit pattern. Increase the nummber of read
		 * commands in the loop to generate longer burst signal
		 */
		while (count) {
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			(void)*addr++;
			PDEBUG("Read 0x%016llX from 0x%p\n", val, addr - 1);
			/*
			 * underflow cannot happen since n_word = end -
			 * start, end & start addresses are checked to be
			 * multiple of 16
			 */
			count -= 8;
		}
	}
}

static void ddr_write32(u64 start_addr, u64 n_word, unsigned long iter)
{
	while (iter--) {
		register u32 *addr = (u32 *)start_addr;
		register u32 ptn = *test_pattern.s;
		register u64 count = n_word;

		while (count) {
			PDEBUG("Write 0x%08X to 0x%p\n", ptn, addr);
			*addr++ = ptn;
			count--;
		}
	}
}

static void ddr_write64(u64 start_addr, u64 n_word, unsigned long iter)
{
	while (iter--) {
		register u64 *addr = (u64 *)start_addr;
		register u64 ptnA = test_pattern.l[0];
		register u64 ptnB = test_pattern.l[1];
		register u64 count = n_word;

		if (num_ptn32 == 2)
			ptnB = ptnA;
		else
			count *= 2;

		/*
		 * 64 & 128 bit pattern. Increase the nummber of write
		 * commands in the loop to generate longer burst signal
		 */
		while (count) {
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnA, addr);
			*addr++ = ptnA;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnB, addr);
			*addr++ = ptnB;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnA, addr);
			*addr++ = ptnA;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnB, addr);
			*addr++ = ptnB;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnA, addr);
			*addr++ = ptnA;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnB, addr);
			*addr++ = ptnB;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnA, addr);
			*addr++ = ptnA;
			PDEBUG("Write 0x%016llX to 0x%p\n", ptnB, addr);
			*addr++ = ptnB;
			/*
			 * underflow cannot happen since n_word = end -
			 * start, end & start addresses are checked to be
			 * multiple of 16
			 */
			count -= 8;
		}
	}
}

static int do_ddr_si_test(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u64 start_addr, end_addr, n_word;
	u64 ts_start, ts_end;
	unsigned long iteration, wr_iter;
	int direction, i;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	/* get arguments */
	direction = strcmp(argv[0], "read") ? DIR_WRITE : DIR_READ;
	start_addr = simple_strtoul(argv[1], NULL, 0);
	end_addr = simple_strtoul(argv[2], NULL, 0);
	iteration = simple_strtoul(argv[3], NULL, 10);

	n_word = (end_addr - start_addr) / (num_ptn32 * 4);
	printf("\nDDR signal integrity %s test: start\n", argv[0]);
	/* checks */
	if (start_addr & 0xF) {
		printf("ERROR: start_address should be 16 bytes aligned\n\n");
		return CMD_RET_USAGE;
	}

	if (end_addr & 0xF) {
		printf("ERROR: end_address should be 16 bytes aligned\n\n");
		return CMD_RET_USAGE;
	}

	if (start_addr >= end_addr) {
		printf("ERROR: end_address is not bigger than start_address\n\n");
		return CMD_RET_USAGE;
	}

	if (!iteration) {
		printf("ERROR: no iteration specified\n\n");
		return CMD_RET_USAGE;
	}

	if (!num_ptn32) {
		printf("ERROR: no test pattern specified\n\n");
		return CMD_RET_USAGE;
	}

	/* print parameters */
	printf("start_address = 0x%016llX\n", start_addr);
	printf("end_address   = 0x%016llX\n", end_addr);
	printf("iterations = %lu\n", iteration);

	/* print pattern */
	printf("test pattern 0x");
	for (i = 0; i < num_ptn32; i++)
		printf("%08X", test_pattern.s[i]);

	printf("\n");

	wdt_disable();

	/* writing */
	printf("Writing..\n");
	ts_start = get_timer_us(0);

	if (direction == DIR_READ)
		wr_iter = 1;
	else
		wr_iter = iteration;

	if (num_ptn32 == 1)
		ddr_write32(start_addr, n_word, wr_iter);
	else
		ddr_write64(start_addr, n_word, wr_iter);

	ts_end = get_timer_us(0);

	/* reading */
	if (direction == DIR_READ) {
		printf("Reading..\n");
		/* we need read time, just overwrite */
		ts_start = get_timer_us(0);

		if (num_ptn32 == 1)
			ddr_read32(start_addr, n_word, iteration);
		else
			ddr_read64(start_addr, n_word, iteration);

		ts_end = get_timer_us(0);
	}

	wdt_enable();

	/* print stats */
	printf("DONE.");
	printf(" Bytes=%llu ", n_word * num_ptn32 * 4 * iteration);
	printf(" Time=%llu us ", ts_end - ts_start);
	printf("\nDDR signal integrity %s test: end\n", argv[0]);

	return CMD_RET_SUCCESS;
}

static char ddr_si_help_text[] =
	"- DDR signal integrity test\n\n"
	"ddr_si setptn <pattern> [<pattern>] : set [1,2,4] 32-bit patterns\n"
	"ddr_si showptn : show patterns\n"
	"ddr_si read <start> <end> <iterations> : run test for reading\n"
	"ddr_si write <start> <end> <iterations> : run test for writing\n"
	"\nWith\n"
	"\t<pattern>: 32-bit pattern in hex format\n"
	"\t<start>: test start address in hex format\n"
	"\t<end>: test end address in hex format\n"
	"\t<iterations>: number of iterations\n";

U_BOOT_CMD_WITH_SUBCMDS(ddr_si, "DDR si test", ddr_si_help_text,
			U_BOOT_SUBCMD_MKENT(setptn, 5, 0, do_ddr_set_ptn),
			U_BOOT_SUBCMD_MKENT(showptn, 1, 0, do_ddr_show_ptn),
			U_BOOT_SUBCMD_MKENT(read, 4, 0, do_ddr_si_test),
			U_BOOT_SUBCMD_MKENT(write, 4, 0, do_ddr_si_test));
