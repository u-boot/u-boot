/*
 * Copyright (C) 2017 Linaro
 * Bryan O'Donoghue <bryan.odonoghue@linaro.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <tee/optee.h>

int optee_verify_image(struct optee_header *hdr, unsigned long tzdram_start,
		       unsigned long tzdram_len, unsigned long image_len)
{
	unsigned long tzdram_end = tzdram_start + tzdram_len;
	uint32_t tee_file_size;

	tee_file_size = hdr->init_size + hdr->paged_size +
			sizeof(struct optee_header);

	if (hdr->magic != OPTEE_MAGIC ||
	    hdr->version != OPTEE_VERSION ||
	    hdr->init_load_addr_hi > tzdram_end ||
	    hdr->init_load_addr_lo < tzdram_start ||
	    tee_file_size > tzdram_len ||
	    tee_file_size != image_len ||
	    (hdr->init_load_addr_lo + tee_file_size) > tzdram_end) {
		return -EINVAL;
	}

	return 0;
}
