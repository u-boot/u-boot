/**
 * @file IxHssAcc.h
 * 
 * @date 07-DEC-2001
 *
 * @brief This file contains the public API of the IXP400 HSS Access
 * component
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
 
/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */
/**
 * @defgroup IxHssAccAPI IXP400 HSS Access (IxHssAcc) API
 *
 * @brief The public API for the IXP400 HssAccess component
 *
 * IxHssAcc is the access layer to the HSS packetised and channelised
 * services
 * 
 * <b> Design Notes </b><br>
 * <UL>
 * <LI>When a packet-pipe is configured for 56Kbps RAW mode, byte alignment of 
 *     the transmitted data is not preserved. All raw data that is transmitted 
 *     will be received in proper order by the receiver, but the first bit of 
 *     the packet may be seen at any offset within a byte; all subsequent bytes 
 *     will have the same offset for the duration of the packet. The same offset 
 *     also applies to all subsequent packets received on the packet-pipe too. 
 *     (Similar results will occur for data received from remote end.) While 
 *     this behavior will also occur for 56Kbps HDLC mode, the HDLC 
 *     encoding/decoding will preserve the original byte alignment at the 
 *     receiver end.
 * </UL>
 *
 * <b> 56Kbps Packetised Service Bandwidth Limitation </b><br>
 * <UL>
 * <LI>IxHssAcc supports 56Kbps packetised service at a maximum aggregate rate
 *     for all HSS ports/HDLC channels of 12.288Mbps[1] in each direction, i.e.
 *     it supports 56Kbps packetised service on up to 8 T1 trunks. It does
 *     not support 56Kbps packetised service on 8 E1 trunks (i.e. 4 trunks per 
 *     HSS port) unless those trunks are running 'fractional E1' with maximum 
 *     aggregate rate of 12.288 Mbps in each direction.<br>
 *     [1] 12.288Mbps = 1.536Mbp * 8 T1
 * </UL>
 * @{ */

#ifndef IXHSSACC_H
#define IXHSSACC_H

#include "IxOsal.h"

/*
 * #defines for function return types, etc.
 */

/**
 * @def IX_HSSACC_TSLOTS_PER_HSS_PORT
 *
 * @brief The max number of TDM timeslots supported per HSS port - 4E1's =
 *  32x4 = 128 
 */
#define IX_HSSACC_TSLOTS_PER_HSS_PORT 128

/* -----------------------------------------------------------
   The following are HssAccess return values returned through 
   service interfaces. The globally defined IX_SUCCESS (0) and
   IX_FAIL (1) in IxOsalTypes.h are also used.
   ----------------------------------------------------------- */
/**
 * @def IX_HSSACC_PARAM_ERR
 *
 * @brief HssAccess function return value for a parameter error
 */
#define IX_HSSACC_PARAM_ERR 2

/**
 * @def IX_HSSACC_RESOURCE_ERR
 *
 * @brief HssAccess function return value for a resource error
 */
#define IX_HSSACC_RESOURCE_ERR 3

/**
 * @def IX_HSSACC_PKT_DISCONNECTING
 *
 * @brief Indicates that a disconnect call is progressing and will 
 * disconnect soon
 */
#define IX_HSSACC_PKT_DISCONNECTING 4

/**
 * @def IX_HSSACC_Q_WRITE_OVERFLOW
 *
 * @brief Indicates that an attempt to Tx or to replenish an 
 * RxFree Q failed due to Q overflow.  
 */
#define IX_HSSACC_Q_WRITE_OVERFLOW 5

/* -------------------------------------------------------------------
   The following errors are HSS/NPE errors returned on error retrieval
   ------------------------------------------------------------------- */
/**
 * @def IX_HSSACC_NO_ERROR
 *
 * @brief HSS port no error present
 */
#define IX_HSSACC_NO_ERROR 0

/**
 * @def IX_HSSACC_TX_FRM_SYNC_ERR
 *
 * @brief HSS port TX Frame Sync error
 */
#define IX_HSSACC_TX_FRM_SYNC_ERR 1

/**
 * @def IX_HSSACC_TX_OVER_RUN_ERR
 *
 * @brief HSS port TX over-run error
 */
#define IX_HSSACC_TX_OVER_RUN_ERR 2

/**
 * @def IX_HSSACC_CHANNELISED_SW_TX_ERR
 *
 * @brief NPE software error in channelised TX
 */
#define IX_HSSACC_CHANNELISED_SW_TX_ERR 3

/**
 * @def IX_HSSACC_PACKETISED_SW_TX_ERR
 *
 * @brief NPE software error in packetised TX
 */
#define IX_HSSACC_PACKETISED_SW_TX_ERR 4

/**
 * @def IX_HSSACC_RX_FRM_SYNC_ERR
 *
 * @brief HSS port RX Frame Sync error
 */
#define IX_HSSACC_RX_FRM_SYNC_ERR 5

/**
 * @def IX_HSSACC_RX_OVER_RUN_ERR
 *
 * @brief HSS port RX over-run error
 */
#define IX_HSSACC_RX_OVER_RUN_ERR 6

/**
 * @def IX_HSSACC_CHANNELISED_SW_RX_ERR
 *
 * @brief NPE software error in channelised RX
 */
#define IX_HSSACC_CHANNELISED_SW_RX_ERR 7

/**
 * @def IX_HSSACC_PACKETISED_SW_RX_ERR
 *
 * @brief NPE software error in packetised TX
 */
#define IX_HSSACC_PACKETISED_SW_RX_ERR 8

/* -----------------------------------
   Packetised service specific defines
   ----------------------------------- */

/**
 * @def IX_HSSACC_PKT_MIN_RX_MBUF_SIZE
 *
 * @brief Minimum size of the Rx mbuf in bytes which the client must supply 
 * to the component. 
 */
