/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <external/mbedtls/include/mbedtls/sha1.h>

#ifndef _MBEDTLS_SHA1_H
#define _MBEDTLS_SHA1_H

#define SHA1_SUM_LEN	20
#define SHA1_DER_LEN	15

#define CHUNKSZ_SHA1	(64 * 1024)

extern const u8 sha1_der_prefix[];

typedef mbedtls_sha1_context sha1_context;

void sha1_starts_mb(sha1_context *ctx);
void sha1_update_mb(sha1_context *ctx, const unsigned char *input,
		    unsigned int length);
void sha1_finish_mb(sha1_context *ctx, unsigned char output[SHA1_SUM_LEN]);
void sha1_csum_wd_mb(const unsigned char *input, unsigned int length,
		     unsigned char *output, unsigned int chunk_sz);

#endif /* _MBEDTLS_SHA1_H */
