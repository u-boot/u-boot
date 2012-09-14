/*
 * (C) Copyright 2012 Xilinx
 *
 * This header provides operations to manage buffer descriptors in support
 * of scatter-gather DMA.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef XEMACPSS_BD_H		/* prevent circular inclusions */
#define XEMACPSS_BD_H		/* by using protection macros */

/*
 *
 * The API exported by this header defines abstracted macros that allow the
 * user to read/write specific BD fields.
 *
 * Buffer Descriptors:
 *
 * A buffer descriptor (BD) defines a DMA transaction. The macros defined by
 * this header file allow access to most fields within a BD to tailor a DMA
 * transaction according to user and hardware requirements.  See the hardware
 * IP DMA spec for more information on BD fields and how they affect transfers.
 *
 * The XEmacPss_Bd structure defines a BD. The organization of this structure
 * is driven mainly by the hardware for use in scatter-gather DMA transfers.
 *
 * Performance:
 *
 * Limiting I/O to BDs can improve overall performance of the DMA channel.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

//#include <string.h>
//#include "xbasic_types.h"
#include <common.h>
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* Minimum BD alignment */
#define XEMACPSS_DMABD_MINIMUM_ALIGNMENT  4

/**
 * The XEmacPss_Bd is the type for buffer descriptors (BDs).
 */
#define XEMACPSS_BD_NUM_WORDS 2
typedef u32 XEmacPss_Bd[XEMACPSS_BD_NUM_WORDS];


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * Zero out BD fields
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Nothing
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdClear(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdClear(BdPtr)                                  \
    memset((BdPtr), 0, sizeof(XEmacPss_Bd))

/****************************************************************************/
/**
*
* Read the given Buffer Descriptor word.
*
* @param    BaseAddress is the base address of the BD to read
* @param    Offset is the word offset to be read
*
* @return   The 32-bit value of the field
*
* @note
* C-style signature:
*    u32 XEmacPss_BdRead(u32 BaseAddress, u32 Offset)
*
*****************************************************************************/
#define XEmacPss_BdRead(BaseAddress, Offset)             \
    (*(u32*)((u32)(BaseAddress) + (u32)(Offset)))

/****************************************************************************/
/**
*
* Write the given Buffer Descriptor word.
*
* @param    BaseAddress is the base address of the BD to write
* @param    Offset is the word offset to be written
* @param    Data is the 32-bit value to write to the field
*
* @return   None.
*
* @note
* C-style signature:
*    void XEmacPss_BdWrite(u32 BaseAddress, u32 Offset, u32 Data)
*
*****************************************************************************/
#define XEmacPss_BdWrite(BaseAddress, Offset, Data)              \
    (*(u32*)((u32)(BaseAddress) + (u32)(Offset)) = (Data))

/*****************************************************************************/
/**
 * Set the BD's Address field (word 0).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Addr  is the value to write to BD's status field.
 *
 * @note :
 * 
 * C-style signature:
 *    void XEmacPss_BdSetAddressTx(XEmacPss_Bd* BdPtr, u32 Addr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetAddressTx(BdPtr, Addr)                        \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_ADDR_OFFSET, (u32)(Addr)))


/*****************************************************************************/
/**
 * Set the BD's Address field (word 0).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Data  is the value to write to BD's status field.
 *
 * @note : Due to some bits are mixed within recevie BD's address field,
 *         read-modify-write is performed.
 * 
 * C-style signature:
 *    void XEmacPss_BdSetAddressRx(XEmacPss_Bd* BdPtr, u32 Addr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetAddressRx(BdPtr, Addr)                        \
    XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_ADDR_OFFSET,              \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET) &           \
    ~XEMACPSS_RXBUF_ADD_MASK) | (u32)(Addr)))


/*****************************************************************************/
/**
 * Set the BD's Status field (word 1).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Data  is the value to write to BD's status field.
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetStatus(XEmacPss_Bd* BdPtr, u32 Data)
 *
 *****************************************************************************/
#define XEmacPss_BdSetStatus(BdPtr, Data)                           \
    XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,              \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) | Data)


