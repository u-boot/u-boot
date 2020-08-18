// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if IS_ENABLED(CONFIG_LZO)
#include <linux/lzo.h>
#endif

#if IS_ENABLED(CONFIG_ZLIB)
#include <u-boot/zlib.h>
#endif

#if IS_ENABLED(CONFIG_ZSTD)
#include <linux/zstd.h>
#endif

#include "sqfs_decompressor.h"
#include "sqfs_utils.h"

int sqfs_decompressor_init(struct squashfs_ctxt *ctxt)
{
	u16 comp_type = get_unaligned_le16(&ctxt->sblk->compression);

	switch (comp_type) {
#if IS_ENABLED(CONFIG_LZO)
	case SQFS_COMP_LZO:
		break;
#endif
#if IS_ENABLED(CONFIG_ZLIB)
	case SQFS_COMP_ZLIB:
		break;
#endif
#if IS_ENABLED(CONFIG_ZSTD)
	case SQFS_COMP_ZSTD:
		ctxt->zstd_workspace = malloc(ZSTD_DCtxWorkspaceBound());
		if (!ctxt->zstd_workspace)
			return -ENOMEM;
		break;
#endif
	default:
		printf("Error: unknown compression type.\n");
		return -EINVAL;
	}

	return 0;
}

void sqfs_decompressor_cleanup(struct squashfs_ctxt *ctxt)
{
	u16 comp_type = get_unaligned_le16(&ctxt->sblk->compression);

	switch (comp_type) {
#if IS_ENABLED(CONFIG_LZO)
	case SQFS_COMP_LZO:
		break;
#endif
#if IS_ENABLED(CONFIG_ZLIB)
	case SQFS_COMP_ZLIB:
		break;
#endif
#if IS_ENABLED(CONFIG_ZSTD)
	case SQFS_COMP_ZSTD:
		free(ctxt->zstd_workspace);
		break;
#endif
	}
}

#if IS_ENABLED(CONFIG_ZLIB)
static void zlib_decompression_status(int ret)
{
	switch (ret) {
	case Z_BUF_ERROR:
		printf("Error: 'dest' buffer is not large enough.\n");
		break;
	case Z_DATA_ERROR:
		printf("Error: corrupted compressed data.\n");
		break;
	case Z_MEM_ERROR:
		printf("Error: insufficient memory.\n");
		break;
	}
}
#endif

#if IS_ENABLED(CONFIG_ZSTD)
static int sqfs_zstd_decompress(struct squashfs_ctxt *ctxt, void *dest,
				unsigned long dest_len, void *source, u32 src_len)
{
	ZSTD_DCtx *ctx;
	size_t wsize;
	int ret;

	wsize = ZSTD_DCtxWorkspaceBound();
	ctx = ZSTD_initDCtx(ctxt->zstd_workspace, wsize);
	ret = ZSTD_decompressDCtx(ctx, dest, dest_len, source, src_len);

	return ZSTD_isError(ret);
}
#endif /* CONFIG_ZSTD */

int sqfs_decompress(struct squashfs_ctxt *ctxt, void *dest,
		    unsigned long *dest_len, void *source, u32 src_len)
{
	u16 comp_type = get_unaligned_le16(&ctxt->sblk->compression);
	int ret = 0;

	switch (comp_type) {
#if IS_ENABLED(CONFIG_LZO)
	case SQFS_COMP_LZO: {
		size_t lzo_dest_len = *dest_len;
		ret = lzo1x_decompress_safe(source, src_len, dest, &lzo_dest_len);
		if (ret) {
			printf("LZO decompression failed. Error code: %d\n", ret);
			return -EINVAL;
		}

		break;
	}
#endif
#if IS_ENABLED(CONFIG_ZLIB)
	case SQFS_COMP_ZLIB:
		ret = uncompress(dest, dest_len, source, src_len);
		if (ret) {
			zlib_decompression_status(ret);
			return -EINVAL;
		}

		break;
#endif
#if IS_ENABLED(CONFIG_ZSTD)
	case SQFS_COMP_ZSTD:
		ret = sqfs_zstd_decompress(ctxt, dest, *dest_len, source, src_len);
		if (ret) {
			printf("ZSTD Error code: %d\n", ZSTD_getErrorCode(ret));
			return -EINVAL;
		}

		break;
#endif
	default:
		printf("Error: unknown compression type.\n");
		return -EINVAL;
	}

	return ret;
}
