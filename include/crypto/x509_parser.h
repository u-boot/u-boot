/* SPDX-License-Identifier: GPL-2.0-or-later */
/* X.509 certificate parser internal definitions
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#ifndef _X509_PARSER_H
#define _X509_PARSER_H

#include <linux/time.h>
#include <crypto/public_key.h>
#include <keys/asymmetric-type.h>
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include <image.h>
#include <mbedtls/error.h>
#include <mbedtls/asn1.h>
#endif

#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
struct x509_cert_mbedtls_ctx {
	void	*tbs;			/* Signed data */
	void	*raw_serial;		/* Raw serial number in ASN.1 */
	void	*raw_issuer;		/* Raw issuer name in ASN.1 */
	void	*raw_subject;		/* Raw subject name in ASN.1 */
	void	*raw_skid;		/* Raw subjectKeyId in ASN.1 */
};
#endif

/*
 * MbedTLS integration Notes:
 *
 * Fields we don't need to populate from MbedTLS context:
 * 'raw_sig' and 'raw_sig_size' are buffer for x509_parse_context,
 * not needed for MbedTLS.
 * 'signer' and 'seen' are used internally by pkcs7_verify.
 * 'verified' is not in use.
 */
struct x509_certificate {
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
	struct x509_cert_mbedtls_ctx *mbedtls_ctx;
#endif
	struct x509_certificate *next;
	struct x509_certificate *signer;	/* Certificate that signed this one */
	struct public_key *pub;			/* Public key details */
	struct public_key_signature *sig;	/* Signature parameters */
	char		*issuer;		/* Name of certificate issuer */
	char		*subject;		/* Name of certificate subject */
	struct asymmetric_key_id *id;		/* Issuer + Serial number */
	struct asymmetric_key_id *skid;		/* Subject + subjectKeyId (optional) */
	time64_t	valid_from;
	time64_t	valid_to;
	const void	*tbs;			/* Signed data */
	unsigned	tbs_size;		/* Size of signed data */
	unsigned	raw_sig_size;		/* Size of sigature */
	const void	*raw_sig;		/* Signature data */
	const void	*raw_serial;		/* Raw serial number in ASN.1 */
	unsigned	raw_serial_size;
	unsigned	raw_issuer_size;
	const void	*raw_issuer;		/* Raw issuer name in ASN.1 */
	const void	*raw_subject;		/* Raw subject name in ASN.1 */
	unsigned	raw_subject_size;
	unsigned	raw_skid_size;
	const void	*raw_skid;		/* Raw subjectKeyId in ASN.1 */
	unsigned	index;
	bool		seen;			/* Infinite recursion prevention */
	bool		verified;
	bool		self_signed;		/* T if self-signed (check unsupported_sig too) */
	bool		unsupported_key;	/* T if key uses unsupported crypto */
	bool		unsupported_sig;	/* T if signature uses unsupported crypto */
	bool		blacklisted;
};

/*
 * x509_cert_parser.c
 */
extern void x509_free_certificate(struct x509_certificate *cert);
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
/**
 * x509_populate_pubkey() - Populate public key from MbedTLS context
 *
 * @cert:	Pointer to MbedTLS X509 cert
 * @pub_key:	Pointer to the populated public key handle
 * Return: 0 on succcess, error code on failure
 */
int x509_populate_pubkey(mbedtls_x509_crt *cert, struct public_key **pub_key);
/**
 * x509_populate_cert() - Populate X509 cert from MbedTLS context
 *
 * @mbedtls_cert:	Pointer to MbedTLS X509 cert
 * @pcert:		Pointer to the populated X509 cert handle
 * Return: 0 on succcess, error code on failure
 */
int x509_populate_cert(mbedtls_x509_crt *mbedtls_cert,
		       struct x509_certificate **pcert);
/**
 * x509_get_timestamp() - Translate timestamp from MbedTLS context
 *
 * @x509_time:	Pointer to MbedTLS time
 * Return: Time in time64_t format
 */
time64_t x509_get_timestamp(const mbedtls_x509_time *x509_time);
#endif
extern struct x509_certificate *x509_cert_parse(const void *data, size_t datalen);
extern int x509_decode_time(time64_t *_t,  size_t hdrlen,
			    unsigned char tag,
			    const unsigned char *value, size_t vlen);

/*
 * x509_public_key.c
 */
#if !CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
extern int x509_get_sig_params(struct x509_certificate *cert);
#endif
extern int x509_check_for_self_signed(struct x509_certificate *cert);
#endif /* _X509_PARSER_H */
