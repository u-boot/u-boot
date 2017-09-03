/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "btrfs.h"
#include <linux/lzo.h>
#include <u-boot/zlib.h>

static u32 decompress_lzo(const u8 *cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	u32 tot_len, in_len, res;
	size_t out_len;
	int ret;

	if (clen < 4)
		return -1;

	tot_len = le32_to_cpu(*(u32 *) cbuf);
	cbuf += 4;
	clen -= 4;
	tot_len -= 4;

	if (tot_len == 0 && dlen)
		return -1;
	if (tot_len < 4)
		return -1;

	res = 0;

	while (tot_len > 4) {
		in_len = le32_to_cpu(*(u32 *) cbuf);
		cbuf += 4;
		clen -= 4;

		if (in_len > clen || tot_len < 4 + in_len)
			return -1;

		tot_len -= 4 + in_len;

		out_len = dlen;
		ret = lzo1x_decompress_safe(cbuf, in_len, dbuf, &out_len);
		if (ret != LZO_E_OK)
			return -1;

		cbuf += in_len;
		clen -= in_len;
		dbuf += out_len;
		dlen -= out_len;

		res += out_len;
	}

	return res;
}

/* from zutil.h */
#define PRESET_DICT 0x20

static u32 decompress_zlib(const u8 *_cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	int wbits = MAX_WBITS, ret = -1;
	z_stream stream;
	u8 *cbuf;
	u32 res;

	memset(&stream, 0, sizeof(stream));

	cbuf = (u8 *) _cbuf;

	stream.total_in = 0;

	stream.next_out = dbuf;
	stream.avail_out = dlen;
	stream.total_out = 0;

	/* skip adler32 check if deflate and no dictionary */
	if (clen > 2 && !(cbuf[1] & PRESET_DICT) &&
	    ((cbuf[0] & 0x0f) == Z_DEFLATED) &&
	    !(((cbuf[0] << 8) + cbuf[1]) % 31)) {
		wbits = -((cbuf[0] >> 4) + 8);
		cbuf += 2;
		clen -= 2;
	}

	if (Z_OK != inflateInit2(&stream, wbits))
		return -1;

	while (stream.total_in < clen) {
		stream.next_in = cbuf + stream.total_in;
		stream.avail_in = min((u32) (clen - stream.total_in),
				      (u32) btrfs_info.sb.sectorsize);

		ret = inflate(&stream, Z_NO_FLUSH);
		if (ret != Z_OK)
			break;
	}

	res = stream.total_out;
	inflateEnd(&stream);

	if (ret != Z_STREAM_END)
		return -1;

	return res;
}

u32 btrfs_decompress(u8 type, const char *c, u32 clen, char *d, u32 dlen)
{
	u32 res;
	const u8 *cbuf;
	u8 *dbuf;

	cbuf = (const u8 *) c;
	dbuf = (u8 *) d;

	switch (type) {
	case BTRFS_COMPRESS_NONE:
		res = dlen < clen ? dlen : clen;
		memcpy(dbuf, cbuf, res);
		return res;
	case BTRFS_COMPRESS_ZLIB:
		return decompress_zlib(cbuf, clen, dbuf, dlen);
	case BTRFS_COMPRESS_LZO:
		return decompress_lzo(cbuf, clen, dbuf, dlen);
	default:
		printf("%s: Unsupported compression in extent: %i\n", __func__,
		       type);
		return -1;
	}
}
