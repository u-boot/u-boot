/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <external/mbedtls/include/mbedtls/sha256.h>

#ifndef _MBEDTLS_SHA256_H
#define _MBEDTLS_SHA256_H

#define SHA224_SUM_LEN	28
#define SHA256_SUM_LEN	32

#define SHA224_DER_LEN	19
#define SHA256_DER_LEN	19

#define CHUNKSZ_SHA224	(64 * 1024)
#define CHUNKSZ_SHA256	(64 * 1024)

extern const u8 sha256_der_prefix[];

typedef mbedtls_sha256_context sha256_context;

void sha256_starts_mb(sha256_context *ctx);
void
sha256_update_mb(sha256_context *ctx, const uint8_t *input, uint32_t length);
void sha256_finish_mb(sha256_context *ctx, uint8_t digest[SHA256_SUM_LEN]);
void sha256_csum_wd_mb(const unsigned char *input, unsigned int length,
		       unsigned char *output, unsigned int chunk_sz);

#endif /* _MBEDTLS_SHA256_H */
