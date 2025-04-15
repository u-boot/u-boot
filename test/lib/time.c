// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <errno.h>
#include <time.h>
#include <linux/delay.h>
#include <test/lib.h>
#include <test/ut.h>

static int test_get_timer(struct unit_test_state *uts)
{
	ulong base, start, next, diff;
	int iter;

	base = get_timer(0);
	start = get_timer(0);
	for (iter = 0; iter < 10; iter++) {
		do {
			next = get_timer(0);
		} while (start == next);

		if (start + 1 != next) {
			printf("%s: iter=%d, start=%lu, next=%lu, expected a difference of 1\n",
			       __func__, iter, start, next);
			return -EINVAL;
		}
		start++;
	}

	/*
	 * Check that get_timer(base) matches our elapsed time, allowing that
	 * an extra millisecond may have passed.
	 */
	diff = get_timer(base);
	if (diff != iter && diff != iter + 1) {
		printf("%s: expected get_timer(base) to match elapsed time: diff=%lu, expected=%d\n",
		       __func__, diff, iter);
			return -EINVAL;
	}

	return 0;
}
LIB_TEST(test_get_timer, 0);

static int test_timer_get_us(struct unit_test_state *uts)
{
	ulong prev, next, min = 1000000;
	long delta;
	int iter;

	/* Find the minimum delta we can measure, in microseconds */
	prev = timer_get_us();
	for (iter = 0; iter < 100; ) {
		next = timer_get_us();
		if (next != prev) {
			delta = next - prev;
			if (delta < 0) {
				printf("%s: timer_get_us() went backwards from %lu to %lu\n",
				       __func__, prev, next);
				return -EINVAL;
			} else if (delta != 0) {
				if (delta < min)
					min = delta;
				prev = next;
				iter++;
			}
		}
	}

	if (min != 1) {
		printf("%s: Minimum microsecond delta should be 1 but is %lu\n",
		       __func__, min);
		return -EINVAL;
	}

	return 0;
}
LIB_TEST(test_timer_get_us, 0);

static int test_time_comparison(struct unit_test_state *uts)
{
	ulong start_us, end_us, delta_us;
	long error;
	ulong start;

	start = get_timer(0);
	start_us = timer_get_us();
	while (get_timer(start) < 1000)
		;
	end_us = timer_get_us();
	delta_us = end_us - start_us;
	error = delta_us - 1000000;
	printf("%s: Microsecond time for 1 second: %lu, error = %ld\n",
	       __func__, delta_us, error);
	if (abs(error) > 1000)
		return -EINVAL;

	return 0;
}
LIB_TEST(test_time_comparison, 0);

static int test_udelay(struct unit_test_state *uts)
{
	long error;
	ulong start, delta;
	int iter;

	start = get_timer(0);
	for (iter = 0; iter < 1000; iter++)
		udelay(1000);
	delta = get_timer(start);
	error = delta - 1000;
	printf("%s: Delay time for 1000 udelay(1000): %lu ms, error = %ld\n",
	       __func__, delta, error);
	if (abs(error) > 100)
		return -EINVAL;

	return 0;
}
LIB_TEST(test_udelay, 0);
