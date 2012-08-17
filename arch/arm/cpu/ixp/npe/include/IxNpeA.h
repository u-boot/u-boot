#ifndef __doxygen_HIDE  /* This file is not part of the API */

/**
 * @file    IxNpeA.h
 *
 * @date    22-Mar-2002
 *
 * @brief   Header file for the IXP400 ATM NPE API
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

/**
 * @defgroup IxNpeA IXP400 NPE-A (IxNpeA) API
 *
 * @brief The Public API for the IXP400 NPE-A
 *
 * @{
 */

#ifndef IX_NPE_A_H
#define IX_NPE_A_H

#include "IxQMgr.h"
#include "IxOsal.h"
#include "IxQueueAssignments.h"

/* General Message Ids */

/* ATM Message Ids */

/**
 * @def IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_WRITE
 *
 * @brief ATM Message ID command to write the data to the offset in the
 * Utopia Configuration Table
 */
#define IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_WRITE       0x20

/**
 * @def IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_LOAD
 *
 * @brief ATM Message ID command triggers the NPE to copy the Utopia
 * Configuration Table to the Utopia coprocessor
 */
#define IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_LOAD        0x21

/**
 * @def IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_UPLOAD
 *
 * @brief ATM Message ID command triggers the NPE to read-back the Utopia
 * status registers and update the Utopia Status Table.
 */
#define IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_UPLOAD      0x22

/**
 * @def IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_READ
 *
 * @brief ATM Message ID command to read the Utopia Status Table at the
 * specified offset.
 */
#define IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_READ        0x23

/**
 * @def IX_NPE_A_MSSG_ATM_TX_ENABLE
 *
 * @brief ATM Message ID command triggers the NPE to re-enable processing
 * of any entries on the TxVcQ for this port.
 *
 * This command will be ignored for a port already enabled
 */
#define IX_NPE_A_MSSG_ATM_TX_ENABLE                 0x25

 /**
 * @def IX_NPE_A_MSSG_ATM_TX_DISABLE
 *
 * @brief ATM Message ID command triggers the NPE to disable processing on
 * this port
 *
 * This command will be ignored for a port already disabled
 */
#define IX_NPE_A_MSSG_ATM_TX_DISABLE                0x26

/**
 * @def IX_NPE_A_MSSG_ATM_RX_ENABLE
 *
 * @brief ATM Message ID command triggers the NPE to process any received
 * cells for this VC according to the VC Lookup Table.
 *
 * Re-issuing this command with different contents for a VC that is not
 * disabled will cause unpredictable behavior.
 */
#define IX_NPE_A_MSSG_ATM_RX_ENABLE                 0x27

/**
 * @def IX_NPE_A_MSSG_ATM_RX_DISABLE
 *
 * @brief ATM Message ID command triggers the NPE to disable processing for
 * this VC.
 *
 * This command will be ignored for a VC already disabled
 */
#define IX_NPE_A_MSSG_ATM_RX_DISABLE                0x28

/**
 * @def IX_NPE_A_MSSG_ATM_STATUS_READ
 *
 * @brief ATM Message ID command to read the ATM status. The data is returned via
 * a response message
 */
#define IX_NPE_A_MSSG_ATM_STATUS_READ               0x29

/*--------------------------------------------------------------------------
 * HSS Message IDs
 *--------------------------------------------------------------------------*/

/**
 * @def IX_NPE_A_MSSG_HSS_PORT_CONFIG_WRITE
 *
 * @brief HSS Message ID command writes the ConfigWord value to the location
 * in the HSS_CONFIG_TABLE specified by offset for HSS port hPort.
 */
#define IX_NPE_A_MSSG_HSS_PORT_CONFIG_WRITE         0x40

/**
 * @def IX_NPE_A_MSSG_HSS_PORT_CONFIG_LOAD
 *
 * @brief HSS Message ID command triggers the NPE to copy the contents of the
 * HSS Configuration Table to the appropriate configuration registers in the
 * HSS coprocessor for the port specified by hPort.
 */
#define IX_NPE_A_MSSG_HSS_PORT_CONFIG_LOAD          0x41

