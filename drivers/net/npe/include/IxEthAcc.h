/** @file    IxEthAcc.h
 *
 * @brief this file contains the public API of @ref IxEthAcc component
 *
 * Design notes:
 * The IX_OSAL_MBUF address is to be specified on bits [31-5] and must 
 * be cache aligned (bits[4-0] cleared)
 *
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
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 *
 */

#ifndef IxEthAcc_H
#define IxEthAcc_H

#include <IxOsBuffMgt.h>
#include <IxTypes.h>

/**
 * @defgroup IxEthAcc IXP400 Ethernet Access (IxEthAcc) API
 *
 * @brief ethAcc is a library that does provides access to the internal IXP400 10/100Bt Ethernet MACs.
 *
 *@{
 */

/**
 * @ingroup IxEthAcc
 * @brief Definition of the Ethernet Access status
 */
typedef enum /* IxEthAccStatus */
{
    IX_ETH_ACC_SUCCESS = IX_SUCCESS, /**< return success*/
    IX_ETH_ACC_FAIL = IX_FAIL, /**< return fail*/
    IX_ETH_ACC_INVALID_PORT, /**< return invalid port*/
    IX_ETH_ACC_PORT_UNINITIALIZED, /**< return uninitialized*/
    IX_ETH_ACC_MAC_UNINITIALIZED, /**< return MAC uninitialized*/
    IX_ETH_ACC_INVALID_ARG, /**< return invalid arg*/
    IX_ETH_TX_Q_FULL, /**< return tx queue is full*/
    IX_ETH_ACC_NO_SUCH_ADDR /**< return no such address*/
} IxEthAccStatus;

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccPortId
 * @brief Definition of the IXP400 Mac Ethernet device.
 */
typedef enum  
{
	IX_ETH_PORT_1 = 0, /**< Ethernet Port 1 */
	IX_ETH_PORT_2 = 1  /**< Ethernet port 2 */
	,IX_ETH_PORT_3 = 2 /**< Ethernet port 3 */
} IxEthAccPortId;

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETH_ACC_NUMBER_OF_PORTS
 *
 * @brief  Definition of the number of ports
 *
 */
#ifdef __ixp46X
#define IX_ETH_ACC_NUMBER_OF_PORTS (3)
#else
#define IX_ETH_ACC_NUMBER_OF_PORTS (2)
#endif

/**
 * @ingroup IxEthAcc
 *
 * @def IX_IEEE803_MAC_ADDRESS_SIZE
 *
 * @brief  Definition of the size of the MAC address
 *
 */
#define IX_IEEE803_MAC_ADDRESS_SIZE (6)


/**
 *
 * @brief Definition of the IEEE 802.3 Ethernet MAC address structure.
 *
 * The data should be packed with bytes xx:xx:xx:xx:xx:xx 
 * @note
 * The data must be packed in network byte order.
 */
typedef struct  
{
    UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE]; /**< MAC address */
} IxEthAccMacAddr;

/**
 * @ingroup IxEthAcc
 * @def IX_ETH_ACC_NUM_TX_PRIORITIES
 * @brief Definition of the number of transmit priorities
 * 
 */
#define IX_ETH_ACC_NUM_TX_PRIORITIES (8)

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccTxPriority
 * @brief Definition of the relative priority used to transmit a frame
 * 
 */
typedef enum  
{
	IX_ETH_ACC_TX_PRIORITY_0 = 0, /**<Lowest Priority submission */
	IX_ETH_ACC_TX_PRIORITY_1 = 1, /**<submission prority of 1 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_2 = 2, /**<submission prority of 2 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_3 = 3, /**<submission prority of 3 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_4 = 4, /**<submission prority of 4 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_5 = 5, /**<submission prority of 5 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_6 = 6, /**<submission prority of 6 (0 is lowest)*/
	IX_ETH_ACC_TX_PRIORITY_7 = 7, /**<Highest priority submission */

	IX_ETH_ACC_TX_DEFAULT_PRIORITY = IX_ETH_ACC_TX_PRIORITY_0 /**< By default send all 
								 packets with lowest priority */
} IxEthAccTxPriority;

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccRxFrameType
 * @brief Identify the type of a frame.
 * 
 * @sa IX_ETHACC_NE_FLAGS
 * @sa IX_ETHACC_NE_LINKMASK
 */
typedef enum  
{
	IX_ETHACC_RX_LLCTYPE = 0x00, /**< 802.3 - 8802, with LLC/SNAP */
	IX_ETHACC_RX_ETHTYPE = 0x10, /**< 802.3 (Ethernet) without LLC/SNAP */
	IX_ETHACC_RX_STATYPE = 0x20, /**< 802.11, AP <=> STA */
	IX_ETHACC_RX_APTYPE  = 0x30  /**< 802.11, AP <=> AP */
} IxEthAccRxFrameType;

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccDuplexMode
 * @brief Definition to provision the duplex mode of the MAC. 
 * 
 */
typedef enum
{
    IX_ETH_ACC_FULL_DUPLEX, /**< Full duplex operation of the MAC */
    IX_ETH_ACC_HALF_DUPLEX  /**< Half duplex operation of the MAC */
} IxEthAccDuplexMode;


/**
 * @ingroup IxEthAcc
 * @struct IxEthAccNe
 * @brief Definition of service-specific informations.
 * 
 * This structure defines the Ethernet service-specific informations
 * and enable QoS and VLAN features.
 */
typedef struct
{
    UINT32 ixReserved_next;    /**< reserved for chaining */
    UINT32 ixReserved_lengths; /**< reserved for buffer lengths */
    UINT32 ixReserved_data;    /**< reserved for buffer pointer */
    UINT8  ixDestinationPortId; /**< Destination portId for this packet, if known by NPE */
    UINT8  ixSourcePortId; /**< Source portId for this packet */
    UINT16 ixFlags;        /**< BitField of option for this frame */
    UINT8  ixQoS;          /**< QoS class of the frame */
    UINT8  ixReserved;     /**< reserved */
    UINT16 ixVlanTCI;      /**< Vlan TCI */
    UINT8  ixDestMac[IX_IEEE803_MAC_ADDRESS_SIZE]; /**< Destination MAC address */
    UINT8  ixSourceMac[IX_IEEE803_MAC_ADDRESS_SIZE]; /**< Source MAC address */
} IxEthAccNe;

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_PORT_UNKNOWN
 *
 * @brief  Contents of the field @a IX_ETHACC_NE_DESTPORTID when no
 * destination port can be found by the NPE for this frame.
 *
 */
#define IX_ETHACC_NE_PORT_UNKNOWN   (0xff)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_DESTMAC
 *
 * @brief The location of the destination MAC address in the Mbuf header.
 *
 */
#define IX_ETHACC_NE_DESTMAC(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixDestMac

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_SOURCEMAC
 *
 * @brief The location of the source MAC address in the Mbuf header.
 *
 */
#define IX_ETHACC_NE_SOURCEMAC(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixSourceMac

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_VLANTCI
 *
 * @brief The VLAN Tag Control Information associated with this frame
 * 
 * The VLAN Tag Control Information associated with this frame. On Rx
 * path, this field is extracted from the packet header.
 * On Tx path, the value of this field is inserted in the frame when
 * the port is configured to insert or replace vlan tags in the 
 * egress frames.
 *
 * @sa IX_ETHACC_NE_FLAGS
 */
#define IX_ETHACC_NE_VLANTCI(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixVlanTCI

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_SOURCEPORTID
 *
 * @brief The port where this frame came from.
 *
 * The port where this frame came from. This field is set on receive
 * with the port information. This field is ignored on Transmit path.
 */
