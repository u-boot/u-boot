// SPDX-License-Identifier: GPL-2.0-or-later
/* X.509 certificate parser
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#define pr_fmt(fmt) "X.509: "fmt
#include <log.h>
#include <dm/devres.h>
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/export.h>
#include <linux/slab.h>
#endif
#include <linux/err.h>
#include <linux/oid_registry.h>
#ifdef __UBOOT__
#include <linux/printk.h>
#include <linux/string.h>
#endif
#include <crypto/public_key.h>
#ifdef __UBOOT__
#include <crypto/x509_parser.h>
#else
#include "x509_parser.h"
#endif
#if !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include "x509.asn1.h"
#include "x509_akid.asn1.h"
#endif

struct x509_parse_context {
	struct x509_certificate	*cert;		/* Certificate being constructed */
	unsigned long	data;			/* Start of data */
	const void	*cert_start;		/* Start of cert content */
	const void	*key;			/* Key data */
	size_t		key_size;		/* Size of key data */
	const void	*params;		/* Key parameters */
	size_t		params_size;		/* Size of key parameters */
	enum OID	key_algo;		/* Public key algorithm */
	enum OID	last_oid;		/* Last OID encountered */
	enum OID	algo_oid;		/* Algorithm OID */
	unsigned char	nr_mpi;			/* Number of MPIs stored */
	u8		o_size;			/* Size of organizationName (O) */
	u8		cn_size;		/* Size of commonName (CN) */
	u8		email_size;		/* Size of emailAddress */
	u16		o_offset;		/* Offset of organizationName (O) */
	u16		cn_offset;		/* Offset of commonName (CN) */
	u16		email_offset;		/* Offset of emailAddress */
	unsigned	raw_akid_size;
	const void	*raw_akid;		/* Raw authorityKeyId in ASN.1 */
	const void	*akid_raw_issuer;	/* Raw directoryName in authorityKeyId */
	unsigned	akid_raw_issuer_size;
};

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)

static void x509_free_mbedtls_ctx(struct x509_cert_mbedtls_ctx *ctx)
{
	if (ctx) {
		kfree(ctx->tbs);
		kfree(ctx->raw_serial);
		kfree(ctx->raw_issuer);
		kfree(ctx->raw_subject);
		kfree(ctx->raw_skid);
		kfree(ctx);
	}
}

static int x509_set_cert_flags(struct x509_certificate *cert)
{
	struct public_key_signature *sig = cert->sig;

	if (!sig || !cert->pub) {
		pr_err("Signature or public key is not initialized\n");
		return -ENOPKG;
	}

	if (!cert->pub->pkey_algo)
		cert->unsupported_key = true;

	if (!sig->pkey_algo)
		cert->unsupported_sig = true;

	if (!sig->hash_algo)
		cert->unsupported_sig = true;

	/* TODO: is_hash_blacklisted()? */

	/* Detect self-signed certificates and set self_signed flag */
	return x509_check_for_self_signed(cert);
}

time64_t x509_get_timestamp(const mbedtls_x509_time *x509_time)
{
	unsigned int year, mon, day, hour, min, sec;

	/* Adjust for year since 1900 */
	year = x509_time->year - 1900;
	/* Adjust for 0-based month */
	mon = x509_time->mon - 1;
	day = x509_time->day;
	hour = x509_time->hour;
	min = x509_time->min;
	sec = x509_time->sec;

	return (time64_t)mktime64(year, mon, day, hour, min, sec);
}

static char *x509_populate_dn_name_string(const mbedtls_x509_name *name)
{
	size_t len = 256;
	size_t wb;
	char *name_str;

	do {
		name_str = kzalloc(len, GFP_KERNEL);
		if (!name_str)
			return NULL;

		wb = mbedtls_x509_dn_gets(name_str, len, name);
		if (wb < 0) {
			pr_err("Get DN string failed, ret:-0x%04x\n",
			       (unsigned int)-wb);
			kfree(name_str);
			len = len * 2; /* Try with a bigger buffer */
		}
	} while (wb < 0);

	name_str[wb] = '\0'; /* add the terminator */

	return name_str;
}

static int x509_populate_signature_params(const mbedtls_x509_crt *cert,
					  struct public_key_signature **sig)
{
	struct public_key_signature *s;
	struct image_region region;
	size_t akid_len;
	unsigned char *akid_data;
	int ret;

