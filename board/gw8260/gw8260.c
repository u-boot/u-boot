/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Jay Monkman <jtm@smoothsmoothie.com>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Oliver Brown <oliverb@alumni.utexas.net>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*********************************************************************/
/* DESCRIPTION:
 *   This file contains the board routines for the GW8260 board.
 *
 * MODULE DEPENDENCY:
 *   None
 *
 * RESTRICTIONS/LIMITATIONS:
 *   None
 *
 * Copyright (c) 2001, Advent Networks, Inc.
 */
/*********************************************************************/

#include <common.h>
#include <ioports.h>
#include <mpc8260.h>

/*
 * I/O Port configuration table
 *
 */
const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	       conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   0,	 0,   1,   0,	0   }, /* TP14		*/
	/* PA30 */ {   1,   1,	 1,   1,   0,	0   }, /* US_RTS	*/
	/* PA29 */ {   1,   0,	 0,   1,   0,	1   }, /* LSSI_DATA	*/
	/* PA28 */ {   1,   0,	 0,   1,   0,	1   }, /* LSSI_CLK	*/
	/* PA27 */ {   1,   0,	 0,   1,   0,	0   }, /* TP12		*/
	/* PA26 */ {   1,   0,	 0,   0,   0,	0   }, /* IO_STATUS	*/
	/* PA25 */ {   1,   0,	 0,   0,   0,	0   }, /* IO_CLOCK	*/
	/* PA24 */ {   1,   0,	 0,   0,   0,	0   }, /* IO_CONFIG	*/
	/* PA23 */ {   1,   0,	 0,   0,   0,	0   }, /* IO_DONE	*/
	/* PA22 */ {   1,   0,	 0,   0,   0,	0   }, /* IO_DATA	*/
	/* PA21 */ {   1,   1,	 0,   1,   0,	0   }, /* US_TXD3	*/
	/* PA20 */ {   1,   1,	 0,   1,   0,	0   }, /* US_TXD2	*/
	/* PA19 */ {   1,   1,	 0,   1,   0,	0   }, /* US_TXD1	*/
	/* PA18 */ {   1,   1,	 0,   1,   0,	0   }, /* US_TXD0	*/
	/* PA17 */ {   1,   1,	 0,   0,   0,	0   }, /* DS_RXD0	*/
	/* PA16 */ {   1,   1,	 0,   0,   0,	0   }, /* DS_RXD1	*/
	/* PA15 */ {   1,   1,	 0,   0,   0,	0   }, /* DS_RXD2	*/
	/* PA14 */ {   1,   1,	 0,   0,   0,	0   }, /* DS_RXD3	*/
	/* PA13 */ {   1,   0,	 0,   1,   0,	0   }, /* SPARE7	*/
	/* PA12 */ {   1,   0,	 0,   1,   0,	0   }, /* SPARE6	*/
	/* PA11 */ {   1,   0,	 0,   1,   0,	0   }, /* SPARE5	*/
	/* PA10 */ {   1,   0,	 0,   1,   0,	0   }, /* SPARE4	*/
	/* PA9	*/ {   1,   0,	 0,   1,   0,	0   }, /* SPARE3	*/
	/* PA8	*/ {   1,   0,	 0,   1,   0,	0   }, /* SPARE2	*/
	/* PA7	*/ {   1,   0,	 0,   0,   0,	0   }, /* LSSI_IN	*/
	/* PA6	*/ {   1,   0,	 0,   1,   0,	0   }, /* SPARE0	*/
	/* PA5	*/ {   1,   0,	 0,   1,   0,	0   }, /* DEMOD_RESET_	*/
	/* PA4	*/ {   1,   0,	 0,   1,   0,	0   }, /* MOD_RESET_	*/
	/* PA3	*/ {   1,   0,	 0,   1,   0,	0   }, /* IO_RESET	*/
	/* PA2	*/ {   1,   0,	 0,   1,   0,	0   }, /* TX_ENABLE	*/
	/* PA1	*/ {   1,   0,	 0,   0,   0,	0   }, /* RX_LOCK	*/
	/* PA0	*/ {   1,   0,	 0,   1,   0,	1   }  /* MPC_RESET_	*/
    },

    /* Port B configuration */
    {	/*	       conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH0_TX_ER */
	/* PB30 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RX_DV */
	/* PB29 */ {   1,   1,	 1,   1,   0,	0   }, /* FETH0_TX_EN */
	/* PB28 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RX_ER */
	/* PB27 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_COL   */
	/* PB26 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_CRS   */
	/* PB25 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH0_TXD3  */
	/* PB24 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH0_TXD2  */
	/* PB23 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH0_TXD1  */
	/* PB22 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH0_TXD0  */
	/* PB21 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RXD0  */
	/* PB20 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RXD1  */
	/* PB19 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RXD2  */
	/* PB18 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RXD3  */
	/* PB17 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RX_DV */
	/* PB16 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RX_ER */
	/* PB15 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TX_ER */
	/* PB14 */ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TX_EN */
	/* PB13 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_COL   */
	/* PB12 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_CRS   */
	/* PB11 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RXD3  */
	/* PB10 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RXD2  */
	/* PB9	*/ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RXD1  */
	/* PB8	*/ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RXD0  */
	/* PB7	*/ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TXD0  */
	/* PB6	*/ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TXD1  */
	/* PB5	*/ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TXD2  */
	/* PB4	*/ {   1,   1,	 0,   1,   0,	0   }, /* FETH1_TXD3  */
	/* PB3	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB2	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB1	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PB0	*/ {   0,   0,	 0,   0,   0,	0   }  /* pin doesn't exist */
    },

    /* Port C */
    {	/*	       conf ppar psor pdir podr pdat */
	/* PC31 */ {   1,   0,	 0,   1,   0,	1   }, /* FAST_RESET_	*/
	/* PC30 */ {   1,   0,	 0,   1,   0,	1   }, /* FAST_PAUSE_	*/
	/* PC29 */ {   1,   0,	 0,   1,   0,	0   }, /* FAST_SLEW1	*/
	/* PC28 */ {   1,   0,	 0,   1,   0,	0   }, /* FAST_SLEW0	*/
	/* PC27 */ {   1,   0,	 0,   1,   0,	0   }, /* TP13		*/
	/* PC26 */ {   1,   0,	 0,   0,   0,	0   }, /* RXDECDFLG	*/
	/* PC25 */ {   1,   0,	 0,   0,   0,	0   }, /* RXACQFAIL	*/
	/* PC24 */ {   1,   0,	 0,   0,   0,	0   }, /* RXACQFLG	*/
	/* PC23 */ {   1,   0,	 0,   1,   0,	0   }, /* WD_TCL	*/
	/* PC22 */ {   1,   0,	 0,   1,   0,	0   }, /* WD_EN		*/
	/* PC21 */ {   1,   0,	 0,   1,   0,	0   }, /* US_TXCLK	*/
	/* PC20 */ {   1,   0,	 0,   0,   0,	0   }, /* DS_RXCLK	*/
	/* PC19 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_RX_CLK	*/
	/* PC18 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH0_TX_CLK	*/
	/* PC17 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_RX_CLK	*/
	/* PC16 */ {   1,   1,	 0,   0,   0,	0   }, /* FETH1_TX_CLK	*/
	/* PC15 */ {   1,   0,	 0,   1,   0,	0   }, /* TX_SHUTDOWN_	*/
	/* PC14 */ {   1,   0,	 0,   0,   0,	0   }, /* RS_232_DTR_	*/
	/* PC13 */ {   1,   0,	 0,   0,   0,	0   }, /* TXERR		*/
	/* PC12 */ {   1,   0,	 0,   1,   0,	1   }, /* FETH1_MDDIS	*/
	/* PC11 */ {   1,   0,	 0,   1,   0,	1   }, /* FETH0_MDDIS	*/
	/* PC10 */ {   1,   0,	 0,   1,   0,	0   }, /* MDC		*/
	/* PC9	*/ {   1,   0,	 0,   1,   1,	1   }, /* MDIO		*/
	/* PC8	*/ {   1,   0,	 0,   1,   1,	1   }, /* SER_NUM	*/
	/* PC7	*/ {   1,   1,	 0,   0,   0,	0   }, /* US_CTS	*/
	/* PC6	*/ {   1,   1,	 0,   0,   0,	0   }, /* DS_CD_	*/
	/* PC5	*/ {   1,   0,	 0,   1,   0,	0   }, /* FETH1_PWRDWN	*/
	/* PC4	*/ {   1,   0,	 0,   1,   0,	0   }, /* FETH0_PWRDWN	*/
	/* PC3	*/ {   1,   0,	 0,   1,   0,	0   }, /* MPULED3	*/
	/* PC2	*/ {   1,   0,	 0,   1,   0,	0   }, /* MPULED2	*/
	/* PC1	*/ {   1,   0,	 0,   1,   0,	0   }, /* MPULED1	*/
	/* PC0	*/ {   1,   0,	 0,   1,   0,	1   }, /* MPULED0	*/
    },

    /* Port D */
    {	/*	       conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD30 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD29 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD28 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD27 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD26 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD25 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD24 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD23 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD22 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD21 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD20 */ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD19 */ {   1,   1,	 1,   0,   0,	0   }, /*  not used	*/
	/* PD18 */ {   1,   1,	 1,   0,   0,	0   }, /*  not used	*/
	/* PD17 */ {   1,   1,	 1,   0,   0,	0   }, /*  not used	*/
	/* PD16 */ {   1,   1,	 1,   0,   0,	0   }, /*  not used	*/
	/* PD15 */ {   1,   1,	 1,   0,   1,	1   }, /*  SDRAM_SDA	*/
	/* PD14 */ {   1,   1,	 1,   0,   1,	1   }, /*  SDRAM_SCL	*/
	/* PD13 */ {   1,   0,	 0,   1,   0,	0   }, /*  MPULED7	*/
	/* PD12 */ {   1,   0,	 0,   1,   0,	0   }, /*  MPULED6	*/
	/* PD11 */ {   1,   0,	 0,   1,   0,	0   }, /*  MPULED5	*/
	/* PD10 */ {   1,   0,	 0,   1,   0,	0   }, /*  MPULED4	*/
	/* PD9	*/ {   1,   1,	 0,   1,   0,	0   }, /*  RS232_TXD	*/
	/* PD8	*/ {   1,   1,	 0,   0,   0,	0   }, /*  RD232_RXD	*/
	/* PD7	*/ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD6	*/ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD5	*/ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD4	*/ {   1,   0,	 0,   0,   0,	0   }, /*  not used	*/
	/* PD3	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD2	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD1	*/ {   0,   0,	 0,   0,   0,	0   }, /* pin doesn't exist */
	/* PD0	*/ {   0,   0,	 0,   0,   0,	0   }  /* pin doesn't exist */
    }
};