#define IX_ETHACC_NE_SOURCEPORTID(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixSourcePortId

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_DESTPORTID
 *
 * @brief The destination port where this frame should be sent.
 *
 * The destination port where this frame should be sent.
 *
 * @li In the transmit direction, this field contains the destination port
 * and is ignored unless @a IX_ETHACC_NE_FLAG_DST is set.
 * 
 * @li In the receive direction, this field contains the port where the
 * destination MAC addresses has been learned. If the destination
 * MAC address is unknown, then this value is set to the reserved value
 * @a IX_ETHACC_NE_PORT_UNKNOWN
 *
 */
#define IX_ETHACC_NE_DESTPORTID(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixDestinationPortId

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_QOS
 *
 * @brief QualityOfService class (QoS) for this received frame.
 *
 */
#define IX_ETHACC_NE_QOS(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixQoS

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_FLAGS
 *
 * @brief Bit Mask of the different flags associated with a frame
 * 
 * The flags are the bit-oring combination 
 * of the following different fields :
 *
 *      @li IP flag (Rx @a IX_ETHACC_NE_IPMASK)
 *      @li Spanning Tree flag (Rx @a IX_ETHACC_NE_STMASK)
 *      @li Link layer type (Rx and Tx @a IX_ETHACC_NE_LINKMASK)
 *      @li VLAN Tagged Frame (Rx @a IX_ETHACC_NE_VLANMASK)
 *      @li New source MAC address (Rx @a IX_ETHACC_NE_NEWSRCMASK)
 *      @li Multicast flag (Rx @a IX_ETHACC_NE_MCASTMASK)
 *      @li Broadcast flag (Rx @a IX_ETHACC_NE_BCASTMASK)
 *      @li Destination port flag (Tx @a IX_ETHACC_NE_PORTMASK)
 *      @li Tag/Untag Tx frame (Tx @a IX_ETHACC_NE_TAGMODEMASK)
 *      @li Overwrite destination port (Tx @a IX_ETHACC_NE_PORTOVERMASK)
 *      @li Filtered frame (Rx @a IX_ETHACC_NE_STMASK)
 *      @li VLAN Enabled (Rx and Tx @a IX_ETHACC_NE_VLANENABLEMASK)
 */
#define IX_ETHACC_NE_FLAGS(mBufPtr) ((IxEthAccNe *)&((mBufPtr)->ix_ne))->ixFlags

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_BCASTMASK
 *
 * @brief This mask defines if a received frame is a broadcast frame.
 *
 * This mask defines if a received frame is a broadcast frame.
 * The BCAST flag is set when the destination MAC address of 
 * a frame is broadcast.
 *
 * @sa IX_ETHACC_NE_FLAGS 
 *
 */
#define IX_ETHACC_NE_BCASTMASK      (0x1)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_MCASTMASK
 *
 * @brief This mask defines if a received frame is a multicast frame.
 *
 * This mask defines if a received frame is a multicast frame.
 * The MCAST flag is set when the destination MAC address of 
 * a frame is multicast.
 *
 * @sa IX_ETHACC_NE_FLAGS 
 *
 */
#define IX_ETHACC_NE_MCASTMASK      (0x1 << 1)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_IPMASK
 *
 * @brief This mask defines if a received frame is a IP frame.
 *
 * This mask applies to @a IX_ETHACC_NE_FLAGS and defines if a received 
 * frame is a IP frame. The IP flag is set on Rx direction, depending on 
 * the frame contents. The flag is set when the length/type field of a 
 * received frame is 0x8000.
 *
 * @sa IX_ETHACC_NE_FLAGS
 *
 */
#define IX_ETHACC_NE_IPMASK         (0x1 << 2)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_VLANMASK
 *
 * @brief This mask defines if a received frame is VLAN tagged.
 *
 * This mask defines if a received frame is VLAN tagged.
 * When set, the Rx frame is VLAN-tagged and the tag value 
 * is available thru @a IX_ETHACC_NE_VLANID.
 * Note that when sending frames which are already tagged
 * this flag should be set, to avoid inserting another VLAN tag.
 *
 * @sa IX_ETHACC_NE_FLAGS 
 * @sa IX_ETHACC_NE_VLANID
 *
 */
#define IX_ETHACC_NE_VLANMASK       (0x1 << 3)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_LINKMASK
 *
 * @brief This mask is the link layer protocol indicator
 *
 * This mask applies to @a IX_ETHACC_NE_FLAGS.
 * It reflects the state of a frame as it exits an NPE on the Rx path
 * or enters an NPE on the Tx path. Its values are as follows:
 *      @li 0x00 - IEEE802.3 - 8802 (Rx) / IEEE802.3 - 8802 (Tx)
 *      @li 0x01 - IEEE802.3 - Ethernet (Rx) / IEEE802.3 - Ethernet (Tx)
 *      @li 0x02 - IEEE802.11 AP -> STA (Rx) / IEEE802.11 STA -> AP (Tx)
 *      @li 0x03 - IEEE802.11 AP -> AP (Rx) / IEEE802.11 AP->AP (Tx)
 *
 * @sa IX_ETHACC_NE_FLAGS
 *
 */
#define IX_ETHACC_NE_LINKMASK       (0x3 << 4)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_STMASK
 *
 * @brief This mask defines if a received frame is a Spanning Tree frame.
 *
 * This mask applies to @a IX_ETHACC_NE_FLAGS.
 * On rx direction, it defines if a received if frame is a Spanning Tree frame.
 * Setting this fkag on transmit direction overrides the port settings 
 * regarding the VLAN options and 
 *
 * @sa IX_ETHACC_NE_FLAGS
 *
 */
#define IX_ETHACC_NE_STMASK         (0x1 << 6)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_FILTERMASK
 *
 * @brief This bit indicates whether a frame has been filtered by the Rx service.
 *
 * This mask applies to @a IX_ETHACC_NE_FLAGS.
 * Certain frames, which should normally be fully filtered by the NPE to due
 * the destination MAC address being on the same segment as the Rx port are
 * still forwarded to the XScale (although the payload is invalid) in order
 * to learn the MAC address of the transmitting station, if this is unknown.
 * Normally EthAcc will filter and recycle these framess internally and no
 * frames with the FILTER bit set will be received by the client.
 *
 * @sa IX_ETHACC_NE_FLAGS
 *
 */
#define IX_ETHACC_NE_FILTERMASK     (0x1 << 7)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_PORTMASK
 *
 * @brief This mask defines the rule to transmit a frame
 *
 * This mask defines the rule to transmit a frame. When set, a frame
 * is transmitted to the destination port as set by the macro
 * @a IX_ETHACC_NE_DESTPORTID. If not set, the destination port 
 * is searched using the destination MAC address.
 *
 * @note This flag is meaningful only for multiport Network Engines.
 * 
 * @sa IX_ETHACC_NE_FLAGS 
 * @sa IX_ETHACC_NE_DESTPORTID
 *
 */
#define IX_ETHACC_NE_PORTOVERMASK   (0x1 << 8)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_TAGMODEMASK
 *
 * @brief This mask defines the tagging rules to apply to a transmit frame.
 *
 * This mask defines the tagging rules to apply to a transmit frame
 * regardless of the default setting for a port. When used together 
 * with @a IX_ETHACC_NE_TAGOVERMASK and when set, the 
 * frame will be tagged prior to transmission. When not set,
 * the frame will be untagged prior to transmission. This is accomplished
 * irrespective of the Egress tagging rules, constituting a per-frame override.
 *
 * @sa IX_ETHACC_NE_FLAGS
 * @sa IX_ETHACC_NE_TAGOVERMASK 
 *
 */
#define IX_ETHACC_NE_TAGMODEMASK    (0x1 << 9)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_TAGOVERMASK
 *
 * @brief This mask defines the rule to transmit a frame
 *
 * This mask defines the rule to transmit a frame. When set, the
 * default transmit rules of a port are overriden.
 * When not set, the default rules as set by @ref IxEthDB should apply.
 *
 * @sa IX_ETHACC_NE_FLAGS
 * @sa IX_ETHACC_NE_TAGMODEMASK
 *
 */