	/* Check if signed data exist */
	if (!cert->tbs.p || !cert->tbs.len)
		return -EINVAL;

	region.data = cert->tbs.p;
	region.size = cert->tbs.len;

	s = kzalloc(sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	/*
	 * Get the public key algorithm.
	 * Note: ECRDSA (Elliptic Curve RedDSA) from Red Hat is not supported by
	 *	 MbedTLS.
	 */
	switch (cert->sig_pk) {
	case MBEDTLS_PK_RSA:
		s->pkey_algo = "rsa";
		break;
	default:
		ret = -EINVAL;
		goto error_sig;
	}

	/* Get the hash algorithm */
	switch (cert->sig_md) {
	case MBEDTLS_MD_SHA1:
		s->hash_algo = "sha1";
		s->digest_size = SHA1_SUM_LEN;
		break;
	case MBEDTLS_MD_SHA256:
		s->hash_algo = "sha256";
		s->digest_size = SHA256_SUM_LEN;
		break;
	case MBEDTLS_MD_SHA384:
		s->hash_algo = "sha384";
		s->digest_size = SHA384_SUM_LEN;
		break;
	case MBEDTLS_MD_SHA512:
		s->hash_algo = "sha512";
		s->digest_size = SHA512_SUM_LEN;
		break;
	/* Unsupported algo */
	case MBEDTLS_MD_MD5:
	case MBEDTLS_MD_SHA224:
	default:
		ret = -EINVAL;
		goto error_sig;
	}

	/*
	 * Optional attributes:
	 * auth_ids holds AuthorityKeyIdentifier (information of issuer),
	 * aka akid, which is used to match with a cert's id or skid to
	 * indicate that is the issuer when we lookup a cert chain.
	 *
	 * auth_ids[0]:
	 *	[PKCS#7 or CMS ver 1] - generated from "Issuer + Serial number"
	 *	[CMS ver 3] - generated from skid (subjectKeyId)
	 * auth_ids[1]: generated from skid (subjectKeyId)
	 *
	 * Assume that we are using PKCS#7 (msg->version=1),
	 * not CMS ver 3 (msg->version=3).
	 */
	akid_len = cert->authority_key_id.authorityCertSerialNumber.len;
	akid_data = cert->authority_key_id.authorityCertSerialNumber.p;

	/* Check if serial number exists */
	if (akid_len && akid_data) {
		s->auth_ids[0] = asymmetric_key_generate_id(akid_data,
							    akid_len,
							    cert->issuer_raw.p,
							    cert->issuer_raw.len);
		if (!s->auth_ids[0]) {
			ret = -ENOMEM;
			goto error_sig;
		}
	}

	akid_len = cert->authority_key_id.keyIdentifier.len;
	akid_data = cert->authority_key_id.keyIdentifier.p;

	/* Check if subjectKeyId exists */
	if (akid_len && akid_data) {
		s->auth_ids[1] = asymmetric_key_generate_id(akid_data,
							    akid_len,
							    "", 0);
		if (!s->auth_ids[1]) {
			ret = -ENOMEM;
			goto error_sig;
		}
	}

	/*
	 * Encoding can be pkcs1 or raw, but only pkcs1 is supported.
	 * Set the encoding explicitly to pkcs1.
	 */
	s->encoding = "pkcs1";

	/* Copy the signature data */
	s->s = kmemdup(cert->sig.p, cert->sig.len, GFP_KERNEL);
	if (!s->s) {
		ret = -ENOMEM;
		goto error_sig;
	}
	s->s_size = cert->sig.len;

	/* Calculate the digest of signed data (tbs) */
	s->digest = kzalloc(s->digest_size, GFP_KERNEL);
	if (!s->digest) {
		ret = -ENOMEM;
		goto error_sig;
	}

	ret = hash_calculate(s->hash_algo, &region, 1, s->digest);
	if (!ret)
		*sig = s;

	return ret;

error_sig:
	public_key_signature_free(s);
	return ret;
}

static int x509_save_mbedtls_ctx(const mbedtls_x509_crt *cert,
				 struct x509_cert_mbedtls_ctx **pctx)
{
	struct x509_cert_mbedtls_ctx *ctx;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	/* Signed data (tbs - The part that is To Be Signed)*/
	ctx->tbs = kmemdup(cert->tbs.p, cert->tbs.len,
			   GFP_KERNEL);
	if (!ctx->tbs)
		goto error_ctx;

