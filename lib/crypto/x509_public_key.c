// SPDX-License-Identifier: GPL-2.0-or-later
/* Instantiate a public key crypto key from an X.509 Certificate
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#define pr_fmt(fmt) "X.509: "fmt
#ifdef __UBOOT__
#include <image.h>
#include <dm/devres.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/printk.h>
#else
#include <linux/module.h>
#endif
#include <linux/kernel.h>
#ifdef __UBOOT__
#include <crypto/x509_parser.h>
#include <u-boot/hash-checksum.h>
#else
#include <linux/slab.h>
#include <keys/asymmetric-subtype.h>
#include <keys/asymmetric-parser.h>
#include <keys/system_keyring.h>
#include <crypto/hash.h>
#include "asymmetric_keys.h"
#include "x509_parser.h"
#endif

#if !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)

/*
 * Set up the signature parameters in an X.509 certificate.  This involves
 * digesting the signed data and extracting the signature.
 */
int x509_get_sig_params(struct x509_certificate *cert)
{
	struct public_key_signature *sig = cert->sig;
#ifdef __UBOOT__
	struct image_region region;
#else
	struct crypto_shash *tfm;
	struct shash_desc *desc;
	size_t desc_size;
#endif
	int ret;

	pr_devel("==>%s()\n", __func__);

	if (!cert->pub->pkey_algo)
		cert->unsupported_key = true;

	if (!sig->pkey_algo)
		cert->unsupported_sig = true;

	/* We check the hash if we can - even if we can't then verify it */
	if (!sig->hash_algo) {
		cert->unsupported_sig = true;
		return 0;
	}

	sig->s = kmemdup(cert->raw_sig, cert->raw_sig_size, GFP_KERNEL);
	if (!sig->s)
		return -ENOMEM;

	sig->s_size = cert->raw_sig_size;

#ifdef __UBOOT__
	if (!sig->hash_algo)
		return -ENOPKG;
	if (!strcmp(sig->hash_algo, "sha256"))
		sig->digest_size = SHA256_SUM_LEN;
	else if (!strcmp(sig->hash_algo, "sha384"))
		sig->digest_size = SHA384_SUM_LEN;
	else if (!strcmp(sig->hash_algo, "sha512"))
		sig->digest_size = SHA512_SUM_LEN;
	else if (!strcmp(sig->hash_algo, "sha1"))
		sig->digest_size = SHA1_SUM_LEN;
	else
		return -ENOPKG;

	sig->digest = calloc(1, sig->digest_size);
	if (!sig->digest)
		return -ENOMEM;

	region.data = cert->tbs;
	region.size = cert->tbs_size;
	hash_calculate(sig->hash_algo, &region, 1, sig->digest);

	/* TODO: is_hash_blacklisted()? */

	ret = 0;
#else
	/* Allocate the hashing algorithm we're going to need and find out how
	 * big the hash operational data will be.
	 */
	tfm = crypto_alloc_shash(sig->hash_algo, 0, 0);
	if (IS_ERR(tfm)) {
		if (PTR_ERR(tfm) == -ENOENT) {
			cert->unsupported_sig = true;
			return 0;
		}
		return PTR_ERR(tfm);
	}

	desc_size = crypto_shash_descsize(tfm) + sizeof(*desc);
	sig->digest_size = crypto_shash_digestsize(tfm);

	ret = -ENOMEM;
	sig->digest = kmalloc(sig->digest_size, GFP_KERNEL);
	if (!sig->digest)
		goto error;

	desc = kzalloc(desc_size, GFP_KERNEL);
	if (!desc)
		goto error;

	desc->tfm = tfm;

	ret = crypto_shash_digest(desc, cert->tbs, cert->tbs_size, sig->digest);
	if (ret < 0)
		goto error_2;

	ret = is_hash_blacklisted(sig->digest, sig->digest_size, "tbs");
	if (ret == -EKEYREJECTED) {
		pr_err("Cert %*phN is blacklisted\n",
		       sig->digest_size, sig->digest);
		cert->blacklisted = true;
		ret = 0;
	}

error_2:
	kfree(desc);
error:
	crypto_free_shash(tfm);
#endif /* __UBOOT__ */
	pr_devel("<==%s() = %d\n", __func__, ret);
	return ret;
}

