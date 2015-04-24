/*
 * Copyright (C) 2013-2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _FSL_DPNI_H
#define _FSL_DPNI_H

/* DPNI Version */
#define DPNI_VER_MAJOR				4
#define DPNI_VER_MINOR				0

/* Command IDs */
#define DPNI_CMDID_OPEN				0x801
#define DPNI_CMDID_CLOSE			0x800

#define DPNI_CMDID_ENABLE			0x002
#define DPNI_CMDID_DISABLE			0x003
#define DPNI_CMDID_GET_ATTR			0x004
#define DPNI_CMDID_RESET			0x005

#define DPNI_CMDID_SET_POOLS			0x200
#define DPNI_CMDID_GET_RX_BUFFER_LAYOUT		0x201
#define DPNI_CMDID_SET_RX_BUFFER_LAYOUT		0x202
#define DPNI_CMDID_GET_TX_BUFFER_LAYOUT		0x203
#define DPNI_CMDID_SET_TX_BUFFER_LAYOUT		0x204
#define DPNI_CMDID_SET_TX_CONF_BUFFER_LAYOUT	0x205
#define DPNI_CMDID_GET_TX_CONF_BUFFER_LAYOUT	0x206

#define DPNI_CMDID_GET_QDID			0x210
#define DPNI_CMDID_GET_TX_DATA_OFFSET		0x212
#define DPNI_CMDID_GET_COUNTER			0x213
#define DPNI_CMDID_SET_COUNTER			0x214
#define DPNI_CMDID_GET_LINK_STATE		0x215
#define DPNI_CMDID_SET_LINK_CFG		0x21A

#define DPNI_CMDID_SET_PRIM_MAC			0x224
#define DPNI_CMDID_GET_PRIM_MAC			0x225
#define DPNI_CMDID_ADD_MAC_ADDR			0x226
#define DPNI_CMDID_REMOVE_MAC_ADDR		0x227

#define DPNI_CMDID_SET_TX_FLOW			0x236
#define DPNI_CMDID_GET_TX_FLOW			0x237
#define DPNI_CMDID_SET_RX_FLOW			0x238
#define DPNI_CMDID_GET_RX_FLOW			0x239

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_OPEN(cmd, dpni_id) \
	MC_CMD_OP(cmd,	 0,	0,	32,	int,	dpni_id)


