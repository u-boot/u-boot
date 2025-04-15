#ifndef _SHA256_H
#define _SHA256_H

#include <linux/compiler_attributes.h>
#include <linux/errno.h>
#include <linux/kconfig.h>
#include <linux/types.h>

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_CRYPTO)
#include "mbedtls_options.h"
#include <mbedtls/sha256.h>
#endif

#define SHA224_SUM_LEN	28
#define SHA256_SUM_LEN	32
#define SHA256_DER_LEN	19

extern const uint8_t sha256_der_prefix[];

/* Reset watchdog each time we process this many bytes */
#define CHUNKSZ_SHA256	(64 * 1024)

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_CRYPTO)
typedef mbedtls_sha256_context sha256_context;
#else
typedef struct {
	uint32_t total[2];
	uint32_t state[8];
	uint8_t buffer[64];
} sha256_context;
#endif

void sha256_starts(sha256_context * ctx);
void sha256_update(sha256_context *ctx, const uint8_t *input, uint32_t length);
void sha256_finish(sha256_context * ctx, uint8_t digest[SHA256_SUM_LEN]);

void sha256_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);

int sha256_hmac(const unsigned char *key, int keylen,
		const unsigned char *input, unsigned int ilen,
		unsigned char *output);

#if CONFIG_IS_ENABLED(HKDF_MBEDTLS)
int sha256_hkdf(const unsigned char *salt, int saltlen,
		const unsigned char *ikm, int ikmlen,
		const unsigned char *info, int infolen,
		unsigned char *output, int outputlen);
#else
static inline int sha256_hkdf(const unsigned char __always_unused *salt,
			      int __always_unused saltlen,
			      const unsigned char __always_unused *ikm,
			      int __always_unused ikmlen,
			      const unsigned char __always_unused *info,
			      int __always_unused infolen,
			      unsigned char __always_unused *output,
			      int __always_unused outputlen) {
	return -EOPNOTSUPP;
}
#endif

#endif /* _SHA256_H */
