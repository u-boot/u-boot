/*
 * Copyright (c) 2014, Ruchika Gupta.
 *
 * SPDX-License-Identifier:    GPL-2.0+
*/

#ifndef _RSA_MOD_EXP_H
#define _RSA_MOD_EXP_H

#include <errno.h>
#include <image.h>

/**
 * struct key_prop - holder for a public key properties
 *
 * The struct has pointers to modulus (Typically called N),
 * The inverse, R^2, exponent. These can be typecasted and
 * used as byte arrays or converted to the required format
 * as per requirement of RSA implementation.
 */
struct key_prop {
	const void *rr;		/* R^2 can be treated as byte array */
	const void *modulus;	/* modulus as byte array */
	const void *public_exponent; /* public exponent as byte array */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	int num_bits;		/* Key length in bits */
	uint32_t exp_len;	/* Exponent length in number of uint8_t */
};

/**
 * rsa_mod_exp_sw() - Perform RSA Modular Exponentiation in sw
 *
 * Operation: out[] = sig ^ exponent % modulus
 *
 * @sig:	RSA PKCS1.5 signature
 * @sig_len:	Length of signature in number of bytes
 * @node:	Node with RSA key elements like modulus, exponent, R^2, n0inv
 * @out:	Result in form of byte array
 */
int rsa_mod_exp_sw(const uint8_t *sig, uint32_t sig_len,
		struct key_prop *node, uint8_t *out);

#endif
