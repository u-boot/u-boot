/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <u-boot/zlib.h>

#define	ZALLOC_ALIGNMENT	16
#define HEAD_CRC		2
#define EXTRA_FIELD		4
#define ORIG_NAME		8
#define COMMENT			0x10
#define RESERVED		0xe0
#define DEFLATED		8

void *gzalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

void gzfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
	int i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp) {
		puts ("Error: gunzip out of data in header\n");
		return (-1);
	}

	return zunzip(dst, dstlen, src, lenp, 1, i);
}

/*
 * Uncompress blocks compressed with zlib without headers
 */
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset)
{
	z_stream s;
	int r;

	s.zalloc = gzalloc;
	s.zfree = gzfree;

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf ("Error: inflateInit2() returned %d\n", r);
		return -1;
	}
	s.next_in = src + offset;
	s.avail_in = *lenp - offset;
	s.next_out = dst;
	s.avail_out = dstlen;
	do {
		r = inflate(&s, Z_FINISH);
		if (stoponerr == 1 && r != Z_STREAM_END &&
		    (s.avail_out == 0 || r != Z_BUF_ERROR)) {
			printf("Error: inflate() returned %d\n", r);
			inflateEnd(&s);
			return -1;
		}
		s.avail_in = *lenp - offset - (int)(s.next_out - (unsigned char*)dst);
	} while (r == Z_BUF_ERROR);
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);

	return 0;
}
