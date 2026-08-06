// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/mii.h>

#define REG_EPHY_TOP_WRAP (u32 *)0x03009800
#define REG_EPHY_BASE     (u32 *)0x03009000

#define REG_EPHY_CTL         REG_EPHY_TOP_WRAP
#define REG_EPHY_APB_RW_SEL  REG_EPHY_TOP_WRAP + 1

/* Page 0 register */
#define REG_PHY_ID1          REG_EPHY_BASE + MII_PHYSID1
#define REG_PHY_ID2          REG_EPHY_BASE + MII_PHYSID2
#define REG_PHY_PAGE_SEL     REG_EPHY_BASE + 0x1f

/* Page 5 register */
#define REG_PD_EN_CTL        REG_EPHY_BASE + 0x10

/* REG_EPHY_CTL */
#define REG_EPHY_SHUTDOWN    BIT(0)
#define REG_EPHY_ANA_RST_N   BIT(1)
#define REG_EPHY_DIG_RST_N   BIT(2)
#define REG_EPHY_MAIN_RST_N  BIT(3)

/* REG_PD_EN_CTL */
#define REG_EN_ETH_TXRT          BIT(0)
#define REG_EN_ETH_CLK100M       BIT(1)
#define REG_EN_ETH_CLK125M       BIT(2)
#define REG_EN_ETH_PLL_LCKDET    BIT(3)
#define REG_EN_ETH_RXADC         BIT(4)
#define REG_EN_ETH_RXPGA         BIT(5)
#define REG_EN_ETH_RXRT          BIT(6)
#define REG_EN_ETH_TXCROSSOVER   BIT(7)
#define REG_PD_ETH_PLL           BIT(8)
#define REG_PD_ETH_TXDAC         BIT(9)
#define REG_PD_ETH_TXDACBST      BIT(10)
#define REG_PD_ETH_TXECHO        BIT(11)
#define REG_PD_ETH_TXDRV_NMOS    BIT(12)
#define REG_PD_ETH_TXLDO         BIT(13)

void cv1800b_ephy_init(void)
{
	u32 reg;
	u32 phy_id = 1;

	/* enable direct memory access for phy register */
	writel(1, REG_EPHY_APB_RW_SEL);

	reg = readl(REG_EPHY_CTL);
	reg &= ~REG_EPHY_SHUTDOWN;
	reg |= REG_EPHY_ANA_RST_N | REG_EPHY_DIG_RST_N | REG_EPHY_MAIN_RST_N;
	writel(reg, REG_EPHY_CTL);

	/* switch to page 5 */
	writel(5 << 8, REG_PHY_PAGE_SEL);
	reg = readl(REG_PD_EN_CTL);
	reg &= ~(REG_PD_ETH_TXLDO | REG_PD_ETH_TXDRV_NMOS | REG_PD_ETH_TXDAC | REG_PD_ETH_PLL);
	reg |= REG_EN_ETH_TXRT | REG_EN_ETH_CLK100M | REG_EN_ETH_CLK125M
		| REG_EN_ETH_PLL_LCKDET | REG_EN_ETH_RXADC | REG_EN_ETH_RXPGA | REG_EN_ETH_RXRT;
	writel(reg, REG_PD_EN_CTL);

	/* switch to page 0 */
	writel(0 << 8, REG_PHY_PAGE_SEL);
	/*
	 * As the phy_id in the cv1800b PHY register is initialized to 0, it
	 * is necessary to manually initialize the phy_id to an arbitrary
	 * value so that it could corresponds to the generic PHY driver.
	 */
	writel(phy_id >> 16, REG_PHY_ID1);
	writel(phy_id & 0xffff, REG_PHY_ID2);

	/* switch to MDIO control */
	writel(0, REG_EPHY_APB_RW_SEL);
}
