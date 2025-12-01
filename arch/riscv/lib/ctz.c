// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * libgcc replacement - count trailing bits
 */

#include <linux/types.h>

/**
 * __ctzti2() - count number of trailing zero bits
 *
 * @x:		number to check
 * Return:	number of trailing zero bits
 */
int __ctzti2(long long x)
{
	int ret = 0;

	if (!x)
		return 64;

	if (!(x & 0xFFFFFFFFLL)) {
		ret += 32;
		x >>= 32;
	}
	if (!(x & 0xFFFFLL)) {
		ret += 16;
		x >>= 16;
	}
	if (!(x & 0xFFLL)) {
		ret += 8;
		x >>= 8;
	}
	if (!(x & 0xFLL)) {
		ret += 4;
		x >>= 4;
	}
	if (!(x & 0x3LL)) {
		ret += 2;
		x >>= 2;
	}
	if (!(x & 0x1ll))
		ret += 1;

	return ret;
}

/**
 * __ctzsi2() - count number of trailing zero bits
 *
 * @x:		number to check
 * Return:	number of trailing zero bits
 */
int __ctzsi2(int x)
{
	int ret = 0;

	if (!x)
		return 32;

	if (!(x & 0xFFFF)) {
		ret += 16;
		x >>= 16;
	}
	if (!(x & 0xFF)) {
		ret += 8;
		x >>= 8;
	}
	if (!(x & 0xF)) {
		ret += 4;
		x >>= 4;
	}
	if (!(x & 0x3)) {
		ret += 2;
		x >>= 2;
	}
	if (!(x & 0x1))
		ret += 1;

	return ret;
}

/**
 * __ctzdi2() - count number of trailing zero bits
 *
 * @x:		number to check
 * Return:	number of trailing zero bits
 */
int __ctzdi2(long x)
{
#if BITS_PER_LONG == 64
	return __ctzti2(x);
#else
	return __ctzsi2(x);
#endif
}
