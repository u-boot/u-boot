// SPDX-License-Identifier: GPL-2.0+
/*
 * Hash shim layer on MbedTLS Crypto library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include "compiler.h"

#ifndef USE_HOSTCC
#include <watchdog.h>
#endif /* USE_HOSTCC */
#include <u-boot/md5.h>

void MD5Init(MD5Context *ctx)
{
	mbedtls_md5_init(ctx);
	mbedtls_md5_starts(ctx);
}

void MD5Update(MD5Context *ctx, unsigned char const *buf, unsigned int len)
{
	mbedtls_md5_update(ctx, buf, len);
}

void MD5Final(unsigned char digest[16], MD5Context *ctx)
{
	mbedtls_md5_finish(ctx, digest);
	mbedtls_md5_free(ctx);
}

void md5_wd(const unsigned char *input, unsigned int len,
	    unsigned char output[16], unsigned int chunk_sz)
{
	MD5Context context;

	MD5Init(&context);

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || IS_ENABLED(CONFIG_WATCHDOG)) {
		const unsigned char *curr = input;
		const unsigned char *end = input + len;
		int chunk;

		while (curr < end) {
			chunk = end - curr;
			if (chunk > chunk_sz)
				chunk = chunk_sz;
			MD5Update(&context, curr, chunk);
			curr += chunk;
			schedule();
		}
	} else {
		MD5Update(&context, input, len);
	}

	MD5Final(output, &context);
}
