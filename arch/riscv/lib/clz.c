// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * libgcc replacement - count leading bits
 *
 * Copyright 2025, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <linux/types.h>

/**
 * __clzti2() - count number of leading zero bits
 *
 * @x:		number to check
 * Return:	number of leading zero bits
 */
int __clzti2(long long x)
{
	int ret = 64;

	if (!x)
		return 64;

	if (x & 0xFFFFFFFF00000000LL) {
		ret -= 32;
		x >>= 32;
	}
	if (x & 0xFFFF0000LL) {
		ret -= 16;
		x >>= 16;
	}
	if (x & 0xFF00LL) {
		ret -= 8;
		x >>= 8;
	}
	if (x & 0xF0LL) {
		ret -= 4;
		x >>= 4;
	}
	if (x & 0xCLL) {
		ret -= 2;
		x >>= 2;
	}
	if (x & 0x2LL) {
		ret -= 1;
		x >>= 1;
	}
	if (x)
		ret -= 1;

	return ret;
}

/**
 * __clzsi2() - count number of leading zero bits
 *
 * @x:		number to check
 * Return:	number of leading zero bits
 */
int __clzsi2(int x)
{
	int ret = 32;

	if (!x)
		return 32;

	if (x & 0xFFFF0000) {
		ret -= 16;
		x >>= 16;
	}
	if (x & 0xFF00) {
		ret -= 8;
		x >>= 8;
	}
	if (x & 0xF0) {
		ret -= 4;
		x >>= 4;
	}
	if (x & 0xC) {
		ret -= 2;
		x >>= 2;
	}
	if (x & 0x2) {
		ret -= 1;
		x >>= 1;
	}
	if (x)
		ret -= 1;

	return ret;
}

/**
 * __clzdi2() - count number of leading zero bits
 *
 * @x:		number to check
 * Return:	number of leading zero bits
 */
int __clzdi2(long x)
{
#if BITS_PER_LONG == 64
	return __clzti2(x);
#else
	return __clzsi2(x);
#endif
}
