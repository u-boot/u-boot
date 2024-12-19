// SPDX-License-Identifier: GPL-2.0+
/*
 * Hash shim layer on MbedTLS Crypto library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#ifndef USE_HOSTCC
#include <cyclic.h>
#endif /* USE_HOSTCC */
#include <u-boot/sha256.h>

#include <mbedtls/md.h>

const u8 sha256_der_prefix[SHA256_DER_LEN] = {
	0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
	0x00, 0x04, 0x20
};

void sha256_starts(sha256_context *ctx)
{
	mbedtls_sha256_init(ctx);
	mbedtls_sha256_starts(ctx, 0);
}

void
sha256_update(sha256_context *ctx, const uint8_t *input, uint32_t length)
{
	mbedtls_sha256_update(ctx, input, length);
}

void sha256_finish(sha256_context *ctx, uint8_t digest[SHA256_SUM_LEN])
{
	mbedtls_sha256_finish(ctx, digest);
	mbedtls_sha256_free(ctx);
}

int sha256_hmac(const unsigned char *key, int keylen,
		const unsigned char *input, unsigned int ilen,
		unsigned char *output)
{
	const mbedtls_md_info_t *md;

	md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	if (!md)
		return MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE;

	return mbedtls_md_hmac(md, key, keylen, input, ilen, output);
}