	/* Raw serial number */
	ctx->raw_serial = kmemdup(cert->serial.p,
				  cert->serial.len, GFP_KERNEL);
	if (!ctx->raw_serial)
		goto error_ctx;

	/* Raw issuer */
	ctx->raw_issuer = kmemdup(cert->issuer_raw.p,
				  cert->issuer_raw.len, GFP_KERNEL);
	if (!ctx->raw_issuer)
		goto error_ctx;

	/* Raw subject */
	ctx->raw_subject = kmemdup(cert->subject_raw.p,
				   cert->subject_raw.len, GFP_KERNEL);
	if (!ctx->raw_subject)
		goto error_ctx;

	/* Raw subjectKeyId */
	ctx->raw_skid = kmemdup(cert->subject_key_id.p,
				cert->subject_key_id.len, GFP_KERNEL);
	if (!ctx->raw_skid)
		goto error_ctx;

	*pctx = ctx;

	return 0;

error_ctx:
	x509_free_mbedtls_ctx(ctx);
	return -ENOMEM;
}

#endif /* CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

/*
 * Free an X.509 certificate
 */
void x509_free_certificate(struct x509_certificate *cert)
{
	if (cert) {
		public_key_free(cert->pub);
		public_key_signature_free(cert->sig);
		kfree(cert->issuer);
		kfree(cert->subject);
		kfree(cert->id);
		kfree(cert->skid);
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
		x509_free_mbedtls_ctx(cert->mbedtls_ctx);
#endif
		kfree(cert);
	}
}
EXPORT_SYMBOL_GPL(x509_free_certificate);

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
int x509_populate_pubkey(mbedtls_x509_crt *cert, struct public_key **pub_key)
{
	struct public_key *pk;

	pk = kzalloc(sizeof(*pk), GFP_KERNEL);
	if (!pk)
		return -ENOMEM;

	pk->key = kzalloc(cert->pk_raw.len, GFP_KERNEL);
	if (!pk->key) {
		kfree(pk);
		return -ENOMEM;
	}
	memcpy(pk->key, cert->pk_raw.p, cert->pk_raw.len);
	pk->keylen = cert->pk_raw.len;

	/*
	 * For ECC keys, params field might include information about the curve used,
	 * the generator point, or other algorithm-specific parameters.
	 * For RSA keys, it's common for the params field to be NULL.
	 * FIXME: Assume that we just support RSA keys with id_type X509.
	 */
	pk->params = NULL;
	pk->paramlen = 0;

	pk->key_is_private = false;
	pk->id_type = "X509";
	pk->pkey_algo = "rsa";
	pk->algo = OID_rsaEncryption;

	*pub_key = pk;

	return 0;
}
EXPORT_SYMBOL_GPL(x509_populate_pubkey);

int x509_populate_cert(mbedtls_x509_crt *mbedtls_cert,
		       struct x509_certificate **pcert)
{
	struct x509_certificate *cert;
	struct asymmetric_key_id *kid;
	struct asymmetric_key_id *skid;
	int ret;

	cert = kzalloc(sizeof(*cert), GFP_KERNEL);
	if (!cert)
		return -ENOMEM;

	/* Public key details */
	ret = x509_populate_pubkey(mbedtls_cert, &cert->pub);
	if (ret)
		goto error_cert_pop;

	/* Signature parameters */
	ret = x509_populate_signature_params(mbedtls_cert, &cert->sig);
	if (ret)
		goto error_cert_pop;

	ret = -ENOMEM;

	/* Name of certificate issuer */
	cert->issuer = x509_populate_dn_name_string(&mbedtls_cert->issuer);
	if (!cert->issuer)
		goto error_cert_pop;

	/* Name of certificate subject */
	cert->subject = x509_populate_dn_name_string(&mbedtls_cert->subject);
	if (!cert->subject)
		goto error_cert_pop;

	/* Certificate validity */
	cert->valid_from = x509_get_timestamp(&mbedtls_cert->valid_from);
	cert->valid_to = x509_get_timestamp(&mbedtls_cert->valid_to);

	/* Save mbedtls context we need */
	ret = x509_save_mbedtls_ctx(mbedtls_cert, &cert->mbedtls_ctx);
	if (ret)
		goto error_cert_pop;

