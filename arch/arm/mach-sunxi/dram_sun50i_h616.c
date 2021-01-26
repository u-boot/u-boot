// SPDX-License-Identifier: GPL-2.0+
/*
 * sun50i H616 platform dram controller driver
 *
 * While controller is very similar to that in H6, PHY is completely
 * unknown. That's why this driver has plenty of magic numbers. Some
 * meaning was nevertheless deduced from strings found in boot0 and
 * known meaning of some dram parameters.
 * This driver only supports DDR3 memory and omits logic for all
 * other supported types supported by hardware.
 *
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 *
 */
#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kconfig.h>

enum {
	MBUS_QOS_LOWEST = 0,
	MBUS_QOS_LOW,
	MBUS_QOS_HIGH,
	MBUS_QOS_HIGHEST
};

inline void mbus_configure_port(u8 port,
				bool bwlimit,
				bool priority,
				u8 qos,
				u8 waittime,
				u8 acs,
				u16 bwl0,
				u16 bwl1,
				u16 bwl2)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	const u32 cfg0 = ( (bwlimit ? (1 << 0) : 0)
			   | (priority ? (1 << 1) : 0)
			   | ((qos & 0x3) << 2)
			   | ((waittime & 0xf) << 4)
			   | ((acs & 0xff) << 8)
			   | (bwl0 << 16) );
	const u32 cfg1 = ((u32)bwl2 << 16) | (bwl1 & 0xffff);

	debug("MBUS port %d cfg0 %08x cfg1 %08x\n", port, cfg0, cfg1);
	writel_relaxed(cfg0, &mctl_com->master[port].cfg0);
	writel_relaxed(cfg1, &mctl_com->master[port].cfg1);
}

