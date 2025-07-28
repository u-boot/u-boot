// SPDX-License-Identifier: GPL-2.0+
/*
 * sun55i A523/A527/T527/H728 platform DRAM controller driver
 *
 * This driver supports DDR3 and LPDDR4 memory.
 *
 * (C) Copyright 2024 Jernej Skrabec <jernej.skrabec@gmail.com>
 *
 */
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <asm/arch/prcm.h>
#include <linux/bitops.h>
#include <linux/delay.h>

static void mctl_sys_init(u32 clk_rate)
{
	void * const ccm = (void *)SUNXI_CCM_BASE;

	/* Put all DRAM-related blocks to reset state */
	clrbits_le32(ccm + CCU_H6_MBUS_CFG, MBUS_ENABLE);
	clrbits_le32(ccm + CCU_H6_MBUS_CFG, MBUS_RESET);
	setbits_le32(ccm + CCU_H6_MBUS_CFG, MBUS_UPDATE);
	clrbits_le32(ccm + CCU_H6_DRAM_GATE_RESET, BIT(GATE_SHIFT));
	udelay(5);
	clrbits_le32(ccm + CCU_H6_DRAM_GATE_RESET, BIT(RESET_SHIFT));
	clrbits_le32(ccm + CCU_H6_PLL5_CFG, CCM_PLL_CTRL_EN);
	clrsetbits_le32(ccm + CCU_H6_DRAM_CLK_CFG,
			DRAM_CLK_ENABLE, DRAM_CLK_UPDATE);

	udelay(5);

	/* Set PLL5 rate to doubled DRAM clock rate */
	writel(CCM_PLL_CTRL_EN | CCM_PLL_LDO_EN | CCM_PLL_LOCK_EN |
	       CCM_PLL_OUT_EN | CCM_PLL5_CTRL_N(clk_rate * 2 / 24),
	       ccm + CCU_H6_PLL5_CFG);
	mctl_await_completion(ccm + CCU_H6_PLL5_CFG,
			      CCM_PLL_LOCK, CCM_PLL_LOCK);

	/* Configure DRAM mod clock */
	writel(DRAM_CLK_SRC_PLL5, ccm + CCU_H6_DRAM_CLK_CFG);
	writel(BIT(RESET_SHIFT), ccm + CCU_H6_DRAM_GATE_RESET);
	udelay(5);
	setbits_le32(ccm + CCU_H6_DRAM_GATE_RESET, BIT(GATE_SHIFT));

	/* Configure MBUS and enable DRAM clock */
	setbits_le32(ccm + CCU_H6_MBUS_CFG, MBUS_RESET | MBUS_UPDATE);
	setbits_le32(ccm + CCU_H6_MBUS_CFG, MBUS_ENABLE | MBUS_UPDATE);

	clrsetbits_le32(ccm + CCU_H6_DRAM_CLK_CFG, DRAM_CLK_M_MASK,
			DRAM_CLK_ENABLE | DRAM_CLK_UPDATE | DRAM_CLK_M(4));
	udelay(5);
}

static void mctl_set_addrmap(const struct dram_config *config)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u8 cols = config->cols;
	u8 rows = config->rows;
	u8 ranks = config->ranks;

	if (!config->bus_full_width)
		cols -= 1;

	/* Ranks */
	if (ranks == 2)
		mctl_ctl->addrmap[0] = 0x1F00 | (rows + cols - 3);
	else
		mctl_ctl->addrmap[0] = 0x1F1F;

	/* Banks, hardcoded to 8 banks now */
	mctl_ctl->addrmap[1] = (cols - 2) | (cols - 2) << 8 | (cols - 2) << 16;

	/* Columns */
	mctl_ctl->addrmap[2] = 0;
	switch (cols) {
	case 7:
		mctl_ctl->addrmap[3] = 0x1F1F1F00;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 8:
		mctl_ctl->addrmap[3] = 0x1F1F0000;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 9:
		mctl_ctl->addrmap[3] = 0x1F000000;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 10:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 11:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0x1F00;
		break;
	case 12:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0;
		break;
	default:
		panic("Unsupported DRAM configuration: column number invalid\n");
	}

	/* Rows */
	mctl_ctl->addrmap[5] = (cols - 3) | ((cols - 3) << 8) |
			       ((cols - 3) << 16) | ((cols - 3) << 24);
	switch (rows) {
	case 13:
		mctl_ctl->addrmap[6] = (cols - 3) | 0x0F0F0F00;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 14:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) |
				       0x0F0F0000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 15:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) |
				       ((cols - 3) << 16) | 0x0F000000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 16:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) |
				       ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 17:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) |
				       ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | 0x0F00;
		break;
	case 18:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) |
				       ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | ((cols - 3) << 8);
		break;
	default:
		panic("Unsupported DRAM configuration: row number invalid\n");
	}

	/* Bank groups, DDR4 only */
	mctl_ctl->addrmap[8] = 0x3F3F;
}

#define MASK_BYTE(reg, nr) (((reg) >> ((nr) * 8)) & 0x1f)
static void mctl_phy_configure_odt(const struct dram_para *para)
{
	u32 val_lo, val_hi;

	val_hi = para->dx_dri;
	val_lo = (para->type != SUNXI_DRAM_TYPE_LPDDR4) ? para->dx_dri :
		 (para->tpr1 & 0x1f1f1f1f) ? para->tpr1 : 0x04040404;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x304, 0x1f1f0000,
			(MASK_BYTE(val_hi, 0) << 24) |
			(MASK_BYTE(val_lo, 0) << 16));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x484, 0x1f1f0000,
			(MASK_BYTE(val_hi, 1) << 24) |
			(MASK_BYTE(val_lo, 1) << 16));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x604, 0x1f1f0000,
			(MASK_BYTE(val_hi, 2) << 24) |
			(MASK_BYTE(val_lo, 2) << 16));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x784, 0x1f1f0000,
			(MASK_BYTE(val_hi, 3) << 24) |
			(MASK_BYTE(val_lo, 3) << 16));

	val_lo = para->ca_dri;
	val_hi = para->ca_dri;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xf4, 0x1f1f1f1f,
			(MASK_BYTE(val_hi, 0) << 24) |
			(MASK_BYTE(val_lo, 0) << 16) |
			(MASK_BYTE(val_hi, 1) << 8) |
			(MASK_BYTE(val_lo, 1)));

	val_hi = para->dx_odt;
	val_lo = (para->type == SUNXI_DRAM_TYPE_LPDDR4) ? 0 : para->dx_odt;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x304, 0x00001f1f,
			(MASK_BYTE(val_hi, 0) << 8) | MASK_BYTE(val_lo, 0));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x484, 0x00001f1f,
			(MASK_BYTE(val_hi, 1) << 8) | MASK_BYTE(val_lo, 1));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x604, 0x00001f1f,
			(MASK_BYTE(val_hi, 2) << 8) | MASK_BYTE(val_lo, 2));
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x784, 0x00001f1f,
			(MASK_BYTE(val_hi, 3) << 8) | MASK_BYTE(val_lo, 3));
}

