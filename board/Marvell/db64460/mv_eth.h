/*
 * (C) Copyright 2003
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 * based on - Driver for MV64460X ethernet ports
 * Copyright (C) 2002 rabeeh@galileo.co.il
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mv_eth.h - header file for the polled mode GT ethernet driver
 */

#ifndef __DB64460_ETH_H__
#define __DB64460_ETH_H__

#include <asm/types.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <common.h>
#include <net.h>
#include "mv_regs.h"
#include <asm/errno.h>


/*************************************************************************
**************************************************************************
**************************************************************************
*  The first part is the high level driver of the gigE ethernet ports.	 *
**************************************************************************
**************************************************************************
*************************************************************************/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* In case not using SG on Tx, define MAX_SKB_FRAGS as 0 */
#ifndef MAX_SKB_FRAGS
#define MAX_SKB_FRAGS 0
#endif

/* Port attributes */
/*#define MAX_RX_QUEUE_NUM	8*/
/*#define MAX_TX_QUEUE_NUM	8*/
#define MAX_RX_QUEUE_NUM	1
#define MAX_TX_QUEUE_NUM	1


/* Use one TX queue and one RX queue */
#define MV64460_TX_QUEUE_NUM 1
#define MV64460_RX_QUEUE_NUM 1

/*
 * Number of RX / TX descriptors on RX / TX rings.
 * Note that allocating RX descriptors is done by allocating the RX
 * ring AND a preallocated RX buffers (skb's) for each descriptor.
 * The TX descriptors only allocates the TX descriptors ring,
 * with no pre allocated TX buffers (skb's are allocated by higher layers.
 */

/* Default TX ring size is 10 descriptors */
#ifdef CONFIG_MV64460_ETH_TXQUEUE_SIZE
#define MV64460_TX_QUEUE_SIZE CONFIG_MV64460_ETH_TXQUEUE_SIZE
#else
#define MV64460_TX_QUEUE_SIZE 4
#endif

/* Default RX ring size is 4 descriptors */
#ifdef	CONFIG_MV64460_ETH_RXQUEUE_SIZE
#define MV64460_RX_QUEUE_SIZE CONFIG_MV64460_ETH_RXQUEUE_SIZE
#else
#define MV64460_RX_QUEUE_SIZE 4
#endif

#ifdef CONFIG_RX_BUFFER_SIZE
#define MV64460_RX_BUFFER_SIZE CONFIG_RX_BUFFER_SIZE
#else
#define MV64460_RX_BUFFER_SIZE 1600
#endif

#ifdef CONFIG_TX_BUFFER_SIZE
#define MV64460_TX_BUFFER_SIZE CONFIG_TX_BUFFER_SIZE
#else
#define MV64460_TX_BUFFER_SIZE 1600
#endif

/*
 *	Network device statistics. Akin to the 2.0 ether stats but
 *	with byte counters.
 */

struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received		*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;

	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};


/* Private data structure used for ethernet device */
struct mv64460_eth_priv {
    unsigned int port_num;
    struct net_device_stats *stats;

/* to buffer area aligned */
    char * p_eth_tx_buffer[MV64460_TX_QUEUE_SIZE+1];	/*pointers to alligned tx buffs in memory space */
    char * p_eth_rx_buffer[MV64460_RX_QUEUE_SIZE+1];	/*pointers to allinged rx buffs in memory space */

    /* Size of Tx Ring per queue */
    unsigned int tx_ring_size [MAX_TX_QUEUE_NUM];


    /* Size of Rx Ring per queue */
    unsigned int rx_ring_size [MAX_RX_QUEUE_NUM];

    /* Magic Number for Ethernet running */
    unsigned int eth_running;

};

int mv64460_eth_init (struct eth_device *dev);
int mv64460_eth_stop (struct eth_device *dev);
int mv64460_eth_start_xmit (struct eth_device*, volatile void* packet, int length);
/*	return db64460_eth0_poll(); */

int mv64460_eth_open (struct eth_device *dev);


/*************************************************************************
**************************************************************************
**************************************************************************
*  The second part is the low level driver of the gigE ethernet ports.	 *
**************************************************************************
**************************************************************************
*************************************************************************/


/********************************************************************************
 * Header File for : MV-643xx network interface header
 *
 * DESCRIPTION:
 *	 This header file contains macros typedefs and function declaration for
 *	 the Marvell Gig Bit Ethernet Controller.
 *
 * DEPENDENCIES:
 *	 None.
 *
 *******************************************************************************/


#ifdef CONFIG_SPECIAL_CONSISTENT_MEMORY
#ifdef CONFIG_MV64460_SRAM_CACHEABLE
/* In case SRAM is cacheable but not cache coherent */
#define D_CACHE_FLUSH_LINE(addr, offset)       \
{		    \
  __asm__ __volatile__ ("dcbf %0,%1" : : "r" (addr), "r" (offset)); \
}
#else
/* In case SRAM is cache coherent or non-cacheable */
#define D_CACHE_FLUSH_LINE(addr, offset) ;
#endif
#else
#ifdef CONFIG_NOT_COHERENT_CACHE
/* In case of descriptors on DDR but not cache coherent */
#define D_CACHE_FLUSH_LINE(addr, offset)       \
{		    \
  __asm__ __volatile__ ("dcbf %0,%1" : : "r" (addr), "r" (offset)); \
}
#else
/* In case of descriptors on DDR and cache coherent */
#define D_CACHE_FLUSH_LINE(addr, offset) ;
#endif /* CONFIG_NOT_COHERENT_CACHE */
#endif /* CONFIG_SPECIAL_CONSISTENT_MEMORY */