#define IX_ETHACC_NE_TAGOVERMASK    (0x1 << 10)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_VLANENABLEMASK
 *
 * @brief This mask defines if a frame is a VLAN frame or not
 *
 * When set, frames undergo normal VLAN processing on the Tx path
 * (membership filtering, tagging, tag removal etc). If this flag is
 * not set, the frame is considered to be a regular non-VLAN frame
 * and no VLAN processing will be performed.
 *
 * Note that VLAN-enabled NPE images will always set this flag in all
 * Rx frames, and images which are not VLAN enabled will clear this
 * flag for all received frames.
 *
 * @sa IX_ETHACC_NE_FLAGS
 *
 */
#define IX_ETHACC_NE_VLANENABLEMASK (0x1 << 14)

/**
 * @ingroup IxEthAcc
 *
 * @def IX_ETHACC_NE_NEWSRCMASK
 *
 * @brief This mask defines if a received frame has been learned.
 *
 * This mask defines if the source MAC address of a frame is 
 * already known. If the bit is set, the source MAC address was
 * unknown to the NPE at the time the frame was received.
 *
 * @sa IX_ETHACC_NE_FLAGS 
 *
 */
#define IX_ETHACC_NE_NEWSRCMASK     (0x1 << 15)

/**
 * @ingroup IxEthAcc
 *
 * @brief This defines the recommanded minimum size of MBUF's submitted
 * to the frame receive service.
 *
 */
#define IX_ETHACC_RX_MBUF_MIN_SIZE (2048)

/**
 * @ingroup IxEthAcc
 *
 * @brief This defines the highest MII address of any attached PHYs 
 * 
 * The maximum number for PHY address is 31, add on for range checking.
 *
 */
#define IXP425_ETH_ACC_MII_MAX_ADDR   32

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccInit(void)
 * 
 * @brief Initializes the IXP400 Ethernet Access Service.
 * 
 * @li Reentrant    - no
 * @li ISR Callable - no
 * 
 * This should be called once per module initialization.
 * @pre
 *   The NPE must first be downloaded with the required microcode which supports all
 *   required features.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL	:  Service has failed to initialize.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccInit(void);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccUnload(void)
 * 
 * @brief Unload the Ethernet Access Service.
 * 
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @return void 
 *
 * <hr>
 */
PUBLIC void ixEthAccUnload(void);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortInit( IxEthAccPortId portId)
 *
 * @brief Initializes an NPE/Ethernet MAC Port.
 *
 * The NPE/Ethernet port initialisation includes the following steps
 * @li Initialize the NPE/Ethernet MAC hardware.
 * @li Verify NPE downloaded and operational.
 * @li The NPE shall be available for usage once this API returns.
 * @li Verify that the Ethernet port is present before initializing
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * This should be called once per mac device.
 * The NPE/MAC shall be in disabled state after init.
 *
 * @pre
 *   The component must be initialized via @a ixEthAccInit
 *   The NPE must first be downloaded with the required microcode which supports all
 *   required features.
 *
 * Dependant on Services: (Must be initialized before using this service may be initialized)
 *	ixNPEmh - NPE Message handling service.
 *	ixQmgr	- Queue Manager component.
 *
 * @param portId  @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS: if the ethernet port is not present, a warning is issued.
 * @li @a IX_ETH_ACC_FAIL : The NPE processor has failed to initialize.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortInit(IxEthAccPortId portId);


