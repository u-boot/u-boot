/*
 * (C) Copyright 2005 Freescale Semiconductor, Inc.
 *
 * Roy Zang <tie-fei.zang@freescale.com>
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
 *
 * modifications for the Tsi108 Emul Board by avb@Tundra
 */

/*
 * board support/init functions for the 
 * Freescale MPC7448 HPC2 (High-Performance Computing 2 Platform).
 */

#include <common.h>
#include <74xx_7xx.h>
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
extern void ft_cpu_setup(void *blob, bd_t *bd);
#endif

#undef	DEBUG

extern void flush_data_cache(void);
extern void invalidate_l1_instruction_cache(void);
extern void tsi108_init_f(void);

int display_mem_map(void);

void after_reloc(ulong dest_addr)
{
	DECLARE_GLOBAL_DATA_PTR;
	
	/*
	 * Jump to the main U-Boot board init code
	 */
	board_init_r((gd_t *) gd, dest_addr);
	/* NOTREACHED */
}

/*
 * Check Board Identity:
 *
 * report board type
 */

int checkboard(void)
{
	int l_type = 0;

	printf("BOARD: %s\n", CFG_BOARD_NAME);
	return (l_type);
}

/*
 * Read Processor ID:
 *
 * report calling processor number
 */

int read_pid(void)
{
	return 0;		/* we are on single CPU platform for a while */
}

long int dram_size(int board_type)
{
	return 0x20000000;	/* 256M bytes */
}

long int initdram(int board_type)
{
	return dram_size(board_type);
}

/* DRAM check routines copied from gw8260 */

#if defined (CFG_DRAM_TEST)

/*********************************************************************/
/* NAME:  move64() -  moves a double word (64-bit)		     */
/*								     */
/* DESCRIPTION:							     */
/*   this function performs a double word move from the data at	     */
/*   the source pointer to the location at the destination pointer.  */
/*								     */
/* INPUTS:							     */
/*   unsigned long long *src  - pointer to data to move		     */
/*								     */
/* OUTPUTS:							     */
/*   unsigned long long *dest - pointer to locate to move data	     */
/*								     */
/* RETURNS:							     */
/*   None							     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*   May cloober fr0.						     */
/*								     */
/*********************************************************************/
static void move64(unsigned long long *src, unsigned long long *dest)
{
	asm("lfd  0, 0(3)\n\t"	/* fpr0   =  *scr       */
	    "stfd 0, 0(4)"	/* *dest  =  fpr0       */
      : : :"fr0");		/* Clobbers fr0         */
	return;
}

#if defined (CFG_DRAM_TEST_DATA)

unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,
	0xccccccccccccccccULL,
	0xf0f0f0f0f0f0f0f0ULL,
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,
	0x00000000ffffffffULL,
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};

/*********************************************************************/
/* NAME:  mem_test_data() -  test data lines for shorts and opens    */
/*								     */
/* DESCRIPTION:							     */
/*   Tests data lines for shorts and opens by forcing adjacent data  */
/*   to opposite states. Because the data lines could be routed in   */
/*   an arbitrary manner the must ensure test patterns ensure that   */
/*   every case is tested. By using the following series of binary   */
/*   patterns every combination of adjacent bits is test regardless  */
/*   of routing.						     */
/*								     */
/*     ...101010101010101010101010				     */
/*     ...110011001100110011001100				     */
/*     ...111100001111000011110000				     */
/*     ...111111110000000011111111				     */
/*								     */
/*   Carrying this out, gives us six hex patterns as follows:	     */
/*								     */
/*     0xaaaaaaaaaaaaaaaa					     */
/*     0xcccccccccccccccc					     */
/*     0xf0f0f0f0f0f0f0f0					     */
/*     0xff00ff00ff00ff00					     */
/*     0xffff0000ffff0000					     */
/*     0xffffffff00000000					     */
/*								     */
/*   The number test patterns will always be given by:		     */
/*								     */
/*   log(base 2)(number data bits) = log2 (64) = 6		     */
/*								     */
/*   To test for short and opens to other signals on our boards. we  */
/*   simply							     */
/*   test with the 1's complemnt of the paterns as well.	     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern				     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*  Assumes only one one SDRAM bank				     */
/*								     */
/*********************************************************************/
int mem_test_data(void)
{
	unsigned long long *pmem = (unsigned long long *)CFG_MEMTEST_START;
	unsigned long long temp64;
	int num_patterns = sizeof(pattern) / sizeof(pattern[0]);
	int i;
	unsigned int hi, lo;

	for (i = 0; i < num_patterns; i++) {
		move64(&(pattern[i]), pmem);
		move64(pmem, &temp64);

		/* hi = (temp64>>32) & 0xffffffff;          */
		/* lo = temp64 & 0xffffffff;                */
		/* printf("\ntemp64 = 0x%08x%08x", hi, lo); */

		hi = (pattern[i] >> 32) & 0xffffffff;
		lo = pattern[i] & 0xffffffff;
		/* printf("\npattern[%d] = 0x%08x%08x", i, hi, lo);  */

		if (temp64 != pattern[i]) {
			printf("\n   Data Test Failed, pattern 0x%08x%08x",
			       hi, lo);
			return 1;
		}
	}

	return 0;
}
#endif	/* CFG_DRAM_TEST_DATA */

