// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <hash.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/iopoll.h>

#define SHA512_BLOCK_LENGTH     (1024 / 8)

/* Register fields */
#define HASH_CTR_STS_SHA_EN             BIT(0)
#define HASH_CTR_STS_SHA_BUSY           BIT(1)
#define HASH_CTR_STS_SHA_RST            BIT(2)
#define HASH_CFG_SHA1_SHA2              BIT(0)
#define SHA512_CMD_SHA_512		BIT(3)
#define SHA512_CMD_INTERNAL_ROUND	BIT(2)
#define SHA512_CMD_WRITE		BIT(1)
#define SHA512_CMD_READ			BIT(0)

enum {
	type_sha1 = 0,
	type_sha256,
	type_sha384,
	type_sha512,
};

struct npcm_sha_regs {
	u8 data_in;
	u8 data_out;
	u8 ctr_sts;
	u8 hash_cfg;
	u8 sha512_cmd;
};

struct hash_info {
	u32 block_sz;
	u32 digest_len;
	u8 length_bytes;
	u8 type;
};

struct message_block {
	u64 length[2];
	u64 nonhash_sz;
	u8 buffer[SHA512_BLOCK_LENGTH * 2];
};

struct npcm_sha_priv {
	void *base;
	struct npcm_sha_regs *regs;
	struct hash_info *hash;
	struct message_block block;
	bool internal_round;
	bool support_sha512;
};

static struct npcm_sha_regs npcm_sha_reg_tbl[] = {
	{ .data_in = 0x0, .data_out = 0x20, .ctr_sts = 0x4, .hash_cfg = 0x8 },
	{ .data_in = 0x10, .data_out = 0x1c, .ctr_sts = 0x14, .sha512_cmd = 0x18 },
};

static struct hash_info npcm_hash_tbl[] = {
	{ .type = type_sha1, .block_sz = 64, .digest_len = 160, .length_bytes = 8 },
	{ .type = type_sha256, .block_sz = 64, .digest_len = 256, .length_bytes = 8 },
	{ .type = type_sha384, .block_sz = 128, .digest_len = 384, .length_bytes = 16 },
	{ .type = type_sha512, .block_sz = 128, .digest_len = 512, .length_bytes = 16 },
};

static struct npcm_sha_priv *sha_priv;

static int npcm_sha_init(u8 type)
{
	struct message_block *block = &sha_priv->block;

	if (type > type_sha512 ||
	    (!sha_priv->support_sha512 &&
	    (type == type_sha384 || type == type_sha512)))
		return -ENOTSUPP;

	sha_priv->regs = &npcm_sha_reg_tbl[type / 2];
	sha_priv->hash = &npcm_hash_tbl[type];
	block->length[0] = 0;
	block->length[1] = 0;
	block->nonhash_sz = 0;
	sha_priv->internal_round = false;

	return 0;
}

static void npcm_sha_reset(void)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	struct hash_info *hash = sha_priv->hash;
	u8 val;

	if (hash->type == type_sha1)
		writeb(HASH_CFG_SHA1_SHA2, sha_priv->base + regs->hash_cfg);
	else if (hash->type == type_sha256)
		writeb(0, sha_priv->base + regs->hash_cfg);
	else if (hash->type == type_sha384)
		writeb(0, sha_priv->base + regs->sha512_cmd);
	else if (hash->type == type_sha512)
		writeb(SHA512_CMD_SHA_512, sha_priv->base + regs->sha512_cmd);

	val = readb(sha_priv->base + regs->ctr_sts) & ~HASH_CTR_STS_SHA_EN;
	writeb(val | HASH_CTR_STS_SHA_RST, sha_priv->base + regs->ctr_sts);
}

static void npcm_sha_enable(bool on)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	u8 val;

	val = readb(sha_priv->base + regs->ctr_sts) & ~HASH_CTR_STS_SHA_EN;
	val |= on;
	writeb(val | on, sha_priv->base + regs->ctr_sts);
}