/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_POOLS(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  8,  uint8_t,  cfg->num_dpbp); \
	MC_CMD_OP(cmd, 0, 32, 32, int,      cfg->pools[0].dpbp_id); \
	MC_CMD_OP(cmd, 4, 32, 16, uint16_t, cfg->pools[0].buffer_size);\
	MC_CMD_OP(cmd, 1, 0,  32, int,      cfg->pools[1].dpbp_id); \
	MC_CMD_OP(cmd, 4, 48, 16, uint16_t, cfg->pools[1].buffer_size);\
	MC_CMD_OP(cmd, 1, 32, 32, int,      cfg->pools[2].dpbp_id); \
	MC_CMD_OP(cmd, 5, 0,  16, uint16_t, cfg->pools[2].buffer_size);\
	MC_CMD_OP(cmd, 2, 0,  32, int,      cfg->pools[3].dpbp_id); \
	MC_CMD_OP(cmd, 5, 16, 16, uint16_t, cfg->pools[3].buffer_size);\
	MC_CMD_OP(cmd, 2, 32, 32, int,      cfg->pools[4].dpbp_id); \
	MC_CMD_OP(cmd, 5, 32, 16, uint16_t, cfg->pools[4].buffer_size);\
	MC_CMD_OP(cmd, 3, 0,  32, int,      cfg->pools[5].dpbp_id); \
	MC_CMD_OP(cmd, 5, 48, 16, uint16_t, cfg->pools[5].buffer_size);\
	MC_CMD_OP(cmd, 3, 32, 32, int,      cfg->pools[6].dpbp_id); \
	MC_CMD_OP(cmd, 6, 0,  16, uint16_t, cfg->pools[6].buffer_size);\
	MC_CMD_OP(cmd, 4, 0,  32, int,      cfg->pools[7].dpbp_id); \
	MC_CMD_OP(cmd, 6, 16, 16, uint16_t, cfg->pools[7].buffer_size);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_ATTR(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,	    attr->id);\
	MC_RSP_OP(cmd, 0, 32, 8,  uint8_t,  attr->max_tcs); \
	MC_RSP_OP(cmd, 0, 40, 8,  uint8_t,  attr->max_senders); \
	MC_RSP_OP(cmd, 0, 48, 8,  enum net_prot, attr->start_hdr); \
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, attr->options); \
	MC_RSP_OP(cmd, 2, 0,  8,  uint8_t,  attr->max_unicast_filters); \
	MC_RSP_OP(cmd, 2, 8,  8,  uint8_t,  attr->max_multicast_filters);\
	MC_RSP_OP(cmd, 2, 16, 8,  uint8_t,  attr->max_vlan_filters); \
	MC_RSP_OP(cmd, 2, 24, 8,  uint8_t,  attr->max_qos_entries); \
	MC_RSP_OP(cmd, 2, 32, 8,  uint8_t,  attr->max_qos_key_size); \
	MC_RSP_OP(cmd, 2, 40, 8,  uint8_t,  attr->max_dist_key_size); \
	MC_RSP_OP(cmd, 3, 0,  8,  uint8_t,  attr->max_dist_per_tc[0]); \
	MC_RSP_OP(cmd, 3, 8,  8,  uint8_t,  attr->max_dist_per_tc[1]); \
	MC_RSP_OP(cmd, 3, 16, 8,  uint8_t,  attr->max_dist_per_tc[2]); \
	MC_RSP_OP(cmd, 3, 24, 8,  uint8_t,  attr->max_dist_per_tc[3]); \
	MC_RSP_OP(cmd, 3, 32, 8,  uint8_t,  attr->max_dist_per_tc[4]); \
	MC_RSP_OP(cmd, 3, 40, 8,  uint8_t,  attr->max_dist_per_tc[5]); \
	MC_RSP_OP(cmd, 3, 48, 8,  uint8_t,  attr->max_dist_per_tc[6]); \
	MC_RSP_OP(cmd, 3, 56, 8,  uint8_t,  attr->max_dist_per_tc[7]); \
	MC_RSP_OP(cmd, 4, 0,	16, uint16_t, \
				    attr->ipr_cfg.max_reass_frm_size); \
	MC_RSP_OP(cmd, 4, 16,	16, uint16_t, \
				    attr->ipr_cfg.min_frag_size_ipv4); \
	MC_RSP_OP(cmd, 4, 32,	16, uint16_t, \
				    attr->ipr_cfg.min_frag_size_ipv6); \
	MC_RSP_OP(cmd, 5, 0,	16, uint16_t, \
				  attr->ipr_cfg.max_open_frames_ipv4); \
	MC_RSP_OP(cmd, 5, 16,	16, uint16_t, \
				  attr->ipr_cfg.max_open_frames_ipv6); \
	MC_RSP_OP(cmd, 5, 32, 16, uint16_t, attr->version.major);\
	MC_RSP_OP(cmd, 5, 48, 16, uint16_t, attr->version.minor);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_RX_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_RSP_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_RSP_OP(cmd, 1, 0,  1,  int,	    layout->pass_timestamp); \
	MC_RSP_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_RSP_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_RSP_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_RSP_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_RX_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_CMD_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_CMD_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_CMD_OP(cmd, 0, 32, 32, uint32_t, layout->options); \
	MC_CMD_OP(cmd, 1, 0,  1,  int,	    layout->pass_timestamp); \
	MC_CMD_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_CMD_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_CMD_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_CMD_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_TX_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_RSP_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_RSP_OP(cmd, 1, 0,  1,  int,      layout->pass_timestamp); \
	MC_RSP_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_RSP_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_RSP_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_RSP_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_TX_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_CMD_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_CMD_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_CMD_OP(cmd, 0, 32, 32, uint32_t, layout->options); \
	MC_CMD_OP(cmd, 1, 0,  1,  int,	    layout->pass_timestamp); \
	MC_CMD_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_CMD_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_CMD_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_CMD_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_TX_CONF_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_RSP_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_RSP_OP(cmd, 1, 0,  1,  int,      layout->pass_timestamp); \
	MC_RSP_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_RSP_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_RSP_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_RSP_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_TX_CONF_BUFFER_LAYOUT(cmd, layout) \
do { \
	MC_CMD_OP(cmd, 0, 0,  16, uint16_t, layout->private_data_size); \
	MC_CMD_OP(cmd, 0, 16, 16, uint16_t, layout->data_align); \
	MC_CMD_OP(cmd, 0, 32, 32, uint32_t, layout->options); \
	MC_CMD_OP(cmd, 1, 0,  1,  int,	    layout->pass_timestamp); \
	MC_CMD_OP(cmd, 1, 1,  1,  int,	    layout->pass_parser_result); \
	MC_CMD_OP(cmd, 1, 2,  1,  int,	    layout->pass_frame_status); \
	MC_CMD_OP(cmd, 1, 16, 16, uint16_t, layout->data_head_room); \
	MC_CMD_OP(cmd, 1, 32, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_QDID(cmd, qdid) \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, qdid)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_TX_DATA_OFFSET(cmd, data_offset) \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, data_offset)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_GET_COUNTER(cmd, counter) \
	MC_CMD_OP(cmd, 0, 0,  16, enum dpni_counter, counter)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_COUNTER(cmd, value) \
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, value)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_COUNTER(cmd, counter, value) \
do { \
	MC_CMD_OP(cmd, 0, 0,  16, enum dpni_counter, counter); \
	MC_CMD_OP(cmd, 1, 0,  64, uint64_t, value); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_LINK_CFG(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 1, 0,  64, uint64_t, cfg->rate);\
	MC_CMD_OP(cmd, 2, 0,  64, uint64_t, cfg->options);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_LINK_STATE(cmd, state) \
do { \
	MC_RSP_OP(cmd, 0, 32,  1, int,      state->up);\
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, state->rate);\
	MC_RSP_OP(cmd, 2, 0,  64, uint64_t, state->options);\
} while (0)