/**
 * @def IX_NPE_A_MSSG_HSS_PORT_ERROR_READ
 *
 * @brief HSS Message ID command triggers the NPE to return an HssErrorReadResponse
 * message for HSS port hPort.
 */
#define IX_NPE_A_MSSG_HSS_PORT_ERROR_READ           0x42

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_FLOW_ENABLE
 *
 * @brief HSS Message ID command triggers the NPE to reset internal status and
 * enable the HssChannelized operation on the HSS port specified by hPort.
 */
#define IX_NPE_A_MSSG_HSS_CHAN_FLOW_ENABLE          0x43

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_FLOW_DISABLE
 *
 * @brief HSS Message ID command triggers the NPE to disable the HssChannelized
 * operation on the HSS port specified by hPort.
 */
#define IX_NPE_A_MSSG_HSS_CHAN_FLOW_DISABLE         0x44

/**
 * @def  IX_NPE_A_MSSG_HSS_CHAN_IDLE_PATTERN_WRITE
 *
 * @brief HSS Message ID command writes the HSSnC_IDLE_PATTERN value for HSS
 * port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_IDLE_PATTERN_WRITE   0x45

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_NUM_CHANS_WRITE
 *
 * @brief HSS Message ID command writes the HSSnC_NUM_CHANNELS value for HSS
 * port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_NUM_CHANS_WRITE      0x46

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_ADDR_WRITE
 *
 * @brief HSS Message ID command writes the HSSnC_RX_BUF_ADDR value for HSS
 * port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_ADDR_WRITE    0x47

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_CFG_WRITE
 *
 * @brief HSS Message ID command  writes the HSSnC_RX_BUF_SIZEB and
 * HSSnC_RX_TRIG_PERIOD values for HSS port hPort.  (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_CFG_WRITE     0x48

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_TX_BLK_CFG_WRITE
 *
 * @brief HSS Message ID command writes the HSSnC_TX_BLK1_SIZEB,
 * HSSnC_TX_BLK1_SIZEW, HSSnC_TX_BLK2_SIZEB, and HSSnC_TX_BLK2_SIZEW  values
 * for HSS port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_TX_BLK_CFG_WRITE     0x49

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_ADDR_WRITE
 * @brief HSS Message ID command writes the HSSnC_TX_BUF_ADDR value for HSS
 * port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_ADDR_WRITE    0x4A

/**
 * @def IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_SIZE_WRITE
 *
 * @brief HSS Message ID command writes the HSSnC_TX_BUF_SIZEN value for HSS
 * port hPort. (n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_SIZE_WRITE    0x4B

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_ENABLE
 *
 * @brief HSS Message ID command triggers the NPE to reset internal status and
 * enable the HssPacketized operation for the flow specified by pPipe on
 * the HSS port specified by hPort.
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_ENABLE      0x50

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_DISABLE
 * @brief HSS Message ID command triggers the NPE to disable the HssPacketized
 * operation for the flow specified by pPipe on the HSS port specified by hPort.
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_DISABLE     0x51

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_NUM_PIPES_WRITE
 * @brief HSS Message ID command writes the HSSnP_NUM_PIPES value for HSS
 * port hPort.(n=hPort)
 */
#define IX_NPE_A_MSSG_HSS_PKT_NUM_PIPES_WRITE       0x52

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_FIFO_SIZEW_WRITE
 *
 * @brief HSS Message ID command writes the HSSnP_PIPEp_FIFOSIZEW value for
 * packet-pipe pPipe on HSS port hPort.  (n=hPort, p=pPipe)
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_FIFO_SIZEW_WRITE 0x53

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_HDLC_CFG_WRITE
 *
 * @brief HSS Message ID command writes the HSSnP_PIPEp_HDLC_RXCFG and
 * HSSnP_PIPEp_HDLC_TXCFG values for packet-pipe pPipe on HSS port hPort.
 * (n=hPort, p=pPipe)
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_HDLC_CFG_WRITE   0x54

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE
 *
 * @brief HSS Message ID command writes the HSSnP_PIPEp_IDLE_PATTERN value
 * for packet-pipe pPipe on HSS port hPort.  (n=hPort, p=pPipe)
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE 0x55

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_RX_SIZE_WRITE
 *
 * @brief HSS Message ID command writes the HSSnP_PIPEp_RXSIZEB value for
 * packet-pipe pPipe on HSS port hPort.  (n=hPort, p=pPipe)
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_RX_SIZE_WRITE    0x56

