/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _SM3_H
#define _SM3_H

#define SM3_DIGEST_SIZE	32	/* 256 bits */
#define SM3_BLOCK_SIZE	64	/* 512 bits */
#define SM3_PAD_UNIT	56	/* 448 bits */

#define SM3_T1		0x79CC4519
#define SM3_T2		0x7A879D8A

#define SM3_IVA		0x7380166f
#define SM3_IVB		0x4914b2b9
#define SM3_IVC		0x172442d7
#define SM3_IVD		0xda8a0600
#define SM3_IVE		0xa96f30bc
#define SM3_IVF		0x163138aa
#define SM3_IVG		0xe38dee4d
#define SM3_IVH		0xb0fb0e4e

struct sm3_context {
	uint32_t state[SM3_DIGEST_SIZE / 4];
	uint64_t count; /* Message length in bits */
	uint8_t buffer[SM3_BLOCK_SIZE];
	int buflen;
};

void sm3_init(struct sm3_context *sctx);
void sm3_update(struct sm3_context *sctx, const uint8_t *input, size_t ilen);
void sm3_final(struct sm3_context *sctx, uint8_t output[SM3_DIGEST_SIZE]);
void sm3_hash(const uint8_t *input, size_t ilen, uint8_t output[SM3_DIGEST_SIZE]);

void sm3_csum_wd(const unsigned char *input, uint32_t len,
		 unsigned char *output, unsigned int chunk_sz);
#endif