/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_PRIMARY_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_PRIMARY_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_RSP_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_RSP_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_RSP_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_RSP_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_RSP_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_RSP_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_ADD_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_REMOVE_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_TX_FLOW(cmd, flow_id, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,     \
			   cfg->conf_err_cfg.queue_cfg.dest_cfg.dest_id);\
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t, \
			   cfg->conf_err_cfg.queue_cfg.dest_cfg.priority);\
	MC_CMD_OP(cmd, 0, 40, 2,  enum dpni_dest, \
			   cfg->conf_err_cfg.queue_cfg.dest_cfg.dest_type);\
	MC_CMD_OP(cmd, 0, 42, 1,  int,	    cfg->conf_err_cfg.errors_only);\
	MC_CMD_OP(cmd, 0, 43, 1,  int,	    cfg->l3_chksum_gen);\
	MC_CMD_OP(cmd, 0, 44, 1,  int,	    cfg->l4_chksum_gen);\
	MC_CMD_OP(cmd, 0, 45, 1,  int,	    \
			   cfg->conf_err_cfg.use_default_queue);\
	MC_CMD_OP(cmd, 0, 48, 16, uint16_t, flow_id);\
	MC_CMD_OP(cmd, 1, 0,  64, uint64_t, \
			   cfg->conf_err_cfg.queue_cfg.user_ctx);\
	MC_CMD_OP(cmd, 2, 0,  32, uint32_t, cfg->options);\
	MC_CMD_OP(cmd, 2, 32,  32, uint32_t, \
			   cfg->conf_err_cfg.queue_cfg.options);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_SET_TX_FLOW(cmd, flow_id) \
	MC_RSP_OP(cmd, 0, 48, 16, uint16_t, flow_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_GET_TX_FLOW(cmd, flow_id) \
	MC_CMD_OP(cmd, 0, 48, 16, uint16_t, flow_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_TX_FLOW(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,      \
			attr->conf_err_attr.queue_attr.dest_cfg.dest_id);\
	MC_RSP_OP(cmd, 0, 32, 8,  uint8_t,  \
			attr->conf_err_attr.queue_attr.dest_cfg.priority);\
	MC_RSP_OP(cmd, 0, 40, 2,  enum dpni_dest, \
			attr->conf_err_attr.queue_attr.dest_cfg.dest_type);\
	MC_RSP_OP(cmd, 0, 42, 1,  int,	    attr->conf_err_attr.errors_only);\
	MC_RSP_OP(cmd, 0, 43, 1,  int,	    attr->l3_chksum_gen);\
	MC_RSP_OP(cmd, 0, 44, 1,  int,	    attr->l4_chksum_gen);\
	MC_RSP_OP(cmd, 0, 45, 1,  int,	    \
			attr->conf_err_attr.use_default_queue);\
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, \
			attr->conf_err_attr.queue_attr.user_ctx);\
	MC_RSP_OP(cmd, 2, 32, 32, uint32_t, \
			attr->conf_err_attr.queue_attr.fqid);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_RX_FLOW(cmd, tc_id, flow_id, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,      cfg->dest_cfg.dest_id); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  cfg->dest_cfg.priority);\
	MC_CMD_OP(cmd, 0, 40, 2,  enum dpni_dest, cfg->dest_cfg.dest_type);\
	MC_CMD_OP(cmd, 0, 48, 16, uint16_t, flow_id); \
	MC_CMD_OP(cmd, 1, 0,  64, uint64_t, cfg->user_ctx); \
	MC_CMD_OP(cmd, 2, 16, 8,  uint8_t,  tc_id); \
	MC_CMD_OP(cmd, 2, 32,  32, uint32_t, cfg->options); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_GET_RX_FLOW(cmd, tc_id, flow_id) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  tc_id); \
	MC_CMD_OP(cmd, 0, 48, 16, uint16_t, flow_id); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_RX_FLOW(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,      attr->dest_cfg.dest_id); \
	MC_RSP_OP(cmd, 0, 32, 8,  uint8_t,  attr->dest_cfg.priority);\
	MC_RSP_OP(cmd, 0, 40, 2,  enum dpni_dest, attr->dest_cfg.dest_type); \
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, attr->user_ctx); \
	MC_RSP_OP(cmd, 2, 32, 32, uint32_t, attr->fqid); \
} while (0)

