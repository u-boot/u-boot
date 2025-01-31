/* SPDX-License-Identifier: GPL-2.0-or-later */
/* PKCS#7 crypto data parser internal definitions
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#ifndef _PKCS7_PARSER_H
#define _PKCS7_PARSER_H

#include <linux/oid_registry.h>
#include <crypto/pkcs7.h>
#include <crypto/x509_parser.h>
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include "mbedtls_options.h"
#include <mbedtls/pkcs7.h>
#include <library/x509_internal.h>
#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#endif
#include <linux/printk.h>

#define kenter(FMT, ...) \
	pr_devel("==> %s("FMT")\n", __func__, ##__VA_ARGS__)
#define kleave(FMT, ...) \
	pr_devel("<== %s()"FMT"\n", __func__, ##__VA_ARGS__)

/* Backup the parsed MedTLS context that we need */
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
struct pkcs7_mbedtls_ctx {
	void *content_data;
};

struct pkcs7_sinfo_mbedtls_ctx {
	void *authattrs_data;
	void *content_data_digest;
};
#endif

/*
 * MbedTLS integration Notes:
 *
 * MbedTLS PKCS#7 library does not originally support parsing MicroSoft
 * Authentication Code which is used for verifying the PE image digest.
 *
 * 1.	Authenticated Attributes (authenticatedAttributes)
 *	MbedTLS assumes unauthenticatedAttributes and authenticatedAttributes
 *	fields not exist.
 *	See MbedTLS function 'pkcs7_get_signer_info' for details.
 *
 * 2.	MicroSoft Authentication Code (mscode)
 *	MbedTLS only supports Content Data type defined as 1.2.840.113549.1.7.1
 *	(MBEDTLS_OID_PKCS7_DATA, aka OID_data).
 *	1.3.6.1.4.1.311.2.1.4 (MicroSoft Authentication Code, aka
 *	OID_msIndirectData) is not supported.
 *	See MbedTLS function 'pkcs7_get_content_info_type' for details.
 *
 * But the EFI loader assumes that a PKCS#7 message with an EFI image always
 * contains MicroSoft Authentication Code as Content Data (msg->data is NOT
 * NULL), see function 'efi_signature_verify'.
 *
 * MbedTLS patch "0002-support-MicroSoft-authentication-code-in-PKCS7-lib.patch"
 * is to support both above features by parsing the Content Data and
 * Authenticate Attributes from a given PKCS#7 message.
 *
 * Other fields we don't need to populate from MbedTLS, which are used
 * internally by pkcs7_verify:
 * 'signer', 'unsupported_crypto', 'blacklisted'
 * 'sig->digest' is used internally by pkcs7_digest to calculate the hash of
 * Content Data or Authenticate Attributes.
 */
struct pkcs7_signed_info {
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
	struct pkcs7_sinfo_mbedtls_ctx *mbedtls_ctx;
#endif
	struct pkcs7_signed_info *next;
	struct x509_certificate *signer; /* Signing certificate (in msg->certs) */
	unsigned	index;
	bool		unsupported_crypto;	/* T if not usable due to missing crypto */
	bool		blacklisted;

	/* Message digest - the digest of the Content Data (or NULL) */
	const void	*msgdigest;
	unsigned	msgdigest_len;

	/* Authenticated Attribute data (or NULL) */
	unsigned	authattrs_len;
	const void	*authattrs;
	unsigned long	aa_set;
#define	sinfo_has_content_type		0
#define	sinfo_has_signing_time		1
#define	sinfo_has_message_digest	2
#define sinfo_has_smime_caps		3
#define	sinfo_has_ms_opus_info		4
#define	sinfo_has_ms_statement_type	5
	time64_t	signing_time;

	/* Message signature.
	 *
	 * This contains the generated digest of _either_ the Content Data or
	 * the Authenticated Attributes [RFC2315 9.3].  If the latter, one of
	 * the attributes contains the digest of the the Content Data within
	 * it.
	 *
	 * THis also contains the issuing cert serial number and issuer's name
	 * [PKCS#7 or CMS ver 1] or issuing cert's SKID [CMS ver 3].
	 */
	struct public_key_signature *sig;
};

struct pkcs7_message {
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
	struct pkcs7_mbedtls_ctx *mbedtls_ctx;
#endif
	struct x509_certificate *certs;	/* Certificate list */
	struct x509_certificate *crl;	/* Revocation list */
	struct pkcs7_signed_info *signed_infos;
	u8		version;	/* Version of cert (1 -> PKCS#7 or CMS; 3 -> CMS) */
	bool		have_authattrs;	/* T if have authattrs */

	/* Content Data (or NULL) */
	enum OID	data_type;	/* Type of Data */
	size_t		data_len;	/* Length of Data */
	size_t		data_hdrlen;	/* Length of Data ASN.1 header */
	const void	*data;		/* Content Data (or 0) */
};
#endif /* _PKCS7_PARSER_H */