#define CPU_PIPE_FLUSH		   \
{		  \
  __asm__ __volatile__ ("eieio");	  \
}


/* defines  */

/* Default port configuration value */
#define PORT_CONFIG_VALUE			\
	     ETH_UNICAST_NORMAL_MODE		|   \
	     ETH_DEFAULT_RX_QUEUE_0		|   \
	     ETH_DEFAULT_RX_ARP_QUEUE_0		|   \
	     ETH_RECEIVE_BC_IF_NOT_IP_OR_ARP	|   \
	     ETH_RECEIVE_BC_IF_IP		|   \
	     ETH_RECEIVE_BC_IF_ARP		|   \
	     ETH_CAPTURE_TCP_FRAMES_DIS		|   \
	     ETH_CAPTURE_UDP_FRAMES_DIS		|   \
	     ETH_DEFAULT_RX_TCP_QUEUE_0		|   \
	     ETH_DEFAULT_RX_UDP_QUEUE_0		|   \
	     ETH_DEFAULT_RX_BPDU_QUEUE_0

/* Default port extend configuration value */
#define PORT_CONFIG_EXTEND_VALUE		\
	     ETH_SPAN_BPDU_PACKETS_AS_NORMAL	|   \
	     ETH_PARTITION_DISABLE


/* Default sdma control value */
#ifdef CONFIG_NOT_COHERENT_CACHE
#define PORT_SDMA_CONFIG_VALUE				\
			 ETH_RX_BURST_SIZE_16_64BIT	|	\
			 GT_ETH_IPG_INT_RX(0)			|	\
			 ETH_TX_BURST_SIZE_16_64BIT;
#else
#define PORT_SDMA_CONFIG_VALUE			\
			 ETH_RX_BURST_SIZE_4_64BIT	|	\
			 GT_ETH_IPG_INT_RX(0)			|	\
			 ETH_TX_BURST_SIZE_4_64BIT;
#endif

#define GT_ETH_IPG_INT_RX(value)		\
	    ((value & 0x3fff) << 8)

/* Default port serial control value */
#define PORT_SERIAL_CONTROL_VALUE			    \
			ETH_FORCE_LINK_PASS			|	\
			ETH_ENABLE_AUTO_NEG_FOR_DUPLX		|	\
			ETH_DISABLE_AUTO_NEG_FOR_FLOW_CTRL	|	\
			ETH_ADV_SYMMETRIC_FLOW_CTRL		|	\
			ETH_FORCE_FC_MODE_NO_PAUSE_DIS_TX	|	\
			ETH_FORCE_BP_MODE_NO_JAM		|	\
			BIT9					|	\
			ETH_DO_NOT_FORCE_LINK_FAIL		|	\
			ETH_RETRANSMIT_16_ETTEMPTS		|	\
			ETH_ENABLE_AUTO_NEG_SPEED_GMII		|	\
			ETH_DTE_ADV_0				|	\
			ETH_DISABLE_AUTO_NEG_BYPASS		|	\
			ETH_AUTO_NEG_NO_CHANGE			|	\
			ETH_MAX_RX_PACKET_1552BYTE		|	\
			ETH_CLR_EXT_LOOPBACK			|	\
			ETH_SET_FULL_DUPLEX_MODE		|	\
			ETH_ENABLE_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX;

#define RX_BUFFER_MAX_SIZE  0xFFFF
#define TX_BUFFER_MAX_SIZE  0xFFFF   /* Buffer are limited to 64k */

#define RX_BUFFER_MIN_SIZE  0x8
#define TX_BUFFER_MIN_SIZE  0x8

/* Tx WRR confoguration macros */
#define PORT_MAX_TRAN_UNIT	    0x24    /* MTU register (default) 9KByte */
#define PORT_MAX_TOKEN_BUCKET_SIZE  0x_fFFF  /* PMTBS register (default)      */
#define PORT_TOKEN_RATE		    1023    /* PTTBRC register (default)     */

/* MAC accepet/reject macros */
#define ACCEPT_MAC_ADDR	    0
#define REJECT_MAC_ADDR	    1

/* Size of a Tx/Rx descriptor used in chain list data structure */
#define RX_DESC_ALIGNED_SIZE		0x20
#define TX_DESC_ALIGNED_SIZE		0x20

/* An offest in Tx descriptors to store data for buffers less than 8 Bytes */
#define TX_BUF_OFFSET_IN_DESC	    0x18
/* Buffer offset from buffer pointer */
#define RX_BUF_OFFSET				0x2

