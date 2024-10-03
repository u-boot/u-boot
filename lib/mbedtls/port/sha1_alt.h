/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef SHA1_ALT_H
#define SHA1_ALT_H

#include <image.h>
#include <u-boot/sha1.h>

typedef sha1_context mbedtls_sha1_context;

static inline void mbedtls_sha1_init(mbedtls_sha1_context *ctx)
{
}

static inline void mbedtls_sha1_free(mbedtls_sha1_context *ctx)
{
}

static inline void mbedtls_sha1_clone(mbedtls_sha1_context *dst,
				      const mbedtls_sha1_context *src)
{
	*dst = *src;
}

static inline int mbedtls_sha1_starts(mbedtls_sha1_context *ctx)
{
	sha1_starts(ctx);
	return 0;
}

static inline int mbedtls_sha1_update(mbedtls_sha1_context *ctx,
				      const unsigned char *input,
				      size_t ilen)
{
	sha1_update(ctx, input, ilen);
	return 0;
}

static inline int mbedtls_sha1_finish(mbedtls_sha1_context *ctx,
				      unsigned char output[20])
{
	sha1_finish(ctx, output);
	return 0;
}

static inline int mbedtls_sha1(const unsigned char *input,
			       size_t ilen,
			       unsigned char output[20])
{
	sha1_csum_wd(input, ilen, output, CHUNKSZ_SHA1);
	return 0;
}

#endif /* sha1_alt.h */