#endif /* !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

#ifndef __UBOOT__
/*
 * Attempt to parse a data blob for a key as an X509 certificate.
 */
static int x509_key_preparse(struct key_preparsed_payload *prep)
{
	struct asymmetric_key_ids *kids;
	struct x509_certificate *cert;
	const char *q;
	size_t srlen, sulen;
	char *desc = NULL, *p;
	int ret;

	cert = x509_cert_parse(prep->data, prep->datalen);
	if (IS_ERR(cert))
		return PTR_ERR(cert);

	pr_devel("Cert Issuer: %s\n", cert->issuer);
	pr_devel("Cert Subject: %s\n", cert->subject);

	if (cert->unsupported_key) {
		ret = -ENOPKG;
		goto error_free_cert;
	}

	pr_devel("Cert Key Algo: %s\n", cert->pub->pkey_algo);
	pr_devel("Cert Valid period: %lld-%lld\n", cert->valid_from, cert->valid_to);

	cert->pub->id_type = "X509";

	if (cert->unsupported_sig) {
		public_key_signature_free(cert->sig);
		cert->sig = NULL;
	} else {
		pr_devel("Cert Signature: %s + %s\n",
			 cert->sig->pkey_algo, cert->sig->hash_algo);
	}

	/* Don't permit addition of blacklisted keys */
	ret = -EKEYREJECTED;
	if (cert->blacklisted)
		goto error_free_cert;

	/* Propose a description */
	sulen = strlen(cert->subject);
	if (cert->raw_skid) {
		srlen = cert->raw_skid_size;
		q = cert->raw_skid;
	} else {
		srlen = cert->raw_serial_size;
		q = cert->raw_serial;
	}

	ret = -ENOMEM;
	desc = kmalloc(sulen + 2 + srlen * 2 + 1, GFP_KERNEL);
	if (!desc)
		goto error_free_cert;
	p = memcpy(desc, cert->subject, sulen);
	p += sulen;
	*p++ = ':';
	*p++ = ' ';
	p = bin2hex(p, q, srlen);
	*p = 0;

	kids = kmalloc(sizeof(struct asymmetric_key_ids), GFP_KERNEL);
	if (!kids)
		goto error_free_desc;
	kids->id[0] = cert->id;
	kids->id[1] = cert->skid;

	/* We're pinning the module by being linked against it */
	__module_get(public_key_subtype.owner);
	prep->payload.data[asym_subtype] = &public_key_subtype;
	prep->payload.data[asym_key_ids] = kids;
	prep->payload.data[asym_crypto] = cert->pub;
	prep->payload.data[asym_auth] = cert->sig;
	prep->description = desc;
	prep->quotalen = 100;

	/* We've finished with the certificate */
	cert->pub = NULL;
	cert->id = NULL;
	cert->skid = NULL;
	cert->sig = NULL;
	desc = NULL;
	ret = 0;

error_free_desc:
	kfree(desc);
error_free_cert:
	x509_free_certificate(cert);
	return ret;
}

static struct asymmetric_key_parser x509_key_parser = {
	.owner	= THIS_MODULE,
	.name	= "x509",
	.parse	= x509_key_preparse,
};

/*
 * Module stuff
 */
static int __init x509_key_init(void)
{
	return register_asymmetric_key_parser(&x509_key_parser);
}

static void __exit x509_key_exit(void)
{
	unregister_asymmetric_key_parser(&x509_key_parser);
}

module_init(x509_key_init);
module_exit(x509_key_exit);
#endif /* !__UBOOT__ */

MODULE_DESCRIPTION("X.509 certificate parser");
MODULE_AUTHOR("Red Hat, Inc.");
MODULE_LICENSE("GPL");
