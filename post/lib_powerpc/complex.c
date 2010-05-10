/*
 * (C) Copyright 2002
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

#include <common.h>

/*
 * CPU test
 * Complex calculations
 *
 * The calculations in this test are just a combination of simpler
 * calculations, but probably under different timing conditions, etc.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CONFIG_SYS_POST_CPU

extern int cpu_post_complex_1_asm (int a1, int a2, int a3, int a4, int n);
extern int cpu_post_complex_2_asm (int x, int n);

  /*
   *     n
   *	SUM (a1 * a2 - a3) / a4 = n * result
   *    i=1
   */
static int cpu_post_test_complex_1 (void)
{
    int a1 = 666;
    int a2 = 667;
    int a3 = 668;
    int a4 = 66;
    int n = 100;
    int result = 6720; /* (a1 * a2 - a3) / a4 */

    if (cpu_post_complex_1_asm(a1, a2, a3, a4, n) != n * result)
    {
	return -1;
    }

    return 0;
}

  /*	(1 + x + x^2 + ... + x^n) * (1 - x) = 1 - x^(n+1)
   */
static int cpu_post_test_complex_2 (void)
{
    int ret = -1;
    int x;
    int n;
    int k;
    int left;
    int right;

    for (x = -8; x <= 8; x ++)
    {
	n = 9;

	left = cpu_post_complex_2_asm(x, n);
	left *= 1 - x;

	right = 1;
	for (k = 0; k <= n; k ++)
	{
	    right *= x;
	}
	right = 1 - right;

	if (left != right)
	{
	    goto Done;
	}
    }

    ret = 0;
    Done:

    return ret;
}

int cpu_post_test_complex (void)
{
    int ret = 0;
    int flag = disable_interrupts();

    if (ret == 0)
    {
	ret = cpu_post_test_complex_1();
    }

    if (ret == 0)
    {
	ret = cpu_post_test_complex_2();
    }

    if (ret != 0)
    {
	post_log ("Error at complex test !\n");
    }

    if (flag)
	enable_interrupts();

    return ret;
}

#endif
