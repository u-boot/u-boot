// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 James Hilliard
 */

#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <errno.h>
#include <time.h>
#include <u-boot/schedule.h>
#include <vsprintf.h>
#include <asm/arch/cpu.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include "sun8i-ce.h"

#define SUNXI_CE_TDQ			0x00
#define SUNXI_CE_ICR			0x08
#define SUNXI_CE_ISR			0x0c
#define SUNXI_CE_TLR			0x10
#define SUNXI_CE_ESR			0x18
#define SUNXI_CE_SCSA			0x24
#define SUNXI_CE_SCDA			0x28
#define SUNXI_CE_HCSA			0x34
#define SUNXI_CE_HCDA			0x38
#define SUNXI_CE_ACSA			0x44
#define SUNXI_CE_ACDA			0x48
#define SUNXI_CE_XCSA			0x54
#define SUNXI_CE_XCDA			0x58

#define SUNXI_CE_ERR_ALGO_NOTSUP	BIT(0)
#define SUNXI_CE_ERR_DATALEN		BIT(1)
#define SUNXI_CE_ERR_KEYSRAM		BIT(2)
#define SUNXI_CE_ERR_ADDR_INVALID	BIT(5)
#define SUNXI_CE_ERR_KEYLADDER		BIT(6)
#define SUNXI_CE_TASK_START		BIT(0)
#define SUNXI_CE_METHOD_MASK		GENMASK(6, 0)
#define SUNXI_CE_TLR_METHOD_SHIFT	8
#define SUNXI_CE_WORD_SHIFT		2
#define SUNXI_CE_TIMEOUT_US		3000000

#define SUN50I_H6_CCU_CE_CLK		0x680
#define SUN50I_H6_CCU_CE_CLK_SRC_MASK	BIT(24)
#define SUN50I_H6_CCU_CE_CLK_N_MASK	GENMASK(9, 8)
#define SUN50I_H6_CCU_CE_CLK_M_MASK	GENMASK(3, 0)
#define SUN50I_H6_CCU_CE_CLK_GATE	BIT(31)
#define SUN50I_H6_CCU_CE_CLK_MASK	(SUN50I_H6_CCU_CE_CLK_GATE | \
					 SUN50I_H6_CCU_CE_CLK_SRC_MASK | \
					 SUN50I_H6_CCU_CE_CLK_N_MASK | \
					 SUN50I_H6_CCU_CE_CLK_M_MASK)
/* PLL_PERI0(2X) / 4 = 300 MHz */
#define SUN50I_H616_CCU_CE_CLK_M	3

static int sunxi_ce_reset(struct sunxi_ce_priv *priv);

struct sunxi_ce_channel_route {
	u16 src_reg;
	u16 dst_reg;
};

static const struct sunxi_ce_channel_route
sunxi_ce_channel_routes[SUNXI_CE_MAX_CHANS] = {
	[SUNXI_CE_CHANNEL_AES] = {
		.src_reg = SUNXI_CE_SCSA,
		.dst_reg = SUNXI_CE_SCDA,
	},
	[SUNXI_CE_CHANNEL_RAES] = {
		.src_reg = SUNXI_CE_XCSA,
		.dst_reg = SUNXI_CE_XCDA,
	},
	[SUNXI_CE_CHANNEL_HASH] = {
		.src_reg = SUNXI_CE_HCSA,
		.dst_reg = SUNXI_CE_HCDA,
	},
	[SUNXI_CE_CHANNEL_ASYM] = {
		.src_reg = SUNXI_CE_ACSA,
		.dst_reg = SUNXI_CE_ACDA,
	},
};

u32 sunxi_ce_desc_dma_addr(struct sunxi_ce_priv *priv, dma_addr_t addr)
{
	if (priv->variant->needs_word_addresses)
		addr >>= SUNXI_CE_WORD_SHIFT;

	return (u32)addr;
}