#if defined (CFG_DRAM_TEST_ADDRESS)
/*********************************************************************/
/* NAME:  mem_test_address() -	test address lines		     */
/*								     */
/* DESCRIPTION:							     */
/*   This function performs a test to verify that each word im	     */
/*   memory is uniquly addressable. The test sequence is as follows: */
/*								     */
/*   1) write the address of each word to each word.		     */
/*   2) verify that each location equals its address		     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern and address			     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_test_address(void)
{
	volatile unsigned int *pmem =
	    (volatile unsigned int *)CFG_MEMTEST_START;
	const unsigned int size = (CFG_MEMTEST_END - CFG_MEMTEST_START) / 4;
	unsigned int i;

	/* write address to each location */
	for (i = 0; i < size; i++) {
		pmem[i] = i;
	}

	/* verify each loaction */
	for (i = 0; i < size; i++) {
		if (pmem[i] != i) {
			printf("\n   Address Test Failed at 0x%x", i);
			return 1;
		}
	}
	return 0;
}
#endif				/* CFG_DRAM_TEST_ADDRESS */

#if defined (CFG_DRAM_TEST_WALK)
/*********************************************************************/
/* NAME:   mem_march() -  memory march				     */
/*								     */
/* DESCRIPTION:							     */
/*   Marches up through memory. At each location verifies rmask if   */
/*   read = 1. At each location write wmask if	write = 1. Displays  */
/*   failing address and pattern.				     */
/*								     */
/* INPUTS:							     */
/*   volatile unsigned long long * base - start address of test	     */
/*   unsigned int size - number of dwords(64-bit) to test	     */
/*   unsigned long long rmask - read verify mask		     */
/*   unsigned long long wmask - wrtie verify mask		     */
/*   short read - verifies rmask if read = 1			     */
/*   short write  - writes wmask if write = 1			     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern and address			     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_march(volatile unsigned long long *base,
	      unsigned int size,
	      unsigned long long rmask,
	      unsigned long long wmask, short read, short write)
{
	unsigned int i;
	unsigned long long temp;
	unsigned int hitemp, lotemp, himask, lomask;

	for (i = 0; i < size; i++) {
		if (read != 0) {
			/* temp = base[i]; */
			move64((unsigned long long *)&(base[i]), &temp);
			if (rmask != temp) {
				hitemp = (temp >> 32) & 0xffffffff;
				lotemp = temp & 0xffffffff;
				himask = (rmask >> 32) & 0xffffffff;
				lomask = rmask & 0xffffffff;

				printf("\n Walking one's test failed:	\ 
					address = 0x%08x," "\n\texpected \
					0x%08x%08x, found 0x%08x%08x", i << 3,\
					himask, lomask, hitemp, lotemp);
				return 1;
			}
		}
		if (write != 0) {
			/*  base[i] = wmask; */
			move64(&wmask, (unsigned long long *)&(base[i]));
		}
	}
	return 0;
}
#endif				/* CFG_DRAM_TEST_WALK */

