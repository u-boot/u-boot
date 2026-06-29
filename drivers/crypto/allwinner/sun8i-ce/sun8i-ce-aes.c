// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 James Hilliard
 */

#define LOG_CATEGORY UCLASS_AES

#include <dm.h>
#include <limits.h>
#include <malloc.h>
#include <memalign.h>
#include <u-boot/schedule.h>
#include <uboot_aes.h>
#include <asm/cache.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include "sun8i-ce.h"

#define SUNXI_CE_ENCRYPTION		0
#define SUNXI_CE_DECRYPTION		BIT(8)

#define SUNXI_CE_OP_ECB			0
#define SUNXI_CE_OP_CBC			BIT(8)

#define SUNXI_CE_AES_KEY_128BIT		0
#define SUNXI_CE_AES_KEY_192BIT		1
#define SUNXI_CE_AES_KEY_256BIT		2

#define SUNXI_CE_AES_TASK_SIZE		(128 * 1024)
#define SUNXI_CE_AES_TASK_BLOCKS		\
	(SUNXI_CE_AES_TASK_SIZE / AES_BLOCK_LENGTH)
#define SUNXI_CE_AES_CHAIN_DEPTH		10
#define SUNXI_CE_AES_MAX_LANES		2
#define SUNXI_CE_AES_BANK_COUNT		2
#define SUNXI_CE_AES_DUAL_MIN_BLOCKS	\
	(SUNXI_CE_AES_MAX_LANES * SUNXI_CE_AES_TASK_BLOCKS)
#define SUNXI_CE_AES_REPACK_SIZE		(64 * 1024)
#define SUNXI_CE_AES_REPACK_BLOCKS	\
	(SUNXI_CE_AES_REPACK_SIZE / AES_BLOCK_LENGTH)
#define SUNXI_CE_AES_NO_BANK		SUNXI_CE_AES_BANK_COUNT

static_assert(SUNXI_CE_AES_BANK_COUNT == 2);

enum sunxi_aes_segment_type {
	SUNXI_AES_SEGMENT_HEAD,
	SUNXI_AES_SEGMENT_MIDDLE,
	SUNXI_AES_SEGMENT_TAIL,
	SUNXI_AES_SEGMENT_COUNT,
};

struct sunxi_aes_priv {
	u8 key[AES256_KEY_LENGTH] __aligned(sizeof(u32));
	u8 key_len;
	u8 ce_key_size;
};

struct sunxi_aes_segment {
	dma_addr_t dma;
	u32 len;
};

struct sunxi_aes_bank_map {
	u8 src_edges[2][ARCH_DMA_MINALIGN] __aligned(ARCH_DMA_MINALIGN);
	u8 dst_edges[2][ARCH_DMA_MINALIGN] __aligned(ARCH_DMA_MINALIGN);
	struct sunxi_ce_dma_buf src_edges_dma;
	struct sunxi_ce_dma_buf src_middle_dma;
	struct sunxi_ce_dma_buf dst_edges_dma;
	struct sunxi_ce_dma_buf dst_middle_dma;
	struct sunxi_aes_segment src_segments[SUNXI_AES_SEGMENT_COUNT];
	struct sunxi_aes_segment dst_segments[SUNXI_AES_SEGMENT_COUNT];
	u32 dst_head;
	u32 dst_tail;
};

struct sunxi_aes_bank {
	struct sunxi_ce_task tasks[SUNXI_CE_AES_CHAIN_DEPTH]
		__aligned(ARCH_DMA_MINALIGN);
	size_t offset;
	u32 len;
	/* Zero means free; otherwise every DMA mapping remains owned here. */
	u32 task_count;
	u8 ivs[SUNXI_CE_AES_CHAIN_DEPTH][AES_BLOCK_LENGTH]
		__aligned(ARCH_DMA_MINALIGN);
	struct sunxi_ce_dma_buf iv_dma;
	struct sunxi_aes_bank_map data;
};

struct sunxi_aes_lane {
	struct sunxi_aes_bank banks[SUNXI_CE_AES_BANK_COUNT];
	/* Submitted bank, or SUNXI_CE_AES_NO_BANK while the lane is idle. */
	u8 active_bank;
};

