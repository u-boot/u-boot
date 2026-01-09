/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2026, Philippe Reynes <philippe.reynes@softathome.com>
 */
#ifndef _ECDSA_HELPER_
#define _ECDSA_HELPER_

#include <linux/types.h>

/**
 * struct ecdsa_public_key - ECDSA public key properties
 *
 * The struct has pointers to the (x, y) curve coordinates to an ECDSA public
 * key, as well as the name of the ECDSA curve. The size of the key is inferred
 * from the 'curve_name'
 */
struct ecdsa_public_key {
	const char *curve_name;	/* Name of curve, e.g. "prime256v1" */
	const void *x;		/* x coordinate of public key */
	const void *y;		/* y coordinate of public key */
	unsigned int size_bits;	/* key size in bits, derived from curve name */
};

/**
 * ecdsa_hash_verify() - Verify the ecdsa signature of a hash
 *
 * @pubkey: ecdsa public key
 * @hash: Hash
 * @hash_len: Size of the hash
 * @signature: Signature
 * @sig_len: Size of the signature
 *
 * Return: 0 if all verified ok, <0 on error
 */
int ecdsa_hash_verify(const struct ecdsa_public_key *pubkey,
		      const void *hash, size_t hash_len,
		      const void *signature, size_t sig_len);

#endif
