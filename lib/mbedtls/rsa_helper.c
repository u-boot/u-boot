// SPDX-License-Identifier: GPL-2.0+
/*
 * RSA helper functions using MbedTLS
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <linux/err.h>
#include <crypto/internal/rsa.h>
#include <external/mbedtls/library/common.h>
#include <external/mbedtls/include/mbedtls/pk.h>
#include <external/mbedtls/include/mbedtls/rsa.h>
#include <external/mbedtls/include/mbedtls/asn1.h>

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
	if (ret)
		goto clean_pubkey;

	/* Extract RSA context from the parsed key */
	rsa = mbedtls_pk_rsa(pk);
	if (!rsa) {
		ret = -EINVAL;;
		goto clean_pubkey;
	}

	/* Copy modulus (n) */
	rsa_key->n_sz = mbedtls_rsa_get_len(rsa);
	rsa_key->n = kzalloc(rsa_key->n_sz, GFP_KERNEL);
	if (!rsa_key->n) {
		ret = -ENOMEM;
		goto clean_pubkey;
	}
	ret = mbedtls_mpi_write_binary(&rsa->N, (unsigned char *)rsa_key->n,
				       rsa_key->n_sz);
	if (ret)
		goto clean_modulus;

	/* Copy public exponent (e) */
	rsa_key->e_sz = mbedtls_rsa_get_len(rsa);
	rsa_key->e = kzalloc(rsa_key->e_sz, GFP_KERNEL);
	if (!rsa_key->e) {
		ret = -ENOMEM;
		goto clean_modulus;
	}
	ret = mbedtls_mpi_write_binary(&rsa->E, (unsigned char *)rsa_key->e,
				       rsa_key->e_sz);
	if (!ret)
		return 0;

	kfree(rsa_key->e);
clean_modulus:
	kfree(rsa_key->n);
clean_pubkey:
	mbedtls_pk_free(&pk);
	return ret;
}
EXPORT_SYMBOL_GPL(rsa_parse_pub_key);
