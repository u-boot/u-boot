// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2025, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

int __ctzsi2(int a);
int __ctzdi2(long a);
int __ctzti2(long long a);

/**
 * test_ctz() - test library functions to count trailing zero bits
 *
 * @uts:	unit test state
 */
static int test_ctz(struct unit_test_state *uts)
{
	ut_asserteq(0, __ctzti2(0xffffffffffffffffLL));
	ut_asserteq(63, __ctzti2(0x8000000000000000LL));
	ut_asserteq(62, __ctzti2(0x4000000000000000LL));
	ut_asserteq(21, __ctzti2(0x0000500000a00000LL));
	ut_asserteq(1, __ctzti2(0x0000000000000002LL));
	ut_asserteq(0, __ctzti2(0x0000000000000001LL));

#if BITS_PER_LONG == 64
	ut_asserteq(0, __ctzdi2(0xffffffffffffffffLL));
	ut_asserteq(63, __ctzdi2(0x8000000000000000LL));
	ut_asserteq(62, __ctzdi2(0x4000000000000000LL));
	ut_asserteq(21, __ctzdi2(0x0000500000a00000LL));
	ut_asserteq(1, __ctzdi2(0x0000000000000002LL));
	ut_asserteq(0, __ctzdi2(0x0000000000000001LL));
#else
	ut_asserteq(0, __ctzdi2(0xffffffff));
	ut_asserteq(31, __ctzdi2(0x80000000));
	ut_asserteq(30, __ctzdi2(0x40000000));
	ut_asserteq(13, __ctzdi2(0x0050a000));
	ut_asserteq(1, __ctzdi2(0x00000002));
	ut_asserteq(0, __ctzdi2(0x00000001));
#endif

	ut_asserteq(0, __ctzsi2(0xffffffff));
	ut_asserteq(31, __ctzsi2(0x80000000));
	ut_asserteq(30, __ctzsi2(0x40000000));
	ut_asserteq(13, __ctzsi2(0x0050a000));
	ut_asserteq(1, __ctzsi2(0x00000002));
	ut_asserteq(0, __ctzsi2(0x00000001));

	return 0;
}
LIB_TEST(test_ctz, 0);
