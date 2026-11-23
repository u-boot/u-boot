// SPDX-License-Identifier: GPL-2.0+
/*
 * libavb SHA-256/SHA-512 on top of U-Boot's SHA library, so that libavb
 * shares one implementation with the rest of U-Boot (including hardware
 * acceleration and the mbedTLS back end) instead of carrying its own.
 */

#include "../avb_sha.h"
#include "avb_crypto_ops_impl.h"

/* U-Boot's update functions take a 32-bit length */
#define SHA_CHUNK 0x40000000

void avb_sha256_init(AvbSHA256Ctx* avb_ctx) {
  sha256_starts(&avb_ctx->impl);
}

void avb_sha256_update(AvbSHA256Ctx* avb_ctx, const uint8_t* data, size_t len) {
  while (len) {
    size_t n = len > SHA_CHUNK ? SHA_CHUNK : len;

    sha256_update(&avb_ctx->impl, data, n);
    data += n;
    len -= n;
  }
}

uint8_t* avb_sha256_final(AvbSHA256Ctx* avb_ctx) {
  sha256_finish(&avb_ctx->impl, avb_ctx->buf);
  return avb_ctx->buf;
}

void avb_sha512_init(AvbSHA512Ctx* avb_ctx) {
  sha512_starts(&avb_ctx->impl);
}

void avb_sha512_update(AvbSHA512Ctx* avb_ctx, const uint8_t* data, size_t len) {
  while (len) {
    size_t n = len > SHA_CHUNK ? SHA_CHUNK : len;

    sha512_update(&avb_ctx->impl, data, n);
    data += n;
    len -= n;
  }
}

uint8_t* avb_sha512_final(AvbSHA512Ctx* avb_ctx) {
  sha512_finish(&avb_ctx->impl, avb_ctx->buf);
  return avb_ctx->buf;
}
