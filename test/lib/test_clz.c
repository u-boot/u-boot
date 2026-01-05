// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2025, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

int __clzsi2(int a);
int __clzdi2(long a);
int __clzti2(long long a);

/**
 * test_clz() - test library functions to count leading zero bits
 *
 * @uts:	unit test state
 */
static int test_clz(struct unit_test_state *uts)
{
	ut_asserteq(0, __clzti2(0xffffffffffffffffLL));
	ut_asserteq(0, __clzti2(0x8000000000000000LL));
	ut_asserteq(1, __clzti2(0x4000000000000000LL));
	ut_asserteq(17, __clzti2(0x0000500000a00000LL));
	ut_asserteq(62, __clzti2(0x0000000000000002LL));
	ut_asserteq(63, __clzti2(0x0000000000000001LL));

#if BITS_PER_LONG == 64
	ut_asserteq(0, __clzdi2(0xffffffffffffffffLL));
	ut_asserteq(0, __clzti2(0x8000000000000000LL));
	ut_asserteq(1, __clzti2(0x4000000000000000LL));
	ut_asserteq(17, __clzdi2(0x0000500000a00000LL));
	ut_asserteq(62, __clzdi2(0x0000000000000002LL));
	ut_asserteq(63, __clzdi2(0x0000000000000001LL));
#else
	ut_asserteq(0, __clzdi2(0xffffffff));
	ut_asserteq(0, __clzdi2(0x80000000));
	ut_asserteq(1, __clzdi2(0x40000000));
	ut_asserteq(9, __clzdi2(0x0050a000));
	ut_asserteq(30, __clzdi2(0x00000002));
	ut_asserteq(31, __clzdi2(0x00000001));
#endif

	ut_asserteq(0, __clzsi2(0xffffffff));
	ut_asserteq(0, __clzsi2(0x80000000));
	ut_asserteq(1, __clzsi2(0x40000000));
	ut_asserteq(9, __clzsi2(0x0050a000));
	ut_asserteq(30, __clzsi2(0x00000002));
	ut_asserteq(31, __clzsi2(0x00000001));

	return 0;
}
LIB_TEST(test_clz, 0);