static bool mctl_phy_write_leveling(const struct dram_para *para,
				    const struct dram_config *config)
{
	u32 mr2, low, high, val = 0;
	bool result = true;

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0xf00, 0xe00);

	if (para->type == SUNXI_DRAM_TYPE_LPDDR4) {
		if (config->clk <= 936)
			mr2 = 0x1b;
		else if (config->clk <= 1200)
			mr2 = 0x2d;
		else
			mr2 = 0x36;
		writeb(mr2, SUNXI_DRAM_PHY0_BASE + 3);
	}

	low = readw(SUNXI_DRAM_PHY0_BASE + 2) | 4;
	high = readw(SUNXI_DRAM_PHY0_BASE + 4);
	writew(low, SUNXI_DRAM_PHY0_BASE + 2);
	writew(high, SUNXI_DRAM_PHY0_BASE + 4);

	if (config->bus_full_width)
		val = 0xf;
	else
		val = 3;

	mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x62), val, val);

	low = readw(SUNXI_DRAM_PHY0_BASE + 2) & 0xfffb;
	high = readw(SUNXI_DRAM_PHY0_BASE + 4);
	writew(low, SUNXI_DRAM_PHY0_BASE + 2);
	writew(high, SUNXI_DRAM_PHY0_BASE + 4);

	val = readl(SUNXI_DRAM_PHY0_BASE + 0x96);
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0x97); //TODO: ???
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0xc6);
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0xc7); //TODO: ???
	if (val == 0 || val == 0x3f)
		result = false;

	low = readw(SUNXI_DRAM_PHY0_BASE + 2) & 0xff3f;
	high = readw(SUNXI_DRAM_PHY0_BASE + 4);
	writew(low, SUNXI_DRAM_PHY0_BASE + 2);
	writew(high, SUNXI_DRAM_PHY0_BASE + 4);

	if (config->ranks == 2) {
		low = (readw(SUNXI_DRAM_PHY0_BASE + 2) & 0xff3f) | 0x40;
		high = readw(SUNXI_DRAM_PHY0_BASE + 4);
		writew(low, SUNXI_DRAM_PHY0_BASE + 2);
		writew(high, SUNXI_DRAM_PHY0_BASE + 4);

		low = readw(SUNXI_DRAM_PHY0_BASE + 2) | 4;
		high = readw(SUNXI_DRAM_PHY0_BASE + 4);
		writew(low, SUNXI_DRAM_PHY0_BASE + 2);
		writew(high, SUNXI_DRAM_PHY0_BASE + 4);

		if (config->bus_full_width)
			val = 0xf;
		else
			val = 3;

		mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x62), val, val);

		low = readw(SUNXI_DRAM_PHY0_BASE + 2) & 0xfffb;
		high = readw(SUNXI_DRAM_PHY0_BASE + 4);
		writew(low, SUNXI_DRAM_PHY0_BASE + 2);
		writew(high, SUNXI_DRAM_PHY0_BASE + 4);
	}

	low = readw(SUNXI_DRAM_PHY0_BASE + 2) & 0xff3f;
	high = readw(SUNXI_DRAM_PHY0_BASE + 4);
	writew(low, SUNXI_DRAM_PHY0_BASE + 2);
	writew(high, SUNXI_DRAM_PHY0_BASE + 4);

	return result;
}

static bool mctl_phy_read_calibration(const struct dram_para *para,
				      const struct dram_config *config)
{
	bool result = true;
	u32 val;

	if (para->type == SUNXI_DRAM_TYPE_LPDDR4)
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x44, 0x20000000);

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x3c, 0x38);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 1);

	if (config->bus_full_width)
		val = 0xf;
	else
		val = 3;

	while ((readl(SUNXI_DRAM_PHY0_BASE + 0x20c) & val) != val) {
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x20c) & 0x20) {
			result = false;
			break;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 1);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x3c);

	if (config->ranks == 2) {
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x3c, 0x34);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 1);

		while ((readl(SUNXI_DRAM_PHY0_BASE + 0x20c) & val) != val) {
			if (readl(SUNXI_DRAM_PHY0_BASE + 0x20c) & 0x20) {
				result = false;
				break;
			}
		}

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 1);
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x3c);
	}

	return result;
}

static bool mctl_phy_read_training(const struct dram_para *para,
				   const struct dram_config *config)
{
	u32 val1, val2, *ptr1, *ptr2;
	bool result = true;
	int i;

	if (para->type == SUNXI_DRAM_TYPE_LPDDR4) {
		writel(0, SUNXI_DRAM_PHY0_BASE + 0x200);
		writeb(0, SUNXI_DRAM_PHY0_BASE + 0x207);
		writeb(0, SUNXI_DRAM_PHY0_BASE + 0x208);
		writeb(0, SUNXI_DRAM_PHY0_BASE + 0x209);
		writeb(0, SUNXI_DRAM_PHY0_BASE + 0x20a);
	}

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3, 2);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x804, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x808, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa04, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa08, 0x3f, 0xf);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 6);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 1);

	mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x840), 0xc, 0xc);
	if (readl(SUNXI_DRAM_PHY0_BASE + 0x840) & 3)
		result = false;

	if (config->bus_full_width) {
		mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0xa40), 0xc, 0xc);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0xa40) & 3)
			result = false;
	}

	ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x898);
	ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x850);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}
	ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x8bc);
	ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x874);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}

	if (config->bus_full_width) {
		ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xa98);
		ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xa50);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}

		ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xabc);
		ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xa74);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 3);

	if (config->ranks == 2) {
		/* maybe last parameter should be 1? */
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3, 2);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 6);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 1);

		mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x840), 0xc, 0xc);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x840) & 3)
			result = false;

		if (config->bus_full_width) {
			mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0xa40), 0xc, 0xc);
			if (readl(SUNXI_DRAM_PHY0_BASE + 0xa40) & 3)
				result = false;
		}

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 3);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3);

	return result;
}

