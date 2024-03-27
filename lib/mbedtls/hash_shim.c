// SPDX-License-Identifier: GPL-2.0+
/*
 * Hash shim layer on MbedTLS Crypto library
 *
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include "common.h"
#include <malloc.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/md5.h>

void sha1_starts_mb(sha1_context *ctx)
{
	mbedtls_sha1_init(ctx);
	mbedtls_sha1_starts(ctx);
}

void sha1_update_mb(sha1_context *ctx, const unsigned char *input,
		    unsigned int length)
{
	mbedtls_sha1_update(ctx, input, length);
}

void sha1_finish_mb(sha1_context *ctx, unsigned char output[SHA1_SUM_LEN])
{
	mbedtls_sha1_finish(ctx, output);
	mbedtls_sha1_free(ctx);
}

void sha1_csum_wd_mb(const unsigned char *input, unsigned int length,
		     unsigned char *output, unsigned int chunk_sz)
{
	mbedtls_sha1(input, length, output);
}

void sha256_starts_mb(sha256_context *ctx)
{
	mbedtls_sha256_init(ctx);
	mbedtls_sha256_starts(ctx, 0);
}

void
sha256_update_mb(sha256_context *ctx, const uint8_t *input, uint32_t length)
{
	mbedtls_sha256_update(ctx, input, length);
}

void sha256_finish_mb(sha256_context *ctx, uint8_t digest[SHA256_SUM_LEN])
{
	mbedtls_sha256_finish(ctx, digest);
	mbedtls_sha256_free(ctx);
}

void sha256_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz)
{
	mbedtls_sha256(input, length, output, 0);
}

void sha384_starts_mb(sha512_context *ctx)
{
	mbedtls_sha512_init(ctx);
	mbedtls_sha512_starts(ctx, 1);
}

void
sha384_update_mb(sha512_context *ctx, const uint8_t *input, uint32_t length)
{
	mbedtls_sha512_update(ctx, input, length);
}

void sha384_finish_mb(sha512_context *ctx, uint8_t digest[SHA384_SUM_LEN])
{
	mbedtls_sha512_finish(ctx, digest);
	mbedtls_sha512_free(ctx);
}

void sha384_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz)
{
	mbedtls_sha512(input, length, output, 1);
}

void sha512_starts_mb(sha512_context *ctx)
{
	mbedtls_sha512_init(ctx);
	mbedtls_sha512_starts(ctx, 0);
}

void
sha512_update_mb(sha512_context *ctx, const uint8_t *input, uint32_t length)
{
	mbedtls_sha512_update(ctx, input, length);
}

void sha512_finish_mb(sha512_context *ctx, uint8_t digest[SHA512_SUM_LEN])
{
	mbedtls_sha512_finish(ctx, digest);
	mbedtls_sha512_free(ctx);
}

void sha512_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz)
{
	mbedtls_sha512(input, length, output, 0);
}

void
md5_wd_mb(const unsigned char *input, unsigned int len,
	  unsigned char output[16], unsigned int __always_unused chunk_sz)
{
	mbedtls_md5(input, len, output);
}