	/* Signed data (tbs - The part that is To Be Signed)*/
	cert->tbs = cert->mbedtls_ctx->tbs;
	cert->tbs_size = mbedtls_cert->tbs.len;

	/* Raw serial number */
	cert->raw_serial = cert->mbedtls_ctx->raw_serial;
	cert->raw_serial_size = mbedtls_cert->serial.len;

	/* Raw issuer */
	cert->raw_issuer = cert->mbedtls_ctx->raw_issuer;
	cert->raw_issuer_size = mbedtls_cert->issuer_raw.len;

	/* Raw subject */
	cert->raw_subject = cert->mbedtls_ctx->raw_subject;
	cert->raw_subject_size = mbedtls_cert->subject_raw.len;

	/* Raw subjectKeyId */
	cert->raw_skid = cert->mbedtls_ctx->raw_skid;
	cert->raw_skid_size = mbedtls_cert->subject_key_id.len;

	/* Generate cert issuer + serial number key ID */
	kid = asymmetric_key_generate_id(cert->raw_serial,
					 cert->raw_serial_size,
					 cert->raw_issuer,
					 cert->raw_issuer_size);
	if (IS_ERR(kid)) {
		ret = PTR_ERR(kid);
		goto error_cert_pop;
	}
	cert->id = kid;

	/* Generate subject + subjectKeyId */
	skid = asymmetric_key_generate_id(cert->raw_skid, cert->raw_skid_size, "", 0);
	if (IS_ERR(skid)) {
		ret = PTR_ERR(skid);
		goto error_cert_pop;
	}
	cert->skid = skid;

	/*
	 * Set the certificate flags:
	 * self_signed, unsupported_key, unsupported_sig, blacklisted
	 */
	ret = x509_set_cert_flags(cert);
	if (!ret) {
		*pcert = cert;
		return 0;
	}

error_cert_pop:
	x509_free_certificate(cert);
	return ret;
}
EXPORT_SYMBOL_GPL(x509_populate_cert);

struct x509_certificate *x509_cert_parse(const void *data, size_t datalen)
{
	mbedtls_x509_crt mbedtls_cert;
	struct x509_certificate *cert = NULL;
	long ret;

	/* Parse DER encoded certificate */
	mbedtls_x509_crt_init(&mbedtls_cert);
	ret = mbedtls_x509_crt_parse_der(&mbedtls_cert, data, datalen);
	if (ret)
		goto clean_up_ctx;

	/* Populate x509_certificate from mbedtls_x509_crt */
	ret = x509_populate_cert(&mbedtls_cert, &cert);
	if (ret)
		goto clean_up_ctx;

clean_up_ctx:
	mbedtls_x509_crt_free(&mbedtls_cert);
	if (!ret)
		return cert;

	return ERR_PTR(ret);
}
#else	/* !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */
/*
 * Parse an X.509 certificate
 */
struct x509_certificate *x509_cert_parse(const void *data, size_t datalen)
{
	struct x509_certificate *cert;
	struct x509_parse_context *ctx;
	struct asymmetric_key_id *kid;
	long ret;

	ret = -ENOMEM;
	cert = kzalloc(sizeof(struct x509_certificate), GFP_KERNEL);
	if (!cert)
		goto error_no_cert;
	cert->pub = kzalloc(sizeof(struct public_key), GFP_KERNEL);
	if (!cert->pub)
		goto error_no_ctx;
	cert->sig = kzalloc(sizeof(struct public_key_signature), GFP_KERNEL);
	if (!cert->sig)
		goto error_no_ctx;
	ctx = kzalloc(sizeof(struct x509_parse_context), GFP_KERNEL);
	if (!ctx)
		goto error_no_ctx;

	ctx->cert = cert;
	ctx->data = (unsigned long)data;

	/* Attempt to decode the certificate */
	ret = asn1_ber_decoder(&x509_decoder, ctx, data, datalen);
	if (ret < 0)
		goto error_decode;

	/* Decode the AuthorityKeyIdentifier */
	if (ctx->raw_akid) {
		pr_devel("AKID: %u %*phN\n",
			 ctx->raw_akid_size, ctx->raw_akid_size, ctx->raw_akid);
		ret = asn1_ber_decoder(&x509_akid_decoder, ctx,
				       ctx->raw_akid, ctx->raw_akid_size);
		if (ret < 0) {
			pr_warn("Couldn't decode AuthKeyIdentifier\n");
			goto error_decode;
		}
	}