enum net_prot {
	NET_PROT_NONE = 0,
	NET_PROT_PAYLOAD,
	NET_PROT_ETH,
	NET_PROT_VLAN,
	NET_PROT_IPV4,
	NET_PROT_IPV6,
	NET_PROT_IP,
	NET_PROT_TCP,
	NET_PROT_UDP,
	NET_PROT_UDP_LITE,
	NET_PROT_IPHC,
	NET_PROT_SCTP,
	NET_PROT_SCTP_CHUNK_DATA,
	NET_PROT_PPPOE,
	NET_PROT_PPP,
	NET_PROT_PPPMUX,
	NET_PROT_PPPMUX_SUBFRM,
	NET_PROT_L2TPV2,
	NET_PROT_L2TPV3_CTRL,
	NET_PROT_L2TPV3_SESS,
	NET_PROT_LLC,
	NET_PROT_LLC_SNAP,
	NET_PROT_NLPID,
	NET_PROT_SNAP,
	NET_PROT_MPLS,
	NET_PROT_IPSEC_AH,
	NET_PROT_IPSEC_ESP,
	NET_PROT_UDP_ENC_ESP, /* RFC 3948 */
	NET_PROT_MACSEC,
	NET_PROT_GRE,
	NET_PROT_MINENCAP,
	NET_PROT_DCCP,
	NET_PROT_ICMP,
	NET_PROT_IGMP,
	NET_PROT_ARP,
	NET_PROT_CAPWAP_DATA,
	NET_PROT_CAPWAP_CTRL,
	NET_PROT_RFC2684,
	NET_PROT_ICMPV6,
	NET_PROT_FCOE,
	NET_PROT_FIP,
	NET_PROT_ISCSI,
	NET_PROT_GTP,
	NET_PROT_USER_DEFINED_L2,
	NET_PROT_USER_DEFINED_L3,
	NET_PROT_USER_DEFINED_L4,
	NET_PROT_USER_DEFINED_L5,
	NET_PROT_USER_DEFINED_SHIM1,
	NET_PROT_USER_DEFINED_SHIM2,

	NET_PROT_DUMMY_LAST
};

/* Data Path Network Interface API
 * Contains initialization APIs and runtime control APIs for DPNI
 */

struct fsl_mc_io;

/* General DPNI macros */

/* Maximum number of traffic classes */
#define DPNI_MAX_TC				8
/* Maximum number of buffer pools per DPNI */
#define DPNI_MAX_DPBP				8

/* All traffic classes considered; see dpni_set_rx_flow() */
#define DPNI_ALL_TCS				(uint8_t)(-1)
/* All flows within traffic class considered; see dpni_set_rx_flow() */
#define DPNI_ALL_TC_FLOWS			(uint16_t)(-1)
/* Generate new flow ID; see dpni_set_tx_flow() */
#define DPNI_NEW_FLOW_ID			(uint16_t)(-1)

/**
 * dpni_open() - Open a control session for the specified object
 * @mc_io:	Pointer to MC portal's I/O object
 * @dpni_id:	DPNI unique ID
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpni_create() function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_open(struct fsl_mc_io *mc_io, int dpni_id, uint16_t *token);

/**
 * dpni_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_close(struct fsl_mc_io *mc_io, uint16_t token);

/**
 * struct dpni_ipr_cfg - Structure representing IP reassembly configuration
 * @max_reass_frm_size: Maximum size of the reassembled frame
 * @min_frag_size_ipv4: Minimum fragment size of IPv4 fragments
 * @min_frag_size_ipv6: Minimum fragment size of IPv6 fragments
 * @max_open_frames_ipv4: Maximum concurrent IPv4 packets in reassembly process
 * @max_open_frames_ipv6: Maximum concurrent IPv6 packets in reassembly process
 */
struct dpni_ipr_cfg {
	uint16_t max_reass_frm_size;
	uint16_t min_frag_size_ipv4;
	uint16_t min_frag_size_ipv6;
	uint16_t max_open_frames_ipv4;
	uint16_t max_open_frames_ipv6;
};

/**
 * struct dpni_pools_cfg - Structure representing buffer pools configuration
 * @num_dpbp: Number of DPBPs
 * @pools: Array of buffer pools parameters; The number of valid entries
 *	must match 'num_dpbp' value
 */
struct dpni_pools_cfg {
	uint8_t num_dpbp;
	/**
	 * struct pools - Buffer pools parameters
	 * @dpbp_id: DPBP object ID
	 * @buffer_size: Buffer size
	 */
	struct {
		int dpbp_id;
		uint16_t buffer_size;
	} pools[DPNI_MAX_DPBP];
};

