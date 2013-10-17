/* 
 * @file:    IxQMgrAqmIf.c
 *
 * @author Intel Corporation
 * @date     30-Oct-2001
 *
 * @brief    This component provides a set of functions for
 * perfoming I/O on the AQM hardware.
 * 
 * Design Notes: 
 *              These functions are intended to be as fast as possible
 * and as a result perform NO PARAMETER CHECKING.
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
*/

/*
 * Inlines are compiled as function when this is defined.
 * N.B. Must be placed before #include of "IxQMgrAqmIf_p.h
 */
#ifndef IXQMGRAQMIF_P_H
#    define IXQMGRAQMIF_C
#else
#    error
#endif

/*
 * User defined include files.
 */
#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxQMgrAqmIf_p.h"
#include "IxQMgrLog_p.h"


/*
 * #defines and macros used in this file.
 */

/* These defines are the bit offsets of the various fields of
 * the queue configuration register
 */
#define IX_QMGR_Q_CONFIG_WRPTR_OFFSET       0x00
#define IX_QMGR_Q_CONFIG_RDPTR_OFFSET       0x07
#define IX_QMGR_Q_CONFIG_BADDR_OFFSET       0x0E
#define IX_QMGR_Q_CONFIG_ESIZE_OFFSET       0x16
#define IX_QMGR_Q_CONFIG_BSIZE_OFFSET       0x18
#define IX_QMGR_Q_CONFIG_NE_OFFSET          0x1A
#define IX_QMGR_Q_CONFIG_NF_OFFSET          0x1D

#define IX_QMGR_BASE_ADDR_16_WORD_ALIGN     0x40
#define IX_QMGR_BASE_ADDR_16_WORD_SHIFT     0x6

#define IX_QMGR_NE_NF_CLEAR_MASK            0x03FFFFFF
#define IX_QMGR_NE_MASK                     0x7
#define IX_QMGR_NF_MASK                     0x7
#define IX_QMGR_SIZE_MASK                   0x3
#define IX_QMGR_ENTRY_SIZE_MASK             0x3
#define IX_QMGR_BADDR_MASK                  0x003FC000
#define IX_QMGR_RDPTR_MASK                  0x7F
#define IX_QMGR_WRPTR_MASK                  0x7F
#define IX_QMGR_RDWRPTR_MASK                0x00003FFF

#define IX_QMGR_AQM_ADDRESS_SPACE_SIZE_IN_WORDS 0x1000

/* Base address of AQM SRAM */
#define IX_QMGR_AQM_SRAM_BASE_ADDRESS_OFFSET \
((IX_QMGR_QUECONFIG_BASE_OFFSET) + (IX_QMGR_QUECONFIG_SIZE))

/* Min buffer size used for generating buffer size in QUECONFIG */
#define IX_QMGR_MIN_BUFFER_SIZE 16

/* Reset values of QMgr hardware registers */
#define IX_QMGR_QUELOWSTAT_RESET_VALUE    0x33333333
#define IX_QMGR_QUEUOSTAT_RESET_VALUE     0x00000000
#define IX_QMGR_QUEUPPSTAT0_RESET_VALUE   0xFFFFFFFF
#define IX_QMGR_QUEUPPSTAT1_RESET_VALUE   0x00000000
#define IX_QMGR_INT0SRCSELREG_RESET_VALUE 0x00000000
#define IX_QMGR_QUEIEREG_RESET_VALUE      0x00000000
#define IX_QMGR_QINTREG_RESET_VALUE       0xFFFFFFFF
#define IX_QMGR_QUECONFIG_RESET_VALUE     0x00000000

#define IX_QMGR_PHYSICAL_AQM_BASE_ADDRESS IX_OSAL_IXP400_QMGR_PHYS_BASE

#define IX_QMGR_QUELOWSTAT_BITS_PER_Q (BITS_PER_WORD/IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD)

