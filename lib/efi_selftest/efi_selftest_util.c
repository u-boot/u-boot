// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_util
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Utility functions
 */

#include <efi_selftest.h>

int efi_st_memcmp(const void *buf1, const void *buf2, size_t length)
{
	const u8 *pos1 = buf1;
	const u8 *pos2 = buf2;

	for (; length; --length) {
		if (*pos1 != *pos2)
			return *pos1 - *pos2;
		++pos1;
		++pos2;
	}
	return 0;
}

int efi_st_strcmp_16_8(const u16 *buf1, const char *buf2)
{
	for (; *buf1 || *buf2; ++buf1, ++buf2) {
		if (*buf1 != *buf2)
			return *buf1 - *buf2;
	}
	return 0;
}
