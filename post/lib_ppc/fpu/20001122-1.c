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

#if CONFIG_POST & CONFIG_SYS_POST_FPU

GNU_FPOST_ATTR

int fpu_post_test_math1 (void)
{
	volatile double a, *p;
	double c, d;
	volatile double b;

	d = 1.0;
	p = &b;

	do
	{
		c = d;
		d = c * 0.5;
		b = 1 + d;
	} while (b != 1.0);

	a = 1.0 + c;

	if (a == 1.0) {
		post_log ("Error in FPU math1 test\n");
		return -1;
	}

	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