/**
 * dpni_set_pools() - Set buffer pools configuration
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @cfg:	Buffer pools configuration
 *
 * mandatory for DPNI operation
 * warning:Allowed only when DPNI is disabled
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_pools(struct fsl_mc_io		*mc_io,
		   uint16_t			token,
		   const struct dpni_pools_cfg	*cfg);

/**
 * dpni_enable() - Enable the DPNI, allow sending and receiving frames.
 * @mc_io:		Pointer to MC portal's I/O object
 * @token:		Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_enable(struct fsl_mc_io *mc_io, uint16_t token);

/**
 * dpni_disable() - Disable the DPNI, stop sending and receiving frames.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_disable(struct fsl_mc_io *mc_io, uint16_t token);


/**
 * @dpni_reset() - Reset the DPNI, returns the object to initial state.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_reset(struct fsl_mc_io *mc_io, uint16_t token);

/**
 * struct dpni_attr - Structure representing DPNI attributes
 * @id: DPNI object ID
 * @version: DPNI version
 * @start_hdr: Indicates the packet starting header for parsing
 * @options: Mask of available options; reflects the value as was given in
 *		object's creation
 * @max_senders: Maximum number of different senders; used as the number
 *		of dedicated Tx flows;
 * @max_tcs: Maximum number of traffic classes (for both Tx and Rx)
 * @max_dist_per_tc: Maximum distribution size per Rx traffic class;
 *			Set to the required value minus 1
 * @max_unicast_filters: Maximum number of unicast filters
 * @max_multicast_filters: Maximum number of multicast filters
 * @max_vlan_filters: Maximum number of VLAN filters
 * @max_qos_entries: if 'max_tcs > 1', declares the maximum entries in QoS table
 * @max_qos_key_size: Maximum key size for the QoS look-up
 * @max_dist_key_size: Maximum key size for the distribution look-up
 * @ipr_cfg: IP reassembly configuration
 */
struct dpni_attr {
	int id;
	/**
	 * struct version - DPNI version
	 * @major: DPNI major version
	 * @minor: DPNI minor version
	 */
	struct {
		uint16_t major;
		uint16_t minor;
	} version;
	enum net_prot start_hdr;
	uint64_t options;
	uint8_t max_senders;
	uint8_t max_tcs;
	uint8_t max_dist_per_tc[DPNI_MAX_TC];
	uint8_t max_unicast_filters;
	uint8_t max_multicast_filters;
	uint8_t max_vlan_filters;
	uint8_t max_qos_entries;
	uint8_t max_qos_key_size;
	uint8_t max_dist_key_size;
	struct dpni_ipr_cfg ipr_cfg;
};
/**
 * dpni_get_attributes() - Retrieve DPNI attributes.
 * @mc_io:	Pointer to MC portal's I/O objec
 * @token:	Token of DPNI object
 * @attr:	Returned object's attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_attributes(struct fsl_mc_io	*mc_io,
			uint16_t		token,
			struct dpni_attr	*attr);

/* DPNI buffer layout modification options */

/* Select to modify the time-stamp setting */
#define DPNI_BUF_LAYOUT_OPT_TIMESTAMP		0x00000001
/* Select to modify the parser-result setting; not applicable for Tx */
#define DPNI_BUF_LAYOUT_OPT_PARSER_RESULT	0x00000002
/* Select to modify the frame-status setting */
#define DPNI_BUF_LAYOUT_OPT_FRAME_STATUS	0x00000004
/* Select to modify the private-data-size setting */
#define DPNI_BUF_LAYOUT_OPT_PRIVATE_DATA_SIZE	0x00000008
/* Select to modify the data-alignment setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_ALIGN		0x00000010
/* Select to modify the data-head-room setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_HEAD_ROOM	0x00000020
/*!< Select to modify the data-tail-room setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_TAIL_ROOM	0x00000040

/**
 * struct dpni_buffer_layout - Structure representing DPNI buffer layout
 * @options: Flags representing the suggested modifications to the buffer
 *		layout; Use any combination of 'DPNI_BUF_LAYOUT_OPT_<X>' flags
 * @pass_timestamp: Pass timestamp value
 * @pass_parser_result: Pass parser results
 * @pass_frame_status: Pass frame status
 * @private_data_size: Size kept for private data (in bytes)
 * @data_align: Data alignment
 * @data_head_room: Data head room
 * @data_tail_room: Data tail room
 */
struct dpni_buffer_layout {
	uint32_t options;
	int pass_timestamp;
	int pass_parser_result;
	int pass_frame_status;
	uint16_t private_data_size;
	uint16_t data_align;
	uint16_t data_head_room;
	uint16_t data_tail_room;
};

/**
 * dpni_get_rx_buffer_layout() - Retrieve Rx buffer layout attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @layout:	Returns buffer layout attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_rx_buffer_layout(struct fsl_mc_io		*mc_io,
			      uint16_t			token,
			      struct dpni_buffer_layout	*layout);
/**
 * dpni_set_rx_buffer_layout() - Set Rx buffer layout configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @layout:	Buffer layout configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Allowed only when DPNI is disabled
 */
int dpni_set_rx_buffer_layout(struct fsl_mc_io			*mc_io,
			      uint16_t				token,
			      const struct dpni_buffer_layout	*layout);

