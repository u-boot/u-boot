// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_crypto.h"
#include "avb_rsa.h"
#include "avb_sha.h"
#include "avb_util.h"

size_t avb_get_algorithm_hash_size(AvbAlgorithmType algorithm) {
  switch (algorithm) {
    case AVB_ALGORITHM_TYPE_NONE:
      return 0;
    case AVB_ALGORITHM_TYPE_SHA256_RSA2048:
    case AVB_ALGORITHM_TYPE_SHA256_RSA4096:
    case AVB_ALGORITHM_TYPE_SHA256_RSA8192:
      return AVB_SHA256_DIGEST_SIZE;
    case AVB_ALGORITHM_TYPE_SHA512_RSA2048:
    case AVB_ALGORITHM_TYPE_SHA512_RSA4096:
    case AVB_ALGORITHM_TYPE_SHA512_RSA8192:
      return AVB_SHA512_DIGEST_SIZE;
    case AVB_ALGORITHM_TYPE_MLDSA65:
    case AVB_ALGORITHM_TYPE_MLDSA87:
      return 0;
    default:
      return SIZE_MAX;
  }
}

bool avb_rsa_public_key_header_validate_and_byteswap(
    const AvbRSAPublicKeyHeader* src, AvbRSAPublicKeyHeader* dest) {
  avb_memcpy(dest, src, sizeof(AvbRSAPublicKeyHeader));

  dest->key_num_bits = avb_be32toh(dest->key_num_bits);
  dest->n0inv = avb_be32toh(dest->n0inv);

  return true;
}
