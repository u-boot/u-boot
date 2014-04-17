/*
 * Copyright (c) 2013, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "mkimage.h"
#include <stdio.h>
#include <string.h>
#include <image.h>
#include <time.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define HAVE_ERR_REMOVE_THREAD_STATE
#endif

static int rsa_err(const char *msg)
{
	unsigned long sslErr = ERR_get_error();

	fprintf(stderr, "%s", msg);
	fprintf(stderr, ": %s\n",
		ERR_error_string(sslErr, 0));

	return -1;
}

/**
 * rsa_get_pub_key() - read a public key from a .crt file
 *
 * @keydir:	Directory containins the key
 * @name	Name of key file (will have a .crt extension)
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_get_pub_key(const char *keydir, const char *name, RSA **rsap)
{
	char path[1024];
	EVP_PKEY *key;
	X509 *cert;
	RSA *rsa;
	FILE *f;
	int ret;

	*rsap = NULL;
	snprintf(path, sizeof(path), "%s/%s.crt", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA certificate: '%s': %s\n",
			path, strerror(errno));
		return -EACCES;
	}

	/* Read the certificate */
	cert = NULL;
	if (!PEM_read_X509(f, &cert, NULL, NULL)) {
		rsa_err("Couldn't read certificate");
		ret = -EINVAL;
		goto err_cert;
	}

	/* Get the public key from the certificate. */
	key = X509_get_pubkey(cert);
	if (!key) {
		rsa_err("Couldn't read public key\n");
		ret = -EINVAL;
		goto err_pubkey;
	}

	/* Convert to a RSA_style key. */
	rsa = EVP_PKEY_get1_RSA(key);
	if (!rsa) {
		rsa_err("Couldn't convert to a RSA style key");
		goto err_rsa;
	}
	fclose(f);
	EVP_PKEY_free(key);
	X509_free(cert);
	*rsap = rsa;

	return 0;

err_rsa:
	EVP_PKEY_free(key);
err_pubkey:
	X509_free(cert);
err_cert:
	fclose(f);
	return ret;
}

/**
 * rsa_get_priv_key() - read a private key from a .key file
 *
 * @keydir:	Directory containins the key
 * @name	Name of key file (will have a .key extension)
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_get_priv_key(const char *keydir, const char *name, RSA **rsap)
{
	char path[1024];
	RSA *rsa;
	FILE *f;

	*rsap = NULL;
	snprintf(path, sizeof(path), "%s/%s.key", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA private key: '%s': %s\n",
			path, strerror(errno));
		return -ENOENT;
	}

	rsa = PEM_read_RSAPrivateKey(f, 0, NULL, path);
	if (!rsa) {
		rsa_err("Failure reading private key");
		fclose(f);
		return -EPROTO;
	}
	fclose(f);
	*rsap = rsa;

	return 0;
}

static int rsa_init(void)
{
	int ret;

	ret = SSL_library_init();
	if (!ret) {
		fprintf(stderr, "Failure to init SSL library\n");
		return -1;
	}
	SSL_load_error_strings();

	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_ciphers();

	return 0;
}

static void rsa_remove(void)
{
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
#ifdef HAVE_ERR_REMOVE_THREAD_STATE
	ERR_remove_thread_state(NULL);
#else
	ERR_remove_state(0);
#endif
	EVP_cleanup();
}

static int rsa_sign_with_key(RSA *rsa, struct checksum_algo *checksum_algo,
		const struct image_region region[], int region_count,
		uint8_t **sigp, uint *sig_size)
{
	EVP_PKEY *key;
	EVP_MD_CTX *context;
	int size, ret = 0;
	uint8_t *sig;
	int i;

	key = EVP_PKEY_new();
	if (!key)
		return rsa_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(key, rsa)) {
		ret = rsa_err("EVP key setup failed");
		goto err_set;
	}

	size = EVP_PKEY_size(key);
	sig = malloc(size);
	if (!sig) {
		fprintf(stderr, "Out of memory for signature (%d bytes)\n",
			size);
		ret = -ENOMEM;
		goto err_alloc;
	}

	context = EVP_MD_CTX_create();
	if (!context) {
		ret = rsa_err("EVP context creation failed");
		goto err_create;
	}
	EVP_MD_CTX_init(context);
	if (!EVP_SignInit(context, checksum_algo->calculate_sign())) {
		ret = rsa_err("Signer setup failed");
		goto err_sign;
	}

	for (i = 0; i < region_count; i++) {
		if (!EVP_SignUpdate(context, region[i].data, region[i].size)) {
			ret = rsa_err("Signing data failed");
			goto err_sign;
		}
	}

	if (!EVP_SignFinal(context, sig, sig_size, key)) {
		ret = rsa_err("Could not obtain signature");
		goto err_sign;
	}
	EVP_MD_CTX_cleanup(context);
	EVP_MD_CTX_destroy(context);
	EVP_PKEY_free(key);

	debug("Got signature: %d bytes, expected %d\n", *sig_size, size);
	*sigp = sig;
	*sig_size = size;

	return 0;

err_sign:
	EVP_MD_CTX_destroy(context);
err_create:
	free(sig);
err_alloc:
err_set:
	EVP_PKEY_free(key);
	return ret;
}

int rsa_sign(struct image_sign_info *info,
	     const struct image_region region[], int region_count,
	     uint8_t **sigp, uint *sig_len)
{
	RSA *rsa;
	int ret;

	ret = rsa_init();
	if (ret)
		return ret;

	ret = rsa_get_priv_key(info->keydir, info->keyname, &rsa);
	if (ret)
		goto err_priv;
	ret = rsa_sign_with_key(rsa, info->algo->checksum, region,
				region_count, sigp, sig_len);
	if (ret)
		goto err_sign;

	RSA_free(rsa);
	rsa_remove();

	return ret;

err_sign:
	RSA_free(rsa);
err_priv:
	rsa_remove();
	return ret;
}

/*
 * rsa_get_params(): - Get the important parameters of an RSA public key
 */
