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

GNU_FPOST_ATTR

#if CONFIG_POST & CONFIG_SYS_POST_FPU

static double func (const double *array)
{
	double d = *array;

	if (d == 0.0)
		return d;
	else
		return d + func (array + 1);
}

int fpu_post_test_math5 (void)
{
	double values[] = { 0.1e-100, 1.0, -1.0, 0.0 };

	if (func (values) != 0.1e-100) {
		post_log ("Error in FPU math5 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
