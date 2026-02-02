// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Philippe Reynes <philippe.reynes@softathome.com>
 */
#include <crypto/ecdsa-uclass.h>
#include <crypto/internal/ecdsa.h>
#include <dm.h>
#include <linux/types.h>

static int ops_sw_ecdsa_verify(__always_unused struct udevice *dev,
			       const struct ecdsa_public_key *pubkey,
			       const void *hash, size_t hash_len,
			       const void *signature, size_t sig_len)
{
	return ecdsa_hash_verify(pubkey, hash, hash_len, signature, sig_len);
}

static const struct ecdsa_ops sw_ecdsa_ops = {
	.verify = ops_sw_ecdsa_verify,
};

U_BOOT_DRIVER(sw_ecdsa) = {
	.name	= "sw_ecdsa",
	.id	= UCLASS_ECDSA,
	.ops	= &sw_ecdsa_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(sw_ecdsa) = {
	.name = "sw_ecdsa",
};

