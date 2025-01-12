// SPDX-License-Identifier: GPL-2.0+
/*
 * FIPS-180-2 compliant SHA-256 implementation
 *
 * Copyright (C) 2001-2003  Christophe Devine
 */

#ifndef USE_HOSTCC
#include <u-boot/schedule.h>
#endif /* USE_HOSTCC */
#include <string.h>
#include <u-boot/sha256.h>

#include <linux/compiler_attributes.h>

/*
 * Output = SHA-256( input buffer ). Trigger the watchdog every 'chunk_sz'
 * bytes of input processed.
 */
void sha256_csum_wd(const unsigned char *input, unsigned int ilen,
		    unsigned char *output, unsigned int chunk_sz)
{
	sha256_context ctx;
#if !defined(USE_HOSTCC) && \
	(defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG))
	const unsigned char *end;
	unsigned char *curr;
	int chunk;
#endif

	sha256_starts(&ctx);

#if !defined(USE_HOSTCC) && \
	(defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG))
	curr = (unsigned char *)input;
	end = input + ilen;
	while (curr < end) {
		chunk = end - curr;
		if (chunk > chunk_sz)
			chunk = chunk_sz;
		sha256_update(&ctx, curr, chunk);
		curr += chunk;
		schedule();
	}
#else
	sha256_update(&ctx, input, ilen);
#endif

	sha256_finish(&ctx, output);
}

void sha256_hmac(const unsigned char *key, int keylen,
		 const unsigned char *input, unsigned int ilen,
		 unsigned char *output)
{
	int i;
	sha256_context ctx;
	unsigned char keybuf[64];
	unsigned char k_ipad[64];
	unsigned char k_opad[64];
	unsigned char tmpbuf[32];
	int keybuf_len;

	if (keylen > 64) {
		sha256_starts(&ctx);
		sha256_update(&ctx, key, keylen);
		sha256_finish(&ctx, keybuf);

		keybuf_len = 32;
	} else {
		memcpy(keybuf, key, keylen);
		keybuf_len = keylen;
	}

	memset(k_ipad, 0x36, 64);
	memset(k_opad, 0x5C, 64);

	for (i = 0; i < keybuf_len; i++) {
		k_ipad[i] ^= keybuf[i];
		k_opad[i] ^= keybuf[i];
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
	memset(keybuf, 0, sizeof(keybuf));
	memset(&ctx, 0, sizeof(sha256_context));
}
