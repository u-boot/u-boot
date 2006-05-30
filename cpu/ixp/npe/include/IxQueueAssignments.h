/**
 * @file IxQueueAssignments.h
 *
 * @author Intel Corporation
 * @date 29-Oct-2004
 *
 * @brief Central definition for queue assignments
 *
 * Design Notes:
 * This file contains queue assignments used by Ethernet (EthAcc),
 * HSS (HssAcc), ATM (atmdAcc) and DMA (dmaAcc) access libraries.
 *
 * Note: Ethernet QoS traffic class definitions are managed separately
 * by EthDB in IxEthDBQoS.h.
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxQueueAssignments_H
#define IxQueueAssignments_H

#include "IxQMgr.h"

/***************************************************************************
 *  Queue assignments for ATM
 ***************************************************************************/

/** 
 * @brief Global compiler switch to select between 3 possible NPE Modes
 * Define this macro to enable MPHY mode
 *
 * Default(No Switch) = MultiPHY Utopia2
 * IX_UTOPIAMODE = 1 for single Phy Utopia1
 * IX_MPHYSINGLEPORT = 1 for single Phy Utopia2 
 */
#define IX_NPE_MPHYMULTIPORT 1
#if IX_UTOPIAMODE == 1
#undef  IX_NPE_MPHYMULTIPORT
#endif
#if IX_MPHYSINGLEPORT == 1
#undef  IX_NPE_MPHYMULTIPORT
#endif

/**
 * @def IX_NPE_A_TXDONE_QUEUE_HIGHWATERMARK
 *
 * @brief The NPE reserves the High Watermark for its operation. But it must be set by the Xscale
 */
#define IX_NPE_A_TXDONE_QUEUE_HIGHWATERMARK  2

/**
 * @def IX_NPE_A_QMQ_ATM_TX_DONE
 *
 * @brief Queue ID for ATM Transmit Done queue
 */
#define IX_NPE_A_QMQ_ATM_TX_DONE       IX_QMGR_QUEUE_1

/**
 * @def IX_NPE_A_QMQ_ATM_TX0
 *
 * @brief Queue ID for ATM transmit Queue in a single phy configuration
 */
#define IX_NPE_A_QMQ_ATM_TX0           IX_QMGR_QUEUE_2


/**
 * @def IX_NPE_A_QMQ_ATM_TXID_MIN
 *
 * @brief Queue Manager Queue ID for ATM transmit Queue with minimum number of queue
 *
 */

/**
 * @def IX_NPE_A_QMQ_ATM_TXID_MAX
 *
 * @brief Queue Manager Queue ID for ATM transmit Queue with maximum number of queue
 *
 */

/**
 * @def IX_NPE_A_QMQ_ATM_RX_HI
 *
 * @brief Queue Manager Queue ID for ATM Receive high Queue
 *
 */

/**
 * @def IX_NPE_A_QMQ_ATM_RX_LO
 *
 * @brief Queue Manager Queue ID for ATM Receive low Queue
 */

#ifdef IX_NPE_MPHYMULTIPORT
/**
 * @def IX_NPE_A_QMQ_ATM_TX1
 *
 * @brief Queue ID for ATM transmit Queue Multiphy from 1 to 11
 */
