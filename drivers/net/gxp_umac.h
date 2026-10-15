/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * HPE GXP UMAC ethernet driver
 *
 * Copyright (C) 2020 Hewlett Packard Enterprise Development LP.
 * Copyright (C) 2026 9elements GmbH
 */

#ifndef _GXP_UMAC_H_
#define _GXP_UMAC_H_

struct gxp_umac_regs {
	u32 config_status;		/* 0x00: Configuration and Status Register I */
	u32 ring_ptr;			/* 0x04: Ring Pointer Register */
	u32 ring_prompt;		/* 0x08: Ring Prompt Register (W) */
	u32 clear_status;		/* 0x0c: Clear Status Register (W) */
	u32 cksum_config;		/* 0x10: Checksum Config Register */
	u32 ring_size;			/* 0x14: Ring Size Register */
	u32 mac_addr_hi;		/* 0x18: MAC Address[47:32] */
	u32 mac_addr_mid;		/* 0x1c: MAC Address[31:16] */
	u32 mac_addr_lo;		/* 0x20: MAC Address[15:0] */
	u32 mc_addr_filt_hi;		/* 0x24: LAF[63:32] */
	u32 mc_addr_filt_lo;		/* 0x28: LAF[31:0] */
	u32 config_status2;		/* 0x2c: Configuration and Status Register II */
	u32 interrupt;			/* 0x30: MAC Interrupt Config and Status */
	u32 overrun_count;		/* 0x34: Overrun Counter Register */
	u32 rx_int_config;		/* 0x38: Rx Interrupt Config Register */
	u32 tx_int_config;		/* 0x3c: Tx Interrupt Config Register */
	u32 packet_length;		/* 0x40: Packet Length Register */
	u32 bcast_filter;		/* 0x44: Broadcast Filter Config Register */
	u32 bcast_prompt;		/* 0x48: Broadcast Prompt Register (W) */
	u32 rx_ring_addr;		/* 0x4c: Rx Ring Base Address Register */
	u32 tx_ring_addr;		/* 0x50: Tx Ring Base Address Register */
	u32 dma_config;			/* 0x54: DMA Config Register */
	u32 burst_config;		/* 0x58: Bursting Config Register */
	u32 pause_config;		/* 0x5c: PAUSE Frame Config Register */
	u32 pause_control;		/* 0x60: PAUSE Frame Control and Status */
	u32 congestion_config;		/* 0x64: Channel Congestion Config */
	u32 frame_filter_config;	/* 0x68: Frame Type Filter Config */
	u32 rx_fifo_config_status;	/* 0x6c: RX FIFO Config and Status */
	u32 rx_ring1_base_addr;		/* 0x70: RX Ring 1 Base Address */
	u32 config_status3;		/* 0x74: Configuration and Status Register III */
	u32 reserved[2];		/* 0x78-0x7c: Reserved */
};

/* MDIO registers (at offset 0x80 from UMAC base) */
struct gxp_mdio_regs {
	u32 mmi;			/* 0x00: MMI Register */
	u32 mmi_data;			/* 0x04: MMI Data Register */
};

/* Config/Status Register I bits */
#define UMAC_CFG_TXEN			BIT(12)
#define UMAC_CFG_RXEN			BIT(11)
#define UMAC_CFG_GIGE_EN		BIT(10)
#define UMAC_CFG_100M_EN		BIT(9)
#define UMAC_CFG_GIGE_MODE		BIT(2)
#define UMAC_CFG_FULL_DUPLEX		BIT(0)
#define UMAC_CFG_MISSED			BIT(7)

/* Interrupt register bits */
#define UMAC_INT_TX			BIT(0)
#define UMAC_INT_RX			BIT(2)
#define UMAC_INT_OVERRUN		BIT(4)

/* MMI register bits */
#define UMAC_MMI_PHY_ADDR_MASK		0x001f0000
#define UMAC_MMI_PHY_ADDR_SHIFT		16
#define UMAC_MMI_MOWNER			BIT(9)
#define UMAC_MMI_MRNW			BIT(8)
#define UMAC_MMI_REG_ADDR		0x0000001f
#define UMAC_MMI_DATA_MASK		0x0000ffff

/* Ring size register shifts */
#define UMAC_RING_SIZE_TX_SHIFT		24
#define UMAC_RING_SIZE_RX_SHIFT		16

/* Ring entry status bits */
#define UMAC_RING_ENTRY_HW_OWN		BIT(15)
#define UMAC_RING_RX_ERR_MASK		0x38e0

/* Frame sizes */
#define UMAC_MIN_FRAME_SIZE		60
#define UMAC_MAX_FRAME_SIZE		1518
#define UMAC_MAX_PACKET_ROUNDED		0x600	/* 1536, aligned */

/* Ring configuration */
#define UMAC_MAX_RING_ENTRIES		8
#define UMAC_RING_ENTRY_SIZE		16

/* Ring entry structures */
struct gxp_umac_rx_desc {
	u32 dma_address;
	u16 status;
	u16 count;
	u16 checksum;
	u16 control;
	u32 reserved;
} __packed;

struct gxp_umac_tx_desc {
	u32 dma_address;
	u16 status;
	u16 count;
	u32 cksum_offset;
	u32 reserved;
} __packed;

/* Packet buffer structure */
struct gxp_umac_packet {
	u8 data[UMAC_MAX_FRAME_SIZE];
	u8 pad[UMAC_MAX_PACKET_ROUNDED - UMAC_MAX_FRAME_SIZE];
};

/* Private data structure */
struct gxp_umac_priv {
	struct gxp_umac_regs __iomem *regs;
	struct gxp_mdio_regs __iomem *mdio_regs;

	struct gxp_umac_tx_desc *txdes;
	struct gxp_umac_rx_desc *rxdes;
	struct gxp_umac_packet *rx_packets;

	int tx_index;
	int rx_index;

	struct mii_dev *bus;
	struct phy_device *phydev;
	u32 phy_addr;
	u32 phy_mode;
};

/* MDIO timeout */
#define GXP_MDIO_TIMEOUT_USEC		10000

#endif /* _GXP_UMAC_H_ */
