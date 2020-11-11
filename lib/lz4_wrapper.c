// SPDX-License-Identifier: GPL 2.0+ OR BSD-3-Clause
/*
 * Copyright 2015 Google Inc.
 */

#include <common.h>
#include <compiler.h>
#include <image.h>
#include <lz4.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/unaligned.h>

static u16 LZ4_readLE16(const void *src) { return le16_to_cpu(*(u16 *)src); }
static void LZ4_copy4(void *dst, const void *src) { *(u32 *)dst = *(u32 *)src; }
static void LZ4_copy8(void *dst, const void *src) { *(u64 *)dst = *(u64 *)src; }

typedef  uint8_t BYTE;
typedef uint16_t U16;
typedef uint32_t U32;
typedef  int32_t S32;
typedef uint64_t U64;

#define FORCE_INLINE static inline __attribute__((always_inline))

/* lz4.c is unaltered (except removing unrelated code) from github.com/Cyan4973/lz4. */
#include "lz4.c"	/* #include for inlining, do not link! */

#define LZ4F_BLOCKUNCOMPRESSED_FLAG 0x80000000U

int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn)
{
	const void *end = dst + *dstn;
	const void *in = src;
	void *out = dst;
	int has_block_checksum;
	int ret;
	*dstn = 0;

	{ /* With in-place decompression the header may become invalid later. */
		u32 magic;
		u8 flags, version, independent_blocks, has_content_size;
		u8 block_desc;

		if (srcn < sizeof(u32) + 3*sizeof(u8))
			return -EINVAL;	/* input overrun */

		magic = get_unaligned_le32(in);
		in += sizeof(u32);
		flags = *(u8 *)in;
		in += sizeof(u8);
		block_desc = *(u8 *)in;
		in += sizeof(u8);

		version = (flags >> 6) & 0x3;
		independent_blocks = (flags >> 5) & 0x1;
		has_block_checksum = (flags >> 4) & 0x1;
		has_content_size = (flags >> 3) & 0x1;

		/* We assume there's always only a single, standard frame. */
		if (magic != LZ4F_MAGIC || version != 1)
			return -EPROTONOSUPPORT;	/* unknown format */
		if ((flags & 0x03) || (block_desc & 0x8f))
			return -EINVAL;	/* reserved bits must be zero */
		if (!independent_blocks)
			return -EPROTONOSUPPORT; /* we can't support this yet */

		if (has_content_size) {
			if (srcn < sizeof(u32) + 3*sizeof(u8) + sizeof(u64))
				return -EINVAL;	/* input overrun */
			in += sizeof(u64);
		}
		/* Header checksum byte */
		in += sizeof(u8);
	}

	while (1) {
		u32 block_header, block_size;

		block_header = get_unaligned_le32(in);
		in += sizeof(u32);
		block_size = block_header & ~LZ4F_BLOCKUNCOMPRESSED_FLAG;

		if (in - src + block_size > srcn) {
			ret = -EINVAL;		/* input overrun */
			break;
		}

		if (!block_size) {
			ret = 0;	/* decompression successful */
			break;
		}

		if (block_header & LZ4F_BLOCKUNCOMPRESSED_FLAG) {
			size_t size = min((ptrdiff_t)block_size, end - out);
			memcpy(out, in, size);
			out += size;
			if (size < block_size) {
				ret = -ENOBUFS;	/* output overrun */
				break;
			}
		} else {
			/* constant folding essential, do not touch params! */
			ret = LZ4_decompress_generic(in, out, block_size,
					end - out, endOnInputSize,
					full, 0, noDict, out, NULL, 0);
			if (ret < 0) {
				ret = -EPROTO;	/* decompression error */
				break;
			}
			out += ret;
		}

		in += block_size;
		if (has_block_checksum)
			in += sizeof(u32);
	}

	*dstn = out - dst;
	return ret;
}