#define IX_NPE_A_QMQ_ATM_TX1           IX_NPE_A_QMQ_ATM_TX0+1
#define IX_NPE_A_QMQ_ATM_TX2           IX_NPE_A_QMQ_ATM_TX1+1
#define IX_NPE_A_QMQ_ATM_TX3           IX_NPE_A_QMQ_ATM_TX2+1
#define IX_NPE_A_QMQ_ATM_TX4           IX_NPE_A_QMQ_ATM_TX3+1
#define IX_NPE_A_QMQ_ATM_TX5           IX_NPE_A_QMQ_ATM_TX4+1
#define IX_NPE_A_QMQ_ATM_TX6           IX_NPE_A_QMQ_ATM_TX5+1
#define IX_NPE_A_QMQ_ATM_TX7           IX_NPE_A_QMQ_ATM_TX6+1
#define IX_NPE_A_QMQ_ATM_TX8           IX_NPE_A_QMQ_ATM_TX7+1
#define IX_NPE_A_QMQ_ATM_TX9           IX_NPE_A_QMQ_ATM_TX8+1
#define IX_NPE_A_QMQ_ATM_TX10          IX_NPE_A_QMQ_ATM_TX9+1
#define IX_NPE_A_QMQ_ATM_TX11          IX_NPE_A_QMQ_ATM_TX10+1
#define IX_NPE_A_QMQ_ATM_TXID_MIN      IX_NPE_A_QMQ_ATM_TX0
#define IX_NPE_A_QMQ_ATM_TXID_MAX      IX_NPE_A_QMQ_ATM_TX11
#define IX_NPE_A_QMQ_ATM_RX_HI         IX_QMGR_QUEUE_21
#define IX_NPE_A_QMQ_ATM_RX_LO         IX_QMGR_QUEUE_22
#else
#define IX_NPE_A_QMQ_ATM_TXID_MIN      IX_NPE_A_QMQ_ATM_TX0
#define IX_NPE_A_QMQ_ATM_TXID_MAX      IX_NPE_A_QMQ_ATM_TX0
#define IX_NPE_A_QMQ_ATM_RX_HI         IX_QMGR_QUEUE_10
#define IX_NPE_A_QMQ_ATM_RX_LO         IX_QMGR_QUEUE_11
#endif /* MPHY */

/**
 * @def IX_NPE_A_QMQ_ATM_FREE_VC0
 *
 * @brief Hardware QMgr Queue ID for ATM Free VC Queue.
 *
 * There are 32 Hardware QMgr Queue ID; from IX_NPE_A_QMQ_ATM_FREE_VC1 to
 * IX_NPE_A_QMQ_ATM_FREE_VC30
 */
#define IX_NPE_A_QMQ_ATM_FREE_VC0      IX_QMGR_QUEUE_32
#define IX_NPE_A_QMQ_ATM_FREE_VC1      IX_NPE_A_QMQ_ATM_FREE_VC0+1
#define IX_NPE_A_QMQ_ATM_FREE_VC2      IX_NPE_A_QMQ_ATM_FREE_VC1+1
#define IX_NPE_A_QMQ_ATM_FREE_VC3      IX_NPE_A_QMQ_ATM_FREE_VC2+1
#define IX_NPE_A_QMQ_ATM_FREE_VC4      IX_NPE_A_QMQ_ATM_FREE_VC3+1
#define IX_NPE_A_QMQ_ATM_FREE_VC5      IX_NPE_A_QMQ_ATM_FREE_VC4+1
#define IX_NPE_A_QMQ_ATM_FREE_VC6      IX_NPE_A_QMQ_ATM_FREE_VC5+1
#define IX_NPE_A_QMQ_ATM_FREE_VC7      IX_NPE_A_QMQ_ATM_FREE_VC6+1
#define IX_NPE_A_QMQ_ATM_FREE_VC8      IX_NPE_A_QMQ_ATM_FREE_VC7+1
#define IX_NPE_A_QMQ_ATM_FREE_VC9      IX_NPE_A_QMQ_ATM_FREE_VC8+1
#define IX_NPE_A_QMQ_ATM_FREE_VC10     IX_NPE_A_QMQ_ATM_FREE_VC9+1
#define IX_NPE_A_QMQ_ATM_FREE_VC11     IX_NPE_A_QMQ_ATM_FREE_VC10+1
#define IX_NPE_A_QMQ_ATM_FREE_VC12     IX_NPE_A_QMQ_ATM_FREE_VC11+1
#define IX_NPE_A_QMQ_ATM_FREE_VC13     IX_NPE_A_QMQ_ATM_FREE_VC12+1
#define IX_NPE_A_QMQ_ATM_FREE_VC14     IX_NPE_A_QMQ_ATM_FREE_VC13+1
#define IX_NPE_A_QMQ_ATM_FREE_VC15     IX_NPE_A_QMQ_ATM_FREE_VC14+1
#define IX_NPE_A_QMQ_ATM_FREE_VC16     IX_NPE_A_QMQ_ATM_FREE_VC15+1
#define IX_NPE_A_QMQ_ATM_FREE_VC17     IX_NPE_A_QMQ_ATM_FREE_VC16+1
#define IX_NPE_A_QMQ_ATM_FREE_VC18     IX_NPE_A_QMQ_ATM_FREE_VC17+1
#define IX_NPE_A_QMQ_ATM_FREE_VC19     IX_NPE_A_QMQ_ATM_FREE_VC18+1
#define IX_NPE_A_QMQ_ATM_FREE_VC20     IX_NPE_A_QMQ_ATM_FREE_VC19+1
#define IX_NPE_A_QMQ_ATM_FREE_VC21     IX_NPE_A_QMQ_ATM_FREE_VC20+1
#define IX_NPE_A_QMQ_ATM_FREE_VC22     IX_NPE_A_QMQ_ATM_FREE_VC21+1
#define IX_NPE_A_QMQ_ATM_FREE_VC23     IX_NPE_A_QMQ_ATM_FREE_VC22+1
#define IX_NPE_A_QMQ_ATM_FREE_VC24     IX_NPE_A_QMQ_ATM_FREE_VC23+1
#define IX_NPE_A_QMQ_ATM_FREE_VC25     IX_NPE_A_QMQ_ATM_FREE_VC24+1
#define IX_NPE_A_QMQ_ATM_FREE_VC26     IX_NPE_A_QMQ_ATM_FREE_VC25+1
#define IX_NPE_A_QMQ_ATM_FREE_VC27     IX_NPE_A_QMQ_ATM_FREE_VC26+1
#define IX_NPE_A_QMQ_ATM_FREE_VC28     IX_NPE_A_QMQ_ATM_FREE_VC27+1
#define IX_NPE_A_QMQ_ATM_FREE_VC29     IX_NPE_A_QMQ_ATM_FREE_VC28+1
#define IX_NPE_A_QMQ_ATM_FREE_VC30     IX_NPE_A_QMQ_ATM_FREE_VC29+1
#define IX_NPE_A_QMQ_ATM_FREE_VC31     IX_NPE_A_QMQ_ATM_FREE_VC30+1