/*************************************************************************

 #####     ##     #####    ##            #####     ##     #####  #    #
 #    #   #  #      #     #  #           #    #   #  #      #    #    #
 #    #  #    #     #    #    #          #    #  #    #     #    ######
 #    #  ######     #    ######          #####   ######     #    #    #
 #    #  #    #     #    #    #          #       #    #     #    #    #
 #####   #    #     #    #    #          #       #    #     #    #    #

*************************************************************************/


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameSubmit( 
    IxEthAccPortId portId,
    IX_OSAL_MBUF *buffer, 
    IxEthAccTxPriority priority)
 * 
 * @brief This function shall be used to submit MBUFs buffers for transmission on a particular MAC device. 
 *
 * When the frame is transmitted, the buffer shall be returned thru the 
 * callback @a IxEthAccPortTxDoneCallback.
 *
 * In case of over-submitting, the order of the frames on the 
 * network may be modified.
 *
 * Buffers shall be not queued for transmission if the port is disabled.
 * The port can be enabled using @a ixEthAccPortEnable
 *
 * 
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 *
 * @pre 
 *  @a ixEthAccPortTxDoneCallbackRegister must be called to register a function to allow this service to
 *   return the buffer to the calling service. 
 * 
 * @note 
 *  If the buffer submit fails for any reason the user has retained ownership of the buffer.
 *
 * @param portId @ref IxEthAccPortId [in] - MAC port ID to transmit Ethernet frame on.
 * @param buffer @ref IX_OSAL_MBUF [in] - pointer to an MBUF formatted buffer. Chained buffers are supported for transmission.
 *             Chained packets are not supported and the field IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR is ignored. 
 * @param priority @ref IxEthAccTxPriority [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL  : Failed to queue frame for transmission. 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */

PUBLIC IxEthAccStatus ixEthAccPortTxFrameSubmit( 
    IxEthAccPortId portId,
    IX_OSAL_MBUF *buffer, 
    IxEthAccTxPriority priority);

/**
 * @ingroup IxEthAcc
 *
 * @brief Function prototype for Ethernet Tx Buffer Done callback. Registered 
 *  via @a ixEthAccTxBufferDoneCallbackRegister 
 * 
 * This function is called once the previously submitted buffer is no longer required by this service.
 * It may be returned upon successful transmission of the frame or during the shutdown of 
 * the port prior to the transmission of a queued frame.
 * The calling of this registered function is not a guarantee of successful transmission of the buffer.
 *
 *  
 * @li Reentrant    - yes , The user provided function should be reentrant.
 * @li ISR Callable - yes , The user provided function must be callable from an ISR.
 *
 *
 * <b>Calling Context </b>: 
 * @par
 *   This callback is called in the context of the queue manager dispatch loop @a ixQmgrgrDispatcherLoopRun
 *   within the @ref IxQMgrAPI component. The calling context may be from interrupt or high priority thread. 
 *   The decision is system specific.
 *
 * @param callbackTag UINT32 [in] - This tag is that provided when the callback was registered for a particular MAC 
 * via @a ixEthAccPortTxDoneCallbackRegister. It allows the same callback to be used for multiple MACs.
 * @param mbuf @ref IX_OSAL_MBUF [in] - Pointer to the Tx mbuf descriptor. 
 * 
 * @return void
 *
 * @note
 * The field IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR is modified by the access layer and reset to NULL.
 *
 * <hr>
 */
typedef void (*IxEthAccPortTxDoneCallback) ( UINT32 callbackTag, IX_OSAL_MBUF *buffer );



/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxDoneCallbackRegister( IxEthAccPortId portId, 
					   IxEthAccPortTxDoneCallback txCallbackFn, 
					   UINT32 callbackTag)
 *
 * @brief Register a callback function to allow 
 * the transmitted buffers to return to the user.
 * 
 * This function registers the transmit buffer done function callback for a particular port.
 *
 * The registered callback function is called once the previously submitted buffer is no longer required by this service.
 * It may be returned upon successful transmission of the frame or  shutdown of port prior to submission.
 * The calling of this registered function is not a guarantee of successful transmission of the buffer.
 *
 * If called several times the latest callback shall be registered for a particular port.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 * @pre
 *	The port must be initialized via @a ixEthAccPortInit
 *
 *
 * @param portId @ref IxEthAccPortId [in] - Register callback for a particular MAC device.
 * @param txCallbackFn @ref IxEthAccPortTxDoneCallback [in] - Function to be called to return transmit buffers to the user.
 * @param callbackTag UINT32 [in] -  This tag shall be provided to the callback function.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 * @li @a IX_ETH_ACC_INVALID_ARG : An argument other than portId is invalid.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortTxDoneCallbackRegister(IxEthAccPortId portId,
								   IxEthAccPortTxDoneCallback txCallbackFn,
								   UINT32 callbackTag);



/**
 * @ingroup IxEthAcc
 *
 * @brief Function prototype for Ethernet Frame Rx callback. Registered via @a ixEthAccPortRxCallbackRegister 
 * 
 * It is the responsibility of the user function to free any MBUF's which it receives.
 *  
 * @li Reentrant    - yes , The user provided function should be reentrant.
 * @li ISR Callable - yes , The user provided function must be callable from an ISR.
 * @par
 *
 * This function dispatches frames to the user level
 * via the provided function. The invocation shall be made for each
 * frame dequeued from the Ethernet QM queue. The user is required to free any MBUF's 
 * supplied via this callback. In addition the registered callback must free up MBUF's
 * from the receive free queue when the port is disabled 
 * 
 * If called several times the latest callback shall be registered for a particular port.
 *
 * <b>Calling Context </b>: 
 * @par
 *   This callback is called in the context of the queue manager dispatch loop @a ixQmgrgrDispatcherLoopRun
 *   within the @ref IxQMgrAPI component. The calling context may be from interrupt or high priority thread. 
 *   The decision is system specific.
 *
 *
 * @param callbackTag UINT32 [in] - This tag is that provided when the callback was registered for a particular MAC 
 * via @a ixEthAccPortRxCallbackRegister. It allows the same callback to be used for multiple MACs.
 * @param mbuf @ref IX_OSAL_MBUF [in] - Pointer to the Rx mbuf header. Mbufs may be chained if 
 *               the frame length is greater than the supplied mbuf length.
 * @param reserved [in] - deprecated parameter The information is passed 
 *      thru the IxEthAccNe header destination port ID field 
 *      (@sa IX_ETHACC_NE_DESTPORTID). For backward 
 *      compatibility,the value is equal to IX_ETH_DB_UNKNOWN_PORT (0xff). 
 * 
 * @return void
 *
 * @note
 * Buffers may not be filled up to the length supplied in 
 * @a ixEthAccPortRxFreeReplenish(). The firmware fills
 * them to the previous 64 bytes boundary. The user has to be aware 
 * that the length of the received mbufs may be smaller than the length
 * of the supplied mbufs. 
 * The mbuf header contains the following modified field
 * @li @a IX_OSAL_MBUF_PKT_LEN is set in the header of the first mbuf and indicates
 *  the total frame size
 * @li @a IX_OSAL_MBUF_MLEN is set each mbuf header and indicates the payload length
 * @li @a IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR contains a pointer to the next 
 *     mbuf, or NULL at the end of a chain.
 * @li @a IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR is modified. Its value is reset to NULL
 * @li @a IX_OSAL_MBUF_FLAGS contains the bit 4 set for a broadcast packet and the bit 5
 *     set for a multicast packet. Other bits are unmodified.
 *
 * <hr>
 */
typedef void (*IxEthAccPortRxCallback) (UINT32 callbackTag, IX_OSAL_MBUF *buffer, UINT32 reserved);

/**
 * @ingroup IxEthAcc
 *
 * @brief Function prototype for Ethernet Frame Rx callback. Registered via @a ixEthAccPortMultiBufferRxCallbackRegister 
 * 
 * It is the responsibility of the user function to free any MBUF's which it receives.
 *  
 * @li Reentrant    - yes , The user provided function should be reentrant.
 * @li ISR Callable - yes , The user provided function must be callable from an ISR.
 * @par
 *
 * This function dispatches many frames to the user level
 * via the provided function. The invocation shall be made for multiple frames
 * dequeued from the Ethernet QM queue. The user is required to free any MBUF's 
 * supplied via this callback. In addition the registered callback must free up MBUF's
 * from the receive free queue when the port is disabled 
 * 
 * If called several times the latest callback shall be registered for a particular port.
 *
 * <b>Calling Context </b>: 
 * @par
 *   This callback is called in the context of the queue manager dispatch loop @a ixQmgrDispatcherLoopRun
 *   within the @ref IxQMgrAPI component. The calling context may be from interrupt or high priority thread. 
 *   The decision is system specific.
 *
 *
 * @param callbackTag - This tag is that provided when the callback was registered for a particular MAC 
 * via @a ixEthAccPortMultiBufferRxCallbackRegister. It allows the same callback to be used for multiple MACs.
 * @param mbuf - Pointer to an array of Rx mbuf headers. Mbufs 
 *               may be chained if 
 *               the frame length is greater than the supplied mbuf length.
 *               The end of the array contains a zeroed entry (NULL pointer).
 *
 * @return void
 *
 * @note The mbufs passed to this callback have the same structure than the
 *  buffers passed to @a IxEthAccPortRxCallback interfac. 
 *
 * @note The usage of this callback is exclusive with the usage of
 *  @a ixEthAccPortRxCallbackRegister and @a IxEthAccPortRxCallback 
 *
 * @sa ixEthAccPortMultiBufferRxCallbackRegister
 * @sa IxEthAccPortMultiBufferRxCallback
 * @sa ixEthAccPortRxCallbackRegister
 * @sa IxEthAccPortRxCallback
 * <hr>
 */

typedef void (*IxEthAccPortMultiBufferRxCallback) (UINT32 callbackTag, IX_OSAL_MBUF **buffer);




/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxCallbackRegister( IxEthAccPortId portId, IxEthAccPortRxCallback rxCallbackFn, UINT32 callbackTag)
 *
 * @brief Register a callback function to allow 
 * the reception of frames.
 *
 * The registered callback function is called once a frame is received  by this service.
 *
 * If called several times the latest callback shall be registered for a particular port.
 *
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 *
 * @param portId @ref IxEthAccPortId [in] - Register callback for a particular MAC device.
 * @param rxCallbackFn @ref IxEthAccPortRxCallback [in] - Function to be called when Ethernet frames are availble.
 * @param callbackTag UINT32 [in] -  This tag shall be provided to the callback function.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 * @li @a IX_ETH_ACC_INVALID_ARG : An argument other than portId is invalid.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortRxCallbackRegister(IxEthAccPortId portId,
							   IxEthAccPortRxCallback rxCallbackFn,
							   UINT32 callbackTag);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMultiBufferRxCallbackRegister( IxEthAccPortId portId, IxEthAccPortMultiBufferRxCallback rxCallbackFn, UINT32 callbackTag)
 *
 * @brief Register a callback function to allow 
 * the reception of frames.
 * 
 * The registered callback function is called once a frame is 
 * received  by this service. If many frames are already received, 
 * the function is called once.
 *
 * If called several times the latest callback shall be registered for a particular port.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 *
 * @param portId - Register callback for a particular MAC device.
 * @param rxCallbackFn - @a IxEthAccMultiBufferRxCallbackFn - Function to be called when Ethernet frames are availble.
 * @param callbackTag -  This tag shall be provided to the callback function.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 * @li @a IX_ETH_ACC_INVALID_ARG : An argument other than portId is invalid.
 *
 * @sa ixEthAccPortMultiBufferRxCallbackRegister
 * @sa IxEthAccPortMultiBufferRxCallback
 * @sa ixEthAccPortRxCallbackRegister
 * @sa IxEthAccPortRxCallback
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortMultiBufferRxCallbackRegister(IxEthAccPortId portId,
										  IxEthAccPortMultiBufferRxCallback rxCallbackFn,
										  UINT32 callbackTag);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFreeReplenish( IxEthAccPortId portId, IX_OSAL_MBUF *buffer)
 *
 * @brief This function provides buffers for the Ethernet receive path. 
 *
 * This component does not have a buffer management mechanisms built in. All Rx buffers must be supplied to it
 * via this interface. 
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 * @param portId @ref IxEthAccPortId [in] - Provide buffers only to specific Rx MAC. 
 * @param buffer @ref IX_OSAL_MBUF [in] - Provide an MBUF to the Ethernet receive mechanism. 
 *                 Buffers size smaller than IX_ETHACC_RX_MBUF_MIN_SIZE may result in poor
 *                 performances and excessive buffer chaining. Buffers
 *                 larger than this size may be suitable for jumbo frames.
 *                 Chained packets are not supported and the field IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR must be NULL. 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : Buffer has was not able to queue the 
 *                     buffer in the receive service.
 * @li @a IX_ETH_ACC_FAIL : Buffer size is less than IX_ETHACC_RX_MBUF_MIN_SIZE
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * @note
 * If the buffer replenish operation fails it is the responsibility 
 * of the user to free the buffer.
 *
 * @note
 * Sufficient buffers must be supplied to the component to maintain
 * receive throughput and avoid rx buffer underflow conditions.
 * To meet this goal, It is expected that the user preload the 
 * component with a sufficent number of buffers prior to enabling the
 * NPE Ethernet receive path. The recommended minimum number of 
 * buffers is 8.
 *
 * @note
 * For maximum performances, the mbuf size should be greater 
 * than the maximum frame size (Ethernet header, payload and FCS) + 64. 
 * Supplying smaller mbufs to the service results in mbuf
 * chaining and degraded performances. The recommended size
 * is @a IX_ETHACC_RX_MBUF_MIN_SIZE, which is
 * enough to take care of 802.3 frames and "baby jumbo" frames without
 * chaining, and "jumbo" frame within chaining.
 *
 * @note
 * Buffers may not be filled up to their length. The firware fills
 * them up to the previous 64 bytes boundary. The user has to be aware 
 * that the length of the received mbufs may be smaller than the length
 * of the supplied mbufs.
 *
 * @warning This function checks the parameters if the NDEBUG 
 * flag is not defined. Turning on the argument checking (disabled by 
 * default) results in a lower EthAcc performance as this function
 * is part of the data path.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortRxFreeReplenish( IxEthAccPortId portId, IX_OSAL_MBUF *buffer);



/***************************************************************

  ####    ####   #    #   #####  #####    ####   #
 #    #  #    #  ##   #     #    #    #  #    #  #
 #       #    #  # #  #     #    #    #  #    #  #
 #       #    #  #  # #     #    #####   #    #  #
 #    #  #    #  #   ##     #    #   #   #    #  #
  ####    ####   #    #     #    #    #   ####   ######


         #####   #         ##    #    #  ######
         #    #  #        #  #   ##   #  #
         #    #  #       #    #  # #  #  #####
         #####   #       ######  #  # #  #
         #       #       #    #  #   ##  #
         #       ######  #    #  #    #  ######

***************************************************************/

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortEnable(IxEthAccPortId portId)
 *
 * @brief This enables an Ethernet port for both Tx and Rx. 
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre The port must first be initialized via @a ixEthAccPortInit and the MAC address 
 * must be set using @a ixEthAccUnicastMacAddressSet before enabling it
 * The rx and Tx Done callbacks registration via @a
 * ixEthAccPortTxDoneCallbackRegister amd @a  ixEthAccPortRxCallbackRegister
 * has to be done before enabling the traffic.
 * 
 * @param  portId @ref IxEthAccPortId [in] - Port id to act upon.
 * 
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is not initialized
 * @li @a IX_ETH_ACC_MAC_UNINITIALIZED : port MAC address is not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDisable(IxEthAccPortId portId)
 *
 * @brief This disables an Ethernet port for both Tx and Rx. 
 *
 * Free MBufs are returned to the user via the registered callback when the port is disabled 
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre The port must be enabled with @a ixEthAccPortEnable, otherwise this
 * function has no effect
 *
 * @param  portId @ref IxEthAccPortId [in] - Port id to act upon.
 * 
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is not initialized
 * @li @a IX_ETH_ACC_MAC_UNINITIALIZED : port MAC address is not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled)
 *
 * @brief Get the enabled state of a port.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 * @pre The port must first be initialized via @a ixEthAccPortInit
 *
 * @param  portId @ref IxEthAccPortId [in] - Port id to act upon.
 * @param  enabled BOOL [out] - location to store the state of the port
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId)
 *
 * @brief Put the Ethernet MAC device in non-promiscuous mode.
 * 
 * In non-promiscuous mode the MAC filters all frames other than 
 * destination MAC address which matches the following criteria:
 * @li Unicast address provisioned via @a ixEthAccUnicastMacAddressSet
 * @li All broadcast frames.
 * @li Multicast addresses provisioned via @a ixEthAccMulticastAddressJoin
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 * 
 * @sa ixEthAccPortPromiscuousModeSet
 * 
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * 
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId);


