// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifdef USE_HOSTCC
#include <arpa/inet.h>
#endif
#include <asm/sections.h>
#include <u-boot/crc.h>

#define POLY	(0x1070U << 3)

__rcode static unsigned char _crc8(unsigned short data)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}

	return (unsigned char)(data >> 8);
}

__rcode unsigned int crc8(unsigned int crc, const unsigned char *vptr, int len)
{
	int i;

	for (i = 0; i < len; i++)
		crc = _crc8((crc ^ vptr[i]) << 8);

	return crc;
}

void crc8_wd_buf(const unsigned char *input, unsigned int len,
		 unsigned char output[1], unsigned int chunk_sz)
{
	*output = crc8(0, input, len);
}
