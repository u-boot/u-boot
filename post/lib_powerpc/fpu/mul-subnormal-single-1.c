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
 * Check that certain subnormal numbers (formerly known as denormalized
 * numbers) are rounded to within 0.5 ulp.  PR other/14354.
 */

#include <common.h>

#include <post.h>

GNU_FPOST_ATTR

#if CONFIG_POST & CONFIG_SYS_POST_FPU

union uf
{
	unsigned int u;
	float f;
};

static float
u2f (unsigned int v)
{
	union uf u;
	u.u = v;
	return u.f;
}

static unsigned int
f2u (float v)
{
	union uf u;
	u.f = v;
	return u.u;
}

static int ok = 1;

static void
tstmul (unsigned int ux, unsigned int uy, unsigned int ur)
{
	float x = u2f (ux);
	float y = u2f (uy);

	if (f2u (x * y) != ur)
	/* Set a variable rather than aborting here, to simplify tracing when
	   several computations are wrong.  */
		ok = 0;
}

/* We don't want to make this const and static, or else we risk inlining
   causing the test to fold as constants at compile-time.  */
struct
{
  unsigned int p1, p2, res;
} static volatile expected[] =
{
	{0xfff, 0x3f800400, 0xfff},
	{0xf, 0x3fc88888, 0x17},
	{0xf, 0x3f844444, 0xf}
};

int fpu_post_test_math7 (void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(expected); i++)
	{
		tstmul (expected[i].p1, expected[i].p2, expected[i].res);
		tstmul (expected[i].p2, expected[i].p1, expected[i].res);
	}

	if (!ok) {
		post_log ("Error in FPU math7 test\n");
		return -1;
	}
	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