/* Gap define */
#define ETH_BAR_GAP					0x8
#define ETH_SIZE_REG_GAP				0x8
#define ETH_HIGH_ADDR_REMAP_REG_GAP			0x4
#define ETH_PORT_ACCESS_CTRL_GAP			0x4

/* Gigabit Ethernet Unit Global Registers */

/* MIB Counters register definitions */
#define ETH_MIB_GOOD_OCTETS_RECEIVED_LOW   0x0
#define ETH_MIB_GOOD_OCTETS_RECEIVED_HIGH  0x4
#define ETH_MIB_BAD_OCTETS_RECEIVED	   0x8
#define ETH_MIB_INTERNAL_MAC_TRANSMIT_ERR  0xc
#define ETH_MIB_GOOD_FRAMES_RECEIVED	   0x10
#define ETH_MIB_BAD_FRAMES_RECEIVED	   0x14
#define ETH_MIB_BROADCAST_FRAMES_RECEIVED  0x18
#define ETH_MIB_MULTICAST_FRAMES_RECEIVED  0x1c
#define ETH_MIB_FRAMES_64_OCTETS	   0x20
#define ETH_MIB_FRAMES_65_TO_127_OCTETS	   0x24
#define ETH_MIB_FRAMES_128_TO_255_OCTETS   0x28
#define ETH_MIB_FRAMES_256_TO_511_OCTETS   0x2c
#define ETH_MIB_FRAMES_512_TO_1023_OCTETS  0x30
#define ETH_MIB_FRAMES_1024_TO_MAX_OCTETS  0x34
#define ETH_MIB_GOOD_OCTETS_SENT_LOW	   0x38
#define ETH_MIB_GOOD_OCTETS_SENT_HIGH	   0x3c
#define ETH_MIB_GOOD_FRAMES_SENT	   0x40
#define ETH_MIB_EXCESSIVE_COLLISION	   0x44
#define ETH_MIB_MULTICAST_FRAMES_SENT	   0x48
#define ETH_MIB_BROADCAST_FRAMES_SENT	   0x4c
#define ETH_MIB_UNREC_MAC_CONTROL_RECEIVED 0x50
#define ETH_MIB_FC_SENT			   0x54
#define ETH_MIB_GOOD_FC_RECEIVED	   0x58
#define ETH_MIB_BAD_FC_RECEIVED		   0x5c
#define ETH_MIB_UNDERSIZE_RECEIVED	   0x60
#define ETH_MIB_FRAGMENTS_RECEIVED	   0x64
#define ETH_MIB_OVERSIZE_RECEIVED	   0x68
#define ETH_MIB_JABBER_RECEIVED		   0x6c
#define ETH_MIB_MAC_RECEIVE_ERROR	   0x70
#define ETH_MIB_BAD_CRC_EVENT		   0x74
#define ETH_MIB_COLLISION		   0x78
#define ETH_MIB_LATE_COLLISION		   0x7c

/* Port serial status reg (PSR) */
#define ETH_INTERFACE_GMII_MII				0
#define ETH_INTERFACE_PCM				BIT0
#define ETH_LINK_IS_DOWN				0
#define ETH_LINK_IS_UP					BIT1
#define ETH_PORT_AT_HALF_DUPLEX				0
#define ETH_PORT_AT_FULL_DUPLEX				BIT2
#define ETH_RX_FLOW_CTRL_DISABLED			0
#define ETH_RX_FLOW_CTRL_ENBALED			BIT3
#define ETH_GMII_SPEED_100_10				0
#define ETH_GMII_SPEED_1000				BIT4
#define ETH_MII_SPEED_10				0
#define ETH_MII_SPEED_100				BIT5
#define ETH_NO_TX					0
#define ETH_TX_IN_PROGRESS				BIT7
#define ETH_BYPASS_NO_ACTIVE				0
#define ETH_BYPASS_ACTIVE				BIT8
#define ETH_PORT_NOT_AT_PARTITION_STATE			0
#define ETH_PORT_AT_PARTITION_STATE			BIT9
#define ETH_PORT_TX_FIFO_NOT_EMPTY			0
#define ETH_PORT_TX_FIFO_EMPTY				BIT10


