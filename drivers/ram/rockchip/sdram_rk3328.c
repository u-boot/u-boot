// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 */
#include <common.h>
#include <clk.h>
#include <debug_uart.h>
#include <dm.h>
#include <dt-structs.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3328.h>
#include <asm/arch-rockchip/grf_rk3328.h>
#include <asm/arch-rockchip/sdram_common.h>
#include <asm/arch-rockchip/sdram_rk3328.h>
#include <asm/arch-rockchip/uart.h>

struct dram_info {
#ifdef CONFIG_TPL_BUILD
	struct rk3328_ddr_pctl_regs *pctl;
	struct rk3328_ddr_phy_regs *phy;
	struct clk ddr_clk;
	struct rk3328_cru *cru;
	struct rk3328_msch_regs *msch;
	struct rk3328_ddr_grf_regs *ddr_grf;
#endif
	struct ram_info info;
	struct rk3328_grf_regs *grf;
};

#ifdef CONFIG_TPL_BUILD

struct rk3328_sdram_channel sdram_ch;

struct rockchip_dmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3328_dmc dtplat;
#else
	struct rk3328_sdram_params sdram_params;
#endif
	struct regmap *map;
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int conv_of_platdata(struct udevice *dev)
{
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	struct dtd_rockchip_rk3328_dmc *dtplat = &plat->dtplat;
	int ret;

	ret = regmap_init_mem_platdata(dev, dtplat->reg,
				       ARRAY_SIZE(dtplat->reg) / 2,
				       &plat->map);
	if (ret)
		return ret;

	return 0;
}
#endif

static void rkclk_ddr_reset(struct dram_info *dram,
			    u32 ctl_srstn, u32 ctl_psrstn,
			    u32 phy_srstn, u32 phy_psrstn)
{
	writel(ddrctrl_srstn_req(ctl_srstn) | ddrctrl_psrstn_req(ctl_psrstn) |
		ddrphy_srstn_req(phy_srstn) | ddrphy_psrstn_req(phy_psrstn),
		&dram->cru->softrst_con[5]);
	writel(ddrctrl_asrstn_req(ctl_srstn), &dram->cru->softrst_con[9]);
}

static void rkclk_set_dpll(struct dram_info *dram, unsigned int mhz)
{
	unsigned int refdiv, postdiv1, postdiv2, fbdiv;
	int delay = 1000;

	refdiv = 1;
	if (mhz <= 300) {
		postdiv1 = 4;
		postdiv2 = 2;
	} else if (mhz <= 400) {
		postdiv1 = 6;
		postdiv2 = 1;
	} else if (mhz <= 600) {
		postdiv1 = 4;
		postdiv2 = 1;
	} else if (mhz <= 800) {
		postdiv1 = 3;
		postdiv2 = 1;
	} else if (mhz <= 1600) {
		postdiv1 = 2;
		postdiv2 = 1;
	} else {
		postdiv1 = 1;
		postdiv2 = 1;
	}
	fbdiv = (mhz * refdiv * postdiv1 * postdiv2) / 24;

	writel(((0x1 << 4) << 16) | (0 << 4), &dram->cru->mode_con);
	writel(POSTDIV1(postdiv1) | FBDIV(fbdiv), &dram->cru->dpll_con[0]);
	writel(DSMPD(1) | POSTDIV2(postdiv2) | REFDIV(refdiv),
	       &dram->cru->dpll_con[1]);

	while (delay > 0) {
		udelay(1);
		if (LOCK(readl(&dram->cru->dpll_con[1])))
			break;
		delay--;
	}

	writel(((0x1 << 4) << 16) | (1 << 4), &dram->cru->mode_con);
}

static void rkclk_configure_ddr(struct dram_info *dram,
				struct rk3328_sdram_params *sdram_params)
{
	void __iomem *phy_base = dram->phy;

	/* choose DPLL for ddr clk source */
	clrbits_le32(PHY_REG(phy_base, 0xef), 1 << 7);

	/* for inno ddr phy need 2*freq */
	rkclk_set_dpll(dram,  sdram_params->ddr_freq * 2);
}

static void phy_soft_reset(struct dram_info *dram)
{
	void __iomem *phy_base = dram->phy;

	clrbits_le32(PHY_REG(phy_base, 0), 0x3 << 2);
	udelay(1);
	setbits_le32(PHY_REG(phy_base, 0), ANALOG_DERESET);
	udelay(5);
	setbits_le32(PHY_REG(phy_base, 0), DIGITAL_DERESET);
	udelay(1);
}

static int pctl_cfg(struct dram_info *dram,
		    struct rk3328_sdram_params *sdram_params)
{
	u32 i;
	void __iomem *pctl_base = dram->pctl;

