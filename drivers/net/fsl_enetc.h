/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ENETC ethernet controller driver
 * Copyright 2017-2019 NXP
 */

#ifndef _ENETC_H
#define _ENETC_H

#define enetc_dbg(dev, fmt, args...)	debug("%s:" fmt, dev->name, ##args)

/* PCI function IDs */
#define PCI_DEVICE_ID_ENETC_ETH		0xE100

/* ENETC Ethernet controller registers */
/* Station interface register offsets */
#define ENETC_SIMR		0x000
#define  ENETC_SIMR_EN		BIT(31)
#define ENETC_SICAR0		0x040
/* write cache cfg: snoop, no allocate, data & BD coherent */
#define  ENETC_SICAR_WR_CFG	0x6767
/* read cache cfg: coherent copy, look up, don't alloc in cache */
#define  ENETC_SICAR_RD_CFG	0x27270000
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
#define ENETC_PMR			0x0000
#define  ENETC_PMR_SI0_EN		BIT(16)
#define ENETC_PSIPMMR			0x0018
#define ENETC_PSIPMAR0			0x0100
#define ENETC_PSIPMAR1			0x0104
#define ENETC_PSICFGR(n)		(0x0940 + (n) * 0x10)
#define  ENETC_PSICFGR_SET_TXBDR(val)	((val) & 0xff)
#define  ENETC_PSICFGR_SET_RXBDR(val)	(((val) & 0xff) << 16)
/* MAC configuration */
#define ENETC_PM_CC			0x8008
#define  ENETC_PM_CC_DEFAULT		0x0810
#define  ENETC_PM_CC_RX_TX_EN		0x8813
#define ENETC_PM_MAXFRM			0x8014
#define  ENETC_RX_MAXFRM_SIZE		PKTSIZE_ALIGN

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
};

/* register accessors */
#define enetc_read_reg(x)	readl((x))
#define enetc_write_reg(x, val)	writel((val), (x))
#define enetc_read(priv, off)	enetc_read_reg((priv)->regs_base + (off))
#define enetc_write(priv, off, v) \
			enetc_write_reg((priv)->regs_base + (off), v)

/* port register accessors */
#define enetc_port_regs(priv, off) ((priv)->port_regs + (off))
#define enetc_read_port(priv, off) \
			enetc_read_reg(enetc_port_regs((priv), (off)))
#define enetc_write_port(priv, off, v) \
			enetc_write_reg(enetc_port_regs((priv), (off)), v)

/* BDR register accessors, see ENETC_BDR() */
#define enetc_bdr_read(priv, t, n, off) \
			enetc_read(priv, ENETC_BDR(t, n, off))
#define enetc_bdr_write(priv, t, n, off, val) \
			enetc_write(priv, ENETC_BDR(t, n, off), val)

#endif /* _ENETC_H */
