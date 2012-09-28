/*
 * (C) Copyright 2012
 * Lei Wen <leiwen@marvell.com>, Marvell Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <u-boot/zlib.h>
#include "zlib/zutil.h"

#ifndef CONFIG_GZIP_COMPRESS_DEF_SZ
#define CONFIG_GZIP_COMPRESS_DEF_SZ	0x200
#endif
#define ZALLOC_ALIGNMENT		16

static void *zalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

static void zfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

int gzip(void *dst, unsigned long *lenp,
		unsigned char *src, unsigned long srclen)
{
	return zzip(dst, lenp, src, srclen, 1, NULL);
}

/*
 * Compress blocks with zlib
 */
int zzip(void *dst, unsigned long *lenp, unsigned char *src,
		unsigned long srclen, int stoponerr,
		int (*func)(unsigned long, unsigned long))
{
	z_stream s;
	int r, flush, orig, window;
	unsigned long comp_len, left_len;

	if (!srclen)
		return 0;

#ifndef CONFIG_GZIP
	window = MAX_WBITS;
#else
	window = 2 * MAX_WBITS;
#endif
	orig = *lenp;
	s.zalloc = zalloc;
	s.zfree = zfree;
	s.opaque = Z_NULL;

	r = deflateInit2_(&s, Z_BEST_SPEED, Z_DEFLATED,	window,
			DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			ZLIB_VERSION, sizeof(z_stream));
	if (r != Z_OK) {
		printf ("Error: deflateInit2_() returned %d\n", r);
		return -1;
	}

	while (srclen > 0) {
		comp_len = (srclen > CONFIG_GZIP_COMPRESS_DEF_SZ) ?
				CONFIG_GZIP_COMPRESS_DEF_SZ : srclen;

		s.next_in = src;
		s.avail_in = comp_len;
		flush = (srclen > CONFIG_GZIP_COMPRESS_DEF_SZ)?
			Z_NO_FLUSH : Z_FINISH;

		do {
			left_len = (*lenp > CONFIG_GZIP_COMPRESS_DEF_SZ) ?
					CONFIG_GZIP_COMPRESS_DEF_SZ : *lenp;
			s.next_out = dst;
			s.avail_out = left_len;
			r = deflate(&s, flush);
			if (r == Z_STREAM_ERROR && stoponerr == 1) {
				printf("Error: deflate() returned %d\n", r);
				r = -1;
				goto bail;
			}
			if (!func) {
				dst += (left_len - s.avail_out);
				*lenp -= (left_len - s.avail_out);
			} else if (left_len - s.avail_out > 0) {
				r = func((unsigned long)dst,
					left_len - s.avail_out);
				if (r < 0)
					goto bail;
			}
		} while (s.avail_out == 0 && (*lenp > 0));
		if (s.avail_in) {
			printf("Deflate failed to consume %u bytes", s.avail_in);
			r = -1;
			goto bail;
		}
		if (*lenp == 0) {
			printf("Deflate need more space to compress "
				"left %lu bytes\n", srclen);
			r = -1;
			goto bail;
		}
		srclen -= comp_len;
		src += comp_len;
	}

	r = 0;
bail:
	deflateEnd(&s);
	*lenp = orig - *lenp;
	return r;
}