/*********************************************************************/
/* NAME: checkboard() -	 Displays the board type and serial number   */
/*								     */
/* OUTPUTS:							     */
/*   Displays the board type and serial number			     */
/*								     */
/* RETURNS:							     */
/*   Always returns 1						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int checkboard (void)
{
	char *str;

	puts ("Board: Advent Networks gw8260\n");

	str = getenv ("serial#");
	if (str != NULL) {
		printf ("SN:    %s\n", str);
	}
	return 0;
}


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
static void move64 (unsigned long long *src, unsigned long long *dest)
{
	asm ("lfd  0, 0(3)\n\t"	/* fpr0   =  *scr       */
	     "stfd 0, 0(4)"	/* *dest  =  fpr0       */
      : : : "fr0");		/* Clobbers fr0         */
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
	0x5555555555555555ULL,
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
int mem_test_data (void)
{
	unsigned long long *pmem = (unsigned long long *) CFG_SDRAM_BASE;
	unsigned long long temp64 = 0;
	int num_patterns = sizeof (pattern) / sizeof (pattern[0]);
	int i;
	unsigned int hi, lo;

	for (i = 0; i < num_patterns; i++) {
		move64 (&(pattern[i]), pmem);
		move64 (pmem, &temp64);

		/* hi = (temp64>>32) & 0xffffffff;          */
		/* lo = temp64 & 0xffffffff;                */
		/* printf("\ntemp64 = 0x%08x%08x", hi, lo); */

		hi = (pattern[i] >> 32) & 0xffffffff;
		lo = pattern[i] & 0xffffffff;
		/* printf("\npattern[%d] = 0x%08x%08x", i, hi, lo);  */

		if (temp64 != pattern[i]) {
			printf ("\n   Data Test Failed, pattern 0x%08x%08x",
				hi, lo);
			return 1;
		}
	}

	return 0;
}
#endif /* CFG_DRAM_TEST_DATA */

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
int mem_test_address (void)
{
	volatile unsigned int *pmem =
		(volatile unsigned int *) CFG_SDRAM_BASE;
	const unsigned int size = (CFG_SDRAM_SIZE * 1024 * 1024) / 4;
	unsigned int i;

	/* write address to each location */
	for (i = 0; i < size; i++) {
		pmem[i] = i;
	}

	/* verify each loaction */
	for (i = 0; i < size; i++) {
		if (pmem[i] != i) {
			printf ("\n   Address Test Failed at 0x%x", i);
			return 1;
		}
	}
	return 0;
}
#endif /* CFG_DRAM_TEST_ADDRESS */

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
int mem_march (volatile unsigned long long *base,
	       unsigned int size,
	       unsigned long long rmask,
	       unsigned long long wmask, short read, short write)
{
	unsigned int i;
	unsigned long long temp = 0;
	unsigned int hitemp, lotemp, himask, lomask;

	for (i = 0; i < size; i++) {
		if (read != 0) {
			/* temp = base[i]; */
			move64 ((unsigned long long *) &(base[i]), &temp);
			if (rmask != temp) {
				hitemp = (temp >> 32) & 0xffffffff;
				lotemp = temp & 0xffffffff;
				himask = (rmask >> 32) & 0xffffffff;
				lomask = rmask & 0xffffffff;

				printf ("\n Walking one's test failed: address = 0x%08x," "\n\texpected 0x%08x%08x, found 0x%08x%08x", i << 3, himask, lomask, hitemp, lotemp);
				return 1;
			}
		}
		if (write != 0) {
			/*  base[i] = wmask; */
			move64 (&wmask, (unsigned long long *) &(base[i]));
		}
	}
	return 0;
}
#endif /* CFG_DRAM_TEST_WALK */

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
int mem_test_walk (void)
{
	unsigned long long mask;
	volatile unsigned long long *pmem =
		(volatile unsigned long long *) CFG_SDRAM_BASE;
	const unsigned long size = (CFG_SDRAM_SIZE * 1024 * 1024) / 8;

	unsigned int i;

	mask = 0x01;

	printf ("Initial Pass");
	mem_march (pmem, size, 0x0, 0x1, 0, 1);

	printf ("\b\b\b\b\b\b\b\b\b\b\b\b");
	printf ("		");
	printf ("\b\b\b\b\b\b\b\b\b\b\b\b");

	for (i = 0; i < 63; i++) {
		printf ("Pass %2d", i + 2);
		if (mem_march (pmem, size, mask, mask << 1, 1, 1) != 0) {
			/*printf("mask: 0x%x, pass: %d, ", mask, i); */
			return 1;
		}
		mask = mask << 1;
		printf ("\b\b\b\b\b\b\b");
	}

	printf ("Last Pass");
	if (mem_march (pmem, size, 0, mask, 0, 1) != 0) {
		/* printf("mask: 0x%x", mask); */
		return 1;
	}
	printf ("\b\b\b\b\b\b\b\b\b");
	printf ("	     ");
	printf ("\b\b\b\b\b\b\b\b\b");

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
int testdram (void)
{
	char *s;
	int rundata, runaddress, runwalk;

	s = getenv ("testdramdata");
	rundata = (s && (*s == 'y')) ? 1 : 0;
	s = getenv ("testdramaddress");
	runaddress = (s && (*s == 'y')) ? 1 : 0;
	s = getenv ("testdramwalk");
	runwalk = (s && (*s == 'y')) ? 1 : 0;

	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1)) {
		printf ("Testing RAM ... ");
	}
