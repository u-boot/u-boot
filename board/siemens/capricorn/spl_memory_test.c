// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright Siemens AG 2020
 *
 * SPL Full Memory Test
 * - memory test through the full DDR area
 * - refresh over temperature torture (write all, read all)
 *
 * Remark:
 * This test has ran properly with the definition of the RAM sizes in board
 * headers. Since these headers are removed it's necessary to set the correct
 * values to PHYS_SDRAM_1_SIZE & PHYS_SDRAM_2_SIZE before to recompile.
 *
 * An alternative is to refactor the code to get the size info from system
 * controller
 */

#include <init.h>
#include <log.h>

/* ----- Defines ----- */
#define CHECK_LOWER_UPPER

#define LEVEL2_PRINT    0x0FFFFFFF

/* use 0x7FFF0000 for shorter loop test */
#define BASE_OFFSET	0x00000000

/* ----- Types ----- */
struct ct_t {
	unsigned long *start;
	unsigned long *end;
};

/* ----- Variables ----- */
static struct ct_t ct;
static unsigned long error_counter;

static void print_parameters(void)
{
	printf("\nstart addr: %p\n", ct.start);
	printf("end addr  : %p\n", ct.end);
}

static void run_test(void)
{
	/* moved full test in one void */
	unsigned long *address; /* 512 */
	unsigned long ebyte1;
	unsigned long ebyte2;
	unsigned int i;
	unsigned long rpattern;

	for (i = 0; i <= 255; i++) {
		memset(&ebyte1, i, sizeof(ebyte1));
		ebyte2 = ~ebyte1;
		printf("LWord: %016lx  #LWord: %016lx\n", ebyte1, ebyte2);

		/* write  all bytes -> duration ~ 150 s */
		for (address = ct.start; address <= ct.end; address++) {
#ifdef LEVEL2_PRINT
			if (((unsigned long)address & LEVEL2_PRINT) == 0)
				printf("write to %p - %p\n", address,
				       (void *)((unsigned long)address +
				       LEVEL2_PRINT));
#endif
			*address = ebyte1;
			address++;
			*address = ebyte2;
		}

		/* check all bytes */
		for (address = ct.start; address <= ct.end; address++) {
#ifdef LEVEL2_PRINT
			if (((unsigned long)address & LEVEL2_PRINT) == 0)
				printf("check from %p - %p\n", address,
				       (void *)((unsigned long)address +
				       LEVEL2_PRINT));
#endif

			rpattern = *address;
			if (rpattern != ebyte1) {
				error_counter++;
				printf("Error! Read: %016lX Wrote: %016lX Address: %p\n",
				       rpattern, ebyte1, address);
			}

			address++;
			rpattern = *address;
			if (rpattern != ebyte2) {
				error_counter++;
				printf("Error! Read: %016lX Wrote: %016lX Address: %p\n",
				       rpattern, ebyte2, address);
			}
		}
	}
}

#ifdef CHECK_LOWER_UPPER
void test_lower_upper(void)
{
	/*
	 * write different values at the same address of both memory areas
	 * and check them
	 */
#define TEST_ADDRESS	 0x12345670UL
#define LOWER_ADDRESS	(PHYS_SDRAM_1 + TEST_ADDRESS)
#define UPPER_ADDRESS	(PHYS_SDRAM_2 + TEST_ADDRESS)
#define LOWER_VALUE	0x0011223344556677
#define UPPER_VALUE	0x89ab89abffeeddcc

	*(unsigned long *)LOWER_ADDRESS = LOWER_VALUE;
	*(unsigned long *)UPPER_ADDRESS = UPPER_VALUE;

	puts("\nlower-upper memory area test\n");
	printf("write %016lx to   lower address %010lx\n", LOWER_VALUE,
	       LOWER_ADDRESS);
	printf("write %016lx to   upper address %010lx\n", UPPER_VALUE,
	       UPPER_ADDRESS);
	printf("read  %016lx from lower address %010lx\n",
	       *(unsigned long *)LOWER_ADDRESS, LOWER_ADDRESS);
	printf("read  %016lx from upper address %010lx\n",
	       *(unsigned long *)UPPER_ADDRESS, UPPER_ADDRESS);
}
#endif

void spl_siemens_memory_full_test(void)
{
	unsigned long loopc = 0;

	puts("\nSPL: memory cell test\n");

#ifdef CHECK_LOWER_UPPER
	if (PHYS_SDRAM_2_SIZE != 0)
		test_lower_upper();
#endif

	while (true) {
		/* imx8x has 2 memory areas up to 2 GB */

		/* 1st memory area @ 0x80000000 */
		ct.start = (unsigned long *)(PHYS_SDRAM_1 + BASE_OFFSET);
		ct.end = (unsigned long *)(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE - 1);
		print_parameters();
		run_test();

		/* 2nd memory area @ 0x880000000 */
		if (PHYS_SDRAM_2_SIZE != 0) {
			ct.start = (unsigned long *)(PHYS_SDRAM_2 + BASE_OFFSET);
			ct.end = (unsigned long *)(PHYS_SDRAM_2 + PHYS_SDRAM_2_SIZE - 1);
			print_parameters();
			run_test();
		}

		loopc++;
		printf("loop: %ld, errors: %ld\n\n", loopc, error_counter);
	};
}
