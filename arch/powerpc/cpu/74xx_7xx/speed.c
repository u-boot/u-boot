/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>
#include <74xx_7xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long get_board_bus_clk (void);

static const int hid1_multipliers_x_10[] = {
	25,	/* 0000 - 2.5x */
	75,	/* 0001 - 7.5x */
	70,	/* 0010 - 7x */
	10,	/* 0011 - bypass */
	20,	/* 0100 - 2x */
	65,	/* 0101 - 6.5x */
	100,	/* 0110 - 10x */
	45,	/* 0111 - 4.5x */
	30,	/* 1000 - 3x */
	55,	/* 1001 - 5.5x */
	40,	/* 1010 - 4x */
	50,	/* 1011 - 5x */
	80,	/* 1100 - 8x */
	60,	/* 1101 - 6x */
	35,	/* 1110 - 3.5x */
	0	/* 1111 - off */
};

/* PLL_CFG[0:4] table for cpu 7448/7447A/7455/7457 */
static const int hid1_74xx_multipliers_x_10[] = {
	115,	/* 00000 - 11.5x  */
	170,	/* 00001 - 17x    */
	75,	/* 00010 -  7.5x  */
	150,	/* 00011 - 15x    */
	70,	/* 00100 -  7x    */
	180,	/* 00101 - 18x    */
	10,	/* 00110 - bypass */
	200,	/* 00111 - 20x    */
	20,	/* 01000 -  2x    */
	210,	/* 01001 - 21x    */
	65,	/* 01010 -  6.5x  */
	130,	/* 01011 - 13x    */
	85,	/* 01100 -  8.5x  */
	240,	/* 01101 - 24x    */
	95,	/* 01110 -  9.5x  */
	90,	/* 01111 -  9x    */
	30,	/* 10000 -  3x    */
	105,	/* 10001 - 10.5x  */
	55,	/* 10010 -  5.5x  */
	110,	/* 10011 - 11x    */
	40,	/* 10100 -  4x    */
	100,	/* 10101 - 10x    */
	50,	/* 10110 -  5x    */
	120,	/* 10111 - 12x    */
	80,	/* 11000 -  8x    */
	140,	/* 11001 - 14x    */
	60,	/* 11010 -  6x    */
	160,	/* 11011 - 16x    */
	135,	/* 11100 - 13.5x  */
	280,	/* 11101 - 28x    */
	0,	/* 11110 - off    */
	125	/* 11111 - 12.5x  */
};

static const int hid1_fx_multipliers_x_10[] = {
	00,	/* 0000 - off */
	00,	/* 0001 - off */
	10,	/* 0010 - bypass */
	10,	/* 0011 - bypass */
	20,	/* 0100 - 2x */
	25,	/* 0101 - 2.5x */
	30,	/* 0110 - 3x */
	35,	/* 0111 - 3.5x */
	40,	/* 1000 - 4x */
	45,	/* 1001 - 4.5x */
	50,	/* 1010 - 5x */
	55,	/* 1011 - 5.5x */
	60,	/* 1100 - 6x */
	65,	/* 1101 - 6.5x */
	70,	/* 1110 - 7x */
	75,	/* 1111 - 7.5 */
	80,	/* 10000 - 8x */
	85,	/* 10001 - 8.5x */
	90,	/* 10010 - 9x */
	95,	/* 10011 - 9.5x */
	100,	/* 10100 - 10x */
	110,	/* 10101 - 11x */
	120,	/* 10110 - 12x */
};


/* ------------------------------------------------------------------------- */

/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 *
 * (Approx. GCLK frequency in Hz)
 */

int get_clocks (void)
{
	ulong clock = 0;

#ifdef CONFIG_SYS_BUS_CLK
	gd->bus_clk = CONFIG_SYS_BUS_CLK;	/* bus clock is a fixed frequency */
#else
	gd->bus_clk = get_board_bus_clk ();	/* bus clock is configurable */
#endif

	/* calculate the clock frequency based upon the CPU type */
	switch (get_cpu_type()) {
	case CPU_7447A:
	case CPU_7448:
	case CPU_7455:
	case CPU_7457:
		/*
		 * Make sure division is done before multiplication to prevent 32-bit
		 * arithmetic overflows which will cause a negative number
		 */
		clock = (gd->bus_clk / 10) *
			hid1_74xx_multipliers_x_10[(get_hid1 () >> 12) & 0x1F];
		break;

	case CPU_750GX:
	case CPU_750FX:
		clock = (gd->bus_clk / 10) *
			hid1_fx_multipliers_x_10[get_hid1 () >> 27];
		break;

	case CPU_7450:
	case CPU_740:
	case CPU_740P:
	case CPU_745:
	case CPU_750CX:
	case CPU_750:
	case CPU_750P:
	case CPU_755:
	case CPU_7400:
	case CPU_7410:
		/*
		 * Make sure division is done before multiplication to prevent 32-bit
		 * arithmetic overflows which will cause a negative number
		 */
		clock = (gd->bus_clk / 10) *
			hid1_multipliers_x_10[get_hid1 () >> 28];
		break;

	case CPU_UNKNOWN:
	       printf ("get_gclk_freq(): unknown CPU type\n");
	       clock = 0;
	       return (1);
	}

	gd->cpu_clk = clock;

	return (0);
}

/* ------------------------------------------------------------------------- */