#define IX_QMGR_QUELOWSTAT_QID_MASK 0x7
#define IX_QMGR_Q_CONFIG_ADDR_GET(qId)\
        (((qId) * IX_QMGR_NUM_BYTES_PER_WORD) +\
                  IX_QMGR_QUECONFIG_BASE_OFFSET)

#define IX_QMGR_ENTRY1_OFFSET 0
#define IX_QMGR_ENTRY2_OFFSET 1
#define IX_QMGR_ENTRY4_OFFSET 3

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
UINT32 aqmBaseAddress = 0;
/* Store addresses and bit-masks for certain queue access and status registers.
 * This is to facilitate inlining of QRead, QWrite and QStatusGet functions
 * in IxQMgr,h
 */
extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
UINT32 * ixQMgrAqmIfQueAccRegAddr[IX_QMGR_MAX_NUM_QUEUES];
UINT32 ixQMgrAqmIfQueLowStatRegAddr[IX_QMGR_MIN_QUEUPP_QID];
UINT32 ixQMgrAqmIfQueLowStatBitsOffset[IX_QMGR_MIN_QUEUPP_QID];
UINT32 ixQMgrAqmIfQueLowStatBitsMask;
UINT32 ixQMgrAqmIfQueUppStat0RegAddr;
UINT32 ixQMgrAqmIfQueUppStat1RegAddr;
UINT32 ixQMgrAqmIfQueUppStat0BitMask[IX_QMGR_MIN_QUEUPP_QID];
UINT32 ixQMgrAqmIfQueUppStat1BitMask[IX_QMGR_MIN_QUEUPP_QID];

/* 
 * Fast mutexes, one for each queue, used to protect peek & poke functions
 */
IxOsalFastMutex ixQMgrAqmIfPeekPokeFastMutex[IX_QMGR_MAX_NUM_QUEUES];

/*
 * Function prototypes
 */
PRIVATE unsigned
watermarkToAqmWatermark (IxQMgrWMLevel watermark );

PRIVATE unsigned
entrySizeToAqmEntrySize (IxQMgrQEntrySizeInWords entrySize);

PRIVATE unsigned
bufferSizeToAqmBufferSize (unsigned bufferSizeInWords);

PRIVATE void
ixQMgrAqmIfRegistersReset (void);

PRIVATE void
ixQMgrAqmIfEntryAddressGet (unsigned int entryIndex,
			    UINT32 configRegWord,
			    unsigned int qEntrySizeInwords,
			    unsigned int qSizeInWords,
			    UINT32 **address);
/*
 * Function definitions
 */
