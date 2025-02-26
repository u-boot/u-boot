/* SPDX-License-Identifier: GPL-2.0-or-later */
/* PE Binary parser bits
 *
 * Copyright (C) 2014 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#include <crypto/pkcs7.h>
#ifndef __UBOOT__
#include <crypto/hash_info.h>
#endif
#if CONFIG_IS_ENABLED(MBEDTLS_LIB_X509)
#include "mbedtls_options.h"
#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#endif

struct pefile_context {
#ifndef __UBOOT__
	unsigned	header_size;
	unsigned	image_checksum_offset;
	unsigned	cert_dirent_offset;
	unsigned	n_data_dirents;
	unsigned	n_sections;
	unsigned	certs_size;
	unsigned	sig_offset;
	unsigned	sig_len;
	const struct section_header *secs;
#endif

	/* PKCS#7 MS Individual Code Signing content */
	const void	*digest;		/* Digest */
	unsigned	digest_len;		/* Digest length */
	const char	*digest_algo;		/* Digest algorithm */
};

#ifndef __UBOOT__
#define kenter(FMT, ...)					\
	pr_devel("==> %s("FMT")\n", __func__, ##__VA_ARGS__)
#define kleave(FMT, ...) \
	pr_devel("<== %s()"FMT"\n", __func__, ##__VA_ARGS__)
#endif

/*
 * mscode_parser.c
 */
extern int mscode_parse(void *_ctx, const void *content_data, size_t data_len,
			size_t asn1hdrlen);