	for (i = 0; sdram_params->pctl_regs.pctl[i][0] != 0xFFFFFFFF; i++) {
		writel(sdram_params->pctl_regs.pctl[i][1],
		       pctl_base + sdram_params->pctl_regs.pctl[i][0]);
	}
	clrsetbits_le32(pctl_base + DDR_PCTL2_PWRTMG,
			(0xff << 16) | 0x1f,
			((SR_IDLE & 0xff) << 16) | (PD_IDLE & 0x1f));
	/*
	 * dfi_lp_en_pd=1,dfi_lp_wakeup_pd=2
	 * hw_lp_idle_x32=1
	 */
	if (sdram_params->dramtype == LPDDR3) {
		setbits_le32(pctl_base + DDR_PCTL2_DFILPCFG0, 1);
		clrsetbits_le32(pctl_base + DDR_PCTL2_DFILPCFG0,
				0xf << 4,
				2 << 4);
	}
	clrsetbits_le32(pctl_base + DDR_PCTL2_HWLPCTL,
			0xfff << 16,
			1 << 16);
	/* disable zqcs */
	setbits_le32(pctl_base + DDR_PCTL2_ZQCTL0, 1u << 31);
	setbits_le32(pctl_base + 0x2000 + DDR_PCTL2_ZQCTL0, 1u << 31);

	return 0;
}

/* return ddrconfig value
 *       (-1), find ddrconfig fail
 *       other, the ddrconfig value
 * only support cs0_row >= cs1_row
 */
static unsigned int calculate_ddrconfig(struct rk3328_sdram_params *sdram_params)
{
	static const u16 ddr_cfg_2_rbc[] = {
		/***************************
		 * [5:4]  row(13+n)
		 * [3]    cs(0:0 cs, 1:2 cs)
		 * [2]  bank(0:0bank,1:8bank)
		 * [1:0]    col(11+n)
		 ****************************/
		/* row,        cs,       bank,   col */
		((3 << 4) | (0 << 3) | (1 << 2) | 0),
		((3 << 4) | (0 << 3) | (1 << 2) | 1),
		((2 << 4) | (0 << 3) | (1 << 2) | 2),
		((3 << 4) | (0 << 3) | (1 << 2) | 2),
		((2 << 4) | (0 << 3) | (1 << 2) | 3),
		((3 << 4) | (1 << 3) | (1 << 2) | 0),
		((3 << 4) | (1 << 3) | (1 << 2) | 1),
		((2 << 4) | (1 << 3) | (1 << 2) | 2),
		((3 << 4) | (0 << 3) | (0 << 2) | 1),
		((2 << 4) | (0 << 3) | (1 << 2) | 1),
	};

	static const u16 ddr4_cfg_2_rbc[] = {
		/***************************
		 * [6]	cs 0:0cs 1:2 cs
		 * [5:3]  row(13+n)
		 * [2]  cs(0:0 cs, 1:2 cs)
		 * [1]  bw    0: 16bit 1:32bit
		 * [0]  diebw 0:8bit 1:16bit
		 ***************************/
		/*  cs,       row,        cs,       bw,   diebw */
		((0 << 6) | (3 << 3) | (0 << 2) | (1 << 1) | 0),
		((1 << 6) | (2 << 3) | (0 << 2) | (1 << 1) | 0),
		((0 << 6) | (4 << 3) | (0 << 2) | (0 << 1) | 0),
		((1 << 6) | (3 << 3) | (0 << 2) | (0 << 1) | 0),
		((0 << 6) | (4 << 3) | (0 << 2) | (1 << 1) | 1),
		((1 << 6) | (3 << 3) | (0 << 2) | (1 << 1) | 1),
		((1 << 6) | (4 << 3) | (0 << 2) | (0 << 1) | 1),
		((0 << 6) | (2 << 3) | (1 << 2) | (1 << 1) | 0),
		((0 << 6) | (3 << 3) | (1 << 2) | (0 << 1) | 0),
		((0 << 6) | (3 << 3) | (1 << 2) | (1 << 1) | 1),
		((0 << 6) | (4 << 3) | (1 << 2) | (0 << 1) | 1),
	};

	u32 cs, bw, die_bw, col, row, bank;
	u32 i, tmp;
	u32 ddrconf = -1;

	cs = sdram_ch.rank;
	bw = sdram_ch.bw;
	die_bw = sdram_ch.dbw;
	col = sdram_ch.col;
	row = sdram_ch.cs0_row;
	bank = sdram_ch.bk;

	if (sdram_params->dramtype == DDR4) {
		tmp = ((cs - 1) << 6) | ((row - 13) << 3) | (bw & 0x2) | die_bw;
		for (i = 10; i < 17; i++) {
			if (((tmp & 0x7) == (ddr4_cfg_2_rbc[i - 10] & 0x7)) &&
			    ((tmp & 0x3c) <= (ddr4_cfg_2_rbc[i - 10] & 0x3c)) &&
			    ((tmp & 0x40) <= (ddr4_cfg_2_rbc[i - 10] & 0x40))) {
				ddrconf = i;
				goto out;
			}
		}
	} else {
		if (bank == 2) {
			ddrconf = 8;
			goto out;
		}

		tmp = ((row - 13) << 4) | (1 << 2) | ((bw + col - 11) << 0);
		for (i = 0; i < 5; i++)
			if (((tmp & 0xf) == (ddr_cfg_2_rbc[i] & 0xf)) &&
			    ((tmp & 0x30) <= (ddr_cfg_2_rbc[i] & 0x30))) {
				ddrconf = i;
				goto out;
			}
	}

out:
	if (ddrconf > 20)
		printf("calculate_ddrconfig error\n");

	return ddrconf;
}