/*****************************************************************************/
/**
 * Retrieve the BD's Packet DMA transfer status word (word 1).
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Status word
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetStatus(XEmacPss_Bd* BdPtr)
 *
 * Due to the BD bit layout differences in transmit and receive. User's
 * caution is required.
 *****************************************************************************/
#define XEmacPss_BdGetStatus(BdPtr)                                 \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET)


/*****************************************************************************/
/**
 * Get the address (bits 0..31) of the BD's buffer address (word 0)
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetBufAddr(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdGetBufAddr(BdPtr)                               \
    (XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET))


/*****************************************************************************/
/**
 * Set transfer length in bytes for the given BD. The length must be set each
 * time a BD is submitted to hardware.
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  LenBytes is the number of bytes to transfer.
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetLength(XEmacPss_Bd* BdPtr, u32 LenBytes)
 *
 *****************************************************************************/
#define XEmacPss_BdSetLength(BdPtr, LenBytes)                       \
    XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,              \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    ~XEMACPSS_TXBUF_LEN_MASK) | (LenBytes)))


/*****************************************************************************/
/**
 * Retrieve the BD length field.
 *
 * For Tx channels, the returned value is the same as that written with
 * XEmacPss_BdSetLength().
 *
 * For Rx channels, the returned value is the size of the received packet.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Length field processed by hardware or set by
 *         XEmacPss_BdSetLength().
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetLength(XEmacPss_Bd* BdPtr)
 *    XEAMCPSS_RXBUF_LEN_MASK is same as XEMACPSS_TXBUF_LEN_MASK.
 *
 *****************************************************************************/
#define XEmacPss_BdGetLength(BdPtr)                                 \
    (XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &            \
    XEMACPSS_RXBUF_LEN_MASK)