static bool mctl_phy_write_training(const struct dram_config *config)
{
	u32 val1, val2, *ptr1, *ptr2;
	bool result = true;
	int i;

	writel(0, SUNXI_DRAM_PHY0_BASE + 0x134);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x138);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x19c);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x1a0);

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 0xc, 8);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x10);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x20);

	mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x8e0), 3, 3);
	if (readl(SUNXI_DRAM_PHY0_BASE + 0x8e0) & 0xc)
		result = false;

	if (config->bus_full_width) {
		mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0xae0), 3, 3);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0xae0) & 0xc)
			result = false;
	}

	ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x938);
	ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x8f0);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}
	ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x95c);
	ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0x914);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}

	if (config->bus_full_width) {
		ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xb38);
		ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xaf0);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
		ptr1 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xb5c);
		ptr2 = (u32 *)(SUNXI_DRAM_PHY0_BASE + 0xb14);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x60);

	if (config->ranks == 2) {
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 0xc, 4);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x10);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x20);

		mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0x8e0), 3, 3);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x8e0) & 0xc)
			result = false;

		if (config->bus_full_width) {
			mctl_await_completion((u32 *)(SUNXI_DRAM_PHY0_BASE + 0xae0), 3, 3);
			if (readl(SUNXI_DRAM_PHY0_BASE + 0xae0) & 0xc)
				result = false;
		}

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x60);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 0xc);

	return result;
}