#define IX_HSSACC_PKT_MIN_RX_MBUF_SIZE 64

/* --------------------------------------------------------------------
   Enumerated Types - these enumerated values may be used in setting up
   the contents of hardware registers
   -------------------------------------------------------------------- */
/**
 * @enum IxHssAccHssPort
 * @brief The HSS port ID - There are two identical ports (0-1). 
 * 
 */
typedef enum
{
    IX_HSSACC_HSS_PORT_0,   /**< HSS Port 0 */
    IX_HSSACC_HSS_PORT_1,   /**< HSS Port 1 */
    IX_HSSACC_HSS_PORT_MAX  /**< Delimiter for error checks */
} IxHssAccHssPort;

/**
 * @enum IxHssAccHdlcPort
 * @brief The HDLC port ID - There are four identical HDLC ports (0-3) per 
 * HSS port and they correspond to the 4 E1/T1 trunks.
 * 
 */
typedef enum
{
   IX_HSSACC_HDLC_PORT_0,   /**< HDLC Port 0 */
   IX_HSSACC_HDLC_PORT_1,   /**< HDLC Port 1 */
   IX_HSSACC_HDLC_PORT_2,   /**< HDLC Port 2 */
   IX_HSSACC_HDLC_PORT_3,   /**< HDLC Port 3 */
   IX_HSSACC_HDLC_PORT_MAX  /**< Delimiter for error checks */
} IxHssAccHdlcPort;

/**
 * @enum IxHssAccTdmSlotUsage
 * @brief The HSS TDM stream timeslot assignment types
 *
 */
typedef enum
{
    IX_HSSACC_TDMMAP_UNASSIGNED,    /**< Unassigned */
    IX_HSSACC_TDMMAP_HDLC,          /**< HDLC - packetised */
    IX_HSSACC_TDMMAP_VOICE56K,      /**< Voice56K - channelised */
    IX_HSSACC_TDMMAP_VOICE64K,      /**< Voice64K - channelised */
    IX_HSSACC_TDMMAP_MAX            /**< Delimiter for error checks */
} IxHssAccTdmSlotUsage;

/**
 * @enum IxHssAccFrmSyncType
 * @brief The HSS frame sync pulse type
 *
 */
typedef enum
{
    IX_HSSACC_FRM_SYNC_ACTIVE_LOW,   /**< Frame sync is sampled low */
    IX_HSSACC_FRM_SYNC_ACTIVE_HIGH,  /**< sampled high */
    IX_HSSACC_FRM_SYNC_FALLINGEDGE,  /**< sampled on a falling edge */
    IX_HSSACC_FRM_SYNC_RISINGEDGE,   /**< sampled on a rising edge */
    IX_HSSACC_FRM_SYNC_TYPE_MAX      /**< Delimiter for error checks */
} IxHssAccFrmSyncType;

/**
 * @enum IxHssAccFrmSyncEnable
 * @brief The IxHssAccFrmSyncEnable determines how the frame sync pulse is
 * used
 * */
typedef enum
{
    IX_HSSACC_FRM_SYNC_INPUT,          /**< Frame sync is sampled as an input */
    IX_HSSACC_FRM_SYNC_INVALID_VALUE,  /**< 1 is not used */
    IX_HSSACC_FRM_SYNC_OUTPUT_FALLING, /**< Frame sync is an output generated 
					  off a falling clock edge */
    IX_HSSACC_FRM_SYNC_OUTPUT_RISING,  /**< Frame sync is an output generated 
					  off a rising clock edge */
    IX_HSSACC_FRM_SYNC_ENABLE_MAX      /**< Delimiter for error checks */
} IxHssAccFrmSyncEnable;

/**
 * @enum IxHssAccClkEdge
 * @brief IxHssAccClkEdge is used to determine the clk edge to use for 
 * framing and data
 *
 */
typedef enum
{
    IX_HSSACC_CLK_EDGE_FALLING,  /**< Clock sampled off a falling edge */
    IX_HSSACC_CLK_EDGE_RISING,   /**< Clock sampled off a rising edge */
    IX_HSSACC_CLK_EDGE_MAX       /**< Delimiter for error checks */
} IxHssAccClkEdge;

/**
 * @enum IxHssAccClkDir
 * @brief The HSS clock direction
 *
 */
typedef enum
{
    IX_HSSACC_SYNC_CLK_DIR_INPUT,    /**< Clock is an input */
    IX_HSSACC_SYNC_CLK_DIR_OUTPUT,   /**< Clock is an output */
    IX_HSSACC_SYNC_CLK_DIR_MAX       /**< Delimiter for error checks */
} IxHssAccClkDir;

/**
 * @enum IxHssAccFrmPulseUsage
 * @brief The HSS frame pulse usage
 *
 */
typedef enum
{
    IX_HSSACC_FRM_PULSE_ENABLED,     /**< Generate/Receive frame pulses */
    IX_HSSACC_FRM_PULSE_DISABLED,    /**< Disregard frame pulses */
    IX_HSSACC_FRM_PULSE_MAX          /**< Delimiter for error checks */
} IxHssAccFrmPulseUsage;

/**
 * @enum IxHssAccDataRate
 * @brief The HSS Data rate in relation to the clock
 *
 */
typedef enum
{
    IX_HSSACC_CLK_RATE,      /**< Data rate is at the configured clk speed */
    IX_HSSACC_HALF_CLK_RATE, /**< Data rate is half the configured clk speed */
    IX_HSSACC_DATA_RATE_MAX  /**< Delimiter for error checks */
} IxHssAccDataRate;

/**
 * @enum IxHssAccDataPolarity
 * @brief The HSS data polarity type
 *
 */