#define MBUS_CONF(port, bwlimit, qos, acs, bwl0, bwl1, bwl2)	\
	mbus_configure_port(port, bwlimit, false, \
			    MBUS_QOS_ ## qos, 0, acs, bwl0, bwl1, bwl2)

static void mctl_set_master_priority(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(399, &mctl_com->tmr);
	writel(BIT(16), &mctl_com->bwcr);

	MBUS_CONF( 0, true, HIGHEST, 0,  256,  128,  100);
	MBUS_CONF( 1, true,    HIGH, 0, 1536, 1400,  256);
	MBUS_CONF( 2, true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF( 3, true,    HIGH, 0,  256,  100,   80);
	MBUS_CONF( 4, true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF( 5, true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF( 6, true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF( 8, true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(11, true,    HIGH, 0,  256,  128,  100);
	MBUS_CONF(14, true,    HIGH, 0, 1024,  256,   64);
	MBUS_CONF(16, true, HIGHEST, 6, 8192, 2800, 2400);
	MBUS_CONF(21, true, HIGHEST, 6, 2048,  768,  512);
	MBUS_CONF(25, true, HIGHEST, 0,  100,   64,   32);
	MBUS_CONF(26, true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF(37, true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(38, true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF(39, true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF(40, true,    HIGH, 2,  100,   64,   32);

	dmb();
}

static void mctl_sys_init(struct dram_para *para)
{
	struct sunxi_ccm_reg * const ccm =
			(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	/* Put all DRAM-related blocks to reset state */
	clrbits_le32(&ccm->mbus_cfg, MBUS_ENABLE);
	clrbits_le32(&ccm->mbus_cfg, MBUS_RESET);
	clrbits_le32(&ccm->dram_gate_reset, BIT(GATE_SHIFT));
	udelay(5);
	clrbits_le32(&ccm->dram_gate_reset, BIT(RESET_SHIFT));
	clrbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_EN);
	clrbits_le32(&ccm->dram_clk_cfg, DRAM_MOD_RESET);

	udelay(5);

	/* Set PLL5 rate to doubled DRAM clock rate */
	writel(CCM_PLL5_CTRL_EN | CCM_PLL5_LOCK_EN | CCM_PLL5_OUT_EN |
	       CCM_PLL5_CTRL_N(para->clk * 2 / 24 - 1), &ccm->pll5_cfg);
	mctl_await_completion(&ccm->pll5_cfg, CCM_PLL5_LOCK, CCM_PLL5_LOCK);

	/* Configure DRAM mod clock */
	writel(DRAM_CLK_SRC_PLL5, &ccm->dram_clk_cfg);
	writel(BIT(RESET_SHIFT), &ccm->dram_gate_reset);
	udelay(5);
	setbits_le32(&ccm->dram_gate_reset, BIT(GATE_SHIFT));

	/* Disable all channels */
	writel(0, &mctl_com->maer0);
	writel(0, &mctl_com->maer1);
	writel(0, &mctl_com->maer2);

	/* Configure MBUS and enable DRAM mod reset */
	setbits_le32(&ccm->mbus_cfg, MBUS_RESET);
	setbits_le32(&ccm->mbus_cfg, MBUS_ENABLE);

	clrbits_le32(&mctl_com->unk_0x500, BIT(25));

	setbits_le32(&ccm->dram_clk_cfg, DRAM_MOD_RESET);
	udelay(5);

	/* Unknown hack, which enables access of mctl_ctl regs */
	writel(0x8000, &mctl_ctl->clken);
}

static void mctl_set_addrmap(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u8 cols = para->cols;
	u8 rows = para->rows;
	u8 ranks = para->ranks;

	if (!para->bus_full_width)
		cols -= 1;

	/* Ranks */
	if (ranks == 2)
		mctl_ctl->addrmap[0] = rows + cols - 3;
	else
		mctl_ctl->addrmap[0] = 0x1F;

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
	mctl_ctl->addrmap[5] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
	switch (rows) {
	case 13:
		mctl_ctl->addrmap[6] = (cols - 3) | 0x0F0F0F00;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 14:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | 0x0F0F0000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 15:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | 0x0F000000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 16:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 17:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | 0x0F00;
		break;
	case 18:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | ((cols - 3) << 8);
		break;
	default:
		panic("Unsupported DRAM configuration: row number invalid\n");
	}

	/* Bank groups, DDR4 only */
	mctl_ctl->addrmap[8] = 0x3F3F;
}

static const u8 phy_init[] = {
	0x07, 0x0b, 0x02, 0x16, 0x0d, 0x0e, 0x14, 0x19,
	0x0a, 0x15, 0x03, 0x13, 0x04, 0x0c, 0x10, 0x06,
	0x0f, 0x11, 0x1a, 0x01, 0x12, 0x17, 0x00, 0x08,
	0x09, 0x05, 0x18
};

static void mctl_phy_configure_odt(void)
{
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x388);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x38c);

	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x3c8);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x3cc);

	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x408);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x40c);

	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x448);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x44c);

	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x340);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x344);

	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x348);
	writel_relaxed(0xe, SUNXI_DRAM_PHY0_BASE + 0x34c);

	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x380);
	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x384);

	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x3c0);
	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x3c4);

	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x400);
	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x404);

	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x440);
	writel_relaxed(0x8, SUNXI_DRAM_PHY0_BASE + 0x444);

	dmb();
}

static bool mctl_phy_write_leveling(struct dram_para *para)
{
	bool result = true;
	u32 val;

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0xc0, 0x80);
	writel(4, SUNXI_DRAM_PHY0_BASE + 0xc);
	writel(0x40, SUNXI_DRAM_PHY0_BASE + 0x10);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 4);

	if (para->bus_full_width)
		val = 0xf;
	else
		val = 3;

	mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x188), val, val);

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 4);

	val = readl(SUNXI_DRAM_PHY0_BASE + 0x258);
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0x25c);
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0x318);
	if (val == 0 || val == 0x3f)
		result = false;
	val = readl(SUNXI_DRAM_PHY0_BASE + 0x31c);
	if (val == 0 || val == 0x3f)
		result = false;

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0xc0);

	if (para->ranks == 2) {
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0xc0, 0x40);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 4);

		if (para->bus_full_width)
			val = 0xf;
		else
			val = 3;

		mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x188), val, val);

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 4);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0xc0);

	return result;
}

