// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <common.h>
#include <abuf.h>
#include <log.h>
#include <malloc.h>
#include <linux/zstd.h>

int zstd_decompress(struct abuf *in, struct abuf *out)
{
	ZSTD_DStream *dstream;
	ZSTD_inBuffer in_buf;
	ZSTD_outBuffer out_buf;
	void *workspace;
	size_t wsize;
	int ret;

	wsize = ZSTD_DStreamWorkspaceBound(abuf_size(in));
	workspace = malloc(wsize);
	if (!workspace) {
		debug("%s: cannot allocate workspace of size %zu\n", __func__,
			wsize);
		return -ENOMEM;
	}

	dstream = ZSTD_initDStream(abuf_size(in), workspace, wsize);
	if (!dstream) {
		log_err("%s: ZSTD_initDStream failed\n", __func__);
		ret = -EPERM;
		goto do_free;
	}

	in_buf.src = abuf_data(in);
	in_buf.pos = 0;
	in_buf.size = abuf_size(in);

	out_buf.dst = abuf_data(out);
	out_buf.pos = 0;
	out_buf.size = abuf_size(out);

	while (1) {
		size_t res;

		res = ZSTD_decompressStream(dstream, &out_buf, &in_buf);
		if (ZSTD_isError(res)) {
			ret = ZSTD_getErrorCode(res);
			log_err("ZSTD_decompressStream error %d\n", ret);
			goto do_free;
		}

		if (in_buf.pos >= abuf_size(in) || !res)
			break;
	}

	ret = out_buf.pos;
do_free:
	free(workspace);
	return ret;
}
