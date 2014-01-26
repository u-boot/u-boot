/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "linux/crc8.h"

unsigned int crc8(const unsigned char *vptr, int len)
{
	const unsigned char *data = vptr;
	unsigned int crc = 0;
	int i, j;

	for (j = len; j; j--, data++) {
		crc ^= (*data << 8);
		for (i = 8; i; i--) {
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}

	return (crc >> 8) & 0xff;
}
