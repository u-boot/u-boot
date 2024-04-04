/* SPDX-License-Identifier: GPL-2.0 */
/* Texas Instruments ICSSG Ethernet driver
 *
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef __NET_TI_ICSSG_CONFIG_H
#define __NET_TI_ICSSG_CONFIG_H

struct icssg_buffer_pool_cfg {
	__le32  addr;
	__le32  len;
} __packed;

struct icssg_flow_cfg {
	__le16 rx_base_flow;
	__le16 mgm_base_flow;
} __packed;

/* Config area lies in shared RAM */
#define ICSSG_CONFIG_OFFSET_SLICE0   0
#define ICSSG_CONFIG_OFFSET_SLICE1   0x8000

/* pstate speed/duplex command to set speed and duplex settings
 * in firmware.
 * Command format : 0x8102ssPN. ss - sequence number: currently not
 * used by driver, P - port number: For switch, N - Speed/Duplex state
 * - Possible values of N:
 * 0x0 - 10Mbps/Half duplex ;
 * 0x8 - 10Mbps/Full duplex ;
 * 0x2 - 100Mbps/Half duplex;
 * 0xa - 100Mbps/Full duplex;
 * 0xc - 1Gbps/Full duplex;
 * NOTE: The above are same as bits [3..1](slice 0) or bits [8..6](slice 1) of
 * RGMII CFG register. So suggested to read the register to populate the command
 * bits.
 */
#define ICSSG_PSTATE_SPEED_DUPLEX_CMD	0x81020000
#define ICSSG_PSTATE_FULL_DUPLEX	BIT(3)
#define ICSSG_PSTATE_SPEED_100		BIT(1)
#define ICSSG_PSTATE_SPEED_1000		BIT(2)

/* Flow IDs used in config structure to firmware. Should match with
 * flow_id in struct dma for rx channels.
 */
#define ICSSG_RX_CHAN_FLOW_ID		0 /* flow id for host port */
#define ICSSG_RX_MGM_CHAN_FLOW_ID	1 /* flow id for command response */

/* Used to notify the FW of the current link speed */
#define PORT_LINK_SPEED_OFFSET			   0x00A8

#define FW_LINK_SPEED_1G                           (0x00)
#define FW_LINK_SPEED_100M                         (0x01)
#define FW_LINK_SPEED_10M                          (0x02)
#define FW_LINK_SPEED_HD                           (0x80)

#define PRUETH_PKT_TYPE_CMD	0x10
#define PRUETH_NAV_PS_DATA_SIZE	16	/* Protocol specific data size */
#define PRUETH_NAV_SW_DATA_SIZE	16	/* SW related data size */
#define PRUETH_MAX_RX_FLOWS	1	/* excluding default flow */
#define PRUETH_RX_FLOW_DATA	0	/* FIXME: f/w bug to change to highest priority flow */

#define PRUETH_EMAC_BUF_POOL_SIZE	SZ_8K
#define PRUETH_EMAC_POOLS_PER_SLICE	24
#define PRUETH_EMAC_BUF_POOL_START	8
#define PRUETH_NUM_BUF_POOLS	8
#define PRUETH_EMAC_RX_CTX_BUF_SIZE	SZ_16K	/* per slice */
#define MSMC_RAM_SIZE	(2 * (PRUETH_EMAC_BUF_POOL_SIZE * PRUETH_NUM_BUF_POOLS + \
			      PRUETH_EMAC_RX_CTX_BUF_SIZE))

struct icssg_rxq_ctx {
	__le32 start[3];
	__le32 end;
} __packed;

/* Load time Fiwmware Configuration */

#define ICSSG_FW_MGMT_CMD_HEADER	0x81
#define ICSSG_FW_MGMT_FDB_CMD_TYPE	0x03
#define ICSSG_FW_MGMT_CMD_TYPE		0x04
#define ICSSG_FW_MGMT_PKT		0x80000000
#define ICSSG_FW_MGMT_FDB_CMD_TYPE_RX_FLOW	0x05

struct icssg_r30_cmd {
	u32 cmd[4];
} __packed;

