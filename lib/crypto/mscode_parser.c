// SPDX-License-Identifier: GPL-2.0-or-later
/* Parse a Microsoft Individual Code Signing blob
 *
 * Copyright (C) 2014 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#define pr_fmt(fmt) "MSCODE: "fmt
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif
#include <linux/err.h>
#include <linux/oid_registry.h>
#include <crypto/pkcs7.h>
#ifdef __UBOOT__
#include <crypto/mscode.h>
#else
#include "verify_pefile.h"
#endif
#if !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include "mscode.asn1.h"
#endif

/*
 * Parse a Microsoft Individual Code Signing blob
 *
 * U.P.SEQUENCE {
 *    U.P.OBJECTIDENTIFIER 1.3.6.1.4.1.311.2.1.15 (SPC_PE_IMAGE_DATA_OBJID)
 *    U.P.SEQUENCE {
 *       U.P.BITSTRING NaN : 0 unused bit(s);
 *       [C.P.0] {
 *          [C.P.2] {
 *             [C.P.0] <arbitrary string>
 *          }
 *       }
 *    }
 * }
 * U.P.SEQUENCE {
 *    U.P.SEQUENCE {
 *       U.P.OBJECTIDENTIFIER <digest algorithm OID>
 *       U.P.NULL
 *    }
 *    U.P.OCTETSTRING <PE image digest>
 * }
 *
 */
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)

int mscode_parse(void *_ctx, const void *content_data, size_t data_len,
		 size_t asn1hdrlen)
{
	struct pefile_context *ctx = _ctx;
	unsigned char *p = (unsigned char *)content_data;
	unsigned char *end = (unsigned char *)content_data + data_len;
	size_t len = 0;
	int ret;
	unsigned char *inner_p;
	size_t seq_len = 0;

	ret = mbedtls_asn1_get_tag(&p, end, &seq_len,
				   MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret)
		return ret;

	inner_p = p;
	ret = mbedtls_asn1_get_tag(&inner_p, inner_p + seq_len, &len, MBEDTLS_ASN1_OID);
	if (ret)
		return ret;

	/* Sanity check on the PE Image Data OID (1.3.6.1.4.1.311.2.1.15) */
	if (MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_MICROSOFT_PEIMAGEDATA, inner_p, len))
		return -EINVAL;

	p += seq_len;
	ret = mbedtls_asn1_get_tag(&p, end, &seq_len,
				   MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret)
		return ret;

	ret = mbedtls_asn1_get_tag(&p, p + seq_len, &seq_len,
				   MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret)
		return ret;

	inner_p = p;

	/*
	 * Check if the inner sequence contains a supported hash
	 * algorithm OID
	 */
	ret = mbedtls_asn1_get_tag(&inner_p, inner_p + seq_len, &len, MBEDTLS_ASN1_OID);
	if (ret)
		return ret;

	if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_MD5, inner_p, len))
		ctx->digest_algo = "md5";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA1, inner_p, len))
		ctx->digest_algo = "sha1";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA224, inner_p, len))
		ctx->digest_algo = "sha224";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA256, inner_p, len))
		ctx->digest_algo = "sha256";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA384, inner_p, len))
		ctx->digest_algo = "sha384";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA512, inner_p, len))
		ctx->digest_algo = "sha512";

	if (!ctx->digest_algo)
		return -EINVAL;

	p += seq_len;
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if (ret)
		return ret;

	ctx->digest = p;
	ctx->digest_len = len;

	return 0;
}

#else	/* !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

int mscode_parse(void *_ctx, const void *content_data, size_t data_len,
		 size_t asn1hdrlen)
{
	struct pefile_context *ctx = _ctx;

	content_data -= asn1hdrlen;
	data_len += asn1hdrlen;
	pr_devel("Data: %zu [%*ph]\n", data_len, (unsigned)(data_len),
		 content_data);

	return asn1_ber_decoder(&mscode_decoder, ctx, content_data, data_len);
}

#endif	/* CONFIG_IS_ENABLED(MBEDTLS_LIB_X509) */

/*
 * Check the content type OID
 */
int mscode_note_content_type(void *context, size_t hdrlen,
			     unsigned char tag,
			     const void *value, size_t vlen)
{
	enum OID oid;

	oid = look_up_OID(value, vlen);
	if (oid == OID__NR) {
		char buffer[50];

		sprint_oid(value, vlen, buffer, sizeof(buffer));
		pr_err("Unknown OID: %s\n", buffer);
		return -EBADMSG;
	}

	/*
	 * pesign utility had a bug where it was putting
	 * OID_msIndividualSPKeyPurpose instead of OID_msPeImageDataObjId
	 * So allow both OIDs.
	 */
	if (oid != OID_msPeImageDataObjId &&
	    oid != OID_msIndividualSPKeyPurpose) {
		pr_err("Unexpected content type OID %u\n", oid);
		return -EBADMSG;
	}

	return 0;
}

/*
 * Note the digest algorithm OID
 */
int mscode_note_digest_algo(void *context, size_t hdrlen,
			    unsigned char tag,
			    const void *value, size_t vlen)
{
	struct pefile_context *ctx = context;
	char buffer[50];
	enum OID oid;

	oid = look_up_OID(value, vlen);
	switch (oid) {
	case OID_md4:
		ctx->digest_algo = "md4";
		break;
	case OID_md5:
		ctx->digest_algo = "md5";
		break;
	case OID_sha1:
		ctx->digest_algo = "sha1";
		break;
	case OID_sha256:
		ctx->digest_algo = "sha256";
		break;
	case OID_sha384:
		ctx->digest_algo = "sha384";
		break;
	case OID_sha512:
		ctx->digest_algo = "sha512";
		break;
	case OID_sha224:
		ctx->digest_algo = "sha224";
		break;

	case OID__NR:
		sprint_oid(value, vlen, buffer, sizeof(buffer));
		pr_err("Unknown OID: %s\n", buffer);
		return -EBADMSG;

	default:
		pr_err("Unsupported content type: %u\n", oid);
		return -ENOPKG;
	}

	return 0;
}

/*
 * Note the digest we're guaranteeing with this certificate
 */
int mscode_note_digest(void *context, size_t hdrlen,
		       unsigned char tag,
		       const void *value, size_t vlen)
{
	struct pefile_context *ctx = context;

	ctx->digest = kmemdup(value, vlen, GFP_KERNEL);
	if (!ctx->digest)
		return -ENOMEM;

	ctx->digest_len = vlen;

	return 0;
}