void
ixQMgrAqmIfInit (void)
{
    UINT32 aqmVirtualAddr;
    int i;

    /* The value of aqmBaseAddress depends on the logical address
     * assigned by the MMU.
     */
    aqmVirtualAddr =
	(UINT32) IX_OSAL_MEM_MAP(IX_QMGR_PHYSICAL_AQM_BASE_ADDRESS,
				    IX_OSAL_IXP400_QMGR_MAP_SIZE);
    IX_OSAL_ASSERT (aqmVirtualAddr);
    
    ixQMgrAqmIfBaseAddressSet (aqmVirtualAddr);

    ixQMgrAqmIfRegistersReset ();

    for (i = 0; i< IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	ixOsalFastMutexInit(&ixQMgrAqmIfPeekPokeFastMutex[i]);

	/********************************************************************
	 * Register addresses and bit masks are calculated and stored here to
	 * facilitate inlining of QRead, QWrite and QStatusGet functions in
	 * IxQMgr.h.
	 * These calculations are normally performed dynamically in inlined
	 * functions in IxQMgrAqmIf_p.h, and their semantics are reused here.
	 */

	/* AQM Queue access reg addresses, per queue */
	ixQMgrAqmIfQueAccRegAddr[i] = 
	    (UINT32 *)(aqmBaseAddress + IX_QMGR_Q_ACCESS_ADDR_GET(i));
	ixQMgrQInlinedReadWriteInfo[i].qAccRegAddr = 
	    (volatile UINT32 *)(aqmBaseAddress + IX_QMGR_Q_ACCESS_ADDR_GET(i));


	ixQMgrQInlinedReadWriteInfo[i].qConfigRegAddr = 
	    (volatile UINT32 *)(aqmBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(i));

	/* AQM Queue lower-group (0-31), only */
	if (i < IX_QMGR_MIN_QUEUPP_QID)
	{
	    /* AQM Q underflow/overflow status register addresses, per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qUOStatRegAddr = 
		(volatile UINT32 *)(aqmBaseAddress +
		IX_QMGR_QUEUOSTAT0_OFFSET +
		((i / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD) *
		 IX_QMGR_NUM_BYTES_PER_WORD));

	    /* AQM Q underflow status bit masks for status register per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qUflowStatBitMask = 
		(IX_QMGR_UNDERFLOW_BIT_OFFSET + 1) <<
		((i & (IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD - 1)) *
		 (BITS_PER_WORD / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD));

	    /* AQM Q overflow status bit masks for status register, per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qOflowStatBitMask = 
		(IX_QMGR_OVERFLOW_BIT_OFFSET + 1) <<
		((i & (IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD - 1)) *
		 (BITS_PER_WORD / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD));

	    /* AQM Q lower-group (0-31) status register addresses, per queue */
	    ixQMgrAqmIfQueLowStatRegAddr[i] = aqmBaseAddress +
		IX_QMGR_QUELOWSTAT0_OFFSET +
		((i / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD) *
		 IX_QMGR_NUM_BYTES_PER_WORD);

	    /* AQM Q lower-group (0-31) status register bit offset */
	    ixQMgrAqmIfQueLowStatBitsOffset[i] =
		(i & (IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD - 1)) * 
		(BITS_PER_WORD / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD);
	}
	else /* AQM Q upper-group (32-63), only */
	{
	    /* AQM Q upper-group (32-63) Nearly Empty status reg bit masks */
	    ixQMgrAqmIfQueUppStat0BitMask[i - IX_QMGR_MIN_QUEUPP_QID] =
		(1 << (i - IX_QMGR_MIN_QUEUPP_QID));

	    /* AQM Q upper-group (32-63) Full status register bit masks */
	    ixQMgrAqmIfQueUppStat1BitMask[i - IX_QMGR_MIN_QUEUPP_QID] =
		(1 << (i - IX_QMGR_MIN_QUEUPP_QID));
	}
    }

    /* AQM Q lower-group (0-31) status register bit mask */
    ixQMgrAqmIfQueLowStatBitsMask = (1 <<
				    (BITS_PER_WORD /
				     IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD)) - 1;

    /* AQM Q upper-group (32-63) Nearly Empty status register address */
    ixQMgrAqmIfQueUppStat0RegAddr = aqmBaseAddress + IX_QMGR_QUEUPPSTAT0_OFFSET;
    
    /* AQM Q upper-group (32-63) Full status register address */
    ixQMgrAqmIfQueUppStat1RegAddr = aqmBaseAddress + IX_QMGR_QUEUPPSTAT1_OFFSET;
}

/*
 * Uninitialise the AqmIf module by unmapping memory, etc
 */
void
ixQMgrAqmIfUninit (void)
{
    UINT32 virtAddr;

    ixQMgrAqmIfBaseAddressGet (&virtAddr);
    IX_OSAL_MEM_UNMAP (virtAddr);
    ixQMgrAqmIfBaseAddressSet (0);
}

/*
 * Set the the logical base address of AQM
 */
void
ixQMgrAqmIfBaseAddressSet (UINT32 address)
{
    aqmBaseAddress = address;
}

/*
 * Get the logical base address of AQM
 */
void
ixQMgrAqmIfBaseAddressGet (UINT32 *address)
{
    *address = aqmBaseAddress;
}

/*
 * Get the logical base address of AQM SRAM
 */
void
ixQMgrAqmIfSramBaseAddressGet (UINT32 *address)
{
    *address = aqmBaseAddress                +
	IX_QMGR_AQM_SRAM_BASE_ADDRESS_OFFSET;
}