/* These macros describes the Port configuration reg (Px_cR) bits */
#define ETH_UNICAST_NORMAL_MODE				0
#define ETH_UNICAST_PROMISCUOUS_MODE			BIT0
#define ETH_DEFAULT_RX_QUEUE_0				0
#define ETH_DEFAULT_RX_QUEUE_1				BIT1
#define ETH_DEFAULT_RX_QUEUE_2				BIT2
#define ETH_DEFAULT_RX_QUEUE_3				(BIT2 | BIT1)
#define ETH_DEFAULT_RX_QUEUE_4				BIT3
#define ETH_DEFAULT_RX_QUEUE_5				(BIT3 | BIT1)
#define ETH_DEFAULT_RX_QUEUE_6				(BIT3 | BIT2)
#define ETH_DEFAULT_RX_QUEUE_7				(BIT3 | BIT2 | BIT1)
#define ETH_DEFAULT_RX_ARP_QUEUE_0			0
#define ETH_DEFAULT_RX_ARP_QUEUE_1			BIT4
#define ETH_DEFAULT_RX_ARP_QUEUE_2			BIT5
#define ETH_DEFAULT_RX_ARP_QUEUE_3			(BIT5 | BIT4)
#define ETH_DEFAULT_RX_ARP_QUEUE_4			BIT6
#define ETH_DEFAULT_RX_ARP_QUEUE_5			(BIT6 | BIT4)
#define ETH_DEFAULT_RX_ARP_QUEUE_6			(BIT6 | BIT5)
#define ETH_DEFAULT_RX_ARP_QUEUE_7			(BIT6 | BIT5 | BIT4)
#define ETH_RECEIVE_BC_IF_NOT_IP_OR_ARP			0
#define ETH_REJECT_BC_IF_NOT_IP_OR_ARP			BIT7
#define ETH_RECEIVE_BC_IF_IP				0
#define ETH_REJECT_BC_IF_IP				BIT8
#define ETH_RECEIVE_BC_IF_ARP				0
#define ETH_REJECT_BC_IF_ARP				BIT9
#define ETH_TX_AM_NO_UPDATE_ERROR_SUMMARY		BIT12
#define ETH_CAPTURE_TCP_FRAMES_DIS			0
#define ETH_CAPTURE_TCP_FRAMES_EN			BIT14
#define ETH_CAPTURE_UDP_FRAMES_DIS			0
#define ETH_CAPTURE_UDP_FRAMES_EN			BIT15
#define ETH_DEFAULT_RX_TCP_QUEUE_0			0
#define ETH_DEFAULT_RX_TCP_QUEUE_1			BIT16
#define ETH_DEFAULT_RX_TCP_QUEUE_2			BIT17
#define ETH_DEFAULT_RX_TCP_QUEUE_3			(BIT17 | BIT16)
#define ETH_DEFAULT_RX_TCP_QUEUE_4			BIT18
#define ETH_DEFAULT_RX_TCP_QUEUE_5			(BIT18 | BIT16)
#define ETH_DEFAULT_RX_TCP_QUEUE_6			(BIT18 | BIT17)
#define ETH_DEFAULT_RX_TCP_QUEUE_7			(BIT18 | BIT17 | BIT16)
#define ETH_DEFAULT_RX_UDP_QUEUE_0			0
#define ETH_DEFAULT_RX_UDP_QUEUE_1			BIT19
#define ETH_DEFAULT_RX_UDP_QUEUE_2			BIT20
#define ETH_DEFAULT_RX_UDP_QUEUE_3			(BIT20 | BIT19)
#define ETH_DEFAULT_RX_UDP_QUEUE_4			(BIT21
#define ETH_DEFAULT_RX_UDP_QUEUE_5			(BIT21 | BIT19)
#define ETH_DEFAULT_RX_UDP_QUEUE_6			(BIT21 | BIT20)
#define ETH_DEFAULT_RX_UDP_QUEUE_7			(BIT21 | BIT20 | BIT19)
#define ETH_DEFAULT_RX_BPDU_QUEUE_0			 0
#define ETH_DEFAULT_RX_BPDU_QUEUE_1			BIT22
#define ETH_DEFAULT_RX_BPDU_QUEUE_2			BIT23
#define ETH_DEFAULT_RX_BPDU_QUEUE_3			(BIT23 | BIT22)
#define ETH_DEFAULT_RX_BPDU_QUEUE_4			BIT24
#define ETH_DEFAULT_RX_BPDU_QUEUE_5			(BIT24 | BIT22)
#define ETH_DEFAULT_RX_BPDU_QUEUE_6			(BIT24 | BIT23)
#define ETH_DEFAULT_RX_BPDU_QUEUE_7			(BIT24 | BIT23 | BIT22)


/* These macros describes the Port configuration extend reg (Px_cXR) bits*/
#define ETH_CLASSIFY_EN					BIT0
#define ETH_SPAN_BPDU_PACKETS_AS_NORMAL			0
#define ETH_SPAN_BPDU_PACKETS_TO_RX_QUEUE_7		BIT1
#define ETH_PARTITION_DISABLE				0
#define ETH_PARTITION_ENABLE				BIT2


/* Tx/Rx queue command reg (RQCR/TQCR)*/
#define ETH_QUEUE_0_ENABLE				BIT0
#define ETH_QUEUE_1_ENABLE				BIT1
#define ETH_QUEUE_2_ENABLE				BIT2
#define ETH_QUEUE_3_ENABLE				BIT3
#define ETH_QUEUE_4_ENABLE				BIT4
#define ETH_QUEUE_5_ENABLE				BIT5
#define ETH_QUEUE_6_ENABLE				BIT6
#define ETH_QUEUE_7_ENABLE				BIT7
#define ETH_QUEUE_0_DISABLE				BIT8
#define ETH_QUEUE_1_DISABLE				BIT9
#define ETH_QUEUE_2_DISABLE				BIT10
#define ETH_QUEUE_3_DISABLE				BIT11
#define ETH_QUEUE_4_DISABLE				BIT12
#define ETH_QUEUE_5_DISABLE				BIT13
#define ETH_QUEUE_6_DISABLE				BIT14
#define ETH_QUEUE_7_DISABLE				BIT15

