// SPDX-License-Identifier: GPL-2.0+
/*
 * X509 cert parser using MbedTLS X509 library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <linux/err.h>
#include <crypto/public_key.h>
#include <crypto/x509_parser.h>

static void x509_free_mbedtls_ctx(struct x509_cert_mbedtls_ctx *ctx)
{
	if (!ctx)
		return;

	kfree(ctx->tbs);
	kfree(ctx->raw_serial);
	kfree(ctx->raw_issuer);
	kfree(ctx->raw_subject);
	kfree(ctx->raw_skid);
	kfree(ctx);
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
	int wb;
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
	 * Note:
	 * ECRDSA (Elliptic Curve Russian Digital Signature Algorithm) is not
	 * supported by MbedTLS.
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
		x509_free_mbedtls_ctx(cert->mbedtls_ctx);
		kfree(cert);
	}
}

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
