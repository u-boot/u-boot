/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef SHA256_ALT_H
#define SHA256_ALT_H

#include <image.h>
#include <u-boot/sha256.h>

typedef sha256_context mbedtls_sha256_context;

static inline void mbedtls_sha256_init(mbedtls_sha256_context *ctx)
{
}

static inline void mbedtls_sha256_free(mbedtls_sha256_context *ctx)
{
}

static inline void mbedtls_sha256_clone(mbedtls_sha256_context *dst,
					const mbedtls_sha256_context *src)
{
	*dst = *src;
}

static inline int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224)
{
	if (is224)
		return -EOPNOTSUPP;

	sha256_starts(ctx);
	return 0;
}

static inline int mbedtls_sha256_update(mbedtls_sha256_context *ctx,
					const unsigned char *input,
					size_t ilen)
{
	sha256_update(ctx, input, ilen);
	return 0;
}

static inline int mbedtls_sha256_finish(mbedtls_sha256_context *ctx,
					unsigned char *output)
{
	sha256_finish(ctx, output);
	return 0;
}

static inline int mbedtls_sha256(const unsigned char *input,
				 size_t ilen,
				 unsigned char *output,
				 int is224)
{
	if (is224)
		return -EOPNOTSUPP;

	sha256_csum_wd(input, ilen, output, CHUNKSZ_SHA256);
	return 0;
}

#endif /* sha256_alt.h */
