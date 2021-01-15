// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */
#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/microchip-mpfs-clock.h>
#include <linux/err.h>

#include "mpfs_clk.h"

#define MPFS_PERIPH_CLOCK "mpfs_periph_clock"

#define REG_CLOCK_CONFIG_CR 0x08
#define REG_SUBBLK_CLOCK_CR 0x84
#define REG_SUBBLK_RESET_CR 0x88

#define CFG_CPU_SHIFT   0x0
#define CFG_AXI_SHIFT   0x2
#define CFG_AHB_SHIFT   0x4
#define CFG_WIDTH       0x2

/**
 * struct mpfs_periph_clock - per instance of peripheral clock
 * @id: index of a peripheral clock
 * @name: name of a peripheral clock
 * @shift: shift to a peripheral clock bit field
 * @flags: common clock framework flags
 */
struct mpfs_periph_clock {
	unsigned int id;
	const char *name;
	u8 shift;
	unsigned long flags;
};

/**
 * struct mpfs_periph_hw_clock - hardware peripheral clock
 * @periph: peripheral clock instance
 * @sys_base: base address of the mpfs system register
 * @prate: the pll clock rate
 * @hw: clock instance
 */
struct mpfs_periph_hw_clock {
	struct mpfs_periph_clock periph;
	void __iomem *sys_base;
	u32 prate;
	struct clk hw;
};

#define to_mpfs_periph_clk(_hw) container_of(_hw, struct mpfs_periph_hw_clock, hw)

static int mpfs_periph_clk_enable(struct clk *hw)
{
	struct mpfs_periph_hw_clock *periph_hw = to_mpfs_periph_clk(hw);
	struct mpfs_periph_clock *periph = &periph_hw->periph;
	void __iomem *base_addr = periph_hw->sys_base;
	u32 reg, val;

	if (periph->flags != CLK_IS_CRITICAL) {
		reg = readl(base_addr + REG_SUBBLK_RESET_CR);
		val = reg & ~(1u << periph->shift);
		writel(val, base_addr + REG_SUBBLK_RESET_CR);

		reg = readl(base_addr + REG_SUBBLK_CLOCK_CR);
		val = reg | (1u << periph->shift);
		writel(val, base_addr + REG_SUBBLK_CLOCK_CR);
	}

	return 0;
}

static int mpfs_periph_clk_disable(struct clk *hw)
{
	struct mpfs_periph_hw_clock *periph_hw = to_mpfs_periph_clk(hw);
	struct mpfs_periph_clock *periph = &periph_hw->periph;
	void __iomem *base_addr = periph_hw->sys_base;
	u32 reg, val;

	if (periph->flags != CLK_IS_CRITICAL) {
		reg = readl(base_addr + REG_SUBBLK_RESET_CR);
		val = reg | (1u << periph->shift);
		writel(val, base_addr + REG_SUBBLK_RESET_CR);

		reg = readl(base_addr + REG_SUBBLK_CLOCK_CR);
		val = reg & ~(1u << periph->shift);
		writel(val, base_addr + REG_SUBBLK_CLOCK_CR);
	}

	return 0;
}

static ulong mpfs_periph_clk_recalc_rate(struct clk *hw)
{
	struct mpfs_periph_hw_clock *periph_hw = to_mpfs_periph_clk(hw);
	void __iomem *base_addr = periph_hw->sys_base;
	unsigned long rate;
	u32 val;

	val = readl(base_addr + REG_CLOCK_CONFIG_CR) >> CFG_AHB_SHIFT;
	val &= clk_div_mask(CFG_WIDTH);
	rate = periph_hw->prate / (1u << val);
	hw->rate = rate;

	return rate;
}

#define CLK_PERIPH(_id, _name, _shift, _flags) {	\
		.periph.id = _id,			\
		.periph.name = _name,			\
		.periph.shift = _shift,			\
		.periph.flags = _flags,			\
	}

