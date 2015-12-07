/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LDPAA_ETH_H
#define __LDPAA_ETH_H

#include <linux/netdevice.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/fsl_dpaa_fd.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpni.h>
#include <fsl-mc/fsl_dpbp.h>
#include <fsl-mc/fsl_dpio.h>
#include <fsl-mc/fsl_qbman_portal.h>
#include <fsl-mc/fsl_mc_private.h>


enum ldpaa_eth_type {
	LDPAA_ETH_1G_E,
	LDPAA_ETH_10G_E,
};

/* Arbitrary values for now, but we'll need to tune */
#define LDPAA_ETH_NUM_BUFS		(2 * 7)
#define LDPAA_ETH_REFILL_THRESH		(LDPAA_ETH_NUM_BUFS/2)
#define LDPAA_ETH_RX_BUFFER_SIZE	2048

/* Hardware requires alignment for ingress/egress buffer addresses
 * and ingress buffer lengths.
 */
#define LDPAA_ETH_BUF_ALIGN		64

/* So far we're only accomodating a skb backpointer in the frame's
 * software annotation, but the hardware options are either 0 or 64.
 */
#define LDPAA_ETH_SWA_SIZE		64

/* Annotation valid bits in FD FRC */
#define LDPAA_FD_FRC_FASV		0x8000
#define LDPAA_FD_FRC_FAEADV		0x4000
#define LDPAA_FD_FRC_FAPRV		0x2000
#define LDPAA_FD_FRC_FAIADV		0x1000
#define LDPAA_FD_FRC_FASWOV		0x0800
#define LDPAA_FD_FRC_FAICFDV		0x0400

/* Annotation bits in FD CTRL */
#define LDPAA_FD_CTRL_ASAL		0x00020000	/* ASAL = 128 */
#define LDPAA_FD_CTRL_PTA		0x00800000
#define LDPAA_FD_CTRL_PTV1		0x00400000

/* TODO: we may want to move this and other WRIOP related defines
 * to a separate header
 */
/* Frame annotation status */
struct ldpaa_fas {
	u8 reserved;
	u8 ppid;
	__le16 ifpid;
	__le32 status;
} __packed;

/* Debug frame, otherwise supposed to be discarded */
#define LDPAA_ETH_FAS_DISC		0x80000000
/* MACSEC frame */
#define LDPAA_ETH_FAS_MS		0x40000000
#define LDPAA_ETH_FAS_PTP		0x08000000
/* Ethernet multicast frame */
#define LDPAA_ETH_FAS_MC		0x04000000
/* Ethernet broadcast frame */
#define LDPAA_ETH_FAS_BC		0x02000000
#define LDPAA_ETH_FAS_KSE		0x00040000
#define LDPAA_ETH_FAS_EOFHE		0x00020000
#define LDPAA_ETH_FAS_MNLE		0x00010000
#define LDPAA_ETH_FAS_TIDE		0x00008000
#define LDPAA_ETH_FAS_PIEE		0x00004000
/* Frame length error */
#define LDPAA_ETH_FAS_FLE		0x00002000
/* Frame physical error; our favourite pastime */
#define LDPAA_ETH_FAS_FPE		0x00001000
#define LDPAA_ETH_FAS_PTE		0x00000080
#define LDPAA_ETH_FAS_ISP		0x00000040
#define LDPAA_ETH_FAS_PHE		0x00000020
#define LDPAA_ETH_FAS_BLE		0x00000010
/* L3 csum validation performed */
#define LDPAA_ETH_FAS_L3CV		0x00000008
/* L3 csum error */
#define LDPAA_ETH_FAS_L3CE		0x00000004
/* L4 csum validation performed */
#define LDPAA_ETH_FAS_L4CV		0x00000002
/* L4 csum error */
#define LDPAA_ETH_FAS_L4CE		0x00000001
/* These bits always signal errors */
#define LDPAA_ETH_RX_ERR_MASK		(LDPAA_ETH_FAS_DISC	| \
					 LDPAA_ETH_FAS_KSE	| \
					 LDPAA_ETH_FAS_EOFHE	| \
					 LDPAA_ETH_FAS_MNLE	| \
					 LDPAA_ETH_FAS_TIDE	| \
					 LDPAA_ETH_FAS_PIEE	| \
					 LDPAA_ETH_FAS_FLE	| \
					 LDPAA_ETH_FAS_FPE	| \
					 LDPAA_ETH_FAS_PTE	| \
					 LDPAA_ETH_FAS_ISP	| \
					 LDPAA_ETH_FAS_PHE	| \
					 LDPAA_ETH_FAS_BLE	| \
					 LDPAA_ETH_FAS_L3CE	| \
					 LDPAA_ETH_FAS_L4CE)
/* Unsupported features in the ingress */
#define LDPAA_ETH_RX_UNSUPP_MASK	LDPAA_ETH_FAS_MS
/* TODO trim down the bitmask; not all of them apply to Tx-confirm */
#define LDPAA_ETH_TXCONF_ERR_MASK	(LDPAA_ETH_FAS_KSE	| \
					 LDPAA_ETH_FAS_EOFHE	| \
					 LDPAA_ETH_FAS_MNLE	| \
					 LDPAA_ETH_FAS_TIDE)

struct ldpaa_eth_priv {
	struct eth_device *net_dev;
	int dpni_id;
	uint16_t dpni_handle;
	struct dpni_attr dpni_attrs;
	/* Insofar as the MC is concerned, we're using one layout on all 3 types
	 * of buffers (Rx, Tx, Tx-Conf).
	 */
	struct dpni_buffer_layout buf_layout;
	uint16_t tx_data_offset;

	uint32_t rx_dflt_fqid;
	uint16_t tx_qdid;
	uint16_t tx_flow_id;

	enum ldpaa_eth_type type;	/* 1G or 10G ethernet */
	struct phy_device *phydev;
};

extern struct fsl_mc_io *dflt_mc_io;
extern struct fsl_dpbp_obj *dflt_dpbp;
extern struct fsl_dpio_obj *dflt_dpio;

static void ldpaa_dpbp_drain_cnt(int count);
static void ldpaa_dpbp_drain(void);
static int ldpaa_dpbp_seed(uint16_t bpid);
static void ldpaa_dpbp_free(void);
static int ldpaa_dpni_setup(struct ldpaa_eth_priv *priv);
static int ldpaa_dpbp_setup(void);
static int ldpaa_dpni_bind(struct ldpaa_eth_priv *priv);
#endif	/* __LDPAA_H */
