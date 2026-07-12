// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 James Hilliard
 */

#define LOG_CATEGORY UCLASS_HASH

#include <dm.h>
#include <limits.h>
#include <malloc.h>
#include <memalign.h>
#include <u-boot/hash.h>
#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>
#include <linux/kernel.h>
#include "sun8i-ce.h"

#define SUNXI_CE_HASH_MAX_BLOCK_SIZE	SHA512_BLOCK_SIZE
#define SUNXI_CE_HASH_MAX_DIGEST_SIZE	SHA512_SUM_LEN
#define SUNXI_CE_HASH_RESULT_SIZE	ALIGN(SUNXI_CE_HASH_MAX_DIGEST_SIZE, \
					      ARCH_DMA_MINALIGN)
#define SUNXI_CE_HASH_MAX_PAD_SIZE	(2 * SUNXI_CE_HASH_MAX_BLOCK_SIZE)
#define SUNXI_CE_HASH_REPACK_SIZE	(128 * 1024)
#define SUNXI_CE_HASH_DIRECT_SIZE	(64 * 1024 * 1024)
#define SUNXI_CE_HASH_IV_INPUT		BIT(16)
#define SUNXI_CE_HASH_BLOCK_SIZE		64

static_assert(IS_ALIGNED(SUNXI_CE_HASH_REPACK_SIZE,
			 SUNXI_CE_HASH_MAX_BLOCK_SIZE));
static_assert(IS_ALIGNED(SUNXI_CE_HASH_DIRECT_SIZE,
			 SUNXI_CE_HASH_MAX_BLOCK_SIZE));

struct sunxi_hash_job {
	struct sunxi_ce_task task __aligned(ARCH_DMA_MINALIGN);
	u8 pad[SUNXI_CE_HASH_MAX_PAD_SIZE] __aligned(ARCH_DMA_MINALIGN);
	u8 result[SUNXI_CE_HASH_RESULT_SIZE] __aligned(ARCH_DMA_MINALIGN);
	u8 state[SUNXI_CE_HASH_MAX_DIGEST_SIZE] __aligned(ARCH_DMA_MINALIGN);
};

struct sunxi_hash_alg {
	u8 method;
	u8 block_size;
	u8 digest_size;
	u8 state_size;
	bool little_endian_len;
	bool available;
};

static const struct sunxi_hash_alg sunxi_hash_algs[HASH_ALGO_NUM] = {
	[HASH_ALGO_MD5] = {
		.method = SUNXI_CE_METHOD_MD5,
		.block_size = SUNXI_CE_HASH_BLOCK_SIZE,
		.digest_size = MD5_SUM_LEN,
		.state_size = MD5_SUM_LEN,
		.little_endian_len = true,
		.available = !IS_ENABLED(CONFIG_XPL_BUILD) ||
			     CONFIG_IS_ENABLED(MD5),
	},
	[HASH_ALGO_SHA1] = {
		.method = SUNXI_CE_METHOD_SHA1,
		.block_size = SUNXI_CE_HASH_BLOCK_SIZE,
		.digest_size = SHA1_SUM_LEN,
		.state_size = SHA1_SUM_LEN,
		.available = !IS_ENABLED(CONFIG_XPL_BUILD) ||
			     CONFIG_IS_ENABLED(SHA1),
	},
	[HASH_ALGO_SHA256] = {
		.method = SUNXI_CE_METHOD_SHA256,
		.block_size = SUNXI_CE_HASH_BLOCK_SIZE,
		.digest_size = SHA256_SUM_LEN,
		.state_size = SHA256_SUM_LEN,
		.available = !IS_ENABLED(CONFIG_XPL_BUILD) ||
			     CONFIG_IS_ENABLED(SHA256),
	},
	[HASH_ALGO_SHA384] = {
		.method = SUNXI_CE_METHOD_SHA384,
		.block_size = SHA512_BLOCK_SIZE,
		.digest_size = SHA384_SUM_LEN,
		.state_size = SHA512_SUM_LEN,
		.available = !IS_ENABLED(CONFIG_XPL_BUILD) ||
			     CONFIG_IS_ENABLED(SHA384),
	},
	[HASH_ALGO_SHA512] = {
		.method = SUNXI_CE_METHOD_SHA512,
		.block_size = SHA512_BLOCK_SIZE,
		.digest_size = SHA512_SUM_LEN,
		.state_size = SHA512_SUM_LEN,
		.available = !IS_ENABLED(CONFIG_XPL_BUILD) ||
			     CONFIG_IS_ENABLED(SHA512),
	},
};

