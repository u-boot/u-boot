/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#include <common.h>
#include <malloc.h>
#include "jobdesc.h"
#include "desc.h"
#include "jr.h"

#define CRYPTO_MAX_ALG_NAME	80
#define SHA1_DIGEST_SIZE        20
#define SHA256_DIGEST_SIZE      32

struct caam_hash_template {
	char name[CRYPTO_MAX_ALG_NAME];
	unsigned int digestsize;
	u32 alg_type;
};

enum caam_hash_algos {
	SHA1 = 0,
	SHA256
};

static struct caam_hash_template driver_hash[] = {
	{
		.name = "sha1",
		.digestsize = SHA1_DIGEST_SIZE,
		.alg_type = OP_ALG_ALGSEL_SHA1,
	},
	{
		.name = "sha256",
		.digestsize = SHA256_DIGEST_SIZE,
		.alg_type = OP_ALG_ALGSEL_SHA256,
	},
};

int caam_hash(const unsigned char *pbuf, unsigned int buf_len,
	      unsigned char *pout, enum caam_hash_algos algo)
{
	int ret = 0;
	uint32_t *desc;

	desc = malloc(sizeof(int) * MAX_CAAM_DESCSIZE);
	if (!desc) {
		debug("Not enough memory for descriptor allocation\n");
		return -1;
	}

	inline_cnstr_jobdesc_hash(desc, pbuf, buf_len, pout,
				  driver_hash[algo].alg_type,
				  driver_hash[algo].digestsize,
				  0);

	ret = run_descriptor_jr(desc);

	free(desc);
	return ret;
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (caam_hash(pbuf, buf_len, pout, SHA256))
		printf("CAAM was not setup properly or it is faulty\n");
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
			unsigned char *pout, unsigned int chunk_size)
{
	if (caam_hash(pbuf, buf_len, pout, SHA1))
		printf("CAAM was not setup properly or it is faulty\n");
}