/**
 * @def IX_NPE_A_MSSG_HSS_PKT_PIPE_MODE_WRITE
 *
 * @brief HSS Message ID command writes the HSSnP_PIPEp_MODE value for
 * packet-pipe pPipe on HSS port hPort.  (n=hPort, p=pPipe)
 */
#define IX_NPE_A_MSSG_HSS_PKT_PIPE_MODE_WRITE       0x57



/* Queue Entry Masks */

/*--------------------------------------------------------------------------
 *  ATM Descriptor Structure offsets
 *--------------------------------------------------------------------------*/

/**
 * @def IX_NPE_A_RXDESCRIPTOR_STATUS_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Status field
 *
 * It is used for descriptor error reporting.
 */
#define IX_NPE_A_RXDESCRIPTOR_STATUS_OFFSET          0

/**
 * @def IX_NPE_A_RXDESCRIPTOR_VCID_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor VC ID field
 *
 * It is used to hold an identifier number for this VC
 */
#define IX_NPE_A_RXDESCRIPTOR_VCID_OFFSET            1

/**
 * @def IX_NPE_A_RXDESCRIPTOR_CURRMBUFSIZE_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Current Mbuf
 * Size field
 *
 * Number of bytes the current mbuf data buffer can hold
 */
#define IX_NPE_A_RXDESCRIPTOR_CURRMBUFSIZE_OFFSET    2

/**
 * @def IX_NPE_A_RXDESCRIPTOR_ATMHEADER_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor ATM Header
 */
#define IX_NPE_A_RXDESCRIPTOR_ATMHEADER_OFFSET       4

/**
 * @def IX_NPE_A_RXDESCRIPTOR_CURRMBUFLEN_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Current MBuf length
 *
 *
 * RX - Initialized to zero.  The NPE updates this field as each cell is received and
 * zeroes it with every new mbuf for chaining. Will not be bigger than currBbufSize.
 */
#define IX_NPE_A_RXDESCRIPTOR_CURRMBUFLEN_OFFSET    12

/**
 * @def IX_NPE_A_RXDESCRIPTOR_TIMELIMIT__OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Time Limit field
 *
 * Contains the Payload Reassembly Time Limit (used for aal0_xx only)
 */
#define IX_NPE_A_RXDESCRIPTOR_TIMELIMIT_OFFSET        14

/**
 * @def IX_NPE_A_RXDESCRIPTOR_PCURRMBUFF_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Current MBuf Pointer
 *
 * The current mbuf pointer of a chain of mbufs.
 */
#define IX_NPE_A_RXDESCRIPTOR_PCURRMBUFF_OFFSET     20

/**
 * @def IX_NPE_A_RXDESCRIPTOR_PCURRMBUFDATA_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Current MBuf Pointer
 *
 * Pointer to the next byte to be read or next free location to be written.
 */
#define IX_NPE_A_RXDESCRIPTOR_PCURRMBUFDATA_OFFSET  24

/**
 * @def IX_NPE_A_RXDESCRIPTOR_PNEXTMBUF_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Next MBuf Pointer
 *
 * Pointer to the next MBuf in a chain of MBufs.
 */
#define IX_NPE_A_RXDESCRIPTOR_PNEXTMBUF_OFFSET      28

/**
 * @def IX_NPE_A_RXDESCRIPTOR_TOTALLENGTH_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Total Length
 *
 * Total number of bytes written to the chain of MBufs by the NPE
 */
#define IX_NPE_A_RXDESCRIPTOR_TOTALLENGTH_OFFSET    32

/**
 * @def IX_NPE_A_RXDESCRIPTOR_AAL5CRCRESIDUE_OFFSET
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor AAL5 CRC Residue
 *
 * Current CRC value for a PDU
 */
#define IX_NPE_A_RXDESCRIPTOR_AAL5CRCRESIDUE_OFFSET 36

/**
 * @def IX_NPE_A_RXDESCRIPTOR_SIZE
 *
 * @brief ATM Descriptor structure offset for Receive Descriptor Size
 *
 * The size of the Receive descriptor
 */
