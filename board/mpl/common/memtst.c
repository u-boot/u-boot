/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 */

/* NOT Used yet...
  add following code to PIP405.c :
int testdram (void)
{
	unsigned char s[32];
	int i;

	i = getenv_r ("testmem", s, 32);
	if (i != 0) {
		i = (int) simple_strtoul (s, NULL, 10);
		if ((i > 0) && (i < 0xf)) {
			printf ("testing ");
			i = mem_test (0, ramsize, i);
			if (i > 0)
				printf ("ERROR ");
			else
				printf ("Ok ");
		}
	}
	return (1);
}
*/


#include <common.h>
#include <asm/processor.h>
#include <405gp_i2c.h>

#define FALSE           0
#define TRUE            1

#define TEST_QUIET 			8
#define TEST_SHOW_PROG 	4
#define TEST_SHOW_ERR 	2
#define TEST_SHOW_ALL		1

#define TESTPAT1 0xAA55AA55
#define TESTPAT2 0x55AA55AA
#define TEST_PASSED 0
#define TEST_FAILED 1
#define MEGABYTE (1024*1024)


typedef struct {
	volatile unsigned long pat1;
	volatile unsigned long pat2;
} RAM_MEMTEST_PATTERN2;

typedef struct {
	volatile unsigned long addr;
} RAM_MEMTEST_ADDRLINE;

static __inline unsigned long Swap_32 (unsigned long val)
{
	return (((val << 16) & 0xFFFF0000) | ((val >> 16) & 0x0000FFFF));
}

void testm_puts (int quiet, char *buf)
{
	if ((quiet & TEST_SHOW_ALL) == TEST_SHOW_ALL)
		puts (buf);
}


void Write_Error (int mode, unsigned long addr, unsigned long expected,
				  unsigned long actual)
{

	char dispbuf[64];

	sprintf (dispbuf, "\n ERROR @ 0x%08lX: (exp: 0x%08lX act: 0x%08lX) ",
			 addr, expected, actual);
	testm_puts (((mode & TEST_SHOW_ERR) ==
				 TEST_SHOW_ERR) ? TEST_SHOW_ALL : mode, dispbuf);
}


/*
 * fills the memblock of <size> bytes from <startaddr> with pat1 and pat2
 */


void RAM_MemTest_WritePattern2 (unsigned long startaddr,
								unsigned long size, unsigned long pat1,
								unsigned long pat2)
{
	RAM_MEMTEST_PATTERN2 *p, *pe;

	p = (RAM_MEMTEST_PATTERN2 *) startaddr;
	pe = (RAM_MEMTEST_PATTERN2 *) (startaddr + size);

	while (p < pe) {
		p->pat1 = pat1;
		p->pat2 = pat2;
		p++;
	}							/* endwhile */
}

/*
 * checks the memblock of <size> bytes from <startaddr> with pat1 and pat2
 * returns the address of the first error or NULL if all is well
 */

void *RAM_MemTest_CheckPattern2 (int mode, unsigned long startaddr,
								 unsigned long size, unsigned long pat1,
								 unsigned long pat2)
{
	RAM_MEMTEST_PATTERN2 *p, *pe;
	unsigned long actual1, actual2;

	p = (RAM_MEMTEST_PATTERN2 *) startaddr;
	pe = (RAM_MEMTEST_PATTERN2 *) (startaddr + size);

	while (p < pe) {
		actual1 = p->pat1;
		actual2 = p->pat2;

		if (actual1 != pat1) {
			Write_Error (mode, (unsigned long) &(p->pat1), pat1, actual1);
			return ((void *) &(p->pat1));
		}
		/* endif */
		if (actual2 != pat2) {
			Write_Error (mode, (unsigned long) &(p->pat2), pat2, actual2);
			return ((void *) &(p->pat2));
		}
		/* endif */
		p++;
	}							/* endwhile */

	return (NULL);
}

/*
 * fills the memblock of <size> bytes from <startaddr> with the address
 */

void RAM_MemTest_WriteAddrLine (unsigned long startaddr,
								unsigned long size, int swapped)
{
	RAM_MEMTEST_ADDRLINE *p, *pe;

	p = (RAM_MEMTEST_ADDRLINE *) startaddr;
	pe = (RAM_MEMTEST_ADDRLINE *) (startaddr + size);

	if (!swapped) {
		while (p < pe) {
			p->addr = (unsigned long) p;
			p++;
		}						/* endwhile */
	} else {
		while (p < pe) {
			p->addr = Swap_32 ((unsigned long) p);
			p++;
		}						/* endwhile */
	}							/* endif */
}

