/**
 * @file IxDmaAcc.h
 *
 * @date	15 October 2002 
 *
 * @brief   API of the IXP400 DMA Access Driver Component (IxDma)
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

/*---------------------------------------------------------------------
   Doxygen group definitions
  ---------------------------------------------------------------------*/

#ifndef IXDMAACC_H
#define IXDMAACC_H

#include "IxOsal.h"
#include "IxNpeDl.h"
/**
 * @defgroup IxDmaTypes IXP400 DMA Types (IxDmaTypes)
 * @brief The common set of types used in the DMA component
 * @{
 */

/** 
 * @ingroup IxDmaTypes
 * @enum IxDmaReturnStatus
 * @brief Dma return status definitions
 */
typedef enum
{
    IX_DMA_SUCCESS = IX_SUCCESS,  /**< DMA Transfer Success */
    IX_DMA_FAIL = IX_FAIL,        /**< DMA Transfer Fail */
    IX_DMA_INVALID_TRANSFER_WIDTH, /**< Invalid transfer width */
    IX_DMA_INVALID_TRANSFER_LENGTH, /**< Invalid transfer length */
    IX_DMA_INVALID_TRANSFER_MODE, /**< Invalid transfer mode */
    IX_DMA_INVALID_ADDRESS_MODE, /**< Invalid address mode */
    IX_DMA_REQUEST_FIFO_FULL  /**< DMA request queue is full */
} IxDmaReturnStatus;

/** 
 * @ingroup IxDmaTypes
 * @enum IxDmaTransferMode
 * @brief Dma transfer mode definitions
 * @note Copy and byte swap, and copy and reverse modes only support multiples of word data length.
 */
typedef enum
{
    IX_DMA_COPY_CLEAR = 0,      /**< copy and clear source*/
    IX_DMA_COPY,                /**< copy */
    IX_DMA_COPY_BYTE_SWAP,      /**< copy and byte swap (endian) */
    IX_DMA_COPY_REVERSE,        /**< copy and reverse */
    IX_DMA_TRANSFER_MODE_INVALID /**< Invalid transfer mode */
} IxDmaTransferMode;

/** 
 * @ingroup IxDmaTypes
 * @enum IxDmaAddressingMode
 * @brief Dma addressing mode definitions
 * @note Fixed source address to fixed destination address addressing mode is not supported.
 */
typedef enum
{
    IX_DMA_INC_SRC_INC_DST = 0, /**< Incremental source address to incremental destination address */
    IX_DMA_INC_SRC_FIX_DST,     /**< Incremental source address to incremental destination address */
    IX_DMA_FIX_SRC_INC_DST,     /**< Incremental source address to incremental destination address */
    IX_DMA_FIX_SRC_FIX_DST,     /**< Incremental source address to incremental destination address */
    IX_DMA_ADDRESSING_MODE_INVALID /**< Invalid Addressing Mode */
} IxDmaAddressingMode;

/** 
 * @ingroup IxDmaTypes
 * @enum IxDmaTransferWidth
 * @brief Dma transfer width definitions
 * @Note Fixed addresses (either source or destination) do not support burst transfer width.
 */
typedef enum
{
    IX_DMA_32_SRC_32_DST = 0,  /**< 32-bit src to 32-bit dst */
    IX_DMA_32_SRC_16_DST,      /**< 32-bit src to 16-bit dst */
    IX_DMA_32_SRC_8_DST,       /**< 32-bit src to 8-bit dst */
    IX_DMA_16_SRC_32_DST,      /**< 16-bit src to 32-bit dst */
    IX_DMA_16_SRC_16_DST,      /**< 16-bit src to 16-bit dst */
    IX_DMA_16_SRC_8_DST,       /**< 16-bit src to 8-bit dst */
    IX_DMA_8_SRC_32_DST,       /**< 8-bit src to 32-bit dst */
    IX_DMA_8_SRC_16_DST,       /**< 8-bit src to 16-bit dst */
    IX_DMA_8_SRC_8_DST,        /**< 8-bit src to 8-bit dst */
    IX_DMA_8_SRC_BURST_DST,    /**< 8-bit src to burst dst - Not supported for fixed destination address */
    IX_DMA_16_SRC_BURST_DST,   /**< 16-bit src to burst dst - Not supported for fixed destination address */
    IX_DMA_32_SRC_BURST_DST,   /**< 32-bit src to burst dst - Not supported for fixed destination address */
    IX_DMA_BURST_SRC_8_DST,    /**< burst src to 8-bit dst  - Not supported for fixed source address */
    IX_DMA_BURST_SRC_16_DST,   /**< burst src to 16-bit dst - Not supported for fixed source address */
    IX_DMA_BURST_SRC_32_DST,   /**< burst src to 32-bit dst - Not supported for fixed source address*/
    IX_DMA_BURST_SRC_BURST_DST, /**< burst src to burst dst  - Not supported for fixed source and destination address
*/
    IX_DMA_TRANSFER_WIDTH_INVALID /**< Invalid transfer width */
} IxDmaTransferWidth;

/** 
 * @ingroup IxDmaTypes
 * @enum IxDmaNpeId
 * @brief NpeId numbers to identify NPE A, B or C
 */
