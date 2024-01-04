/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef SHA512_ALT_H
#define SHA512_ALT_H

#include <image.h>
#include <u-boot/sha512.h>

typedef struct mbedtls_sha512_context {
	sha512_context *ubctx;
	bool is384;
} mbedtls_sha512_context;

static inline void mbedtls_sha512_init(mbedtls_sha512_context *ctx)
{
}

static inline void mbedtls_sha512_free(mbedtls_sha512_context *ctx)
{
}

static inline void mbedtls_sha512_clone(mbedtls_sha512_context *dst,
					const mbedtls_sha512_context *src)
{
	*dst = *src;
}

static inline int mbedtls_sha512_starts(mbedtls_sha512_context *ctx, int is384)
{
	if (is384)
		sha384_starts(ctx->ubctx);
	else
		sha512_starts(ctx->ubctx);

	ctx->is384 = is384;
	return 0;
}

static inline int mbedtls_sha512_update(mbedtls_sha512_context *ctx,
					const unsigned char *input,
					size_t ilen)
{
	if (ctx->is384)
		sha384_update(ctx->ubctx, input, ilen);
	else
		sha512_update(ctx->ubctx, input, ilen);

	return 0;
}

static inline int mbedtls_sha512_finish(mbedtls_sha512_context *ctx,
					unsigned char *output)
{
	if (ctx->is384)
		sha384_finish(ctx->ubctx, output);
	else
		sha512_finish(ctx->ubctx, output);

	return 0;
}

static inline int mbedtls_sha512(const unsigned char *input,
				 size_t ilen,
				 unsigned char *output,
				 int is384)
{
	if (is384)
		sha384_csum_wd(input, ilen, output, CHUNKSZ_SHA512);
	else
		sha512_csum_wd(input, ilen, output, CHUNKSZ_SHA512);

	return 0;
}

#endif /* sha512_alt.h */