typedef enum
{
    IX_HSSACC_DATA_POLARITY_SAME,   /**< Don't invert data between NPE and 
				       HSS FIFOs */
    IX_HSSACC_DATA_POLARITY_INVERT, /**< Invert data between NPE and HSS 
				       FIFOs */
    IX_HSSACC_DATA_POLARITY_MAX     /**< Delimiter for error checks */
} IxHssAccDataPolarity;

/**
 * @enum IxHssAccBitEndian
 * @brief HSS Data endianness
 *
 */
typedef enum
{
    IX_HSSACC_LSB_ENDIAN,    /**< TX/RX Least Significant Bit first */
    IX_HSSACC_MSB_ENDIAN,    /**< TX/RX Most Significant Bit first */
    IX_HSSACC_ENDIAN_MAX     /**< Delimiter for the purposes of error checks */
} IxHssAccBitEndian;


/**
 * @enum IxHssAccDrainMode
 * @brief Tx pin open drain mode
 *
 */
typedef enum
{
    IX_HSSACC_TX_PINS_NORMAL,       /**< Normal mode */
    IX_HSSACC_TX_PINS_OPEN_DRAIN,   /**< Open Drain mode */
    IX_HSSACC_TX_PINS_MAX           /**< Delimiter for error checks */
} IxHssAccDrainMode;

/**
 * @enum IxHssAccSOFType
 * @brief HSS start of frame types
 *
 */
typedef enum
{
    IX_HSSACC_SOF_FBIT,  /**< Framing bit transmitted and expected on rx */
    IX_HSSACC_SOF_DATA,  /**< Framing bit not transmitted nor expected on rx */
    IX_HSSACC_SOF_MAX    /**< Delimiter for error checks */
} IxHssAccSOFType;

/**
 * @enum IxHssAccDataEnable
 * @brief IxHssAccDataEnable is used to determine whether or not to drive 
 * the data pins
 *
 */
typedef enum
{
    IX_HSSACC_DE_TRI_STATE,   /**< TRI-State the data pins */
    IX_HSSACC_DE_DATA,        /**< Push data out the data pins */
    IX_HSSACC_DE_MAX          /**< Delimiter for error checks */
} IxHssAccDataEnable;

/**
 * @enum IxHssAccTxSigType
 * @brief IxHssAccTxSigType is used to determine how to drive the data pins
 *
 */
typedef enum
{
    IX_HSSACC_TXSIG_LOW,        /**< Drive the data pins low */
    IX_HSSACC_TXSIG_HIGH,       /**< Drive the data pins high */
    IX_HSSACC_TXSIG_HIGH_IMP,   /**< Drive the data pins with high impedance */
    IX_HSSACC_TXSIG_MAX         /**< Delimiter for error checks */
} IxHssAccTxSigType;

/**
 * @enum IxHssAccFbType
 * @brief IxHssAccFbType determines how to drive the Fbit
 *
 * @warning This will only be used for T1 @ 1.544MHz
 *
 */
typedef enum
{
    IX_HSSACC_FB_FIFO,        /**< Fbit is dictated in FIFO */
    IX_HSSACC_FB_HIGH_IMP,    /**< Fbit is high impedance */
    IX_HSSACC_FB_MAX          /**< Delimiter for error checks */
} IxHssAccFbType;

/**
 * @enum IxHssAcc56kEndianness
 * @brief 56k data endianness when using the 56k type
 *
 */
typedef enum
{
    IX_HSSACC_56KE_BIT_7_UNUSED,  /**< High bit is unused */
    IX_HSSACC_56KE_BIT_0_UNUSED,  /**< Low bit is unused */
    IX_HSSACC_56KE_MAX            /**< Delimiter for error checks */
} IxHssAcc56kEndianness;

/**
 * @enum IxHssAcc56kSel
 * @brief 56k data transmission type when using the 56k type
 *
 */
typedef enum
{
    IX_HSSACC_56KS_32_8_DATA,  /**< 32/8 bit data */
    IX_HSSACC_56KS_56K_DATA,   /**< 56K data */
    IX_HSSACC_56KS_MAX         /**< Delimiter for error checks */
} IxHssAcc56kSel;


/**
 * @enum IxHssAccClkSpeed
 * @brief IxHssAccClkSpeed represents the HSS clock speeds available
 *
 */
typedef enum
{
    IX_HSSACC_CLK_SPEED_512KHZ,     /**< 512KHz */
    IX_HSSACC_CLK_SPEED_1536KHZ,    /**< 1.536MHz */
    IX_HSSACC_CLK_SPEED_1544KHZ,    /**< 1.544MHz */
    IX_HSSACC_CLK_SPEED_2048KHZ,    /**< 2.048MHz */
    IX_HSSACC_CLK_SPEED_4096KHZ,    /**< 4.096MHz */
    IX_HSSACC_CLK_SPEED_8192KHZ,    /**< 8.192MHz */
    IX_HSSACC_CLK_SPEED_MAX      /**< Delimiter for error checking */
} IxHssAccClkSpeed;

/**
 * @enum IxHssAccPktStatus
 * @brief Indicates the status of packets passed to the client
 *
 */
typedef enum
{
    IX_HSSACC_PKT_OK,              /**< Error free.*/
    IX_HSSACC_STOP_SHUTDOWN_ERROR, /**< Errored due to stop or shutdown 
				      occurrance.*/
    IX_HSSACC_HDLC_ALN_ERROR,      /**< HDLC alignment error */
    IX_HSSACC_HDLC_FCS_ERROR,       /**< HDLC Frame Check Sum error.*/
    IX_HSSACC_RXFREE_Q_EMPTY_ERROR,       /**< RxFree Q became empty 
					     while receiving this packet.*/
    IX_HSSACC_HDLC_MAX_FRAME_SIZE_EXCEEDED,      /**< HDLC frame size 
						   received is greater than
						   max specified at connect.*/
    IX_HSSACC_HDLC_ABORT_ERROR,   /**< HDLC frame received is invalid due to an 
				   abort sequence received.*/
    IX_HSSACC_DISCONNECT_IN_PROGRESS     /**< Packet returned
					    because a disconnect is in progress */
} IxHssAccPktStatus;


