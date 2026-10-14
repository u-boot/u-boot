// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <log.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>

#include <crypto/internal/ecdsa.h>

#include "mbedtls_options.h" /* required to access private fields */
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>

static mbedtls_ecp_group_id ecdsa_search_group_id(const char *curve_name)
{
	mbedtls_ecp_group_id grp_id = MBEDTLS_ECP_DP_NONE;
	const mbedtls_ecp_curve_info *info;

	if (!curve_name)
		return MBEDTLS_ECP_DP_NONE;

	/*
	 * This curve name is read in the FIT metadata.
	 * When this FIT metadata are filled by mkimage (or binman),
	 * the curve name is the alias/name provided by OpenSSL.
	 * And for secp256r1, OpenSSL uses the alias prime256v1.
	 * So here, we have to manage this OpenSSL alias.
	 */
	if (!strcmp(curve_name, "prime256v1"))
		return MBEDTLS_ECP_DP_SECP256R1;

	info = mbedtls_ecp_curve_list();
	while (info && info->name) {
		if (!strcmp(curve_name, info->name)) {
			grp_id = info->grp_id;
			break;
		}
		info++;
	}

	return grp_id;
}

int ecdsa_hash_verify(const struct ecdsa_public_key *pubkey,
		      const void *hash, size_t hash_len,
		      const void *signature, size_t sig_len)
{
	mbedtls_ecp_group_id grp_id;
	mbedtls_ecp_group grp;
	mbedtls_ecp_point Q;
	mbedtls_mpi r, s;
	int key_len;
	int err;

	key_len = DIV_ROUND_UP(pubkey->size_bits, 8);

	/* check the signature len */
	if (sig_len != 2 * key_len) {
		log_debug("sig len should be twice the key len (sig len = %zu, key len = %d)\n",
			  sig_len, key_len);
		err = -EINVAL;
		goto out;
	}

	/* search the group */
	grp_id = ecdsa_search_group_id(pubkey->curve_name);
	if (grp_id == MBEDTLS_ECP_DP_NONE) {
		log_debug("curve name %s not found\n", pubkey->curve_name);
		err = -EINVAL;
		goto out;
	}

	/* init and load the group */
	mbedtls_ecp_group_init(&grp);
	err = mbedtls_ecp_group_load(&grp, grp_id);
	if (err) {
		err = -EINVAL;
		goto free_grp;
	}

	/* prepare the pubkey */
	mbedtls_ecp_point_init(&Q);
	err = mbedtls_mpi_read_binary(&Q.X, pubkey->x, key_len);
	if (err) {
		log_debug("could not read value x of the public key (err = %d)\n",
			  err);
		err = -EINVAL;
		goto free_q;
	}
	err = mbedtls_mpi_read_binary(&Q.Y, pubkey->y, key_len);
	if (err) {
		log_debug("could not read value y of the public key (err = %d)\n",
			  err);
		err = -EINVAL;
		goto free_q;
	}
	err = mbedtls_mpi_lset(&Q.Z, 1);
	if (err) {
		log_debug("could not set value z of the public key (err = %d)\n",
			  err);
		err = -EINVAL;
		goto free_q;
	}

	/* check if the pubkey is valid */
	err = mbedtls_ecp_check_pubkey(&grp, &Q);
	if (err) {
		log_debug("public key is invalid (err = %d)\n", err);
		err = -EKEYREJECTED;
		goto free_q;
	}

	/* compute r */
	mbedtls_mpi_init(&r);
	err = mbedtls_mpi_read_binary(&r, signature, key_len);
	if (err) {
		log_debug("could not read value r of the signature (err = %d)\n",
			  err);
		err = -EINVAL;
		goto free_r;
	}

	/* compute s */
	mbedtls_mpi_init(&s);
	err = mbedtls_mpi_read_binary(&s, signature + key_len, key_len);
	if (err) {
		log_debug("could not read value s of the signature (err = %d)\n",
			  err);
		err = -EINVAL;
		goto free_s;
	}

	/* check the signature */
	err = mbedtls_ecdsa_verify(&grp, hash, hash_len, &Q, &r, &s);
	if (err)
		err = -EINVAL;

 free_s:
	mbedtls_mpi_free(&s);
 free_r:
	mbedtls_mpi_free(&r);
 free_q:
	mbedtls_ecp_point_free(&Q);
 free_grp:
	mbedtls_ecp_group_free(&grp);
 out:

	return err;
}
