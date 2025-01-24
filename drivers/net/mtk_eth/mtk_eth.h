/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#ifndef _MTK_ETH_H_
#define _MTK_ETH_H_

#include <linker_lists.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>

struct mtk_eth_priv;
struct mtk_eth_switch_priv;

/* struct mtk_soc_data -	This is the structure holding all differences
 *				among various plaforms
 * @caps			Flags shown the extra capability for the SoC
 * @ana_rgc3:			The offset for register ANA_RGC3 related to
 *				sgmiisys syscon
 * @gdma_count:			Number of GDMAs
 * @pdma_base:			Register base of PDMA block
 * @txd_size:			Tx DMA descriptor size.
 * @rxd_size:			Rx DMA descriptor size.
 */
struct mtk_soc_data {
	u32 caps;
	u32 ana_rgc3;
	u32 gdma_count;
	u32 pdma_base;
	u32 txd_size;
	u32 rxd_size;
};

struct mtk_eth_switch {
	const char *name;
	const char *desc;
	size_t priv_size;
	u32 reset_wait_time;

	int (*detect)(struct mtk_eth_priv *priv);
	int (*setup)(struct mtk_eth_switch_priv *priv);
	int (*cleanup)(struct mtk_eth_switch_priv *priv);
	void (*mac_control)(struct mtk_eth_switch_priv *priv, bool enable);
};

#define MTK_ETH_SWITCH(__name)	\
	ll_entry_declare(struct mtk_eth_switch, __name, mtk_eth_switch)

struct mtk_eth_switch_priv {
	struct mtk_eth_priv *eth;
	const struct mtk_eth_switch *sw;
	const struct mtk_soc_data *soc;
	void *ethsys_base;
	int phy_interface;
};

enum mkt_eth_capabilities {
	MTK_TRGMII_BIT,
	MTK_TRGMII_MT7621_CLK_BIT,
	MTK_U3_COPHY_V2_BIT,
	MTK_INFRA_BIT,
	MTK_NETSYS_V2_BIT,
	MTK_NETSYS_V3_BIT,

	/* PATH BITS */
	MTK_ETH_PATH_GMAC1_TRGMII_BIT,
	MTK_ETH_PATH_GMAC2_SGMII_BIT,
	MTK_ETH_PATH_MT7622_SGMII_BIT,
	MTK_ETH_PATH_MT7629_GMAC2_BIT,
};

#define MTK_TRGMII			BIT(MTK_TRGMII_BIT)
#define MTK_TRGMII_MT7621_CLK		BIT(MTK_TRGMII_MT7621_CLK_BIT)
#define MTK_U3_COPHY_V2			BIT(MTK_U3_COPHY_V2_BIT)
#define MTK_INFRA			BIT(MTK_INFRA_BIT)
#define MTK_NETSYS_V2			BIT(MTK_NETSYS_V2_BIT)
#define MTK_NETSYS_V3			BIT(MTK_NETSYS_V3_BIT)

/* Supported path present on SoCs */
#define MTK_ETH_PATH_GMAC1_TRGMII	BIT(MTK_ETH_PATH_GMAC1_TRGMII_BIT)
#define MTK_ETH_PATH_GMAC2_SGMII	BIT(MTK_ETH_PATH_GMAC2_SGMII_BIT)
#define MTK_ETH_PATH_MT7622_SGMII	BIT(MTK_ETH_PATH_MT7622_SGMII_BIT)
#define MTK_ETH_PATH_MT7629_GMAC2	BIT(MTK_ETH_PATH_MT7629_GMAC2_BIT)

#define MTK_GMAC1_TRGMII	(MTK_ETH_PATH_GMAC1_TRGMII | MTK_TRGMII)

#define MTK_GMAC2_U3_QPHY	(MTK_ETH_PATH_GMAC2_SGMII | MTK_U3_COPHY_V2 | MTK_INFRA)

#define MTK_HAS_CAPS(caps, _x)		(((caps) & (_x)) == (_x))

#define MT7621_CAPS  (MTK_GMAC1_TRGMII | MTK_TRGMII_MT7621_CLK)

#define MT7622_CAPS  (MTK_ETH_PATH_MT7622_SGMII)

#define MT7623_CAPS  (MTK_GMAC1_TRGMII)

#define MT7629_CAPS  (MTK_ETH_PATH_MT7629_GMAC2 | MTK_INFRA)

#define MT7981_CAPS  (MTK_GMAC2_U3_QPHY | MTK_NETSYS_V2)

#define MT7986_CAPS  (MTK_NETSYS_V2)

#define MT7987_CAPS  (MTK_NETSYS_V3 | MTK_GMAC2_U3_QPHY | MTK_INFRA)

