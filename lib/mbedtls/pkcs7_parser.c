// SPDX-License-Identifier: GPL-2.0+
/*
 * PKCS#7 parser using MbedTLS PKCS#7 library
 *
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#include <log.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <crypto/public_key.h>
#include <crypto/pkcs7_parser.h>

static void pkcs7_free_mbedtls_ctx(struct pkcs7_mbedtls_ctx *ctx)
{
	if (ctx) {
		kfree(ctx->content_data);
		kfree(ctx);
	}
}

static void pkcs7_free_sinfo_mbedtls_ctx(struct pkcs7_sinfo_mbedtls_ctx *ctx)
{
	if (ctx) {
		kfree(ctx->authattrs_data);
		kfree(ctx->content_data_digest);
		kfree(ctx);
	}
}

/*
 * Parse Authenticate Attributes
 * TODO: Shall we consider to integrate decoding of authenticate attribute into
 *	 MbedTLS library?
 *
 * There are two kinds of structure for the Authenticate Attributes being used
 * in U-Boot.
 *
 * Type 1 - contains in a PE/COFF EFI image:
 *
 * [C.P.0] {
 *   U.P.SEQUENCE {
 *     U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.3 (OID_contentType)
 *     U.P.SET {
 *        U.P.OBJECTIDENTIFIER 1.3.6.1.4.1.311.2.1.4 (OID_msIndirectData)
 *     }
 *  }
 *  U.P.SEQUENCE {
 *     U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.5 (OID_signingTime)
 *     U.P.SET {
 *        U.P.UTCTime '<siging_time>'
 *     }
 *  }
 *  U.P.SEQUENCE {
 *     U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.4 (OID_messageDigest)
 *     U.P.SET {
 *        U.P.OCTETSTRING <digest>
 *     }
 *  }
 *    U.P.SEQUENCE {
 *        U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.15 (OID_smimeCapabilites)
 *       U.P.SET {
 *          U.P.SEQUENCE {
 *             <...>
 *          }
 *       }
 *    }
 * }
 *
 * Type 2 - contains in an EFI Capsule:
 *
 * [C.P.0] {
 *   U.P.SEQUENCE {
 *      U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.3 (OID_contentType)
 *      U.P.SET {
 *         U.P.OBJECTIDENTIFIER 1.2.840.113549.1.7.1 (OID_data)
 *      }
 *   }
 *   U.P.SEQUENCE {
 *      U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.5 (OID_signingTime)
 *      U.P.SET {
 *         U.P.UTCTime '<siging_time>'
 *      }
 *   }
 *   U.P.SEQUENCE {
 *      U.P.OBJECTIDENTIFIER 1.2.840.113549.1.9.4 (OID_messageDigest)
 *      U.P.SET {
 *         U.P.OCTETSTRING <digest>
 *      }
 *  }
 *}
 *
 * Note:
 * They have different Content Type (OID_msIndirectData or OID_data).
 * OID_smimeCapabilites only exists in a PE/COFF EFI image.
 */
static int authattrs_parse(struct pkcs7_message *msg, void *aa, size_t aa_len,
			   struct pkcs7_signed_info *sinfo)
{
	unsigned char *p = aa;
	unsigned char *end = (unsigned char *)aa + aa_len;
	size_t len = 0;
	int ret;
	unsigned char *inner_p;
	size_t seq_len = 0;

	ret = mbedtls_asn1_get_tag(&p, end, &seq_len,
				   MBEDTLS_ASN1_CONTEXT_SPECIFIC |
				   MBEDTLS_ASN1_CONSTRUCTED);
	if (ret)
		return ret;

	while (!mbedtls_asn1_get_tag(&p, end, &seq_len,
				     MBEDTLS_ASN1_CONSTRUCTED |
				     MBEDTLS_ASN1_SEQUENCE)) {
		inner_p = p;
		ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
					   MBEDTLS_ASN1_OID);
		if (ret)
			return ret;

