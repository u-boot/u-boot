/*
 * Altera 10/100/1000 triple speed ethernet mac
 *
 * Copyright (C) 2008 Altera Corporation.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ALTERA_TSE_H_
#define _ALTERA_TSE_H_

#define __packed_1_    __attribute__ ((packed, aligned(1)))

/* PHY Stuff */
#define miim_end -2
#define miim_read -1

#define PHY_AUTONEGOTIATE_TIMEOUT	5000	/* in ms */

#ifndef CONFIG_SYS_TBIPA_VALUE
#define CONFIG_SYS_TBIPA_VALUE	0x1f
#endif
#define MIIMCFG_INIT_VALUE	0x00000003
#define MIIMCFG_RESET		0x80000000

#define MIIMIND_BUSY		0x00000001
#define MIIMIND_NOTVALID	0x00000004

#define MIIM_CONTROL		0x00
#define MIIM_CONTROL_RESET	0x00009140
#define MIIM_CONTROL_INIT	0x00001140
#define MIIM_CONTROL_RESTART	0x00001340
#define MIIM_ANEN		0x00001000

#define MIIM_CR		0x00
#define MIIM_CR_RST		0x00008000
#define MIIM_CR_INIT		0x00001000

#define MIIM_STATUS		0x1
#define MIIM_STATUS_AN_DONE	0x00000020
#define MIIM_STATUS_LINK	0x0004

#define MIIM_PHYIR1		0x2
#define MIIM_PHYIR2		0x3

#define MIIM_ANAR		0x4
#define MIIM_ANAR_INIT		0x1e1

#define MIIM_TBI_ANLPBPA	0x5
#define MIIM_TBI_ANLPBPA_HALF	0x00000040
#define MIIM_TBI_ANLPBPA_FULL	0x00000020

#define MIIM_TBI_ANEX		0x6
#define MIIM_TBI_ANEX_NP	0x00000004
#define MIIM_TBI_ANEX_PRX	0x00000002

#define MIIM_GBIT_CONTROL	0x9
#define MIIM_GBIT_CONTROL_INIT	0xe00

#define MIIM_EXT_PAGE_ACCESS	0x1f

/* 88E1011 PHY Status Register */
#define MIIM_88E1011_PHY_STATUS	0x11
#define MIIM_88E1011_PHYSTAT_SPEED	0xc000
#define MIIM_88E1011_PHYSTAT_GBIT	0x8000
#define MIIM_88E1011_PHYSTAT_100	0x4000
#define MIIM_88E1011_PHYSTAT_DUPLEX	0x2000
#define MIIM_88E1011_PHYSTAT_SPDDONE	0x0800
#define MIIM_88E1011_PHYSTAT_LINK	0x0400

#define MIIM_88E1011_PHY_SCR		0x10
#define MIIM_88E1011_PHY_MDI_X_AUTO	0x0060

#define MIIM_88E1111_PHY_EXT_CR	0x14
#define MIIM_88E1111_PHY_EXT_SR	0x1b

/* 88E1111 PHY LED Control Register */
#define MIIM_88E1111_PHY_LED_CONTROL	24
#define MIIM_88E1111_PHY_LED_DIRECT	0x4100
#define MIIM_88E1111_PHY_LED_COMBINE	0x411C

#define MIIM_READ_COMMAND	0x00000001

/* struct phy_info: a structure which defines attributes for a PHY
 * id will contain a number which represents the PHY.  During
 * startup, the driver will poll the PHY to find out what its
 * UID--as defined by registers 2 and 3--is.  The 32-bit result
 * gotten from the PHY will be shifted right by "shift" bits to
 * discard any bits which may change based on revision numbers
 * unimportant to functionality
 *
 * The struct phy_cmd entries represent pointers to an arrays of
 * commands which tell the driver what to do to the PHY.
 */
struct phy_info {
	uint id;
	char *name;
	uint shift;
	/* Called to configure the PHY, and modify the controller
	 * based on the results */
	struct phy_cmd *config;

	/* Called when starting up the controller */
	struct phy_cmd *startup;

	/* Called when bringing down the controller */
	struct phy_cmd *shutdown;
};