static const struct sunxi_hash_alg *sunxi_hash_get_alg(enum HASH_ALGO algo)
{
	if ((u32)algo >= ARRAY_SIZE(sunxi_hash_algs) ||
	    !sunxi_hash_algs[algo].available)
		return NULL;

	return &sunxi_hash_algs[algo];
}

static size_t sunxi_hash_pad(const struct sunxi_hash_alg *alg, u8 *pad,
			     const u8 *tail, size_t tail_len, size_t len)
{
	size_t block_size = alg->block_size;
	size_t rem = len % block_size;
	size_t len_size = block_size == SHA512_BLOCK_SIZE ? 16 : 8;
	size_t pad_len, len_off;
	u64 bits;

	pad_len = tail_len + (rem < block_size - len_size ?
			       block_size - rem : 2 * block_size - rem);

	memset(pad, 0, pad_len);
	if (tail_len)
		memcpy(pad, tail, tail_len);
	pad[tail_len] = 0x80;

	bits = (u64)len << 3;
	len_off = pad_len - 8;
	if (alg->little_endian_len) {
		bits = cpu_to_le64(bits);
		memcpy(pad + len_off, &bits, sizeof(bits));
	} else {
		bits = cpu_to_be64(bits);
		memcpy(pad + len_off, &bits, sizeof(bits));
	}

	return pad_len;
}

static void sunxi_hash_fill_task(struct sunxi_ce_priv *ce,
				 struct sunxi_hash_job *job,
				 dma_addr_t iv,
				 dma_addr_t src, size_t src_len,
				 dma_addr_t pad, size_t pad_len,
				 dma_addr_t result, size_t state_len)
{
	struct sunxi_ce_task *task = &job->task;
	u32 total_len = src_len + pad_len;
	u32 sg = 0;

	memset(task, 0, sizeof(*task));

	task->t_common_ctl = iv ? SUNXI_CE_HASH_IV_INPUT : 0;
	if (iv)
		task->t_iv = sunxi_ce_desc_dma_addr(ce, iv);
	task->t_dlen = total_len * 8;

	if (src_len) {
		task->t_src[sg].addr = sunxi_ce_desc_dma_addr(ce, src);
		task->t_src[sg].len = src_len / sizeof(u32);
		sg++;
	}
	if (pad_len) {
		task->t_src[sg].addr = sunxi_ce_desc_dma_addr(ce, pad);
		task->t_src[sg].len = pad_len / sizeof(u32);
	}

	task->t_dst[0].addr = sunxi_ce_desc_dma_addr(ce, result);
	task->t_dst[0].len = state_len / sizeof(u32);
}

static int sunxi_hash_run_chunk(struct sunxi_ce_priv *ce,
				struct sunxi_ce_session *session,
				struct sunxi_hash_job *job,
				u32 method,
				const void *src, size_t src_len, size_t pad_len,
				const void *iv, size_t state_len)
{
	struct sunxi_ce_dma_buf result_dma = { };
	struct sunxi_ce_dma_buf src_dma = { };
	struct sunxi_ce_dma_buf pad_dma = { };
	struct sunxi_ce_dma_buf iv_dma = { };
	int ret;

	if (!IS_ALIGNED(src_len, sizeof(u32)) ||
	    !IS_ALIGNED(pad_len, sizeof(u32)) ||
	    src_len > U32_MAX / 8 || pad_len > U32_MAX / 8 - src_len)
		return -EINVAL;

	ret = sunxi_ce_dma_map(ce, &src_dma, (void *)src, src_len,
			       DMA_TO_DEVICE);
	if (ret)
		goto out_unmap;
	ret = sunxi_ce_dma_map(ce, &pad_dma, job->pad, pad_len,
			       DMA_TO_DEVICE);
	if (ret)
		goto out_unmap;
	ret = sunxi_ce_dma_map(ce, &iv_dma, (void *)iv, iv ? state_len : 0,
			       DMA_TO_DEVICE);
	if (ret)
		goto out_unmap;
	ret = sunxi_ce_dma_map(ce, &result_dma, job->result,
			       sizeof(job->result),
			       DMA_FROM_DEVICE);
	if (ret)
		goto out_unmap;

	sunxi_hash_fill_task(ce, job, iv_dma.dma, src_dma.dma,
			     src_len, pad_dma.dma, pad_len, result_dma.dma,
			     state_len);

	ret = sunxi_ce_session_run_chain_or_close(session,
						  SUNXI_CE_CHANNEL_HASH, method,
						  &job->task, 1);

out_unmap:
	sunxi_ce_dma_unmap(&result_dma);
	sunxi_ce_dma_unmap(&iv_dma);
	sunxi_ce_dma_unmap(&pad_dma);
	sunxi_ce_dma_unmap(&src_dma);

	return ret;
}

