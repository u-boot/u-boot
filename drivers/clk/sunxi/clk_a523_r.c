// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2024 Arm Ltd.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun55i-a523-r-ccu.h>
#include <dt-bindings/reset/sun55i-a523-r-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate a523_r_gates[] = {
	[CLK_R_AHB]             = GATE_DUMMY,
	[CLK_R_APB0]            = GATE_DUMMY,
	[CLK_R_APB1]            = GATE_DUMMY,
	[CLK_BUS_R_TWD]         = GATE(0x12c, BIT(0)),
	[CLK_BUS_R_I2C0]        = GATE(0x19c, BIT(0)),
	[CLK_BUS_R_I2C1]        = GATE(0x19c, BIT(1)),
	[CLK_BUS_R_I2C2]        = GATE(0x19c, BIT(2)),
	[CLK_BUS_R_RTC]         = GATE(0x20c, BIT(0)),
};

static struct ccu_reset a523_r_resets[] = {
	[RST_BUS_R_TWD]         = RESET(0x12c, BIT(16)),
	[RST_BUS_R_UART0]       = RESET(0x18c, BIT(16)),
	[RST_BUS_R_I2C0]        = RESET(0x19c, BIT(16)),
	[RST_BUS_R_I2C1]        = RESET(0x19c, BIT(17)),
	[RST_BUS_R_I2C2]        = RESET(0x19c, BIT(18)),
	[RST_BUS_R_PPU1]        = RESET(0x1ac, BIT(17)),
	[RST_BUS_R_RTC]         = RESET(0x20c, BIT(16)),
};

const struct ccu_desc a523_r_ccu_desc = {
	.gates = a523_r_gates,
	.resets = a523_r_resets,
	.num_gates = ARRAY_SIZE(a523_r_gates),
	.num_resets = ARRAY_SIZE(a523_r_resets),
};
