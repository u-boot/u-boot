// SPDX-License-Identifier: GPL-2.0+
/*
 * PKCS7 helper functions
 *
 * Copyright (c) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */
#include <linux/kernel.h>
#include <linux/err.h>
#include <crypto/pkcs7_parser.h>

/**
 * pkcs7_get_content_data - Get access to the PKCS#7 content
 * @pkcs7: The preparsed PKCS#7 message to access
 * @_data: Place to return a pointer to the data
 * @_data_len: Place to return the data length
 * @_headerlen: Size of ASN.1 header not included in _data
 *
 * Get access to the data content of the PKCS#7 message.  The size of the
 * header of the ASN.1 object that contains it is also provided and can be used
 * to adjust *_data and *_data_len to get the entire object.
 *
 * Returns -ENODATA if the data object was missing from the message.
 */
int pkcs7_get_content_data(const struct pkcs7_message *pkcs7,
			   const void **_data, size_t *_data_len,
			   size_t *_headerlen)
{
	if (!pkcs7->data)
		return -ENODATA;

	*_data = pkcs7->data;
	*_data_len = pkcs7->data_len;
	if (_headerlen)
		*_headerlen = pkcs7->data_hdrlen;
	return 0;
}