static struct mpfs_periph_hw_clock mpfs_periph_clks[] = {
	CLK_PERIPH(CLK_ENVM, "clk_periph_envm", 0, CLK_IS_CRITICAL),
	CLK_PERIPH(CLK_MAC0, "clk_periph_mac0", 1, 0),
	CLK_PERIPH(CLK_MAC1, "clk_periph_mac1", 2, 0),
	CLK_PERIPH(CLK_MMC, "clk_periph_mmc", 3, 0),
	CLK_PERIPH(CLK_TIMER, "clk_periph_timer", 4, 0),
	CLK_PERIPH(CLK_MMUART0, "clk_periph_mmuart0", 5, 0),
	CLK_PERIPH(CLK_MMUART1, "clk_periph_mmuart1", 6, 0),
	CLK_PERIPH(CLK_MMUART2, "clk_periph_mmuart2", 7, 0),
	CLK_PERIPH(CLK_MMUART3, "clk_periph_mmuart3", 8, 0),
	CLK_PERIPH(CLK_MMUART4, "clk_periph_mmuart4", 9, 0),
	CLK_PERIPH(CLK_SPI0, "clk_periph_spi0", 10, 0),
	CLK_PERIPH(CLK_SPI1, "clk_periph_spi1", 11, 0),
	CLK_PERIPH(CLK_I2C0, "clk_periph_i2c0", 12, 0),
	CLK_PERIPH(CLK_I2C1, "clk_periph_i2c1", 13, 0),
	CLK_PERIPH(CLK_CAN0, "clk_periph_can0", 14, 0),
	CLK_PERIPH(CLK_CAN1, "clk_periph_can1", 15, 0),
	CLK_PERIPH(CLK_USB, "clk_periph_usb", 16, 0),
	CLK_PERIPH(CLK_RTC, "clk_periph_rtc", 18, 0),
	CLK_PERIPH(CLK_QSPI, "clk_periph_qspi", 19, 0),
	CLK_PERIPH(CLK_GPIO0, "clk_periph_gpio0", 20, 0),
	CLK_PERIPH(CLK_GPIO1, "clk_periph_gpio1", 21, 0),
	CLK_PERIPH(CLK_GPIO2, "clk_periph_gpio2", 22, 0),
	CLK_PERIPH(CLK_DDRC, "clk_periph_ddrc", 23, CLK_IS_CRITICAL),
	CLK_PERIPH(CLK_FIC0, "clk_periph_fic0", 24, 0),
	CLK_PERIPH(CLK_FIC1, "clk_periph_fic1", 25, 0),
	CLK_PERIPH(CLK_FIC2, "clk_periph_fic2", 26, 0),
	CLK_PERIPH(CLK_FIC3, "clk_periph_fic3", 27, 0),
	CLK_PERIPH(CLK_ATHENA, "clk_periph_athena", 28, 0),
	CLK_PERIPH(CLK_CFM, "clk_periph_cfm", 29, 0),
};

int mpfs_clk_register_periphs(void __iomem *base, u32 clk_rate,
			      const char *parent_name)
{
	int ret;
	int i, id, num_clks;
	const char *name;
	struct clk *hw;

	num_clks = ARRAY_SIZE(mpfs_periph_clks);
	for (i = 0; i < num_clks; i++)  {
		hw = &mpfs_periph_clks[i].hw;
		mpfs_periph_clks[i].sys_base = base;
		mpfs_periph_clks[i].prate = clk_rate;
		name = mpfs_periph_clks[i].periph.name;
		ret = clk_register(hw, MPFS_PERIPH_CLOCK, name, parent_name);
		if (ret)
			ERR_PTR(ret);
		id = mpfs_periph_clks[i].periph.id;
		clk_dm(id, hw);
	}

	return 0;
}

const struct clk_ops mpfs_periph_clk_ops = {
	.enable = mpfs_periph_clk_enable,
	.disable = mpfs_periph_clk_disable,
	.get_rate = mpfs_periph_clk_recalc_rate,
};

U_BOOT_DRIVER(mpfs_periph_clock) = {
	.name	= MPFS_PERIPH_CLOCK,
	.id	= UCLASS_CLK,
	.ops	= &mpfs_periph_clk_ops,
};
