// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 * Copyright (c) 2021 Toco Technologies FZE <contact@toco.ae>
 * Copyright (c) 2021 Gabor Juhos <j4g8y7@gmail.com>
 *
 * Qualcomm ESS EDMA ethernet driver
 */

#ifndef _ESSEDMA_ETH_H
#define _ESSEDMA_ETH_H

#define ESS_PORTS_NUM				6

#define ESS_RGMII_CTRL 				0x4

#define ESS_GLOBAL_FW_CTRL1			0x624

#define ESS_PORT0_STATUS			0x7c
#define ESS_PORT_SPEED_MASK			GENMASK(1, 0)
#define ESS_PORT_SPEED_1000			3
#define ESS_PORT_SPEED_100			2
#define ESS_PORT_SPEED_10			1
#define ESS_PORT_TXMAC_EN			BIT(2)
#define ESS_PORT_RXMAC_EN			BIT(3)
#define ESS_PORT_TX_FLOW_EN			BIT(4)
#define ESS_PORT_RX_FLOW_EN			BIT(5)
#define ESS_PORT_DUPLEX_MODE		BIT(6)

#define ESS_PORT_LOOKUP_CTRL(_p)	(0x660 + (_p) * 12)
#define ESS_PORT_LOOP_BACK_EN		BIT(21)
#define ESS_PORT_VID_MEM_MASK		GENMASK(6, 0)

#define ESS_PORT_HOL_CTRL0(_p)		(0x970 + (_p) * 8)
#define EG_PORT_QUEUE_NUM_MASK		GENMASK(29, 24)

/* Ports 0 and 5 have queues 0-5
 * Ports 1 to 4 have queues 0-3
 */
#define EG_PRI5_QUEUE_NUM_MASK		GENMASK(23, 20)
#define EG_PRI4_QUEUE_NUM_MASK		GENMASK(19, 16)
#define EG_PRI3_QUEUE_NUM_MASK		GENMASK(15, 12)
#define EG_PRI2_QUEUE_NUM_MASK		GENMASK(11, 8)
#define EG_PRI1_QUEUE_NUM_MASK		GENMASK(7, 4)
#define EG_PRI0_QUEUE_NUM_MASK		GENMASK(3, 0)

#define ESS_PORT_HOL_CTRL1(_p)		(0x974 + (_p) * 8)
#define ESS_ING_BUF_NUM_0_MASK		GENMASK(3, 0)

/* QCA807x PHY registers */
#define QCA807X_CHIP_CONFIGURATION 	0x1f
#define QCA807X_MEDIA_PAGE_SELECT 	BIT(15)

#define QCA807X_POWER_DOWN 			BIT(11)

#define QCA807X_FUNCTION_CONTROL				0x10
#define QCA807X_MDI_CROSSOVER_MODE_MASK			GENMASK(6, 5)
#define QCA807X_MDI_CROSSOVER_MODE_MANUAL_MDI	0
#define QCA807X_POLARITY_REVERSAL				BIT(1)

#define QCA807X_PHY_SPECIFIC					0x11
#define QCA807X_PHY_SPECIFIC_LINK				BIT(10)

#define QCA807X_MMD7_CRC_PACKET_COUNTER			0x8029
#define QCA807X_MMD7_PACKET_COUNTER_SELFCLR		BIT(1)
#define QCA807X_MMD7_CRC_PACKET_COUNTER_EN		BIT(0)
#define QCA807X_MMD7_VALID_EGRESS_COUNTER_2		0x802e

/* PSGMII specific registers */
#define PSGMIIPHY_VCO_CALIBRATION_CTRL_REGISTER_1		0x9c
#define PSGMIIPHY_VCO_VAL								0x4ada
#define PSGMIIPHY_VCO_RST_VAL							0xada
#define PSGMIIPHY_VCO_CALIBRATION_CTRL_REGISTER_2		0xa0

#define PSGMIIPHY_PLL_VCO_RELATED_CTRL 					0x78c
#define PSGMIIPHY_PLL_VCO_VAL							0x2803

#define RGMII_TCSR_ESS_CFG	0x01953000

/* EDMA registers */
#define IPQ40XX_EDMA_TX_RING_SIZE	8
#define IPQ40XX_EDMA_RSS_TYPE_NONE	0x1

#define EDMA_RSS_TYPE		0
#define EDMA_TPD_EOP_SHIFT	31

/* tpd word 3 bit 18-28 */
#define EDMA_TPD_PORT_BITMAP_SHIFT	18

/* Enable Tx for all ports */
#define EDMA_PORT_ENABLE_ALL	0x3E