/**
 * @def IX_NPE_A_QMQ_ATM_RXFREE_MIN
 *
 * @brief The minimum queue ID for FreeVC queue
 */
#define IX_NPE_A_QMQ_ATM_RXFREE_MIN  IX_NPE_A_QMQ_ATM_FREE_VC0

/**
 * @def IX_NPE_A_QMQ_ATM_RXFREE_MAX
 *
 * @brief The maximum queue ID for FreeVC queue
 */
#define IX_NPE_A_QMQ_ATM_RXFREE_MAX  IX_NPE_A_QMQ_ATM_FREE_VC31

/**
 * @def IX_NPE_A_QMQ_OAM_FREE_VC
 * @brief OAM Rx Free queue ID
 */
#ifdef IX_NPE_MPHYMULTIPORT
#define IX_NPE_A_QMQ_OAM_FREE_VC       IX_QMGR_QUEUE_14
#else
#define IX_NPE_A_QMQ_OAM_FREE_VC       IX_QMGR_QUEUE_3
#endif /* MPHY */

/****************************************************************************
 * Queue assignments for HSS
 ****************************************************************************/

/****  HSS Port 0 ****/

/**
 * @def IX_NPE_A_QMQ_HSS0_CHL_RX_TRIG
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Channelized Receive trigger
 */
#define IX_NPE_A_QMQ_HSS0_CHL_RX_TRIG   IX_QMGR_QUEUE_12

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_RX
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Receive
 */
#define IX_NPE_A_QMQ_HSS0_PKT_RX        IX_QMGR_QUEUE_13

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_TX0
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Transmit queue 0
 */
#define IX_NPE_A_QMQ_HSS0_PKT_TX0       IX_QMGR_QUEUE_14

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_TX1
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Transmit queue 1
 */
#define IX_NPE_A_QMQ_HSS0_PKT_TX1       IX_QMGR_QUEUE_15

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_TX2
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Transmit queue 2
 */
#define IX_NPE_A_QMQ_HSS0_PKT_TX2       IX_QMGR_QUEUE_16

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_TX3
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Transmit queue 3
 */
#define IX_NPE_A_QMQ_HSS0_PKT_TX3       IX_QMGR_QUEUE_17

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_RX_FREE0
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Receive Free queue 0
 */
