/*
 * U-boot - delay.h Routines for introducing delays
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BLACKFIN_DELAY_H
#define _BLACKFIN_DELAY_H

/*
 * Changes made by akbar.hussain@Lineo.com, for BLACKFIN
 * Copyright (C) 1994 Hamish Macdonald
 *
 * Delay routines, using a pre-computed "loops_per_second" value.
 */

static __inline__ void __delay(unsigned long loops)
{
	__asm__ __volatile__("1:\t%0 += -1;\n\t"
			     "cc = %0 == 0;\n\t"
			     "if ! cc jump 1b;\n":"=d"(loops)
			     :"0"(loops));
}

/*
 * Use only for very small delays ( < 1 msec).  Should probably use a
 * lookup table, really, as the multiplications take much too long with
 * short delays.  This is a "reasonable" implementation, though (and the
 * first constant multiplications gets optimized away if the delay is
 * a constant)
 */
static __inline__ void __udelay(unsigned long usecs)
{
	__delay(usecs);
}

#endif