/* n: Unit bytes */
static void copy_to_reg(u32 *dest, u32 *src, u32 n)
{
	int i;

	for (i = 0; i < n / sizeof(u32); i++) {
		writel(*src, dest);
		src++;
		dest++;
	}
}

/*******
 * calculate controller dram address map, and setting to register.
 * argument sdram_ch.ddrconf must be right value before
 * call this function.
 *******/
static void set_ctl_address_map(struct dram_info *dram,
				struct rk3328_sdram_params *sdram_params)
{
	void __iomem *pctl_base = dram->pctl;

	copy_to_reg((u32 *)(pctl_base + DDR_PCTL2_ADDRMAP0),
		    &addrmap[sdram_ch.ddrconfig][0], 9 * 4);
	if (sdram_params->dramtype == LPDDR3 && sdram_ch.row_3_4)
		setbits_le32(pctl_base + DDR_PCTL2_ADDRMAP6, 1 << 31);
	if (sdram_params->dramtype == DDR4 && sdram_ch.bw == 0x1)
		setbits_le32(pctl_base + DDR_PCTL2_PCCFG, 1 << 8);

	if (sdram_ch.rank == 1)
		clrsetbits_le32(pctl_base + DDR_PCTL2_ADDRMAP0, 0x1f, 0x1f);
}

static void phy_dll_bypass_set(struct dram_info *dram, u32 freq)
{
	u32 tmp;
	void __iomem *phy_base = dram->phy;

	setbits_le32(PHY_REG(phy_base, 0x13), 1 << 4);
	clrbits_le32(PHY_REG(phy_base, 0x14), 1 << 3);
	setbits_le32(PHY_REG(phy_base, 0x26), 1 << 4);
	clrbits_le32(PHY_REG(phy_base, 0x27), 1 << 3);
	setbits_le32(PHY_REG(phy_base, 0x36), 1 << 4);
	clrbits_le32(PHY_REG(phy_base, 0x37), 1 << 3);
	setbits_le32(PHY_REG(phy_base, 0x46), 1 << 4);
	clrbits_le32(PHY_REG(phy_base, 0x47), 1 << 3);
	setbits_le32(PHY_REG(phy_base, 0x56), 1 << 4);
	clrbits_le32(PHY_REG(phy_base, 0x57), 1 << 3);

	if (freq <= (400 * MHz))
		/* DLL bypass */
		setbits_le32(PHY_REG(phy_base, 0xa4), 0x1f);
	else
		clrbits_le32(PHY_REG(phy_base, 0xa4), 0x1f);
	if (freq <= (680 * MHz))
		tmp = 2;
	else
		tmp = 1;
	writel(tmp, PHY_REG(phy_base, 0x28));
	writel(tmp, PHY_REG(phy_base, 0x38));
	writel(tmp, PHY_REG(phy_base, 0x48));
	writel(tmp, PHY_REG(phy_base, 0x58));
}

static void set_ds_odt(struct dram_info *dram,
		       struct rk3328_sdram_params *sdram_params)
{
	u32 cmd_drv, clk_drv, dqs_drv, dqs_odt;
	void __iomem *phy_base = dram->phy;

	if (sdram_params->dramtype == DDR3) {
		cmd_drv = PHY_DDR3_RON_RTT_34ohm;
		clk_drv = PHY_DDR3_RON_RTT_45ohm;
		dqs_drv = PHY_DDR3_RON_RTT_34ohm;
		dqs_odt = PHY_DDR3_RON_RTT_225ohm;
	} else {
		cmd_drv = PHY_DDR4_LPDDR3_RON_RTT_34ohm;
		clk_drv = PHY_DDR4_LPDDR3_RON_RTT_43ohm;
		dqs_drv = PHY_DDR4_LPDDR3_RON_RTT_34ohm;
		dqs_odt = PHY_DDR4_LPDDR3_RON_RTT_240ohm;
	}
	/* DS */
	writel(cmd_drv, PHY_REG(phy_base, 0x11));
	clrsetbits_le32(PHY_REG(phy_base, 0x12), 0x1f << 3, cmd_drv << 3);
	writel(clk_drv, PHY_REG(phy_base, 0x16));
	writel(clk_drv, PHY_REG(phy_base, 0x18));
	writel(dqs_drv, PHY_REG(phy_base, 0x20));
	writel(dqs_drv, PHY_REG(phy_base, 0x2f));
	writel(dqs_drv, PHY_REG(phy_base, 0x30));
	writel(dqs_drv, PHY_REG(phy_base, 0x3f));
	writel(dqs_drv, PHY_REG(phy_base, 0x40));
	writel(dqs_drv, PHY_REG(phy_base, 0x4f));
	writel(dqs_drv, PHY_REG(phy_base, 0x50));
	writel(dqs_drv, PHY_REG(phy_base, 0x5f));
	/* ODT */
	writel(dqs_odt, PHY_REG(phy_base, 0x21));
	writel(dqs_odt, PHY_REG(phy_base, 0x2e));
	writel(dqs_odt, PHY_REG(phy_base, 0x31));
	writel(dqs_odt, PHY_REG(phy_base, 0x3e));
	writel(dqs_odt, PHY_REG(phy_base, 0x41));
	writel(dqs_odt, PHY_REG(phy_base, 0x4e));
	writel(dqs_odt, PHY_REG(phy_base, 0x51));
	writel(dqs_odt, PHY_REG(phy_base, 0x5e));
}

