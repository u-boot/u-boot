// SPDX-License-Identifier: GPL-2.0+
/*
 * Public key helper functions using MbedTLS X509 library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <linux/compat.h>
#include <crypto/public_key.h>

int public_key_verify_signature(const struct public_key *pkey,
				const struct public_key_signature *sig)
{
	mbedtls_md_type_t mb_hash_algo;
	mbedtls_pk_context pk_ctx;
	int ret;

	if (!pkey || !sig || pkey->key_is_private)
		return -EINVAL;

	/*
	 * ECRDSA (Elliptic Curve Russian Digital Signature Algorithm) is not
	 * supported by MbedTLS.
	 */
	if (strcmp(pkey->pkey_algo, "rsa")) {
		pr_err("Encryption is not RSA: %s\n", sig->pkey_algo);
		return -EINVAL;
	}

	/*
	 * Can be pkcs1 or raw, but pkcs1 is expected.
	 * This is just for argument checking, not necessarily passed to MbedTLS,
	 * For RSA signatures, MbedTLS typically supports the PKCS#1 v1.5
	 * (aka. pkcs1) encoding by default.
	 * The library internally handles the details of decoding and verifying
	 * the signature according to the expected encoding for the specified algorithm.
	 */
	if (strcmp(sig->encoding, "pkcs1")) {
		pr_err("Encoding %s is not supported, only supports pkcs1\n",
		       sig->encoding);
		return -EINVAL;
	}

	if (!strcmp(sig->hash_algo, "sha1"))
		mb_hash_algo = MBEDTLS_MD_SHA1;
	else if (!strcmp(sig->hash_algo, "sha224"))
		mb_hash_algo = MBEDTLS_MD_SHA224;
	else if (!strcmp(sig->hash_algo, "sha256"))
		mb_hash_algo = MBEDTLS_MD_SHA256;
	else if (!strcmp(sig->hash_algo, "sha384"))
		mb_hash_algo = MBEDTLS_MD_SHA384;
	else if (!strcmp(sig->hash_algo, "sha512"))
		mb_hash_algo = MBEDTLS_MD_SHA512;
	else	/* Unknown or unsupported hash algorithm */
		return -EINVAL;
	/* Initialize the mbedtls_pk_context with RSA key type */
	mbedtls_pk_init(&pk_ctx);

	/* Parse the DER-encoded public key */
	ret = mbedtls_pk_parse_public_key(&pk_ctx, pkey->key, pkey->keylen);
	if (ret) {
		pr_err("Failed to parse public key, ret:-0x%04x\n", -ret);
		ret = -EINVAL;
		goto err_key;
	}

	/* Ensure that it is a RSA key */
	if (mbedtls_pk_get_type(&pk_ctx) != MBEDTLS_PK_RSA) {
		pr_err("Only RSA keys are supported\n");
		ret = -EKEYREJECTED;
		goto err_key;
	}

	/* Verify the hash */
	ret = mbedtls_pk_verify(&pk_ctx, mb_hash_algo, sig->digest,
				sig->digest_size, sig->s, sig->s_size);

err_key:
	mbedtls_pk_free(&pk_ctx);
	return ret;
}
