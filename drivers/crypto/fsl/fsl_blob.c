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

int blob_decrypt(u8 *key_mod, u8 *src, u8 *dst, u8 len)
{
	int ret, i = 0;
	u32 *desc;

	printf("\nDecapsulating data to form blob\n");
	desc = malloc(sizeof(int) * MAX_CAAM_DESCSIZE);
	if (!desc) {
		debug("Not enough memory for descriptor allocation\n");
		return -1;
	}

	inline_cnstr_jobdesc_blob_decap(desc, key_mod, src, dst, len);

	for (i = 0; i < 14; i++)
		printf("%x\n", *(desc + i));
	ret = run_descriptor_jr(desc);

	if (ret)
		printf("Error in Decapsulation %d\n", ret);

	free(desc);
	return ret;
}

int blob_encrypt(u8 *key_mod, u8 *src, u8 *dst, u8 len)
{
	int ret, i = 0;
	u32 *desc;

	printf("\nEncapsulating data to form blob\n");
	desc = malloc(sizeof(int) * MAX_CAAM_DESCSIZE);
	if (!desc) {
		debug("Not enough memory for descriptor allocation\n");
		return -1;
	}

	inline_cnstr_jobdesc_blob_encap(desc, key_mod, src, dst, len);
	for (i = 0; i < 14; i++)
		printf("%x\n", *(desc + i));
	ret = run_descriptor_jr(desc);

	if (ret)
		printf("Error in Encapsulation %d\n", ret);

	free(desc);
	return ret;
}
