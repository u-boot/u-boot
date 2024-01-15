/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <external/mbedtls/include/mbedtls/sha512.h>

#ifndef _MBEDTLS_SHA512_H
#define _MBEDTLS_SHA512_H

#define SHA384_SUM_LEN	48
#define SHA512_SUM_LEN	64

#define SHA384_DER_LEN	19
#define SHA512_DER_LEN	19

#define CHUNKSZ_SHA384	(16 * 1024)
#define CHUNKSZ_SHA512	(16 * 1024)

extern const u8 sha384_der_prefix[];
extern const u8 sha512_der_prefix[];

typedef mbedtls_sha512_context sha384_context;
typedef mbedtls_sha512_context sha512_context;

void sha384_starts_mb(sha512_context *ctx);
void
sha384_update_mb(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha384_finish_mb(sha512_context *ctx, uint8_t digest[SHA384_SUM_LEN]);
void sha384_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz);
void sha512_starts_mb(sha512_context *ctx);
void
sha512_update_mb(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha512_finish_mb(sha512_context *ctx, uint8_t digest[SHA512_SUM_LEN]);
void sha512_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz);

#endif /* _MBEDTLS_SHA512_H */