/*
 * This function will write the status bits of a queue
 * specified by qId.
 */
void
ixQMgrAqmIfQRegisterBitsWrite (IxQMgrQId qId, 
			       UINT32 registerBaseAddrOffset,
			       unsigned queuesPerRegWord,
			       UINT32 value)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 statusBitsMask;
    UINT32 bitsPerQueue;

    bitsPerQueue = BITS_PER_WORD / queuesPerRegWord;

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(aqmBaseAddress +
				registerBaseAddrOffset +
				((qId / queuesPerRegWord) *
				 IX_QMGR_NUM_BYTES_PER_WORD));    

    /* Read the current data */
    ixQMgrAqmIfWordRead (registerAddress, &registerWord);


    if( (registerBaseAddrOffset == IX_QMGR_INT0SRCSELREG0_OFFSET) &&
        (qId == IX_QMGR_QUEUE_0) )
    {
      statusBitsMask = 0x7 ;   

      /* Queue 0 at INT0SRCSELREG should not corrupt the value bit-3  */
      value &=  0x7 ;        
    }
    else
    {     
      /* Calculate the mask for the status bits for this queue. */
      statusBitsMask = ((1 << bitsPerQueue) - 1);
      statusBitsMask <<= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);

      /* Mask out bits in value that would overwrite other q data */
      value <<= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);
      value &= statusBitsMask;
    }

    /* Mask out bits to write to */
    registerWord &= ~statusBitsMask;
    

    /* Set the write bits */
    registerWord |= value;

    /*
     * Write the data
     */
    ixQMgrAqmIfWordWrite (registerAddress, registerWord);
}

/*
 * This function generates the parameters that can be used to
 * check if a Qs status matches the specified source select.
 * It calculates which status word to check (statusWordOffset),
 * the value to check the status against (checkValue) and the
 * mask (mask) to mask out all but the bits to check in the status word.
 */
void
ixQMgrAqmIfQStatusCheckValsCalc (IxQMgrQId qId,
				 IxQMgrSourceId srcSel,
				 unsigned int *statusWordOffset,
				 UINT32 *checkValue,
				 UINT32 *mask)
{
    UINT32 shiftVal;
   
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	switch (srcSel)
	{
	    case IX_QMGR_Q_SOURCE_ID_E:
		*checkValue = IX_QMGR_Q_STATUS_E_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_E_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NE:
		*checkValue = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NF:
		*checkValue = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_F:
		*checkValue = IX_QMGR_Q_STATUS_F_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_F_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_E:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_E_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_NE:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_NF:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_F:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_F_BIT_MASK;
		break;
	    default:
		/* Should never hit */
		IX_OSAL_ASSERT(0);
		break;
	}

	/* One nibble of status per queue so need to shift the
	 * check value and mask out to the correct position.
	 */
	shiftVal = (qId % IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD) * 
	    IX_QMGR_QUELOWSTAT_BITS_PER_Q;

	/* Calculate the which status word to check from the qId,
	 * 8 Qs status per word
	 */
	*statusWordOffset = qId / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD;

	*checkValue <<= shiftVal;
	*mask <<= shiftVal;
    }
    else
    {
	/* One status word */
	*statusWordOffset = 0;
	/* Single bits per queue and int source bit hardwired  NE,
	 * Qs start at 32.
	 */
	*mask = 1 << (qId - IX_QMGR_MIN_QUEUPP_QID);
	*checkValue = *mask;
    }
}

void
ixQMgrAqmIfQInterruptEnable (IxQMgrQId qId)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 actualBitOffset;
    
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {    
	registerAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_QUEIEREG0_OFFSET);
    }
    else
    {
	registerAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_QUEIEREG1_OFFSET);
    }

    actualBitOffset = 1 << (qId % IX_QMGR_MIN_QUEUPP_QID);

    ixQMgrAqmIfWordRead (registerAddress, &registerWord);
    ixQMgrAqmIfWordWrite (registerAddress, (registerWord | actualBitOffset));
}