/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */

void *RAM_MemTest_CheckAddrLine (int mode, unsigned long startaddr,
								 unsigned long size, int swapped)
{
	RAM_MEMTEST_ADDRLINE *p, *pe;
	unsigned long actual, expected;

	p = (RAM_MEMTEST_ADDRLINE *) startaddr;
	pe = (RAM_MEMTEST_ADDRLINE *) (startaddr + size);

	if (!swapped) {
		while (p < pe) {
			actual = p->addr;
			expected = (unsigned long) p;
			if (actual != expected) {
				Write_Error (mode, (unsigned long) &(p->addr), expected,
							 actual);
				return ((void *) &(p->addr));
			}					/* endif */
			p++;
		}						/* endwhile */
	} else {
		while (p < pe) {
			actual = p->addr;
			expected = Swap_32 ((unsigned long) p);
			if (actual != expected) {
				Write_Error (mode, (unsigned long) &(p->addr), expected,
							 actual);
				return ((void *) &(p->addr));
			}					/* endif */
			p++;
		}						/* endwhile */
	}							/* endif */

	return (NULL);
}

/*
 * checks the memblock of <size> bytes from <startaddr+size>
 * returns the address of the error or NULL if all is well
 */

void *RAM_MemTest_CheckAddrLineReverse (int mode, unsigned long startaddr,
										unsigned long size, int swapped)
{
	RAM_MEMTEST_ADDRLINE *p, *pe;
	unsigned long actual, expected;

	p = (RAM_MEMTEST_ADDRLINE *) (startaddr + size - sizeof (p->addr));
	pe = (RAM_MEMTEST_ADDRLINE *) startaddr;

	if (!swapped) {
		while (p > pe) {
			actual = p->addr;
			expected = (unsigned long) p;
			if (actual != expected) {
				Write_Error (mode, (unsigned long) &(p->addr), expected,
							 actual);
				return ((void *) &(p->addr));
			}					/* endif */
			p--;
		}						/* endwhile */
	} else {
		while (p > pe) {
			actual = p->addr;
			expected = Swap_32 ((unsigned long) p);
			if (actual != expected) {
				Write_Error (mode, (unsigned long) &(p->addr), expected,
							 actual);
				return ((void *) &(p->addr));
			}					/* endif */
			p--;
		}						/* endwhile */
	}							/* endif */

	return (NULL);
}

/*
 * fills the memblock of <size> bytes from <startaddr> with walking bit pattern
 */

void RAM_MemTest_WriteWalkBit (unsigned long startaddr, unsigned long size)
{
	volatile unsigned long *p, *pe;
	unsigned long i;

	p = (unsigned long *) startaddr;
	pe = (unsigned long *) (startaddr + size);
	i = 0;

	while (p < pe) {
		*p = 1UL << i;
		i = (i + 1 + (((unsigned long) p) >> 7)) % 32;
		p++;
	}							/* endwhile */
}

/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */

void *RAM_MemTest_CheckWalkBit (int mode, unsigned long startaddr,
								unsigned long size)
{
	volatile unsigned long *p, *pe;
	unsigned long actual, expected;
	unsigned long i;

	p = (unsigned long *) startaddr;
	pe = (unsigned long *) (startaddr + size);
	i = 0;

	while (p < pe) {
		actual = *p;
		expected = (1UL << i);
		if (actual != expected) {
			Write_Error (mode, (unsigned long) p, expected, actual);
			return ((void *) p);
		}						/* endif */
		i = (i + 1 + (((unsigned long) p) >> 7)) % 32;
		p++;
	}							/* endwhile */

	return (NULL);
}

/*
 * fills the memblock of <size> bytes from <startaddr> with "random" pattern
 */

void RAM_MemTest_WriteRandomPattern (unsigned long startaddr,
									 unsigned long size,
									 unsigned long *pat)
{
	unsigned long i, p;

	p = *pat;

	for (i = 0; i < (size / 4); i++) {
		*(unsigned long *) (startaddr + i * 4) = p;
		if ((p % 2) > 0) {
			p ^= i;
			p >>= 1;
			p |= 0x80000000;
		} else {
			p ^= ~i;
			p >>= 1;
		}						/* endif */
	}							/* endfor */
	*pat = p;
}