/**
 * @enum IxHssAccPktCrcType
 * @brief HDLC CRC type
 *
 */
typedef enum
{
    IX_HSSACC_PKT_16_BIT_CRC = 16,  /**< 16 bit CRC is being used */
    IX_HSSACC_PKT_32_BIT_CRC = 32   /**< 32 bit CRC is being used */
} IxHssAccPktCrcType;

/**
 * @enum IxHssAccPktHdlcIdleType
 * @brief HDLC idle transmission type
 *
 */
typedef enum
{
    IX_HSSACC_HDLC_IDLE_ONES,    /**< idle tx/rx will be a succession of ones */
    IX_HSSACC_HDLC_IDLE_FLAGS    /**< idle tx/rx will be repeated flags */
} IxHssAccPktHdlcIdleType;

/**
 * @brief Structure containing HSS port configuration parameters
 *
 * Note: All of these are used for TX. Only some are specific to RX.
 *
 */
typedef struct
{
    IxHssAccFrmSyncType frmSyncType;     /**< frame sync pulse type (tx/rx) */
    IxHssAccFrmSyncEnable frmSyncIO;     /**< how the frame sync pulse is 
					    used (tx/rx) */
    IxHssAccClkEdge frmSyncClkEdge;      /**< frame sync clock edge type 
					    (tx/rx) */
    IxHssAccClkEdge dataClkEdge;         /**< data clock edge type (tx/rx) */
    IxHssAccClkDir clkDirection;         /**< clock direction (tx/rx) */
    IxHssAccFrmPulseUsage frmPulseUsage; /**< whether to use the frame sync 
					    pulse or not (tx/rx) */
    IxHssAccDataRate dataRate;           /**< data rate in relation to the 
					    clock (tx/rx) */
    IxHssAccDataPolarity dataPolarity;   /**< data polarity type (tx/rx) */
    IxHssAccBitEndian dataEndianness;    /**< data endianness (tx/rx) */
    IxHssAccDrainMode drainMode;         /**< tx pin open drain mode (tx) */
    IxHssAccSOFType fBitUsage;           /**< start of frame types (tx/rx) */
    IxHssAccDataEnable dataEnable;       /**< whether or not to drive the data 
					    pins (tx) */
    IxHssAccTxSigType voice56kType;      /**< how to drive the data pins for 
					    voice56k type (tx) */
    IxHssAccTxSigType unassignedType;    /**< how to drive the data pins for 
					    unassigned type (tx) */
    IxHssAccFbType fBitType;             /**< how to drive the Fbit (tx) */
    IxHssAcc56kEndianness voice56kEndian;/**< 56k data endianness when using 
					    the 56k type (tx) */
    IxHssAcc56kSel voice56kSel;          /**< 56k data transmission type when 
					    using the 56k type (tx) */
    unsigned frmOffset;                  /**< frame pulse offset in bits wrt 
					    the first timeslot (0-1023) (tx/rx) */
    unsigned maxFrmSize;                 /**< frame size in bits (1-1024) 
					    (tx/rx) */
} IxHssAccPortConfig;

/**
 * @brief Structure containing HSS configuration parameters
 *
 */
typedef struct
{
    IxHssAccPortConfig txPortConfig; /**< HSS tx port configuration */
    IxHssAccPortConfig rxPortConfig; /**< HSS rx port configuration */
    unsigned numChannelised;         /**< The number of channelised 
					timeslots (0-32) */
    unsigned hssPktChannelCount;     /**< The number of packetised 
					clients (0 - 4) */
    UINT8 channelisedIdlePattern;    /**< The byte to be transmitted on 
					channelised service when there 
					is no client data to tx */
    BOOL loopback;                   /**< The HSS loopback state */
    unsigned packetizedIdlePattern;  /**< The data to be transmitted on 
					packetised service when there is 
					no client data to tx */
    IxHssAccClkSpeed clkSpeed;       /**< The HSS clock speed */
} IxHssAccConfigParams;

/**
 * @brief This structure contains 56Kbps, HDLC-mode configuration parameters
 *
 */
typedef struct
{
    BOOL hdlc56kMode;                    /**< 56kbps(TRUE)/64kbps(FALSE) HDLC */
    IxHssAcc56kEndianness hdlc56kEndian; /**< 56kbps data endianness 
					    - ignored if hdlc56kMode is FALSE*/
    BOOL hdlc56kUnusedBitPolarity0;      /**< The polarity '0'(TRUE)/'1'(FALSE) of the unused
					   bit while in 56kbps mode
					   - ignored if hdlc56kMode is FALSE*/
} IxHssAccHdlcMode;

/**
 * @brief This structure contains information required by the NPE to 
 * configure the HDLC co-processor
 *
 */
typedef struct
{
    IxHssAccPktHdlcIdleType hdlcIdleType;   /**< What to transmit when a HDLC port is idle */
    IxHssAccBitEndian dataEndian;           /**< The HDLC data endianness */
    IxHssAccPktCrcType crcType;             /**< The CRC type to be used for this HDLC port */
} IxHssAccPktHdlcFraming;

/**
 * @typedef UINT32 IxHssAccPktUserId
 *
 * @brief The client supplied value which will be supplied as a parameter
 * with a given callback.
 *
 * This value will be passed into the ixHssAccPktPortConnect function once each
 * with given callbacks.  This value will then be passed back to the client
 * as one of the parameters to each of these callbacks, 
 * when these callbacks are called.
 */