struct sunxi_aes_xfer {
	struct sunxi_ce_session session;
	dma_addr_t key_dma;
	u8 *src;
	u8 *dst;
	size_t cursor;
	u32 remaining;
	u32 comm_ctl;
	u32 sym_ctl;
	u8 lane_count;
	u8 next_iv[AES_BLOCK_LENGTH];
	struct sunxi_aes_lane lanes[SUNXI_CE_AES_MAX_LANES];
};

static const u8 sunxi_aes_methods[SUNXI_CE_AES_MAX_LANES] = {
	SUNXI_CE_METHOD_AES,
	SUNXI_CE_METHOD_RAES,
};

static bool sunxi_aes_is_cbc(const struct sunxi_aes_xfer *xfer)
{
	return xfer->sym_ctl & SUNXI_CE_OP_CBC;
}

static bool sunxi_aes_is_decrypt(const struct sunxi_aes_xfer *xfer)
{
	return xfer->comm_ctl & SUNXI_CE_DECRYPTION;
}

static bool sunxi_aes_is_serial(const struct sunxi_aes_xfer *xfer)
{
	return sunxi_aes_is_cbc(xfer) && !sunxi_aes_is_decrypt(xfer);
}

static u32 sunxi_aes_other_bank(u32 bank_index)
{
	return bank_index ^ 1;
}

static int sunxi_ce_key_size(u32 key_bits)
{
	switch (key_bits) {
	case AES128_KEY_LENGTH * 8:
		return SUNXI_CE_AES_KEY_128BIT;
	case AES192_KEY_LENGTH * 8:
		return SUNXI_CE_AES_KEY_192BIT;
	case AES256_KEY_LENGTH * 8:
		return SUNXI_CE_AES_KEY_256BIT;
	default:
		return -EINVAL;
	}
}

static bool sunxi_aes_ranges_overlap(const u8 *src, const u8 *dst, size_t len)
{
	uintptr_t src_start = (uintptr_t)src;
	uintptr_t dst_start = (uintptr_t)dst;

	if (src_start < dst_start)
		return dst_start - src_start < len;

	return src_start - dst_start < len;
}

static u8 sunxi_aes_lane_count(struct sunxi_ce_priv *ce, bool cbc,
			       bool decrypt, u32 num_blocks)
{
	if (ce->variant->aes_engine_count > 1 && (!cbc || decrypt) &&
	    num_blocks >= SUNXI_CE_AES_DUAL_MIN_BLOCKS)
		return SUNXI_CE_AES_MAX_LANES;

	return 1;
}

static void sunxi_aes_edge_lengths(const u8 *buf, u32 len, u32 *head,
				   u32 *middle, u32 *tail)
{
	*head = (ARCH_DMA_MINALIGN -
		 ((uintptr_t)buf & (ARCH_DMA_MINALIGN - 1))) &
		(ARCH_DMA_MINALIGN - 1);
	*head = min(*head, len);
	*middle = ALIGN_DOWN(len - *head, ARCH_DMA_MINALIGN);
	*tail = len - *head - *middle;
}

static void sunxi_aes_release_bank_data(struct sunxi_aes_bank_map *map,
					u8 *dst, u32 len,
					bool commit_output)
{
	sunxi_ce_dma_unmap(&map->dst_middle_dma);
	sunxi_ce_dma_unmap(&map->dst_edges_dma);
	sunxi_ce_dma_unmap(&map->src_middle_dma);
	sunxi_ce_dma_unmap(&map->src_edges_dma);

	if (commit_output) {
		if (map->dst_head)
			memcpy(dst, map->dst_edges[0], map->dst_head);
		if (map->dst_tail)
			memcpy(dst + len - map->dst_tail,
			       map->dst_edges[1], map->dst_tail);
	}

	memset(map, 0, sizeof(*map));
}