static int npcm_sha_flush_block(u8 *block)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	struct hash_info *hash = sha_priv->hash;
	u32 *blk_dw = (u32 *)block;
	u8 val;
	int i;

	if (readb_poll_timeout(sha_priv->base + regs->ctr_sts, val,
			       !(val & HASH_CTR_STS_SHA_BUSY), 100))
		return -ETIMEDOUT;

	if (hash->type == type_sha384 || hash->type == type_sha512) {
		val = SHA512_CMD_WRITE;
		if (hash->type == type_sha512)
			val |= SHA512_CMD_SHA_512;
		if (sha_priv->internal_round)
			val |= SHA512_CMD_INTERNAL_ROUND;
		writeb(val, sha_priv->base + regs->sha512_cmd);
	}
	for (i = 0; i < (hash->block_sz / sizeof(u32)); i++)
		writel(blk_dw[i], sha_priv->base + regs->data_in);

	sha_priv->internal_round = true;

	return 0;
}

static int npcm_sha_update_block(const u8 *in, u32 len)
{
	struct message_block *block = &sha_priv->block;
	struct hash_info *hash = sha_priv->hash;
	u8 *buffer = &block->buffer[0];
	u32 block_sz = hash->block_sz;
	u32 hash_sz;

	hash_sz = (block->nonhash_sz + len) > block_sz ?
		(block_sz - block->nonhash_sz) : len;
	memcpy(buffer + block->nonhash_sz, in, hash_sz);
	block->nonhash_sz += hash_sz;
	block->length[0] += hash_sz;
	if (block->length[0] < hash_sz)
		block->length[1]++;

	if (block->nonhash_sz == block_sz) {
		block->nonhash_sz = 0;
		if (npcm_sha_flush_block(buffer))
			return -EBUSY;
	}

	return hash_sz;
}

static int npcm_sha_update(const u8 *input, u32 len)
{
	int hash_sz;

	while (len) {
		hash_sz = npcm_sha_update_block(input, len);
		if (hash_sz < 0) {
			printf("SHA512 module busy\n");
			return -EBUSY;
		}
		len -= hash_sz;
		input += hash_sz;
	}

	return 0;
}

static int npcm_sha_finish(u8 *out)
{
	struct npcm_sha_regs *regs = sha_priv->regs;
	struct message_block *block = &sha_priv->block;
	struct hash_info *hash = sha_priv->hash;
	u8 *buffer = &block->buffer[0];
	u32 block_sz = hash->block_sz;
	u32 *out32 = (u32 *)out;
	u32 zero_len, val;
	u64 *length;
	u8 reg_data_out;
	int i;

	/* Padding, minimal padding size is last_byte+length_bytes */
	if ((block_sz - block->nonhash_sz) >= (hash->length_bytes + 1))
		zero_len = block_sz - block->nonhash_sz - (hash->length_bytes + 1);
	else
		zero_len = block_sz * 2 - block->nonhash_sz - (hash->length_bytes + 1);
	/* Last byte */
	buffer[block->nonhash_sz++] = 0x80;
	/* Zero bits padding */
	memset(&buffer[block->nonhash_sz], 0, zero_len);
	block->nonhash_sz += zero_len;
	/* Message length */
	length = (u64 *)&buffer[block->nonhash_sz];
	if (hash->length_bytes == 16) {
		*length++ = cpu_to_be64(block->length[1] << 3 | block->length[0] >> 61);
		block->nonhash_sz += 8;
	}
	*length = cpu_to_be64(block->length[0] << 3);
	block->nonhash_sz += 8;
	if (npcm_sha_flush_block(&block->buffer[0]))
		return -ETIMEDOUT;

	/* After padding, the last message may produce 2 blocks */
	if (block->nonhash_sz > block_sz) {
		if (npcm_sha_flush_block(&block->buffer[block_sz]))
			return -ETIMEDOUT;
	}
	/* Read digest */
	if (readb_poll_timeout(sha_priv->base + regs->ctr_sts, val,
			       !(val & HASH_CTR_STS_SHA_BUSY), 100))
		return -ETIMEDOUT;
	if (hash->type == type_sha384)
		writeb(SHA512_CMD_READ, sha_priv->base + regs->sha512_cmd);
	else if (hash->type == type_sha512)
		writeb(SHA512_CMD_SHA_512 | SHA512_CMD_READ,
		       sha_priv->base + regs->sha512_cmd);

	reg_data_out = regs->data_out;
	for (i = 0; i < (hash->digest_len / 32); i++) {
		*out32 = readl(sha_priv->base + reg_data_out);
		out32++;
		if (hash->type == type_sha1 || hash->type == type_sha256)
			reg_data_out += 4;
	}

	return 0;
}

