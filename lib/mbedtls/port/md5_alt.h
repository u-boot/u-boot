/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef MD5_ALT_H
#define MD5_ALT_H

#include <image.h>
#include <u-boot/md5.h>

typedef MD5Context mbedtls_md5_context;

static inline void mbedtls_md5_init(mbedtls_md5_context *ctx)
{
}

static inline void mbedtls_md5_free(mbedtls_md5_context *ctx)
{
}

static inline void
mbedtls_md5_clone(mbedtls_md5_context *dst, const mbedtls_md5_context *src)
{
	*dst = *src;
}

static inline int mbedtls_md5_starts(mbedtls_md5_context *ctx)
{
	MD5Init(ctx);
	return 0;
}

static inline int mbedtls_md5_update(mbedtls_md5_context *ctx,
				     const unsigned char *input,
				     size_t ilen)
{
	MD5Update(ctx, input, ilen);
	return 0;
}

static inline int mbedtls_md5_finish(mbedtls_md5_context *ctx,
				     unsigned char output[16])
{
	MD5Final(output, ctx);
	return 0;
}

static inline int mbedtls_md5(const unsigned char *input,
			      size_t ilen,
			      unsigned char output[16])
{
	md5_wd(input, ilen, output, CHUNKSZ_MD5);
	return 0;
}

#endif /* md5_alt.h */