/*********************************************************************/
/* NAME:   mem_test_walk() -  a simple walking ones test	     */
/*								     */
/* DESCRIPTION:							     */
/*   Performs a walking ones through entire physical memory. The     */
/*   test uses as series of memory marches, mem_march(), to verify   */
/*   and write the test patterns to memory. The test sequence is as  */
/*   follows:							     */
/*     1) march writing 0000...0001				     */
/*     2) march verifying 0000...0001  , writing  0000...0010	     */
/*     3) repeat step 2 shifting masks left 1 bit each time unitl    */
/*	   the write mask equals 1000...0000			     */
/*     4) march verifying 1000...0000				     */
/*   The test fails if any of the memory marches return a failure.   */
/*								     */
/* OUTPUTS:							     */
/*   Displays which pass on the memory test is executing	     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_test_walk(void)
{
	unsigned long long mask;
	volatile unsigned long long *pmem =
	    (volatile unsigned long long *)CFG_MEMTEST_START;
	const unsigned long size = (CFG_MEMTEST_END - CFG_MEMTEST_START) / 8;

	unsigned int i;

	mask = 0x01;

	printf("Initial Pass");
	mem_march(pmem, size, 0x0, 0x1, 0, 1);

	printf("\b\b\b\b\b\b\b\b\b\b\b\b");
	printf("		");
	printf("         ");
	printf("\b\b\b\b\b\b\b\b\b\b\b\b");

	for (i = 0; i < 63; i++) {
		printf("Pass %2d", i + 2);
		if (mem_march(pmem, size, mask, mask << 1, 1, 1) != 0) {
			/*printf("mask: 0x%x, pass: %d, ", mask, i); */
			return 1;
		}
		mask = mask << 1;
		printf("\b\b\b\b\b\b\b");
	}

	printf("Last Pass");
	if (mem_march(pmem, size, 0, mask, 0, 1) != 0) {
		/* printf("mask: 0x%x", mask); */
		return 1;
	}
	printf("\b\b\b\b\b\b\b\b\b");
	printf("	     ");
	printf("\b\b\b\b\b\b\b\b\b");

	return 0;
}

/*********************************************************************/
/* NAME:    testdram() -  calls any enabled memory tests	     */
/*								     */
/* DESCRIPTION:							     */
/*   Runs memory tests if the environment test variables are set to  */
/*   'y'.							     */
/*								     */
/* INPUTS:							     */
/*   testdramdata    - If set to 'y', data test is run.		     */
/*   testdramaddress - If set to 'y', address test is run.	     */
/*   testdramwalk    - If set to 'y', walking ones test is run	     */
/*								     */
/* OUTPUTS:							     */
/*   None							     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int testdram(void)
{
	char *s;
	int rundata, runaddress, runwalk;

	s = getenv("testdramdata");
	rundata = (s && (*s == 'y')) ? 1 : 0;
	s = getenv("testdramaddress");
	runaddress = (s && (*s == 'y')) ? 1 : 0;
	s = getenv("testdramwalk");
	runwalk = (s && (*s == 'y')) ? 1 : 0;

/*    rundata = 1; */
/*    runaddress = 0; */
/*    runwalk = 0; */

	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1)) {
		printf("Testing RAM from 0x%08x to 0x%08x ...  \
			(don't panic... that will take a moment !!!!)\n", \
			CFG_MEMTEST_START, CFG_MEMTEST_END);
	}
#ifdef CFG_DRAM_TEST_DATA
	if (rundata == 1) {
		printf("Test DATA ...  ");
		if (mem_test_data () == 1) {
			printf("failed \n");
			return 1;
		} else
			printf("ok \n");
	}
#endif
#ifdef CFG_DRAM_TEST_ADDRESS
	if (runaddress == 1) {
		printf("Test ADDRESS ...  ");
		if (mem_test_address () == 1) {
			printf("failed \n");
			return 1;
		} else
			printf("ok \n");
	}
#endif
#ifdef CFG_DRAM_TEST_WALK
	if (runwalk == 1) {
		printf("Test WALKING ONEs ...  ");
		if (mem_test_walk() == 1) {
			printf("failed \n");
			return 1;
		} else
			printf("ok \n");
	}
#endif
	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1)) {
		printf("passed\n");
	}
	return 0;

}
#endif /* CFG_DRAM_TEST */

#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;

	ft_cpu_setup(blob, bd);

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
}
#endif