/* These macros describes the Port Sdma configuration reg (SDCR) bits */
#define ETH_RIFB					BIT0
#define ETH_RX_BURST_SIZE_1_64BIT			0
#define ETH_RX_BURST_SIZE_2_64BIT			BIT1
#define ETH_RX_BURST_SIZE_4_64BIT			BIT2
#define ETH_RX_BURST_SIZE_8_64BIT			(BIT2 | BIT1)
#define ETH_RX_BURST_SIZE_16_64BIT			BIT3
#define ETH_BLM_RX_NO_SWAP				BIT4
#define ETH_BLM_RX_BYTE_SWAP				0
#define ETH_BLM_TX_NO_SWAP				BIT5
#define ETH_BLM_TX_BYTE_SWAP				0
#define ETH_DESCRIPTORS_BYTE_SWAP			BIT6
#define ETH_DESCRIPTORS_NO_SWAP				0
#define ETH_TX_BURST_SIZE_1_64BIT			0
#define ETH_TX_BURST_SIZE_2_64BIT			BIT22
#define ETH_TX_BURST_SIZE_4_64BIT			BIT23
#define ETH_TX_BURST_SIZE_8_64BIT			(BIT23 | BIT22)
#define ETH_TX_BURST_SIZE_16_64BIT			BIT24

/* These macros describes the Port serial control reg (PSCR) bits */
#define ETH_SERIAL_PORT_DISABLE				0
#define ETH_SERIAL_PORT_ENABLE				BIT0
#define ETH_FORCE_LINK_PASS				BIT1
#define ETH_DO_NOT_FORCE_LINK_PASS			0
#define ETH_ENABLE_AUTO_NEG_FOR_DUPLX			0
#define ETH_DISABLE_AUTO_NEG_FOR_DUPLX			BIT2
#define ETH_ENABLE_AUTO_NEG_FOR_FLOW_CTRL		0
#define ETH_DISABLE_AUTO_NEG_FOR_FLOW_CTRL		BIT3
#define ETH_ADV_NO_FLOW_CTRL				0
#define ETH_ADV_SYMMETRIC_FLOW_CTRL			BIT4
#define ETH_FORCE_FC_MODE_NO_PAUSE_DIS_TX		0
#define ETH_FORCE_FC_MODE_TX_PAUSE_DIS			BIT5
#define ETH_FORCE_BP_MODE_NO_JAM			0
#define ETH_FORCE_BP_MODE_JAM_TX			BIT7
#define ETH_FORCE_BP_MODE_JAM_TX_ON_RX_ERR		BIT8
#define ETH_FORCE_LINK_FAIL				0
#define ETH_DO_NOT_FORCE_LINK_FAIL			BIT10
#define ETH_RETRANSMIT_16_ETTEMPTS			0
#define ETH_RETRANSMIT_FOREVER				BIT11
#define ETH_DISABLE_AUTO_NEG_SPEED_GMII			BIT13
#define ETH_ENABLE_AUTO_NEG_SPEED_GMII			0
#define ETH_DTE_ADV_0					0
#define ETH_DTE_ADV_1					BIT14
#define ETH_DISABLE_AUTO_NEG_BYPASS			0
#define ETH_ENABLE_AUTO_NEG_BYPASS			BIT15
#define ETH_AUTO_NEG_NO_CHANGE				0
#define ETH_RESTART_AUTO_NEG				BIT16
#define ETH_MAX_RX_PACKET_1518BYTE			0
#define ETH_MAX_RX_PACKET_1522BYTE			BIT17
#define ETH_MAX_RX_PACKET_1552BYTE			BIT18
#define ETH_MAX_RX_PACKET_9022BYTE			(BIT18 | BIT17)
#define ETH_MAX_RX_PACKET_9192BYTE			BIT19
#define ETH_MAX_RX_PACKET_9700BYTE			(BIT19 | BIT17)
#define ETH_SET_EXT_LOOPBACK				BIT20
#define ETH_CLR_EXT_LOOPBACK				0
#define ETH_SET_FULL_DUPLEX_MODE			BIT21
#define ETH_SET_HALF_DUPLEX_MODE			0
#define ETH_ENABLE_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	BIT22
#define ETH_DISABLE_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	0
#define ETH_SET_GMII_SPEED_TO_10_100			0
#define ETH_SET_GMII_SPEED_TO_1000			BIT23
#define ETH_SET_MII_SPEED_TO_10				0
#define ETH_SET_MII_SPEED_TO_100			BIT24


/* SMI reg */
#define ETH_SMI_BUSY		BIT28	/* 0 - Write, 1 - Read		*/
#define ETH_SMI_READ_VALID	BIT27	/* 0 - Write, 1 - Read		*/
#define ETH_SMI_OPCODE_WRITE	0	/* Completion of Read operation */
#define ETH_SMI_OPCODE_READ	BIT26	/* Operation is in progress		*/

/* SDMA command status fields macros */