/**
 * @ingroup IxEthAcc
 *
 * @fn  ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId)
 *
 * @brief Put the MAC device in promiscuous mode.
 * 
 * If the device is in promiscuous mode then all all received frames shall be forwared
 * to the NPE for processing.
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 * 
 * @sa ixEthAccPortPromiscuousModeClear
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * 
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastMacAddressSet(      IxEthAccPortId portId,
                                                  IxEthAccMacAddr *macAddr)
 *
 * @brief Configure unicast MAC address for a particular port
 *
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * @param *macAddr @ref IxEthAccMacAddr [in] - Ethernet Mac address.
 *
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortUnicastMacAddressSet(IxEthAccPortId portId,
													   IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastMacAddressGet(	IxEthAccPortId portId, 
					IxEthAccMacAddr *macAddr)
 *
 * @brief Get unicast MAC address for a particular MAC port 
 *
 * @pre
 * The MAC address must first be set via @a ixEthAccMacPromiscuousModeSet
 * If the MAC address has not been set, the function returns a 
 * IX_ETH_ACC_MAC_UNINITIALIZED status
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * @param *macAddr @ref IxEthAccMacAddr [out] - Ethernet MAC address.
 *
 * @return  IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_MAC_UNINITIALIZED : port MAC address is not initialized.
 * @li @a IX_ETH_ACC_FAIL : macAddr is invalid.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortUnicastMacAddressGet(IxEthAccPortId portId,
								 IxEthAccMacAddr *macAddr);




/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressJoin(      IxEthAccPortId portId,
                                             IxEthAccMacAddr *macAddr)
 *
 * @brief Add a multicast address to the MAC address table.
 *
 *  @note
 *  Due to the operation of the Ethernet MAC multicast filtering mechanism, frames which do not 
 *  have a multicast destination address which were provisioned via this API may be forwarded 
 *  to the NPE's. This is a result of the hardware comparison  algorithm used in the destination mac address logic
 *  within the Ethernet MAC. 
 *
 *  See Also: IXP425 hardware development manual.
 * 
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * @param *macAddr @ref IxEthAccMacAddr [in] - Ethernet Mac address.
 *
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : Error writing to the MAC registers
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoin(IxEthAccPortId portId,
								 IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressJoinAll(  IxEthAccPortId portId)
 *
 * @brief Filter all frames with multicast dest.
 *
 * This function clears the MAC address table, and then sets
 * the MAC to forward ALL multicast frames to the NPE.
 * Specifically, it forwards all frames whose destination address
 * has the LSB of the highest byte set  (01:00:00:00:00:00).  This
 * bit is commonly referred to as the "multicast bit".
 * Broadcast frames will still be forwarded. 
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 *
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoinAll(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressLeave( IxEthAccPortId portId,
                                         IxEthAccMacAddr *macAddr)
 *
 * @brief Remove a multicast address from the MAC address table.
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 * @param *macAddr @ref IxEthAccMacAddr [in] - Ethernet Mac address.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_NO_SUCH_ADDR :  Failed if MAC address was not in the table.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeave(IxEthAccPortId portId,
								  IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressLeaveAll( IxEthAccPortId portId)
 *
 * @brief This function unconfigures the multicast filtering settings
 *
 * This function first clears the MAC address table, and then sets
 * the MAC as configured by the promiscuous mode current settings.
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeaveAll(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastAddressShow(IxEthAccPortId portId)
 *
 * @brief Displays unicast MAC address
 *
 * Displays unicast address which is configured using 
 * @a ixEthAccUnicastMacAddressSet. This function also displays the MAC filter used
 * to filter multicast frames.
 *
 * Other functions modify the MAC filtering
 *
 * @li @a ixEthAccPortMulticastAddressJoinAll() - all multicast
 *     frames are forwarded to the application
 * @li @a ixEthAccPortMulticastAddressLeaveAll() - rollback the
 *     effects of @a ixEthAccPortMulticastAddressJoinAll()
 * @li @a ixEthAccPortMulticastAddressLeave() - unprovision a new 
 *     filtering address
 * @li @a ixEthAccPortMulticastAddressJoin() - provision a new 
 *     filtering address
 * @li @a ixEthAccPortPromiscuousModeSet() - all frames are 
 *     forwarded to the application regardless of the multicast 
 *     address provisioned
 * @li @a ixEthAccPortPromiscuousModeClear() - frames are forwarded 
 *     to the application following the multicast address provisioned
 *
 * In all cases, unicast and broadcast addresses are forwarded to 
 * the application.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 *
 * @return void
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccPortUnicastAddressShow(IxEthAccPortId portId);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressShow( IxEthAccPortId portId)
 *
 * @brief Displays multicast MAC address
 *
 * Displays multicast address which have been configured using @a ixEthAccMulticastAddressJoin
 * 
 * @li Reentrant    - yes
 * @li ISR Callable - no
 * 
 * @param portId @ref IxEthAccPortId [in] - Ethernet port id.
 *
 * @return void
 *
 * <hr>
 */
