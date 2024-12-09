/*
 * This file was transplanted with slight modifications from Linux sources
 * (fs/cifs/md5.h) into U-Boot by Bartlomiej Sieka <tur@semihalf.com>.
 */

#ifndef _MD5_H
#define _MD5_H

#include <linux/kconfig.h>

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_CRYPTO)
#include <mbedtls/md5.h>
#endif
#include "compiler.h"

#define MD5_SUM_LEN	16
#define MD5_DEF_CHUNK_SZ 0x10000

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_CRYPTO)
typedef mbedtls_md5_context MD5Context;
#else
typedef struct MD5Context {
	__u32 buf[4];
	__u32 bits[2];
	union {
		unsigned char in[64];
		__u32 in32[16];
	};
} MD5Context;
#endif

void MD5Init(MD5Context *ctx);
void MD5Update(MD5Context *ctx, unsigned char const *buf, unsigned int len);
void MD5Final(unsigned char digest[16], MD5Context *ctx);

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at 'input'.
 * 'output' must have enough space to hold 16 bytes. If 'chunk' Trigger the
 * watchdog every 'chunk_sz' bytes of input processed.
 */
void md5_wd(const unsigned char *input, unsigned int len,
	     unsigned char output[16], unsigned int chunk_sz);

#endif /* _MD5_H */