		if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_PKCS9_CONTENTTYPE, inner_p, len)) {
			inner_p += len;
			ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
						   MBEDTLS_ASN1_CONSTRUCTED |
						   MBEDTLS_ASN1_SET);
			if (ret)
				return ret;

			ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
						   MBEDTLS_ASN1_OID);
			if (ret)
				return ret;

			/*
			 * We should only support 1.2.840.113549.1.7.1 (OID_data)
			 * for PKCS7 DATA that is used in EFI Capsule and
			 * 1.3.6.1.4.1.311.2.1.4 (OID_msIndirectData) for
			 * MicroSoft Authentication Code that is used in EFI
			 * Secure Boot.
			 */
			if (MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_MICROSOFT_INDIRECTDATA,
						inner_p, len) &&
			    MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_PKCS7_DATA,
						inner_p, len))
				return -EINVAL;

			if (__test_and_set_bit(sinfo_has_content_type, &sinfo->aa_set))
				return -EINVAL;
		} else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_PKCS9_MESSAGEDIGEST, inner_p,
						len)) {
			inner_p += len;
			ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
						   MBEDTLS_ASN1_CONSTRUCTED |
						   MBEDTLS_ASN1_SET);
			if (ret)
				return ret;

			ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
						   MBEDTLS_ASN1_OCTET_STRING);
			if (ret)
				return ret;

			sinfo->msgdigest = inner_p;
			sinfo->msgdigest_len = len;

			if (__test_and_set_bit(sinfo_has_message_digest, &sinfo->aa_set))
				return -EINVAL;
		} else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_PKCS9_SIGNINGTIME, inner_p,
						len)) {
			mbedtls_x509_time st;

			inner_p += len;
			ret = mbedtls_asn1_get_tag(&inner_p, p + seq_len, &len,
						   MBEDTLS_ASN1_CONSTRUCTED |
						   MBEDTLS_ASN1_SET);
			if (ret)
				return ret;

			ret = mbedtls_x509_get_time(&inner_p, p + seq_len, &st);
			if (ret)
				return ret;
			sinfo->signing_time = x509_get_timestamp(&st);

			if (__test_and_set_bit(sinfo_has_signing_time, &sinfo->aa_set))
				return -EINVAL;
		} else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_PKCS9_SMIMECAP, inner_p,
						len)) {
			if (__test_and_set_bit(sinfo_has_smime_caps, &sinfo->aa_set))
				return -EINVAL;
		} else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_MICROSOFT_SPOPUSINFO, inner_p,
						len)) {
			if (__test_and_set_bit(sinfo_has_ms_opus_info, &sinfo->aa_set))
				return -EINVAL;
		} else if (!MBEDTLS_OID_CMP_RAW(MBEDTLS_OID_MICROSOFT_STATETYPE, inner_p,
						len)) {
			if (__test_and_set_bit(sinfo_has_ms_statement_type, &sinfo->aa_set))
				return -EINVAL;
		}

		p += seq_len;
	}

	msg->have_authattrs = true;

	/*
	 * Skip the leading tag byte (MBEDTLS_ASN1_CONTEXT_SPECIFIC |
	 * MBEDTLS_ASN1_CONSTRUCTED) to satisfy pkcs7_digest() when calculating
	 * the digest of authattrs.
	 */
	sinfo->authattrs = aa + 1;
	sinfo->authattrs_len = aa_len - 1;

	return 0;
}

static int x509_populate_content_data(struct pkcs7_message *msg,
				      mbedtls_pkcs7 *pkcs7_ctx)
{
	struct pkcs7_mbedtls_ctx *mctx;

	if (!pkcs7_ctx->content_data.data ||
	    !pkcs7_ctx->content_data.data_len)
		return 0;

	mctx = kzalloc(sizeof(*mctx), GFP_KERNEL);
	if (!mctx)
		return -ENOMEM;

	mctx->content_data = kmemdup(pkcs7_ctx->content_data.data,
				     pkcs7_ctx->content_data.data_len,
				     GFP_KERNEL);
	if (!mctx->content_data) {
		pkcs7_free_mbedtls_ctx(mctx);
		return -ENOMEM;
	}

	msg->data = mctx->content_data;
	msg->data_len = pkcs7_ctx->content_data.data_len;
	msg->data_hdrlen = pkcs7_ctx->content_data.data_hdrlen;
	msg->data_type = pkcs7_ctx->content_data.data_type;

	msg->mbedtls_ctx = mctx;
	return 0;
}