typedef UINT32 IxHssAccPktUserId;


/**
 * @typedef IxHssAccLastErrorCallback
 * @brief Prototype of the clients function to accept notification of the 
 * last error
 *
 * This function is registered through the config. The client will initiate
 * the last error retrieval. The HssAccess component will send a message to
 * the NPE through the NPE Message Handler. When a response to the read is
 * received, the NPE Message Handler will callback the HssAccess component
 * which will execute this function in the same IxNpeMh context. The client
 * will be passed the last error and the related service port (packetised
 * 0-3, channelised 0)
 *
 * @param lastHssError unsigned [in] - The last Hss error registered that
 *                                      has been registered.
 * @param servicePort unsigned [in] - This is the service port number.  
 *                                     (packetised 0-3, channelised 0) 
 * 
 * @return void
 */
typedef void (*IxHssAccLastErrorCallback) (unsigned lastHssError, 
					   unsigned servicePort);

/**
 * @typedef IxHssAccPktRxCallback
 * @brief  Prototype of the clients function to accept notification of 
 * packetised rx
 *
 * This function is registered through the ixHssAccPktPortConnect. hssPktAcc will pass
 * received data in the form of mbufs to the client.  The mbuf passed back
 * to the client could contain a chain of buffers, depending on the packet
 * size received. 
 * 
 * @param *buffer @ref IX_OSAL_MBUF [in] - This is the mbuf which contains the 
 * payload received.
 * @param numHssErrs unsigned [in] - This is the number of hssErrors 
 * the Npe has received
 * @param pktStatus @ref IxHssAccPktStatus [in] - This is the status of the 
 * mbuf that has been received.
 * @param rxUserId @ref IxHssAccPktUserId [in] - This is the client supplied value 
 * passed in at ixHssAccPktPortConnect time which is now returned to the client.			       
 * 
 * @return void
 */
typedef void (*IxHssAccPktRxCallback) (IX_OSAL_MBUF *buffer, 
				       unsigned numHssErrs, 
				       IxHssAccPktStatus pktStatus, 
				       IxHssAccPktUserId rxUserId);

/**
 * @typedef IxHssAccPktRxFreeLowCallback
 * @brief Prototype of the clients function to accept notification of 
 * requirement of more Rx Free buffers
 *
 * The client can choose to register a callback of this type when
 * calling a connecting. This function is registered through the ixHssAccPktPortConnect. 
 * If defined, the access layer will provide the trigger for
 * this callback. The callback will be responsible for supplying mbufs to
 * the access layer for use on the receive path from the HSS using
 * ixHssPktAccFreeBufReplenish. 
 *
 * @return void
 */
typedef void (*IxHssAccPktRxFreeLowCallback) (IxHssAccPktUserId rxFreeLowUserId);

/**
 * @typedef IxHssAccPktTxDoneCallback
 * @brief  Prototype of the clients function to accept notification of 
 * completion with Tx buffers
 *
 * This function is registered through the ixHssAccPktPortConnect.  It enables
 * the hssPktAcc to pass buffers back to the client
 * when transmission is complete.
 *
 * @param *buffer @ref IX_OSAL_MBUF [in] - This is the mbuf which contained 
 * the payload that was for Tx.
 * @param numHssErrs unsigned [in] - This is the number of hssErrors 
 * the Npe has received
 * @param pktStatus @ref IxHssAccPktStatus [in] - This is the status of the 
 * mbuf that has been transmitted.
 * @param txDoneUserId @ref IxHssAccPktUserId [in] - This is the client supplied value 
 * passed in at ixHssAccPktPortConnect time which is now returned to the client.	 
 *
 * @return void
 */
typedef void (*IxHssAccPktTxDoneCallback) (IX_OSAL_MBUF *buffer, 
					   unsigned numHssErrs,
					   IxHssAccPktStatus pktStatus, 
					   IxHssAccPktUserId txDoneUserId);

/**
 * @typedef IxHssAccChanRxCallback
 * @brief Prototype of the clients function to accept notification of 
 * channelised rx
 *
 * This callback, if defined by the client in the connect, will get called
 * in the context of an IRQ. The IRQ will be triggered when the hssSyncQMQ
 * is not empty. The queued entry will be dequeued and this function will
 * be executed.
 *
 * @param hssPortId @ref IxHssAccHssPort - The HSS port Id. There are two
 * identical ports (0-1).
 * @param txOffset unsigned [in] - an offset indicating from where within
 * the txPtrList the NPE is currently transmitting from.
 * @param rxOffset unsigned [in] - an offset indicating where within the
 * receive buffers the NPE has just written the received data to.
 * @param numHssErrs unsigned [in] - This is the number of hssErrors 
 * the Npe has received
 *
 * @return void
 */
typedef void (*IxHssAccChanRxCallback) (IxHssAccHssPort hssPortId,
					unsigned rxOffset, 
					unsigned txOffset, 
					unsigned numHssErrs);

/*
 * Prototypes for interface functions.
 */

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccPortInit (IxHssAccHssPort hssPortId, 
               IxHssAccConfigParams *configParams, 
               IxHssAccTdmSlotUsage *tdmMap, 
               IxHssAccLastErrorCallback lastHssErrorCallback)
 *
 * @brief Initialise a HSS port. No channelised or packetised connections
 * should exist in the HssAccess layer while this interface is being called.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param *configParams @ref IxHssAccConfigParams [in] - A pointer to the HSS 
 * configuration structure
 * @param *tdmMap @ref IxHssAccTdmSlotUsage [in] - A pointer to an array of size
 * IX_HSSACC_TSLOTS_PER_HSS_PORT, defining the slot usage over the HSS port
 * @param lastHssErrorCallback @ref IxHssAccLastErrorCallback [in] - Client 
 * callback to report last error
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccPortInit (IxHssAccHssPort hssPortId, 
		  IxHssAccConfigParams *configParams, 
		  IxHssAccTdmSlotUsage *tdmMap, 
		  IxHssAccLastErrorCallback lastHssErrorCallback);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccLastErrorRetrievalInitiate (
               IxHssAccHssPort hssPortId)
 *
 * @brief Initiate the retrieval of the last HSS error. The HSS port
 * should be configured before attempting to call this interface.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - the HSS port ID
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccLastErrorRetrievalInitiate (IxHssAccHssPort hssPortId);


