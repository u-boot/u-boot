// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#ifndef _ASM_ARCH_CCU_H
#define _ASM_ARCH_CCU_H

/**
 * enum ccu_flags - ccu clock flags
 *
 * @CCU_CLK_F_IS_VALID:		is given clock gate is valid?
 */
enum ccu_flags {
	CCU_CLK_F_IS_VALID		= BIT(0),
};

/**
 * struct ccu_clk_gate - ccu clock gate
 * @off:	gate offset
 * @bit:	gate bit
 * @flags:	ccu clock gate flags
 */
struct ccu_clk_gate {
	u16 off;
	u32 bit;
	enum ccu_flags flags;
};

#define GATE(_off, _bit) {			\
	.off = _off,				\
	.bit = _bit,				\
	.flags = CCU_CLK_F_IS_VALID,		\
}

/**
 * struct ccu_desc - clock control unit descriptor
 *
 * @gates:	clock gates
 */
struct ccu_desc {
	const struct ccu_clk_gate *gates;
};

/**
 * struct ccu_priv - sunxi clock control unit
 *
 * @base:	base address
 * @desc:	ccu descriptor
 */
struct ccu_priv {
	void *base;
	const struct ccu_desc *desc;
};

/**
 * sunxi_clk_probe - common sunxi clock probe
 * @dev:	clock device
 */
int sunxi_clk_probe(struct udevice *dev);

extern struct clk_ops sunxi_clk_ops;

#endif /* _ASM_ARCH_CCU_H */
