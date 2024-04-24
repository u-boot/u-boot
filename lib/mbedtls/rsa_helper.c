// SPDX-License-Identifier: GPL-2.0+
/*
 * RSA helper functions using MbedTLS
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <linux/err.h>
#include <crypto/internal/rsa.h>
#include <library/common.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/asn1.h>

/**
 * rsa_parse_pub_key() - decodes the BER encoded buffer and stores in the
 *                       provided struct rsa_key, pointers to the raw key as is,
 *                       so that the caller can copy it or MPI parse it, etc.
 *
 * @rsa_key:	struct rsa_key key representation
 * @key:	key in BER format
 * @key_len:	length of key
 *
 * Return:	0 on success or error code in case of error
 */
int rsa_parse_pub_key(struct rsa_key *rsa_key, const void *key,
		      unsigned int key_len)
{
	int ret = 0;
	mbedtls_pk_context pk;
	mbedtls_rsa_context *rsa;

	mbedtls_pk_init(&pk);

	ret = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)key,
					  key_len);
	if (ret) {
		pr_err("Failed to parse public key, ret:-0x%04x\n", -ret);
		ret = -EINVAL;
		goto clean_pubkey;
	}

	/* Ensure that it is a RSA key */
	if (mbedtls_pk_get_type(&pk) != MBEDTLS_PK_RSA) {
		pr_err("Non-RSA keys are not supported\n");
		ret = -EKEYREJECTED;
		goto clean_pubkey;
	}

	/* Get RSA key context */
	rsa = mbedtls_pk_rsa(pk);
	if (!rsa) {
		pr_err("Failed to get RSA key context, ret:-0x%04x\n", -ret);
		ret = -EINVAL;
		goto clean_pubkey;
	}

	/* Parse modulus (n) */
	rsa_key->n_sz = mbedtls_mpi_size(&rsa->N);
	rsa_key->n = kzalloc(rsa_key->n_sz, GFP_KERNEL);
	if (!rsa_key->n) {
		ret = -ENOMEM;
		goto clean_pubkey;
	}
	ret = mbedtls_mpi_write_binary(&rsa->N, (unsigned char *)rsa_key->n,
				       rsa_key->n_sz);
	if (ret) {
		pr_err("Failed to parse modulus (n), ret:-0x%04x\n", -ret);
		ret = -EINVAL;
		goto clean_modulus;
	}

	/* Parse public exponent (e) */
	rsa_key->e_sz = mbedtls_mpi_size(&rsa->E);
	rsa_key->e = kzalloc(rsa_key->e_sz, GFP_KERNEL);
	if (!rsa_key->e) {
		ret = -ENOMEM;
		goto clean_modulus;
	}
	ret = mbedtls_mpi_write_binary(&rsa->E, (unsigned char *)rsa_key->e,
				       rsa_key->e_sz);
	if (!ret)
		return 0;

	pr_err("Failed to parse public exponent (e), ret:-0x%04x\n", -ret);
	ret = -EINVAL;

	kfree(rsa_key->e);
clean_modulus:
	kfree(rsa_key->n);
clean_pubkey:
	mbedtls_pk_free(&pk);
	return ret;
}