/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccInit ()
 *
 * @brief This function is responsible for initialising resources for use
 * by the packetised and channelised clients. It should be called after 
 * HSS NPE image has been downloaded into NPE-A and before any other
 * HssAccess interface is called. 
 * No other HssAccPacketised interface should be called while this interface
 * is being processed.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error 
 */
PUBLIC IX_STATUS 
ixHssAccInit (void);


/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn ixHssAccPktPortConnect (IxHssAccHssPort hssPortId, 
			IxHssAccHdlcPort hdlcPortId, 
			BOOL hdlcFraming, 
			IxHssAccHdlcMode hdlcMode,
			BOOL hdlcBitInvert,
			unsigned blockSizeInWords,
			UINT32 rawIdleBlockPattern,
			IxHssAccPktHdlcFraming hdlcTxFraming, 
			IxHssAccPktHdlcFraming hdlcRxFraming, 
			unsigned frmFlagStart, 
			IxHssAccPktRxCallback rxCallback,
			IxHssAccPktUserId rxUserId, 
			IxHssAccPktRxFreeLowCallback rxFreeLowCallback, 
			IxHssAccPktUserId rxFreeLowUserId,
			IxHssAccPktTxDoneCallback txDoneCallback,
			IxHssAccPktUserId txDoneUserId) 
 *
 * @brief This function is responsible for connecting a client to one of 
 * the 4 available HDLC ports. The HSS port should be configured before 
 * attempting a connect. No other HssAccPacketised interface should be
 * called while this connect is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - This is the number of the HDLC port and 
 * it corresponds to the physical E1/T1 trunk i.e. 0, 1, 2, 3 
 * @param hdlcFraming BOOL [in] - This value determines whether the service 
 * will use HDLC data or the debug, raw data type i.e. no HDLC processing
 * @param hdlcMode @ref IxHssAccHdlcMode [in] - This structure contains 56Kbps, HDLC-mode
 * configuration parameters
 * @param hdlcBitInvert BOOL [in] - This value determines whether bit inversion
 * will occur between HDLC and HSS co-processors i.e. post-HDLC processing for
 * transmit and pre-HDLC processing for receive, for the specified HDLC Termination
 * Point
 * @param blockSizeInWords unsigned [in] -  The max tx/rx block size 
 * @param rawIdleBlockPattern UINT32 [in] -  Tx idle pattern in raw mode 
 * @param hdlcTxFraming @ref IxHssAccPktHdlcFraming [in] - This structure contains 
 * the following information required by the NPE to configure the HDLC 
 * co-processor for TX
 * @param hdlcRxFraming @ref IxHssAccPktHdlcFraming [in] -  This structure contains 
 * the following information required by the NPE to configure the HDLC 
 * co-processor for RX
 * @param frmFlagStart unsigned - Number of flags to precede to 
 * transmitted flags (0-2).
 * @param rxCallback @ref IxHssAccPktRxCallback [in] - Pointer to 
 * the clients packet receive function.
 * @param rxUserId @ref IxHssAccPktUserId [in] - The client supplied rx value
 * to be passed back as an argument to the supplied rxCallback
 * @param rxFreeLowCallback @ref IxHssAccPktRxFreeLowCallback [in] - Pointer to 
 * the clients Rx free buffer request function.  If NULL, assume client will 
 * trigger independently.
 * @param rxFreeLowUserId @ref IxHssAccPktUserId [in] - The client supplied RxFreeLow value
 * to be passed back as an argument to the supplied rxFreeLowCallback
 * @param txDoneCallback @ref IxHssAccPktTxDoneCallback [in] - Pointer to the 
 * clients Tx done callback function
 * @param txDoneUserId @ref IxHssAccPktUserId [in] - The client supplied txDone value
 * to be passed back as an argument to the supplied txDoneCallback
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error
 */