static void sunxi_ce_flush(void *buf, size_t len)
{
	ulong start = ALIGN_DOWN((ulong)buf, ARCH_DMA_MINALIGN);
	ulong end = ALIGN((ulong)buf + len, ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static int sunxi_ce_validate_dma_range(struct sunxi_ce_priv *priv,
				       dma_addr_t addr, size_t len)
{
	dma_addr_t last;

	if (!IS_ALIGNED(addr, sizeof(u32)))
		return -EINVAL;
	if (!len)
		return 0;
	if (len - 1 > (dma_addr_t)-1 - addr)
		return -EOVERFLOW;

	last = addr + len - 1;
	if (priv->variant->needs_word_addresses)
		last >>= SUNXI_CE_WORD_SHIFT;

	return last > U32_MAX ? -ERANGE : 0;
}

int sunxi_ce_dma_map(struct sunxi_ce_priv *priv,
		     struct sunxi_ce_dma_buf *map, void *buf, size_t len,
		     enum dma_data_direction dir)
{
	ulong addr, map_start, map_end;
	dma_addr_t base;
	int ret;

	if (!priv || !map || (!buf && len) || map->map_len)
		return -EINVAL;
	if (dir != DMA_TO_DEVICE && dir != DMA_FROM_DEVICE &&
	    dir != DMA_BIDIRECTIONAL)
		return -EINVAL;
	if (!len)
		return 0;
	if (dir != DMA_TO_DEVICE &&
	    (!IS_ALIGNED((ulong)buf, ARCH_DMA_MINALIGN) ||
	     !IS_ALIGNED(len, ARCH_DMA_MINALIGN)))
		return -EINVAL;

	addr = (ulong)buf;
	if (len > ULONG_MAX - addr)
		return -EOVERFLOW;
	if (addr + len > ULONG_MAX - (ARCH_DMA_MINALIGN - 1))
		return -EOVERFLOW;
	map_start = ALIGN_DOWN(addr, ARCH_DMA_MINALIGN);
	map_end = ALIGN(addr + len, ARCH_DMA_MINALIGN);

	base = dma_map_single((void *)map_start, map_end - map_start, dir);
	if (dma_mapping_error(NULL, base))
		return -EIO;

	map->base = base;
	map->dma = base + addr - map_start;
	map->map_len = map_end - map_start;
	map->dir = dir;
	ret = sunxi_ce_validate_dma_range(priv, map->dma, len);
	if (ret) {
		sunxi_ce_dma_unmap(map);
		return ret;
	}

	return 0;
}

void sunxi_ce_dma_unmap(struct sunxi_ce_dma_buf *map)
{
	if (!map || !map->map_len)
		return;

	dma_unmap_single(map->base, map->map_len, map->dir);
	memset(map, 0, sizeof(*map));
}

static void sunxi_ce_print_error(u32 err)
{
	printf("CE ERROR: %#x\n", err);
	if (err & SUNXI_CE_ERR_ALGO_NOTSUP)
		printf("CE ERROR: algorithm not supported\n");
	if (err & SUNXI_CE_ERR_DATALEN)
		printf("CE ERROR: data length error\n");
	if (err & SUNXI_CE_ERR_KEYSRAM)
		printf("CE ERROR: keysram access error for AES\n");
	if (err & SUNXI_CE_ERR_ADDR_INVALID)
		printf("CE ERROR: address invalid\n");
	if (err & SUNXI_CE_ERR_KEYLADDER)
		printf("CE ERROR: key ladder configuration error\n");
}

static int sunxi_ce_wait(void __iomem *addr, u32 mask, u32 expect)
{
	unsigned long timeout = timer_get_us() + SUNXI_CE_TIMEOUT_US;
	u32 val;

	do {
		val = readl(addr);
		if ((val & mask) == expect)
			return 0;
		schedule();
	} while (!time_after(timer_get_us(), timeout));

	val = readl(addr);
	if ((val & mask) == expect)
		return 0;

	return -ETIMEDOUT;
}

static u32 sunxi_ce_error_mask(u32 channel_mask)
{
	u32 error_mask = 0;
	u32 chan;

	for (chan = 0; chan < SUNXI_CE_MAX_CHANS; chan++) {
		if (channel_mask & SUNXI_CE_CHAN_MASK(chan))
			error_mask |= SUNXI_CE_CHAN_ERR_MASK(chan);
	}

	return error_mask;
}

static int sunxi_ce_prepare_channels(struct sunxi_ce_priv *priv,
				     u32 channel_mask)
{
	u32 error_mask, val;
	int ret;

	error_mask = sunxi_ce_error_mask(channel_mask);

	val = readl(priv->base + SUNXI_CE_ICR);
	writel(val | channel_mask, priv->base + SUNXI_CE_ICR);
	writel(channel_mask, priv->base + SUNXI_CE_ISR);
	writel(error_mask, priv->base + SUNXI_CE_ESR);
	ret = sunxi_ce_wait(priv->base + SUNXI_CE_ISR, channel_mask, 0);
	if (ret) {
		printf("%s: timeout waiting for stale interrupt\n", __func__);
		clrbits_le32(priv->base + SUNXI_CE_ICR, channel_mask);
		return ret;
	}

	return 0;
}

static int sunxi_ce_submit_task(struct sunxi_ce_priv *priv,
				dma_addr_t task_dma, u32 method)
{
	u32 load = (method << SUNXI_CE_TLR_METHOD_SHIFT) |
		   SUNXI_CE_TASK_START;
	int ret;

	ret = sunxi_ce_wait(priv->base + SUNXI_CE_TLR,
			    SUNXI_CE_TASK_START, 0);
	if (ret) {
		printf("%s: timeout waiting for task launcher\n", __func__);
		return ret;
	}

	writel(sunxi_ce_desc_dma_addr(priv, task_dma),
	       priv->base + SUNXI_CE_TDQ);
	/* Be sure all data is written before enabling the task. */
	wmb();
	writel(load, priv->base + SUNXI_CE_TLR);
	ret = sunxi_ce_wait(priv->base + SUNXI_CE_TLR,
			    SUNXI_CE_TASK_START, 0);
	if (ret)
		printf("%s: timeout registering task\n", __func__);

	return ret;
}

static int sunxi_ce_ack_channel(struct sunxi_ce_priv *priv, u32 chan)
{
	u32 channel_mask, error_mask, err;

	channel_mask = SUNXI_CE_CHAN_MASK(chan);
	writel(channel_mask, priv->base + SUNXI_CE_ISR);
	error_mask = SUNXI_CE_CHAN_ERR_MASK(chan);
	err = readl(priv->base + SUNXI_CE_ESR) & error_mask;
	writel(error_mask, priv->base + SUNXI_CE_ESR);
	if (err) {
		sunxi_ce_print_error(err >> (chan * 8));
		return -EIO;
	}

	return 0;
}

static int sunxi_ce_wait_channel_idle(struct sunxi_ce_priv *priv, u32 chan)
{
	const struct sunxi_ce_channel_route *route;
	int ret;

	if (chan >= SUNXI_CE_MAX_CHANS)
		return -EINVAL;
	route = &sunxi_ce_channel_routes[chan];

	ret = sunxi_ce_wait(priv->base + route->src_reg, ~0U, 0);
	if (!ret)
		ret = sunxi_ce_wait(priv->base + route->dst_reg, ~0U, 0);
	if (ret)
		printf("%s: timeout waiting for channel %u DMA idle\n",
		       __func__, chan);

	return ret;
}

static int sunxi_ce_wait_channels_idle(struct sunxi_ce_priv *priv,
				       u32 channel_mask)
{
	u32 chan;
	int cleanup_ret, ret = 0;

	for (chan = 0; chan < SUNXI_CE_MAX_CHANS; chan++) {
		if (!(channel_mask & SUNXI_CE_CHAN_MASK(chan)))
			continue;

		cleanup_ret = sunxi_ce_wait_channel_idle(priv, chan);
		if (cleanup_ret && !ret)
			ret = cleanup_ret;
	}

	return ret;
}

static int sunxi_ce_clear_errors(struct sunxi_ce_priv *priv, u32 channel_mask)
{
	u32 error_mask, err;

	error_mask = sunxi_ce_error_mask(channel_mask);
	err = readl(priv->base + SUNXI_CE_ESR) & error_mask;
	writel(error_mask, priv->base + SUNXI_CE_ESR);

	if (err) {
		u32 chan;

		for (chan = 0; chan < SUNXI_CE_MAX_CHANS; chan++) {
			u32 chan_err = (err >> (chan * 8)) & 0xff;

			if (chan_err)
				sunxi_ce_print_error(chan_err);
		}
		return -EIO;
	}

	return 0;
}

static int sunxi_ce_session_check(struct sunxi_ce_session *session)
{
	if (!session || !session->ce)
		return -EINVAL;
	if (session->ce->active_session != session)
		return -ECANCELED;

	return 0;
}

int sunxi_ce_session_begin(struct sunxi_ce_priv *priv, u32 channel_mask,
			   struct sunxi_ce_session *session)
{
	u32 valid_channels = GENMASK(SUNXI_CE_MAX_CHANS - 1, 0);
	int ret;

	if (!priv || !session || !channel_mask ||
	    channel_mask & ~valid_channels)
		return -EINVAL;
	if (priv->active_session)
		return -EBUSY;

	memset(session, 0, sizeof(*session));
	session->ce = priv;
	session->channel_mask = channel_mask;
	priv->active_session = session;

	ret = sunxi_ce_prepare_channels(priv, channel_mask);
	if (ret) {
		priv->active_session = NULL;
		session->ce = NULL;
	}

	return ret;
}

int sunxi_ce_session_submit_chain(struct sunxi_ce_session *session,
				  u32 channel, u32 method,
				  struct sunxi_ce_task *tasks, u32 task_count)
{
	dma_addr_t tasks_dma;
	size_t tasks_len;
	u32 channel_mask;
	u32 i;
	int ret;

	ret = sunxi_ce_session_check(session);
	if (ret)
		return ret;
	if (channel >= SUNXI_CE_MAX_CHANS || method & ~SUNXI_CE_METHOD_MASK)
		return -EINVAL;
	channel_mask = SUNXI_CE_CHAN_MASK(channel);
	if (!(session->channel_mask & channel_mask))
		return -EINVAL;
	if (session->inflight_mask & channel_mask)
		return -EBUSY;
	if (!tasks || !task_count)
		return -EINVAL;
	if (task_count > SIZE_MAX / sizeof(*tasks))
		return -EOVERFLOW;
	tasks_len = task_count * sizeof(*tasks);
	tasks_dma = virt_to_phys(tasks);
	ret = sunxi_ce_validate_dma_range(session->ce, tasks_dma, tasks_len);
	if (ret)
		return ret;

	for (i = 0; i < task_count; i++) {
		tasks[i].t_id = channel;
		tasks[i].t_common_ctl &= ~(SUNXI_CE_METHOD_MASK |
						SUNXI_CE_COMM_INT);
		tasks[i].t_common_ctl |= method;
		if (i + 1 == task_count)
			tasks[i].t_common_ctl |= SUNXI_CE_COMM_INT;
		tasks[i].next = i + 1 < task_count ?
			sunxi_ce_desc_dma_addr(session->ce,
					       tasks_dma +
					       (i + 1) * sizeof(*tasks)) : 0;
	}

	sunxi_ce_flush(tasks, tasks_len);

	ret = sunxi_ce_submit_task(session->ce, tasks_dma, method);
	if (!ret) {
		session->inflight_mask |= channel_mask;
		session->deadline[channel] = timer_get_us() + SUNXI_CE_TIMEOUT_US;
	}

	return ret;
}

int sunxi_ce_session_poll(struct sunxi_ce_session *session)
{
	unsigned long now;
	u32 channel_mask, completed, pending;
	unsigned int chan;
	int ret;

	ret = sunxi_ce_session_check(session);
	if (ret)
		return ret;

	completed = readl(session->ce->base + SUNXI_CE_ISR) &
		    session->inflight_mask;
	pending = completed;
	while (pending) {
		chan = __ffs(pending);
		channel_mask = SUNXI_CE_CHAN_MASK(chan);
		pending &= ~channel_mask;

		ret = sunxi_ce_ack_channel(session->ce, chan);
		if (ret)
			return ret;
		ret = sunxi_ce_wait_channel_idle(session->ce, chan);
		if (ret)
			return ret;
		session->inflight_mask &= ~channel_mask;
		session->deadline[chan] = 0;
	}

	if (completed) {
		/* A chain-tail completion orders every preceding output DMA. */
		rmb();
	}

	now = timer_get_us();
	pending = session->inflight_mask;
	while (pending) {
		chan = __ffs(pending);
		channel_mask = SUNXI_CE_CHAN_MASK(chan);
		pending &= ~channel_mask;
		if (!time_after(now, session->deadline[chan]))
			continue;

		/* Do not report a timeout for a completion racing this snapshot. */
		if (readl(session->ce->base + SUNXI_CE_ISR) & channel_mask)
			continue;

		printf("%s: DMA timeout on channel %u\n", __func__, chan);
		return -ETIMEDOUT;
	}

	return completed;
}

static int sunxi_ce_session_wait(struct sunxi_ce_session *session)
{
	int completed, ret;

	ret = sunxi_ce_session_check(session);
	if (ret)
		return ret;

	while (session->inflight_mask) {
		completed = sunxi_ce_session_poll(session);
		if (completed < 0)
			return completed;
		if (!completed)
			schedule();
	}

	return 0;
}

int sunxi_ce_session_run_chain_or_close(struct sunxi_ce_session *session,
					u32 channel, u32 method,
					struct sunxi_ce_task *tasks,
					u32 task_count)
{
	int ret;

	ret = sunxi_ce_session_submit_chain(session, channel, method, tasks,
					    task_count);
	if (!ret)
		ret = sunxi_ce_session_wait(session);
	if (ret)
		return sunxi_ce_session_close(session, ret);

	return 0;
}

static void sunxi_ce_session_release(struct sunxi_ce_session *session)
{
	struct sunxi_ce_priv *priv = session->ce;

	priv->active_session = NULL;
	memset(session, 0, sizeof(*session));
}

static int sunxi_ce_session_finish(struct sunxi_ce_session *session, bool abort)
{
	struct sunxi_ce_priv *priv;
	u32 channel_mask, valid_channels;
	int cleanup_ret, idle_ret, ret;

	ret = sunxi_ce_session_check(session);
	if (ret)
		return ret;

	priv = session->ce;
	channel_mask = session->channel_mask;
	valid_channels = GENMASK(SUNXI_CE_MAX_CHANS - 1, 0);
	ret = 0;

	if (!abort && session->inflight_mask) {
		ret = -EBUSY;
		abort = true;
	}

	if (!abort) {
		clrbits_le32(priv->base + SUNXI_CE_ICR, channel_mask);
		writel(channel_mask, priv->base + SUNXI_CE_ISR);
		ret = sunxi_ce_clear_errors(priv, channel_mask);

		writel(0, priv->base + SUNXI_CE_TLR);
		writel(0, priv->base + SUNXI_CE_TDQ);
		cleanup_ret = sunxi_ce_wait(priv->base + SUNXI_CE_TLR,
					    SUNXI_CE_TASK_START, 0);
		readl(priv->base + SUNXI_CE_TDQ);
		if (cleanup_ret) {
			printf("%s: timeout clearing task launcher\n", __func__);
			if (!ret)
				ret = cleanup_ret;
			abort = true;
		}
	}

	if (abort) {
		cleanup_ret = sunxi_ce_reset(priv);
		if (cleanup_ret) {
			idle_ret = sunxi_ce_wait_channels_idle(priv, channel_mask);
			if (idle_ret)
				panic_str("CE: failed to stop DMA");
			if (!ret)
				ret = cleanup_ret;
		}

		/* Reset or observed idle DMA makes releasing mapped memory safe. */
		rmb();
		clrbits_le32(priv->base + SUNXI_CE_ICR, valid_channels);
		writel(valid_channels, priv->base + SUNXI_CE_ISR);
		writel(sunxi_ce_error_mask(valid_channels),
		       priv->base + SUNXI_CE_ESR);
		writel(0, priv->base + SUNXI_CE_TLR);
		writel(0, priv->base + SUNXI_CE_TDQ);
	}

	sunxi_ce_session_release(session);

	return ret;
}

int sunxi_ce_session_close(struct sunxi_ce_session *session, int status)
{
	int cleanup_ret;

	if (!session || !session->ce)
		return status ? status : -EINVAL;

	cleanup_ret = sunxi_ce_session_finish(session, status != 0);

	return status ? status : cleanup_ret;
}

int sunxi_ce_run_task(struct sunxi_ce_priv *priv, u32 channel, u32 method,
		      struct sunxi_ce_task *task)
{
	struct sunxi_ce_session session;
	u32 channel_mask;
	int ret;

	if (channel >= SUNXI_CE_MAX_CHANS)
		return -EINVAL;
	channel_mask = SUNXI_CE_CHAN_MASK(channel);

	ret = sunxi_ce_session_begin(priv, channel_mask, &session);
	if (ret)
		return ret;

	ret = sunxi_ce_session_run_chain_or_close(&session, channel, method,
						  task, 1);
	if (ret)
		return ret;

	return sunxi_ce_session_close(&session, 0);
}

static int sunxi_ce_reset(struct sunxi_ce_priv *priv)
{
	int ret;

	ret = reset_assert_bulk(&priv->resets);
	if (ret)
		return ret;

	udelay(1);

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	udelay(10);

	return 0;
}

static void sunxi_ce_setup_mod_clock(const struct sunxi_ce_variant *variant)
{
	void __iomem *ccu = (void __iomem *)SUNXI_CCM_BASE;

	clrsetbits_le32(ccu + SUN50I_H6_CCU_CE_CLK,
			SUN50I_H6_CCU_CE_CLK_MASK, variant->mod_clk_cfg);
}

static int sunxi_ce_bind_child(struct udevice *dev, const char *name)
{
	return device_bind_driver(dev, name, name, NULL);
}

static int sunxi_ce_bind(struct udevice *dev)
{
	int ret;

	if (CONFIG_IS_ENABLED(SUNXI_CE_AES)) {
		ret = sunxi_ce_bind_child(dev, "sun8i-ce-aes");
		if (ret)
			return ret;
	}

	if (CONFIG_IS_ENABLED(SUNXI_CE_ECDSA)) {
		ret = sunxi_ce_bind_child(dev, "sun8i-ce-ecdsa");
		if (ret)
			return ret;
	}

	return 0;
}

static int sunxi_ce_probe(struct udevice *dev)
{
	struct sunxi_ce_priv *priv = dev_get_priv(dev);
	int ret;

	priv->variant = (const struct sunxi_ce_variant *)
			dev_get_driver_data(dev);
	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret) {
		dev_err(dev, "failed to get resets: %d\n", ret);
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks: %d\n", ret);
		goto err_release_resets;
	}

	sunxi_ce_setup_mod_clock(priv->variant);

	ret = reset_deassert_bulk(&priv->resets);
	if (ret) {
		dev_err(dev, "failed to deassert resets: %d\n", ret);
		goto err_release_clks;
	}

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		goto err_assert_resets;
	}

	ret = sunxi_ce_reset(priv);
	if (ret) {
		dev_err(dev, "failed to reset CE: %d\n", ret);
		goto err_disable_clks;
	}

	return 0;

err_disable_clks:
	clk_disable_bulk(&priv->clks);
err_assert_resets:
	reset_assert_bulk(&priv->resets);
err_release_clks:
	clk_release_bulk(&priv->clks);
err_release_resets:
	reset_release_bulk(&priv->resets);

	return ret;
}