/* Tx & Rx descriptors status */
#define ETH_ERROR_SUMMARY		    (BIT0)

/* Tx & Rx descriptors command */
#define ETH_BUFFER_OWNED_BY_DMA		    (BIT31)

/* Tx descriptors status */
#define ETH_LC_ERROR			    (0	  )
#define ETH_UR_ERROR			    (BIT1 )
#define ETH_RL_ERROR			    (BIT2 )
#define ETH_LLC_SNAP_FORMAT		    (BIT9 )

/* Rx descriptors status */
#define ETH_CRC_ERROR			    (0	  )
#define ETH_OVERRUN_ERROR		    (BIT1 )
#define ETH_MAX_FRAME_LENGTH_ERROR	    (BIT2 )
#define ETH_RESOURCE_ERROR		    ((BIT2 | BIT1))
#define ETH_VLAN_TAGGED			    (BIT19)
#define ETH_BPDU_FRAME			    (BIT20)
#define ETH_TCP_FRAME_OVER_IP_V_4	    (0	  )
#define ETH_UDP_FRAME_OVER_IP_V_4	    (BIT21)
#define ETH_OTHER_FRAME_TYPE		    (BIT22)
#define ETH_LAYER_2_IS_ETH_V_2		    (BIT23)
#define ETH_FRAME_TYPE_IP_V_4		    (BIT24)
#define ETH_FRAME_HEADER_OK		    (BIT25)
#define ETH_RX_LAST_DESC		    (BIT26)
#define ETH_RX_FIRST_DESC		    (BIT27)
#define ETH_UNKNOWN_DESTINATION_ADDR	    (BIT28)
#define ETH_RX_ENABLE_INTERRUPT		    (BIT29)
#define ETH_LAYER_4_CHECKSUM_OK		    (BIT30)

/* Rx descriptors byte count */
#define ETH_FRAME_FRAGMENTED		    (BIT2)

/* Tx descriptors command */
#define ETH_LAYER_4_CHECKSUM_FIRST_DESC		(BIT10)
#define ETH_FRAME_SET_TO_VLAN		    (BIT15)
#define ETH_TCP_FRAME			    (0	  )
#define ETH_UDP_FRAME			    (BIT16)
#define ETH_GEN_TCP_UDP_CHECKSUM	    (BIT17)
#define ETH_GEN_IP_V_4_CHECKSUM		    (BIT18)
#define ETH_ZERO_PADDING		    (BIT19)
#define ETH_TX_LAST_DESC		    (BIT20)
#define ETH_TX_FIRST_DESC		    (BIT21)
#define ETH_GEN_CRC			    (BIT22)
#define ETH_TX_ENABLE_INTERRUPT		    (BIT23)
#define ETH_AUTO_MODE			    (BIT30)

/* Address decode parameters */
/* Ethernet Base Address Register bits */
#define EBAR_TARGET_DRAM					0x00000000
#define EBAR_TARGET_DEVICE					0x00000001
#define EBAR_TARGET_CBS						0x00000002
#define EBAR_TARGET_PCI0					0x00000003
#define EBAR_TARGET_PCI1					0x00000004
#define EBAR_TARGET_CUNIT					0x00000005
#define EBAR_TARGET_AUNIT					0x00000006
#define EBAR_TARGET_GUNIT					0x00000007

/* Window attributes */
#define EBAR_ATTR_DRAM_CS0					0x00000E00
#define EBAR_ATTR_DRAM_CS1					0x00000D00
#define EBAR_ATTR_DRAM_CS2					0x00000B00
#define EBAR_ATTR_DRAM_CS3					0x00000700

/* DRAM Target interface */
#define EBAR_ATTR_DRAM_NO_CACHE_COHERENCY	0x00000000
#define EBAR_ATTR_DRAM_CACHE_COHERENCY_WT	0x00001000
#define EBAR_ATTR_DRAM_CACHE_COHERENCY_WB	0x00002000

/* Device Bus Target interface */
#define EBAR_ATTR_DEVICE_DEVCS0				0x00001E00
#define EBAR_ATTR_DEVICE_DEVCS1				0x00001D00
#define EBAR_ATTR_DEVICE_DEVCS2				0x00001B00
#define EBAR_ATTR_DEVICE_DEVCS3				0x00001700
#define EBAR_ATTR_DEVICE_BOOTCS3			0x00000F00

/* PCI Target interface */
#define EBAR_ATTR_PCI_BYTE_SWAP				0x00000000
#define EBAR_ATTR_PCI_NO_SWAP				0x00000100
#define EBAR_ATTR_PCI_BYTE_WORD_SWAP		0x00000200
#define EBAR_ATTR_PCI_WORD_SWAP				0x00000300
#define EBAR_ATTR_PCI_NO_SNOOP_NOT_ASSERT	0x00000000
#define EBAR_ATTR_PCI_NO_SNOOP_ASSERT		0x00000400
#define EBAR_ATTR_PCI_IO_SPACE				0x00000000
#define EBAR_ATTR_PCI_MEMORY_SPACE			0x00000800
#define EBAR_ATTR_PCI_REQ64_FORCE			0x00000000
#define EBAR_ATTR_PCI_REQ64_SIZE			0x00001000

