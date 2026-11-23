// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2025 The Android Open Source Project
 */

#include "../avb_mldsa.h"
#include "../avb_sysdeps.h"
#include "../avb_util.h"

bool avb_mldsa_prehash_init_impl(AvbMLDSAPrehashCtx* ctx,
                                 AvbAlgorithmType algorithm,
                                 const uint8_t* key,
                                 size_t key_num_bytes,
                                 const uint8_t* context,
                                 size_t context_len) {
  (void)ctx;
  (void)algorithm;
  (void)key;
  (void)key_num_bytes;
  (void)context;
  (void)context_len;
  avb_error("ML-DSA prehashing not implemented.\n");
  return false;
}

void avb_mldsa_prehash_update(AvbMLDSAPrehashCtx* ctx,
                              const uint8_t* data,
                              size_t data_len) {
  (void)ctx;
  (void)data;
  (void)data_len;
}

bool avb_mldsa_prehash_finalize(AvbMLDSAPrehashCtx* ctx,
                                uint8_t out_msg_rep[AVB_MLDSA_MU_BYTES]) {
  (void)ctx;
  (void)out_msg_rep;
  avb_error("ML-DSA prehashing not implemented.\n");
  return false;
}

bool avb_mldsa_verify_message_representative_impl(
    AvbAlgorithmType algorithm,
    const uint8_t* key,
    size_t key_num_bytes,
    const uint8_t* sig,
    size_t sig_num_bytes,
    const uint8_t msg_rep[AVB_MLDSA_MU_BYTES]) {
  (void)algorithm;
  (void)key;
  (void)key_num_bytes;
  (void)sig;
  (void)sig_num_bytes;
  (void)msg_rep;
  avb_error("ML-DSA verification not implemented.\n");
  return false;
}
