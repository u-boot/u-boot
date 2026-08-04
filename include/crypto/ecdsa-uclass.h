/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 */

#include <dm/device.h>
#include <crypto/internal/ecdsa.h>

struct ecdsa_ops {
	/**
	 * Verify signature of hash against given public key
	 *
	 * @dev:	ECDSA Device
	 * @pubkey:	ECDSA public key
	 * @hash:	Hash of binary image
	 * @hash_len:	Length of hash in bytes
	 * @signature:	Signature in a raw (R, S) point pair
	 * @sig_len:	Length of signature in bytes
	 *
	 * This function verifies that the 'signature' of the given 'hash' was
	 * signed by the private key corresponding to 'pubkey'.
	 */
	int (*verify)(struct udevice *dev, const struct ecdsa_public_key *pubkey,
		      const void *hash, size_t hash_len,
		      const void *signature, size_t sig_len);
};
