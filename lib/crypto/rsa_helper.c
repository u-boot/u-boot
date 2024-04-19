// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * RSA key extract helper
 *
 * Copyright (c) 2015, Intel Corporation
 * Authors: Tadeusz Struk <tadeusz.struk@intel.com>
 */
#ifndef __UBOOT__
#include <linux/compat.h>
#include <linux/kernel.h>
#include <linux/export.h>
#endif
#include <linux/err.h>
#ifndef __UBOOT__
#include <linux/fips.h>
#endif
#include <crypto/internal/rsa.h>
#include <linux/printk.h>
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include <external/mbedtls/library/common.h>
#include <external/mbedtls/include/mbedtls/pk.h>
#include <external/mbedtls/include/mbedtls/rsa.h>
#include <external/mbedtls/include/mbedtls/asn1.h>
#else
#include "rsapubkey.asn1.h"
#endif
#ifndef __UBOOT__
#include "rsaprivkey.asn1.h"
#endif

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)

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

#else /* !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

int rsa_get_n(void *context, size_t hdrlen, unsigned char tag,
	      const void *value, size_t vlen)
{
	struct rsa_key *key = context;
#ifndef __UBOOT__
	const u8 *ptr = value;
	size_t n_sz = vlen;
#endif

	/* invalid key provided */
	if (!value || !vlen)
		return -EINVAL;

#ifndef __UBOOT__
	if (fips_enabled) {
		while (n_sz && !*ptr) {
			ptr++;
			n_sz--;
		}

		/* In FIPS mode only allow key size 2K and higher */
		if (n_sz < 256) {
			pr_err("RSA: key size not allowed in FIPS mode\n");
			return -EINVAL;
		}
	}
#endif

	key->n = value;
	key->n_sz = vlen;

	return 0;
}

int rsa_get_e(void *context, size_t hdrlen, unsigned char tag,
	      const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !key->n_sz || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->e = value;
	key->e_sz = vlen;

	return 0;
}

int rsa_get_d(void *context, size_t hdrlen, unsigned char tag,
	      const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !key->n_sz || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->d = value;
	key->d_sz = vlen;

	return 0;
}

int rsa_get_p(void *context, size_t hdrlen, unsigned char tag,
	      const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->p = value;
	key->p_sz = vlen;

	return 0;
}

int rsa_get_q(void *context, size_t hdrlen, unsigned char tag,
	      const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->q = value;
	key->q_sz = vlen;

	return 0;
}

int rsa_get_dp(void *context, size_t hdrlen, unsigned char tag,
	       const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->dp = value;
	key->dp_sz = vlen;

	return 0;
}

int rsa_get_dq(void *context, size_t hdrlen, unsigned char tag,
	       const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->dq = value;
	key->dq_sz = vlen;

	return 0;
}

int rsa_get_qinv(void *context, size_t hdrlen, unsigned char tag,
		 const void *value, size_t vlen)
{
	struct rsa_key *key = context;

	/* invalid key provided */
	if (!value || !vlen || vlen > key->n_sz)
		return -EINVAL;

	key->qinv = value;
	key->qinv_sz = vlen;

	return 0;
}

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
	return asn1_ber_decoder(&rsapubkey_decoder, rsa_key, key, key_len);
}
EXPORT_SYMBOL_GPL(rsa_parse_pub_key);

#endif /* CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

#ifndef __UBOOT__
/**
 * rsa_parse_priv_key() - decodes the BER encoded buffer and stores in the
 *                        provided struct rsa_key, pointers to the raw key
 *                        as is, so that the caller can copy it or MPI parse it,
 *                        etc.
 *
 * @rsa_key:	struct rsa_key key representation
 * @key:	key in BER format
 * @key_len:	length of key
 *
 * Return:	0 on success or error code in case of error
 */
int rsa_parse_priv_key(struct rsa_key *rsa_key, const void *key,
		       unsigned int key_len)
{
	return asn1_ber_decoder(&rsaprivkey_decoder, rsa_key, key, key_len);
}
EXPORT_SYMBOL_GPL(rsa_parse_priv_key);
#endif