static void mctl_phy_bit_delay_compensation(const struct dram_para *para,
					    const struct dram_config *config)
{
	u8 array0[32], array1[32];
	u32 tmp;
	int i;

	for (i = 0; i < 32; i++) {
		array0[i] = (config->tpr11 >> (i & 0xf8)) & 0xff;
		array1[i] = (config->tpr12 >> (i & 0xf8)) & 0x7f;
	}

	if (para->tpr10 & TPR10_DX_BIT_DELAY1) {
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa0, 3);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x80);
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x44, BIT(28));

		writel(array0[0], SUNXI_DRAM_PHY0_BASE + 0x320);
		writel((array0[0] << 24) | (array0[1] << 16) |
		       (array0[2] << 8) |
		       array0[3], SUNXI_DRAM_PHY0_BASE + 0x324);
		writel((array0[4] << 24) | (array0[5] << 16) |
		       (array0[6] << 8) |
		       array0[7], SUNXI_DRAM_PHY0_BASE + 0x328);

		writel(array0[0], SUNXI_DRAM_PHY0_BASE + 0x340);
		writel((array0[0] << 24) | (array0[1] << 16) |
		       (array0[2] << 8) |
		       array0[3], SUNXI_DRAM_PHY0_BASE + 0x344);
		writel((array0[4] << 24) | (array0[5] << 16) |
		       (array0[6] << 8) |
		       array0[7], SUNXI_DRAM_PHY0_BASE + 0x348);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x40c, 0xff00,
				array0[0] << 8);
		writel((array0[0] << 24) | (array0[1] << 16) |
		       (array0[2] << 8) | array0[3],
		       SUNXI_DRAM_PHY0_BASE + 0x400);
		writel((array0[4] << 24) | (array0[5] << 16) |
		       (array0[6] << 8) | array0[7],
		       SUNXI_DRAM_PHY0_BASE + 0x404);

		writel(array0[0], SUNXI_DRAM_PHY0_BASE + 0x41c);
		writel((array0[0] << 24) | (array0[1] << 16) |
		       (array0[2] << 8) | array0[3],
		       SUNXI_DRAM_PHY0_BASE + 0x420);
		writel((array0[4] << 24) | (array0[5] << 16) |
		       (array0[6] << 8) | array0[7],
		       SUNXI_DRAM_PHY0_BASE + 0x424);

		tmp = config->odt_en & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x32c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x34c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x408);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x428);

		writel(array0[8], SUNXI_DRAM_PHY0_BASE + 0x4a0);
		writel((array0[8] << 24) | (array0[9] << 16) |
		       (array0[10] << 8) | array0[11],
		       SUNXI_DRAM_PHY0_BASE + 0x4a4);
		writel((array0[12] << 24) | (array0[13] << 16) |
		       (array0[14] << 8) | array0[15],
		       SUNXI_DRAM_PHY0_BASE + 0x4a8);

		writel(array0[8], SUNXI_DRAM_PHY0_BASE + 0x4c0);
		writel((array0[8] << 24) | (array0[9] << 16) |
		       (array0[10] << 8) | array0[11],
		       SUNXI_DRAM_PHY0_BASE + 0x4c4);
		writel((array0[12] << 24) | (array0[13] << 16) |
		       (array0[14] << 8) | array0[15],
		       SUNXI_DRAM_PHY0_BASE + 0x4c8);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x58c, 0xff00,
				array0[8] << 8);
		writel((array0[8] << 24) | (array0[9] << 16) |
		       (array0[10] << 8) | array0[11],
		       SUNXI_DRAM_PHY0_BASE + 0x580);
		writel((array0[12] << 24) | (array0[13] << 16) |
		       (array0[14] << 8) | array0[15],
		       SUNXI_DRAM_PHY0_BASE + 0x584);

		writel(array0[8], SUNXI_DRAM_PHY0_BASE + 0x59c);
		writel((array0[8] << 24) | (array0[9] << 16) |
		       (array0[10] << 8) | array0[11],
		       SUNXI_DRAM_PHY0_BASE + 0x5a0);
		writel((array0[12] << 24) | (array0[13] << 16) |
		       (array0[14] << 8) | array0[15],
		       SUNXI_DRAM_PHY0_BASE + 0x5a4);

		tmp = (config->odt_en >> 8) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x4ac);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x4cc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x588);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x5a8);

		writel(array0[16], SUNXI_DRAM_PHY0_BASE + 0x620);
		writel((array0[16] << 24) | (array0[17] << 16) |
		       (array0[18] << 8) | array0[19],
		       SUNXI_DRAM_PHY0_BASE + 0x624);
		writel((array0[20] << 24) | (array0[21] << 16) |
		       (array0[22] << 8) | array0[23],
		       SUNXI_DRAM_PHY0_BASE + 0x628);

		writel(array0[16], SUNXI_DRAM_PHY0_BASE + 0x640);
		writel((array0[16] << 24) | (array0[17] << 16) |
		       (array0[18] << 8) | array0[19],
		       SUNXI_DRAM_PHY0_BASE + 0x644);
		writel((array0[20] << 24) | (array0[21] << 16) |
		       (array0[22] << 8) | array0[23],
		       SUNXI_DRAM_PHY0_BASE + 0x648);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x70c,
				0xff00, array0[16] << 8);
		writel((array0[16] << 24) | (array0[17] << 16) |
		       (array0[18] << 8) | array0[19],
		       SUNXI_DRAM_PHY0_BASE + 0x700);
		writel((array0[20] << 24) | (array0[21] << 16) |
		       (array0[22] << 8) | array0[23],
		       SUNXI_DRAM_PHY0_BASE + 0x704);

		writel(array0[16], SUNXI_DRAM_PHY0_BASE + 0x71c);
		writel((array0[16] << 24) | (array0[17] << 16) |
		       (array0[18] << 8) | array0[19],
		      SUNXI_DRAM_PHY0_BASE + 0x720);
		writel((array0[20] << 24) | (array0[21] << 16) |
		       (array0[22] << 8) | array0[23], SUNXI_DRAM_PHY0_BASE + 0x724);

		tmp = (config->odt_en >> 16) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x62c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x64c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x708);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x728);

		writel(array0[24], SUNXI_DRAM_PHY0_BASE + 0x7a0);
		writel((array0[24] << 24) | (array0[25] << 16) |
		       (array0[26] << 8) | array0[27],
		       SUNXI_DRAM_PHY0_BASE + 0x7a4);
		writel((array0[28] << 24) | (array0[29] << 16) |
		       (array0[30] << 8) | array0[31],
		       SUNXI_DRAM_PHY0_BASE + 0x7a8);

		writel(array0[24], SUNXI_DRAM_PHY0_BASE + 0x7c0);
		writel((array0[24] << 24) | (array0[25] << 16) |
		       (array0[26] << 8) | array0[27],
		       SUNXI_DRAM_PHY0_BASE + 0x7c4);
		writel((array0[28] << 24) | (array0[29] << 16) |
		       (array0[30] << 8) | array0[31],
		       SUNXI_DRAM_PHY0_BASE + 0x7c8);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x88c, 0xff00,
				array0[24] << 8);
		writel((array0[24] << 24) | (array0[25] << 16) |
		       (array0[26] << 8) | array0[27],
		       SUNXI_DRAM_PHY0_BASE + 0x880);
		writel((array0[28] << 24) | (array0[29] << 16) |
		       (array0[30] << 8) | array0[31],
		       SUNXI_DRAM_PHY0_BASE + 0x884);

		writel(array0[24], SUNXI_DRAM_PHY0_BASE + 0x89c);
		writel((array0[24] << 24) | (array0[25] << 16) |
		       (array0[26] << 8) | array0[27],
		       SUNXI_DRAM_PHY0_BASE + 0x8a0);
		writel((array0[28] << 24) | (array0[29] << 16) |
		       (array0[30] << 8) | array0[31],
		       SUNXI_DRAM_PHY0_BASE + 0x8a4);

		tmp = (config->odt_en >> 24) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x7ac);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x7cc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x888);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x8a8);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x44, BIT(28));
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x44, BIT(28));
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);
	}

	if (para->tpr10 & TPR10_DX_BIT_DELAY0) {
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);

		writel(array1[0] << 8, SUNXI_DRAM_PHY0_BASE + 0x330);
		writel((array1[0] << 24) | (array1[1] << 16) |
		       (array1[2] << 8) | array1[3],
		       SUNXI_DRAM_PHY0_BASE + 0x334);
		writel((array1[4] << 24) | (array1[5] << 16) |
		       (array1[6] << 8) | array1[7],
		       SUNXI_DRAM_PHY0_BASE + 0x338);

		writel(array1[0] << 8, SUNXI_DRAM_PHY0_BASE + 0x350);
		writel((array1[0] << 24) | (array1[1] << 16) |
		       (array1[2] << 8) | array1[3],
		       SUNXI_DRAM_PHY0_BASE + 0x354);
		writel((array1[4] << 24) | (array1[5] << 16) |
		       (array1[6] << 8) | array1[7],
		       SUNXI_DRAM_PHY0_BASE + 0x358);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x40c, 0xff, array1[0]);
		writel((array1[0] << 24) | (array1[1] << 16) |
		       (array1[2] << 8) | array1[3],
		       SUNXI_DRAM_PHY0_BASE + 0x410);
		writel((array1[4] << 24) | (array1[5] << 16) |
		       (array1[6] << 8) | array1[7],
		       SUNXI_DRAM_PHY0_BASE + 0x414);

		writel(array1[0] << 8, SUNXI_DRAM_PHY0_BASE + 0x42c);
		writel((array1[0] << 24) | (array1[1] << 16) |
		       (array1[2] << 8) | array1[3],
		       SUNXI_DRAM_PHY0_BASE + 0x430);
		writel((array1[4] << 24) | (array1[5] << 16) |
		       (array1[6] << 8) | array1[7],
		       SUNXI_DRAM_PHY0_BASE + 0x434);

		tmp = config->tpr14 & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x33c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x35c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x418);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x438);

		writel(array1[8] << 8, SUNXI_DRAM_PHY0_BASE + 0x4b0);
		writel((array1[8] << 24) | (array1[9] << 16) |
		       (array1[10] << 8) | array1[11],
		       SUNXI_DRAM_PHY0_BASE + 0x4b4);
		writel((array1[12] << 24) | (array1[13] << 16) |
		       (array1[14] << 8) | array1[15],
		       SUNXI_DRAM_PHY0_BASE + 0x4b8);

		writel(array1[8] << 8, SUNXI_DRAM_PHY0_BASE + 0x4d0);
		writel((array1[8] << 24) | (array1[9] << 16) |
		       (array1[10] << 8) | array1[11],
		       SUNXI_DRAM_PHY0_BASE + 0x4d4);
		writel((array1[12] << 24) | (array1[13] << 16) |
		       (array1[14] << 8) | array1[15],
		       SUNXI_DRAM_PHY0_BASE + 0x4d8);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x58c, 0xff, array1[8]);
		writel((array1[8] << 24) | (array1[9] << 16) |
		       (array1[10] << 8) | array1[11],
		       SUNXI_DRAM_PHY0_BASE + 0x590);
		writel((array1[12] << 24) | (array1[13] << 16) |
		       (array1[14] << 8) | array1[15],
		       SUNXI_DRAM_PHY0_BASE + 0x594);

		writel(array1[8] << 8, SUNXI_DRAM_PHY0_BASE + 0x5ac);
		writel((array1[8] << 24) | (array1[9] << 16) |
		       (array1[10] << 8) | array1[11],
		       SUNXI_DRAM_PHY0_BASE + 0x5b0);
		writel((array1[12] << 24) | (array1[13] << 16) |
		       (array1[14] << 8) | array1[15],
		       SUNXI_DRAM_PHY0_BASE + 0x5b4);

		tmp = (config->tpr14 >> 8) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x4bc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x4dc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x598);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x5b8);

		writel(array1[16] << 8, SUNXI_DRAM_PHY0_BASE + 0x630);
		writel((array1[16] << 24) | (array1[17] << 16) |
		       (array1[18] << 8) | array1[19],
		       SUNXI_DRAM_PHY0_BASE + 0x634);
		writel((array1[20] << 24) | (array1[21] << 16) |
		       (array1[22] << 8) | array1[23],
		       SUNXI_DRAM_PHY0_BASE + 0x638);

		writel(array1[16] << 8, SUNXI_DRAM_PHY0_BASE + 0x650);
		writel((array1[16] << 24) | (array1[17] << 16) |
		       (array1[18] << 8) | array1[19],
		       SUNXI_DRAM_PHY0_BASE + 0x654);
		writel((array1[20] << 24) | (array1[21] << 16) |
		       (array1[22] << 8) | array1[23],
		       SUNXI_DRAM_PHY0_BASE + 0x658);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x70c, 0xff, array1[16]);
		writel((array1[16] << 24) | (array1[17] << 16) |
		       (array1[18] << 8) | array1[19],
		       SUNXI_DRAM_PHY0_BASE + 0x710);
		writel((array1[20] << 24) | (array1[21] << 16) |
		       (array1[22] << 8) | array1[23],
		       SUNXI_DRAM_PHY0_BASE + 0x714);

		writel(array1[16] << 8, SUNXI_DRAM_PHY0_BASE + 0x72c);
		writel((array1[16] << 24) | (array1[17] << 16) |
		       (array1[18] << 8) | array1[19],
		       SUNXI_DRAM_PHY0_BASE + 0x730);
		writel((array1[20] << 24) | (array1[21] << 16) |
		       (array1[22] << 8) | array1[23],
		       SUNXI_DRAM_PHY0_BASE + 0x734);

		tmp = (config->tpr14 >> 16) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x63c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x65c);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x718);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x738);

		writel(array1[24] << 8, SUNXI_DRAM_PHY0_BASE + 0x7b0);
		writel((array1[24] << 24) | (array1[25] << 16) |
		       (array1[26] << 8) | array1[27],
		       SUNXI_DRAM_PHY0_BASE + 0x7b4);
		writel((array1[28] << 24) | (array1[29] << 16) |
		       (array1[30] << 8) | array1[31],
		       SUNXI_DRAM_PHY0_BASE + 0x7b8);

		writel(array1[24] << 8, SUNXI_DRAM_PHY0_BASE + 0x7d0);
		writel((array1[24] << 24) | (array1[25] << 16) |
		       (array1[26] << 8) | array1[27],
		       SUNXI_DRAM_PHY0_BASE + 0x7d4);
		writel((array1[28] << 24) | (array1[29] << 16) |
		       (array1[30] << 8) | array1[31],
		       SUNXI_DRAM_PHY0_BASE + 0x7d8);

		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x88c, 0xff, array1[24]);
		writel((array1[24] << 24) | (array1[25] << 16) |
		       (array1[26] << 8) | array1[27],
		       SUNXI_DRAM_PHY0_BASE + 0x890);
		writel((array1[28] << 24) | (array1[29] << 16) |
		       (array1[30] << 8) | array1[31],
		       SUNXI_DRAM_PHY0_BASE + 0x894);

		writel(array1[24] << 8, SUNXI_DRAM_PHY0_BASE + 0x8ac);
		writel((array1[24] << 24) | (array1[25] << 16) |
		       (array1[26] << 8) | array1[27],
		       SUNXI_DRAM_PHY0_BASE + 0x8b0);
		writel((array1[28] << 24) | (array1[29] << 16) |
		       (array1[30] << 8) | array1[31],
		       SUNXI_DRAM_PHY0_BASE + 0x8b4);

		tmp = (config->tpr14 >> 24) & 0xff;
		tmp = (tmp << 24) | (tmp << 8);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x7bc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x7dc);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x898);
		writel(tmp, SUNXI_DRAM_PHY0_BASE + 0x8b8);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x94, 4);
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x94, 4);
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);
	}
}

