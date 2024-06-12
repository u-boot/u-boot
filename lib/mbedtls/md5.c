// SPDX-License-Identifier: GPL-2.0+
/*
 * Hash shim layer on MbedTLS Crypto library
 *
 * Copyright (c) 2023 Linaro Limited
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

void md5(unsigned char *input, int len, unsigned char output[16])
{
	MD5Context context;

	MD5Init(&context);
	MD5Update(&context, input, len);
	MD5Final(output, &context);
}

void md5_wd(const unsigned char *input, unsigned int len,
	    unsigned char output[16], unsigned int chunk_sz)
{
	MD5Context context;
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	const unsigned char *end, *curr;
	int chunk;
#endif

	MD5Init(&context);

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	curr = input;
	end = input + len;
	while (curr < end) {
		chunk = end - curr;
		if (chunk > chunk_sz)
			chunk = chunk_sz;
		MD5Update(&context, curr, chunk);
		curr += chunk;
		schedule();
	}
#else
	MD5Update(&context, input, len);
#endif

	MD5Final(output, &context);
}