#define MT7988_CAPS  (MTK_NETSYS_V3 | MTK_INFRA)

/* Frame Engine Register Bases */
#define PDMA_V1_BASE			0x0800
#define PDMA_V2_BASE			0x6000
#define PDMA_V3_BASE			0x6800
#define GDMA1_BASE			0x0500
#define GDMA2_BASE			0x1500
#define GDMA3_BASE			0x0540
#define GMAC_BASE			0x10000
#define GSW_BASE			0x20000

/* Ethernet subsystem registers */
#define ETHSYS_SYSCFG1_REG		0x14
#define SYSCFG1_GE_MODE_S(n)		(12 + ((n) * 2))
#define SYSCFG1_GE_MODE_M		0x3
#define SYSCFG1_SGMII_SEL_M		GENMASK(9, 8)
#define SYSCFG1_SGMII_SEL(gmac)		BIT(9 - (gmac))

#define ETHSYS_CLKCFG0_REG		0x2c
#define ETHSYS_TRGMII_CLK_SEL362_5	BIT(11)

/* Top misc registers */
#define TOPMISC_NETSYS_PCS_MUX		0x84
#define NETSYS_PCS_MUX_MASK		GENMASK(1, 0)
#define MUX_G2_USXGMII_SEL		BIT(1)
#define MUX_HSGMII1_G1_SEL		BIT(0)

#define USB_PHY_SWITCH_REG		0x218
#define QPHY_SEL_MASK			0x3
#define SGMII_QPHY_SEL			0x2

#define MT7629_INFRA_MISC2_REG		0x70c
#define INFRA_MISC2_BONDING_OPTION	GENMASK(15, 0)

/* SYSCFG1_GE_MODE: GE Modes */
#define GE_MODE_RGMII			0
#define GE_MODE_MII			1
#define GE_MODE_MII_PHY			2
#define GE_MODE_RMII			3

/* SGMII subsystem config registers */
#define SGMSYS_PCS_CONTROL_1		0x0
#define SGMII_LINK_STATUS		BIT(18)
#define SGMII_AN_ENABLE			BIT(12)
#define SGMII_AN_RESTART		BIT(9)

#define SGMSYS_SGMII_MODE		0x20
#define SGMII_AN_MODE			0x31120103
#define SGMII_FORCE_MODE		0x31120019

#define SGMSYS_QPHY_PWR_STATE_CTRL	0xe8
#define SGMII_PHYA_PWD			BIT(4)

#define SGMSYS_QPHY_WRAP_CTRL		0xec
#define SGMII_PN_SWAP_TX_RX		0x03

#define SGMSYS_GEN2_SPEED		0x2028
#define SGMSYS_GEN2_SPEED_V2		0x128
#define SGMSYS_SPEED_MASK		GENMASK(3, 2)
#define SGMSYS_SPEED_2500		1

/* USXGMII subsystem config registers */
/* Register to control USXGMII XFI PLL digital */
#define XFI_PLL_DIG_GLB8		0x08
#define RG_XFI_PLL_EN			BIT(31)

/* Register to control USXGMII XFI PLL analog */
#define XFI_PLL_ANA_GLB8		0x108
#define RG_XFI_PLL_ANA_SWWA		0x02283248

/* Frame Engine Registers */
#define PSE_NO_DROP_CFG_REG		0x108
#define PSE_NO_DROP_GDM1		BIT(1)

#define FE_GLO_MISC_REG			0x124
#define PDMA_VER_V2			BIT(4)

/* PDMA */
#define TX_BASE_PTR_REG(n)		(0x000 + (n) * 0x10)
#define TX_MAX_CNT_REG(n)		(0x004 + (n) * 0x10)
#define TX_CTX_IDX_REG(n)		(0x008 + (n) * 0x10)
#define TX_DTX_IDX_REG(n)		(0x00c + (n) * 0x10)

#define RX_BASE_PTR_REG(n)		(0x100 + (n) * 0x10)
#define RX_MAX_CNT_REG(n)		(0x104 + (n) * 0x10)
#define RX_CRX_IDX_REG(n)		(0x108 + (n) * 0x10)
#define RX_DRX_IDX_REG(n)		(0x10c + (n) * 0x10)

#define PDMA_GLO_CFG_REG		0x204
#define TX_WB_DDONE			BIT(6)
#define RX_DMA_BUSY			BIT(3)
#define RX_DMA_EN			BIT(2)
#define TX_DMA_BUSY			BIT(1)
#define TX_DMA_EN			BIT(0)

#define PDMA_RST_IDX_REG		0x208
#define RST_DRX_IDX0			BIT(16)
#define RST_DTX_IDX0			BIT(0)