static void mctl_phy_ca_bit_delay_compensation(const struct dram_para *para,
					       const struct dram_config *config)
{
	u32 val, low, high;

	if (para->tpr10 & BIT(31)) {
		val = para->tpr0;
	} else {
		val = ((para->tpr10 & 0xf0) << 5) | ((para->tpr10 & 0xf) << 1);
		if (para->tpr10 >> 29)
			val <<= 1;
	}

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0xac, 0x1000);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x48, 0xc0000000);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
	case SUNXI_DRAM_TYPE_DDR4:
	case SUNXI_DRAM_TYPE_LPDDR3:
		low = val & 0xff;
		high = (val >> 8) & 0xff;

		val = (high << 24) | (high << 16) | (high << 8) | high;
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x104);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x108);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x10c);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x114);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x118);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x11c);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x120);

		val = (low << 24) | (low << 16) | (high << 8) | high;
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x11c);
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		low = val & 0xff;
		high = (val >> 8) & 0xff;

		val = (high << 24) | (high << 16) | (high << 8) | high;
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x104);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x108);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x10c);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x114);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x118);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x11c);
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x120);

		val = (high << 24) | (high << 16) | (low << 8) | low;
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x110);

		val = (low << 24) | (high << 16) | (low << 8) | high;
		writel(val, SUNXI_DRAM_PHY0_BASE + 0x11c);
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x38, 1);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x38, 1);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);
}

