// SPDX-License-Identifier: GPL-2.0+
/*
 * MSCode parser using MbedTLS ASN1 library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <crypto/pkcs7.h>
#include <crypto/mscode.h>

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
 * @ctx: PE file context.
 * @content_data: content data pointer.
 * @data_len: content data length.
 * @asn1hdrlen: ASN1 header length.
 */
int mscode_parse(void *ctx, const void *content_data, size_t data_len,
		 size_t asn1hdrlen)
{
	struct pefile_context *_ctx = ctx;
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
	ret = mbedtls_asn1_get_tag(&inner_p, inner_p + seq_len, &len,
				   MBEDTLS_ASN1_OID);
	if (ret)
		return ret;

	/* Sanity check on the PE Image Data OID (1.3.6.1.4.1.311.2.1.15) */
	if (MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_MICROSOFT_PEIMAGEDATA, inner_p,
				len))
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
	ret = mbedtls_asn1_get_tag(&inner_p, inner_p + seq_len, &len,
				   MBEDTLS_ASN1_OID);
	if (ret)
		return ret;

	if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_MD5, inner_p, len))
		_ctx->digest_algo = "md5";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA1, inner_p,
				      len))
		_ctx->digest_algo = "sha1";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA224, inner_p,
				      len))
		_ctx->digest_algo = "sha224";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA256, inner_p,
				      len))
		_ctx->digest_algo = "sha256";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA384, inner_p,
				      len))
		_ctx->digest_algo = "sha384";
	else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_DIGEST_ALG_SHA512, inner_p,
				      len))
		_ctx->digest_algo = "sha512";

	if (!_ctx->digest_algo)
		return -EINVAL;

	p += seq_len;
	ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if (ret)
		return ret;

	_ctx->digest = p;
	_ctx->digest_len = len;

	return 0;
}
