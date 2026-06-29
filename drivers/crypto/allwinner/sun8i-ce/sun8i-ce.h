/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 James Hilliard
 */

#ifndef __SUN8I_CE_H
#define __SUN8I_CE_H

#include <stddef.h>
#include <clk.h>
#include <reset.h>
#include <linux/bitops.h>
#include <linux/dma-direction.h>
#include <linux/types.h>

#define SUNXI_CE_CHANNEL_AES		0
#define SUNXI_CE_CHANNEL_RAES		1
#define SUNXI_CE_CHANNEL_HASH		2
#define SUNXI_CE_CHANNEL_ASYM		3
#define SUNXI_CE_CHAN_MASK(x)		BIT(x)
#define SUNXI_CE_COMM_INT		BIT(31)
#define SUNXI_CE_METHOD_AES		0
#define SUNXI_CE_METHOD_RAES		0x30
#define SUNXI_CE_METHOD_ECC		33
#define SUNXI_CE_ECC_OP_VERIFY		7
#define SUNXI_CE_ECC_OP_SHIFT		16
#define SUNXI_CE_MAX_SG			8
#define SUNXI_CE_MAX_CHANS		4
#define SUNXI_CE_CHAN_ERR_MASK(x)	(0xffU << ((x) * 8))

struct sunxi_ce_sginfo {
	u32 addr;
	u32 len;
};

struct sunxi_ce_task {
	u32 t_id;
	u32 t_common_ctl;
	u32 t_sym_ctl;
	u32 t_asym_ctl;
	u32 t_key;
	u32 t_iv;
	u32 t_ctr;
	u32 t_dlen;
	struct sunxi_ce_sginfo t_src[SUNXI_CE_MAX_SG];
	struct sunxi_ce_sginfo t_dst[SUNXI_CE_MAX_SG];
	u32 next;
	u32 reserved[3];
};

static_assert(sizeof(struct sunxi_ce_sginfo) == 8);
static_assert(offsetof(struct sunxi_ce_task, next) == 160);
static_assert(sizeof(struct sunxi_ce_task) == 176);

struct sunxi_ce_variant {
	bool needs_word_addresses;
	u8 aes_engine_count;
	u32 mod_clk_cfg;
};

struct sunxi_ce_session;

struct sunxi_ce_priv {
	void __iomem *base;
	const struct sunxi_ce_variant *variant;
	struct clk_bulk clks;
	struct reset_ctl_bulk resets;
	struct sunxi_ce_session *active_session;
};

struct sunxi_ce_session {
	struct sunxi_ce_priv *ce;
	u32 channel_mask;
	u32 inflight_mask;
	unsigned long deadline[SUNXI_CE_MAX_CHANS];
};

static inline bool sunxi_ce_session_busy(const struct sunxi_ce_session *session)
{
	return session && session->inflight_mask;
}

/*
 * DMA_FROM_DEVICE and DMA_BIDIRECTIONAL buffers must cover complete cache
 * lines. DMA_TO_DEVICE buffers may use a rounded cache envelope. Keep every
 * mapping alive until its chain completes or the owning session is closed.
 */
struct sunxi_ce_dma_buf {
	dma_addr_t base;
	dma_addr_t dma;
	size_t map_len;
	enum dma_data_direction dir;
};

u32 sunxi_ce_desc_dma_addr(struct sunxi_ce_priv *priv, dma_addr_t addr);
int sunxi_ce_dma_map(struct sunxi_ce_priv *priv,
		     struct sunxi_ce_dma_buf *map, void *buf, size_t len,
		     enum dma_data_direction dir);
void sunxi_ce_dma_unmap(struct sunxi_ce_dma_buf *map);
/*
 * A session owns at most one descriptor chain per selected channel. Submission
 * supplies the descriptor transport fields, including the tail interrupt.
 * Polling returns a completed-channel mask (or a negative error) after output
 * DMA is idle and ordered. A nonzero close status aborts all active channels
 * before callers release their DMA mappings. A blocking run also closes the
 * session on error so its caller can immediately unwind local DMA mappings.
 */
int sunxi_ce_session_begin(struct sunxi_ce_priv *priv, u32 channel_mask,
			   struct sunxi_ce_session *session);
int sunxi_ce_session_submit_chain(struct sunxi_ce_session *session,
				  u32 channel, u32 method,
				  struct sunxi_ce_task *tasks,
				  u32 task_count);
int sunxi_ce_session_poll(struct sunxi_ce_session *session);
int sunxi_ce_session_run_chain_or_close(struct sunxi_ce_session *session,
					u32 channel, u32 method,
					struct sunxi_ce_task *tasks,
					u32 task_count);
int sunxi_ce_session_close(struct sunxi_ce_session *session, int status);
int sunxi_ce_run_task(struct sunxi_ce_priv *priv, u32 channel, u32 method,
		      struct sunxi_ce_task *task);

#endif
