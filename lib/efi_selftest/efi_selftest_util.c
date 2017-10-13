/*
 * efi_selftest_util
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	return EFI_ST_SUCCESS;
}