static bool mctl_phy_read_calibration(struct dram_para *para)
{
	bool result = true;
	u32 val, tmp;

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0x30, 0x20);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 1);

	if (para->bus_full_width)
		val = 0xf;
	else
		val = 3;

	while ((readl(SUNXI_DRAM_PHY0_BASE + 0x184) & val) != val) {
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x184) & 0x20) {
			result = false;
			break;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 1);

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0x30);

	if (para->ranks == 2) {
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0x30, 0x10);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 1);

		while ((readl(SUNXI_DRAM_PHY0_BASE + 0x184) & val) != val) {
			if (readl(SUNXI_DRAM_PHY0_BASE + 0x184) & 0x20) {
				result = false;
				break;
			}
		}

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 1);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 0x30);

	val = readl(SUNXI_DRAM_PHY0_BASE + 0x274) & 7;
	tmp = readl(SUNXI_DRAM_PHY0_BASE + 0x26c) & 7;
	if (val < tmp)
		val = tmp;
	tmp = readl(SUNXI_DRAM_PHY0_BASE + 0x32c) & 7;
	if (val < tmp)
		val = tmp;
	tmp = readl(SUNXI_DRAM_PHY0_BASE + 0x334) & 7;
	if (val < tmp)
		val = tmp;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x38, 0x7, (val + 2) & 7);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 0x20);

	return result;
}

static bool mctl_phy_read_training(struct dram_para *para)
{
	u32 val1, val2, *ptr1, *ptr2;
	bool result = true;
	int i;

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3, 2);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x804, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x808, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa04, 0x3f, 0xf);
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0xa08, 0x3f, 0xf);

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 6);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 1);

	mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x840), 0xc, 0xc);
	if (readl(SUNXI_DRAM_PHY0_BASE + 0x840) & 3)
		result = false;

	if (para->bus_full_width) {
		mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0xa40), 0xc, 0xc);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0xa40) & 3)
			result = false;
	}

	ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x898);
	ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x850);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}
	ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x8bc);
	ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x874);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}

	if (para->bus_full_width) {
		ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xa98);
		ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xa50);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}

		ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xabc);
		ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xa74);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 3);

	if (para->ranks == 2) {
		/* maybe last parameter should be 1? */
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3, 2);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 6);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 1);

		mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x840), 0xc, 0xc);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x840) & 3)
			result = false;

		if (para->bus_full_width) {
			mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0xa40), 0xc, 0xc);
			if (readl(SUNXI_DRAM_PHY0_BASE + 0xa40) & 3)
				result = false;
		}

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 3);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 3);

	return result;
}

static bool mctl_phy_write_training(struct dram_para *para)
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

	mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x8e0), 3, 3);
	if (readl(SUNXI_DRAM_PHY0_BASE + 0x8e0) & 0xc)
		result = false;

	if (para->bus_full_width) {
		mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0xae0), 3, 3);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0xae0) & 0xc)
			result = false;
	}

	ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x938);
	ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x8f0);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}
	ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x95c);
	ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x914);
	for (i = 0; i < 9; i++) {
		val1 = readl(&ptr1[i]);
		val2 = readl(&ptr2[i]);
		if (val1 - val2 <= 6)
			result = false;
	}

	if (para->bus_full_width) {
		ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xb38);
		ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xaf0);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
		ptr1 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xb5c);
		ptr2 = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xb14);
		for (i = 0; i < 9; i++) {
			val1 = readl(&ptr1[i]);
			val2 = readl(&ptr2[i]);
			if (val1 - val2 <= 6)
				result = false;
		}
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x60);

	if (para->ranks == 2) {
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 0xc, 4);

		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x10);
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x20);

		mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x8e0), 3, 3);
		if (readl(SUNXI_DRAM_PHY0_BASE + 0x8e0) & 0xc)
			result = false;

		if (para->bus_full_width) {
			mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0xae0), 3, 3);
			if (readl(SUNXI_DRAM_PHY0_BASE + 0xae0) & 0xc)
				result = false;
		}

		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x60);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x198, 0xc);

	return result;
}

