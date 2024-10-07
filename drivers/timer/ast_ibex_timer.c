// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Aspeed Technology Inc.
 */

#include <asm/csr.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>

#define CSR_MCYCLE	0xb00
#define CSR_MCYCLEH	0xb80

static u64 ast_ibex_timer_get_count(struct udevice *dev)
{
	uint32_t cnt_l, cnt_h;

	cnt_l = csr_read(CSR_MCYCLE);
	cnt_h = csr_read(CSR_MCYCLEH);

	return ((uint64_t)cnt_h << 32) | cnt_l;
}

static int ast_ibex_timer_probe(struct udevice *dev)
{
	return 0;
}

static const struct timer_ops ast_ibex_timer_ops = {
	.get_count = ast_ibex_timer_get_count,
};

static const struct udevice_id ast_ibex_timer_ids[] = {
	{ .compatible = "aspeed,ast2700-ibex-timer" },
	{ }
};

U_BOOT_DRIVER(ast_ibex_timer) = {
	.name = "ast_ibex_timer",
	.id = UCLASS_TIMER,
	.of_match = ast_ibex_timer_ids,
	.probe = ast_ibex_timer_probe,
	.ops = &ast_ibex_timer_ops,
};