static int x509_populate_sinfo(struct pkcs7_message *msg,
			       mbedtls_pkcs7_signer_info *mb_sinfo,
			       struct pkcs7_signed_info **sinfo)
{
	struct pkcs7_signed_info *signed_info;
	struct public_key_signature *s;
	mbedtls_md_type_t md_alg;
	struct pkcs7_sinfo_mbedtls_ctx *mctx;
	int ret;

	signed_info = kzalloc(sizeof(*signed_info), GFP_KERNEL);
	if (!signed_info)
		return -ENOMEM;

	s = kzalloc(sizeof(*s), GFP_KERNEL);
	if (!s) {
		ret = -ENOMEM;
		goto out_no_sig;
	}

	mctx = kzalloc(sizeof(*mctx), GFP_KERNEL);
	if (!mctx) {
		ret = -ENOMEM;
		goto out_no_mctx;
	}

	/*
	 * Hash algorithm:
	 *
	 * alg_identifier =	digestAlgorithm (DigestAlgorithmIdentifier)
	 *			MbedTLS internally checks this field to ensure
	 *			it is the same as digest_alg_identifiers.
	 * sig_alg_identifier =	digestEncryptionAlgorithm
	 *			(DigestEncryptionAlgorithmIdentifier)
	 *			MbedTLS just saves this field without any actions.
	 * See function pkcs7_get_signer_info() for reference.
	 *
	 * Public key algorithm:
	 * No information related to public key algorithm under MbedTLS signer
	 * info. Assume that we are using RSA.
	 */
	ret = mbedtls_oid_get_md_alg(&mb_sinfo->alg_identifier, &md_alg);
	if (ret)
		goto out_err_sinfo;
	s->pkey_algo = "rsa";

	/* Translate the hash algorithm */
	switch (md_alg) {
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
		goto out_err_sinfo;
	}

	/*
	 * auth_ids holds AuthorityKeyIdentifier, aka akid
	 * auth_ids[0]:
	 *	[PKCS#7 or CMS ver 1] - generated from "Issuer + Serial number"
	 *	[CMS ver 3] - generated from skid (subjectKeyId)
	 * auth_ids[1]: generated from skid (subjectKeyId)
	 *
	 * Assume that we are using PKCS#7 (msg->version=1),
	 * not CMS ver 3 (msg->version=3).
	 */
	s->auth_ids[0] = asymmetric_key_generate_id(mb_sinfo->serial.p,
						    mb_sinfo->serial.len,
						    mb_sinfo->issuer_raw.p,
						    mb_sinfo->issuer_raw.len);
	if (!s->auth_ids[0]) {
		ret = -ENOMEM;
		goto out_err_sinfo;
	}

	/* skip s->auth_ids[1], no subjectKeyId in MbedTLS signer info ctx */

	/*
	 * Encoding can be pkcs1 or raw, but only pkcs1 is supported.
	 * Set the encoding explicitly to pkcs1.
	 */
	s->encoding = "pkcs1";

	/* Copy the signature data */
	s->s = kmemdup(mb_sinfo->sig.p, mb_sinfo->sig.len, GFP_KERNEL);
	if (!s->s) {
		ret = -ENOMEM;
		goto out_err_sinfo;
	}
	s->s_size = mb_sinfo->sig.len;
	signed_info->sig = s;

	/* Save the Authenticate Attributes data if exists */
	if (!mb_sinfo->authattrs.data || !mb_sinfo->authattrs.data_len) {
		kfree(mctx);
		goto no_authattrs;
	}

	mctx->authattrs_data = kmemdup(mb_sinfo->authattrs.data,
				       mb_sinfo->authattrs.data_len,
				       GFP_KERNEL);
	if (!mctx->authattrs_data) {
		ret = -ENOMEM;
		goto out_err_sinfo;
	}
	signed_info->mbedtls_ctx = mctx;

	/* If authattrs exists, decode it and parse msgdigest from it */
	ret = authattrs_parse(msg, mctx->authattrs_data,
			      mb_sinfo->authattrs.data_len,
			      signed_info);
	if (ret)
		goto out_err_sinfo;

no_authattrs:
	*sinfo = signed_info;
	return 0;

out_err_sinfo:
	pkcs7_free_sinfo_mbedtls_ctx(mctx);
out_no_mctx:
	public_key_signature_free(s);
out_no_sig:
	kfree(signed_info);
	return ret;
}