static bool mctl_phy_init(const struct dram_para *para,
			  const struct dram_config *config)
{
	void * const mctl_com = (void *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	void *const prcm = (void *)SUNXI_PRCM_BASE;
	u32 val, val2, mr1, mr2;
	int i;

	clrbits_le32(prcm + CCU_PRCM_SYS_PWROFF_GATING, 1);
	udelay(1);

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x40000);

	if (config->bus_full_width)
		val = 0xf00;
	else
		val = 0x300;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x00, 0xf00, val);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		val = 9;
		val2 = 13;
		break;
	case SUNXI_DRAM_TYPE_DDR4:
		if (config->clk <= 936) {
			val = 10;
			val2 = 14;
		} else if (config->clk <= 1200) {
			val = 12;
			val2 = 16;
		} else {
			val = 14;
			val2 = 18;
		}
		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		val = 8;
		val2 = 14;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		if (config->clk <= 936) {
			val = 10;
			val2 = 20;
		} else if (config->clk <= 1200) {
			val = 14;
			val2 = 28;
		} else {
			val = 16;
			val2 = 32;
		}
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	writel((val << 24) | (val << 16) | (val << 8) | val, SUNXI_DRAM_PHY0_BASE + 0x10);
	writel((val2 << 24) | (val2 << 16) | (val2 << 8) | val2, SUNXI_DRAM_PHY0_BASE + 0x0c);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x08);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		writel(0x150a0310, SUNXI_DRAM_PHY0_BASE + 0x54);
		writel(0x13140816, SUNXI_DRAM_PHY0_BASE + 0x58);
		writel(0x001c0d1b, SUNXI_DRAM_PHY0_BASE + 0x5c);
		writel(0x050c1d1a, SUNXI_DRAM_PHY0_BASE + 0x60);
		writel(0x0411060b, SUNXI_DRAM_PHY0_BASE + 0x64);
		writel(0x09071217, SUNXI_DRAM_PHY0_BASE + 0x68);
		writel(0x18190e01, SUNXI_DRAM_PHY0_BASE + 0x6c);
		writel(0x020f1e00, SUNXI_DRAM_PHY0_BASE + 0x70);
		break;
	case SUNXI_DRAM_TYPE_DDR4:
		writel(0x090c1c14, SUNXI_DRAM_PHY0_BASE + 0x54);
		writel(0x1300060f, SUNXI_DRAM_PHY0_BASE + 0x58);
		writel(0x12030807, SUNXI_DRAM_PHY0_BASE + 0x5c);
		writel(0x0b100a02, SUNXI_DRAM_PHY0_BASE + 0x60);
		writel(0x1a110e05, SUNXI_DRAM_PHY0_BASE + 0x64);
		writel(0x0d041617, SUNXI_DRAM_PHY0_BASE + 0x68);
		writel(0x1819011b, SUNXI_DRAM_PHY0_BASE + 0x6c);
		writel(0x151d1e00, SUNXI_DRAM_PHY0_BASE + 0x70);
		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		writel(0x010a1a0f, SUNXI_DRAM_PHY0_BASE + 0x54);
		writel(0x10081b07, SUNXI_DRAM_PHY0_BASE + 0x58);
		writel(0x11061c12, SUNXI_DRAM_PHY0_BASE + 0x5c);
		writel(0x00131409, SUNXI_DRAM_PHY0_BASE + 0x60);
		writel(0x15030e16, SUNXI_DRAM_PHY0_BASE + 0x64);
		writel(0x0b0c0d17, SUNXI_DRAM_PHY0_BASE + 0x68);
		writel(0x18190204, SUNXI_DRAM_PHY0_BASE + 0x6c);
		writel(0x051d1e00, SUNXI_DRAM_PHY0_BASE + 0x70);
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		writel(0x00010203, SUNXI_DRAM_PHY0_BASE + 0x54);
		writel(0x04050607, SUNXI_DRAM_PHY0_BASE + 0x58);
		writel(0x08090a0b, SUNXI_DRAM_PHY0_BASE + 0x5c);
		writel(0x0c0d0e0f, SUNXI_DRAM_PHY0_BASE + 0x60);
		writel(0x10111213, SUNXI_DRAM_PHY0_BASE + 0x64);
		writel(0x14151617, SUNXI_DRAM_PHY0_BASE + 0x68);
		writel(0x18191a1b, SUNXI_DRAM_PHY0_BASE + 0x6c);
		writel(0x1c1d1e00, SUNXI_DRAM_PHY0_BASE + 0x70);
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	mctl_phy_configure_odt(para);

	if (para->tpr10 & TPR10_CA_BIT_DELAY)
		mctl_phy_ca_bit_delay_compensation(para, config);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		val = 0x2bbd4900;
		break;
	case SUNXI_DRAM_TYPE_DDR4:
		val = 0x3841b800;
		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		val = 0x19016300;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		val = 0x18fd6300;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa8, 0xffffff00, val);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x00, 0x70);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		val = 0x20;
		break;
	case SUNXI_DRAM_TYPE_DDR4:
		val = 0x40;
		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		val = 0x30;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		val = 0x50;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x00, val);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x00, 0x80);

	// TODO: fix intervals
	if (config->clk - 251 < 250) {
		val = 0x18000000;
		val2 = 0x18181818;
	} else if (config->clk - 126 < 125) {
		val = 0x28000000;
		val2 = 0x28282828;
	} else if (config->clk < 126) {
		val = 0x38000000;
		val2 = 0x38383838;
	} else {
		val = 0x18000000;
		val2 = 0;
	}
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xc0, 0x78000000, val);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xd0, 0x78787878, val2);

	clrbits_le32(mctl_com + MCTL_COM_UNK_008, BIT(9));
	udelay(10);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		val = para->tpr6 & 0xff;
		break;
	case SUNXI_DRAM_TYPE_DDR4:
		val = para->tpr6 >> 8 & 0xff;
		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		val = para->tpr6 >> 16 & 0xff;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		val = para->tpr6 >> 24;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};
	val <<= 24;

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x300, 0xff800060, val | 0x40);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x600, 0xff800060, val | 0x40);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x480, 0xff800060, val | 0x40);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x780, 0xff800060, val | 0x40);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x8000000);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x94, 0x80);
	udelay(10);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x94, 0x80);
	udelay(10);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x84, 0x8000000);

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x308, 0x200);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x488, 0x200);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x608, 0x200);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x788, 0x200);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x908, 0x200);
	if (para->type == SUNXI_DRAM_TYPE_LPDDR4) {
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x308, 0x200);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x488, 0x200);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x608, 0x200);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x788, 0x200);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x908, 0x200);
	}

	if (config->clk < 936)
		val = 0x1b000000;
	else
		val = 0xc000000;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x14, 0x1f000000, val);

	setbits_le32(mctl_com + MCTL_COM_MAER0, BIT(8));

	/* start DFI init */
	writel(0, &mctl_ctl->swctl);
	setbits_le32(&mctl_ctl->dfimisc, 1);
	setbits_le32(&mctl_ctl->dfimisc, 0x20);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);
	mctl_await_completion(&mctl_ctl->dfistat, 1, 1);

	udelay(500);
	setbits_le32(prcm + CCU_PRCM_SYS_PWROFF_GATING, 1);
	udelay(1);

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->dfimisc, 0x20);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->pwrctl, 0x20);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);
	mctl_await_completion(&mctl_ctl->statr, 3, 1);

	udelay(500);

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->dfimisc, 1);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		writel(0x1f14, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(4, &mctl_ctl->mrctrl1);
		writel(0x800010f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x20, &mctl_ctl->mrctrl1);
		writel(0x800020f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0, &mctl_ctl->mrctrl1);
		writel(0x800030f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		if (config->clk <= 936) {
			mr1 = 0x34;
			mr2 = 0x1b;
		} else if (config->clk <= 1200) {
			mr1 = 0x54;
			mr2 = 0x2d;
		} else {
			mr1 = 0x64;
			mr2 = 0x36;
		}

		writel(0x0, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x100 | mr1, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x200 | mr2, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x333, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x403, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0xb04, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0xc72, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0xd00, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0xe08, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

		writel(0x1626, &mctl_ctl->mrctrl1);
		writel(0x800000f0, &mctl_ctl->mrctrl0);
		mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->rfshctl3, 1);
	writel(1, &mctl_ctl->swctl);

	if (para->tpr10 & TPR10_WRITE_LEVELING) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_write_leveling(para, config))
				break;
		if (i == 5) {
			debug("write leveling failed!\n");
			return false;
		}
	}

	if (para->tpr10 & TPR10_READ_CALIBRATION) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_read_calibration(para, config))
				break;
		if (i == 5) {
			debug("read calibration failed!\n");
			return false;
		}
	}

	if (para->tpr10 & TPR10_READ_TRAINING) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_read_training(para, config))
				break;
		if (i == 5) {
			debug("read training failed!\n");
			return false;
		}
	}

	if (para->tpr10 & TPR10_WRITE_TRAINING) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_write_training(config))
				break;
		if (i == 5) {
			debug("write training failed!\n");
			return false;
		}
	}

	mctl_phy_bit_delay_compensation(para, config);

	return true;
}