static bool mctl_phy_bit_delay_compensation(struct dram_para *para)
{
	u32 *ptr;
	int i;

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x60, 1);
	setbits_le32(SUNXI_DRAM_PHY0_BASE + 8, 8);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 0x10);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x484);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x16, ptr);
		writel_relaxed(0x16, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x4d0);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x590);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x4cc);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x58c);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x4d8);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x1a, ptr);
		writel_relaxed(0x1a, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x524);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x5e4);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x520);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x5e0);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x604);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x1a, ptr);
		writel_relaxed(0x1a, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x650);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x710);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x64c);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x70c);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x658);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x1a, ptr);
		writel_relaxed(0x1a, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x6a4);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x764);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x6a0);
	writel_relaxed(0x1e, SUNXI_DRAM_PHY0_BASE + 0x760);

	dmb();

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x60, 1);

	/* second part */
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x54, 0x80);
	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x190, 4);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x480);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x10, ptr);
		writel_relaxed(0x10, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x18, SUNXI_DRAM_PHY0_BASE + 0x528);
	writel_relaxed(0x18, SUNXI_DRAM_PHY0_BASE + 0x5e8);
	writel_relaxed(0x18, SUNXI_DRAM_PHY0_BASE + 0x4c8);
	writel_relaxed(0x18, SUNXI_DRAM_PHY0_BASE + 0x588);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x4d4);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x12, ptr);
		writel_relaxed(0x12, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x52c);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x5ec);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x51c);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x5dc);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x600);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x12, ptr);
		writel_relaxed(0x12, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x6a8);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x768);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x648);
	writel_relaxed(0x1a, SUNXI_DRAM_PHY0_BASE + 0x708);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x654);
	for (i = 0; i < 9; i++) {
		writel_relaxed(0x14, ptr);
		writel_relaxed(0x14, ptr + 0x30);
		ptr += 2;
	}
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x6ac);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x76c);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x69c);
	writel_relaxed(0x1c, SUNXI_DRAM_PHY0_BASE + 0x75c);

	dmb();

	setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x54, 0x80);

	return true;
}

static bool mctl_phy_init(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u32 val, *ptr;
	int i;

	if (para->bus_full_width)
		val = 0xf;
	else
		val = 3;
	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x3c, 0xf, val);

	writel(0xd, SUNXI_DRAM_PHY0_BASE + 0x14);
	writel(0xd, SUNXI_DRAM_PHY0_BASE + 0x35c);
	writel(0xd, SUNXI_DRAM_PHY0_BASE + 0x368);
	writel(0xd, SUNXI_DRAM_PHY0_BASE + 0x374);

	writel(0, SUNXI_DRAM_PHY0_BASE + 0x18);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x360);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x36c);
	writel(0, SUNXI_DRAM_PHY0_BASE + 0x378);

	writel(9, SUNXI_DRAM_PHY0_BASE + 0x1c);
	writel(9, SUNXI_DRAM_PHY0_BASE + 0x364);
	writel(9, SUNXI_DRAM_PHY0_BASE + 0x370);
	writel(9, SUNXI_DRAM_PHY0_BASE + 0x37c);

	ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0xc0);
	for (i = 0; i < ARRAY_SIZE(phy_init); i++)
		writel(phy_init[i], &ptr[i]);

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_UNKNOWN_FEATURE)) {
		ptr = (u32*)(SUNXI_DRAM_PHY0_BASE + 0x780);
		for (i = 0; i < 32; i++)
			writel(0x16, &ptr[i]);
		writel(0xe, SUNXI_DRAM_PHY0_BASE + 0x78c);
		writel(0xe, SUNXI_DRAM_PHY0_BASE + 0x7a4);
		writel(0xe, SUNXI_DRAM_PHY0_BASE + 0x7b8);
		writel(0x8, SUNXI_DRAM_PHY0_BASE + 0x7d4);
		writel(0xe, SUNXI_DRAM_PHY0_BASE + 0x7dc);
		writel(0xe, SUNXI_DRAM_PHY0_BASE + 0x7e0);
	}

	writel(0x80, SUNXI_DRAM_PHY0_BASE + 0x3dc);
	writel(0x80, SUNXI_DRAM_PHY0_BASE + 0x45c);

	if (IS_ENABLED(DRAM_ODT_EN))
		mctl_phy_configure_odt();

	clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 4, 7, 0xa);

	if (para->clk <= 672)
		writel(0xf, SUNXI_DRAM_PHY0_BASE + 0x20);
	if (para->clk > 500) {
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x144, BIT(7));
		clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x14c, 0xe0);
	} else {
		setbits_le32(SUNXI_DRAM_PHY0_BASE + 0x144, BIT(7));
		clrsetbits_le32(SUNXI_DRAM_PHY0_BASE + 0x14c, 0xe0, 0x20);
	}

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x14c, 8);

	mctl_await_completion((u32*)(SUNXI_DRAM_PHY0_BASE + 0x180), 4, 4);

	writel(0x37, SUNXI_DRAM_PHY0_BASE + 0x58);
	clrbits_le32(&mctl_com->unk_0x500, 0x200);

	writel(0, &mctl_ctl->swctl);
	setbits_le32(&mctl_ctl->dfimisc, 1);

	/* start DFI init */
	setbits_le32(&mctl_ctl->dfimisc, 0x20);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);
	/* poll DFI init complete */
	mctl_await_completion(&mctl_ctl->dfistat, 1, 1);
	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->dfimisc, 0x20);

	clrbits_le32(&mctl_ctl->pwrctl, 0x20);
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);
	mctl_await_completion(&mctl_ctl->statr, 3, 1);

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->dfimisc, 1);

	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);

	writel(0x1f14, &mctl_ctl->mrctrl1);
	writel(0x80000030, &mctl_ctl->mrctrl0);
	mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

	writel(4, &mctl_ctl->mrctrl1);
	writel(0x80001030, &mctl_ctl->mrctrl0);
	mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

	writel(0x20, &mctl_ctl->mrctrl1);
	writel(0x80002030, &mctl_ctl->mrctrl0);
	mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

	writel(0, &mctl_ctl->mrctrl1);
	writel(0x80003030, &mctl_ctl->mrctrl0);
	mctl_await_completion(&mctl_ctl->mrctrl0, BIT(31), 0);

	writel(0, SUNXI_DRAM_PHY0_BASE + 0x54);

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->rfshctl3, 1);
	writel(1, &mctl_ctl->swctl);

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_WRITE_LEVELING)) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_write_leveling(para))
				break;
		if (i == 5) {
			debug("write leveling failed!\n");
			return false;
		}
	}

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_READ_CALIBRATION)) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_read_calibration(para))
				break;
		if (i == 5) {
			debug("read calibration failed!\n");
			return false;
		}
	}

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_READ_TRAINING)) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_read_training(para))
				break;
		if (i == 5) {
			debug("read training failed!\n");
			return false;
		}
	}

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_WRITE_TRAINING)) {
		for (i = 0; i < 5; i++)
			if (mctl_phy_write_training(para))
				break;
		if (i == 5) {
			debug("write training failed!\n");
			return false;
		}
	}

	if (IS_ENABLED(CONFIG_DRAM_SUN50I_H616_BIT_DELAY_COMPENSATION))
		mctl_phy_bit_delay_compensation(para);

	clrbits_le32(SUNXI_DRAM_PHY0_BASE + 0x60, 4);

	return true;
}

