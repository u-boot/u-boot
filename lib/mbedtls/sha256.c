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
#include <string.h>
#include <u-boot/sha256.h>

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

void sha256_csum_wd(const unsigned char *input, unsigned int ilen,
		    unsigned char *output, unsigned int chunk_sz)
{
	sha256_context ctx;

	sha256_starts(&ctx);

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || IS_ENABLED(CONFIG_WATCHDOG)) {
		const unsigned char *curr = input;
		const unsigned char *end = input + ilen;
		int chunk;

		while (curr < end) {
			chunk = end - curr;
			if (chunk > chunk_sz)
				chunk = chunk_sz;
			sha256_update(&ctx, curr, chunk);
			curr += chunk;
			schedule();
		}
	} else {
		sha256_update(&ctx, input, ilen);
	}

	sha256_finish(&ctx, output);
}

void sha256_hmac(const unsigned char *key, int keylen,
		 const unsigned char *input, unsigned int ilen,
		 unsigned char *output)
{
	int i;
	sha256_context ctx;
	unsigned char k_ipad[64];
	unsigned char k_opad[64];
	unsigned char tmpbuf[32];

	memset(k_ipad, 0x36, 64);
	memset(k_opad, 0x5C, 64);

	for (i = 0; i < keylen; i++) {
		if (i >= 64)
			break;

		k_ipad[i] ^= key[i];
		k_opad[i] ^= key[i];
	}

	sha256_starts(&ctx);
	sha256_update(&ctx, k_ipad, sizeof(k_ipad));
	sha256_update(&ctx, input, ilen);
	sha256_finish(&ctx, tmpbuf);

	sha256_starts(&ctx);
	sha256_update(&ctx, k_opad, sizeof(k_opad));
	sha256_update(&ctx, tmpbuf, sizeof(tmpbuf));
	sha256_finish(&ctx, output);

	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memset(tmpbuf, 0, sizeof(tmpbuf));
	memset(&ctx, 0, sizeof(sha256_context));
}
