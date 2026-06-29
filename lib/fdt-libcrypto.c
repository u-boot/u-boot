// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 * Copyright (c) 2013, Google Inc.
 */

#include <libfdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <u-boot/fdt-libcrypto.h>

int fdt_add_bignum(void *blob, int noffset, const char *prop_name,
		   const BIGNUM *num, int num_bits)
{
	int size = (num_bits + 7) / 8;
	unsigned char *buf;
	int ret;

	if (size <= 0)
		return -FDT_ERR_BADVALUE;

	buf = malloc(size);
	if (!buf) {
		fprintf(stderr, "Out of memory (%d bytes)\n", size);
		return -FDT_ERR_NOSPACE;
	}

	if (BN_bn2binpad(num, buf, size) != size) {
		free(buf);
		return -FDT_ERR_BADVALUE;
	}

	/* Callers may retry with a larger FDT if the property does not fit. */
	ret = fdt_setprop(blob, noffset, prop_name, buf, size);
	free(buf);

	return ret;
}