/* Edma receive consumer index */
/* x = queue id */
#define EDMA_REG_RX_SW_CONS_IDX_Q(x)	(0x220 + ((x) << 2))
/* Edma transmit consumer index */
#define EDMA_REG_TX_SW_CONS_IDX_Q(x)	(0x240 + ((x) << 2))
/* TPD Index Register */
#define EDMA_REG_TPD_IDX_Q(x)		(0x460 + ((x) << 2))
/* Tx Descriptor Control Register */
#define EDMA_REG_TPD_RING_SIZE		0x41C
#define EDMA_TPD_RING_SIZE_MASK		0xFFFF

/* Transmit descriptor base address */
 /* x = queue id */
#define EDMA_REG_TPD_BASE_ADDR_Q(x)	(0x420 + ((x) << 2))
#define EDMA_TPD_PROD_IDX_MASK		GENMASK(15, 0)
#define EDMA_TPD_CONS_IDX_MASK		GENMASK(31, 16)

#define EDMA_REG_TX_SRAM_PART		0x400
#define EDMA_LOAD_PTR_SHIFT		16

/* TXQ Control Register */
#define EDMA_REG_TXQ_CTRL		0x404
#define EDMA_TXQ_CTRL_TXQ_EN		0x20
#define EDMA_TXQ_CTRL_TPD_BURST_EN	0x100
#define EDMA_TXQ_NUM_TPD_BURST_SHIFT	0
#define EDMA_TXQ_TXF_BURST_NUM_SHIFT	16
#define EDMA_TXF_BURST			0x100
#define EDMA_TPD_BURST			5

#define EDMA_REG_TXF_WATER_MARK		0x408

/* RSS Indirection Register */
/* x = No. of indirection table */
#define EDMA_REG_RSS_IDT(x)		(0x840 + ((x) << 2))
#define EDMA_NUM_IDT			16
#define EDMA_RSS_IDT_VALUE		0x64206420

/* RSS Hash Function Type Register */
#define EDMA_REG_RSS_TYPE	0x894

/* x = queue id */
#define EDMA_REG_RFD_BASE_ADDR_Q(x)	(0x950 + ((x) << 2))
/* RFD Index Register */
#define EDMA_RFD_BURST		8
#define EDMA_RFD_THR		16
#define EDMA_RFD_LTHR		0
#define EDMA_REG_RFD_IDX_Q(x)	(0x9B0 + ((x) << 2))

#define EDMA_RFD_CONS_IDX_MASK	GENMASK(27, 16)

/* Rx Descriptor Control Register */
#define EDMA_REG_RX_DESC0		0xA10
#define EDMA_RFD_RING_SIZE_MASK		0xFFF
#define EDMA_RX_BUF_SIZE_MASK		0xFFFF
#define EDMA_RFD_RING_SIZE_SHIFT	0
#define EDMA_RX_BUF_SIZE_SHIFT		16

#define EDMA_REG_RX_DESC1		0xA14
#define EDMA_RXQ_RFD_BURST_NUM_SHIFT	0
#define EDMA_RXQ_RFD_PF_THRESH_SHIFT	8
#define EDMA_RXQ_RFD_LOW_THRESH_SHIFT	16

/* RXQ Control Register */
#define EDMA_REG_RXQ_CTRL		0xA18
#define EDMA_FIFO_THRESH_128_BYTE	0x0
#define EDMA_RXQ_CTRL_RMV_VLAN		0x00000002
#define EDMA_RXQ_CTRL_EN		0x0000FF00

/* MAC Control Register */
#define REG_MAC_CTRL0		0xC20
#define REG_MAC_CTRL1		0xC24

/* Transmit Packet Descriptor */
struct edma_tpd {
	u16 len; /* full packet including CRC */
	u16 svlan_tag; /* vlan tag */
	u32 word1; /* byte 4-7 */
	u32 addr; /* address of buffer */
	u32 word3; /* byte 12 */
};

/* Receive Return Descriptor */
struct edma_rrd {
	u16 rrd0;
	u16 rrd1;
	u16 rrd2;
	u16 rrd3;
	u16 rrd4;
	u16 rrd5;
	u16 rrd6;
	u16 rrd7;
} __packed;

#define EDMA_RRD_SIZE			sizeof(struct edma_rrd)

#define EDMA_RRD7_DESC_VALID		BIT(15)

/* Receive Free Descriptor */
struct edma_rfd {
	u32 buffer_addr; /* buffer address */
};

#endif	/* _ESSEDMA_ETH_H */
