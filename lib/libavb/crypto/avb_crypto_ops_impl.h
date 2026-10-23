/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * U-Boot crypto back end for libavb: the SHA contexts come from U-Boot's own
 * SHA-256/SHA-512 library (lib/sha256.c, lib/sha512.c, or mbedTLS when
 * CONFIG_MBEDTLS_LIB_CRYPTO is set), which may also use hardware
 * acceleration. ML-DSA is not supported, see crypto/mldsa_impl.c.
 */

#ifdef AVB_INSIDE_LIBAVB_H
#error "You can't include avb_crypto_ops_impl.h in the public header libavb.h."
#endif

#ifndef AVB_COMPILATION
#error "Never include this file, it may only be used from internal avb code."
#endif

#ifndef AVB_CRYPTO_OPS_IMPL_H_
#define AVB_CRYPTO_OPS_IMPL_H_

#include <u-boot/sha256.h>
#include <u-boot/sha512.h>

/* Block size in bytes of a SHA-256 digest. */
#define AVB_SHA256_BLOCK_SIZE 64

/* Block size in bytes of a SHA-512 digest. */
#define AVB_SHA512_BLOCK_SIZE 128

/* SHA-256 context: U-Boot's. */
typedef sha256_context AvbSHA256ImplCtx;

/* SHA-512 context: U-Boot's. */
typedef sha512_context AvbSHA512ImplCtx;

/* Dummy structs for the stub ML-DSA implementation */
typedef struct {
  int dummy;
} AvbMLDSA65PrehashImplCtx;
typedef struct {
  int dummy;
} AvbMLDSA87PrehashImplCtx;

#endif /* AVB_CRYPTO_OPS_IMPL_H_ */