typedef enum
{
    IX_DMA_NPEID_NPEA = 0, /**< Identifies NPE A */
    IX_DMA_NPEID_NPEB,     /**< Identifies NPE B */
    IX_DMA_NPEID_NPEC,     /**< Identifies NPE C */
    IX_DMA_NPEID_MAX       /**< Total Number of NPEs */
} IxDmaNpeId;
/* @} */
/**
 * @defgroup IxDmaAcc IXP400 DMA Access Driver (IxDmaAcc) API
 *
 * @brief The public API for the IXP400 IxDmaAcc component
 *
 * @{
 */

/**
 * @ingroup IxDmaAcc
 * @brief DMA Request Id type
 */
typedef UINT32 IxDmaAccRequestId;

/**
 * @ingroup IxDmaAcc
 * @def IX_DMA_REQUEST_FULL
 * @brief DMA request queue is full
 * This constant is a return value used to tell the user that the IxDmaAcc
 * queue is full.
 *
 */
#define IX_DMA_REQUEST_FULL 16

/**
 * @ingroup 	IxDmaAcc
 * @brief       DMA completion notification
 * This function is called to notify a client that the DMA has been completed
 * @param status @ref IxDmaReturnStatus [out] - reporting to client
 *
 */
typedef void (*IxDmaAccDmaCompleteCallback) (IxDmaReturnStatus status);

/**
 * @ingroup 	IxDmaAcc
 * 
 * @fn ixDmaAccInit(IxNpeDlNpeId npeId)
 * 
 * @brief 	Initialise the DMA Access component
 * This function will initialise the DMA Access component internals
 * @param npeId @ref IxNpeDlNpeId [in] - NPE to use for Dma Transfer
 * @return @li IX_SUCCESS succesfully initialised the component
 * @return @li IX_FAIL Initialisation failed for some unspecified
 * internal reason.
 */
PUBLIC IX_STATUS
ixDmaAccInit(IxNpeDlNpeId npeId);

/**
 * @ingroup 	IxDmaAcc
 * 
 * @fn ixDmaAccDmaTransfer(
    IxDmaAccDmaCompleteCallback callback,
    UINT32 SourceAddr,
    UINT32 DestinationAddr,
    UINT16 TransferLength,
    IxDmaTransferMode TransferMode,
    IxDmaAddressingMode AddressingMode,
    IxDmaTransferWidth TransferWidth)
 *
 * @brief       Perform DMA transfer
 * This function will perform DMA transfer between devices within the
 * IXP400 memory map.
 * @note The following are restrictions for IxDmaAccDmaTransfer:
 *      @li The function is non re-entrant.
 *      @li The function assumes host devices are operating in big-endian mode.
 *      @li Fixed address does not suport burst transfer width
 *      @li Fixed source address to fixed destinatiom address mode is not suported
 *      @li The incrementing source address for expansion bus will not support a burst transfer width and copy and clear mode
 *
 * @param callback @ref IxDmaAccDmaCompleteCallback [in] - function pointer to be stored and called when the DMA transfer is completed. This cannot be NULL.
 * @param SourceAddr UINT32 [in] -	Starting address of DMA source. Must be a valid IXP400 memory map address.
 * @param DestinationAddr UINT32 [in] - Starting address of DMA destination. Must be a valid IXP400 memory map address.
 * @param TransferLength UINT16 [in] - The size of DMA data transfer. The range must be from 1-64Kbyte
 * @param TransferMode @ref IxDmaTransferMode [in] - The DMA transfer mode
 * @param AddressingMode @ref IxDmaAddressingMode [in] - The DMA addressing mode
 * @param TransferWidth	@ref IxDmaTransferWidth [in] - The DMA transfer width
 *
 * @return @li IX_DMA_SUCCESS 	Notification that the DMA request is succesful
 * @return @li IX_DMA_FAIL 	IxDmaAcc not yet initialised or some internal error has occured
 * @return @li IX_DMA_INVALID_TRANSFER_WIDTH Transfer width is nit valid
 * @return @li IX_DMA_INVALID_TRANSFER_LENGTH Transfer length outside of valid range
 * @return @li IX_DMA_INVALID_TRANSFER_MODE Transfer Mode not valid
 * @return @li IX_DMA_REQUEST_FIFO_FULL IxDmaAcc request queue is full
 */
PUBLIC IxDmaReturnStatus
ixDmaAccDmaTransfer(
    IxDmaAccDmaCompleteCallback callback,
    UINT32 SourceAddr,
    UINT32 DestinationAddr,
    UINT16 TransferLength,
    IxDmaTransferMode TransferMode,
    IxDmaAddressingMode AddressingMode,
    IxDmaTransferWidth TransferWidth);
/**
 * @ingroup IxDmaAcc
 *
 * @fn ixDmaAccShow(void)
 *
 * @brief Display some component information for debug purposes
 * Show some internal operation information relating to the DMA service.
 * At a minimum the following will show.
 * - the number of the DMA pend (in queue)
 * @param None
 * @return @li None
 */
PUBLIC IX_STATUS
ixDmaAccShow(void);

#endif /* IXDMAACC_H */