static void phy_cfg(struct dram_info *dram,
		    struct rk3328_sdram_params *sdram_params)
{
	u32 i;
	void __iomem *phy_base = dram->phy;

	phy_dll_bypass_set(dram, sdram_params->ddr_freq);
	for (i = 0; sdram_params->phy_regs.phy[i][0] != 0xFFFFFFFF; i++) {
		writel(sdram_params->phy_regs.phy[i][1],
		       phy_base + sdram_params->phy_regs.phy[i][0]);
	}
	if (sdram_ch.bw == 2) {
		clrsetbits_le32(PHY_REG(phy_base, 0), 0xf << 4, 0xf << 4);
	} else {
		clrsetbits_le32(PHY_REG(phy_base, 0), 0xf << 4, 3 << 4);
		/* disable DQS2,DQS3 tx dll  for saving power */
		clrbits_le32(PHY_REG(phy_base, 0x46), 1 << 3);
		clrbits_le32(PHY_REG(phy_base, 0x56), 1 << 3);
	}
	set_ds_odt(dram, sdram_params);
	/* deskew */
	setbits_le32(PHY_REG(phy_base, 2), 8);
	copy_to_reg(PHY_REG(phy_base, 0xb0),
		    &sdram_params->skew.a0_a1_skew[0], 15 * 4);
	copy_to_reg(PHY_REG(phy_base, 0x70),
		    &sdram_params->skew.cs0_dm0_skew[0], 44 * 4);
	copy_to_reg(PHY_REG(phy_base, 0xc0),
		    &sdram_params->skew.cs0_dm1_skew[0], 44 * 4);
}

static int update_refresh_reg(struct dram_info *dram)
{
	void __iomem *pctl_base = dram->pctl;
	u32 ret;

	ret = readl(pctl_base + DDR_PCTL2_RFSHCTL3) ^ (1 << 1);
	writel(ret, pctl_base + DDR_PCTL2_RFSHCTL3);

	return 0;
}

static int data_training(struct dram_info *dram, u32 cs, u32 dramtype)
{
	u32 ret;
	u32 dis_auto_zq = 0;
	void __iomem *pctl_base = dram->pctl;
	void __iomem *phy_base = dram->phy;

	/* disable zqcs */
	if (!(readl(pctl_base + DDR_PCTL2_ZQCTL0) &
		(1ul << 31))) {
		dis_auto_zq = 1;
		setbits_le32(pctl_base + DDR_PCTL2_ZQCTL0, 1 << 31);
	}
	/* disable auto refresh */
	setbits_le32(pctl_base + DDR_PCTL2_RFSHCTL3, 1);
	update_refresh_reg(dram);

	if (dramtype == DDR4) {
		clrsetbits_le32(PHY_REG(phy_base, 0x29), 0x3, 0);
		clrsetbits_le32(PHY_REG(phy_base, 0x39), 0x3, 0);
		clrsetbits_le32(PHY_REG(phy_base, 0x49), 0x3, 0);
		clrsetbits_le32(PHY_REG(phy_base, 0x59), 0x3, 0);
	}
	/* choose training cs */
	clrsetbits_le32(PHY_REG(phy_base, 2), 0x33, (0x20 >> cs));
	/* enable gate training */
	clrsetbits_le32(PHY_REG(phy_base, 2), 0x33, (0x20 >> cs) | 1);
	udelay(50);
	ret = readl(PHY_REG(phy_base, 0xff));
	/* disable gate training */
	clrsetbits_le32(PHY_REG(phy_base, 2), 0x33, (0x20 >> cs) | 0);
	/* restore zqcs */
	if (dis_auto_zq)
		clrbits_le32(pctl_base + DDR_PCTL2_ZQCTL0, 1 << 31);
	/* restore auto refresh */
	clrbits_le32(pctl_base + DDR_PCTL2_RFSHCTL3, 1);
	update_refresh_reg(dram);

	if (dramtype == DDR4) {
		clrsetbits_le32(PHY_REG(phy_base, 0x29), 0x3, 0x2);
		clrsetbits_le32(PHY_REG(phy_base, 0x39), 0x3, 0x2);
		clrsetbits_le32(PHY_REG(phy_base, 0x49), 0x3, 0x2);
		clrsetbits_le32(PHY_REG(phy_base, 0x59), 0x3, 0x2);
	}

	if (ret & 0x10) {
		ret = -1;
	} else {
		ret = (ret & 0xf) ^ (readl(PHY_REG(phy_base, 0)) >> 4);
		ret = (ret == 0) ? 0 : -1;
	}
	return ret;
}

/* rank = 1: cs0
 * rank = 2: cs1
 * rank = 3: cs0 & cs1
 * note: be careful of keep mr original val
 */