PUBLIC IX_STATUS 
ixHssAccPktPortConnect (IxHssAccHssPort hssPortId, 
			IxHssAccHdlcPort hdlcPortId, 
			BOOL hdlcFraming, 
			IxHssAccHdlcMode hdlcMode,
			BOOL hdlcBitInvert,
			unsigned blockSizeInWords,
			UINT32 rawIdleBlockPattern,
			IxHssAccPktHdlcFraming hdlcTxFraming, 
			IxHssAccPktHdlcFraming hdlcRxFraming, 
			unsigned frmFlagStart, 
			IxHssAccPktRxCallback rxCallback,
			IxHssAccPktUserId rxUserId, 
			IxHssAccPktRxFreeLowCallback rxFreeLowCallback, 
			IxHssAccPktUserId rxFreeLowUserId,
			IxHssAccPktTxDoneCallback txDoneCallback,
			IxHssAccPktUserId txDoneUserId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccPktPortEnable (IxHssAccHssPort hssPortId, 
           IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This function is responsible for enabling a packetised service
 * for the specified HSS/HDLC port combination. It enables the RX flow. The
 * client must have already connected to a packetised service and is responsible 
 * for ensuring an adequate amount of RX mbufs have been supplied to the access
 * component before enabling the packetised service. This function must be called
 * on a given port before any call to ixHssAccPktPortTx on the same port. 
 * No other HssAccPacketised interface should be called while this interface is 
 * being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1).   
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - The port id (0,1,2,3) to enable the service
 * on.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccPktPortEnable (IxHssAccHssPort hssPortId, 
		       IxHssAccHdlcPort hdlcPortId);

/**
 * @fn IX_STATUS ixHssAccPktPortDisable (IxHssAccHssPort hssPortId, 
           IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This function is responsible for disabling a packetised service
 * for the specified HSS/HDLC port combination. It disables the RX flow. 
 * The client must have already connected to and enabled a packetised service 
 * for the specified HDLC port. This disable interface can be called before a
 * disconnect, but is not required to.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - The port id (0,1,2,3) to disable 
 * the service on.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccPktPortDisable (IxHssAccHssPort hssPortId, 
			IxHssAccHdlcPort hdlcPortId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccPktPortDisconnect (IxHssAccHssPort hssPortId, 
           IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This function is responsible for disconnecting a client from one
 * of the 4 available HDLC ports. It is not required that the Rx Flow 
 * has been disabled before calling this function.  If the RX Flow has not been
 * disabled, the disconnect will disable it before proceeding with the
 * disconnect.  No other HssAccPacketised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - This is the number of the HDLC port
 * to disconnect and it corresponds to the physical E1/T1 trunk i.e. 0, 1, 2, 3
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully 
 *         - IX_HSSACC_PKT_DISCONNECTING The function has initiated the disconnecting
 *                             procedure but it has not completed yet.
 */
PUBLIC IX_STATUS 
ixHssAccPktPortDisconnect (IxHssAccHssPort hssPortId, 
			   IxHssAccHdlcPort hdlcPortId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn BOOL ixHssAccPktPortIsDisconnectComplete (IxHssAccHssPort hssPortId, 
           IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This function is called to check if a given HSS/HDLC port 
 * combination is in a connected state or not. This function may be called 
 * at any time to determine a ports state.  No other HssAccPacketised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - This is the number of the HDLC port
 * to disconnect and it corresponds to the physical E1/T1 trunk i.e. 0, 1, 2, 3
 *
 * @return 
 *         - TRUE The state of this HSS/HDLC port combination is disconnected,
 *                so if a disconnect was called, it is now completed.
 *         - FALSE The state of this HSS/HDLC port combination is connected,
 *                so if a disconnect was called, it is not yet completed.
 */
PUBLIC BOOL 
ixHssAccPktPortIsDisconnectComplete (IxHssAccHssPort hssPortId, 
				     IxHssAccHdlcPort hdlcPortId);


/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccPktPortRxFreeReplenish (IxHssAccHssPort hssPortId, 
              IxHssAccHdlcPort hdlcPortId, 
	      IX_OSAL_MBUF *buffer)
 *
 * @brief Function which the client calls at regular intervals to provide
 * mbufs to the access component for RX. A connection should exist for
 * the specified hssPortId/hdlcPortId combination before attempting to call this 
 * interface. Also, the connection should not be in a disconnecting state.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - This is the number of the HDLC port
 * and it corresponds to the physical E1/T1 trunk i.e. 0, 1, 2, 3
 * @param *buffer @ref IX_OSAL_MBUF [in] - A pointer to a free mbuf to filled with payload.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error
 *         - IX_HSSACC_Q_WRITE_OVERFLOW The function did not succeed due to a Q
 *                                      overflow
 */
PUBLIC IX_STATUS 
ixHssAccPktPortRxFreeReplenish (IxHssAccHssPort hssPortId, 
				IxHssAccHdlcPort hdlcPortId, 
				IX_OSAL_MBUF *buffer);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccPktPortTx (IxHssAccHssPort hssPortId, 
    IxHssAccHdlcPort hdlcPortId, 
    IX_OSAL_MBUF *buffer)
 *
 * @brief Function which the client calls when it wants to transmit
 * packetised data. An enabled connection should exist on the specified
 * hssPortId/hdlcPortId combination before attempting to call this interface.
 * No other HssAccPacketised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param hdlcPortId @ref IxHssAccHdlcPort [in] - This is the number of the HDLC port
 * and it corresponds to the physical E1/T1 trunk i.e. 0, 1, 2, 3
 * @param *buffer @ref IX_OSAL_MBUF [in] - A pointer to a chain of mbufs which the
 * client has filled with the payload
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error. See note.
 *         - IX_HSSACC_Q_WRITE_OVERFLOW The function did not succeed due to a Q
 *                                      overflow
 *
 * @note IX_HSSACC_RESOURCE_ERR is returned when a free descriptor cannot be
 * obtained to send the chain of mbufs to the NPE.  This is a normal scenario.
 * HssAcc has a pool of descriptors and this error means that they are currently
 * all in use.
 * The recommended approach to this is to retry until a descriptor becomes free
 * and the packet is successfully transmitted.
 * Alternatively, the user could wait until the next IxHssAccPktTxDoneCallback
 * callback is triggered, and then retry, as it is this event that causes a
 * transmit descriptor to be freed. 
 */
PUBLIC IX_STATUS 
ixHssAccPktPortTx (IxHssAccHssPort hssPortId, 
		   IxHssAccHdlcPort hdlcPortId, 
		   IX_OSAL_MBUF *buffer);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccChanConnect (IxHssAccHssPort hssPortId, 
           unsigned bytesPerTSTrigger, 
	   UINT8 *rxCircular, 
	   unsigned numRxBytesPerTS, 
	   UINT32 *txPtrList, 
	   unsigned numTxPtrLists, 
	   unsigned numTxBytesPerBlk, 
	   IxHssAccChanRxCallback rxCallback)
 *
 * @brief This function allows the client to connect to the Tx/Rx NPE
 * Channelised Service. There can only be one client per HSS port. The
 * client is responsible for ensuring that the HSS port is configured
 * appropriately before its connect request. No other HssAccChannelised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param bytesPerTSTrigger unsigned [in] - The NPE will trigger the access
 * component after bytesPerTSTrigger have been received for all trunk
 * timeslots. This figure is a multiple of 8 e.g. 8 for 1ms trigger, 16 for
 * 2ms trigger.
 * @param *rxCircular UINT8 [in] - A pointer to memory allocated by the
 * client to be filled by data received. The buffer at this address is part
 * of a pool of buffers to be accessed in a circular fashion. This address
 * will be written to by the NPE. Therefore, it needs to be a physical address.
 * @param numRxBytesPerTS unsigned [in] - The number of bytes allocated per
 * timeslot within the receive memory. This figure will depend on the
 * latency of the system. It needs to be deep enough for data to be read by
 * the client before the NPE re-writes over that memory e.g. if the client
 * samples at a rate of 40bytes per timeslot, numRxBytesPerTS may need to
 * be 40bytes * 3. This would give the client 3 * 5ms of time before
 * received data is over-written.
 * @param *txPtrList UINT32 [in] - The address of an area of contiguous
 * memory allocated by the client to be populated with pointers to data for
 * transmission. Each pointer list contains a pointer per active channel.
 * The txPtrs will point to data to be transmitted by the NPE. Therefore,
 * they must point to physical addresses.
 * @param numTxPtrLists unsigned [in] - The number of pointer lists in
 * txPtrList. This figure is dependent on jitter.
 * @param numTxBytesPerBlk unsigned [in] - The size of the Tx data, in
 * bytes, that each pointer within the PtrList points to.
 * @param rxCallback @ref IxHssAccChanRxCallback [in] - A client function
 * pointer to be called back to handle the actual tx/rx of channelised
 * data. If this is not NULL, an ISR will call this function. If this
 * pointer is NULL, it implies that the client will use a polling mechanism
 * to detect when the tx and rx of channelised data is to occur. The client
 * will use hssChanAccStatus for this.
 *
 * @return
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
 
PUBLIC IX_STATUS 
ixHssAccChanConnect (IxHssAccHssPort hssPortId, 
		     unsigned bytesPerTSTrigger, 
		     UINT8 *rxCircular, 
		     unsigned numRxBytesPerTS, 
		     UINT32 *txPtrList, 
		     unsigned numTxPtrLists, 
		     unsigned numTxBytesPerBlk, 
		     IxHssAccChanRxCallback rxCallback);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccChanPortEnable (IxHssAccHssPort hssPortId)
 *
 * @brief This function is responsible for enabling a channelised service
 * for the specified HSS port. It enables the NPE RX flow. The client must
 * have already connected to a channelised service before enabling the
 * channelised service. No other HssAccChannelised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 *
 * @return
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccChanPortEnable (IxHssAccHssPort hssPortId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccChanPortDisable (IxHssAccHssPort hssPortId)
 *
 * @brief This function is responsible for disabling a channelised service
 * for the specified HSS port. It disables the NPE RX flow. The client must
 * have already connected to and enabled a channelised service for the
 * specified HSS port. This disable interface can be called before a
 * disconnect, but is not required to. No other HssAccChannelised 
 * interface should be called while this interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccChanPortDisable (IxHssAccHssPort hssPortId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccChanDisconnect (IxHssAccHssPort hssPortId)
 *
 * @brief This function allows the client to Disconnect from a channelised
 * service. If the NPE RX Flow has not been disabled, the disconnect will
 * disable it before proceeding with other disconnect functionality.
 * No other HssAccChannelised interface should be called while this 
 * interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccChanDisconnect (IxHssAccHssPort hssPortId);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn IX_STATUS ixHssAccChanStatusQuery (IxHssAccHssPort hssPortId, 
           BOOL *dataRecvd, 
	   unsigned *rxOffset, 
	   unsigned *txOffset, 
	   unsigned *numHssErrs)
 *
 * @brief This function is called by the client to query whether or not
 * channelised data has been received. If there is, hssChanAcc will return
 * the details in the output parameters. An enabled connection should
 * exist on the specified hssPortId before attempting to call this interface.  
 * No other HssAccChannelised interface should be called while this 
 * interface is being processed.
 *
 * @param hssPortId @ref IxHssAccHssPort [in] - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param *dataRecvd BOOL [out] - This BOOL indicates to the client whether
 * or not the access component has read any data for the client. If
 * FALSE, the other output parameters will not have been written to.
 * @param *rxOffset unsigned [out] - An offset to indicate to the client
 * where within the receive buffers the NPE has just written the received
 * data to.
 * @param *txOffset unsigned [out] - An offset to indicate to the client
 * from where within the txPtrList the NPE is currently transmitting from
 * @param *numHssErrs unsigned [out] - The total number of HSS port errors
 * since initial port configuration
 *
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
PUBLIC IX_STATUS 
ixHssAccChanStatusQuery (IxHssAccHssPort hssPortId, 
			 BOOL *dataRecvd, 
			 unsigned *rxOffset, 
			 unsigned *txOffset, 
			 unsigned *numHssErrs);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn void ixHssAccShow (void)
 *
 * @brief This function will display the current state of the IxHssAcc
 * component. The output is sent to stdout.
 *
 * @return void
 */
PUBLIC void 
ixHssAccShow (void);

/**
 *
 * @ingroup IxHssAccAPI
 *
 * @fn void ixHssAccStatsInit (void)
 *
 * @brief This function will reset the IxHssAcc statistics.
 *
 * @return void
 */
PUBLIC void 
ixHssAccStatsInit (void);

#endif /* IXHSSACC_H */

/**
 * @} defgroup IxHssAcc
 */