/**
 * dpni_get_tx_buffer_layout() - Retrieve Tx buffer layout attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @layout:	Returns buffer layout attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_buffer_layout(struct fsl_mc_io		*mc_io,
			      uint16_t			token,
			      struct dpni_buffer_layout	*layout);

/**
 * @brief	Set Tx buffer layout configuration.
 *
 * @param[in]	mc_io	Pointer to MC portal's I/O object
 * @param[in]   token	Token of DPNI object
 * @param[in]	layout	Buffer layout configuration
 *
 * @returns	'0' on Success; Error code otherwise.
 *
 * @warning	Allowed only when DPNI is disabled
 */
int dpni_set_tx_buffer_layout(struct fsl_mc_io			*mc_io,
			      uint16_t				token,
			      const struct dpni_buffer_layout	*layout);
/**
 * dpni_get_tx_conf_buffer_layout() - Retrieve Tx confirmation buffer layout
 *				attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @layout:	Returns buffer layout attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_conf_buffer_layout(struct fsl_mc_io		*mc_io,
				   uint16_t			token,
				   struct dpni_buffer_layout	*layout);
/**
 * dpni_set_tx_conf_buffer_layout() - Set Tx confirmation buffer layout
 *					configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @layout:	Buffer layout configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Allowed only when DPNI is disabled
 */
int dpni_set_tx_conf_buffer_layout(struct fsl_mc_io		   *mc_io,
				   uint16_t			   token,
				   const struct dpni_buffer_layout *layout);
/**
 * dpni_get_spid() - Get the AIOP storage profile ID associated with the DPNI
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @spid:	Returned aiop storage-profile ID
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Only relevant for DPNI that belongs to AIOP container.
 */
int dpni_get_qdid(struct fsl_mc_io *mc_io, uint16_t token, uint16_t *qdid);

/**
 * dpni_get_tx_data_offset() - Get the Tx data offset (from start of buffer)
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @data_offset: Tx data offset (from start of buffer)
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_data_offset(struct fsl_mc_io	*mc_io,
			    uint16_t		token,
			    uint16_t		*data_offset);

/**
 * enum dpni_counter - DPNI counter types
 * @DPNI_CNT_ING_FRAME: Counts ingress frames
 * @DPNI_CNT_ING_BYTE: Counts ingress bytes
 * @DPNI_CNT_ING_FRAME_DROP: Counts ingress frames dropped due to explicit
 *		'drop' setting
 * @DPNI_CNT_ING_FRAME_DISCARD: Counts ingress frames discarded due to errors
 * @DPNI_CNT_ING_MCAST_FRAME: Counts ingress multicast frames
 * @DPNI_CNT_ING_MCAST_BYTE: Counts ingress multicast bytes
 * @DPNI_CNT_ING_BCAST_FRAME: Counts ingress broadcast frames
 * @DPNI_CNT_ING_BCAST_BYTES: Counts ingress broadcast bytes
 * @DPNI_CNT_EGR_FRAME: Counts egress frames
 * @DPNI_CNT_EGR_BYTE: Counts egress bytes
 * @DPNI_CNT_EGR_FRAME_DISCARD: Counts egress frames discarded due to errors
 */
enum dpni_counter {
	DPNI_CNT_ING_FRAME = 0x0,
	DPNI_CNT_ING_BYTE = 0x1,
	DPNI_CNT_ING_FRAME_DROP = 0x2,
	DPNI_CNT_ING_FRAME_DISCARD = 0x3,
	DPNI_CNT_ING_MCAST_FRAME = 0x4,
	DPNI_CNT_ING_MCAST_BYTE = 0x5,
	DPNI_CNT_ING_BCAST_FRAME = 0x6,
	DPNI_CNT_ING_BCAST_BYTES = 0x7,
	DPNI_CNT_EGR_FRAME = 0x8,
	DPNI_CNT_EGR_BYTE = 0x9,
	DPNI_CNT_EGR_FRAME_DISCARD = 0xa
};

/**
 * dpni_get_counter() - Read a specific DPNI counter
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @counter:	The requested counter
 * @value:	Returned counter's current value
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_counter(struct fsl_mc_io	*mc_io,
		     uint16_t		token,
		     enum dpni_counter	counter,
		     uint64_t		*value);

/**
 * dpni_set_counter() - Set (or clear) a specific DPNI counter
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @counter:	The requested counter
 * @value:	New counter value; typically pass '0' for resetting
 *			the counter.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_counter(struct fsl_mc_io	*mc_io,
		     uint16_t		token,
		     enum dpni_counter	counter,
		     uint64_t		value);
/**
 * struct - Structure representing DPNI link configuration
 * @rate: Rate
 * @options: Mask of available options; use 'DPNI_LINK_OPT_<X>' values
 */
struct dpni_link_cfg {
	uint64_t rate;
	uint64_t options;
};

/**
 * dpni_set_link_cfg() - set the link configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @cfg:	Link configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_link_cfg(struct fsl_mc_io *mc_io,
		      uint16_t token,
		      struct dpni_link_cfg *cfg);

/**
 * struct dpni_link_state - Structure representing DPNI link state
 * @rate: Rate
 * @options: Mask of available options; use 'DPNI_LINK_OPT_<X>' values
 * @up: Link state; '0' for down, '1' for up
 */