/*****************************************************************************/
/**
 * Test whether the given BD has been marked as the last BD of a packet.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return TRUE if BD represents the "Last" BD of a packet, FALSE otherwise
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsLast(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsLast(BdPtr)                                    \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_EOF_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Tell the DMA engine that the given transmit BD marks the end of the current
 * packet to be processed.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetLast(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetLast(BdPtr)                                   \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) |             \
    XEMACPSS_TXBUF_LAST_MASK))


/*****************************************************************************/
/**
 * Tell the DMA engine that the current packet does not end with the given
 * BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdClearLast(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdClearLast(BdPtr)                                 \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &             \
    ~XEMACPSS_TXBUF_LAST_MASK))


/*****************************************************************************/
/**
 * Set this bit to mark the last descriptor in the receive buffer descriptor
 * list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetRxWrap(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetRxWrap(BdPtr)                                 \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_ADDR_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET) |             \
    XEMACPSS_RXBUF_WRAP_MASK))


/*****************************************************************************/
/**
 * Determine the wrap bit of the receive BD which indicates end of the
 * BD list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxWrap(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxWrap(BdPtr)                                  \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET) &           \
    XEMACPSS_RXBUF_WRAP_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Sets this bit to mark the last descriptor in the transmit buffer
 * descriptor list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetTxWrap(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetTxWrap(BdPtr)                                 \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) |             \
    XEMACPSS_TXBUF_WRAP_MASK))


/*****************************************************************************/
/**
 * Determine the wrap bit of the transmit BD which indicates end of the
 * BD list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetTxWrap(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsTxWrap(BdPtr)                                  \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_TXBUF_WRAP_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/*
 * Must clear this bit to enable the MAC to write data to the receive
 * buffer. Hardware sets this bit once it has successfully written a frame to
 * memory. Once set, software has to clear the bit before the buffer can be
 * used again. This macro clear the new bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdClearRxNew(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdClearRxNew(BdPtr)                                \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_ADDR_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET) &             \
    ~XEMACPSS_RXBUF_NEW_MASK))


/*****************************************************************************/
/**
 * Determine the new bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxNew(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxNew(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_ADDR_OFFSET) &           \
    XEMACPSS_RXBUF_NEW_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Software sets this bit to disable the buffer to be read by the hardware.
 * Hardware sets this bit for the first buffer of a frame once it has been
 * successfully transmitted. This macro sets this bit of transmit BD to avoid
 * confusion.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdSetTxUsed(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetTxUsed(BdPtr)                                 \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) |             \
    XEMACPSS_TXBUF_USED_MASK))


/*****************************************************************************/
/**
 * Software clears this bit to enable the buffer to be read by the hardware.
 * Hardware sets this bit for the first buffer of a frame once it has been
 * successfully transmitted. This macro clears this bit of transmit BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void XEmacPss_BdClearTxUsed(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdClearTxUsed(BdPtr)                               \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &             \
    ~XEMACPSS_TXBUF_USED_MASK))


/*****************************************************************************/
/**
 * Determine the used bit of the transmit BD. 
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsTxUsed(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsTxUsed(BdPtr)                                  \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_TXBUF_USED_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to too many retries.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsTxRetry(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsTxRetry(BdPtr)                                 \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_TXBUF_RETRY_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to data can not be
 * feteched in time or buffers are exhausted.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsTxUrun(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsTxUrun(BdPtr)                                  \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_TXBUF_URUN_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to buffer is exhausted
 * mid-frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsTxExh(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsTxExh(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_TXBUF_EXH_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Sets this bit, no CRC will be appended to the current frame. This control
 * bit must be set for the first buffer in a frame and will be ignored for
 * the subsequent buffers of a frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * This bit must be clear when using the transmit checksum generation offload,
 * otherwise checksum generation and substitution will not occur.
 *
 * C-style signature:
 *    u32 XEmacPss_BdSetTxNoCRC(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdSetTxNoCRC(BdPtr)                                \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) |             \
    XEMACPSS_TXBUF_NOCRC_MASK))


/*****************************************************************************/
/**
 * Clear this bit, CRC will be appended to the current frame. This control
 * bit must be set for the first buffer in a frame and will be ignored for
 * the subsequent buffers of a frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * This bit must be clear when using the transmit checksum generation offload,
 * otherwise checksum generation and substitution will not occur.
 *
 * C-style signature:
 *    u32 XEmacPss_BdClearTxNoCRC(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdClearTxNoCRC(BdPtr)                              \
    (XEmacPss_BdWrite((BdPtr), XEMACPSS_BD_STAT_OFFSET,             \
    XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &             \
    ~XEMACPSS_TXBUF_NOCRC_MASK))


/*****************************************************************************/
/**
 * Determine the broadcast bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxBcast(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxBcast(BdPtr)                                 \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_BCAST_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine the multicast hash bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxMultiHash(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxMultiHash(BdPtr)                             \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_MULTIHASH_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine the unicast hash bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxUniHash(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxUniHash(BdPtr)                               \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_UNIHASH_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if the received frame is a VLAN Tagged frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxVlan(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxVlan(BdPtr)                                  \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_VLAN_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if the received frame has Type ID of 8100h and null VLAN
 * identifier(Priority tag).
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxPri(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxPri(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_PRI_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine if the received frame's Concatenation Format Indicator (CFI) of
 * the frames VLANTCI field was set.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdIsRxCFI(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxCFI(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_CFI_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine the End Of Frame (EOF) bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetRxEOF(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxEOF(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_EOF_MASK) ? TRUE : FALSE)


/*****************************************************************************/
/**
 * Determine the Start Of Frame (SOF) bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u32 XEmacPss_BdGetRxSOF(XEmacPss_Bd* BdPtr)
 *
 *****************************************************************************/
#define XEmacPss_BdIsRxSOF(BdPtr)                                   \
    ((XEmacPss_BdRead((BdPtr), XEMACPSS_BD_STAT_OFFSET) &           \
    XEMACPSS_RXBUF_SOF_MASK) ? TRUE : FALSE)


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