#define IX_NPE_A_QMQ_HSS0_PKT_RX_FREE0  IX_QMGR_QUEUE_18

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_RX_FREE1
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Receive Free queue 1
 */
#define IX_NPE_A_QMQ_HSS0_PKT_RX_FREE1  IX_QMGR_QUEUE_19

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_RX_FREE2
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Receive Free queue 2
 */
#define IX_NPE_A_QMQ_HSS0_PKT_RX_FREE2  IX_QMGR_QUEUE_20

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_RX_FREE3
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Receive Free queue 3
 */
#define IX_NPE_A_QMQ_HSS0_PKT_RX_FREE3  IX_QMGR_QUEUE_21

/**
 * @def IX_NPE_A_QMQ_HSS0_PKT_TX_DONE
 *
 * @brief Hardware QMgr Queue ID for HSS Port 0 Packetized Transmit Done queue
 */
#define IX_NPE_A_QMQ_HSS0_PKT_TX_DONE   IX_QMGR_QUEUE_22

/****  HSS Port 1 ****/

/**
 * @def IX_NPE_A_QMQ_HSS1_CHL_RX_TRIG
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Channelized Receive trigger
 */
#define IX_NPE_A_QMQ_HSS1_CHL_RX_TRIG   IX_QMGR_QUEUE_10

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_RX
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Receive
 */
#define IX_NPE_A_QMQ_HSS1_PKT_RX        IX_QMGR_QUEUE_0

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_TX0
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Transmit queue 0
 */
#define IX_NPE_A_QMQ_HSS1_PKT_TX0       IX_QMGR_QUEUE_5

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_TX1
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Transmit queue 1
 */
#define IX_NPE_A_QMQ_HSS1_PKT_TX1       IX_QMGR_QUEUE_6

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_TX2
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Transmit queue 2
 */
#define IX_NPE_A_QMQ_HSS1_PKT_TX2       IX_QMGR_QUEUE_7

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_TX3
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Transmit queue 3
 */
#define IX_NPE_A_QMQ_HSS1_PKT_TX3       IX_QMGR_QUEUE_8

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_RX_FREE0
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Receive Free queue 0
 */
#define IX_NPE_A_QMQ_HSS1_PKT_RX_FREE0  IX_QMGR_QUEUE_1

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_RX_FREE1
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Receive Free queue 1
 */
#define IX_NPE_A_QMQ_HSS1_PKT_RX_FREE1  IX_QMGR_QUEUE_2

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_RX_FREE2
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Receive Free queue 2
 */
#define IX_NPE_A_QMQ_HSS1_PKT_RX_FREE2  IX_QMGR_QUEUE_3

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_RX_FREE3
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Receive Free queue 3
 */
#define IX_NPE_A_QMQ_HSS1_PKT_RX_FREE3  IX_QMGR_QUEUE_4

/**
 * @def IX_NPE_A_QMQ_HSS1_PKT_TX_DONE
 *
 * @brief Hardware QMgr Queue ID for HSS Port 1 Packetized Transmit Done queue
 */
#define IX_NPE_A_QMQ_HSS1_PKT_TX_DONE   IX_QMGR_QUEUE_9

/*****************************************************************************************
 * Queue assignments for DMA
 *****************************************************************************************/

#define IX_DMA_NPE_A_REQUEST_QID IX_QMGR_QUEUE_19   /**< Queue Id for NPE A DMA Request */
#define IX_DMA_NPE_A_DONE_QID    IX_QMGR_QUEUE_20   /**< Queue Id for NPE A DMA Done    */
#define IX_DMA_NPE_B_REQUEST_QID IX_QMGR_QUEUE_24   /**< Queue Id for NPE B DMA Request */
#define IX_DMA_NPE_B_DONE_QID    IX_QMGR_QUEUE_26   /**< Queue Id for NPE B DMA Done    */
#define IX_DMA_NPE_C_REQUEST_QID IX_QMGR_QUEUE_25   /**< Queue Id for NPE C DMA Request */
#define IX_DMA_NPE_C_DONE_QID    IX_QMGR_QUEUE_27   /**< Queue Id for NPE C DMA Done    */


/*****************************************************************************************
 * Queue assignments for Ethernet
 *
 * Note: Rx queue definitions, which include QoS traffic class definitions
 * are managed by EthDB and declared in IxEthDBQoS.h
 *****************************************************************************************/

