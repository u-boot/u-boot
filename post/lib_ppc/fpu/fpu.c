/*
 * Copyright (C) 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Sergei Poselenov <sposelenov@emcraft.com>
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
 * FPU test
 *
 * This test checks the arithmetic logic unit (ALU) of CPU.
 * It tests independently various groups of instructions using
 * run-time modification of the code to reduce the memory footprint.
 * For more details refer to post/cpu/ *.c files.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_FPU

#include <watchdog.h>

GNU_FPOST_ATTR

extern int fpu_status (void);
extern void fpu_enable (void);
extern void fpu_disable (void);

extern int fpu_post_test_math1 (void);
extern int fpu_post_test_math2 (void);
extern int fpu_post_test_math3 (void);
extern int fpu_post_test_math4 (void);
extern int fpu_post_test_math5 (void);
extern int fpu_post_test_math6 (void);
extern int fpu_post_test_math7 (void);

int fpu_post_test (int flags)
{
	int fpu = fpu_status ();

	int ret = 0;

	WATCHDOG_RESET ();

	if (!fpu)
		fpu_enable ();

	if (ret == 0)
		ret = fpu_post_test_math1 ();
	if (ret == 0)
		ret = fpu_post_test_math2 ();
	if (ret == 0)
		ret = fpu_post_test_math3 ();
	if (ret == 0)
		ret = fpu_post_test_math4 ();
	if (ret == 0)
		ret = fpu_post_test_math5 ();
	if (ret == 0)
		ret = fpu_post_test_math6 ();
	if (ret == 0)
		ret = fpu_post_test_math7 ();

	if (!fpu)
		fpu_disable ();

	WATCHDOG_RESET ();

	return ret;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_FPU */