int npcm_sha_calc(const u8 *input, u32 len, u8 *output, u8 type)
{
	if (npcm_sha_init(type))
		return -ENOTSUPP;
	npcm_sha_reset();
	npcm_sha_enable(true);
	npcm_sha_update(input, len);
	npcm_sha_finish(output);
	npcm_sha_enable(false);

	return 0;
}

void hw_sha512(const unsigned char *input, unsigned int len,
	       unsigned char *output, unsigned int chunk_sz)
{
	if (!sha_priv->support_sha512) {
		puts(" HW accelerator not support\n");
		return;
	}
	puts(" using BMC HW accelerator\n");
	npcm_sha_calc(input, len, output, type_sha512);
}

void hw_sha384(const unsigned char *input, unsigned int len,
	       unsigned char *output, unsigned int chunk_sz)
{
	if (!sha_priv->support_sha512) {
		puts(" HW accelerator not support\n");
		return;
	}
	puts(" using BMC HW accelerator\n");
	npcm_sha_calc(input, len, output, type_sha384);
}

void hw_sha256(const unsigned char *input, unsigned int len,
	       unsigned char *output, unsigned int chunk_sz)
{
	puts(" using BMC HW accelerator\n");
	npcm_sha_calc(input, len, output, type_sha256);
}

void hw_sha1(const unsigned char *input, unsigned int len,
	     unsigned char *output, unsigned int chunk_sz)
{
	puts(" using BMC HW accelerator\n");
	npcm_sha_calc(input, len, output, type_sha1);
}

int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	if (!strcmp("sha1", algo->name)) {
		npcm_sha_init(type_sha1);
	} else if (!strcmp("sha256", algo->name)) {
		npcm_sha_init(type_sha256);
	} else if (!strcmp("sha384", algo->name)) {
		if (!sha_priv->support_sha512)
			return -ENOTSUPP;
		npcm_sha_init(type_sha384);
	} else if (!strcmp("sha512", algo->name)) {
		if (!sha_priv->support_sha512)
			return -ENOTSUPP;
		npcm_sha_init(type_sha512);
	} else {
		return -ENOTSUPP;
	}

	printf("Using npcm SHA engine\n");
	npcm_sha_reset();
	npcm_sha_enable(true);

	return 0;
}

int hw_sha_update(struct hash_algo *algo, void *ctx, const void *buf,
		  unsigned int size, int is_last)
{
	return npcm_sha_update(buf, size);
}

int hw_sha_finish(struct hash_algo *algo, void *ctx, void *dest_buf,
		  int size)
{
	int ret;

	ret = npcm_sha_finish(dest_buf);
	npcm_sha_enable(false);

	return ret;
}

static int npcm_sha_bind(struct udevice *dev)
{
	sha_priv = calloc(1, sizeof(struct npcm_sha_priv));
	if (!sha_priv)
		return -ENOMEM;

	sha_priv->base = dev_read_addr_ptr(dev);
	if (!sha_priv->base) {
		printf("Cannot find sha reg address, binding failed\n");
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_ARCH_NPCM8XX))
		sha_priv->support_sha512 = true;

	printf("SHA: NPCM SHA module bind OK\n");

	return 0;
}

static const struct udevice_id npcm_sha_ids[] = {
	{ .compatible = "nuvoton,npcm845-sha" },
	{ .compatible = "nuvoton,npcm750-sha" },
	{ }
};

U_BOOT_DRIVER(npcm_sha) = {
	.name = "npcm_sha",
	.id = UCLASS_MISC,
	.of_match = npcm_sha_ids,
	.priv_auto = sizeof(struct npcm_sha_priv),
	.bind = npcm_sha_bind,
};
