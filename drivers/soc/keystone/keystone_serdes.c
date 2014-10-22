/*
 * TI serdes driver for keystone2.
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

#define SERDES_LANE_REGS(x)		(0x0200 + (0x200 * (x)))

struct serdes_cfg {
	u32 ofs;
	u32 val;
	u32 mask;
};

static struct serdes_cfg cfg_cmu_156p25m_5g[] = {
	{0x0000, 0x00800000, 0xffff0000},
	{0x0014, 0x00008282, 0x0000ffff},
	{0x0060, 0x00142438, 0x00ffffff},
	{0x0064, 0x00c3c700, 0x00ffff00},
	{0x0078, 0x0000c000, 0x0000ff00}
};

static struct serdes_cfg cfg_comlane_156p25m_5g[] = {
	{0x0a00, 0x00000800, 0x0000ff00},
	{0x0a08, 0x38a20000, 0xffff0000},
	{0x0a30, 0x008a8a00, 0x00ffff00},
	{0x0a84, 0x00000600, 0x0000ff00},
	{0x0a94, 0x10000000, 0xff000000},
	{0x0aa0, 0x81000000, 0xff000000},
	{0x0abc, 0xff000000, 0xff000000},
	{0x0ac0, 0x0000008b, 0x000000ff},
	{0x0b08, 0x583f0000, 0xffff0000},
	{0x0b0c, 0x0000004e, 0x000000ff}
};

static struct serdes_cfg cfg_lane_156p25mhz_5g[] = {
	{0x0004, 0x38000080, 0xff0000ff},
	{0x0008, 0x00000000, 0x000000ff},
	{0x000c, 0x02000000, 0xff000000},
	{0x0010, 0x1b000000, 0xff000000},
	{0x0014, 0x00006fb8, 0x0000ffff},
	{0x0018, 0x758000e4, 0xffff00ff},
	{0x00ac, 0x00004400, 0x0000ff00},
	{0x002c, 0x00100800, 0x00ffff00},
	{0x0080, 0x00820082, 0x00ff00ff},
	{0x0084, 0x1d0f0385, 0xffffffff}

};

static inline void ks2_serdes_rmw(u32 addr, u32 value, u32 mask)
{
	writel(((readl(addr) & (~mask)) | (value & mask)), addr);
}

static void ks2_serdes_cfg_setup(u32 base, struct serdes_cfg *cfg, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++)
		ks2_serdes_rmw(base + cfg[i].ofs, cfg[i].val, cfg[i].mask);
}

static void ks2_serdes_lane_config(u32 base, struct serdes_cfg *cfg_lane,
				   u32 size, u32 lane)
{
	u32 i;

	for (i = 0; i < size; i++)
		ks2_serdes_rmw(base + cfg_lane[i].ofs + SERDES_LANE_REGS(lane),
			       cfg_lane[i].val, cfg_lane[i].mask);
}

static int ks2_serdes_init_156p25m_5g(u32 base, u32 num_lanes)
{
	u32 i;

	ks2_serdes_cfg_setup(base, cfg_cmu_156p25m_5g,
			     ARRAY_SIZE(cfg_cmu_156p25m_5g));
	ks2_serdes_cfg_setup(base, cfg_comlane_156p25m_5g,
			     ARRAY_SIZE(cfg_comlane_156p25m_5g));

	for (i = 0; i < num_lanes; i++)
		ks2_serdes_lane_config(base, cfg_lane_156p25mhz_5g,
				       ARRAY_SIZE(cfg_lane_156p25mhz_5g), i);

	return 0;
}

void ks2_serdes_sgmii_156p25mhz_setup(void)
{
	unsigned int cnt;

	ks2_serdes_init_156p25m_5g(CONFIG_KS2_SERDES_SGMII_BASE,
				   CONFIG_KS2_SERDES_LANES_PER_SGMII);

	/*Bring SerDes out of Reset if SerDes is Shutdown & is in Reset Mode*/
	clrbits_le32(0x0232a010, 1 << 28);

	/* Enable TX and RX via the LANExCTL_STS 0x0000 + x*4 */
	clrbits_le32(0x0232a228, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe0);
	clrbits_le32(0x0232a428, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe4);
	clrbits_le32(0x0232a628, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe8);
	clrbits_le32(0x0232a828, 1 << 29);
	writel(0xF800F8C0, 0x0232bfec);

	/*Enable pll via the pll_ctrl 0x0014*/
	writel(0xe0000000, 0x0232bff4)
		;

	/*Waiting for SGMII Serdes PLL lock.*/
	for (cnt = 10000; cnt > 0 && ((readl(0x02090114) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090214) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090414) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090514) & 0x10) == 0); cnt--)
		;

	udelay(45000);
}