static bool mctl_ctrl_init(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u32 reg_val;

	clrsetbits_le32(&mctl_com->unk_0x500, BIT(24), 0x200);
	writel(0x8000, &mctl_ctl->clken);

	setbits_le32(&mctl_com->unk_0x008, 0xff00);

	clrsetbits_le32(&mctl_ctl->sched[0], 0xff00, 0x3000);

	writel(0, &mctl_ctl->hwlpctl);

	setbits_le32(&mctl_com->unk_0x008, 0xff00);

	reg_val = MSTR_BURST_LENGTH(8) | MSTR_ACTIVE_RANKS(para->ranks);
	reg_val |= MSTR_DEVICETYPE_DDR3 | MSTR_2TMODE;
	if (para->bus_full_width)
		reg_val |= MSTR_BUSWIDTH_FULL;
	else
		reg_val |= MSTR_BUSWIDTH_HALF;
	writel(BIT(31) | BIT(30) | reg_val, &mctl_ctl->mstr);

	if (para->ranks == 2)
		writel(0x0303, &mctl_ctl->odtmap);
	else
		writel(0x0201, &mctl_ctl->odtmap);

	writel(0x06000400, &mctl_ctl->odtcfg);
	writel(0x06000400, &mctl_ctl->unk_0x2240);
	writel(0x06000400, &mctl_ctl->unk_0x3240);
	writel(0x06000400, &mctl_ctl->unk_0x4240);

	setbits_le32(&mctl_com->cr, BIT(31));

	mctl_set_addrmap(para);

	mctl_set_timing_params(para);

	writel(0, &mctl_ctl->pwrctl);

	setbits_le32(&mctl_ctl->dfiupd[0], BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->zqctl[0], BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x2180, BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x3180, BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->unk_0x4180, BIT(31) | BIT(30));

	setbits_le32(&mctl_ctl->rfshctl3, BIT(0));
	clrbits_le32(&mctl_ctl->dfimisc, BIT(0));

	writel(0, &mctl_com->maer0);
	writel(0, &mctl_com->maer1);
	writel(0, &mctl_com->maer2);

	writel(0x20, &mctl_ctl->pwrctl);
	setbits_le32(&mctl_ctl->clken, BIT(8));

	clrsetbits_le32(&mctl_com->unk_0x500, BIT(24), 0x300);
	/* this write seems to enable PHY MMIO region */
	setbits_le32(&mctl_com->unk_0x500, BIT(24));

	if (!mctl_phy_init(para))
		return false;

	writel(0, &mctl_ctl->swctl);
	clrbits_le32(&mctl_ctl->rfshctl3, BIT(0));

	setbits_le32(&mctl_com->unk_0x014, BIT(31));
	writel(0xffffffff, &mctl_com->maer0);
	writel(0x7ff, &mctl_com->maer1);
	writel(0xffff, &mctl_com->maer2);

	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);

	return true;
}

