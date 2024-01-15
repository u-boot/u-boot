// SPDX-License-Identifier: GPL-2.0+
/*
 * Hash shim layer on MbedTLS Crypto library
 *
 * Copyright (c) 2023 Linaro Limited
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

void sha1_csum(const unsigned char *input, unsigned int ilen,
	       unsigned char *output)
{
	sha1_context ctx;

	sha1_starts(&ctx);
	sha1_update(&ctx, input, ilen);
	sha1_finish(&ctx, output);
}

void sha1_csum_wd(const unsigned char *input, unsigned int ilen,
		  unsigned char *output, unsigned int chunk_sz)
{
	sha1_context ctx;
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	const unsigned char *end, *curr;
	int chunk;
#endif

	sha1_starts(&ctx);

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	curr = input;
	end = input + ilen;
	while (curr < end) {
		chunk = end - curr;
		if (chunk > chunk_sz)
			chunk = chunk_sz;
		sha1_update(&ctx, curr, chunk);
		curr += chunk;
		schedule();
	}
#else
	sha1_update(&ctx, input, ilen);
#endif

	sha1_finish(&ctx, output);
}

void sha1_hmac(const unsigned char *key, int keylen,
	       const unsigned char *input, unsigned int ilen,
	       unsigned char *output)
{
	int i;
	sha1_context ctx;
	unsigned char k_ipad[64];
	unsigned char k_opad[64];
	unsigned char tmpbuf[20];

	memset(k_ipad, 0x36, 64);
	memset(k_opad, 0x5C, 64);

	for (i = 0; i < keylen; i++) {
		if (i >= 64)
			break;

		k_ipad[i] ^= key[i];
		k_opad[i] ^= key[i];
	}

	sha1_starts(&ctx);
	sha1_update(&ctx, k_ipad, 64);
	sha1_update(&ctx, input, ilen);
	sha1_finish(&ctx, tmpbuf);

	sha1_starts(&ctx);
	sha1_update(&ctx, k_opad, 64);
	sha1_update(&ctx, tmpbuf, 20);
	sha1_finish(&ctx, output);

	memset(k_ipad, 0, 64);
	memset(k_opad, 0, 64);
	memset(tmpbuf, 0, 20);
	memset(&ctx, 0, sizeof(sha1_context));
}