int rsa_get_params(RSA *key, uint32_t *n0_invp, BIGNUM **modulusp,
		   BIGNUM **r_squaredp)
{
	BIGNUM *big1, *big2, *big32, *big2_32;
	BIGNUM *n, *r, *r_squared, *tmp;
	BN_CTX *bn_ctx = BN_CTX_new();
	int ret = 0;

	/* Initialize BIGNUMs */
	big1 = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	r = BN_new();
	r_squared = BN_new();
	tmp = BN_new();
	big2_32 = BN_new();
	n = BN_new();
	if (!big1 || !big2 || !big32 || !r || !r_squared || !tmp || !big2_32 ||
	    !n) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}

	if (!BN_copy(n, key->n) || !BN_set_word(big1, 1L) ||
	    !BN_set_word(big2, 2L) || !BN_set_word(big32, 32L))
		ret = -1;

	/* big2_32 = 2^32 */
	if (!BN_exp(big2_32, big2, big32, bn_ctx))
		ret = -1;

	/* Calculate n0_inv = -1 / n[0] mod 2^32 */
	if (!BN_mod_inverse(tmp, n, big2_32, bn_ctx) ||
	    !BN_sub(tmp, big2_32, tmp))
		ret = -1;
	*n0_invp = BN_get_word(tmp);

	/* Calculate R = 2^(# of key bits) */
	if (!BN_set_word(tmp, BN_num_bits(n)) ||
	    !BN_exp(r, big2, tmp, bn_ctx))
		ret = -1;

	/* Calculate r_squared = R^2 mod n */
	if (!BN_copy(r_squared, r) ||
	    !BN_mul(tmp, r_squared, r, bn_ctx) ||
	    !BN_mod(r_squared, tmp, n, bn_ctx))
		ret = -1;

	*modulusp = n;
	*r_squaredp = r_squared;

	BN_free(big1);
	BN_free(big2);
	BN_free(big32);
	BN_free(r);
	BN_free(tmp);
	BN_free(big2_32);
	if (ret) {
		fprintf(stderr, "Bignum operations failed\n");
		return -ENOMEM;
	}

	return ret;
}