#define IX_NPE_A_RXDESCRIPTOR_SIZE                  40

/**
 * @def IX_NPE_A_TXDESCRIPTOR_PORT_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Port
 *
 * Port identifier.
 */
#define IX_NPE_A_TXDESCRIPTOR_PORT_OFFSET            0

/**
 * @def IX_NPE_A_TXDESCRIPTOR_RSVD_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor RSVD
 */
#define IX_NPE_A_TXDESCRIPTOR_RSVD_OFFSET            1

/**
 * @def IX_NPE_A_TXDESCRIPTOR_CURRMBUFLEN_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Current MBuf Length
 *
 * TX - Initialized by the XScale to the number of bytes in the current MBuf data buffer.
 * The NPE decrements this field for every transmitted cell.  Thus, when the NPE writes a
 * descriptor the TxDone queue, this field will equal zero.
 */
#define IX_NPE_A_TXDESCRIPTOR_CURRMBUFLEN_OFFSET     2

/**
 * @def IX_NPE_A_TXDESCRIPTOR_ATMHEADER_OFFSET
 * @brief ATM Descriptor structure offset for Transmit Descriptor ATM Header
 */
#define IX_NPE_A_TXDESCRIPTOR_ATMHEADER_OFFSET       4

/**
 * @def IX_NPE_A_TXDESCRIPTOR_PCURRMBUFF_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Pointer to the current MBuf chain
 */
#define IX_NPE_A_TXDESCRIPTOR_PCURRMBUFF_OFFSET      8

/**
 * @def IX_NPE_A_TXDESCRIPTOR_PCURRMBUFDATA_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Pointer to the current MBuf Data
 *
 * Pointer to the next byte to be read or next free location to be written.
 */
#define IX_NPE_A_TXDESCRIPTOR_PCURRMBUFDATA_OFFSET  12

/**
 * @def IX_NPE_A_TXDESCRIPTOR_PNEXTMBUF_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Pointer to the Next MBuf chain
 */
#define IX_NPE_A_TXDESCRIPTOR_PNEXTMBUF_OFFSET      16

/**
 * @def IX_NPE_A_TXDESCRIPTOR_TOTALLENGTH_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Total Length
 *
 * Total number of bytes written to the chain of MBufs by the NPE
 */
#define IX_NPE_A_TXDESCRIPTOR_TOTALLENGTH_OFFSET    20

/**
 * @def IX_NPE_A_TXDESCRIPTOR_AAL5CRCRESIDUE_OFFSET
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor AAL5 CRC Residue
 *
 * Current CRC value for a PDU
 */
#define IX_NPE_A_TXDESCRIPTOR_AAL5CRCRESIDUE_OFFSET 24

/**
 * @def IX_NPE_A_TXDESCRIPTOR_SIZE
 *
 * @brief ATM Descriptor structure offset for Transmit Descriptor Size
 */
#define IX_NPE_A_TXDESCRIPTOR_SIZE                  28

/**
 * @def IX_NPE_A_CHAIN_DESC_COUNT_MAX
 *
 * @brief Maximum number of chained MBufs that can be chained together
 */
#define IX_NPE_A_CHAIN_DESC_COUNT_MAX            256

/*
 *  Definition of the ATM cell header
 *
 * This would most conviently be defined as the bit field shown below.
 * Endian portability prevents this, therefore a set of macros
 * are defined to access the fields within the cell header assumed to
 * be passed as a UINT32.
 *
 * Changes to field sizes or orders must be reflected in the offset
 * definitions above.
 *
 *    typedef struct
 *    {
 *       unsigned int gfc:4;
 *       unsigned int vpi:8;
 *       unsigned int vci:16;
 *       unsigned int pti:3;
 *       unsigned int clp:1;
 *    } IxNpeA_AtmCellHeader;
 *
 */

/** Mask to acess GFC */
#define GFC_MASK        0xf0000000

/** return GFC from ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_GFC_GET( header ) \
(((header) & GFC_MASK) >> 28)

/** set GFC into ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_GFC_SET( header,gfc ) \
do { \
    (header) &= ~GFC_MASK; \
    (header) |= (((gfc) << 28) & GFC_MASK); \
} while(0)

/** Mask to acess VPI */
#define VPI_MASK        0x0ff00000

