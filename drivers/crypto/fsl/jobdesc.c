/*
 * SEC Descriptor Construction Library
 * Basic job descriptor construction
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#include <common.h>
#include "desc_constr.h"
#include "jobdesc.h"

void inline_cnstr_jobdesc_hash(uint32_t *desc,
			  const uint8_t *msg, uint32_t msgsz, uint8_t *digest,
			  u32 alg_type, uint32_t alg_size, int sg_tbl)
{
	/* SHA 256 , output is of length 32 words */
	uint32_t storelen = alg_size;
	u32 options;
	dma_addr_t dma_addr_in, dma_addr_out;

	dma_addr_in = virt_to_phys((void *)msg);
	dma_addr_out = virt_to_phys((void *)digest);

	init_job_desc(desc, 0);
	append_operation(desc, OP_TYPE_CLASS2_ALG |
			 OP_ALG_AAI_HASH | OP_ALG_AS_INITFINAL |
			 OP_ALG_ENCRYPT | OP_ALG_ICV_OFF | alg_type);

	options = LDST_CLASS_2_CCB | FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST2;
	if (sg_tbl)
		options |= FIFOLDST_SGF;
	if (msgsz > 0xffff) {
		options |= FIFOLDST_EXT;
		append_fifo_load(desc, dma_addr_in, 0, options);
		append_cmd(desc, msgsz);
	} else {
		append_fifo_load(desc, dma_addr_in, msgsz, options);
	}

	append_store(desc, dma_addr_out, storelen,
		     LDST_CLASS_2_CCB | LDST_SRCDST_BYTE_CONTEXT);
}