struct dpni_link_state {
	uint64_t rate;
	uint64_t options;
	int up;
};

/**
 * dpni_get_link_state() - Return the link state (either up or down)
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @state:	Returned link state;
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_link_state(struct fsl_mc_io *mc_io,
			uint16_t token,
			struct dpni_link_state *state);

/**
 * dpni_set_primary_mac_addr() - Set the primary MAC address
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to set as primary address
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_primary_mac_addr(struct fsl_mc_io	*mc_io,
			      uint16_t		token,
			      const uint8_t	mac_addr[6]);
/**
 * dpni_get_primary_mac_addr() - Get the primary MAC address
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @mac_addr:	Returned MAC address
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_primary_mac_addr(struct fsl_mc_io	*mc_io,
			      uint16_t		token,
			      uint8_t		mac_addr[6]);
/**
 * dpni_add_mac_addr() - Add MAC address filter
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to add
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_add_mac_addr(struct fsl_mc_io	*mc_io,
		      uint16_t		token,
		      const uint8_t	mac_addr[6]);

/**
 * dpni_remove_mac_addr() - Remove MAC address filter
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to remove
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_remove_mac_addr(struct fsl_mc_io	*mc_io,
			 uint16_t		token,
			 const uint8_t		mac_addr[6]);

/**
 * enum dpni_dest - DPNI destination types
 * DPNI_DEST_NONE: Unassigned destination; The queue is set in parked mode and
 *		does not generate FQDAN notifications; user is expected to
 *		dequeue from the queue based on polling or other user-defined
 *		method
 * @DPNI_DEST_DPIO: The queue is set in schedule mode and generates FQDAN
 *		notifications to the specified DPIO; user is expected to dequeue
 *		from the queue only after notification is received
 * @DPNI_DEST_DPCON: The queue is set in schedule mode and does not generate
 *		FQDAN notifications, but is connected to the specified DPCON
 *		object; user is expected to dequeue from the DPCON channel
 */
enum dpni_dest {
	DPNI_DEST_NONE = 0,
	DPNI_DEST_DPIO = 1,
	DPNI_DEST_DPCON = 2
};

/**
 * struct dpni_dest_cfg - Structure representing DPNI destination parameters
 * @dest_type: Destination type
 * @dest_id: Either DPIO ID or DPCON ID, depending on the destination type
 * @priority: Priority selection within the DPIO or DPCON channel; valid values
 *		are 0-1 or 0-7, depending on the number of priorities in that
 *		channel; not relevant for 'DPNI_DEST_NONE' option
 */
struct dpni_dest_cfg {
	enum dpni_dest dest_type;
	int dest_id;
	uint8_t priority;
};

/* DPNI queue modification options */

/* Select to modify the user's context associated with the queue */
#define DPNI_QUEUE_OPT_USER_CTX		0x00000001
/* Select to modify the queue's destination */
#define DPNI_QUEUE_OPT_DEST		0x00000002

/**
 * struct dpni_queue_cfg - Structure representing queue configuration
 * @options: Flags representing the suggested modifications to the queue;
 *		Use any combination of 'DPNI_QUEUE_OPT_<X>' flags
 * @user_ctx: User context value provided in the frame descriptor of each
 *		dequeued frame; valid only if 'DPNI_QUEUE_OPT_USER_CTX'
 *		is contained in 'options'
 * @dest_cfg: Queue destination parameters;
 *		valid only if 'DPNI_QUEUE_OPT_DEST' is contained in 'options'
 */
struct dpni_queue_cfg {
	uint32_t options;
	uint64_t user_ctx;
	struct dpni_dest_cfg dest_cfg;
};

/**
 * struct dpni_queue_attr - Structure representing queue attributes
 * @user_ctx: User context value provided in the frame descriptor of each
 *	dequeued frame
 * @dest_cfg: Queue destination configuration
 * @fqid: Virtual fqid value to be used for dequeue operations
 */
struct dpni_queue_attr {
	uint64_t user_ctx;
	struct dpni_dest_cfg dest_cfg;
	uint32_t fqid;
};

/* DPNI Tx flow modification options */

/* Select to modify the settings for dedicate Tx confirmation/error */
#define DPNI_TX_FLOW_OPT_TX_CONF_ERROR	0x00000001
/*!< Select to modify the Tx confirmation and/or error setting */
#define DPNI_TX_FLOW_OPT_ONLY_TX_ERROR	0x00000002
/*!< Select to modify the queue configuration */
#define DPNI_TX_FLOW_OPT_QUEUE		0x00000004
/*!< Select to modify the L3 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L3_CHKSUM_GEN	0x00000010
/*!< Select to modify the L4 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L4_CHKSUM_GEN	0x00000020

/**
 * struct dpni_tx_flow_cfg - Structure representing Tx flow configuration
 * @options: Flags representing the suggested modifications to the Tx flow;
 *		Use any combination 'DPNI_TX_FLOW_OPT_<X>' flags
 * @conf_err_cfg: Tx confirmation and error configuration; these settings are
 *		ignored if 'DPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED' was set at
 *		DPNI creation
 * @l3_chksum_gen: Set to '1' to enable L3 checksum generation; '0' to disable;
 *		valid only if 'DPNI_TX_FLOW_OPT_L3_CHKSUM_GEN' is contained in
 *		'options'
 * @l4_chksum_gen: Set to '1' to enable L4 checksum generation; '0' to disable;
 *		valid only if 'DPNI_TX_FLOW_OPT_L4_CHKSUM_GEN' is contained in
 *		'options'
 */