#ifdef CFG_DRAM_TEST_DATA
	if (rundata == 1) {
		if (mem_test_data () == 1) {
			return 1;
		}
	}
#endif
#ifdef CFG_DRAM_TEST_ADDRESS
	if (runaddress == 1) {
		if (mem_test_address () == 1) {
			return 1;
		}
	}
#endif
#ifdef CFG_DRAM_TEST_WALK
	if (runwalk == 1) {
		if (mem_test_walk () == 1) {
			return 1;
		}
	}
#endif
	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1)) {
		printf ("passed");
	}
	return 0;

}
#endif /* CFG_DRAM_TEST */

/*********************************************************************/
/* NAME: initdram() -  initializes SDRAM controller		     */
/*								     */
/* DESCRIPTION:							     */
/*   Initializes the MPC8260's SDRAM controller.		     */
/*								     */
/* INPUTS:							     */
/*   CFG_IMMR	    -  MPC8260 Internal memory map		     */
/*   CFG_SDRAM_BASE -  Physical start address of SDRAM		     */
/*   CFG_PSDMR -       SDRAM mode register			     */
/*   CFG_MPTPR -       Memory refresh timer prescaler register	     */
/*   CFG_SDRAM0_SIZE - SDRAM size				     */
/*								     */
/* RETURNS:							     */
/*   SDRAM size in bytes					     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0, *ramaddr = (uchar *) (CFG_SDRAM_BASE + 0x8);
	ulong psdmr = CFG_PSDMR;
	int i;

	/*
	 * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
	 *
	 * "At system reset, initialization software must set up the
	 *  programmable parameters in the memory controller banks registers
	 *  (ORx, BRx, P/LSDMR). After all memory parameters are configured,
	 *  system software should execute the following initialization sequence
	 *  for each SDRAM device.
	 *
	 *  1. Issue a PRECHARGE-ALL-BANKS command
	 *  2. Issue eight CBR REFRESH commands
	 *  3. Issue a MODE-SET command to initialize the mode register
	 *
	 *  The initial commands are executed by setting P/LSDMR[OP] and
	 *  accessing the SDRAM with a single-byte transaction."
	 *
	 * The appropriate BRx/ORx registers have already been set when we
	 * get here. The SDRAM can be accessed at the address CFG_SDRAM_BASE.
	 */

	memctl->memc_psrt = CFG_PSRT;
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++) {
		*ramaddr = c;
	}
	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;

	/* return total ram size */
	return (CFG_SDRAM0_SIZE * 1024 * 1024);
}

/*********************************************************************/
/*			   End of gw8260.c			     */
/*********************************************************************/