static int sunxi_aes_map_bank_data(struct sunxi_ce_priv *ce,
				   struct sunxi_aes_bank_map *map,
				   u8 *src, u8 *dst, u32 len)
{
	u32 src_head, src_middle, src_tail;
	u32 dst_head, dst_middle, dst_tail;
	enum dma_data_direction src_dir;
	bool in_place = src == dst;
	int ret;

	if (!IS_ALIGNED((uintptr_t)src, sizeof(u32)) ||
	    !IS_ALIGNED((uintptr_t)dst, sizeof(u32)) ||
	    !IS_ALIGNED(len, sizeof(u32)))
		return -EINVAL;

	memset(map, 0, sizeof(*map));
	src_dir = in_place ? DMA_BIDIRECTIONAL : DMA_TO_DEVICE;

	/* Independently refilled banks must not share DMA cache envelopes. */
	sunxi_aes_edge_lengths(src, len, &src_head, &src_middle, &src_tail);
	sunxi_aes_edge_lengths(dst, len, &dst_head, &dst_middle, &dst_tail);
	map->dst_head = dst_head;
	map->dst_tail = dst_tail;

	if (src_head)
		memcpy(map->src_edges[0], src, src_head);
	if (src_tail)
		memcpy(map->src_edges[1], src + len - src_tail, src_tail);
	if (src_head || src_tail) {
		ret = sunxi_ce_dma_map(ce, &map->src_edges_dma, map->src_edges,
				       sizeof(map->src_edges), DMA_TO_DEVICE);
		if (ret)
			return ret;
	}

	if (src_middle) {
		ret = sunxi_ce_dma_map(ce, &map->src_middle_dma, src + src_head,
				       src_middle, src_dir);
		if (ret)
			return ret;
	}

	if (dst_head || dst_tail) {
		ret = sunxi_ce_dma_map(ce, &map->dst_edges_dma, map->dst_edges,
				       sizeof(map->dst_edges), DMA_FROM_DEVICE);
		if (ret)
			return ret;
	}

	if (!in_place && dst_middle) {
		ret = sunxi_ce_dma_map(ce, &map->dst_middle_dma, dst + dst_head,
				       dst_middle, DMA_FROM_DEVICE);
		if (ret)
			return ret;
	}

	map->src_segments[SUNXI_AES_SEGMENT_HEAD].dma = map->src_edges_dma.dma;
	map->src_segments[SUNXI_AES_SEGMENT_HEAD].len = src_head;
	map->src_segments[SUNXI_AES_SEGMENT_MIDDLE].dma = map->src_middle_dma.dma;
	map->src_segments[SUNXI_AES_SEGMENT_MIDDLE].len = src_middle;
	map->src_segments[SUNXI_AES_SEGMENT_TAIL].dma =
		map->src_edges_dma.dma + ARCH_DMA_MINALIGN;
	map->src_segments[SUNXI_AES_SEGMENT_TAIL].len = src_tail;

	map->dst_segments[SUNXI_AES_SEGMENT_HEAD].dma = map->dst_edges_dma.dma;
	map->dst_segments[SUNXI_AES_SEGMENT_HEAD].len = dst_head;
	map->dst_segments[SUNXI_AES_SEGMENT_MIDDLE].dma =
		in_place ? map->src_middle_dma.dma : map->dst_middle_dma.dma;
	map->dst_segments[SUNXI_AES_SEGMENT_MIDDLE].len = dst_middle;
	map->dst_segments[SUNXI_AES_SEGMENT_TAIL].dma =
		map->dst_edges_dma.dma + ARCH_DMA_MINALIGN;
	map->dst_segments[SUNXI_AES_SEGMENT_TAIL].len = dst_tail;

	return 0;
}

static int sunxi_aes_fill_sg(struct sunxi_ce_priv *ce,
			     struct sunxi_ce_sginfo *sg,
			     const struct sunxi_aes_segment *segments,
			     u32 offset, u32 len)
{
	u32 end = offset + len;
	u32 covered = 0;
	u32 segment_offset = 0;
	u8 i, sg_count = 0;

	for (i = 0; i < SUNXI_AES_SEGMENT_COUNT; i++) {
		u32 segment_end = segment_offset + segments[i].len;
		u32 start = max(offset, segment_offset);
		u32 stop = min(end, segment_end);
		u32 part_len;
		dma_addr_t dma;

		if (start >= stop) {
			segment_offset = segment_end;
			continue;
		}
		if (sg_count >= SUNXI_CE_MAX_SG)
			return -EINVAL;

		part_len = stop - start;
		if (!IS_ALIGNED(part_len, sizeof(u32)))
			return -EINVAL;
		dma = segments[i].dma + start - segment_offset;
		sg[sg_count].addr = sunxi_ce_desc_dma_addr(ce, dma);
		sg[sg_count].len = part_len / sizeof(u32);
		covered += part_len;
		sg_count++;
		segment_offset = segment_end;
	}

	return covered == len ? 0 : -EINVAL;
}