static bool mctl_ctrl_init(const struct dram_para *para,
			   const struct dram_config *config)
{
	void * const mctl_com = (void *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u32 reg_val;

	clrsetbits_le32(mctl_com + MCTL_COM_UNK_008, BIT(24), BIT(25) | BIT(9));
	setbits_le32(mctl_com + MCTL_COM_MAER0, BIT(15) | BIT(9));

	if (para->type == SUNXI_DRAM_TYPE_LPDDR4) {
		setbits_le32(0x02023ea8, 1); // NSI
		setbits_le32(0x02071008, 1); // NSI_CPU
	}

	clrsetbits_le32(&mctl_ctl->sched[0], 0xff08, 0x3000);
	clrsetbits_le32(&mctl_ctl->sched[1], 0x77000000, 0x33000000);
	clrsetbits_le32(&mctl_ctl->unk_0x270, 0xffff, 0x808);
	clrsetbits_le32(&mctl_ctl->unk_0x264, 0xff00ffff, 0x1f000030);

	writel(0, &mctl_ctl->hwlpctl);

	reg_val = MSTR_ACTIVE_RANKS(config->ranks);
	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		reg_val |= MSTR_BURST_LENGTH(8) | MSTR_DEVICETYPE_DDR3 | MSTR_2TMODE;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		reg_val |= MSTR_BURST_LENGTH(16) | MSTR_DEVICETYPE_LPDDR4;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};
	if (config->bus_full_width)
		reg_val |= MSTR_BUSWIDTH_FULL;
	else
		reg_val |= MSTR_BUSWIDTH_HALF;
	writel(BIT(31) | BIT(30) | reg_val, &mctl_ctl->mstr);

	if (config->ranks == 2)
		writel(0x0303, &mctl_ctl->odtmap);
	else
		writel(0x0201, &mctl_ctl->odtmap);

	switch (para->type) {
	case SUNXI_DRAM_TYPE_DDR3:
		reg_val = 0x06000400;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		reg_val = 0x04000400;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};
	writel(reg_val, &mctl_ctl->odtcfg);
	writel(reg_val, &mctl_ctl->unk_0x2240);
	writel(reg_val, &mctl_ctl->unk_0x3240);
	writel(reg_val, &mctl_ctl->unk_0x4240);

	mctl_set_addrmap(config);

	mctl_set_timing_params(config->clk);

	writel(0, &mctl_ctl->pwrctl);

	setbits_le32(&mctl_ctl->dfiupd[0], BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->zqctl[0], BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x2180, BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x3180, BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x4180, BIT(31) | BIT(30));

	if (para->type == SUNXI_DRAM_TYPE_LPDDR4)
		setbits_le32(&mctl_ctl->dbictl, 0x1);

	setbits_le32(&mctl_ctl->rfshctl3, BIT(0));
	clrbits_le32(&mctl_ctl->dfimisc, BIT(0));

	writel(0x20, &mctl_ctl->pwrctl);
	setbits_le32(&mctl_ctl->clken, BIT(8));

	clrsetbits_le32(mctl_com + MCTL_COM_UNK_008, BIT(24), BIT(9));
	udelay(1);
	/* this write seems to enable PHY MMIO region */
	setbits_le32(mctl_com + MCTL_COM_UNK_008, BIT(24));

	if (!mctl_phy_init(para, config))
		return false;

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->rfshctl3, BIT(0));
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);

	return true;
}

static bool mctl_core_init(const struct dram_para *para,
			   const struct dram_config *config)
{
	mctl_sys_init(config->clk);

	return mctl_ctrl_init(para, config);
}

