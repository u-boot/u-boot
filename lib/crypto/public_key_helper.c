// SPDX-License-Identifier: GPL-2.0+
/*
 * X509 helper functions
 *
 * Copyright (c) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */
#include <linux/compat.h>
#include <crypto/public_key.h>

/*
 * Destroy a public key algorithm key.
 */
void public_key_free(struct public_key *key)
{
	if (key) {
		kfree(key->key);
		kfree(key->params);
		kfree(key);
	}
}

/*
 * from <linux>/crypto/asymmetric_keys/signature.c
 *
 * Destroy a public key signature.
 */
void public_key_signature_free(struct public_key_signature *sig)
{
	int i;

	if (sig) {
		for (i = 0; i < ARRAY_SIZE(sig->auth_ids); i++)
			kfree(sig->auth_ids[i]);
		kfree(sig->s);
		kfree(sig->digest);
		kfree(sig);
	}
}