void
ixQMgrAqmIfQInterruptDisable (IxQMgrQId qId)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 actualBitOffset;

    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {    
	registerAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_QUEIEREG0_OFFSET);
    }
    else
    {
	registerAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_QUEIEREG1_OFFSET);
    }

    actualBitOffset = 1 << (qId % IX_QMGR_MIN_QUEUPP_QID);

    ixQMgrAqmIfWordRead (registerAddress, &registerWord);
    ixQMgrAqmIfWordWrite (registerAddress, registerWord & (~actualBitOffset));
}

void
ixQMgrAqmIfQueCfgWrite (IxQMgrQId qId,
		       IxQMgrQSizeInWords qSizeInWords,
		       IxQMgrQEntrySizeInWords entrySizeInWords,
		       UINT32 freeSRAMAddress)
{
    volatile UINT32 *cfgAddress = NULL;
    UINT32 qCfg = 0;
    UINT32 baseAddress = 0;
    unsigned aqmEntrySize = 0;
    unsigned aqmBufferSize = 0;

    /* Build config register */
    aqmEntrySize = entrySizeToAqmEntrySize (entrySizeInWords);
    qCfg |= (aqmEntrySize&IX_QMGR_ENTRY_SIZE_MASK) <<
	IX_QMGR_Q_CONFIG_ESIZE_OFFSET;

    aqmBufferSize = bufferSizeToAqmBufferSize (qSizeInWords);
    qCfg |= (aqmBufferSize&IX_QMGR_SIZE_MASK) << IX_QMGR_Q_CONFIG_BSIZE_OFFSET;

    /* baseAddress, calculated relative to aqmBaseAddress and start address  */
    baseAddress = freeSRAMAddress -
	(aqmBaseAddress + IX_QMGR_QUECONFIG_BASE_OFFSET);
		   
    /* Verify base address aligned to a 16 word boundary */
    if ((baseAddress % IX_QMGR_BASE_ADDR_16_WORD_ALIGN) != 0)
    {
	IX_QMGR_LOG_ERROR0("ixQMgrAqmIfQueCfgWrite () address is not on 16 word boundary\n");
    }
    /* Now convert it to a 16 word pointer as required by QUECONFIG register */
    baseAddress >>= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    
    
    qCfg |= (baseAddress << IX_QMGR_Q_CONFIG_BADDR_OFFSET);


    cfgAddress = (UINT32*)(aqmBaseAddress +
			IX_QMGR_Q_CONFIG_ADDR_GET(qId));


    /* NOTE: High and Low watermarks are set to zero */
    ixQMgrAqmIfWordWrite (cfgAddress, qCfg);
}

void
ixQMgrAqmIfQueCfgRead (IxQMgrQId qId,
		       unsigned int numEntries,
		       UINT32 *baseAddress,
		       unsigned int *ne,
		       unsigned int *nf,
		       UINT32 *readPtr,
		       UINT32 *writePtr)
{
    UINT32 qcfg;
    UINT32 *cfgAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;
    UINT32 *readPtr_ = NULL;
	
    /* Read the queue configuration register */
    ixQMgrAqmIfWordRead (cfgAddress, &qcfg);
    
    /* Extract the base address */
    *baseAddress = (UINT32)((qcfg & IX_QMGR_BADDR_MASK) >>
			    (IX_QMGR_Q_CONFIG_BADDR_OFFSET));

    /* Base address is a 16 word pointer from the start of AQM SRAM.
     * Convert to absolute word address.
     */
    *baseAddress <<= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    *baseAddress += (UINT32)IX_QMGR_QUECONFIG_BASE_OFFSET;

    /*
     * Extract the watermarks. 0->0 entries, 1->1 entries, 2->2 entries, 3->4 entries......
     * If ne > 0 ==> neInEntries = 2^(ne - 1)
     * If ne == 0 ==> neInEntries = 0
     * The same applies.
     */
    *ne = ((qcfg) >> (IX_QMGR_Q_CONFIG_NE_OFFSET)) & IX_QMGR_NE_MASK;
    *nf = ((qcfg) >> (IX_QMGR_Q_CONFIG_NF_OFFSET)) & IX_QMGR_NF_MASK;

    if (0 != *ne)
    {
	*ne = 1 << (*ne - 1);	
    }
    if (0 != *nf)
    {
	*nf = 1 << (*nf - 1);
    }

    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    ixQMgrAqmIfEntryAddressGet (0/* Entry 0. i.e the readPtr*/,
				qcfg,
				qEntrySizeInwords,
				qSizeInWords,
				&readPtr_);
    *readPtr = (UINT32)readPtr_;
    *readPtr -= (UINT32)aqmBaseAddress;/* Offset, not absolute address */

    *writePtr = (qcfg >> IX_QMGR_Q_CONFIG_WRPTR_OFFSET) & IX_QMGR_WRPTR_MASK;
    *writePtr = *baseAddress + (*writePtr * (IX_QMGR_NUM_BYTES_PER_WORD));
    return;
}