static void sunxi_aes_init_task(struct sunxi_ce_priv *ce,
				struct sunxi_ce_task *task,
				dma_addr_t key, dma_addr_t iv,
				u32 len, u32 comm_ctl, u32 sym_ctl)
{
	memset(task, 0, sizeof(*task));

	task->t_common_ctl = comm_ctl;
	task->t_sym_ctl = sym_ctl;
	task->t_key = sunxi_ce_desc_dma_addr(ce, key);
	if (iv)
		task->t_iv = sunxi_ce_desc_dma_addr(ce, iv);
	task->t_dlen = len;
}

static u32 sunxi_aes_fair_blocks(struct sunxi_aes_xfer *xfer,
				 u32 lanes_left)
{
	if (!xfer->remaining)
		return 0;

	return 1 + (xfer->remaining - 1) / lanes_left;
}

static void sunxi_aes_release_bank(struct sunxi_aes_xfer *xfer,
				   struct sunxi_aes_bank *bank,
				   bool commit_output)
{
	u8 *dst = xfer->dst + bank->offset;
	u32 len = bank->len;

	sunxi_aes_release_bank_data(&bank->data, dst, len, commit_output);
	sunxi_ce_dma_unmap(&bank->iv_dma);

	if (commit_output && sunxi_aes_is_serial(xfer))
		memcpy(xfer->next_iv, dst + len - AES_BLOCK_LENGTH,
		       AES_BLOCK_LENGTH);

	bank->offset = 0;
	bank->len = 0;
	bank->task_count = 0;
}

static void sunxi_aes_release_all_banks(struct sunxi_aes_xfer *xfer)
{
	u32 bank_index, lane_index;

	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
		struct sunxi_aes_lane *lane = &xfer->lanes[lane_index];

		for (bank_index = 0; bank_index < SUNXI_CE_AES_BANK_COUNT;
		     bank_index++)
			sunxi_aes_release_bank(xfer, &lane->banks[bank_index],
					       false);
		lane->active_bank = SUNXI_CE_AES_NO_BANK;
	}
}

static int sunxi_aes_prepare_bank(struct sunxi_aes_xfer *xfer, u32 lane_index,
				  u32 bank_index, u32 max_blocks)
{
	struct sunxi_aes_lane *lane = &xfer->lanes[lane_index];
	struct sunxi_aes_bank *bank = &lane->banks[bank_index];
	struct sunxi_ce_priv *ce = xfer->session.ce;
	u32 chain_depth = sunxi_aes_is_serial(xfer) ? 1 :
			  SUNXI_CE_AES_CHAIN_DEPTH;
	u32 blocks, offset = 0;
	u32 task_index;
	int ret;

	if (!xfer->remaining)
		return 0;
	if (lane->active_bank == bank_index || bank->task_count)
		return -EBUSY;

	blocks = min(xfer->remaining, max_blocks);
	blocks = min_t(u32, blocks,
		       chain_depth * SUNXI_CE_AES_TASK_BLOCKS);
	bank->offset = xfer->cursor;
	bank->len = blocks * AES_BLOCK_LENGTH;
	bank->task_count = DIV_ROUND_UP(blocks, SUNXI_CE_AES_TASK_BLOCKS);

	if (sunxi_aes_is_cbc(xfer)) {
		for (task_index = 0; task_index < bank->task_count; task_index++) {
			u32 task_len = min_t(u32, bank->len - offset,
						SUNXI_CE_AES_TASK_SIZE);

			/* Snapshot before an in-place task can overwrite ciphertext. */
			memcpy(bank->ivs[task_index], xfer->next_iv,
			       AES_BLOCK_LENGTH);
			if (sunxi_aes_is_decrypt(xfer))
				memcpy(xfer->next_iv,
				       xfer->src + bank->offset + offset + task_len -
				       AES_BLOCK_LENGTH, AES_BLOCK_LENGTH);
			offset += task_len;
		}

		ret = sunxi_ce_dma_map(ce, &bank->iv_dma, bank->ivs,
				       bank->task_count * AES_BLOCK_LENGTH,
				       DMA_TO_DEVICE);
		if (ret)
			goto out_release;
	}

	ret = sunxi_aes_map_bank_data(ce, &bank->data,
				      xfer->src + bank->offset,
				      xfer->dst + bank->offset, bank->len);
	if (ret)
		goto out_release;

	for (task_index = 0, offset = 0; task_index < bank->task_count;
	     task_index++) {
		struct sunxi_ce_task *task = &bank->tasks[task_index];
		u32 task_len = min_t(u32, bank->len - offset,
						SUNXI_CE_AES_TASK_SIZE);
		dma_addr_t iv = sunxi_aes_is_cbc(xfer) ? bank->iv_dma.dma +
					      task_index * AES_BLOCK_LENGTH : 0;

		sunxi_aes_init_task(ce, task, xfer->key_dma, iv,
				    task_len, xfer->comm_ctl, xfer->sym_ctl);
		ret = sunxi_aes_fill_sg(ce, task->t_src,
					bank->data.src_segments,
					offset, task_len);
		if (ret)
			goto out_release;
		ret = sunxi_aes_fill_sg(ce, task->t_dst,
					bank->data.dst_segments,
					offset, task_len);
		if (ret)
			goto out_release;

		offset += task_len;
	}
	xfer->cursor += bank->len;
	xfer->remaining -= blocks;

	return 0;

out_release:
	sunxi_aes_release_bank(xfer, bank, false);

	return ret;
}