/** return VPI from ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_VPI_GET( header ) \
(((header) & VPI_MASK) >> 20)

/** set VPI into ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_VPI_SET( header, vpi ) \
do { \
    (header) &= ~VPI_MASK; \
    (header) |= (((vpi) << 20) & VPI_MASK); \
} while(0)

/** Mask to acess VCI */
#define VCI_MASK        0x000ffff0

/** return VCI from ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_VCI_GET( header ) \
(((header) & VCI_MASK) >> 4)

/** set VCI into ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_VCI_SET( header, vci ) \
do { \
    (header) &= ~VCI_MASK; \
    (header) |= (((vci) << 4) & VCI_MASK); \
} while(0)

/** Mask to acess PTI */
#define PTI_MASK        0x0000000e

/** return PTI from ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_PTI_GET( header ) \
(((header) & PTI_MASK) >> 1)

/** set PTI into ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_PTI_SET( header, pti ) \
do { \
    (header) &= ~PTI_MASK; \
    (header) |= (((pti) << 1) & PTI_MASK); \
} while(0)

/** Mask to acess CLP */
#define CLP_MASK        0x00000001

/** return CLP from ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_CLP_GET( header ) \
((header) & CLP_MASK)

/** set CLP into ATM cell header */
#define IX_NPE_A_ATMCELLHEADER_CLP_SET( header, clp ) \
do { \
    (header) &= ~CLP_MASK; \
    (header) |= ((clp) & CLP_MASK); \
} while(0)


/*
* Definition of the Rx bitfield
*
* This would most conviently be defined as the bit field shown below.
* Endian portability prevents this, therefore a set of macros
* are defined to access the fields within the rxBitfield assumed to
* be passed as a UINT32.
*
* Changes to field sizes or orders must be reflected in the offset
* definitions above.
*
* Rx bitfield
*    struct
*    {   IX_NPEA_RXBITFIELD(
*        unsigned int status:1,
*        unsigned int port:7,
*        unsigned int vcId:8,
*        unsigned int currMbufSize:16);
*    } rxBitField;
*
*/

/** Mask to acess the rxBitField status */
#define STATUS_MASK     0x80000000

/** return the rxBitField status */
#define IX_NPE_A_RXBITFIELD_STATUS_GET( rxbitfield ) \
(((rxbitfield) & STATUS_MASK) >> 31)

/** set the rxBitField status */
#define IX_NPE_A_RXBITFIELD_STATUS_SET( rxbitfield, status ) \
do { \
    (rxbitfield) &= ~STATUS_MASK; \
    (rxbitfield) |= (((status) << 31) & STATUS_MASK); \
} while(0)

/** Mask to acess the rxBitField port */
#define PORT_MASK       0x7f000000

/** return the rxBitField port */
#define IX_NPE_A_RXBITFIELD_PORT_GET( rxbitfield ) \
(((rxbitfield) & PORT_MASK) >> 24)

/** set the rxBitField port */
#define IX_NPE_A_RXBITFIELD_PORT_SET( rxbitfield, port ) \
do { \
    (rxbitfield) &= ~PORT_MASK; \
    (rxbitfield) |= (((port) << 24) & PORT_MASK); \
} while(0)

/** Mask to acess the rxBitField vcId */
#define VCID_MASK       0x00ff0000

/** return the rxBitField vcId */
#define IX_NPE_A_RXBITFIELD_VCID_GET( rxbitfield ) \
(((rxbitfield) & VCID_MASK) >> 16)

/** set the rxBitField vcId */
#define IX_NPE_A_RXBITFIELD_VCID_SET( rxbitfield, vcid ) \
do { \
    (rxbitfield) &= ~VCID_MASK; \
    (rxbitfield) |= (((vcid) << 16) & VCID_MASK); \
} while(0)

/** Mask to acess the rxBitField mbuf size */
#define CURRMBUFSIZE_MASK       0x0000ffff

/** return the rxBitField mbuf size */
#define IX_NPE_A_RXBITFIELD_CURRMBUFSIZE_GET( rxbitfield ) \
((rxbitfield) & CURRMBUFSIZE_MASK)