/* SGDMA Stuff */
#define ALT_SGDMA_STATUS_ERROR_MSK			(0x00000001)
#define ALT_SGDMA_STATUS_EOP_ENCOUNTERED_MSK		(0x00000002)
#define ALT_SGDMA_STATUS_DESC_COMPLETED_MSK		(0x00000004)
#define ALT_SGDMA_STATUS_CHAIN_COMPLETED_MSK		(0x00000008)
#define ALT_SGDMA_STATUS_BUSY_MSK			(0x00000010)

#define ALT_SGDMA_CONTROL_IE_ERROR_MSK			(0x00000001)
#define ALT_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_MSK	(0x00000002)
#define ALT_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK	(0x00000004)
#define ALT_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK	(0x00000008)
#define ALT_SGDMA_CONTROL_IE_GLOBAL_MSK		(0x00000010)
#define ALT_SGDMA_CONTROL_RUN_MSK			(0x00000020)
#define ALT_SGDMA_CONTROL_STOP_DMA_ER_MSK		(0x00000040)
#define ALT_SGDMA_CONTROL_IE_MAX_DESC_PROCESSED_MSK	(0x00000080)
#define ALT_SGDMA_CONTROL_MAX_DESC_PROCESSED_MSK	(0x0000FF00)
#define ALT_SGDMA_CONTROL_SOFTWARERESET_MSK		(0x00010000)
#define ALT_SGDMA_CONTROL_PARK_MSK			(0x00020000)
#define ALT_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK		(0x80000000)

#define ALTERA_TSE_SGDMA_INTR_MASK  (ALT_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK \
			| ALT_SGDMA_STATUS_DESC_COMPLETED_MSK \
			| ALT_SGDMA_CONTROL_IE_GLOBAL_MSK)

/*
 * Descriptor control bit masks & offsets
 *
 * Note: The control byte physically occupies bits [31:24] in memory.
 *	 The following bit-offsets are expressed relative to the LSB of
 *	 the control register bitfield.
 */
#define ALT_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK		(0x00000001)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_MSK	(0x00000002)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK	(0x00000004)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_ATLANTIC_CHANNEL_MSK	(0x00000008)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK		(0x00000080)

/*
 * Descriptor status bit masks & offsets
 *
 * Note: The status byte physically occupies bits [23:16] in memory.
 *	 The following bit-offsets are expressed relative to the LSB of
 *	 the status register bitfield.
 */
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_CRC_MSK			(0x00000001)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_PARITY_MSK		(0x00000002)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_MSK		(0x00000004)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_SYNC_MSK			(0x00000008)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_UEOP_MSK			(0x00000010)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_MEOP_MSK			(0x00000020)
#define ALT_SGDMA_DESCRIPTOR_STATUS_E_MSOP_MSK			(0x00000040)
#define ALT_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK	(0x00000080)
#define ALT_SGDMA_DESCRIPTOR_STATUS_ERROR_MSK			(0x0000007F)

/*
 * The SGDMA controller buffer descriptor allocates
 * 64 bits for each address. To support ANSI C, the
 * struct implementing a descriptor places 32-bits
 * of padding directly above each address; each pad must
 * be cleared when initializing a descriptor.
 */

/*
 * Buffer Descriptor data structure
 *
 */
struct alt_sgdma_descriptor {
	unsigned int *source;	/* the address of data to be read. */
	unsigned int source_pad;

	unsigned int *destination;	/* the address to write data */
	unsigned int destination_pad;

	unsigned int *next;	/* the next descriptor in the list. */
	unsigned int next_pad;

	unsigned short bytes_to_transfer; /* the number of bytes to transfer */
	unsigned char read_burst;
	unsigned char write_burst;

	unsigned short actual_bytes_transferred;/* bytes transferred by DMA */
	unsigned char descriptor_status;
	unsigned char descriptor_control;

} __packed_1_;

/* SG-DMA Control/Status Slave registers map */

struct alt_sgdma_registers {
	unsigned int status;
	unsigned int status_pad[3];
	unsigned int control;
	unsigned int control_pad[3];
	unsigned int next_descriptor_pointer;
	unsigned int descriptor_pad[3];
};