/**
*
* @def IX_ETH_ACC_RX_FRAME_ETH_Q 
*
* @brief  Eth0/Eth1 NPE Frame Recieve Q.
*
* @note THIS IS NOT USED - the Rx queues are read from EthDB QoS configuration
* 
*/
#define IX_ETH_ACC_RX_FRAME_ETH_Q 	(IX_QMGR_QUEUE_4)

/**
*
* @def IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q
*
* @brief Supply Rx Buffers Ethernet Q for NPEB - Eth 0 - Port 1
*
*/
#define IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q    (IX_QMGR_QUEUE_27)

/**
*
* @def IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q
*
* @brief Supply Rx Buffers Ethernet Q for NPEC - Eth 1 - Port 2
*
*/
#define IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q    (IX_QMGR_QUEUE_28)

/**
*
* @def IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q
*
* @brief Supply Rx Buffers Ethernet Q for NPEA - Eth 2 - Port 3
*
*/
#define IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q    (IX_QMGR_QUEUE_26)


/**
*
* @def IX_ETH_ACC_TX_FRAME_ENET0_Q
*
* @brief Submit frame Q for NPEB Eth 0 - Port 1
*
*/
#define IX_ETH_ACC_TX_FRAME_ENET0_Q    (IX_QMGR_QUEUE_24)


/**
*
* @def IX_ETH_ACC_TX_FRAME_ENET1_Q
*
* @brief Submit frame Q for NPEC Eth 1 - Port 2
*
*/
#define IX_ETH_ACC_TX_FRAME_ENET1_Q    (IX_QMGR_QUEUE_25)

/**
*
* @def IX_ETH_ACC_TX_FRAME_ENET2_Q
*
* @brief Submit frame Q for NPEA Eth 2 - Port 3
*
*/
#define IX_ETH_ACC_TX_FRAME_ENET2_Q    (IX_QMGR_QUEUE_23)

/**
*
* @def IX_ETH_ACC_TX_FRAME_DONE_ETH_Q
*
* @brief Transmit complete Q for NPE Eth 0/1, Port 1&2
*
*/
#define IX_ETH_ACC_TX_FRAME_DONE_ETH_Q    (IX_QMGR_QUEUE_31)

/*****************************************************************************************
 * Queue assignments for Crypto
 *****************************************************************************************/

/** Crypto Service Request Queue */
#define IX_CRYPTO_ACC_CRYPTO_REQ_Q  (IX_QMGR_QUEUE_29)

/** Crypto Service Done Queue */
#define IX_CRYPTO_ACC_CRYPTO_DONE_Q (IX_QMGR_QUEUE_30)

/** Crypto Req Q CB tag */
#define IX_CRYPTO_ACC_CRYPTO_REQ_Q_CB_TAG   (0)

/** Crypto Done Q CB tag */
#define IX_CRYPTO_ACC_CRYPTO_DONE_Q_CB_TAG  (1)

/** WEP Service Request Queue */
#define IX_CRYPTO_ACC_WEP_REQ_Q  (IX_QMGR_QUEUE_21)

/** WEP Service Done Queue */
#define IX_CRYPTO_ACC_WEP_DONE_Q (IX_QMGR_QUEUE_22)

/** WEP Req Q CB tag */
#define IX_CRYPTO_ACC_WEP_REQ_Q_CB_TAG      (2)

/** WEP Done Q CB tag */
#define IX_CRYPTO_ACC_WEP_DONE_Q_CB_TAG     (3)

/** Number of queues allocate to crypto hardware accelerator services */
#define IX_CRYPTO_ACC_NUM_OF_CRYPTO_Q       (2)

/** Number of queues allocate to WEP NPE services */
#define IX_CRYPTO_ACC_NUM_OF_WEP_NPE_Q      (2)
                                                      
/** Number of queues allocate to CryptoAcc component */
#define IX_CRYPTO_ACC_NUM_OF_Q (IX_CRYPTO_ACC_NUM_OF_CRYPTO_Q + IX_CRYPTO_ACC_NUM_OF_WEP_NPE_Q)   

#endif /* IxQueueAssignments_H */