static int fdt_add_bignum(void *blob, int noffset, const char *prop_name,
			  BIGNUM *num, int num_bits)
{
	int nwords = num_bits / 32;
	int size;
	uint32_t *buf, *ptr;
	BIGNUM *tmp, *big2, *big32, *big2_32;
	BN_CTX *ctx;
	int ret;

	tmp = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	big2_32 = BN_new();
	if (!tmp || !big2 || !big32 || !big2_32) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}
	ctx = BN_CTX_new();
	if (!tmp) {
		fprintf(stderr, "Out of memory (bignum context)\n");
		return -ENOMEM;
	}
	BN_set_word(big2, 2L);
	BN_set_word(big32, 32L);
	BN_exp(big2_32, big2, big32, ctx); /* B = 2^32 */

	size = nwords * sizeof(uint32_t);
	buf = malloc(size);
	if (!buf) {
		fprintf(stderr, "Out of memory (%d bytes)\n", size);
		return -ENOMEM;
	}

	/* Write out modulus as big endian array of integers */
	for (ptr = buf + nwords - 1; ptr >= buf; ptr--) {
		BN_mod(tmp, num, big2_32, ctx); /* n = N mod B */
		*ptr = cpu_to_fdt32(BN_get_word(tmp));
		BN_rshift(num, num, 32); /*  N = N/B */
	}

	ret = fdt_setprop(blob, noffset, prop_name, buf, size);
	if (ret) {
		fprintf(stderr, "Failed to write public key to FIT\n");
		return -ENOSPC;
	}
	free(buf);
	BN_free(tmp);
	BN_free(big2);
	BN_free(big32);
	BN_free(big2_32);

	return ret;
}

int rsa_add_verify_data(struct image_sign_info *info, void *keydest)
{
	BIGNUM *modulus, *r_squared;
	uint32_t n0_inv;
	int parent, node;
	char name[100];
	int ret;
	int bits;
	RSA *rsa;

	debug("%s: Getting verification data\n", __func__);
	ret = rsa_get_pub_key(info->keydir, info->keyname, &rsa);
	if (ret)
		return ret;
	ret = rsa_get_params(rsa, &n0_inv, &modulus, &r_squared);
	if (ret)
		return ret;
	bits = BN_num_bits(modulus);
	parent = fdt_subnode_offset(keydest, 0, FIT_SIG_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(keydest, 0, FIT_SIG_NODENAME);
		if (parent < 0) {
			fprintf(stderr, "Couldn't create signature node: %s\n",
				fdt_strerror(parent));
			return -EINVAL;
		}
	}

	/* Either create or overwrite the named key node */
	snprintf(name, sizeof(name), "key-%s", info->keyname);
	node = fdt_subnode_offset(keydest, parent, name);
	if (node == -FDT_ERR_NOTFOUND) {
		node = fdt_add_subnode(keydest, parent, name);
		if (node < 0) {
			fprintf(stderr, "Could not create key subnode: %s\n",
				fdt_strerror(node));
			return -EINVAL;
		}
	} else if (node < 0) {
		fprintf(stderr, "Cannot select keys parent: %s\n",
			fdt_strerror(node));
		return -ENOSPC;
	}

	ret = fdt_setprop_string(keydest, node, "key-name-hint",
				 info->keyname);
	ret |= fdt_setprop_u32(keydest, node, "rsa,num-bits", bits);
	ret |= fdt_setprop_u32(keydest, node, "rsa,n0-inverse", n0_inv);
	ret |= fdt_add_bignum(keydest, node, "rsa,modulus", modulus, bits);
	ret |= fdt_add_bignum(keydest, node, "rsa,r-squared", r_squared, bits);
	ret |= fdt_setprop_string(keydest, node, FIT_ALGO_PROP,
				  info->algo->name);
	if (info->require_keys) {
		fdt_setprop_string(keydest, node, "required",
				   info->require_keys);
	}
	BN_free(modulus);
	BN_free(r_squared);
	if (ret)
		return -EIO;

	return 0;
}