static void mctl_auto_detect_rank_width(const struct dram_para *para,
					struct dram_config *config)
{
	/* this is minimum size that it's supported */
	config->cols = 8;
	config->rows = 13;

	/*
	 * Strategy here is to test most demanding combination first and least
	 * demanding last, otherwise HW might not be fully utilized. For
	 * example, half bus width and rank = 1 combination would also work
	 * on HW with full bus width and rank = 2, but only 1/4 RAM would be
	 * visible.
	 */

	debug("testing 32-bit width, rank = 2\n");
	config->bus_full_width = 1;
	config->ranks = 2;
	if (mctl_core_init(para, config))
		return;

	debug("testing 32-bit width, rank = 1\n");
	config->bus_full_width = 1;
	config->ranks = 1;
	if (mctl_core_init(para, config))
		return;

	debug("testing 16-bit width, rank = 2\n");
	config->bus_full_width = 0;
	config->ranks = 2;
	if (mctl_core_init(para, config))
		return;

	debug("testing 16-bit width, rank = 1\n");
	config->bus_full_width = 0;
	config->ranks = 1;
	if (mctl_core_init(para, config))
		return;

	panic("This DRAM setup is currently not supported.\n");
}

static void mctl_auto_detect_dram_size(const struct dram_para *para,
				       struct dram_config *config)
{
	/* detect row address bits */
	config->cols = 8;
	config->rows = 16;
	mctl_core_init(para, config);

	for (config->rows = 13; config->rows < 16; config->rows++) {
		/* 8 banks, 8 bit per byte and 16/32 bit width */
		if (mctl_mem_matches((1 << (config->rows + config->cols +
					    4 + config->bus_full_width))))
			break;
	}

	/* detect column address bits */
	config->cols = 11;
	mctl_core_init(para, config);

	for (config->cols = 8; config->cols < 11; config->cols++) {
		/* 8 bits per byte and 16/32 bit width */
		if (mctl_mem_matches(1 << (config->cols + 1 +
					   config->bus_full_width)))
			break;
	}
}

static unsigned long long mctl_calc_size(const struct dram_config *config)
{
	u8 width = config->bus_full_width ? 4 : 2;

	/* 8 banks */
	return (1ULL << (config->cols + config->rows + 3)) * width * config->ranks;
}

static const struct dram_para para = {
#ifdef CONFIG_SUNXI_DRAM_A523_DDR3
	.type = SUNXI_DRAM_TYPE_DDR3,
#elif defined(CONFIG_SUNXI_DRAM_A523_LPDDR4)
	.type = SUNXI_DRAM_TYPE_LPDDR4,
#endif
	.dx_odt = CONFIG_DRAM_SUNXI_DX_ODT,
	.dx_dri = CONFIG_DRAM_SUNXI_DX_DRI,
	.ca_dri = CONFIG_DRAM_SUNXI_CA_DRI,
	.tpr0 = CONFIG_DRAM_SUNXI_TPR0,
	.tpr1 = CONFIG_DRAM_SUNXI_TPR1,
	.tpr2 = CONFIG_DRAM_SUNXI_TPR2,
	.tpr6 = CONFIG_DRAM_SUNXI_TPR6,
	.tpr10 = CONFIG_DRAM_SUNXI_TPR10,
};

static void sunxi_nsi_init(void)
{
	/* IOMMU prio 3 */
	writel(0x1, 0x02021418);
	writel(0xf, 0x02021414);
	/* DE prio 2 */
	writel(0x1, 0x02021a18);
	writel(0xa, 0x02021a14);
	/* VE R prio 2 */
	writel(0x1, 0x02021618);
	writel(0xa, 0x02021614);
	/* VE RW prio 2 */
	writel(0x1, 0x02021818);
	writel(0xa, 0x02021814);
	/* ISP prio 2 */
	writel(0x1, 0x02020c18);
	writel(0xa, 0x02020c14);
	/* CSI prio 2 */
	writel(0x1, 0x02021c18);
	writel(0xa, 0x02021c14);
	/* NPU prio 2 */
	writel(0x1, 0x02020a18);
	writel(0xa, 0x02020a14);

	/* close ra0 autogating */
	writel(0x0, 0x02023c00);
	/* close ta autogating */
	writel(0x0, 0x02023e00);
	/* close pcie autogating */
	writel(0x0, 0x02020600);
}

static void init_something(void)

{
	u32 *ptr = (u32 *)0x02000804;

	do {
		*ptr++ = 0xffffffff;
	} while (ptr != (u32 *)0x20008e4);

	writel(0, 0x07002400);
	writel(0, 0x07002404);
	writel(0, 0x07002408);

	writel(0xffffffff, 0x07002004);
	writel(0xffffffff, 0x07002014);
	writel(0xffffffff, 0x07002024);
	setbits_le32(0x07010290, 7);

	writel(7, 0x02001f00);
	writel(0xffff, 0x03002020);
	writel(3, 0x020008e0);
	writel(7, 0x07102008);
}

unsigned long sunxi_dram_init(void)
{
	struct dram_config config;
	unsigned long size;

	config.clk = 360;
	switch (para.type) {
	case SUNXI_DRAM_TYPE_DDR3:
		config.odt_en = 0x90909090;
		config.tpr11 = 0x8f919190;
		config.tpr12 = 0x22222723;
		config.tpr14 = 0x48484848;
		break;
	case SUNXI_DRAM_TYPE_LPDDR4:
		config.odt_en = 0x84848484;
		config.tpr11 = 0x9a9a9a9a;
		config.tpr12 = 0x0e0f070a;
		config.tpr14 = 0x48484848;
		break;
	default:
		panic("This DRAM setup is currently not supported.\n");
	};

	setbits_le32(0x03000160, BIT(8));
	clrbits_le32(0x03000168, 0x3f);

	mctl_auto_detect_rank_width(&para, &config);
	mctl_auto_detect_dram_size(&para, &config);

	config.clk = CONFIG_DRAM_CLK;
	config.odt_en = CONFIG_DRAM_SUNXI_ODT_EN;
	config.tpr11 = CONFIG_DRAM_SUNXI_TPR11;
	config.tpr12 = CONFIG_DRAM_SUNXI_TPR12;
	config.tpr14 = CONFIG_DRAM_SUNXI_TPR14;

	mctl_core_init(&para, &config);

	size = mctl_calc_size(&config);

	sunxi_nsi_init();
	init_something();

	return size;
};