unsigned
ixQMgrAqmIfLog2 (unsigned number)
{
    unsigned count = 0;

    /*
     * N.B. this function will return 0
     * for ixQMgrAqmIfLog2 (0)
     */
    while (number/2)
    {
	number /=2;
	count++;	
    }

    return count;
}

void ixQMgrAqmIfIntSrcSelReg0Bit3Set (void)
{

    volatile UINT32 *registerAddress;
    UINT32 registerWord; 

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(aqmBaseAddress +
				IX_QMGR_INT0SRCSELREG0_OFFSET);    

    /* Read the current data */
    ixQMgrAqmIfWordRead (registerAddress, &registerWord);

    /* Set the write bits */
    registerWord |= (1<<IX_QMGR_INT0SRCSELREG0_BIT3) ;

    /*
     * Write the data
     */
    ixQMgrAqmIfWordWrite (registerAddress, registerWord);
}  


void
ixQMgrAqmIfIntSrcSelWrite (IxQMgrQId qId,
			  IxQMgrSourceId sourceId)
{
    ixQMgrAqmIfQRegisterBitsWrite (qId,
				   IX_QMGR_INT0SRCSELREG0_OFFSET,
				   IX_QMGR_INTSRC_NUM_QUE_PER_WORD,
				   sourceId);
}



void
ixQMgrAqmIfWatermarkSet (IxQMgrQId qId,
			unsigned ne,
			unsigned nf)
{
    volatile UINT32 *address = 0;
    UINT32 value = 0;
    unsigned aqmNeWatermark = 0;
    unsigned aqmNfWatermark = 0;

    address = (UINT32*)(aqmBaseAddress +
		     IX_QMGR_Q_CONFIG_ADDR_GET(qId));

    aqmNeWatermark = watermarkToAqmWatermark (ne);
    aqmNfWatermark = watermarkToAqmWatermark (nf);

    /* Read the current watermarks */
    ixQMgrAqmIfWordRead (address, &value);

    /* Clear out the old watermarks */
    value &=  IX_QMGR_NE_NF_CLEAR_MASK;
    
    /* Generate the value to write */
    value |= (aqmNeWatermark << IX_QMGR_Q_CONFIG_NE_OFFSET) |
	(aqmNfWatermark << IX_QMGR_Q_CONFIG_NF_OFFSET); 

    ixQMgrAqmIfWordWrite (address, value);

}