static int sunxi_ce_remove(struct udevice *dev)
{
	struct sunxi_ce_priv *priv = dev_get_priv(dev);

	clk_disable_bulk(&priv->clks);
	clk_release_bulk(&priv->clks);
	reset_assert_bulk(&priv->resets);
	reset_release_bulk(&priv->resets);

	return 0;
}

static const struct sunxi_ce_variant sun50i_h6_variant = {
	.aes_engine_count = 2,
};

static const struct sunxi_ce_variant sun50i_h616_variant = {
	.needs_word_addresses = true,
	.aes_engine_count = 2,
	.mod_clk_cfg = SUN50I_H6_CCU_CE_CLK_SRC_MASK |
		       SUN50I_H616_CCU_CE_CLK_M,
};

static const struct udevice_id sunxi_ce_ids[] = {
	{
		.compatible = "allwinner,sun50i-h6-crypto",
		.data = (ulong)&sun50i_h6_variant,
	}, {
		.compatible = "allwinner,sun50i-h616-crypto",
		.data = (ulong)&sun50i_h616_variant,
	},
	{ }
};

U_BOOT_DRIVER(sun8i_ce) = {
	.name = "sun8i-ce",
	.id = UCLASS_NOP,
	.of_match = sunxi_ce_ids,
	.bind = sunxi_ce_bind,
	.probe = sunxi_ce_probe,
	.remove = sunxi_ce_remove,
	.priv_auto = sizeof(struct sunxi_ce_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
