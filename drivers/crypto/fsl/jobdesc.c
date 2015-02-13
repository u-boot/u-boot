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
#include "rsa_caam.h"

#define KEY_BLOB_SIZE			32
#define MAC_SIZE			16

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

void inline_cnstr_jobdesc_blob_encap(uint32_t *desc, uint8_t *key_idnfr,
				     uint8_t *plain_txt, uint8_t *enc_blob,
				     uint32_t in_sz)
{
	dma_addr_t dma_addr_key_idnfr, dma_addr_in, dma_addr_out;
	uint32_t key_sz = KEY_IDNFR_SZ_BYTES;
	/* output blob will have 32 bytes key blob in beginning and
	 * 16 byte HMAC identifier at end of data blob */
	uint32_t out_sz = in_sz + KEY_BLOB_SIZE + MAC_SIZE;

	dma_addr_key_idnfr = virt_to_phys((void *)key_idnfr);
	dma_addr_in	= virt_to_phys((void *)plain_txt);
	dma_addr_out	= virt_to_phys((void *)enc_blob);

	init_job_desc(desc, 0);

	append_key(desc, dma_addr_key_idnfr, key_sz, CLASS_2);

	append_seq_in_ptr(desc, dma_addr_in, in_sz, 0);

	append_seq_out_ptr(desc, dma_addr_out, out_sz, 0);

	append_operation(desc, OP_TYPE_ENCAP_PROTOCOL | OP_PCLID_BLOB);
}

void inline_cnstr_jobdesc_blob_decap(uint32_t *desc, uint8_t *key_idnfr,
				     uint8_t *enc_blob, uint8_t *plain_txt,
				     uint32_t out_sz)
{
	dma_addr_t dma_addr_key_idnfr, dma_addr_in, dma_addr_out;
	uint32_t key_sz = KEY_IDNFR_SZ_BYTES;
	uint32_t in_sz = out_sz + KEY_BLOB_SIZE + MAC_SIZE;

	dma_addr_key_idnfr = virt_to_phys((void *)key_idnfr);
	dma_addr_in	= virt_to_phys((void *)enc_blob);
	dma_addr_out	= virt_to_phys((void *)plain_txt);

	init_job_desc(desc, 0);

	append_key(desc, dma_addr_key_idnfr, key_sz, CLASS_2);

	append_seq_in_ptr(desc, dma_addr_in, in_sz, 0);

	append_seq_out_ptr(desc, dma_addr_out, out_sz, 0);

	append_operation(desc, OP_TYPE_DECAP_PROTOCOL | OP_PCLID_BLOB);
}

/*
 * Descriptor to instantiate RNG State Handle 0 in normal mode and
 * load the JDKEK, TDKEK and TDSK registers
 */
void inline_cnstr_jobdesc_rng_instantiation(uint32_t *desc)
{
	u32 *jump_cmd;

	init_job_desc(desc, 0);

	/* INIT RNG in non-test mode */
	append_operation(desc, OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
			 OP_ALG_AS_INIT);

	/* wait for done */
	jump_cmd = append_jump(desc, JUMP_CLASS_CLASS1);
	set_jump_tgt_here(desc, jump_cmd);

	/*
	 * load 1 to clear written reg:
	 * resets the done interrrupt and returns the RNG to idle.
	 */
	append_load_imm_u32(desc, 1, LDST_SRCDST_WORD_CLRW);

	/* generate secure keys (non-test) */
	append_operation(desc, OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
			 OP_ALG_RNG4_SK);
}

/* Change key size to bytes form bits in calling function*/
void inline_cnstr_jobdesc_pkha_rsaexp(uint32_t *desc,
				      struct pk_in_params *pkin, uint8_t *out,
				      uint32_t out_siz)
{
	dma_addr_t dma_addr_e, dma_addr_a, dma_addr_n, dma_addr_out;

	dma_addr_e = virt_to_phys((void *)pkin->e);
	dma_addr_a = virt_to_phys((void *)pkin->a);
	dma_addr_n = virt_to_phys((void *)pkin->n);
	dma_addr_out = virt_to_phys((void *)out);

	init_job_desc(desc, 0);
	append_key(desc, dma_addr_e, pkin->e_siz, KEY_DEST_PKHA_E | CLASS_1);

	append_fifo_load(desc, dma_addr_a,
			 pkin->a_siz, LDST_CLASS_1_CCB | FIFOLD_TYPE_PK_A);

	append_fifo_load(desc, dma_addr_n,
			 pkin->n_siz, LDST_CLASS_1_CCB | FIFOLD_TYPE_PK_N);

	append_operation(desc, OP_TYPE_PK | OP_ALG_PK | OP_ALG_PKMODE_MOD_EXPO);

	append_fifo_store(desc, dma_addr_out, out_siz,
			  LDST_CLASS_1_CCB | FIFOST_TYPE_PKHA_B);
}