/* TSE Stuff */
#define ALTERA_TSE_CMD_TX_ENA_MSK		(0x00000001)
#define ALTERA_TSE_CMD_RX_ENA_MSK		(0x00000002)
#define ALTERA_TSE_CMD_XON_GEN_MSK		(0x00000004)
#define ALTERA_TSE_CMD_ETH_SPEED_MSK		(0x00000008)
#define ALTERA_TSE_CMD_PROMIS_EN_MSK		(0x00000010)
#define ALTERA_TSE_CMD_PAD_EN_MSK		(0x00000020)
#define ALTERA_TSE_CMD_CRC_FWD_MSK		(0x00000040)
#define ALTERA_TSE_CMD_PAUSE_FWD_MSK		(0x00000080)
#define ALTERA_TSE_CMD_PAUSE_IGNORE_MSK	(0x00000100)
#define ALTERA_TSE_CMD_TX_ADDR_INS_MSK		(0x00000200)
#define ALTERA_TSE_CMD_HD_ENA_MSK		(0x00000400)
#define ALTERA_TSE_CMD_EXCESS_COL_MSK		(0x00000800)
#define ALTERA_TSE_CMD_LATE_COL_MSK		(0x00001000)
#define ALTERA_TSE_CMD_SW_RESET_MSK		(0x00002000)
#define ALTERA_TSE_CMD_MHASH_SEL_MSK		(0x00004000)
#define ALTERA_TSE_CMD_LOOPBACK_MSK		(0x00008000)
/* Bits (18:16) = address select */
#define ALTERA_TSE_CMD_TX_ADDR_SEL_MSK		(0x00070000)
#define ALTERA_TSE_CMD_MAGIC_ENA_MSK		(0x00080000)
#define ALTERA_TSE_CMD_SLEEP_MSK		(0x00100000)
#define ALTERA_TSE_CMD_WAKEUP_MSK		(0x00200000)
#define ALTERA_TSE_CMD_XOFF_GEN_MSK		(0x00400000)
#define ALTERA_TSE_CMD_CNTL_FRM_ENA_MSK	(0x00800000)
#define ALTERA_TSE_CMD_NO_LENGTH_CHECK_MSK	(0x01000000)
#define ALTERA_TSE_CMD_ENA_10_MSK		(0x02000000)
#define ALTERA_TSE_CMD_RX_ERR_DISC_MSK		(0x04000000)
/* Bits (30..27) reserved */
#define ALTERA_TSE_CMD_CNT_RESET_MSK		(0x80000000)

#define ALTERA_TSE_TX_CMD_STAT_TX_SHIFT16	(0x00040000)
#define ALTERA_TSE_TX_CMD_STAT_OMIT_CRC	(0x00020000)

#define ALTERA_TSE_RX_CMD_STAT_RX_SHIFT16	(0x02000000)

#define ALT_TSE_SW_RESET_WATCHDOG_CNTR		10000
#define ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR	90000000

/* Command_Config Register Bit Definitions */

typedef volatile union __alt_tse_command_config {
	unsigned int image;
	struct {
		unsigned int
		 transmit_enable:1,		/* bit 0 */
		 receive_enable:1,		/* bit 1 */
		 pause_frame_xon_gen:1,	/* bit 2 */
		 ethernet_speed:1,		/* bit 3 */
		 promiscuous_enable:1,		/* bit 4 */
		 pad_enable:1,			/* bit 5 */
		 crc_forward:1,		/* bit 6 */
		 pause_frame_forward:1,	/* bit 7 */
		 pause_frame_ignore:1,		/* bit 8 */
		 set_mac_address_on_tx:1,	/* bit 9 */
		 halfduplex_enable:1,		/* bit 10 */
		 excessive_collision:1,	/* bit 11 */
		 late_collision:1,		/* bit 12 */
		 software_reset:1,		/* bit 13 */
		 multicast_hash_mode_sel:1,	/* bit 14 */
		 loopback_enable:1,		/* bit 15 */
		 src_mac_addr_sel_on_tx:3,	/* bit 18:16 */
		 magic_packet_detect:1,	/* bit 19 */
		 sleep_mode_enable:1,		/* bit 20 */
		 wake_up_request:1,		/* bit 21 */
		 pause_frame_xoff_gen:1,	/* bit 22 */
		 control_frame_enable:1,	/* bit 23 */
		 payload_len_chk_disable:1,	/* bit 24 */
		 enable_10mbps_intf:1,		/* bit 25 */
		 rx_error_discard_enable:1,	/* bit 26 */
		 reserved_bits:4,		/* bit 30:27 */
		 self_clear_counter_reset:1;	/* bit 31 */
	} __packed_1_ bits;
} __packed_1_ alt_tse_command_config;