/** set the rxBitField mbuf size */
#define IX_NPE_A_RXBITFIELD_CURRMBUFSIZE_SET( rxbitfield, currmbufsize ) \
do { \
    (rxbitfield) &= ~CURRMBUFSIZE_MASK; \
    (rxbitfield) |= ((currmbufsize) & CURRMBUFSIZE_MASK); \
} while(0)



/**
 * @brief Tx Descriptor definition
 */
typedef struct
{
    UINT8 port;				/**< Tx Port number */
    UINT8 aalType; 			/**< AAL Type */
    UINT16 currMbufLen;			/**< mbuf length */
    UINT32 atmCellHeader;		/**< ATM cell header */
    IX_OSAL_MBUF *pCurrMbuf;	        /**< pointer to mbuf */
    unsigned char *pCurrMbufData;	/**< Pointer to mbuf->dat */
    IX_OSAL_MBUF *pNextMbuf;		/**< Pointer to next mbuf */
    UINT32  totalLen;			/**< Total Length */
    UINT32  aal5CrcResidue;		/**< AAL5 CRC Residue */
} IxNpeA_TxAtmVc;

/* Changes to field sizes or orders must be reflected in the offset
 * definitions above. */




/**
 * @brief Rx Descriptor definition
 */
typedef struct
{
    UINT32  rxBitField;			/**< Received bit field */
    UINT32  atmCellHeader;		/**< ATM Cell Header */
    UINT32  rsvdWord0;                  /**< Reserved field */
    UINT16  currMbufLen;		/**< Mbuf Length */
    UINT8   timeLimit; 			/**< Payload Reassembly timeLimit (used for aal0_xx only) */
    UINT8   rsvdByte0;                  /**< Reserved field */ 
    UINT32  rsvdWord1;   		/**< Reserved field */
    IX_OSAL_MBUF *pCurrMbuf;		/**< Pointer to current mbuf */
    unsigned char *pCurrMbufData;	/**< Pointer to current mbuf->data */
    IX_OSAL_MBUF *pNextMbuf;		/**< Pointer to next mbuf */
    UINT32  totalLen;			/**< Total Length */
    UINT32  aal5CrcResidue;		/**< AAL5 CRC Residue */
} IxNpeA_RxAtmVc;


/**
 * @brief NPE-A AAL Type
 */
typedef enum
{
    IX_NPE_A_AAL_TYPE_INVALID = 0,	/**< Invalid AAL type */
    IX_NPE_A_AAL_TYPE_0_48    = 0x1,	/**< AAL0 - 48 byte */
    IX_NPE_A_AAL_TYPE_0_52    = 0x2,	/**< AAL0 - 52 byte */
    IX_NPE_A_AAL_TYPE_5       = 0x5,	/**< AAL5 */
    IX_NPE_A_AAL_TYPE_OAM     = 0xF	/**< OAM */
} IxNpeA_AalType;

/**
 * @brief NPE-A Payload format 52-bytes & 48-bytes
 */
typedef enum
{
    IX_NPE_A_52_BYTE_PAYLOAD = 0,	/**< 52 byte payload */
    IX_NPE_A_48_BYTE_PAYLOAD		/**< 48 byte payload */
} IxNpeA_PayloadFormat;

/**
 * @brief  HSS Packetized NpePacket Descriptor Structure
 */
typedef struct
{
    UINT8   status;		/**< Status of the packet passed to the client */
    UINT8   errorCount;		/**< Number of errors */
    UINT8   chainCount;		/**< Mbuf chain count e.g. 0 - No mbuf chain */
    UINT8   rsvdByte0;		/**< Reserved byte to make the descriptor word align */

    UINT16  packetLength;	/**< Packet Length */
    UINT16  rsvdShort0;		/**< Reserved short to make the descriptor a word align */

    IX_OSAL_MBUF *pRootMbuf;	/**< Pointer to Root mbuf */
    IX_OSAL_MBUF *pNextMbuf;	/**< Pointer to next mbuf */
    UINT8   *pMbufData;		/**< Pointer to the current mbuf->data */
    UINT32  mbufLength;		/**< Current mbuf length */

} IxNpeA_NpePacketDescriptor;


#endif
/**
 *@}
 */

#endif /* __doxygen_HIDE */