PUBLIC void ixEthAccPortMulticastAddressShow( IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDuplexModeSet( IxEthAccPortId portId, IxEthAccDuplexMode mode )
 *
 * @brief  Set the duplex mode for the MAC.
 *
 * Configure the IXP400 MAC to either full or half duplex. 
 *
 * @note 
 * The configuration should match that provisioned on the PHY.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in]
 * @param mode @ref IxEthAccDuplexMode [in]
 *
 * @return IxEthAccStatus 
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeSet(IxEthAccPortId portId,IxEthAccDuplexMode mode);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDuplexModeGet( IxEthAccPortId portId, IxEthAccDuplexMode *mode )
 *
 * @brief  Get the duplex mode for the MAC.
 *
 * return the duplex configuration of the IXP400 MAC.
 *
 * @note
 * The configuration should match that provisioned on the PHY.
 * See @a ixEthAccDuplexModeSet
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in]
 * @param *mode @ref IxEthAccDuplexMode [out]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 *
 */
PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeGet(IxEthAccPortId portId,IxEthAccDuplexMode *mode );

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendPaddingEnable( IxEthAccPortId portId)
 *
 * @brief  Enable padding bytes to be appended to runt frames submitted to
 * this port
 * 
 * Enable up to 60 null-bytes padding bytes to be appended to runt frames 
 * submitted to this port. This is the default behavior of the access 
 * component.
 *
 * @warning Do not change this behaviour while the port is enabled.
 *
 * @note When Tx padding is enabled, Tx FCS generation is turned on
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @sa ixEthAccPortTxFrameAppendFCSDusable
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendPaddingDisable( IxEthAccPortId portId)
 *
 * @brief  Disable padding bytes to be appended to runt frames submitted to
 * this port
 * 
 * Disable padding bytes to be appended to runt frames 
 * submitted to this port. This is not the default behavior of the access 
 * component.
 *
 * @warning Do not change this behaviour while the port is enabled.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendFCSEnable( IxEthAccPortId portId)
 *
 * @brief  Enable the appending of Ethernet FCS to all frames submitted to this port
 * 
 * When enabled, the FCS is added to the submitted frames. This is the default 
 * behavior of the access component.
 * Do not change this behaviour while the port is enabled.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in] 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendFCSDisable( IxEthAccPortId portId)
 *
 * @brief  Disable the appending of Ethernet FCS to all frames submitted to this port.
 * 
 * When disabled, the Ethernet FCS is not added to the submitted frames. 
 * This is not the default
 * behavior of the access component.
 *
 * @note Since the FCS is not appended to the frame it is expected that the frame submitted to the 
 * component includes a valid FCS at the end of the data, although this will not be validated.
 *
 * The component shall forward the frame to the Ethernet MAC WITHOUT modification.
 *
 * Do not change this behaviour while the port is enabled.
 *
 * @note Tx FCS append is not disabled while Tx padding is enabled.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @sa ixEthAccPortTxFrameAppendPaddingEnable
 *
 * @param portId @ref IxEthAccPortId [in] 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFrameAppendFCSEnable( IxEthAccPortId portId)
 *
 * @brief Forward frames with FCS included in the receive buffer.
 *
 * The FCS is not striped from the receive buffer. 
 * The received frame length includes the FCS size (4 bytes). ie. 
 * A minimum sized ethernet frame shall have a length of 64bytes.
 *
 * Frame FCS validity checks are still carried out on all received frames.
 *
 * This is not the default
 * behavior of the access component.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFrameAppendFCSDisable( IxEthAccPortId portId)
 *
 * @brief  Do not forward the FCS portion of the received Ethernet frame to the user. 
 * The FCS is striped from the receive buffer. 
 * The received frame length does not include the FCS size (4 bytes).
 * Frame FCS validity checks are still carried out on all received frames.
 *
 * This is the default behavior of the component.
 * Do not change this behaviour while the port is enabled.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS 
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSDisable(IxEthAccPortId portId);




/**
 * @ingroup IxEthAcc
 *
 * @enum IxEthAccSchedulerDiscipline
 *
 * @brief  Definition for the port scheduling discipline
 *
 * Select the port scheduling discipline on receive and transmit path
 * @li FIFO : No Priority : In this configuration all frames are processed
 *                       in the access component in the strict order in which 
 *                        the component received them.
 * @li FIFO : Priority : This shall be a very simple priority mechanism. 
 *                     Higher prior-ity frames shall be forwarded 
 *                     before lower priority frames. There shall be no 
 *                     fairness mechanisms applied across different 
 *                     priorities. Higher priority frames could starve 
 *                     lower priority frames indefinitely.
 */
typedef  enum 
{
    FIFO_NO_PRIORITY, /**<frames submitted with no priority*/
    FIFO_PRIORITY /**<higher prority frames submitted before lower priority*/
}IxEthAccSchedulerDiscipline;

/**
 * @ingroup IxEthAcc
 *
 * @def IxEthAccTxSchedulerDiscipline
 *
 * @brief  Deprecated definition for the port transmit scheduling discipline
 */ 
#define IxEthAccTxSchedulerDiscipline IxEthAccSchedulerDiscipline