static int write_mr(struct dram_info *dram, u32 rank, u32 mr_num, u32 arg,
		    u32 dramtype)
{
	void __iomem *pctl_base = dram->pctl;

	while (readl(pctl_base + DDR_PCTL2_MRSTAT) & MR_WR_BUSY)
		continue;
	if (dramtype == DDR3 || dramtype == DDR4) {
		writel((mr_num << 12) | (rank << 4) | (0 << 0),
		       pctl_base + DDR_PCTL2_MRCTRL0);
		writel(arg, pctl_base + DDR_PCTL2_MRCTRL1);
	} else {
		writel((rank << 4) | (0 << 0),
		       pctl_base + DDR_PCTL2_MRCTRL0);
		writel((mr_num << 8) | (arg & 0xff),
		       pctl_base + DDR_PCTL2_MRCTRL1);
	}

	setbits_le32(pctl_base + DDR_PCTL2_MRCTRL0, 1u << 31);
	while (readl(pctl_base + DDR_PCTL2_MRCTRL0) & (1u << 31))
		continue;
	while (readl(pctl_base + DDR_PCTL2_MRSTAT) & MR_WR_BUSY)
		continue;

	return 0;
}

/*
 * rank : 1:cs0, 2:cs1, 3:cs0&cs1
 * vrefrate: 4500: 45%,
 */
static int write_vrefdq(struct dram_info *dram, u32 rank, u32 vrefrate,
			u32 dramtype)
{
	u32 tccd_l, value;
	u32 dis_auto_zq = 0;
	void __iomem *pctl_base = dram->pctl;

	if (dramtype != DDR4 || vrefrate < 4500 || vrefrate > 9200)
		return -1;

	tccd_l = (readl(pctl_base + DDR_PCTL2_DRAMTMG4) >> 16) & 0xf;
	tccd_l = (tccd_l - 4) << 10;

	if (vrefrate > 7500) {
		/* range 1 */
		value = ((vrefrate - 6000) / 65) | tccd_l;
	} else {
		/* range 2 */
		value = ((vrefrate - 4500) / 65) | tccd_l | (1 << 6);
	}

	/* disable zqcs */
	if (!(readl(pctl_base + DDR_PCTL2_ZQCTL0) &
		(1ul << 31))) {
		dis_auto_zq = 1;
		setbits_le32(pctl_base + DDR_PCTL2_ZQCTL0, 1 << 31);
	}
	/* disable auto refresh */
	setbits_le32(pctl_base + DDR_PCTL2_RFSHCTL3, 1);
	update_refresh_reg(dram);

	/* enable vrefdq calibratin */
	write_mr(dram, rank, 6, value | (1 << 7), dramtype);
	udelay(1);/* tvrefdqe */
	/* write vrefdq value */
	write_mr(dram, rank, 6, value | (1 << 7), dramtype);
	udelay(1);/* tvref_time */
	write_mr(dram, rank, 6, value | (0 << 7), dramtype);
	udelay(1);/* tvrefdqx */

	/* restore zqcs */
	if (dis_auto_zq)
		clrbits_le32(pctl_base + DDR_PCTL2_ZQCTL0, 1 << 31);
	/* restore auto refresh */
	clrbits_le32(pctl_base + DDR_PCTL2_RFSHCTL3, 1);
	update_refresh_reg(dram);

	return 0;
}

#define _MAX_(x, y) ((x) > (y) ? (x) : (y))

static void rx_deskew_switch_adjust(struct dram_info *dram)
{
	u32 i, deskew_val;
	u32 gate_val = 0;
	void __iomem *phy_base = dram->phy;

	for (i = 0; i < 4; i++)
		gate_val = _MAX_(readl(PHY_REG(phy_base, 0xfb + i)), gate_val);

	deskew_val = (gate_val >> 3) + 1;
	deskew_val = (deskew_val > 0x1f) ? 0x1f : deskew_val;
	clrsetbits_le32(PHY_REG(phy_base, 0x6e), 0xc, (deskew_val & 0x3) << 2);
	clrsetbits_le32(PHY_REG(phy_base, 0x6f), 0x7 << 4,
			(deskew_val & 0x1c) << 2);
}

#undef _MAX_

static void tx_deskew_switch_adjust(struct dram_info *dram)
{
	void __iomem *phy_base = dram->phy;

	clrsetbits_le32(PHY_REG(phy_base, 0x6e), 0x3, 1);
}

static void set_ddrconfig(struct dram_info *dram, u32 ddrconfig)
{
	writel(ddrconfig, &dram->msch->ddrconf);
}

static void dram_all_config(struct dram_info *dram,
			    struct rk3328_sdram_params *sdram_params)
{
	u32 sys_reg = 0, tmp = 0;

	set_ddrconfig(dram, sdram_ch.ddrconfig);

	sys_reg |= SYS_REG_ENC_DDRTYPE(sdram_params->dramtype);
	sys_reg |= SYS_REG_ENC_ROW_3_4(sdram_ch.row_3_4, 0);
	sys_reg |= SYS_REG_ENC_RANK(sdram_ch.rank, 0);
	sys_reg |= SYS_REG_ENC_COL(sdram_ch.col, 0);
	sys_reg |= SYS_REG_ENC_BK(sdram_ch.bk, 0);
	SYS_REG_ENC_CS0_ROW(sdram_ch.cs0_row, sys_reg, tmp, 0);
	if (sdram_ch.cs1_row)
		SYS_REG_ENC_CS1_ROW(sdram_ch.cs1_row, sys_reg, tmp, 0);
	sys_reg |= SYS_REG_ENC_BW(sdram_ch.bw, 0);
	sys_reg |= SYS_REG_ENC_DBW(sdram_ch.dbw, 0);