enum icssg_port_state_cmd {
	ICSSG_EMAC_PORT_DISABLE = 0,
	ICSSG_EMAC_PORT_BLOCK,
	ICSSG_EMAC_PORT_FORWARD,
	ICSSG_EMAC_PORT_FORWARD_WO_LEARNING,
	ICSSG_EMAC_PORT_ACCEPT_ALL,
	ICSSG_EMAC_PORT_ACCEPT_TAGGED,
	ICSSG_EMAC_PORT_ACCEPT_UNTAGGED_N_PRIO,
	ICSSG_EMAC_PORT_TAS_TRIGGER,
	ICSSG_EMAC_PORT_TAS_ENABLE,
	ICSSG_EMAC_PORT_TAS_RESET,
	ICSSG_EMAC_PORT_TAS_DISABLE,
	ICSSG_EMAC_PORT_UC_FLOODING_ENABLE,
	ICSSG_EMAC_PORT_UC_FLOODING_DISABLE,
	ICSSG_EMAC_PORT_MC_FLOODING_ENABLE,
	ICSSG_EMAC_PORT_MC_FLOODING_DISABLE,
	ICSSG_EMAC_PORT_PREMPT_TX_ENABLE,
	ICSSG_EMAC_PORT_PREMPT_TX_DISABLE,
	ICSSG_EMAC_PORT_MAX_COMMANDS
};

#define EMAC_NONE           0xffff0000
#define EMAC_PRU0_P_DI      0xffff0004
#define EMAC_PRU1_P_DI      0xffff0040
#define EMAC_TX_P_DI        0xffff0100

#define EMAC_PRU0_P_EN      0xfffb0000
#define EMAC_PRU1_P_EN      0xffbf0000
#define EMAC_TX_P_EN        0xfeff0000

#define EMAC_P_BLOCK        0xffff0040
#define EMAC_TX_P_BLOCK     0xffff0200
#define EMAC_P_UNBLOCK      0xffbf0000
#define EMAC_TX_P_UNBLOCK   0xfdff0000
#define EMAC_LEAN_EN        0xfff70000
#define EMAC_LEAN_DI        0xffff0008

#define EMAC_ACCEPT_ALL     0xffff0001
#define EMAC_ACCEPT_TAG     0xfffe0002
#define EMAC_ACCEPT_PRIOR   0xfffc0000

/* Config area lies in DRAM */
#define ICSSG_CONFIG_OFFSET			0x0

#define ICSSG_NUM_NORMAL_PDS	64
#define ICSSG_NUM_SPECIAL_PDS	16

#define ICSSG_NORMAL_PD_SIZE	8
#define ICSSG_SPECIAL_PD_SIZE	20

#define ICSSG_FLAG_MASK		0xff00ffff

struct icssg_setclock_desc {
	u8 request;
	u8 restore;
	u8 acknowledgment;
	u8 cmp_status;
	u32 margin;
	u32 cyclecounter0_set;
	u32 cyclecounter1_set;
	u32 iepcount_set;
	u32 rsvd1;
	u32 rsvd2;
	u32 CMP0_current;
	u32 iepcount_current;
	u32 difference;
	u32 cyclecounter0_new;
	u32 cyclecounter1_new;
	u32 CMP0_new;
} __packed;

struct mgmt_cmd {
	u8 param;
	u8 seqnum;
	u8 type;
	u8 header;
	u32 cmd_args[3];
} __packed;

struct mgmt_cmd_rsp {
	u32 reserved;
	u8 status;
	u8 seqnum;
	u8 type;
	u8 header;
	u32 cmd_args[3];
} __packed;

#define ICSSG_CMD_POP_SLICE0	56
#define ICSSG_CMD_POP_SLICE1	60

#define ICSSG_CMD_PUSH_SLICE0	57
#define ICSSG_CMD_PUSH_SLICE1	61

#define ICSSG_RSP_POP_SLICE0	58
#define ICSSG_RSP_POP_SLICE1	62

#define ICSSG_RSP_PUSH_SLICE0	56
#define ICSSG_RSP_PUSH_SLICE1	60

#define ICSSG_TS_POP_SLICE0	59
#define ICSSG_TS_POP_SLICE1	63

#define ICSSG_TS_PUSH_SLICE0	40
#define ICSSG_TS_PUSH_SLICE1	41

#endif /* __NET_TI_ICSSG_CONFIG_H */