/*
 * Free a signed information block.
 */
static void pkcs7_free_signed_info(struct pkcs7_signed_info *sinfo)
{
	if (sinfo) {
		public_key_signature_free(sinfo->sig);
		pkcs7_free_sinfo_mbedtls_ctx(sinfo->mbedtls_ctx);
		kfree(sinfo);
	}
}

/**
 * pkcs7_free_message - Free a PKCS#7 message
 * @pkcs7: The PKCS#7 message to free
 */
void pkcs7_free_message(struct pkcs7_message *pkcs7)
{
	struct x509_certificate *cert;
	struct pkcs7_signed_info *sinfo;

	if (pkcs7) {
		while (pkcs7->certs) {
			cert = pkcs7->certs;
			pkcs7->certs = cert->next;
			x509_free_certificate(cert);
		}
		while (pkcs7->crl) {
			cert = pkcs7->crl;
			pkcs7->crl = cert->next;
			x509_free_certificate(cert);
		}
		while (pkcs7->signed_infos) {
			sinfo = pkcs7->signed_infos;
			pkcs7->signed_infos = sinfo->next;
			pkcs7_free_signed_info(sinfo);
		}
		pkcs7_free_mbedtls_ctx(pkcs7->mbedtls_ctx);
		kfree(pkcs7);
	}
}

struct pkcs7_message *pkcs7_parse_message(const void *data, size_t datalen)
{
	int i;
	int ret;
	mbedtls_pkcs7 pkcs7_ctx;
	mbedtls_pkcs7_signer_info *mb_sinfos;
	mbedtls_x509_crt *mb_certs;
	struct pkcs7_message *msg;
	struct x509_certificate **cert;
	struct pkcs7_signed_info **sinfos;

	msg = kzalloc(sizeof(*msg), GFP_KERNEL);
	if (!msg) {
		ret = -ENOMEM;
		goto out_no_msg;
	}

	/* Parse the DER encoded PKCS#7 message using MbedTLS */
	mbedtls_pkcs7_init(&pkcs7_ctx);
	ret = mbedtls_pkcs7_parse_der(&pkcs7_ctx, data, datalen);
	/* Check if it is a PKCS#7 message with signed data */
	if (ret != MBEDTLS_PKCS7_SIGNED_DATA)
		goto parse_fail;

	/* Assume that we are using PKCS#7, not CMS ver 3 */
	msg->version = 1;	/* 1 for [PKCS#7 or CMS ver 1] */

	/* Populate the certs to msg->certs */
	for (i = 0, cert = &msg->certs, mb_certs = &pkcs7_ctx.signed_data.certs;
	     i < pkcs7_ctx.signed_data.no_of_certs && mb_certs;
	     i++, cert = &(*cert)->next, mb_certs = mb_certs->next) {
		ret = x509_populate_cert(mb_certs, cert);
		if (ret)
			goto parse_fail;

		(*cert)->index = i + 1;
	}

	/*
	 * Skip populating crl, that is not currently in-use.
	 */

	/* Populate content data */
	ret = x509_populate_content_data(msg, &pkcs7_ctx);
	if (ret)
		goto parse_fail;

	/* Populate signed info to msg->signed_infos */
	for (i = 0, sinfos = &msg->signed_infos,
	     mb_sinfos = &pkcs7_ctx.signed_data.signers;
	     i < pkcs7_ctx.signed_data.no_of_signers && mb_sinfos;
	     i++, sinfos = &(*sinfos)->next, mb_sinfos = mb_sinfos->next) {
		ret = x509_populate_sinfo(msg, mb_sinfos, sinfos);
		if (ret)
			goto parse_fail;

		(*sinfos)->index = i + 1;
	}

	mbedtls_pkcs7_free(&pkcs7_ctx);
	return msg;

parse_fail:
	mbedtls_pkcs7_free(&pkcs7_ctx);
	pkcs7_free_message(msg);
out_no_msg:
	msg = ERR_PTR(ret);
	return msg;
}