	writel(sys_reg, &dram->grf->os_reg[2]);

	writel(sdram_ch.noc_timings.ddrtiming.d32, &dram->msch->ddrtiming);

	writel(sdram_ch.noc_timings.ddrmode.d32, &dram->msch->ddrmode);
	writel(sdram_ch.noc_timings.readlatency, &dram->msch->readlatency);

	writel(sdram_ch.noc_timings.activate.d32, &dram->msch->activate);
	writel(sdram_ch.noc_timings.devtodev.d32, &dram->msch->devtodev);
	writel(sdram_ch.noc_timings.ddr4timing.d32, &dram->msch->ddr4_timing);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging0);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging1);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging2);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging3);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging4);
	writel(sdram_ch.noc_timings.agingx0, &dram->msch->aging5);
}

static void enable_low_power(struct dram_info *dram,
			     struct rk3328_sdram_params *sdram_params)
{
	void __iomem *pctl_base = dram->pctl;

	/* enable upctl2 axi clock auto gating */
	writel(0x00800000, &dram->ddr_grf->ddr_grf_con[0]);
	writel(0x20012001, &dram->ddr_grf->ddr_grf_con[2]);
	/* enable upctl2 core clock auto gating */
	writel(0x001e001a, &dram->ddr_grf->ddr_grf_con[2]);
	/* enable sr, pd */
	if (PD_IDLE == 0)
		clrbits_le32(pctl_base + DDR_PCTL2_PWRCTL, (1 << 1));
	else
		setbits_le32(pctl_base + DDR_PCTL2_PWRCTL, (1 << 1));
	if (SR_IDLE == 0)
		clrbits_le32(pctl_base + DDR_PCTL2_PWRCTL,	1);
	else
		setbits_le32(pctl_base + DDR_PCTL2_PWRCTL, 1);
	setbits_le32(pctl_base + DDR_PCTL2_PWRCTL, (1 << 3));
}

static int sdram_init(struct dram_info *dram,
		      struct rk3328_sdram_params *sdram_params, u32 pre_init)
{
	void __iomem *pctl_base = dram->pctl;

	rkclk_ddr_reset(dram, 1, 1, 1, 1);
	udelay(10);
	/*
	 * dereset ddr phy psrstn to config pll,
	 * if using phy pll psrstn must be dereset
	 * before config pll
	 */
	rkclk_ddr_reset(dram, 1, 1, 1, 0);
	rkclk_configure_ddr(dram, sdram_params);
	if (pre_init == 0) {
		switch (sdram_params->dramtype) {
		case DDR3:
			printf("DDR3\n");
			break;
		case DDR4:
			printf("DDR4\n");
			break;
		case LPDDR3:
		default:
			printf("LPDDR3\n");
			break;
		}
	}
	/* release phy srst to provide clk to ctrl */
	rkclk_ddr_reset(dram, 1, 1, 0, 0);
	udelay(10);
	phy_soft_reset(dram);
	/* release ctrl presetn, and config ctl registers */
	rkclk_ddr_reset(dram, 1, 0, 0, 0);
	pctl_cfg(dram, sdram_params);
	sdram_ch.ddrconfig = calculate_ddrconfig(sdram_params);
	set_ctl_address_map(dram, sdram_params);
	phy_cfg(dram, sdram_params);

	/* enable dfi_init_start to init phy after ctl srstn deassert */
	setbits_le32(pctl_base + DDR_PCTL2_DFIMISC, (1 << 5) | (1 << 4));
	rkclk_ddr_reset(dram, 0, 0, 0, 0);
	/* wait for dfi_init_done and dram init complete */
	while ((readl(pctl_base + DDR_PCTL2_STAT) & 0x7) == 0)
		continue;

	/* do ddr gate training */
	if (data_training(dram, 0, sdram_params->dramtype) != 0) {
		printf("data training error\n");
		return -1;
	}

	if (sdram_params->dramtype == DDR4)
		write_vrefdq(dram, 0x3, 5670, sdram_params->dramtype);

	if (pre_init == 0) {
		rx_deskew_switch_adjust(dram);
		tx_deskew_switch_adjust(dram);
	}

	dram_all_config(dram, sdram_params);
	enable_low_power(dram, sdram_params);

	return 0;
}

