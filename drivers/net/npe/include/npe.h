/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef NPE_H
#define NPE_H

/*
 * defines...
 */
#define CONFIG_SYS_NPE_NUMS		1
#ifdef CONFIG_HAS_ETH1
#undef CONFIG_SYS_NPE_NUMS
#define CONFIG_SYS_NPE_NUMS		2
#endif

#define NPE_NUM_PORTS		3
#define ACTIVE_PORTS		1

#define NPE_PKT_SIZE		1600

#define CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS	64
#define CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS	2

#define NPE_MBUF_POOL_SIZE					\
	((CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS +	\
	  CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) *	\
	 sizeof(IX_OSAL_MBUF) * ACTIVE_PORTS)

#define NPE_PKT_POOL_SIZE					\
	((CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS +	\
	  CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) *	\
	 NPE_PKT_SIZE * ACTIVE_PORTS)

#define NPE_MEM_POOL_SIZE (NPE_MBUF_POOL_SIZE + NPE_PKT_POOL_SIZE)

#define PHY_AUTONEGOTIATE_TIMEOUT 4000	/* 4000 ms autonegotiate timeout */

/*
 * structs...
 */
struct npe {
	u8			active;           /* NPE active				*/
	u8			eth_id;           /* IX_ETH_PORT_1 or IX_ETH_PORT_2	*/
	u8			phy_no;           /* which PHY (0 - 31)			*/
	u8			mac_address[6];

	IX_OSAL_MBUF		*rxQHead;
	IX_OSAL_MBUF		*txQHead;

	u8			*tx_pkts;
	u8			*rx_pkts;
	IX_OSAL_MBUF		*rx_mbufs;
	IX_OSAL_MBUF		*tx_mbufs;

	int			print_speed;

	int			rx_read;
	int			rx_write;
	int			rx_len[PKTBUFSRX];
};

/*
 * prototypes...
 */
extern int npe_miiphy_read (const char *devname, unsigned char addr,
			    unsigned char reg, unsigned short *value);
extern int npe_miiphy_write (const char *devname, unsigned char addr,
			     unsigned char reg, unsigned short value);

#endif /* ifndef NPE_H */