/* Tx_Cmd_Stat Register Bit Definitions */

typedef volatile union __alt_tse_tx_cmd_stat {
	unsigned int image;
	struct {
		unsigned int reserved_lsbs:17,	/* bit 16:0  */
		 omit_crc:1,			/* bit 17 */
		 tx_shift16:1,			/* bit 18 */
		 reserved_msbs:13;		/* bit 31:19 */

	} __packed_1_ bits;
} alt_tse_tx_cmd_stat;

/* Rx_Cmd_Stat Register Bit Definitions */

typedef volatile union __alt_tse_rx_cmd_stat {
	unsigned int image;
	struct {
		unsigned int reserved_lsbs:25,	/* bit 24:0  */
		 rx_shift16:1,			/* bit 25 */
		 reserved_msbs:6;		/* bit 31:26 */

	} __packed_1_ bits;
} alt_tse_rx_cmd_stat;

struct alt_tse_mdio {
	unsigned int control;	/*PHY device operation control register */
	unsigned int status;	/*PHY device operation status register */
	unsigned int phy_id1;	/*Bits 31:16 of PHY identifier. */
	unsigned int phy_id2;	/*Bits 15:0 of PHY identifier. */
	unsigned int auto_negotiation_advertisement;
	unsigned int remote_partner_base_page_ability;

	unsigned int reg6;
	unsigned int reg7;
	unsigned int reg8;
	unsigned int reg9;
	unsigned int rega;
	unsigned int regb;
	unsigned int regc;
	unsigned int regd;
	unsigned int rege;
	unsigned int regf;
	unsigned int reg10;
	unsigned int reg11;
	unsigned int reg12;
	unsigned int reg13;
	unsigned int reg14;
	unsigned int reg15;
	unsigned int reg16;
	unsigned int reg17;
	unsigned int reg18;
	unsigned int reg19;
	unsigned int reg1a;
	unsigned int reg1b;
	unsigned int reg1c;
	unsigned int reg1d;
	unsigned int reg1e;
	unsigned int reg1f;
};

/* MAC register Space */

struct alt_tse_mac {
	unsigned int megacore_revision;
	unsigned int scratch_pad;
	alt_tse_command_config command_config;
	unsigned int mac_addr_0;
	unsigned int mac_addr_1;
	unsigned int max_frame_length;
	unsigned int pause_quanta;
	unsigned int rx_sel_empty_threshold;
	unsigned int rx_sel_full_threshold;
	unsigned int tx_sel_empty_threshold;
	unsigned int tx_sel_full_threshold;
	unsigned int rx_almost_empty_threshold;
	unsigned int rx_almost_full_threshold;
	unsigned int tx_almost_empty_threshold;
	unsigned int tx_almost_full_threshold;
	unsigned int mdio_phy0_addr;
	unsigned int mdio_phy1_addr;

	/* only if 100/1000 BaseX PCS, reserved otherwise */
	unsigned int reservedx44[5];

	unsigned int reg_read_access_status;
	unsigned int min_tx_ipg_length;

	/* IEEE 802.3 oEntity Managed Object Support */
	unsigned int aMACID_1;	/*The MAC addresses */
	unsigned int aMACID_2;
	unsigned int aFramesTransmittedOK;
	unsigned int aFramesReceivedOK;
	unsigned int aFramesCheckSequenceErrors;
	unsigned int aAlignmentErrors;
	unsigned int aOctetsTransmittedOK;
	unsigned int aOctetsReceivedOK;