static u64 dram_detect_cap(struct dram_info *dram,
			   struct rk3328_sdram_params *sdram_params,
			   unsigned char channel)
{
	void __iomem *pctl_base = dram->pctl;

	/*
	 * for ddr3: ddrconf = 3
	 * for ddr4: ddrconf = 12
	 * for lpddr3: ddrconf = 3
	 * default bw = 1
	 */
	u32 bk, bktmp;
	u32 col, coltmp;
	u32 row, rowtmp, row_3_4;
	void __iomem *test_addr, *test_addr1;
	u32 dbw;
	u32 cs;
	u32 bw = 1;
	u64 cap = 0;
	u32 dram_type = sdram_params->dramtype;
	u32 pwrctl;

	if (dram_type != DDR4) {
		/* detect col and bk for ddr3/lpddr3 */
		coltmp = 12;
		bktmp = 3;
		rowtmp = 16;

		for (col = coltmp; col >= 9; col -= 1) {
			writel(0, SDRAM_ADDR);
			test_addr = (void __iomem *)(SDRAM_ADDR +
					(1ul << (col + bw - 1ul)));
			writel(PATTERN, test_addr);
			if ((readl(test_addr) == PATTERN) &&
			    (readl(SDRAM_ADDR) == 0))
				break;
		}
		if (col == 8) {
			printf("col error\n");
			goto cap_err;
		}

		test_addr = (void __iomem *)(SDRAM_ADDR +
				(1ul << (coltmp + bktmp + bw - 1ul)));
		writel(0, SDRAM_ADDR);
		writel(PATTERN, test_addr);
		if ((readl(test_addr) == PATTERN) &&
		    (readl(SDRAM_ADDR) == 0))
			bk = 3;
		else
			bk = 2;
		if (dram_type == LPDDR3)
			dbw = 2;
		else
			dbw = 1;
	} else {
		/* detect bg for ddr4 */
		coltmp = 10;
		bktmp = 4;
		rowtmp = 17;

		col = 10;
		bk = 2;
		test_addr = (void __iomem *)(SDRAM_ADDR +
				(1ul << (coltmp + bw + 1ul)));
		writel(0, SDRAM_ADDR);
		writel(PATTERN, test_addr);
		if ((readl(test_addr) == PATTERN) &&
		    (readl(SDRAM_ADDR) == 0))
			dbw = 0;
		else
			dbw = 1;
	}
	/* detect row */
	for (row = rowtmp; row > 12; row--) {
		writel(0, SDRAM_ADDR);
		test_addr = (void __iomem *)(SDRAM_ADDR +
				(1ul << (row + bktmp + coltmp + bw - 1ul)));
		writel(PATTERN, test_addr);
		if ((readl(test_addr) == PATTERN) &&
		    (readl(SDRAM_ADDR) == 0))
			break;
	}
	if (row == 12) {
		printf("row error");
		goto cap_err;
	}
	/* detect row_3_4 */
	test_addr = SDRAM_ADDR;
	test_addr1 = (void __iomem *)(SDRAM_ADDR +
			(0x3ul << (row + bktmp + coltmp + bw - 1ul - 1ul)));

	writel(0, test_addr);
	writel(PATTERN, test_addr1);
	if ((readl(test_addr) == 0) &&
	    (readl(test_addr1) == PATTERN))
		row_3_4 = 0;
	else
		row_3_4 = 1;

	/* disable auto low-power */
	pwrctl = readl(pctl_base + DDR_PCTL2_PWRCTL);
	writel(0, pctl_base + DDR_PCTL2_PWRCTL);

	/* bw and cs detect using phy read gate training */
	if (data_training(dram, 1, dram_type) == 0)
		cs = 1;
	else
		cs = 0;

	bw = 2;

	/* restore auto low-power */
	writel(pwrctl, pctl_base + DDR_PCTL2_PWRCTL);

	sdram_ch.rank = cs + 1;
	sdram_ch.col = col;
	sdram_ch.bk = bk;
	sdram_ch.dbw = dbw;
	sdram_ch.bw = bw;
	sdram_ch.cs0_row = row;
	if (cs)
		sdram_ch.cs1_row = row;
	else
		sdram_ch.cs1_row = 0;
	sdram_ch.row_3_4 = row_3_4;

	if (dram_type == DDR4)
		cap = 1llu << (cs + row + bk + col + ((dbw == 0) ? 2 : 1) + bw);
	else
		cap = 1llu << (cs + row + bk + col + bw);

	return cap;

cap_err:
	return 0;
}

static u32 remodify_sdram_params(struct rk3328_sdram_params *sdram_params)
{
	u32 tmp = 0, tmp_adr = 0, i;

	for (i = 0; sdram_params->pctl_regs.pctl[i][0] != 0xFFFFFFFF; i++) {
		if (sdram_params->pctl_regs.pctl[i][0] == 0) {
			tmp = sdram_params->pctl_regs.pctl[i][1];/* MSTR */
			tmp_adr = i;
		}
	}

	tmp &= ~((3ul << 30) | (3ul << 24) | (3ul << 12));

	switch (sdram_ch.dbw) {
	case 2:
		tmp |= (3ul << 30);
		break;
	case 1:
		tmp |= (2ul << 30);
		break;
	case 0:
	default:
		tmp |= (1ul << 30);
		break;
	}

	if (sdram_ch.rank == 2)
		tmp |= 3 << 24;
	else
		tmp |= 1 << 24;

	tmp |= (2 - sdram_ch.bw) << 12;

	sdram_params->pctl_regs.pctl[tmp_adr][1] = tmp;

	if (sdram_ch.bw == 2)
		sdram_ch.noc_timings.ddrtiming.b.bwratio = 0;
	else
		sdram_ch.noc_timings.ddrtiming.b.bwratio = 1;

	return 0;
}