/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */

void *RAM_MemTest_CheckRandomPattern (int mode, unsigned long startaddr,
									  unsigned long size,
									  unsigned long *pat)
{
	void *perr = NULL;
	unsigned long i, p, p1;

	p = *pat;

	for (i = 0; i < (size / 4); i++) {
		p1 = *(unsigned long *) (startaddr + i * 4);
		if (p1 != p) {
			if (perr == NULL) {
				Write_Error (mode, startaddr + i * 4, p, p1);
				perr = (void *) (startaddr + i * 4);
			}					/* endif */
		}
		/* endif */
		if ((p % 2) > 0) {
			p ^= i;
			p >>= 1;
			p |= 0x80000000;
		} else {
			p ^= ~i;
			p >>= 1;
		}						/* endif */
	}							/* endfor */

	*pat = p;
	return (perr);
}


void RAM_MemTest_WriteData1 (unsigned long startaddr, unsigned long size,
							 unsigned long *pat)
{
	RAM_MemTest_WritePattern2 (startaddr, size, TESTPAT1, TESTPAT2);
}

void *RAM_MemTest_CheckData1 (int mode, unsigned long startaddr,
							  unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckPattern2
			(mode, startaddr, size, TESTPAT1, TESTPAT2));
}

void RAM_MemTest_WriteData2 (unsigned long startaddr, unsigned long size,
							 unsigned long *pat)
{
	RAM_MemTest_WritePattern2 (startaddr, size, TESTPAT2, TESTPAT1);
}

void *RAM_MemTest_CheckData2 (int mode, unsigned long startaddr,
							  unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckPattern2
			(mode, startaddr, size, TESTPAT2, TESTPAT1));
}

void RAM_MemTest_WriteAddr1 (unsigned long startaddr, unsigned long size,
							 unsigned long *pat)
{
	RAM_MemTest_WriteAddrLine (startaddr, size, FALSE);
}

void *RAM_MemTest_Check1Addr1 (int mode, unsigned long startaddr,
							   unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckAddrLine (mode, startaddr, size, FALSE));
}

void *RAM_MemTest_Check2Addr1 (int mode, unsigned long startaddr,
							   unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckAddrLineReverse
			(mode, startaddr, size, FALSE));
}

void RAM_MemTest_WriteAddr2 (unsigned long startaddr, unsigned long size,
							 unsigned long *pat)
{
	RAM_MemTest_WriteAddrLine (startaddr, size, TRUE);
}

void *RAM_MemTest_Check1Addr2 (int mode, unsigned long startaddr,
							   unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckAddrLine (mode, startaddr, size, TRUE));
}

void *RAM_MemTest_Check2Addr2 (int mode, unsigned long startaddr,
							   unsigned long size, unsigned long *pat)
{
	return (RAM_MemTest_CheckAddrLineReverse
			(mode, startaddr, size, TRUE));
}


typedef struct {
	void (*test_write) (unsigned long startaddr, unsigned long size,
						unsigned long *pat);
	char *test_write_desc;
	void *(*test_check1) (int mode, unsigned long startaddr,
						  unsigned long size, unsigned long *pat);
	void *(*test_check2) (int mode, unsigned long startaddr,
						  unsigned long size, unsigned long *pat);
} RAM_MEMTEST_FUNC;


#define TEST_STAGES 5
static RAM_MEMTEST_FUNC test_stage[TEST_STAGES] = {
	{RAM_MemTest_WriteData1, "data test 1...\n", RAM_MemTest_CheckData1,
	 NULL},
	{RAM_MemTest_WriteData2, "data test 2...\n", RAM_MemTest_CheckData2,
	 NULL},
	{RAM_MemTest_WriteAddr1, "address line test...\n",
	 RAM_MemTest_Check1Addr1, RAM_MemTest_Check2Addr1},
	{RAM_MemTest_WriteAddr2, "address line test (swapped)...\n",
	 RAM_MemTest_Check1Addr2, RAM_MemTest_Check2Addr2},
	{RAM_MemTest_WriteRandomPattern, "random data test...\n",
	 RAM_MemTest_CheckRandomPattern, NULL}
};

