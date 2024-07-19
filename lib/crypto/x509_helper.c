// SPDX-License-Identifier: GPL-2.0+
/*
 * X509 helper functions
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */
#include <linux/err.h>
#include <crypto/public_key.h>
#include <crypto/x509_parser.h>

/*
 * Check for self-signedness in an X.509 cert and if found, check the signature
 * immediately if we can.
 */
int x509_check_for_self_signed(struct x509_certificate *cert)
{
	int ret = 0;

	if (cert->raw_subject_size != cert->raw_issuer_size ||
	    memcmp(cert->raw_subject, cert->raw_issuer,
		   cert->raw_issuer_size))
		goto not_self_signed;

	if (cert->sig->auth_ids[0] || cert->sig->auth_ids[1]) {
		/*
		 * If the AKID is present it may have one or two parts. If
		 * both are supplied, both must match.
		 */
		bool a = asymmetric_key_id_same(cert->skid,
						cert->sig->auth_ids[1]);
		bool b = asymmetric_key_id_same(cert->id,
						cert->sig->auth_ids[0]);

		if (!a && !b)
			goto not_self_signed;

		ret = -EKEYREJECTED;
		if (((a && !b) || (b && !a)) &&
		    cert->sig->auth_ids[0] && cert->sig->auth_ids[1])
			goto out;
	}

	ret = -EKEYREJECTED;
	if (strcmp(cert->pub->pkey_algo, cert->sig->pkey_algo))
		goto out;

	ret = public_key_verify_signature(cert->pub, cert->sig);
	if (ret == -ENOPKG) {
		cert->unsupported_sig = true;
		goto not_self_signed;
	}
	if (ret < 0)
		goto out;

	pr_devel("Cert Self-signature verified");
	cert->self_signed = true;

out:
	return ret;

not_self_signed:
	return 0;
}