/* CPU 60x bus or internal SRAM interface */
#define EBAR_ATTR_CBS_SRAM_BLOCK0			0x00000000
#define EBAR_ATTR_CBS_SRAM_BLOCK1			0x00000100
#define EBAR_ATTR_CBS_SRAM					0x00000000
#define EBAR_ATTR_CBS_CPU_BUS				0x00000800

/* Window access control */
#define EWIN_ACCESS_NOT_ALLOWED 0
#define EWIN_ACCESS_READ_ONLY	BIT0
#define EWIN_ACCESS_FULL	(BIT1 | BIT0)
#define EWIN0_ACCESS_MASK		0x0003
#define EWIN1_ACCESS_MASK		0x000C
#define EWIN2_ACCESS_MASK		0x0030
#define EWIN3_ACCESS_MASK		0x00C0

/* typedefs */

typedef enum _eth_port
{
    ETH_0 = 0,
	ETH_1 = 1,
	ETH_2 = 2
}ETH_PORT;

typedef enum _eth_func_ret_status
{
    ETH_OK,			/* Returned as expected.		    */
    ETH_ERROR,			/* Fundamental error.			    */
    ETH_RETRY,			/* Could not process request. Try later.    */
    ETH_END_OF_JOB,		/* Ring has nothing to process.		    */
    ETH_QUEUE_FULL,		/* Ring resource error.			    */
    ETH_QUEUE_LAST_RESOURCE	/* Ring resources about to exhaust.	    */
}ETH_FUNC_RET_STATUS;

typedef enum _eth_queue
{
	ETH_Q0 = 0,
	ETH_Q1 = 1,
	ETH_Q2 = 2,
	ETH_Q3 = 3,
	ETH_Q4 = 4,
	ETH_Q5 = 5,
	ETH_Q6 = 6,
    ETH_Q7 = 7
} ETH_QUEUE;

typedef enum _addr_win
{
	ETH_WIN0,
	ETH_WIN1,
	ETH_WIN2,
	ETH_WIN3,
	ETH_WIN4,
    ETH_WIN5
} ETH_ADDR_WIN;

typedef enum _eth_target
{
	ETH_TARGET_DRAM	 ,
	ETH_TARGET_DEVICE,
	ETH_TARGET_CBS	 ,
	ETH_TARGET_PCI0	 ,
	ETH_TARGET_PCI1
}ETH_TARGET;

typedef struct _eth_rx_desc
{
	unsigned short	byte_cnt	   ;	/* Descriptor buffer byte count	    */
	unsigned short	buf_size	   ;	/* Buffer size			    */
	unsigned int	cmd_sts	   ;	/* Descriptor command status	    */
	unsigned int	next_desc_ptr;	  /* Next descriptor pointer	      */
	unsigned int	buf_ptr	   ;	/* Descriptor buffer pointer	    */
    unsigned int    return_info ;    /* User resource return information */
} ETH_RX_DESC;


typedef struct _eth_tx_desc
{
    unsigned short  byte_cnt	   ;	/* Descriptor buffer byte count	    */
    unsigned short  l4i_chk	   ;	/* CPU provided TCP Checksum	    */
    unsigned int    cmd_sts	   ;	/* Descriptor command status	    */
    unsigned int    next_desc_ptr;    /* Next descriptor pointer	  */
    unsigned int    buf_ptr	   ;	/* Descriptor buffer pointer	    */
    unsigned int    return_info ;    /* User resource return information */
} ETH_TX_DESC;

/* Unified struct for Rx and Tx operations. The user is not required to */
/* be familier with neither Tx nor Rx descriptors.			 */
typedef struct _pkt_info
{
	unsigned short	byte_cnt   ;	/* Descriptor buffer byte count	    */
	unsigned short	l4i_chk	   ;	/* Tx CPU provided TCP Checksum	    */
	unsigned int	cmd_sts	   ;	/* Descriptor command status	    */
	unsigned int	buf_ptr	   ;	/* Descriptor buffer pointer	    */
    unsigned int    return_info ;    /* User resource return information */
} PKT_INFO;


typedef struct _eth_win_param
{
    ETH_ADDR_WIN win;	/* Window number. See ETH_ADDR_WIN enum */
    ETH_TARGET	target;	   /* System targets. See ETH_TARGET enum */
    unsigned short attributes;	/* BAR attributes. See above macros. */
    unsigned int base_addr; /* Window base address in unsigned int form */
    unsigned int high_addr; /* Window high address in unsigned int form */
    unsigned int size; /* Size in MBytes. Must be % 64Kbyte. */
    bool enable; /* Enable/disable access to the window. */
    unsigned short access_ctrl; /* Access ctrl register. see above macros */
} ETH_WIN_PARAM;


/* Ethernet port specific infomation */

