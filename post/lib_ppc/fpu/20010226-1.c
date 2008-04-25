/*
 * Copyright (C) 2007
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
/*
 * This file is originally a part of the GCC testsuite.
 */

#include <common.h>

#include <post.h>

#if CONFIG_POST & CFG_POST_FPU

int fpu_post_test_math3 (void)
{
	volatile long double dfrom = 1.1;
	volatile long double m1;
	volatile long double m2;
	volatile unsigned long mant_long;

	m1 = dfrom / 2.0;
	m2 = m1 * 4294967296.0;
	mant_long = ((unsigned long) m2) & 0xffffffff;

	if (mant_long != 0x8ccccccc) {
		post_log ("Error in FPU math3 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CFG_POST_FPU */