/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccTxSchedulingDisciplineSet( IxEthAccPortId portId, IxEthAccSchedulerDiscipline sched)
 *
 * @brief Set the port scheduling to one of @a IxEthAccSchedulerDiscipline
 *
 * The default behavior of the component is @a FIFO_NO_PRIORITY.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 *
 * @param portId @ref IxEthAccPortId [in] 
 * @param sched @ref IxEthAccSchedulerDiscipline [in] 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Set appropriate discipline.
 * @li @a IX_ETH_ACC_FAIL :  Invalid/unsupported discipline.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccTxSchedulingDisciplineSet(IxEthAccPortId portId, 
								  IxEthAccSchedulerDiscipline sched);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccRxSchedulingDisciplineSet(IxEthAccSchedulerDiscipline sched)
 *
 * @brief Set the Rx scheduling to one of @a IxEthAccSchedulerDiscipline
 *
 * The default behavior of the component is @a FIFO_NO_PRIORITY.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @param sched : @a IxEthAccSchedulerDiscipline 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Set appropriate discipline.
 * @li @a IX_ETH_ACC_FAIL :  Invalid/unsupported discipline.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccRxSchedulingDisciplineSet(IxEthAccSchedulerDiscipline sched);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccNpeLoopbackEnable(IxEthAccPortId portId)
 *
 * @brief Enable NPE loopback
 *
 * When this loopback mode is enabled all the transmitted frames are
 * received on the same port, without payload.
 *
 * This function is recommended for power-up diagnostic checks and
 * should never be used under normal Ethernet traffic operations.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @param portId : ID of the port 
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : NPE loopback mode enabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortNpeLoopbackDisable(IxEthAccPortId portId)
 *
 * @brief Disable NPE loopback
 *
 * This function is used to disable the NPE loopback if previously
 * enabled using ixEthAccNpeLoopbackEnable.
 *
 * This function is recommended for power-up diagnostic checks and
 * should never be used under normal Ethernet traffic operations.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : NPE loopback successfully disabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortTxEnable(IxEthAccPortId portId)
 *
 * @brief Enable Tx on the port
 *
 * This function is the complement of ixEthAccPortTxDisable and should
 * be used only after Tx was disabled. A MAC core reset is required before
 * this function is called (see @a ixEthAccPortMacReset).
 *
 * This function is the recommended usage scenario for emergency security
 * shutdown and hardware failure recovery and should never be used for throttling 
 * traffic.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Tx successfully enabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortTxEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortTxDisable(IxEthAccPortId portId)
 *
 * @brief Disable Tx on the port
 *
 * This function can be used to disable Tx in the MAC core.
 * Tx can be re-enabled, although this is not guaranteed, by performing
 * a MAC core reset (@a ixEthAccPortMacReset) and calling ixEthAccPortTxEnable.
 * Note that using this function is not recommended, except for shutting
 * down Tx for emergency reasons. For proper port shutdown and re-enabling
 * see ixEthAccPortEnable and ixEthAccPortDisable.
 *
 * This function is the recommended usage scenario for emergency security
 * shutdown and hardware failure recovery and should never be used for throttling 
 * traffic.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @pre
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Tx successfully disabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortTxDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortRxEnable(IxEthAccPortId portId)
 *
 * @brief Enable Rx on the port
 *
 * This function is the complement of ixEthAccPortRxDisable and should
 * be used only after Rx was disabled.
 *
 * This function is the recommended usage scenario for emergency security
 * shutdown and hardware failure recovery and should never be used for throttling 
 * traffic.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @pre
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Rx successfully enabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortRxEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortRxDisable(IxEthAccPortId portId)
 *
 * @brief Disable Rx on the port
 *
 * This function can be used to disable Rx in the MAC core.
 * Rx can be re-enabled, although this is not guaranteed, by performing
 * a MAC core reset (@a ixEthAccPortMacReset) and calling ixEthAccPortRxEnable.
 * Note that using this function is not recommended, except for shutting
 * down Rx for emergency reasons. For proper port shutdown and re-enabling
 * see ixEthAccPortEnable and ixEthAccPortDisable.
 *
 * This function is the recommended usage scenario for emergency security
 * shutdown and hardware failure recovery and should never be used for throttling 
 * traffic.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : Rx successfully disabled
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortRxDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn IxEthAccStatus ixEthAccPortMacReset(IxEthAccPortId portId)
 *
 * @brief Reset MAC core on the port
 *
 * This function will perform a MAC core reset (NPE Ethernet coprocessor).
 * This function is inherently unsafe and the NPE recovery is not guaranteed
 * after this function is called. The proper manner of performing port disable
 * and enable (which will reset the MAC as well) is ixEthAccPortEnable/ixEthAccPortDisable.
 *
 * This function is the recommended usage scenario for hardware failure recovery
 * and should never be used for throttling traffic.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @note Calling ixEthAccPortDisable followed by ixEthAccPortEnable is
 * guaranteed to restore correct Ethernet Tx/Rx operation.
 *
 * @param portId : ID of the port 
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS : MAC core reset
 * @li @a IX_ETH_ACC_FAIL : Invalid port or Ethernet service not initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus 
ixEthAccPortMacReset(IxEthAccPortId portId);

/*********************************************************************************
  ####    #####    ##     #####     #     ####    #####     #     ####    ####
 #          #     #  #      #       #    #          #       #    #    #  #
  ####      #    #    #     #       #     ####      #       #    #        ####
      #     #    ######     #       #         #     #       #    #            #
 #    #     #    #    #     #       #    #    #     #       #    #    #  #    #
  ####      #    #    #     #       #     ####      #       #     ####    ####
**********************************************************************************/


/**
 *
 * @brief This struct defines the statistics returned by this component.
 *
 * The component returns MIB2 EthObj variables which are obtained from the 
 * hardware or maintained by this component. 
 *
 *
 */