static int sunxi_aes_submit_bank(struct sunxi_aes_xfer *xfer, u32 lane_index,
				 u32 bank_index)
{
	struct sunxi_aes_lane *lane = &xfer->lanes[lane_index];
	struct sunxi_aes_bank *bank = &lane->banks[bank_index];
	int ret;

	if (!bank->task_count)
		return 0;
	if (lane->active_bank != SUNXI_CE_AES_NO_BANK)
		return -EINVAL;

	ret = sunxi_ce_session_submit_chain(&xfer->session, lane_index,
					    sunxi_aes_methods[lane_index],
					    bank->tasks, bank->task_count);
	if (!ret)
		lane->active_bank = bank_index;

	return ret;
}

static int sunxi_aes_service(struct sunxi_aes_xfer *xfer, u32 completed_mask)
{
	u8 completed_banks[SUNXI_CE_AES_MAX_LANES] = {
		SUNXI_CE_AES_NO_BANK,
		SUNXI_CE_AES_NO_BANK,
	};
	u32 lane_index, refills_left = 0;
	int ret;

	/* Snapshot completions before changing or releasing any bank state. */
	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
		struct sunxi_aes_lane *lane = &xfer->lanes[lane_index];

		if (!(completed_mask & SUNXI_CE_CHAN_MASK(lane_index)))
			continue;
		if (lane->active_bank == SUNXI_CE_AES_NO_BANK)
			return -EINVAL;

		completed_banks[lane_index] = lane->active_bank;
		lane->active_bank = SUNXI_CE_AES_NO_BANK;
		refills_left++;
	}

	/* Submit every prepared peer before doing CPU-side continuation work. */
	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
		u32 peer_bank;

		if (completed_banks[lane_index] == SUNXI_CE_AES_NO_BANK)
			continue;
		peer_bank = sunxi_aes_other_bank(completed_banks[lane_index]);
		ret = sunxi_aes_submit_bank(xfer, lane_index, peer_bank);
		if (ret)
			return ret;
	}

	/* Retire completed mappings and commit their edge cache lines. */
	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
		struct sunxi_aes_bank *bank;

		if (completed_banks[lane_index] == SUNXI_CE_AES_NO_BANK)
			continue;
		bank = &xfer->lanes[lane_index].banks[completed_banks[lane_index]];
		sunxi_aes_release_bank(xfer, bank, true);
	}

	/* Refill each newly free bank, then submit it if its lane is idle. */
	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
		struct sunxi_aes_lane *lane = &xfer->lanes[lane_index];
		u32 completed_bank = completed_banks[lane_index];

		if (completed_bank == SUNXI_CE_AES_NO_BANK)
			continue;
		if (xfer->remaining) {
			u32 max_blocks = sunxi_aes_fair_blocks(xfer,
							 refills_left);

			ret = sunxi_aes_prepare_bank(xfer, lane_index,
						     completed_bank,
						     max_blocks);
			if (ret)
				return ret;
		}
		refills_left--;
		if (lane->active_bank == SUNXI_CE_AES_NO_BANK) {
			ret = sunxi_aes_submit_bank(xfer, lane_index,
						    completed_bank);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int sunxi_aes_run_scheduler(struct sunxi_aes_xfer *xfer, u8 *src,
				   u8 *dst, u32 num_blocks)
{
	u32 bank_index, lane_index;
	int completed, ret;

	xfer->src = src;
	xfer->dst = dst;
	xfer->cursor = 0;
	xfer->remaining = num_blocks;
	for (lane_index = 0; lane_index < xfer->lane_count; lane_index++)
		xfer->lanes[lane_index].active_bank = SUNXI_CE_AES_NO_BANK;

	/*
	 * Prime bank 0 and submit it, then prepare bank 1 as its peer:
	 *
	 *   active_bank --> submitted chain
	 *   other bank  --> prepared chain, or free when task_count is zero
	 */
	for (bank_index = 0;
	     bank_index < (sunxi_aes_is_serial(xfer) ? 1 :
			   SUNXI_CE_AES_BANK_COUNT);
	     bank_index++) {
		for (lane_index = 0; lane_index < xfer->lane_count; lane_index++) {
			u32 lanes_left = xfer->lane_count - lane_index;
			u32 max_blocks;

			if (!xfer->remaining)
				break;
			max_blocks = sunxi_aes_fair_blocks(xfer, lanes_left);
			ret = sunxi_aes_prepare_bank(xfer, lane_index, bank_index,
						     max_blocks);
			if (ret)
				return ret;
			if (!bank_index) {
				ret = sunxi_aes_submit_bank(xfer, lane_index,
							    bank_index);
				if (ret)
					return ret;
			}
		}
	}

	while (sunxi_ce_session_busy(&xfer->session)) {
		completed = sunxi_ce_session_poll(&xfer->session);
		if (completed < 0)
			return completed;
		if (!completed) {
			schedule();
			continue;
		}

		ret = sunxi_aes_service(xfer, completed);
		if (ret)
			return ret;
	}

	return 0;
}

static int sunxi_aes_run(struct udevice *dev, u8 *iv, u8 *src, u8 *dst,
			 u32 num_blocks, u32 aes_mode, bool decrypt)
{
	struct sunxi_aes_priv *priv = dev_get_priv(dev);
	struct sunxi_ce_priv *ce = dev_get_priv(dev_get_parent(dev));
	struct sunxi_ce_dma_buf key_dma = { };
	struct sunxi_aes_xfer *xfer;
	u8 *repack = NULL;
	u8 lane_count;
	u32 comm_ctl, sym_ctl;
	bool cbc = aes_mode == SUNXI_CE_OP_CBC;
	bool word_addressable;
	size_t total_len;
	int ret;

	if (!priv->key_len)
		return -EINVAL;
	if (!num_blocks)
		return 0;
	if (!src || !dst)
		return -EINVAL;
	if (cbc && !iv)
		return -EINVAL;

	if (num_blocks > SIZE_MAX / AES_BLOCK_LENGTH)
		return -EOVERFLOW;
	total_len = (size_t)num_blocks * AES_BLOCK_LENGTH;
	if (total_len > UINTPTR_MAX - (uintptr_t)src ||
	    total_len > UINTPTR_MAX - (uintptr_t)dst)
		return -EOVERFLOW;
	if (src != dst && sunxi_aes_ranges_overlap(src, dst, total_len))
		return -EINVAL;

	xfer = malloc_cache_aligned(sizeof(*xfer));
	if (!xfer)
		return -ENOMEM;
	memset(xfer, 0, sizeof(*xfer));
	if (cbc)
		memcpy(xfer->next_iv, iv, AES_BLOCK_LENGTH);

	ret = sunxi_ce_dma_map(ce, &key_dma, priv->key, priv->key_len,
			       DMA_TO_DEVICE);
	if (ret)
		goto out_free;

	comm_ctl = decrypt ? SUNXI_CE_DECRYPTION : SUNXI_CE_ENCRYPTION;
	sym_ctl = priv->ce_key_size | aes_mode;
	word_addressable = IS_ALIGNED((uintptr_t)src, sizeof(u32)) &&
			   IS_ALIGNED((uintptr_t)dst, sizeof(u32));
	lane_count = word_addressable ?
		sunxi_aes_lane_count(ce, cbc, decrypt, num_blocks) : 1;
	xfer->key_dma = key_dma.dma;
	xfer->comm_ctl = comm_ctl;
	xfer->sym_ctl = sym_ctl;
	xfer->lane_count = lane_count;

	if (!word_addressable) {
		repack = memalign(ARCH_DMA_MINALIGN, SUNXI_CE_AES_REPACK_SIZE);
		if (!repack) {
			ret = -ENOMEM;
			goto out_unmap_key;
		}
	}

	ret = sunxi_ce_session_begin(ce, GENMASK(lane_count - 1, 0),
				     &xfer->session);
	if (ret)
		goto out_unmap_key;

	if (word_addressable) {
		ret = sunxi_aes_run_scheduler(xfer, src, dst, num_blocks);
		goto out_close;
	}

	while (num_blocks) {
		u32 blocks = min_t(u32, num_blocks,
				       SUNXI_CE_AES_REPACK_BLOCKS);
		u32 len = blocks * AES_BLOCK_LENGTH;

		memcpy(repack, src, len);
		ret = sunxi_aes_run_scheduler(xfer, repack, repack, blocks);
		if (ret)
			goto out_close;
		memcpy(dst, repack, len);

		num_blocks -= blocks;
		src += len;
		dst += len;
	}

	ret = 0;

out_close:
	/* Stop every lane before releasing any DMA-owned memory. */
	ret = sunxi_ce_session_close(&xfer->session, ret);
	sunxi_aes_release_all_banks(xfer);
out_unmap_key:
	sunxi_ce_dma_unmap(&key_dma);
out_free:
	free(repack);
	free(xfer);

	return ret;
}

static int sunxi_aes_available_key_slots(struct udevice *dev)
{
	return 1;
}

static int sunxi_aes_get_software_key_slot(struct udevice *dev)
{
	return 0;
}

static int sunxi_aes_select_key_slot(struct udevice *dev, u32 key_size,
				     u8 slot)
{
	struct sunxi_aes_priv *priv = dev_get_priv(dev);
	int ce_key_size;

	if (slot)
		return -EINVAL;

	ce_key_size = sunxi_ce_key_size(key_size);
	if (ce_key_size < 0)
		return ce_key_size;

	priv->key_len = key_size / 8;
	priv->ce_key_size = ce_key_size;

	return 0;
}

static int sunxi_aes_set_key_for_key_slot(struct udevice *dev, u32 key_size,
					  u8 *key, u8 slot)
{
	struct sunxi_aes_priv *priv = dev_get_priv(dev);
	int ret;

	if (!key)
		return -EINVAL;

	ret = sunxi_aes_select_key_slot(dev, key_size, slot);
	if (ret)
		return ret;

	memcpy(priv->key, key, key_size / 8);

	return 0;
}

static int sunxi_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst,
				 u32 num_blocks)
{
	return sunxi_aes_run(dev, NULL, src, dst, num_blocks,
			     SUNXI_CE_OP_ECB, false);
}

