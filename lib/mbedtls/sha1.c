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
#include <u-boot/sha1.h>

const u8 sha1_der_prefix[SHA1_DER_LEN] = {
	0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e,
	0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14
};

void sha1_starts(sha1_context *ctx)
{
	mbedtls_sha1_init(ctx);
	mbedtls_sha1_starts(ctx);
}

void sha1_update(sha1_context *ctx, const unsigned char *input,
		 unsigned int length)
{
	mbedtls_sha1_update(ctx, input, length);
}

void sha1_finish(sha1_context *ctx, unsigned char output[SHA1_SUM_LEN])
{
	mbedtls_sha1_finish(ctx, output);
	mbedtls_sha1_free(ctx);
}

void sha1_csum_wd(const unsigned char *input, unsigned int ilen,
		  unsigned char *output, unsigned int chunk_sz)
{
	sha1_context ctx;

	sha1_starts(&ctx);

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || IS_ENABLED(CONFIG_WATCHDOG)) {
		const unsigned char *curr = input;
		const unsigned char *end = input + ilen;
		int chunk;

		while (curr < end) {
			chunk = end - curr;
			if (chunk > chunk_sz)
				chunk = chunk_sz;
			sha1_update(&ctx, curr, chunk);
			curr += chunk;
			schedule();
		}
	} else {
		sha1_update(&ctx, input, ilen);
	}

	sha1_finish(&ctx, output);
}

void sha1_hmac(const unsigned char *key, int keylen,
	       const unsigned char *input, unsigned int ilen,
	       unsigned char *output)
{
	int i;
	sha1_context ctx;
	unsigned char k_ipad[K_PAD_LEN];
	unsigned char k_opad[K_PAD_LEN];
	unsigned char tmpbuf[20];

	if (keylen > K_PAD_LEN)
		return;

	memset(k_ipad, K_IPAD_VAL, sizeof(k_ipad));
	memset(k_opad, K_OPAD_VAL, sizeof(k_opad));

	for (i = 0; i < keylen; i++) {
		k_ipad[i] ^= key[i];
		k_opad[i] ^= key[i];
	}

	sha1_starts(&ctx);
	sha1_update(&ctx, k_ipad, sizeof(k_ipad));
	sha1_update(&ctx, input, ilen);
	sha1_finish(&ctx, tmpbuf);

	sha1_starts(&ctx);
	sha1_update(&ctx, k_opad, sizeof(k_opad));
	sha1_update(&ctx, tmpbuf, sizeof(tmpbuf));
	sha1_finish(&ctx, output);

	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memset(tmpbuf, 0, sizeof(tmpbuf));
	memset(&ctx, 0, sizeof(sha1_context));
}