/* GDMA */
#define GDMA_IG_CTRL_REG		0x000
#define GDM_ICS_EN			BIT(22)
#define GDM_TCS_EN			BIT(21)
#define GDM_UCS_EN			BIT(20)
#define STRP_CRC			BIT(16)
#define MYMAC_DP_S			12
#define MYMAC_DP_M			0xf000
#define BC_DP_S				8
#define BC_DP_M				0xf00
#define MC_DP_S				4
#define MC_DP_M				0xf0
#define UN_DP_S				0
#define UN_DP_M				0x0f

#define GDMA_EG_CTRL_REG		0x004
#define GDMA_CPU_BRIDGE_EN		BIT(31)

#define GDMA_MAC_LSB_REG		0x008

#define GDMA_MAC_MSB_REG		0x00c

/* MYMAC_DP/BC_DP/MC_DP/UN_DP: Destination ports */
#define DP_PDMA				0
#define DP_GDMA1			1
#define DP_GDMA2			2
#define DP_PPE				4
#define DP_QDMA				5
#define DP_DISCARD			7

/* GMAC Registers */
#define GMAC_PPSC_REG			0x0000
#define PHY_MDC_CFG			GENMASK(29, 24)
#define MDC_TURBO			BIT(20)
#define MDC_MAX_FREQ			25000000
#define MDC_MAX_DIVIDER			63

#define GMAC_PIAC_REG			0x0004
#define PHY_ACS_ST			BIT(31)
#define MDIO_REG_ADDR_S			25
#define MDIO_REG_ADDR_M			0x3e000000
#define MDIO_PHY_ADDR_S			20
#define MDIO_PHY_ADDR_M			0x1f00000
#define MDIO_CMD_S			18
#define MDIO_CMD_M			0xc0000
#define MDIO_ST_S			16
#define MDIO_ST_M			0x30000
#define MDIO_RW_DATA_S			0
#define MDIO_RW_DATA_M			0xffff

#define GMAC_XGMAC_STS_REG		0x000c
#define P1_XGMAC_FORCE_LINK		BIT(15)

#define GMAC_MAC_MISC_REG		0x0010
#define MISC_MDC_TURBO			BIT(4)

#define GMAC_GSW_CFG_REG		0x0080
#define GSWTX_IPG_M			0xF0000
#define GSWTX_IPG_S			16
#define GSWRX_IPG_M			0xF
#define GSWRX_IPG_S			0

/* MDIO_CMD: MDIO commands */
#define MDIO_CMD_ADDR			0
#define MDIO_CMD_WRITE			1
#define MDIO_CMD_READ			2
#define MDIO_CMD_READ_C45		3

/* MDIO_ST: MDIO start field */
#define MDIO_ST_C45			0
#define MDIO_ST_C22			1

#define GMAC_PORT_MCR(p)		(0x0100 + (p) * 0x100)
#define MAC_RX_PKT_LEN_S		24
#define MAC_RX_PKT_LEN_M		0x3000000
#define IPG_CFG_S			18
#define IPG_CFG_M			0xc0000
#define MAC_MODE			BIT(16)
#define FORCE_MODE			BIT(15)
#define MAC_TX_EN			BIT(14)
#define MAC_RX_EN			BIT(13)
#define DEL_RXFIFO_CLR			BIT(12)
#define BKOFF_EN			BIT(9)
#define BACKPR_EN			BIT(8)
#define FORCE_RX_FC			BIT(5)
#define FORCE_TX_FC			BIT(4)
#define FORCE_SPD_S			2
#define FORCE_SPD_M			0x0c
#define FORCE_DPX			BIT(1)
#define FORCE_LINK			BIT(0)

/* Values of IPG_CFG */
#define IPG_96BIT			0
#define IPG_96BIT_WITH_SHORT_IPG	1
#define IPG_64BIT			2

/* MAC_RX_PKT_LEN: Max RX packet length */
#define MAC_RX_PKT_LEN_1518		0
#define MAC_RX_PKT_LEN_1536		1
#define MAC_RX_PKT_LEN_1552		2
#define MAC_RX_PKT_LEN_JUMBO		3

/* FORCE_SPD: Forced link speed */
#define SPEED_10M			0
#define SPEED_100M			1
#define SPEED_1000M			2

#define GMAC_TRGMII_RCK_CTRL		0x300
#define RX_RST				BIT(31)
#define RXC_DQSISEL			BIT(30)

#define NUM_TRGMII_CTRL			5

#define GMAC_TRGMII_TD_ODT(n)		(0x354 + (n) * 8)
#define TD_DM_DRVN_S			4
#define TD_DM_DRVN_M			0xf0
#define TD_DM_DRVP_S			0
#define TD_DM_DRVP_M			0x0f

/* XGMAC Status Registers */
#define XGMAC_STS(x)			(((x) == 2) ? 0x001C : 0x000C)
#define XGMAC_FORCE_LINK(x)		(((x) == 1) ? BIT(31) : BIT(15))

/* XGMAC Registers */
#define XGMAC_PORT_MCR(x)		(0x2000 + (((x) - 1) * 0x1000))
#define XGMAC_TRX_DISABLE		0xf
#define XGMAC_FORCE_TX_FC		BIT(5)
#define XGMAC_FORCE_RX_FC		BIT(4)

/* MDIO Indirect Access Registers */
#define MII_MMD_ACC_CTL_REG		0x0d
#define MMD_CMD_S			14
#define MMD_CMD_M			0xc000
#define MMD_DEVAD_S			0
#define MMD_DEVAD_M			0x1f

/* MMD_CMD: MMD commands */
#define MMD_ADDR			0
#define MMD_DATA			1
#define MMD_DATA_RW_POST_INC		2
#define MMD_DATA_W_POST_INC		3

#define MII_MMD_ADDR_DATA_REG		0x0e

/* PDMA descriptors */
struct mtk_rx_dma {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
} __packed __aligned(4);

struct mtk_rx_dma_v2 {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
	unsigned int rxd5;
	unsigned int rxd6;
	unsigned int rxd7;
	unsigned int rxd8;
} __packed __aligned(4);

struct mtk_tx_dma {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
} __packed __aligned(4);

struct mtk_tx_dma_v2 {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
	unsigned int txd5;
	unsigned int txd6;
	unsigned int txd7;
	unsigned int txd8;
} __packed __aligned(4);

/* PDMA TXD fields */
#define PDMA_TXD2_DDONE			BIT(31)
#define PDMA_TXD2_LS0			BIT(30)
#define PDMA_V1_TXD2_SDL0_M		GENMASK(29, 16)
#define PDMA_V1_TXD2_SDL0_SET(_v)	FIELD_PREP(PDMA_V1_TXD2_SDL0_M, (_v))
#define PDMA_V2_TXD2_SDL0_M		GENMASK(23, 8)
#define PDMA_V2_TXD2_SDL0_SET(_v)	FIELD_PREP(PDMA_V2_TXD2_SDL0_M, (_v))

#define PDMA_V1_TXD4_FPORT_M		GENMASK(27, 25)
#define PDMA_V1_TXD4_FPORT_SET(_v)	FIELD_PREP(PDMA_V1_TXD4_FPORT_M, (_v))
#define PDMA_V2_TXD4_FPORT_M		GENMASK(27, 24)
#define PDMA_V2_TXD4_FPORT_SET(_v)	FIELD_PREP(PDMA_V2_TXD4_FPORT_M, (_v))

#define PDMA_V2_TXD5_FPORT_M		GENMASK(19, 16)
#define PDMA_V2_TXD5_FPORT_SET(_v)	FIELD_PREP(PDMA_V2_TXD5_FPORT_M, (_v))

/* PDMA RXD fields */
#define PDMA_RXD2_DDONE			BIT(31)
#define PDMA_RXD2_LS0			BIT(30)
#define PDMA_V1_RXD2_PLEN0_M		GENMASK(29, 16)
#define PDMA_V1_RXD2_PLEN0_GET(_v)	FIELD_GET(PDMA_V1_RXD2_PLEN0_M, (_v))
#define PDMA_V1_RXD2_PLEN0_SET(_v)	FIELD_PREP(PDMA_V1_RXD2_PLEN0_M, (_v))
#define PDMA_V2_RXD2_PLEN0_M		GENMASK(23, 8)
#define PDMA_V2_RXD2_PLEN0_GET(_v)	FIELD_GET(PDMA_V2_RXD2_PLEN0_M, (_v))
#define PDMA_V2_RXD2_PLEN0_SET(_v)	FIELD_PREP(PDMA_V2_RXD2_PLEN0_M, (_v))

void mtk_fe_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr, u32 set);
void mtk_gmac_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr, u32 set);
void mtk_ethsys_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr, u32 set);

int mtk_mii_read(struct mtk_eth_priv *priv, u8 phy, u8 reg);
int mtk_mii_write(struct mtk_eth_priv *priv, u8 phy, u8 reg, u16 data);
int mtk_mmd_read(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg);
int mtk_mmd_write(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg,
		  u16 val);
int mtk_mmd_ind_read(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg);
int mtk_mmd_ind_write(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg,
		      u16 val);

#endif /* _MTK_ETH_H_ */