PRIVATE void
ixQMgrAqmIfEntryAddressGet (unsigned int entryIndex,
			    UINT32 configRegWord,
			    unsigned int qEntrySizeInwords,
			    unsigned int qSizeInWords,
			    UINT32 **address)
{
    UINT32 readPtr;
    UINT32 baseAddress;
    UINT32 *topOfAqmSram;

    topOfAqmSram = ((UINT32 *)aqmBaseAddress + IX_QMGR_AQM_ADDRESS_SPACE_SIZE_IN_WORDS);

    /* Extract the base address */
    baseAddress = (UINT32)((configRegWord & IX_QMGR_BADDR_MASK) >>
			   (IX_QMGR_Q_CONFIG_BADDR_OFFSET));

    /* Base address is a 16 word pointer from the start of AQM SRAM.
     * Convert to absolute word address.
     */
    baseAddress <<= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    baseAddress += ((UINT32)aqmBaseAddress + (UINT32)IX_QMGR_QUECONFIG_BASE_OFFSET);

    /* Extract the read pointer. Read pointer is a word pointer */
    readPtr = (UINT32)((configRegWord >>
			IX_QMGR_Q_CONFIG_RDPTR_OFFSET)&IX_QMGR_RDPTR_MASK);

    /* Read/Write pointers(word pointers)  are offsets from the queue buffer space base address.
     * Calculate the absolute read pointer address. NOTE: Queues are circular buffers.
     */
    readPtr  = (readPtr + (entryIndex * qEntrySizeInwords)) & (qSizeInWords - 1); /* Mask by queue size */
    *address = (UINT32 *)(baseAddress + (readPtr * (IX_QMGR_NUM_BYTES_PER_WORD)));

    switch (qEntrySizeInwords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY1_OFFSET) < topOfAqmSram);	    
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY2_OFFSET) < topOfAqmSram);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY4_OFFSET) < topOfAqmSram);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrAqmIfEntryAddressGet");
	    break;
    }
    
}