static int dram_detect_cs1_row(struct rk3328_sdram_params *sdram_params,
			       unsigned char channel)
{
	u32 ret = 0;
	u32 cs1_bit;
	void __iomem *test_addr, *cs1_addr;
	u32 row, bktmp, coltmp, bw;
	u32 ddrconf = sdram_ch.ddrconfig;

	if (sdram_ch.rank == 2) {
		cs1_bit = addrmap[ddrconf][0] + 8;

		if (cs1_bit > 31)
			goto out;

		cs1_addr = (void __iomem *)(1ul << cs1_bit);
		if (cs1_bit < 20)
			cs1_bit = 1;
		else
			cs1_bit = 0;

		if (sdram_params->dramtype == DDR4) {
			if (sdram_ch.dbw == 0)
				bktmp = sdram_ch.bk + 2;
			else
				bktmp = sdram_ch.bk + 1;
		} else {
			bktmp = sdram_ch.bk;
		}
		bw = sdram_ch.bw;
		coltmp = sdram_ch.col;

		/* detect cs1 row */
		for (row = sdram_ch.cs0_row; row > 12; row--) {
			test_addr = (void __iomem *)(SDRAM_ADDR + cs1_addr +
					(1ul << (row + cs1_bit + bktmp +
					 coltmp + bw - 1ul)));
			writel(0, SDRAM_ADDR + cs1_addr);
			writel(PATTERN, test_addr);
			if ((readl(test_addr) == PATTERN) &&
			    (readl(SDRAM_ADDR + cs1_addr) == 0)) {
				ret = row;
				break;
			}
		}
	}

out:
	return ret;
}

static int sdram_init_detect(struct dram_info *dram,
			     struct rk3328_sdram_params *sdram_params)
{
	debug("Starting SDRAM initialization...\n");

	memcpy(&sdram_ch, &sdram_params->ch,
	       sizeof(struct rk3328_sdram_channel));

	sdram_init(dram, sdram_params, 1);
	dram_detect_cap(dram, sdram_params, 0);

	/* modify bw, cs related timing */
	remodify_sdram_params(sdram_params);
	/* reinit sdram by real dram cap */
	sdram_init(dram, sdram_params, 0);

	/* redetect cs1 row */
	sdram_ch.cs1_row =
		dram_detect_cs1_row(sdram_params, 0);

	return 0;
}

static int rk3328_dmc_init(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	int ret;

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3328_sdram_params *params = &plat->sdram_params;
#else
	struct dtd_rockchip_rk3328_dmc *dtplat = &plat->dtplat;
	struct rk3328_sdram_params *params =
					(void *)dtplat->rockchip_sdram_params;

	ret = conv_of_platdata(dev);
	if (ret)
		return ret;
#endif
	priv->phy = regmap_get_range(plat->map, 0);
	priv->pctl = regmap_get_range(plat->map, 1);
	priv->grf = regmap_get_range(plat->map, 2);
	priv->cru = regmap_get_range(plat->map, 3);
	priv->msch = regmap_get_range(plat->map, 4);
	priv->ddr_grf = regmap_get_range(plat->map, 5);

	debug("%s phy %p pctrl %p grf %p cru %p msch %p ddr_grf %p\n",
	      __func__, priv->phy, priv->pctl, priv->grf, priv->cru,
	      priv->msch, priv->ddr_grf);
	ret = sdram_init_detect(priv, params);
	if (ret < 0) {
		printf("%s DRAM init failed%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int rk3328_dmc_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = dev_read_u32_array(dev, "rockchip,sdram-params",
				 (u32 *)&plat->sdram_params,
				 sizeof(plat->sdram_params) / sizeof(u32));
	if (ret) {
		printf("%s: Cannot read rockchip,sdram-params %d\n",
		       __func__, ret);
		return ret;
	}
	ret = regmap_init_mem(dev, &plat->map);
	if (ret)
		printf("%s: regmap failed %d\n", __func__, ret);
#endif
	return 0;
}

#endif

static int rk3328_dmc_probe(struct udevice *dev)
{
#ifdef CONFIG_TPL_BUILD
	if (rk3328_dmc_init(dev))
		return 0;
#else
	struct dram_info *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	debug("%s: grf=%p\n", __func__, priv->grf);
	priv->info.base = CONFIG_SYS_SDRAM_BASE;
	priv->info.size = rockchip_sdram_size(
				(phys_addr_t)&priv->grf->os_reg[2]);
#endif
	return 0;
}

static int rk3328_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3328_dmc_ops = {
	.get_info = rk3328_dmc_get_info,
};

static const struct udevice_id rk3328_dmc_ids[] = {
	{ .compatible = "rockchip,rk3328-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3328) = {
	.name = "rockchip_rk3328_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3328_dmc_ids,
	.ops = &rk3328_dmc_ops,
#ifdef CONFIG_TPL_BUILD
	.ofdata_to_platdata = rk3328_dmc_ofdata_to_platdata,
#endif
	.probe = rk3328_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
#ifdef CONFIG_TPL_BUILD
	.platdata_auto_alloc_size = sizeof(struct rockchip_dmc_plat),
#endif
};