static int sunxi_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst,
				 u32 num_blocks)
{
	return sunxi_aes_run(dev, NULL, src, dst, num_blocks,
			     SUNXI_CE_OP_ECB, true);
}

static int sunxi_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src,
				 u8 *dst, u32 num_blocks)
{
	return sunxi_aes_run(dev, iv, src, dst, num_blocks,
			     SUNXI_CE_OP_CBC, false);
}

static int sunxi_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src,
				 u8 *dst, u32 num_blocks)
{
	return sunxi_aes_run(dev, iv, src, dst, num_blocks,
			     SUNXI_CE_OP_CBC, true);
}

static const struct aes_ops sunxi_aes_ops = {
	.available_key_slots = sunxi_aes_available_key_slots,
	.get_software_key_slot = sunxi_aes_get_software_key_slot,
	.select_key_slot = sunxi_aes_select_key_slot,
	.set_key_for_key_slot = sunxi_aes_set_key_for_key_slot,
	.aes_ecb_encrypt = sunxi_aes_ecb_encrypt,
	.aes_ecb_decrypt = sunxi_aes_ecb_decrypt,
	.aes_cbc_encrypt = sunxi_aes_cbc_encrypt,
	.aes_cbc_decrypt = sunxi_aes_cbc_decrypt,
};

U_BOOT_DRIVER(sun8i_ce_aes) = {
	.name = "sun8i-ce-aes",
	.id = UCLASS_AES,
	.ops = &sunxi_aes_ops,
	.priv_auto = sizeof(struct sunxi_aes_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