typedef struct   
{
    UINT32 dot3StatsAlignmentErrors;            /**< link error count (rx) */
    UINT32 dot3StatsFCSErrors;                  /**< link error count (rx) */
    UINT32 dot3StatsInternalMacReceiveErrors;   /**< link error count (rx) */
    UINT32 RxOverrunDiscards;                   /**< NPE: discarded frames count (rx) */
    UINT32 RxLearnedEntryDiscards;                /**< NPE: discarded frames count(rx)  */
    UINT32 RxLargeFramesDiscards;                 /**< NPE: discarded frames count(rx)  */
    UINT32 RxSTPBlockedDiscards;                  /**< NPE: discarded frames count(rx)  */
    UINT32 RxVLANTypeFilterDiscards;              /**< NPE: discarded frames count (rx) */
    UINT32 RxVLANIdFilterDiscards;                /**< NPE: discarded frames count (rx) */
    UINT32 RxInvalidSourceDiscards;               /**< NPE: discarded frames count (rx) */
    UINT32 RxBlackListDiscards;                   /**< NPE: discarded frames count (rx) */
    UINT32 RxWhiteListDiscards;                   /**< NPE: discarded frames count (rx) */
    UINT32 RxUnderflowEntryDiscards;              /**< NPE: discarded frames count (rx) */
    UINT32 dot3StatsSingleCollisionFrames;      /**< link error count (tx) */
    UINT32 dot3StatsMultipleCollisionFrames;    /**< link error count (tx) */
    UINT32 dot3StatsDeferredTransmissions;      /**< link error count (tx) */
    UINT32 dot3StatsLateCollisions;             /**< link error count (tx) */
    UINT32 dot3StatsExcessiveCollsions;         /**< link error count (tx) */
    UINT32 dot3StatsInternalMacTransmitErrors;  /**< link error count (tx) */
    UINT32 dot3StatsCarrierSenseErrors;         /**< link error count (tx) */
    UINT32 TxLargeFrameDiscards;                /**< NPE: discarded frames count (tx) */
    UINT32 TxVLANIdFilterDiscards;              /**< NPE: discarded frames count (tx) */

}IxEthEthObjStats;

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsGet(IxEthAccPortId portId ,IxEthEthObjStats *retStats )
 *
 * @brief  Returns the statistics maintained for a port.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 *
 * @param portId @ref IxEthAccPortId [in] 
 * @param retStats @ref IxEthEthObjStats [out]
 * @note Please note the user is responsible for cache coheriency of the retStat
 * buffer. The data is actually populated via the NPE's. As such cache safe
 * memory should be used in the retStats argument.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : Invalid arguments.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccMibIIStatsGet(IxEthAccPortId portId, IxEthEthObjStats *retStats );

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsGetClear(IxEthAccPortId portId, IxEthEthObjStats *retStats)
 * 
 * @brief  Returns and clears the statistics maintained for a port. 
 *
 * @li Reentrant    - yes
 * @li ISR Callable - yes
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId [in] 
 * @param retStats @ref IxEthEthObjStats [out]
 * @note Please note the user is responsible for cache coheriency of the retStats
 * buffer. The data is actually populated via the NPE's. As such cache safe
 * memory should be used in the retStats argument.
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : invalid arguments.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccMibIIStatsGetClear(IxEthAccPortId portId, IxEthEthObjStats *retStats);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsClear(IxEthAccPortId portId)
 *
 * @brief   Clears the statistics maintained for a port.
 *
 * @li Reentrant    - yes
 * @li ISR Callable - no
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : Invalid arguments.
 * @li @a IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * @li @a IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccMibIIStatsClear(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMacInit(IxEthAccPortId portId)
 * 
 * @brief Initializes the ethernet MAC settings 
 * 
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_INVALID_PORT	:  portId is invalid.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccMacInit(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccStatsShow(IxEthAccPortId portId)
 *
 *
 * @brief Displays a ports statistics on the standard io console using printf.
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId [in]
 *
 * @return void
 *
 * <hr>
 */
PUBLIC void ixEthAccStatsShow(IxEthAccPortId portId);

/*************************************************************************

 #    #     #       #            #    #  #####      #     ####
 ##  ##     #       #            ##  ##  #    #     #    #    #
 # ## #     #       #            # ## #  #    #     #    #    #
 #    #     #       #            #    #  #    #     #    #    #
 #    #     #       #            #    #  #    #     #    #    #
 #    #     #       #            #    #  #####      #     ####

*************************************************************************/


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiReadRtn (UINT8 phyAddr, 
                           UINT8 phyReg, 
	                   UINT16 *value)
 *
 *
 * @brief Reads a 16 bit value from a PHY
 *
 * Reads a 16-bit word from a register of a MII-compliant PHY. Reading
 * is performed through the MII management interface.  This function returns
 * when the read operation has successfully completed, or when a timeout has elapsed.
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr UINT8 [in] - the address of the Ethernet PHY (0-31)
 * @param phyReg UINT8 [in] -  the number of the MII register to read (0-31)
 * @param value UINT16 [in] -  the value read from the register
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : failed to read the register.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccMiiReadRtn (UINT8 phyAddr, UINT8 phyReg, UINT16 *value);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiWriteRtn (UINT8 phyAddr, 
                            UINT8 phyReg, 
	                    UINT16 value)
 *
 *
 * @brief Writes a 16 bit value to a PHY
 *
 * Writes a 16-bit word from a register of a MII-compliant PHY. Writing
 * is performed through the MII management interface.  This function returns
 * when the write operation has successfully completed, or when a timeout has elapsed.
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr UINT8 [in] - the address of the Ethernet PHY (0-31)
 * @param phyReg UINT8 [in] -  the number of the MII register to write (0-31)
 * @param value UINT16 [out] -  the value to write to the register
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : failed to write register.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccMiiWriteRtn (UINT8 phyAddr, UINT8 phyReg, UINT16 value);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiAccessTimeoutSet(UINT32 timeout)
 *
 * @brief Overrides the default timeout value and retry count when reading or 
 * writing MII registers using ixEthAccMiiWriteRtn or ixEthAccMiiReadRtn
 *
 * The default behavior of the component is to use a IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS ms
 * timeout (declared as 100 in IxEthAccMii_p.h) and a retry count of IX_ETH_ACC_MII_TIMEOUT_10TH_SECS
 * (declared as 5 in IxEthAccMii_p.h).
 *
 * The MII read and write functions will attempt to read the status of the register up
 * to the retry count times, delaying between each attempt with the timeout value.
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @pre
 *
 * @param timeout UINT32 [in] - new timeout value, in milliseconds
 * @param timeout UINT32 [in] - new retry count (a minimum value of 1 must be used)
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : invalid parameter(s)
 *
 * <hr>
 */
PUBLIC IxEthAccStatus
ixEthAccMiiAccessTimeoutSet(UINT32 timeout, UINT32 retryCount);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiStatsShow (UINT32 phyAddr)
 *
 *
 * @brief Displays detailed information on a specified PHY
 *
 * Displays the current values of the first eigth MII registers for a PHY, 
 *
 * @li Reentrant    - no
 * @li ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and 
 *      generating the MDIO clock.
 *   
 * @param phyAddr UINT32 [in] - the address of the Ethernet PHY (0-31)
 *
 * @return IxEthAccStatus
 * @li @a IX_ETH_ACC_SUCCESS
 * @li @a IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
PUBLIC IxEthAccStatus ixEthAccMiiStatsShow (UINT32 phyAddr);



/******* BOARD SPECIFIC DEPRECATED API *********/

/* The following functions are high level functions which rely
 * on the properties and interface of some Ethernet PHYs. The
 * implementation is hardware specific and has been moved to 
 * the hardware-specific component IxEthMii.
 */

 #include "IxEthMii.h"

/**
 * @ingroup IxEthAcc
 *
 * @def  ixEthAccMiiPhyScan
 *
 * @brief : deprecated API entry point. This definition 
 * ensures backward compatibility
 *
 * See @ref ixEthMiiPhyScan
 *
 * @note this feature is board specific
 *
 */
#define ixEthAccMiiPhyScan(phyPresent) ixEthMiiPhyScan(phyPresent,IXP425_ETH_ACC_MII_MAX_ADDR)

/**
 * @ingroup IxEthAcc
 *
 * @def ixEthAccMiiPhyConfig
 *
 * @brief : deprecated API entry point. This definition 
 * ensures backward compatibility
 *
 * See @ref ixEthMiiPhyConfig
 *
 * @note this feature is board specific
 */
#define ixEthAccMiiPhyConfig(phyAddr,speed100,fullDuplex,autonegotiate) \
           ixEthMiiPhyConfig(phyAddr,speed100,fullDuplex,autonegotiate)

/**
 * @ingroup IxEthAcc
 *
 * @def ixEthAccMiiPhyReset
 *
 * @brief : deprecated API entry point. This definition 
 * ensures backward compatibility
 *
 * See @ref ixEthMiiPhyReset
 *
 * @note this feature is board specific
 */
#define ixEthAccMiiPhyReset(phyAddr) \
           ixEthMiiPhyReset(phyAddr)

/**
 * @ingroup IxEthAcc
 *
 * @def ixEthAccMiiLinkStatus
 *
 * @brief : deprecated API entry point. This definition 
 * ensures backward compatibility
 *
 * See @ref ixEthMiiLinkStatus
 *
 * @note this feature is board specific
 */
#define ixEthAccMiiLinkStatus(phyAddr,linkUp,speed100,fullDuplex,autoneg) \
           ixEthMiiLinkStatus(phyAddr,linkUp,speed100,fullDuplex,autoneg)



/**
 * @ingroup IxEthAcc
 *
 * @def ixEthAccMiiShow  
 *
 * @brief : deprecated API entry point. This definition 
 * ensures backward compatibility
 *
 * See @ref ixEthMiiPhyShow
 *
 * @note this feature is board specific
 */
#define ixEthAccMiiShow(phyAddr) \
        ixEthMiiPhyShow(phyAddr)

#endif /* ndef IxEthAcc_H */
/**
 *@}
 */