IX_STATUS
ixQMgrAqmIfQPeek (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry)
{
    UINT32 *cfgRegAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    UINT32 *entryAddress = NULL;
    UINT32 configRegWordOnEntry;
    UINT32 configRegWordOnExit;
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;

    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    /* Read the config register */
    ixQMgrAqmIfWordRead (cfgRegAddress, &configRegWordOnEntry);

    /* Get the entry address */
    ixQMgrAqmIfEntryAddressGet (entryIndex,
				configRegWordOnEntry,
				qEntrySizeInwords,
				qSizeInWords,
				&entryAddress);

    /* Get the lock or return busy */
    if (IX_SUCCESS != ixOsalFastMutexTryLock(&ixQMgrAqmIfPeekPokeFastMutex[qId]))
    {
	return IX_FAIL;
    }

    while(qEntrySizeInwords--)
    {
	ixQMgrAqmIfWordRead (entryAddress++, entry++);
    }

    /* Release the lock */
    ixOsalFastMutexUnlock(&ixQMgrAqmIfPeekPokeFastMutex[qId]);

    /* Read the config register */
    ixQMgrAqmIfWordRead (cfgRegAddress, &configRegWordOnExit);

    /* Check that the read and write pointers have not changed */
    if (configRegWordOnEntry != configRegWordOnExit)
    {
	return IX_FAIL;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrAqmIfQPoke (IxQMgrQId qId,
		  unsigned entryIndex,
		  unsigned int *entry)
{
    UINT32 *cfgRegAddress = (UINT32*)(aqmBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    UINT32 *entryAddress = NULL;
    UINT32 configRegWordOnEntry;
    UINT32 configRegWordOnExit;
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;
    
    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    /* Read the config register */
    ixQMgrAqmIfWordRead (cfgRegAddress, &configRegWordOnEntry);

    /* Get the entry address */
    ixQMgrAqmIfEntryAddressGet (entryIndex,
				configRegWordOnEntry,
				qEntrySizeInwords,
				qSizeInWords,
				&entryAddress);

    /* Get the lock or return busy */
    if (IX_SUCCESS != ixOsalFastMutexTryLock(&ixQMgrAqmIfPeekPokeFastMutex[qId]))
    {
	return IX_FAIL;
    }

    /* Else read the entry directly from SRAM. This will not move the read pointer */
    while(qEntrySizeInwords--)
    {
	ixQMgrAqmIfWordWrite (entryAddress++, *entry++);
    }

    /* Release the lock */
    ixOsalFastMutexUnlock(&ixQMgrAqmIfPeekPokeFastMutex[qId]);

    /* Read the config register */
    ixQMgrAqmIfWordRead (cfgRegAddress, &configRegWordOnExit);

    /* Check that the read and write pointers have not changed */
    if (configRegWordOnEntry != configRegWordOnExit)
    {
	return IX_FAIL;
    }

    return IX_SUCCESS;
}

PRIVATE unsigned
watermarkToAqmWatermark (IxQMgrWMLevel watermark )
{
    unsigned aqmWatermark = 0;

    /*
     * Watermarks 0("000"),1("001"),2("010"),4("011"),
     * 8("100"),16("101"),32("110"),64("111")
     */
    aqmWatermark = ixQMgrAqmIfLog2 (watermark * 2);
    
    return aqmWatermark;
}

PRIVATE unsigned
entrySizeToAqmEntrySize (IxQMgrQEntrySizeInWords entrySize)
{
    /* entrySize  1("00"),2("01"),4("10") */
    return (ixQMgrAqmIfLog2 (entrySize));
}

PRIVATE unsigned
bufferSizeToAqmBufferSize (unsigned bufferSizeInWords)
{
    /* bufferSize 16("00"),32("01),64("10"),128("11") */
    return (ixQMgrAqmIfLog2 (bufferSizeInWords / IX_QMGR_MIN_BUFFER_SIZE));
}

/*
 * Reset AQM registers to default values.
 */
PRIVATE void
ixQMgrAqmIfRegistersReset (void)
{
    volatile UINT32 *qConfigWordAddress = NULL;
    unsigned int i;

    /*
     * Need to initialize AQM hardware registers to an initial
     * value as init may have been called as a result of a soft
     * reset. i.e. soft reset does not reset hardware registers.
     */

    /* Reset queues 0..31 status registers 0..3 */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUELOWSTAT0_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUELOWSTAT1_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUELOWSTAT2_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUELOWSTAT3_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);

    /* Reset underflow/overflow status registers 0..1 */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEUOSTAT0_OFFSET), 
			 IX_QMGR_QUEUOSTAT_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEUOSTAT1_OFFSET), 
			 IX_QMGR_QUEUOSTAT_RESET_VALUE);
    
    /* Reset queues 32..63 nearly empty status registers */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEUPPSTAT0_OFFSET),
			 IX_QMGR_QUEUPPSTAT0_RESET_VALUE);

    /* Reset queues 32..63 full status registers */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEUPPSTAT1_OFFSET),
			 IX_QMGR_QUEUPPSTAT1_RESET_VALUE);

    /* Reset int0 status flag source select registers 0..3 */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_INT0SRCSELREG0_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_INT0SRCSELREG1_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_INT0SRCSELREG2_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_INT0SRCSELREG3_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
	 
    /* Reset queue interrupt enable register 0..1 */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEIEREG0_OFFSET),
			 IX_QMGR_QUEIEREG_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QUEIEREG1_OFFSET),
			 IX_QMGR_QUEIEREG_RESET_VALUE);

    /* Reset queue interrupt register 0..1 */
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QINTREG0_OFFSET),
			 IX_QMGR_QINTREG_RESET_VALUE);
    ixQMgrAqmIfWordWrite((UINT32 *)(aqmBaseAddress + IX_QMGR_QINTREG1_OFFSET),
			 IX_QMGR_QINTREG_RESET_VALUE);

    /* Reset queue configuration words 0..63 */
    qConfigWordAddress = (UINT32 *)(aqmBaseAddress + IX_QMGR_QUECONFIG_BASE_OFFSET);
    for (i = 0; i < (IX_QMGR_QUECONFIG_SIZE / sizeof(UINT32)); i++)
    {
	ixQMgrAqmIfWordWrite(qConfigWordAddress,
			     IX_QMGR_QUECONFIG_RESET_VALUE);
	/* Next word */
	qConfigWordAddress++;
    }
}