	/* IEEE 802.3 oPausedEntity Managed Object Support */
	unsigned int aTxPAUSEMACCtrlFrames;
	unsigned int aRxPAUSEMACCtrlFrames;

	/* IETF MIB (MIB-II) Object Support */
	unsigned int ifInErrors;
	unsigned int ifOutErrors;
	unsigned int ifInUcastPkts;
	unsigned int ifInMulticastPkts;
	unsigned int ifInBroadcastPkts;
	unsigned int ifOutDiscards;
	unsigned int ifOutUcastPkts;
	unsigned int ifOutMulticastPkts;
	unsigned int ifOutBroadcastPkts;

	/* IETF RMON MIB Object Support */
	unsigned int etherStatsDropEvent;
	unsigned int etherStatsOctets;
	unsigned int etherStatsPkts;
	unsigned int etherStatsUndersizePkts;
	unsigned int etherStatsOversizePkts;
	unsigned int etherStatsPkts64Octets;
	unsigned int etherStatsPkts65to127Octets;
	unsigned int etherStatsPkts128to255Octets;
	unsigned int etherStatsPkts256to511Octets;
	unsigned int etherStatsPkts512to1023Octets;
	unsigned int etherStatsPkts1024to1518Octets;

	unsigned int etherStatsPkts1519toXOctets;
	unsigned int etherStatsJabbers;
	unsigned int etherStatsFragments;

	unsigned int reservedxE4;

	/*FIFO control register. */
	alt_tse_tx_cmd_stat tx_cmd_stat;
	alt_tse_rx_cmd_stat rx_cmd_stat;

	unsigned int ipaccTxConf;
	unsigned int ipaccRxConf;
	unsigned int ipaccRxStat;
	unsigned int ipaccRxStatSum;

	/*Multicast address resolution table */
	unsigned int hash_table[64];

	/*Registers 0 to 31 within PHY device 0/1 */
	struct alt_tse_mdio mdio_phy0;
	struct alt_tse_mdio mdio_phy1;

	/*4 Supplemental MAC Addresses */
	unsigned int supp_mac_addr_0_0;
	unsigned int supp_mac_addr_0_1;
	unsigned int supp_mac_addr_1_0;
	unsigned int supp_mac_addr_1_1;
	unsigned int supp_mac_addr_2_0;
	unsigned int supp_mac_addr_2_1;
	unsigned int supp_mac_addr_3_0;
	unsigned int supp_mac_addr_3_1;

	unsigned int reservedx320[56];
};

/* flags: TSE MII modes */
/* GMII/MII	= 0 */
/* RGMII	= 1 */
/* RGMII_ID	= 2 */
/* RGMII_TXID	= 3 */
/* RGMII_RXID	= 4 */
/* SGMII	= 5 */
struct altera_tse_priv {
	char devname[16];
	volatile struct alt_tse_mac *mac_dev;
	volatile struct alt_sgdma_registers *sgdma_rx;
	volatile struct alt_sgdma_registers *sgdma_tx;
	unsigned int rx_sgdma_irq;
	unsigned int tx_sgdma_irq;
	unsigned int has_descriptor_mem;
	unsigned int descriptor_mem_base;
	unsigned int descriptor_mem_size;
	volatile struct alt_sgdma_descriptor *rx_desc;
	volatile struct alt_sgdma_descriptor *tx_desc;
	volatile unsigned char *rx_buf;
	struct phy_info *phyinfo;
	unsigned int phyaddr;
	unsigned int flags;
	unsigned int link;
	unsigned int duplexity;
	unsigned int speed;
};

/* Phy stuff continued */
/*
 * struct phy_cmd:  A command for reading or writing a PHY register
 *
 * mii_reg:  The register to read or write
 *
 * mii_data:  For writes, the value to put in the register.
 *	A value of -1 indicates this is a read.
 *
 * funct: A function pointer which is invoked for each command.
 *	For reads, this function will be passed the value read
 *	from the PHY, and process it.
 *	For writes, the result of this function will be written
 *	to the PHY register
 */
struct phy_cmd {
	uint mii_reg;
	uint mii_data;
	uint(*funct) (uint mii_reg, struct altera_tse_priv *priv);
};
#endif /* _ALTERA_TSE_H_ */