typedef struct _eth_port_ctrl
{
    ETH_PORT  port_num; /* User Ethernet port number */
    int port_phy_addr;	/* User phy address of Ethrnet port */
    unsigned char port_mac_addr[6]; /* User defined port MAC address. */
    unsigned int  port_config; /* User port configuration value */
    unsigned int  port_config_extend; /* User port config extend value */
    unsigned int  port_sdma_config; /* User port SDMA config value */
    unsigned int  port_serial_control; /* User port serial control value */
    unsigned int  port_tx_queue_command; /* Port active Tx queues summary */
    unsigned int  port_rx_queue_command; /* Port active Rx queues summary */

    /* User function to cast virtual address to CPU bus address */
    unsigned int  (*port_virt_to_phys)(unsigned int addr);
    /* User scratch pad for user specific data structures */
    void *port_private;

    bool rx_resource_err[MAX_RX_QUEUE_NUM]; /* Rx ring resource error flag */
    bool tx_resource_err[MAX_TX_QUEUE_NUM]; /* Tx ring resource error flag */

    /* Tx/Rx rings managment indexes fields. For driver use */

    /* Next available Rx resource */
    volatile ETH_RX_DESC *p_rx_curr_desc_q[MAX_RX_QUEUE_NUM];
    /* Returning Rx resource */
    volatile ETH_RX_DESC *p_rx_used_desc_q[MAX_RX_QUEUE_NUM];

    /* Next available Tx resource */
    volatile ETH_TX_DESC *p_tx_curr_desc_q[MAX_TX_QUEUE_NUM];
    /* Returning Tx resource */
    volatile ETH_TX_DESC *p_tx_used_desc_q[MAX_TX_QUEUE_NUM];
    /* An extra Tx index to support transmit of multiple buffers per packet */
    volatile ETH_TX_DESC *p_tx_first_desc_q[MAX_TX_QUEUE_NUM];

    /* Tx/Rx rings size and base variables fields. For driver use */

    volatile ETH_RX_DESC	*p_rx_desc_area_base[MAX_RX_QUEUE_NUM];
    unsigned int		 rx_desc_area_size[MAX_RX_QUEUE_NUM];
    char			*p_rx_buffer_base[MAX_RX_QUEUE_NUM];

    volatile ETH_TX_DESC	*p_tx_desc_area_base[MAX_TX_QUEUE_NUM];
    unsigned int		 tx_desc_area_size[MAX_TX_QUEUE_NUM];
    char			*p_tx_buffer_base[MAX_TX_QUEUE_NUM];

} ETH_PORT_INFO;


/* ethernet.h API list */

/* Port operation control routines */
static void eth_port_init (ETH_PORT_INFO *p_eth_port_ctrl);
static void eth_port_reset(ETH_PORT	eth_port_num);
static bool eth_port_start(ETH_PORT_INFO *p_eth_port_ctrl);


/* Port MAC address routines */
static void eth_port_uc_addr_set (ETH_PORT eth_port_num,
				  unsigned char *p_addr,
				  ETH_QUEUE queue);
#if 0	/* FIXME */
static void eth_port_mc_addr	(ETH_PORT eth_port_num,
				 unsigned char *p_addr,
				 ETH_QUEUE queue,
				 int option);
#endif

/* PHY and MIB routines */
static bool ethernet_phy_reset(ETH_PORT eth_port_num);

static bool eth_port_write_smi_reg(ETH_PORT eth_port_num,
				   unsigned int phy_reg,
				   unsigned int value);

static bool eth_port_read_smi_reg(ETH_PORT eth_port_num,
				  unsigned int phy_reg,
				  unsigned int* value);

static void eth_clear_mib_counters(ETH_PORT	eth_port_num);

/* Port data flow control routines */
static ETH_FUNC_RET_STATUS eth_port_send    (ETH_PORT_INFO *p_eth_port_ctrl,
					     ETH_QUEUE tx_queue,
					     PKT_INFO *p_pkt_info);
static ETH_FUNC_RET_STATUS eth_tx_return_desc(ETH_PORT_INFO *p_eth_port_ctrl,
					      ETH_QUEUE tx_queue,
					      PKT_INFO *p_pkt_info);
static ETH_FUNC_RET_STATUS eth_port_receive (ETH_PORT_INFO *p_eth_port_ctrl,
					     ETH_QUEUE rx_queue,
					     PKT_INFO *p_pkt_info);
static ETH_FUNC_RET_STATUS eth_rx_return_buff(ETH_PORT_INFO *p_eth_port_ctrl,
					      ETH_QUEUE rx_queue,
					      PKT_INFO *p_pkt_info);


static bool ether_init_tx_desc_ring(ETH_PORT_INFO  *p_eth_port_ctrl,
				    ETH_QUEUE	tx_queue,
				    int				tx_desc_num,
				    int				tx_buff_size,
				    unsigned int	tx_desc_base_addr,
				    unsigned int	tx_buff_base_addr);

static bool ether_init_rx_desc_ring(ETH_PORT_INFO  *p_eth_port_ctrl,
				    ETH_QUEUE	rx_queue,
				    int				rx_desc_num,
				    int				rx_buff_size,
				    unsigned int	rx_desc_base_addr,
				    unsigned int	rx_buff_base_addr);

#endif /* MV64460_ETH_ */
