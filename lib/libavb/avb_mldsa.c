// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2025 The Android Open Source Project
 */

#include "avb_mldsa.h"

#include "avb_util.h"
#include "avb_vbmeta_image.h"

bool avb_mldsa_public_key_header_validate_and_byteswap(
    const AvbMLDSAPublicKeyHeader* src, AvbMLDSAPublicKeyHeader* dest) {
  avb_memcpy(dest, src, sizeof(AvbMLDSAPublicKeyHeader));

  dest->key_num_bytes = avb_be32toh(dest->key_num_bytes);

  return true;
}

static bool avb_mldsa_validate_public_key(const uint8_t* key,
                                          size_t key_num_bytes,
                                          AvbAlgorithmType algorithm,
                                          AvbMLDSAPublicKeyHeader* out_header) {
  uint32_t expected_key_num_bytes;

  if (key == NULL || out_header == NULL) {
    avb_error("Invalid input.\n");
    return false;
  }

  if (key_num_bytes < sizeof(AvbMLDSAPublicKeyHeader)) {
    avb_error("Invalid key length.\n");
    return false;
  }

  if (!avb_mldsa_public_key_header_validate_and_byteswap(
          (const AvbMLDSAPublicKeyHeader*)key, out_header)) {
    avb_error("Invalid key.\n");
    return false;
  }

  switch (algorithm) {
    case AVB_ALGORITHM_TYPE_MLDSA65:
      expected_key_num_bytes = 1952;
      break;
    case AVB_ALGORITHM_TYPE_MLDSA87:
      expected_key_num_bytes = 2592;
      break;
    default:
      avb_error("Unexpected algorithm.\n");
      return false;
  }

  if (out_header->key_num_bytes != expected_key_num_bytes) {
    avb_error("Unexpected key length.\n");
    return false;
  }

  if (key_num_bytes !=
      sizeof(AvbMLDSAPublicKeyHeader) + out_header->key_num_bytes) {
    avb_error("Key does not match expected length.\n");
    return false;
  }

  return true;
}

bool avb_mldsa_prehash_init(AvbMLDSAPrehashCtx* ctx,
                            AvbAlgorithmType algorithm,
                            const uint8_t* key,
                            size_t key_num_bytes,
                            const uint8_t* context,
                            size_t context_len) {
  AvbMLDSAPublicKeyHeader h;

  if (ctx == NULL) {
    avb_error("Invalid input.\n");
    return false;
  }

  if (!avb_mldsa_validate_public_key(key, key_num_bytes, algorithm, &h)) {
    return false;
  }

  return avb_mldsa_prehash_init_impl(ctx,
                                     algorithm,
                                     key + sizeof(AvbMLDSAPublicKeyHeader),
                                     h.key_num_bytes,
                                     context,
                                     context_len);
}

bool avb_mldsa_verify_message_representative(
    AvbAlgorithmType algorithm,
    const uint8_t* key,
    size_t key_num_bytes,
    const uint8_t* sig,
    size_t sig_num_bytes,
    const uint8_t msg_rep[AVB_MLDSA_MU_BYTES]) {
  AvbMLDSAPublicKeyHeader h;
  uint32_t expected_sig_num_bytes;

  if (key == NULL || sig == NULL || msg_rep == NULL) {
    avb_error("Invalid input.\n");
    return false;
  }

  if (!avb_mldsa_validate_public_key(key, key_num_bytes, algorithm, &h)) {
    return false;
  }

  switch (algorithm) {
    case AVB_ALGORITHM_TYPE_MLDSA65:
      expected_sig_num_bytes = 3309;
      break;
    case AVB_ALGORITHM_TYPE_MLDSA87:
      expected_sig_num_bytes = 4627;
      break;
    default:
      avb_error("Unexpected algorithm.\n");
      return false;
  }

  if (sig_num_bytes != expected_sig_num_bytes) {
    avb_error("Signature length does not match key length.\n");
    return false;
  }

  return avb_mldsa_verify_message_representative_impl(
      algorithm,
      key + sizeof(AvbMLDSAPublicKeyHeader),
      h.key_num_bytes,
      sig,
      sig_num_bytes,
      msg_rep);
}