static int sunxi_hash_run_stream(struct sunxi_ce_priv *ce,
				 struct sunxi_hash_job *job,
				 u32 method,
				 const u8 *src, size_t src_len,
				 size_t pad_len, size_t state_len)
{
	struct sunxi_ce_session session;
	const void *iv = NULL;
	const void *chunk;
	size_t chunk_size;
	u8 *repack = NULL;
	int ret;

	if (src_len && !IS_ALIGNED((uintptr_t)src, sizeof(u32))) {
		/* CE scatter-gather addresses require word-aligned chunks. */
		chunk_size = SUNXI_CE_HASH_REPACK_SIZE;
		repack = memalign(ARCH_DMA_MINALIGN, chunk_size);
		if (!repack)
			return -ENOMEM;
	} else {
		chunk_size = SUNXI_CE_HASH_DIRECT_SIZE;
	}

	ret = sunxi_ce_session_begin(ce,
				     SUNXI_CE_CHAN_MASK(SUNXI_CE_CHANNEL_HASH),
				     &session);
	if (ret)
		goto out;

	while (src_len > chunk_size) {
		chunk = src;
		if (repack) {
			memcpy(repack, src, chunk_size);
			chunk = repack;
		}
		ret = sunxi_hash_run_chunk(ce, &session, job, method, chunk,
					   chunk_size, 0, iv, state_len);
		if (ret)
			goto out_close;

		memcpy(job->state, job->result, state_len);
		iv = job->state;
		src += chunk_size;
		src_len -= chunk_size;
	}

	chunk = src;
	if (repack) {
		memcpy(repack, src, src_len);
		chunk = repack;
	}
	ret = sunxi_hash_run_chunk(ce, &session, job, method, chunk, src_len,
				   pad_len, iv, state_len);
out_close:
	ret = sunxi_ce_session_close(&session, ret);
out:
	free(repack);

	return ret;
}

static int sunxi_hash_digest(struct udevice *dev, enum HASH_ALGO hash_algo,
			     const void *ibuf, const uint32_t ilen, void *obuf)
{
	struct sunxi_ce_priv *ce = dev_get_priv(dev_get_parent(dev));
	const struct sunxi_hash_alg *alg;
	size_t src_len = ALIGN_DOWN(ilen, sizeof(u32));
	size_t tail_len = ilen - src_len;
	struct sunxi_hash_job *job;
	const u8 *tail = ibuf;
	size_t pad_len;
	u8 digest_size, state_size;
	u32 method;
	int ret;

	alg = sunxi_hash_get_alg(hash_algo);
	if (!alg)
		return -EOPNOTSUPP;
	if ((!ibuf && ilen) || !obuf)
		return -EINVAL;
	if (ilen > UINTPTR_MAX - (uintptr_t)ibuf)
		return -EOVERFLOW;
	method = alg->method;
	digest_size = alg->digest_size;
	state_size = alg->state_size;

	job = malloc_cache_aligned(sizeof(*job));
	if (!job)
		return -ENOMEM;

	if (tail_len)
		tail += src_len;

	pad_len = sunxi_hash_pad(alg, job->pad, tail, tail_len, ilen);
	ret = sunxi_hash_run_stream(ce, job, method, ibuf, src_len, pad_len,
				    state_size);
	if (ret)
		goto out;

	memcpy(obuf, job->result, digest_size);

out:
	free(job);

	return ret;
}

static int sunxi_hash_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
				const void *ibuf, const uint32_t ilen,
				void *obuf, uint32_t chunk_sz)
{
	return sunxi_hash_digest(dev, algo, ibuf, ilen, obuf);
}

static const struct hash_ops sunxi_hash_ops = {
	.hash_digest = sunxi_hash_digest,
	.hash_digest_wd = sunxi_hash_digest_wd,
};

U_BOOT_DRIVER(sun8i_ce_hash) = {
	.name = "sun8i-ce-hash",
	.id = UCLASS_HASH,
	.ops = &sunxi_hash_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