void mem_test_reloc(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned long addr;
	int i;
	for (i=0; i< TEST_STAGES; i++) {
 		addr = (ulong) (test_stage[i].test_write) + gd->reloc_off;
		test_stage[i].test_write=
			(void (*) (unsigned long startaddr, unsigned long size,
						unsigned long *pat))addr;
 		addr = (ulong) (test_stage[i].test_write_desc) + gd->reloc_off;
		test_stage[i].test_write_desc=(char *)addr;
 		if(test_stage[i].test_check1) {
			addr = (ulong) (test_stage[i].test_check1) + gd->reloc_off;
			test_stage[i].test_check1=
				(void *(*) (int mode, unsigned long startaddr,
				 unsigned long size, unsigned long *pat))addr;
		}
 		if(test_stage[i].test_check2) {
			addr = (ulong) (test_stage[i].test_check2) + gd->reloc_off;
			test_stage[i].test_check2=
				(void *(*) (int mode, unsigned long startaddr,
				 unsigned long size, unsigned long *pat))addr;
		}
	}
}


int mem_test (unsigned long start, unsigned long ramsize, int quiet)
{
	unsigned long errors, stage;
	unsigned long startaddr, size, i;
	const unsigned long blocksize = 0x80000;	/* check in 512KB blocks */
	unsigned long *perr;
	unsigned long rdatapat;
	char dispbuf[80];
	int status = TEST_PASSED;
	int prog = 0;

	errors = 0;
	startaddr = start;
	size = ramsize;
	if ((quiet & TEST_SHOW_PROG) == TEST_SHOW_PROG) {
		prog++;
		printf (".");
	}
	sprintf (dispbuf, "\nMemory Test: addr = 0x%lx size = 0x%lx\n",
			 startaddr, size);
	testm_puts (quiet, dispbuf);
	for (stage = 0; stage < TEST_STAGES; stage++) {
		sprintf (dispbuf, test_stage[stage].test_write_desc);
		testm_puts (quiet, dispbuf);
		/* fill SDRAM */
		rdatapat = 0x12345678;
		sprintf (dispbuf, "writing block:     ");
		testm_puts (quiet, dispbuf);
		for (i = 0; i < size; i += blocksize) {
			sprintf (dispbuf, "%04lX\b\b\b\b", i / blocksize);
			testm_puts (quiet, dispbuf);
			test_stage[stage].test_write (startaddr + i, blocksize,
										  &rdatapat);
		}						/* endfor */
		sprintf (dispbuf, "\n");
		testm_puts (quiet, dispbuf);
		if ((quiet & TEST_SHOW_PROG) == TEST_SHOW_PROG) {
			prog++;
			printf (".");
		}
		/* check SDRAM */
		rdatapat = 0x12345678;
		sprintf (dispbuf, "checking block:     ");
		testm_puts (quiet, dispbuf);
		for (i = 0; i < size; i += blocksize) {
			sprintf (dispbuf, "%04lX\b\b\b\b", i / blocksize);
			testm_puts (quiet, dispbuf);
			if ((perr =
				 test_stage[stage].test_check1 (quiet, startaddr + i,
												blocksize,
												&rdatapat)) != NULL) {
				status = TEST_FAILED;
			}					/* endif */
		}						/* endfor */
		sprintf (dispbuf, "\n");
		testm_puts (quiet, dispbuf);
		if ((quiet & TEST_SHOW_PROG) == TEST_SHOW_PROG) {
			prog++;
			printf (".");
		}
		if (test_stage[stage].test_check2 != NULL) {
			/* check2 SDRAM */
			sprintf (dispbuf, "2nd checking block:     ");
			rdatapat = 0x12345678;
			testm_puts (quiet, dispbuf);
			for (i = 0; i < size; i += blocksize) {
				sprintf (dispbuf, "%04lX\b\b\b\b", i / blocksize);
				testm_puts (quiet, dispbuf);
				if ((perr =
					 test_stage[stage].test_check2 (quiet, startaddr + i,
													blocksize,
													&rdatapat)) != NULL) {
					status = TEST_FAILED;
				}				/* endif */
			}					/* endfor */
			sprintf (dispbuf, "\n");
			testm_puts (quiet, dispbuf);
			if ((quiet & TEST_SHOW_PROG) == TEST_SHOW_PROG) {
				prog++;
				printf (".");
			}
		}

	}							/* next stage */
	if ((quiet & TEST_SHOW_PROG) == TEST_SHOW_PROG) {
		while (prog-- > 0)
			printf ("\b \b");
	}

	if (status == TEST_FAILED)
		errors++;

	return (errors);
}
