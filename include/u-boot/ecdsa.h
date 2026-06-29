/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Alexandru Gagniuc <mr.nuke.me@gmail.com>.
 */

#ifndef _ECDSA_H
#define _ECDSA_H

#include <errno.h>
#include <image.h>
#include <string.h>

/**
 * crypto_algo API impementation for ECDSA;
 * @see "struct crypto_algo"
 * @{
 */
/**
 * sign() - calculate and return signature for given input data
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sigp:	Set to an allocated buffer holding the signature
 * @sig_len:	Set to length of the calculated hash
 *
 * This computes input data signature according to selected algorithm.
 * Resulting signature value is placed in an allocated buffer, the
 * pointer is returned as *sigp. The length of the calculated
 * signature is returned via the sig_len pointer argument. The caller
 * should free *sigp.
 *
 * @return: 0, on success, -ve on error
 */
int ecdsa_sign(struct image_sign_info *info, const struct image_region region[],
	       int region_count, uint8_t **sigp, uint *sig_len);

/**
 * add_verify_data() - Add verification information to FDT
 *
 * Add public key information to the FDT node, suitable for
 * verification at run-time. The information added depends on the
 * algorithm being used. I just copypasted this from rsa.h.
 *
 * @info:	Specifies key and FIT information
 * @keydest:	Destination FDT blob for public key data
 * @return: node offset within the FDT blob where the data was written on
 *	success, -ENOSPC if the keydest FDT blob ran out of space, other -ve
 *	value on other error
 */
int ecdsa_add_verify_data(struct image_sign_info *info, void *keydest);

/**
 * verify() - Verify a signature against some data
 *
 * @info:	Specifies key and FIT information
 * @data:	Pointer to the input data
 * @data_len:	Data length
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * Return: 0 if verified, -ve on error
 */
int ecdsa_verify(struct image_sign_info *info,
		 const struct image_region region[], int region_count,
		 uint8_t *sig, uint sig_len);
/** @} */

#define ECDSA224_BYTES	(224 / 8)
#define ECDSA256_BYTES	(256 / 8)
#define ECDSA384_BYTES	(384 / 8)
#define ECDSA521_BYTES	((521 + 7) / 8)

/**
 * ecdsa_curve_size() - Get the size in bits for a named ECDSA curve
 *
 * @curve_name: OpenSSL short name for the curve
 * Return: curve size in bits, or 0 if @curve_name is not supported
 */
static inline unsigned int ecdsa_curve_size(const char *curve_name)
{
	if (!strcmp(curve_name, "secp224r1"))
		return 224;
	else if (!strcmp(curve_name, "prime256v1"))
		return 256;
	else if (!strcmp(curve_name, "secp384r1"))
		return 384;
	else if (!strcmp(curve_name, "secp521r1"))
		return 521;

	return 0;
}

#endif