static bool mctl_core_init(struct dram_para *para)
{
	mctl_sys_init(para);

	return mctl_ctrl_init(para);
}

static void mctl_auto_detect_rank_width(struct dram_para *para)
{
	/* this is minimum size that it's supported */
	para->cols = 8;
	para->rows = 13;

	/*
	 * Strategy here is to test most demanding combination first and least
	 * demanding last, otherwise HW might not be fully utilized. For
	 * example, half bus width and rank = 1 combination would also work
	 * on HW with full bus width and rank = 2, but only 1/4 RAM would be
	 * visible.
	 */

	debug("testing 32-bit width, rank = 2\n");
	para->bus_full_width = 1;
	para->ranks = 2;
	if (mctl_core_init(para))
		return;

	debug("testing 32-bit width, rank = 1\n");
	para->bus_full_width = 1;
	para->ranks = 1;
	if (mctl_core_init(para))
		return;

	debug("testing 16-bit width, rank = 2\n");
	para->bus_full_width = 0;
	para->ranks = 2;
	if (mctl_core_init(para))
		return;

	debug("testing 16-bit width, rank = 1\n");
	para->bus_full_width = 0;
	para->ranks = 1;
	if (mctl_core_init(para))
		return;

	panic("This DRAM setup is currently not supported.\n");
}

static void mctl_auto_detect_dram_size(struct dram_para *para)
{
	/* detect row address bits */
	para->cols = 8;
	para->rows = 18;
	mctl_core_init(para);

	for (para->rows = 13; para->rows < 18; para->rows++) {
		/* 8 banks, 8 bit per byte and 16/32 bit width */
		if (mctl_mem_matches((1 << (para->rows + para->cols +
					    4 + para->bus_full_width))))
			break;
	}

	/* detect column address bits */
	para->cols = 11;
	mctl_core_init(para);

	for (para->cols = 8; para->cols < 11; para->cols++) {
		/* 8 bits per byte and 16/32 bit width */
		if (mctl_mem_matches(1 << (para->cols + 1 +
					   para->bus_full_width)))
			break;
	}
}

static unsigned long mctl_calc_size(struct dram_para *para)
{
	u8 width = para->bus_full_width ? 4 : 2;

	/* 8 banks */
	return (1ULL << (para->cols + para->rows + 3)) * width * para->ranks;
}

unsigned long sunxi_dram_init(void)
{
	struct dram_para para = {
		.clk = CONFIG_DRAM_CLK,
		.type = SUNXI_DRAM_TYPE_DDR3,
	};
	unsigned long size;

	setbits_le32(0x7010310, BIT(8));
	clrbits_le32(0x7010318, 0x3f);

	mctl_auto_detect_rank_width(&para);
	mctl_auto_detect_dram_size(&para);

	mctl_core_init(&para);

	size = mctl_calc_size(&para);

	mctl_set_master_priority();

	return size;
};