	ret = -ENOMEM;
	cert->pub->key = kmemdup(ctx->key, ctx->key_size, GFP_KERNEL);
	if (!cert->pub->key)
		goto error_decode;

	cert->pub->keylen = ctx->key_size;

	cert->pub->params = kmemdup(ctx->params, ctx->params_size, GFP_KERNEL);
	if (!cert->pub->params)
		goto error_decode;

	cert->pub->paramlen = ctx->params_size;
	cert->pub->algo = ctx->key_algo;

	/* Grab the signature bits */
	ret = x509_get_sig_params(cert);
	if (ret < 0)
		goto error_decode;

	/* Generate cert issuer + serial number key ID */
	kid = asymmetric_key_generate_id(cert->raw_serial,
					 cert->raw_serial_size,
					 cert->raw_issuer,
					 cert->raw_issuer_size);
	if (IS_ERR(kid)) {
		ret = PTR_ERR(kid);
		goto error_decode;
	}
	cert->id = kid;

	/* Detect self-signed certificates */
	ret = x509_check_for_self_signed(cert);
	if (ret < 0)
		goto error_decode;

	kfree(ctx);
	return cert;

error_decode:
	kfree(ctx);
error_no_ctx:
	x509_free_certificate(cert);
error_no_cert:
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(x509_cert_parse);

/*
 * Note an OID when we find one for later processing when we know how
 * to interpret it.
 */
int x509_note_OID(void *context, size_t hdrlen,
	     unsigned char tag,
	     const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	ctx->last_oid = look_up_OID(value, vlen);
	if (ctx->last_oid == OID__NR) {
		char buffer[50];
		sprint_oid(value, vlen, buffer, sizeof(buffer));
		pr_debug("Unknown OID: [%lu] %s\n",
			 (unsigned long)value - ctx->data, buffer);
	}
	return 0;
}

/*
 * Save the position of the TBS data so that we can check the signature over it
 * later.
 */
int x509_note_tbs_certificate(void *context, size_t hdrlen,
			      unsigned char tag,
			      const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	pr_debug("x509_note_tbs_certificate(,%zu,%02x,%ld,%zu)!\n",
		 hdrlen, tag, (unsigned long)value - ctx->data, vlen);

	ctx->cert->tbs = value - hdrlen;
	ctx->cert->tbs_size = vlen + hdrlen;
	return 0;
}

/*
 * Record the public key algorithm
 */
int x509_note_pkey_algo(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	pr_debug("PubKey Algo: %u\n", ctx->last_oid);

	switch (ctx->last_oid) {
	case OID_md2WithRSAEncryption:
	case OID_md3WithRSAEncryption:
	default:
		return -ENOPKG; /* Unsupported combination */

	case OID_md4WithRSAEncryption:
		ctx->cert->sig->hash_algo = "md4";
		goto rsa_pkcs1;

	case OID_sha1WithRSAEncryption:
		ctx->cert->sig->hash_algo = "sha1";
		goto rsa_pkcs1;

	case OID_sha256WithRSAEncryption:
		ctx->cert->sig->hash_algo = "sha256";
		goto rsa_pkcs1;

	case OID_sha384WithRSAEncryption:
		ctx->cert->sig->hash_algo = "sha384";
		goto rsa_pkcs1;

	case OID_sha512WithRSAEncryption:
		ctx->cert->sig->hash_algo = "sha512";
		goto rsa_pkcs1;

	case OID_sha224WithRSAEncryption:
		ctx->cert->sig->hash_algo = "sha224";
		goto rsa_pkcs1;

	case OID_gost2012Signature256:
		ctx->cert->sig->hash_algo = "streebog256";
		goto ecrdsa;

	case OID_gost2012Signature512:
		ctx->cert->sig->hash_algo = "streebog512";
		goto ecrdsa;
	}

rsa_pkcs1:
	ctx->cert->sig->pkey_algo = "rsa";
	ctx->cert->sig->encoding = "pkcs1";
	ctx->algo_oid = ctx->last_oid;
	return 0;
ecrdsa:
	ctx->cert->sig->pkey_algo = "ecrdsa";
	ctx->cert->sig->encoding = "raw";
	ctx->algo_oid = ctx->last_oid;
	return 0;
}

/*
 * Note the whereabouts and type of the signature.
 */
int x509_note_signature(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	pr_debug("Signature type: %u size %zu\n", ctx->last_oid, vlen);

	if (ctx->last_oid != ctx->algo_oid) {
		pr_warn("Got cert with pkey (%u) and sig (%u) algorithm OIDs\n",
			ctx->algo_oid, ctx->last_oid);
		return -EINVAL;
	}

	if (strcmp(ctx->cert->sig->pkey_algo, "rsa") == 0 ||
	    strcmp(ctx->cert->sig->pkey_algo, "ecrdsa") == 0) {
		/* Discard the BIT STRING metadata */
		if (vlen < 1 || *(const u8 *)value != 0)
			return -EBADMSG;

		value++;
		vlen--;
	}

	ctx->cert->raw_sig = value;
	ctx->cert->raw_sig_size = vlen;
	return 0;
}

/*
 * Note the certificate serial number
 */
int x509_note_serial(void *context, size_t hdrlen,
		     unsigned char tag,
		     const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	ctx->cert->raw_serial = value;
	ctx->cert->raw_serial_size = vlen;
	return 0;
}

/*
 * Note some of the name segments from which we'll fabricate a name.
 */
int x509_extract_name_segment(void *context, size_t hdrlen,
			      unsigned char tag,
			      const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	switch (ctx->last_oid) {
	case OID_commonName:
		ctx->cn_size = vlen;
		ctx->cn_offset = (unsigned long)value - ctx->data;
		break;
	case OID_organizationName:
		ctx->o_size = vlen;
		ctx->o_offset = (unsigned long)value - ctx->data;
		break;
	case OID_email_address:
		ctx->email_size = vlen;
		ctx->email_offset = (unsigned long)value - ctx->data;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * Fabricate and save the issuer and subject names
 */
static int x509_fabricate_name(struct x509_parse_context *ctx, size_t hdrlen,
			       unsigned char tag,
			       char **_name, size_t vlen)
{
	const void *name, *data = (const void *)ctx->data;
	size_t namesize;
	char *buffer;

	if (*_name)
		return -EINVAL;

	/* Empty name string if no material */
	if (!ctx->cn_size && !ctx->o_size && !ctx->email_size) {
		buffer = kmalloc(1, GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;
		buffer[0] = 0;
		goto done;
	}

	if (ctx->cn_size && ctx->o_size) {
		/* Consider combining O and CN, but use only the CN if it is
		 * prefixed by the O, or a significant portion thereof.
		 */
		namesize = ctx->cn_size;
		name = data + ctx->cn_offset;
		if (ctx->cn_size >= ctx->o_size &&
		    memcmp(data + ctx->cn_offset, data + ctx->o_offset,
			   ctx->o_size) == 0)
			goto single_component;
		if (ctx->cn_size >= 7 &&
		    ctx->o_size >= 7 &&
		    memcmp(data + ctx->cn_offset, data + ctx->o_offset, 7) == 0)
			goto single_component;

		buffer = kmalloc(ctx->o_size + 2 + ctx->cn_size + 1,
				 GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;

		memcpy(buffer,
		       data + ctx->o_offset, ctx->o_size);
		buffer[ctx->o_size + 0] = ':';
		buffer[ctx->o_size + 1] = ' ';
		memcpy(buffer + ctx->o_size + 2,
		       data + ctx->cn_offset, ctx->cn_size);
		buffer[ctx->o_size + 2 + ctx->cn_size] = 0;
		goto done;

	} else if (ctx->cn_size) {
		namesize = ctx->cn_size;
		name = data + ctx->cn_offset;
	} else if (ctx->o_size) {
		namesize = ctx->o_size;
		name = data + ctx->o_offset;
	} else {
		namesize = ctx->email_size;
		name = data + ctx->email_offset;
	}

single_component:
	buffer = kmalloc(namesize + 1, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	memcpy(buffer, name, namesize);
	buffer[namesize] = 0;

done:
	*_name = buffer;
	ctx->cn_size = 0;
	ctx->o_size = 0;
	ctx->email_size = 0;
	return 0;
}

int x509_note_issuer(void *context, size_t hdrlen,
		     unsigned char tag,
		     const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	ctx->cert->raw_issuer = value;
	ctx->cert->raw_issuer_size = vlen;
	return x509_fabricate_name(ctx, hdrlen, tag, &ctx->cert->issuer, vlen);
}

int x509_note_subject(void *context, size_t hdrlen,
		      unsigned char tag,
		      const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	ctx->cert->raw_subject = value;
	ctx->cert->raw_subject_size = vlen;
	return x509_fabricate_name(ctx, hdrlen, tag, &ctx->cert->subject, vlen);
}

/*
 * Extract the parameters for the public key
 */
int x509_note_params(void *context, size_t hdrlen,
		     unsigned char tag,
		     const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	/*
	 * AlgorithmIdentifier is used three times in the x509, we should skip
	 * first and ignore third, using second one which is after subject and
	 * before subjectPublicKey.
	 */
	if (!ctx->cert->raw_subject || ctx->key)
		return 0;
	ctx->params = value - hdrlen;
	ctx->params_size = vlen + hdrlen;
	return 0;
}

/*
 * Extract the data for the public key algorithm
 */
int x509_extract_key_data(void *context, size_t hdrlen,
			  unsigned char tag,
			  const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	ctx->key_algo = ctx->last_oid;
	if (ctx->last_oid == OID_rsaEncryption)
		ctx->cert->pub->pkey_algo = "rsa";
	else if (ctx->last_oid == OID_gost2012PKey256 ||
		 ctx->last_oid == OID_gost2012PKey512)
		ctx->cert->pub->pkey_algo = "ecrdsa";
	else
		return -ENOPKG;

	/* Discard the BIT STRING metadata */
	if (vlen < 1 || *(const u8 *)value != 0)
		return -EBADMSG;
	ctx->key = value + 1;
	ctx->key_size = vlen - 1;
	return 0;
}

/* The keyIdentifier in AuthorityKeyIdentifier SEQUENCE is tag(CONT,PRIM,0) */
#define SEQ_TAG_KEYID (ASN1_CONT << 6)

/*
 * Process certificate extensions that are used to qualify the certificate.
 */
int x509_process_extension(void *context, size_t hdrlen,
			   unsigned char tag,
			   const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	struct asymmetric_key_id *kid;
	const unsigned char *v = value;

	pr_debug("Extension: %u\n", ctx->last_oid);

	if (ctx->last_oid == OID_subjectKeyIdentifier) {
		/* Get hold of the key fingerprint */
		if (ctx->cert->skid || vlen < 3)
			return -EBADMSG;
		if (v[0] != ASN1_OTS || v[1] != vlen - 2)
			return -EBADMSG;
		v += 2;
		vlen -= 2;

		ctx->cert->raw_skid_size = vlen;
		ctx->cert->raw_skid = v;
		kid = asymmetric_key_generate_id(v, vlen, "", 0);
		if (IS_ERR(kid))
			return PTR_ERR(kid);
		ctx->cert->skid = kid;
		pr_debug("subjkeyid %*phN\n", kid->len, kid->data);
		return 0;
	}

	if (ctx->last_oid == OID_authorityKeyIdentifier) {
		/* Get hold of the CA key fingerprint */
		ctx->raw_akid = v;
		ctx->raw_akid_size = vlen;
		return 0;
	}

	return 0;
}

/**
 * x509_decode_time - Decode an X.509 time ASN.1 object
 * @_t: The time to fill in
 * @hdrlen: The length of the object header
 * @tag: The object tag
 * @value: The object value
 * @vlen: The size of the object value
 *
 * Decode an ASN.1 universal time or generalised time field into a struct the
 * kernel can handle and check it for validity.  The time is decoded thus:
 *
 *	[RFC5280 paragraph 74.1.2.5]
 *	CAs conforming to this profile MUST always encode certificate validity
 *	dates through the year 2049 as UTCTime; certificate validity dates in
 *	2050 or later MUST be encoded as GeneralizedTime.  Conforming
 *	applications MUST be able to process validity dates that are encoded in
 *	either UTCTime or GeneralizedTime.
 */
int x509_decode_time(time64_t *_t,  size_t hdrlen,
		     unsigned char tag,
		     const unsigned char *value, size_t vlen)
{
	static const unsigned char month_lengths[] = { 31, 28, 31, 30, 31, 30,
						       31, 31, 30, 31, 30, 31 };
	const unsigned char *p = value;
	unsigned year, mon, day, hour, min, sec, mon_len;

#define dec2bin(X) ({ unsigned char x = (X) - '0'; if (x > 9) goto invalid_time; x; })
#define DD2bin(P) ({ unsigned x = dec2bin(P[0]) * 10 + dec2bin(P[1]); P += 2; x; })

	if (tag == ASN1_UNITIM) {
		/* UTCTime: YYMMDDHHMMSSZ */
		if (vlen != 13)
			goto unsupported_time;
		year = DD2bin(p);
		if (year >= 50)
			year += 1900;
		else
			year += 2000;
	} else if (tag == ASN1_GENTIM) {
		/* GenTime: YYYYMMDDHHMMSSZ */
		if (vlen != 15)
			goto unsupported_time;
		year = DD2bin(p) * 100 + DD2bin(p);
		if (year >= 1950 && year <= 2049)
			goto invalid_time;
	} else {
		goto unsupported_time;
	}

	mon  = DD2bin(p);
	day = DD2bin(p);
	hour = DD2bin(p);
	min  = DD2bin(p);
	sec  = DD2bin(p);

	if (*p != 'Z')
		goto unsupported_time;

	if (year < 1970 ||
	    mon < 1 || mon > 12)
		goto invalid_time;

	mon_len = month_lengths[mon - 1];
	if (mon == 2) {
		if (year % 4 == 0) {
			mon_len = 29;
			if (year % 100 == 0) {
				mon_len = 28;
				if (year % 400 == 0)
					mon_len = 29;
			}
		}
	}

	if (day < 1 || day > mon_len ||
	    hour > 24 || /* ISO 8601 permits 24:00:00 as midnight tomorrow */
	    min > 59 ||
	    sec > 60) /* ISO 8601 permits leap seconds [X.680 46.3] */
		goto invalid_time;

	*_t = mktime64(year, mon, day, hour, min, sec);
	return 0;

unsupported_time:
	pr_debug("Got unsupported time [tag %02x]: '%*phN'\n",
		 tag, (int)vlen, value);
	return -EBADMSG;
invalid_time:
	pr_debug("Got invalid time [tag %02x]: '%*phN'\n",
		 tag, (int)vlen, value);
	return -EBADMSG;
}
EXPORT_SYMBOL_GPL(x509_decode_time);

int x509_note_not_before(void *context, size_t hdrlen,
			 unsigned char tag,
			 const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	return x509_decode_time(&ctx->cert->valid_from, hdrlen, tag, value, vlen);
}

int x509_note_not_after(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	return x509_decode_time(&ctx->cert->valid_to, hdrlen, tag, value, vlen);
}

/*
 * Note a key identifier-based AuthorityKeyIdentifier
 */
int x509_akid_note_kid(void *context, size_t hdrlen,
		       unsigned char tag,
		       const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	struct asymmetric_key_id *kid;

	pr_debug("AKID: keyid: %*phN\n", (int)vlen, value);

	if (ctx->cert->sig->auth_ids[1])
		return 0;

	kid = asymmetric_key_generate_id(value, vlen, "", 0);
	if (IS_ERR(kid))
		return PTR_ERR(kid);
	pr_debug("authkeyid %*phN\n", kid->len, kid->data);
	ctx->cert->sig->auth_ids[1] = kid;
	return 0;
}

/*
 * Note a directoryName in an AuthorityKeyIdentifier
 */
int x509_akid_note_name(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	pr_debug("AKID: name: %*phN\n", (int)vlen, value);

	ctx->akid_raw_issuer = value;
	ctx->akid_raw_issuer_size = vlen;
	return 0;
}

/*
 * Note a serial number in an AuthorityKeyIdentifier
 */
int x509_akid_note_serial(void *context, size_t hdrlen,
			  unsigned char tag,
			  const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	struct asymmetric_key_id *kid;

	pr_debug("AKID: serial: %*phN\n", (int)vlen, value);

	if (!ctx->akid_raw_issuer || ctx->cert->sig->auth_ids[0])
		return 0;

	kid = asymmetric_key_generate_id(value,
					 vlen,
					 ctx->akid_raw_issuer,
					 ctx->akid_raw_issuer_size);
	if (IS_ERR(kid))
		return PTR_ERR(kid);

	pr_debug("authkeyid %*phN\n", kid->len, kid->data);
	ctx->cert->sig->auth_ids[0] = kid;
	return 0;
}
#endif /* CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */
