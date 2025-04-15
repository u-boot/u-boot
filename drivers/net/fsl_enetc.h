/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ENETC ethernet controller driver
 * Copyright 2017-2021 NXP
 */

#ifndef _ENETC_H
#define _ENETC_H

#include <linux/bitops.h>
#define enetc_dbg(dev, fmt, args...)	debug("%s:" fmt, dev->name, ##args)

/* PCI function IDs */
#define PCI_DEVICE_ID_ENETC_ETH		0xE100
#define PCI_DEVICE_ID_ENETC4_ETH	0xE101
#define PCI_DEVICE_ID_ENETC_MDIO	0xEE01
#define PCI_DEVICE_ID_ENETC4_EMDIO	0xEE00

/* ENETC Ethernet controller registers */
/* Station interface register offsets */
#define ENETC_SIMR		0x000
#define  ENETC_SIMR_EN		BIT(31)
#define ENETC_SICAR0		0x040
/* write cache cfg: snoop, no allocate, data & BD coherent */
#define  ENETC_SICAR_WR_CFG	0x6767
/* read cache cfg: coherent copy, look up, don't alloc in cache */
#define  ENETC_SICAR_RD_CFG_LS	0x27270000
#define  ENETC_SICAR_RD_CFG_IMX	0x2b2b0000
#define ENETC_SIROCT		0x300
#define ENETC_SIRFRM		0x308
#define ENETC_SITOCT		0x320
#define ENETC_SITFRM		0x328

/* Rx/Tx Buffer Descriptor Ring registers */
enum enetc_bdr_type {TX, RX};
#define ENETC_BDR(type, n, off)	(0x8000 + (type) * 0x100 + (n) * 0x200 + (off))
#define ENETC_BDR_IDX_MASK	0xffff

/* Rx BDR reg offsets */
#define ENETC_RBMR		0x00
#define  ENETC_RBMR_EN		BIT(31)
#define ENETC_RBBSR		0x08
/* initial consumer index for Rx BDR */
#define ENETC_RBCIR		0x0c
#define ENETC_RBBAR0		0x10
#define ENETC_RBBAR1		0x14
#define ENETC_RBPIR		0x18
#define ENETC_RBLENR		0x20

/* Tx BDR reg offsets */
#define ENETC_TBMR		0x00
#define  ENETC_TBMR_EN		BIT(31)
#define ENETC_TBBAR0		0x10
#define ENETC_TBBAR1		0x14
#define ENETC_TBPIR		0x18
#define ENETC_TBCIR		0x1c
#define ENETC_TBLENR		0x20

/* Port registers offset */
#define ENETC_PORT_REGS_OFF		0x10000

/* Port registers */
#define ENETC_PMR_OFFSET_IMX		0x0010
#define ENETC_PMR_OFFSET_LS		0x0000
#define ENETC_PMR			0x0000
#define  ENETC_PMR_SI0_EN		BIT(16)
#define ENETC_PSIPMMR			0x0018
#define ENETC_PSIPMARn_OFFSET_IMX	0x0000
#define ENETC_PSIPMARn_OFFSET_LS	0x0080
#define ENETC_PSIPMAR0			0x0080
#define ENETC_PSIPMAR1			0x0084
#define ENETC_PCAPR_OFFSET_IMX		0x4008
#define ENETC_PCAPR_OFFSET_LS		0x0900
#define ENETC_PCAPR0			0x0000
#define ENETC_PCAPRO_MDIO		BIT(11)	/* LS only */
#define ENETC_PCS_PROT			GENMASK(15, 0) /* IMX only */
/* ENETC base registers */
#define ENETC_PSICFGR_OFFSET_LS		0x0940
#define ENETC_PSICFGR_SHIFT_LS		0x10
#define ENETC_PSICFGR_OFFSET_IMX	0x2010
#define ENETC_PSICFGR_SHIFT_IMX		0x80
#define ENETC_PSICFGR(n, s)		((n) * (s))
#define  ENETC_PSICFGR_SET_BDR(rx, tx)	(((rx) << 16) | (tx))
/* MAC configuration */
#define ENETC_PM_OFFSET_IMX		0x5000
#define ENETC_PM_OFFSET_LS		0x8000
#define ENETC_PM_CC			0x0008
#define  ENETC_PM_CC_DEFAULT		0x0810
#define  ENETC_PM_CC_TXP_IMX		BIT(15)
#define  ENETC_PM_CC_TXP_LS		BIT(11)
#define  ENETC_PM_CC_PROMIS		BIT(4)
#define  ENETC_PM_CC_TX			BIT(1)
#define  ENETC_PM_CC_RX			BIT(0)
#define ENETC_PM_MAXFRM			0x0014
#define  ENETC_RX_MAXFRM_SIZE		PKTSIZE_ALIGN
#define ENETC_PM_IMDIO_BASE		0x0030
#define ENETC_PM_IF_MODE		0x0300
#define  ENETC_PM_IF_MODE_RG		BIT(2)
#define  ENETC_PM_IF_MODE_AN_ENA	BIT(15)
#define  ENETC_PM_IFM_SSP_MASK		GENMASK(14, 13)
#define  ENETC_PM_IFM_SSP_1000		(2 << 13)
#define  ENETC_PM_IFM_SSP_100		(0 << 13)
#define  ENETC_PM_IFM_SSP_10		(1 << 13)
#define  ENETC_PM_IFM_FULL_DPX_IMX	BIT(6)
#define  ENETC_PM_IFM_FULL_DPX_LS	BIT(12)
#define  ENETC_PM_IF_IFMODE_MASK_IMX	GENMASK(2, 0)
#define  ENETC_PM_IF_IFMODE_MASK_LS	GENMASK(1, 0)

/* i.MX95 specific registers */
#define IMX95_ENETC_SIPMAR0		0x80
#define IMX95_ENETC_SIPMAR1		0x84

/* Port registers */
#define IMX95_ENETC_PMAR0		0x4020
#define IMX95_ENETC_PMAR1		0x4024
#define ENETC_POR			0x4100

/* buffer descriptors count must be multiple of 8 and aligned to 128 bytes */
#define ENETC_BD_CNT		CONFIG_SYS_RX_ETH_BUFFER
#define ENETC_BD_ALIGN		128

/* single pair of Rx/Tx rings */
#define ENETC_RX_BDR_CNT	1
#define ENETC_TX_BDR_CNT	1
#define ENETC_RX_BDR_ID		0
#define ENETC_TX_BDR_ID		0

/* Tx buffer descriptor */
struct enetc_tx_bd {
	__le64 addr;
	__le16 buf_len;
	__le16 frm_len;
	__le16 err_csum;
	__le16 flags;
};

#define ENETC_TXBD_FLAGS_F	BIT(15)
#define ENETC_POLL_TRIES	32000

/* Rx buffer descriptor */
union enetc_rx_bd {
	/* SW provided BD format */
	struct {
		__le64 addr;
		u8 reserved[8];
	} w;

	/* ENETC returned BD format */
	struct {
		__le16 inet_csum;
		__le16 parse_summary;
		__le32 rss_hash;
		__le16 buf_len;
		__le16 vlan_opt;
		union {
			struct {
				__le16 flags;
				__le16 error;
			};
			__le32 lstatus;
		};
	} r;
};

#define ENETC_RXBD_STATUS_R(status)		(((status) >> 30) & 0x1)
#define ENETC_RXBD_STATUS_F(status)		(((status) >> 31) & 0x1)
#define ENETC_RXBD_STATUS_ERRORS(status)	(((status) >> 16) & 0xff)
#define ENETC_RXBD_STATUS(flags)		((flags) << 16)

/* Tx/Rx ring info */
struct bd_ring {
	void *cons_idx;
	void *prod_idx;
	/* next BD index to use */
	int next_prod_idx;
	int next_cons_idx;
	int bd_count;
};

/* ENETC private structure */
struct enetc_priv {
	struct enetc_tx_bd *enetc_txbd;
	union enetc_rx_bd *enetc_rxbd;

	void *regs_base; /* base ENETC registers */
	void *port_regs; /* base ENETC port registers */

	/* Rx/Tx buffer descriptor rings info */
	struct bd_ring tx_bdr;
	struct bd_ring rx_bdr;

	int uclass_id;
	struct mii_dev imdio;
	struct phy_device *phy;
};

struct enetc_data {
	/* Register layout offsets */
	u16			reg_offset_pmr;
	u16			reg_offset_psipmar;
	u16			reg_offset_pcapr;
	u16			reg_offset_psicfgr;
	u16			reg_offset_mac;
};

/* PCS / internal SoC PHY ID, it defaults to 0 on all interfaces */
#define ENETC_PCS_PHY_ADDR	0

/* PCS registers */
#define ENETC_PCS_CR			0x00
#define  ENETC_PCS_CR_RESET_AN		0x1200
#define  ENETC_PCS_CR_DEF_VAL		0x0140
#define  ENETC_PCS_CR_RST		BIT(15)
#define ENETC_PCS_DEV_ABILITY		0x04
#define  ENETC_PCS_DEV_ABILITY_SGMII	0x4001
#define  ENETC_PCS_DEV_ABILITY_SXGMII	0x5001
#define ENETC_PCS_LINK_TIMER1		0x12
#define  ENETC_PCS_LINK_TIMER1_VAL	0x06a0
#define ENETC_PCS_LINK_TIMER2		0x13
#define  ENETC_PCS_LINK_TIMER2_VAL	0x0003
#define ENETC_PCS_IF_MODE		0x14
#define  ENETC_PCS_IF_MODE_SGMII	BIT(0)
#define  ENETC_PCS_IF_MODE_SGMII_AN	BIT(1)
#define  ENETC_PCS_IF_MODE_SPEED_1G	BIT(3)

/* PCS replicator block for USXGMII */
#define ENETC_PCS_DEVAD_REPL		0x1f

#define ENETC_PCS_REPL_LINK_TIMER_1	0x12
#define  ENETC_PCS_REPL_LINK_TIMER_1_DEF	0x0003
#define ENETC_PCS_REPL_LINK_TIMER_2	0x13
#define  ENETC_PCS_REPL_LINK_TIMER_2_DEF	0x06a0

/* ENETC external MDIO registers */
#define ENETC_MDIO_BASE		0x1c00
#define ENETC_MDIO_CFG		0x00
#define  ENETC_EMDIO_CFG_C22	0x00809508
#define  ENETC_EMDIO_CFG_C45	0x00809548
#define  ENETC_EMDIO_CFG_RD_ER	BIT(1)
#define  ENETC_EMDIO_CFG_BSY	BIT(0)
#define ENETC_MDIO_CTL		0x04
#define  ENETC_MDIO_CTL_READ	BIT(15)
#define ENETC_MDIO_DATA		0x08
#define ENETC_MDIO_STAT		0x0c

#define ENETC_MDIO_READ_ERR	0xffff

struct enetc_mdio_priv {
	void *regs_base;
};

/*
 * these functions are implemented by ENETC_MDIO and are re-used by ENETC driver
 * to drive serdes / internal SoC PHYs
 */
int enetc_mdio_read_priv(struct enetc_mdio_priv *priv, int addr, int devad,
			 int reg);
int enetc_mdio_write_priv(struct enetc_mdio_priv *priv, int addr, int devad,
			  int reg, u16 val);

/* sets up primary MAC addresses in DT/IERB */
void fdt_fixup_enetc_mac(void *blob);

#endif /* _ENETC_H */