struct dpni_tx_flow_cfg {
	uint32_t options;
	/**
	 * struct cnf_err_cfg - Tx confirmation and error configuration
	 * @use_default_queue: Set to '1' to use the common (default) Tx
	 *		confirmation and error queue; Set to '0' to use the
	 *		private Tx confirmation and error queue; valid only if
	 *		'DPNI_TX_FLOW_OPT_TX_CONF_ERROR' is contained in
	 *		'options'
	 * @errors_only: Set to '1' to report back only error frames;
	 *		Set to '0' to confirm transmission/error for all
	 *		transmitted frames;
	 *		valid only if 'DPNI_TX_FLOW_OPT_ONLY_TX_ERROR' is
	 *		contained in 'options' and 'use_default_queue = 0';
	 * @queue_cfg: Queue configuration; valid only if
	 *		'DPNI_TX_FLOW_OPT_QUEUE' is contained in 'options'
	 */
	struct {
		int use_default_queue;
		int errors_only;
		struct dpni_queue_cfg queue_cfg;
	} conf_err_cfg;
	int l3_chksum_gen;
	int l4_chksum_gen;
};

/**
 * dpni_set_tx_flow() - Set Tx flow configuration
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @flow_id:	Provides (or returns) the sender's flow ID;
 *				for each new sender set (*flow_id) to
 *				'DPNI_NEW_FLOW_ID' to generate a new flow_id;
 *				this ID should be used as the QDBIN argument
 *				in enqueue operations
 * @cfg:	Tx flow configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_tx_flow(struct fsl_mc_io			*mc_io,
		     uint16_t				token,
		     uint16_t				*flow_id,
		     const struct dpni_tx_flow_cfg	*cfg);

/**
 * struct dpni_tx_flow_attr - Structure representing Tx flow attributes
 * @conf_err_attr: Tx confirmation and error attributes
 * @l3_chksum_gen: '1' if L3 checksum generation is enabled; '0' if disabled
 * @l4_chksum_gen: '1' if L4 checksum generation is enabled; '0' if disabled
 */
struct dpni_tx_flow_attr {
	/**
	 * struct conf_err_attr - Tx confirmation and error attributes
	 * @use_default_queue: '1' if using common (default) Tx confirmation and
	 *			error queue;
	 *			'0' if using private Tx confirmation and error
	 *			queue
	 * @errors_only: '1' if only error frames are reported back; '0' if all
	 *		transmitted frames are confirmed
	 * @queue_attr: Queue attributes
	 */
	struct {
		int use_default_queue;
		int errors_only;
		struct dpni_queue_attr queue_attr;
	} conf_err_attr;
	int l3_chksum_gen;
	int l4_chksum_gen;
};

/**
 * dpni_get_tx_flow() - Get Tx flow attributes
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @flow_id:	The sender's flow ID, as returned by the
 *			dpni_set_tx_flow() function
 * @attr:	Returned Tx flow attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_flow(struct fsl_mc_io		*mc_io,
		     uint16_t			token,
		     uint16_t			flow_id,
		     struct dpni_tx_flow_attr	*attr);

/**
 * dpni_set_rx_flow() - Set Rx flow configuration
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @tc_id:	Traffic class selection (0-7);
 *			use 'DPNI_ALL_TCS' to set all TCs and all flows
 * @flow_id	Rx flow id within the traffic class; use
 *			'DPNI_ALL_TC_FLOWS' to set all flows within
 *			this tc_id; ignored if tc_id is set to
 *			'DPNI_ALL_TCS';
 * @cfg:	Rx flow configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_rx_flow(struct fsl_mc_io			*mc_io,
		     uint16_t				token,
		     uint8_t				tc_id,
		     uint16_t				flow_id,
		     const struct dpni_queue_cfg	*cfg);

/**
 * dpni_get_rx_flow() -	Get Rx flow attributes
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Token of DPNI object
 * @tc_id:	Traffic class selection (0-7)
 * @flow_id:	Rx flow id within the traffic class
 * @attr:	Returned Rx flow attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_rx_flow(struct fsl_mc_io		*mc_io,
		     uint16_t			token,
		     uint8_t			tc_id,
		     uint16_t			flow_id,
		     struct dpni_queue_attr	*attr);

#endif /* _FSL_DPNI_H */
