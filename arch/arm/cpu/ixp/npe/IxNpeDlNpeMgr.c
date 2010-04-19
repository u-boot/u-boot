/**
 * @file IxNpeDlNpeMgr.c
 *
 * @author Intel Corporation
 * @date 09 January 2002
 *
 * @brief This file contains the implementation of the private API for the
 *        IXP425 NPE Downloader NpeMgr module
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


/*
 * Put the user defined include files required.
 */


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxNpeDl.h"
#include "IxNpeDlNpeMgr_p.h"
#include "IxNpeDlNpeMgrUtils_p.h"
#include "IxNpeDlNpeMgrEcRegisters_p.h"
#include "IxNpeDlMacros_p.h"
#include "IxFeatureCtrl.h"

/*
 * #defines and macros used in this file.
 */
#define IX_NPEDL_BYTES_PER_WORD                   4

/* used to read download map from version in microcode image */
#define IX_NPEDL_BLOCK_TYPE_INSTRUCTION           0x00000000
#define IX_NPEDL_BLOCK_TYPE_DATA                  0x00000001
#define IX_NPEDL_BLOCK_TYPE_STATE                 0x00000002
#define IX_NPEDL_END_OF_DOWNLOAD_MAP              0x0000000F

/*
 * masks used to extract address info from State information context
 * register addresses as read from microcode image 
 */
#define IX_NPEDL_MASK_STATE_ADDR_CTXT_REG         0x0000000F
#define IX_NPEDL_MASK_STATE_ADDR_CTXT_NUM         0x000000F0

/* LSB offset of Context Number field in State-Info Context Address */
#define IX_NPEDL_OFFSET_STATE_ADDR_CTXT_NUM       4

/* size (in words) of single State Information entry (ctxt reg address|data) */
#define IX_NPEDL_STATE_INFO_ENTRY_SIZE            2


 #define IX_NPEDL_RESET_NPE_PARITY  0x0800
 #define IX_NPEDL_PARITY_BIT_MASK   0x3F00FFFF
 #define IX_NPEDL_CONFIG_CTRL_REG_MASK  0x3F3FFFFF


/*
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    UINT32 type;
    UINT32 offset;
} IxNpeDlNpeMgrDownloadMapBlockEntry;

typedef union
{
    IxNpeDlNpeMgrDownloadMapBlockEntry block;
    UINT32 eodmMarker;
} IxNpeDlNpeMgrDownloadMapEntry;

typedef struct
{
    /* 1st entry in the download map (there may be more than one) */
    IxNpeDlNpeMgrDownloadMapEntry entry[1];
} IxNpeDlNpeMgrDownloadMap;


/* used to access an instruction or data block in a microcode image */
typedef struct
{
    UINT32 npeMemAddress;
    UINT32 size;
    UINT32 data[1];
} IxNpeDlNpeMgrCodeBlock;

/* used to access each Context Reg entry state-information block */
typedef struct
{
    UINT32 addressInfo;
    UINT32 value;
} IxNpeDlNpeMgrStateInfoCtxtRegEntry;

/* used to access a state-information block in a microcode image */
typedef struct
{
    UINT32 size;
    IxNpeDlNpeMgrStateInfoCtxtRegEntry ctxtRegEntry[1];
} IxNpeDlNpeMgrStateInfoBlock;
 
/* used to store some useful NPE information for easy access */
typedef struct
{
    UINT32 baseAddress;
    UINT32 insMemSize;
    UINT32 dataMemSize;
} IxNpeDlNpeInfo;

/* used to distinguish instruction and data memory operations */
typedef enum 
{
  IX_NPEDL_MEM_TYPE_INSTRUCTION = 0,
  IX_NPEDL_MEM_TYPE_DATA
} IxNpeDlNpeMemType;

/* used to hold a reset value for a particular ECS register */
typedef struct
{
    UINT32 regAddr;
    UINT32 regResetVal;
} IxNpeDlEcsRegResetValue;

/* prototype of function to write either Instruction or Data memory */
typedef IX_STATUS (*IxNpeDlNpeMgrMemWrite) (UINT32 npeBaseAddress,
					    UINT32 npeMemAddress,
					    UINT32 npeMemData,
					    BOOL verify);

/* module statistics counters */
typedef struct
{
    UINT32 instructionBlocksLoaded;
    UINT32 dataBlocksLoaded;
    UINT32 stateInfoBlocksLoaded;
    UINT32 criticalNpeErrors;
    UINT32 criticalMicrocodeErrors;
    UINT32 npeStarts;
    UINT32 npeStops;
    UINT32 npeResets;
} IxNpeDlNpeMgrStats;


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static IxNpeDlNpeInfo ixNpeDlNpeInfo[] =
{
    {
	0,
	IX_NPEDL_INS_MEMSIZE_WORDS_NPEA,
	IX_NPEDL_DATA_MEMSIZE_WORDS_NPEA
    },
    {
	0,
	IX_NPEDL_INS_MEMSIZE_WORDS_NPEB,
	IX_NPEDL_DATA_MEMSIZE_WORDS_NPEB
    },
    {
	0,
	IX_NPEDL_INS_MEMSIZE_WORDS_NPEC,
	IX_NPEDL_DATA_MEMSIZE_WORDS_NPEC
    }
};

/* contains Reset values for Context Store Registers  */
static UINT32 ixNpeDlCtxtRegResetValues[] =
{
    IX_NPEDL_CTXT_REG_RESET_STEVT,
    IX_NPEDL_CTXT_REG_RESET_STARTPC,
    IX_NPEDL_CTXT_REG_RESET_REGMAP,
    IX_NPEDL_CTXT_REG_RESET_CINDEX,
};

/* contains Reset values for Context Store Registers  */
static IxNpeDlEcsRegResetValue ixNpeDlEcsRegResetValues[] =
{
    {IX_NPEDL_ECS_BG_CTXT_REG_0,    IX_NPEDL_ECS_BG_CTXT_REG_0_RESET},
    {IX_NPEDL_ECS_BG_CTXT_REG_1,    IX_NPEDL_ECS_BG_CTXT_REG_1_RESET},
    {IX_NPEDL_ECS_BG_CTXT_REG_2,    IX_NPEDL_ECS_BG_CTXT_REG_2_RESET},
    {IX_NPEDL_ECS_PRI_1_CTXT_REG_0, IX_NPEDL_ECS_PRI_1_CTXT_REG_0_RESET},
    {IX_NPEDL_ECS_PRI_1_CTXT_REG_1, IX_NPEDL_ECS_PRI_1_CTXT_REG_1_RESET},
    {IX_NPEDL_ECS_PRI_1_CTXT_REG_2, IX_NPEDL_ECS_PRI_1_CTXT_REG_2_RESET},
    {IX_NPEDL_ECS_PRI_2_CTXT_REG_0, IX_NPEDL_ECS_PRI_2_CTXT_REG_0_RESET},
    {IX_NPEDL_ECS_PRI_2_CTXT_REG_1, IX_NPEDL_ECS_PRI_2_CTXT_REG_1_RESET},
    {IX_NPEDL_ECS_PRI_2_CTXT_REG_2, IX_NPEDL_ECS_PRI_2_CTXT_REG_2_RESET},
    {IX_NPEDL_ECS_DBG_CTXT_REG_0,   IX_NPEDL_ECS_DBG_CTXT_REG_0_RESET},
    {IX_NPEDL_ECS_DBG_CTXT_REG_1,   IX_NPEDL_ECS_DBG_CTXT_REG_1_RESET},
    {IX_NPEDL_ECS_DBG_CTXT_REG_2,   IX_NPEDL_ECS_DBG_CTXT_REG_2_RESET},
    {IX_NPEDL_ECS_INSTRUCT_REG,     IX_NPEDL_ECS_INSTRUCT_REG_RESET}
};

static IxNpeDlNpeMgrStats ixNpeDlNpeMgrStats;

/* Set when NPE register memory has been mapped */
static BOOL ixNpeDlMemInitialised = FALSE;


/*
 * static function prototypes.
 */
PRIVATE IX_STATUS
ixNpeDlNpeMgrMemLoad (IxNpeDlNpeId npeId, UINT32 npeBaseAddress,
		      IxNpeDlNpeMgrCodeBlock *codeBlockPtr,
		      BOOL verify, IxNpeDlNpeMemType npeMemType);
PRIVATE IX_STATUS
ixNpeDlNpeMgrStateInfoLoad (UINT32 npeBaseAddress,
			    IxNpeDlNpeMgrStateInfoBlock *codeBlockPtr,
			    BOOL verify);
PRIVATE BOOL
ixNpeDlNpeMgrBitsSetCheck (UINT32 npeBaseAddress, UINT32 regOffset,
			   UINT32 expectedBitsSet);

PRIVATE UINT32
ixNpeDlNpeMgrBaseAddressGet (IxNpeDlNpeId npeId);

/*
 * Function definition: ixNpeDlNpeMgrBaseAddressGet
 */
PRIVATE UINT32
ixNpeDlNpeMgrBaseAddressGet (IxNpeDlNpeId npeId)
{
    IX_OSAL_ASSERT (ixNpeDlMemInitialised);
    return ixNpeDlNpeInfo[npeId].baseAddress;
}


/*
 * Function definition: ixNpeDlNpeMgrInit
 */
void
ixNpeDlNpeMgrInit (void)
{
    /* Only map the memory once */
    if (!ixNpeDlMemInitialised)
    {
	UINT32 virtAddr;

	/* map the register memory for NPE-A */
	virtAddr = (UINT32) IX_OSAL_MEM_MAP (IX_NPEDL_NPEBASEADDRESS_NPEA,
					    IX_OSAL_IXP400_NPEA_MAP_SIZE); 
	IX_OSAL_ASSERT(virtAddr);
	ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEA].baseAddress = virtAddr;

	/* map the register memory for NPE-B */
	virtAddr = (UINT32) IX_OSAL_MEM_MAP (IX_NPEDL_NPEBASEADDRESS_NPEB,
					    IX_OSAL_IXP400_NPEB_MAP_SIZE); 
	IX_OSAL_ASSERT(virtAddr);
	ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEB].baseAddress = virtAddr;

	/* map the register memory for NPE-C */
	virtAddr = (UINT32) IX_OSAL_MEM_MAP (IX_NPEDL_NPEBASEADDRESS_NPEC,
					    IX_OSAL_IXP400_NPEC_MAP_SIZE); 
	IX_OSAL_ASSERT(virtAddr);
	ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEC].baseAddress = virtAddr;

	ixNpeDlMemInitialised = TRUE;
    }
}


/*
 * Function definition: ixNpeDlNpeMgrUninit
 */
IX_STATUS
ixNpeDlNpeMgrUninit (void)
{
    if (!ixNpeDlMemInitialised)
    {
	return IX_FAIL;
    }

    IX_OSAL_MEM_UNMAP (ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEA].baseAddress);
    IX_OSAL_MEM_UNMAP (ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEB].baseAddress);
    IX_OSAL_MEM_UNMAP (ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEC].baseAddress);

    ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEA].baseAddress = 0;
    ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEB].baseAddress = 0;
    ixNpeDlNpeInfo[IX_NPEDL_NPEID_NPEC].baseAddress = 0;

    ixNpeDlMemInitialised = FALSE;

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeDlNpeMgrImageLoad
 */
IX_STATUS
ixNpeDlNpeMgrImageLoad (
    IxNpeDlNpeId npeId,
    UINT32 *imageCodePtr,
    BOOL verify)
{
    UINT32 npeBaseAddress;
    IxNpeDlNpeMgrDownloadMap *downloadMap;
    UINT32 *blockPtr;
    UINT32 mapIndex = 0;
    IX_STATUS status = IX_SUCCESS;
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrImageLoad\n");

    /* get base memory address of NPE from npeId */
    npeBaseAddress = ixNpeDlNpeMgrBaseAddressGet (npeId);

    /* check execution status of NPE to verify NPE Stop was successful */
    if (!ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCTL,
				    IX_NPEDL_EXCTL_STATUS_STOP))
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrImageDownload - "
			       "NPE was not stopped before download\n");
	status = IX_FAIL;
    }
    else
    {
	/*
	 * Read Download Map, checking each block type and calling
	 * appropriate function to perform download 
	 */
	downloadMap = (IxNpeDlNpeMgrDownloadMap *) imageCodePtr;
	while ((downloadMap->entry[mapIndex].eodmMarker != 
		IX_NPEDL_END_OF_DOWNLOAD_MAP)
	       && (status == IX_SUCCESS))
	{
	    /* calculate pointer to block to be downloaded */
	    blockPtr = imageCodePtr +
		downloadMap->entry[mapIndex].block.offset;

	    switch (downloadMap->entry[mapIndex].block.type)
	    {
	    case IX_NPEDL_BLOCK_TYPE_INSTRUCTION:
		status = ixNpeDlNpeMgrMemLoad (npeId, npeBaseAddress, 
					     (IxNpeDlNpeMgrCodeBlock *)blockPtr,
					       verify,
					       IX_NPEDL_MEM_TYPE_INSTRUCTION);
		break;
	    case IX_NPEDL_BLOCK_TYPE_DATA:
		status = ixNpeDlNpeMgrMemLoad (npeId, npeBaseAddress,
                                             (IxNpeDlNpeMgrCodeBlock *)blockPtr,
					       verify, IX_NPEDL_MEM_TYPE_DATA);
		break;
	    case IX_NPEDL_BLOCK_TYPE_STATE:
		status = ixNpeDlNpeMgrStateInfoLoad (npeBaseAddress,
				       (IxNpeDlNpeMgrStateInfoBlock *) blockPtr,
						     verify);
		break;
	    default:
		IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrImageLoad: "
				       "unknown block type in download map\n");
		status = IX_NPEDL_CRITICAL_MICROCODE_ERR;
		ixNpeDlNpeMgrStats.criticalMicrocodeErrors++;
		break;
	    }
	    mapIndex++;
	}/* loop: for each entry in download map, while status == SUCCESS */
    }/* condition: NPE stopped before attempting download */
    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlNpeMgrImageLoad : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrMemLoad
 */
PRIVATE IX_STATUS
ixNpeDlNpeMgrMemLoad (
    IxNpeDlNpeId npeId,
    UINT32 npeBaseAddress,
    IxNpeDlNpeMgrCodeBlock *blockPtr,
    BOOL verify,
    IxNpeDlNpeMemType npeMemType)
{
    UINT32 npeMemAddress;
    UINT32 blockSize;
    UINT32 memSize = 0;
    IxNpeDlNpeMgrMemWrite memWriteFunc = NULL;
    UINT32 localIndex = 0;
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrMemLoad\n");
    
    /*
     * select NPE EXCTL reg read/write commands depending on memory
     * type (instruction/data) to be accessed
     */
    if (npeMemType == IX_NPEDL_MEM_TYPE_INSTRUCTION)
    {
	memSize = ixNpeDlNpeInfo[npeId].insMemSize;
	memWriteFunc = (IxNpeDlNpeMgrMemWrite) ixNpeDlNpeMgrInsMemWrite;
    }
    else if (npeMemType == IX_NPEDL_MEM_TYPE_DATA)
    {
	memSize = ixNpeDlNpeInfo[npeId].dataMemSize;
	memWriteFunc = (IxNpeDlNpeMgrMemWrite) ixNpeDlNpeMgrDataMemWrite;
    }

    /*
     * NPE memory is loaded contiguously from each block, so only address
     * of 1st word in block is needed
     */
    npeMemAddress = blockPtr->npeMemAddress;
    /* number of words of instruction/data microcode in block to download */
    blockSize = blockPtr->size;
    if ((npeMemAddress + blockSize) > memSize)
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrMemLoad: "
			       "Block size too big for NPE memory\n");
	status = IX_NPEDL_CRITICAL_MICROCODE_ERR;
	ixNpeDlNpeMgrStats.criticalMicrocodeErrors++;
    }
    else
    {
	for (localIndex = 0; localIndex < blockSize; localIndex++)
	{
	    status = memWriteFunc (npeBaseAddress, npeMemAddress,
				   blockPtr->data[localIndex], verify);

	    if (status != IX_SUCCESS)
	    {
		IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrMemLoad: "
				       "write to NPE memory failed\n");
		status = IX_NPEDL_CRITICAL_NPE_ERR;
		ixNpeDlNpeMgrStats.criticalNpeErrors++;
		break;   /* abort download */
	    }
	    /* increment target (word)address in NPE memory */
	    npeMemAddress++;   
	}
    }/* condition: block size will fit in NPE memory */

    if (status == IX_SUCCESS)
    {
	if (npeMemType == IX_NPEDL_MEM_TYPE_INSTRUCTION)
	{
	    ixNpeDlNpeMgrStats.instructionBlocksLoaded++;
	}
	else if (npeMemType == IX_NPEDL_MEM_TYPE_DATA)
	{
	    ixNpeDlNpeMgrStats.dataBlocksLoaded++;
	}
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrMemLoad : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrStateInfoLoad
 */
PRIVATE IX_STATUS
ixNpeDlNpeMgrStateInfoLoad (
    UINT32 npeBaseAddress,
    IxNpeDlNpeMgrStateInfoBlock *blockPtr,
    BOOL verify)
{
    UINT32 blockSize;
    UINT32 ctxtRegAddrInfo; 
    UINT32 ctxtRegVal;
    IxNpeDlCtxtRegNum ctxtReg; /* identifies Context Store reg (0-3) */
    UINT32 ctxtNum;            /* identifies Context number (0-16)   */
    UINT32 i;
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrStateInfoLoad\n");

    /* block size contains number of words of state-info in block */
    blockSize = blockPtr->size;
    
    ixNpeDlNpeMgrDebugInstructionPreExec (npeBaseAddress);

    /* for each state-info context register entry in block */
    for (i = 0; i < (blockSize/IX_NPEDL_STATE_INFO_ENTRY_SIZE); i++)
    {
	/* each state-info entry is 2 words (address, value) in length */
	ctxtRegAddrInfo = (blockPtr->ctxtRegEntry[i]).addressInfo;
	ctxtRegVal      = (blockPtr->ctxtRegEntry[i]).value;
	
	ctxtReg = (ctxtRegAddrInfo & IX_NPEDL_MASK_STATE_ADDR_CTXT_REG);
	ctxtNum = (ctxtRegAddrInfo & IX_NPEDL_MASK_STATE_ADDR_CTXT_NUM) >> 
	    IX_NPEDL_OFFSET_STATE_ADDR_CTXT_NUM;
	
	/* error-check Context Register No. and Context Number values  */
	/* NOTE that there is no STEVT register for Context 0 */
	if ((ctxtReg < 0) ||
	    (ctxtReg >= IX_NPEDL_CTXT_REG_MAX) ||
	    (ctxtNum > IX_NPEDL_CTXT_NUM_MAX) ||
	    ((ctxtNum == 0) && (ctxtReg == IX_NPEDL_CTXT_REG_STEVT)))
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrStateInfoLoad: "
				   "invalid Context Register Address\n");
	    status = IX_NPEDL_CRITICAL_MICROCODE_ERR;
	    ixNpeDlNpeMgrStats.criticalMicrocodeErrors++;
	    break;   /* abort download */
	}    
	
	status = ixNpeDlNpeMgrCtxtRegWrite (npeBaseAddress, ctxtNum, ctxtReg,
					    ctxtRegVal, verify);
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrStateInfoLoad: "
				   "write of state-info to NPE failed\n");
	    status = IX_NPEDL_CRITICAL_NPE_ERR;
	    ixNpeDlNpeMgrStats.criticalNpeErrors++;
	    break;   /* abort download */
	}
    }/* loop: for each context reg entry in State Info block */
    
    ixNpeDlNpeMgrDebugInstructionPostExec (npeBaseAddress);

    if (status == IX_SUCCESS)
    {
	ixNpeDlNpeMgrStats.stateInfoBlocksLoaded++;
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrStateInfoLoad : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrNpeReset
 */
IX_STATUS
ixNpeDlNpeMgrNpeReset (
    IxNpeDlNpeId npeId)
{
    UINT32 npeBaseAddress;
    IxNpeDlCtxtRegNum ctxtReg; /* identifies Context Store reg (0-3) */
    UINT32 ctxtNum;            /* identifies Context number (0-16)   */
    UINT32 regAddr;
    UINT32 regVal;
    UINT32 localIndex;
    UINT32 indexMax;
    IX_STATUS status = IX_SUCCESS;
    IxFeatureCtrlReg unitFuseReg;
    UINT32 ixNpeConfigCtrlRegVal;
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlNpeMgrNpeReset\n");
    
    /* get base memory address of NPE from npeId */
    npeBaseAddress = ixNpeDlNpeMgrBaseAddressGet (npeId);

    /* pre-store the NPE Config Control Register Value */
    IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_CTL, &ixNpeConfigCtrlRegVal);
    
    ixNpeConfigCtrlRegVal |= 0x3F000000;
    
    /* disable the parity interrupt */
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_CTL, (ixNpeConfigCtrlRegVal & IX_NPEDL_PARITY_BIT_MASK));
    
    ixNpeDlNpeMgrDebugInstructionPreExec (npeBaseAddress);

    /*
     * clear the FIFOs
     */
    while (ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress,
				      IX_NPEDL_REG_OFFSET_WFIFO,
				      IX_NPEDL_MASK_WFIFO_VALID))
    {
	/* read from the Watch-point FIFO until empty */
	IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_WFIFO,
			   &regVal);
    }
    
    while (ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress,
					  IX_NPEDL_REG_OFFSET_STAT,
				      IX_NPEDL_MASK_STAT_OFNE))
    {
	/* read from the outFIFO until empty */
	IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_FIFO,
			   &regVal);
    }
    
    while (ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress,
				      IX_NPEDL_REG_OFFSET_STAT,
				      IX_NPEDL_MASK_STAT_IFNE))
    {
	/*
	 * step execution of the NPE intruction to read inFIFO using
	 * the Debug Executing Context stack
	 */
	status = ixNpeDlNpeMgrDebugInstructionExec (npeBaseAddress,
					   IX_NPEDL_INSTR_RD_FIFO, 0, 0);

    if (IX_SUCCESS != status)
    {
        return status;   
    }
    
    }
    
    /*
     * Reset the mailbox reg
     */
    /* ...from XScale side */
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_MBST,
			IX_NPEDL_REG_RESET_MBST);
    /* ...from NPE side */
    status = ixNpeDlNpeMgrDebugInstructionExec (npeBaseAddress,
				       IX_NPEDL_INSTR_RESET_MBOX, 0, 0);

    if (IX_SUCCESS != status)
    {
        return status;   
    }

    /* 
     *   Reset the physical registers in the NPE register file:
     *   Note: no need to save/restore REGMAP for Context 0 here
     *   since all Context Store regs are reset in subsequent code
     */
    for (regAddr = 0;
	 (regAddr < IX_NPEDL_TOTAL_NUM_PHYS_REG) && (status != IX_FAIL);
	 regAddr++)
    {
	/* for each physical register in the NPE reg file, write 0 : */
	status = ixNpeDlNpeMgrPhysicalRegWrite (npeBaseAddress, regAddr,
						0, TRUE);
	if (status != IX_SUCCESS)
	{
	    return status;  /* abort reset */
	}
    }
    

    /*
     * Reset the context store:
     */
    for (ctxtNum = IX_NPEDL_CTXT_NUM_MIN;
	 ctxtNum <= IX_NPEDL_CTXT_NUM_MAX; ctxtNum++)
    {	
	/* set each context's Context Store registers to reset values: */
	for (ctxtReg = 0; ctxtReg < IX_NPEDL_CTXT_REG_MAX; ctxtReg++)
	{
	    /* NOTE that there is no STEVT register for Context 0 */
	    if (!((ctxtNum == 0) && (ctxtReg == IX_NPEDL_CTXT_REG_STEVT)))
	    { 
		regVal = ixNpeDlCtxtRegResetValues[ctxtReg];
		status = ixNpeDlNpeMgrCtxtRegWrite (npeBaseAddress, ctxtNum,
						    ctxtReg, regVal, TRUE);
		if (status != IX_SUCCESS)
		{
		    return status;  /* abort reset */
		}
	    }
	}
    }

    ixNpeDlNpeMgrDebugInstructionPostExec (npeBaseAddress);

    /* write Reset values to Execution Context Stack registers */
    indexMax = sizeof (ixNpeDlEcsRegResetValues) /
	sizeof (IxNpeDlEcsRegResetValue);
    for (localIndex = 0; localIndex < indexMax; localIndex++)
    {
	regAddr = ixNpeDlEcsRegResetValues[localIndex].regAddr;
	regVal = ixNpeDlEcsRegResetValues[localIndex].regResetVal;
	ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, regAddr, regVal);
    }
    
    /* clear the profile counter */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, 
			       IX_NPEDL_EXCTL_CMD_CLR_PROFILE_CNT);
    
    /* clear registers EXCT, AP0, AP1, AP2 and AP3 */
    for (regAddr = IX_NPEDL_REG_OFFSET_EXCT;
	     regAddr <= IX_NPEDL_REG_OFFSET_AP3;
	 regAddr += IX_NPEDL_BYTES_PER_WORD)
    {
	IX_NPEDL_REG_WRITE (npeBaseAddress, regAddr, 0);
    }
    
    /* Reset the Watch-count register */
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_WC, 0);
    
    /*
     * WR IXA00055043 - Remove IMEM Parity Introduced by NPE Reset Operation
     */

    /*
     * Call the feature control API to fused out and reset the NPE and its
     * coprocessor - to reset internal states and remove parity error
     */
    unitFuseReg = ixFeatureCtrlRead ();
    unitFuseReg |= (IX_NPEDL_RESET_NPE_PARITY << npeId);
    ixFeatureCtrlWrite (unitFuseReg);

    /* call the feature control API to un-fused and un-reset the NPE & COP */
    unitFuseReg &= (~(IX_NPEDL_RESET_NPE_PARITY << npeId));
    ixFeatureCtrlWrite (unitFuseReg);

    /*
     * Call NpeMgr function to stop the NPE again after the Feature Control
     * has unfused and Un-Reset the NPE and its associated Coprocessors
     */
    status = ixNpeDlNpeMgrNpeStop (npeId);

    /* restore NPE configuration bus Control Register - Parity Settings  */
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_CTL, 
        (ixNpeConfigCtrlRegVal & IX_NPEDL_CONFIG_CTRL_REG_MASK));

    ixNpeDlNpeMgrStats.npeResets++;

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrNpeReset : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrNpeStart
 */
IX_STATUS
ixNpeDlNpeMgrNpeStart (
    IxNpeDlNpeId npeId)
{
    UINT32    npeBaseAddress;
    UINT32    ecsRegVal;
    BOOL      npeRunning;
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlNpeMgrNpeStart\n");

    /* get base memory address of NPE from npeId */
    npeBaseAddress = ixNpeDlNpeMgrBaseAddressGet (npeId);

    /*
     * ensure only Background Context Stack Level is Active by turning off
     * the Active bit in each of the other Executing Context Stack levels
     */
    ecsRegVal = ixNpeDlNpeMgrExecAccRegRead (npeBaseAddress,
					     IX_NPEDL_ECS_PRI_1_CTXT_REG_0);
    ecsRegVal &= ~IX_NPEDL_MASK_ECS_REG_0_ACTIVE;
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_PRI_1_CTXT_REG_0,
				  ecsRegVal);

    ecsRegVal = ixNpeDlNpeMgrExecAccRegRead (npeBaseAddress,
					     IX_NPEDL_ECS_PRI_2_CTXT_REG_0);
    ecsRegVal &= ~IX_NPEDL_MASK_ECS_REG_0_ACTIVE;
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_PRI_2_CTXT_REG_0,
				  ecsRegVal);

    ecsRegVal = ixNpeDlNpeMgrExecAccRegRead (npeBaseAddress,
					     IX_NPEDL_ECS_DBG_CTXT_REG_0);
    ecsRegVal &= ~IX_NPEDL_MASK_ECS_REG_0_ACTIVE;
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_0,
				  ecsRegVal);
    
    /* clear the pipeline */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);
    
    /* start NPE execution by issuing command through EXCTL register on NPE */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_START);

    /*
     * check execution status of NPE to verify NPE Start operation was
     * successful
     */
    npeRunning = ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress,
					    IX_NPEDL_REG_OFFSET_EXCTL,
					    IX_NPEDL_EXCTL_STATUS_RUN);
    if (npeRunning)
    {
	ixNpeDlNpeMgrStats.npeStarts++;
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrNpeStart: "
			       "failed to start NPE execution\n");
	status = IX_FAIL;
    }

    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrNpeStart : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrNpeStop
 */
IX_STATUS
ixNpeDlNpeMgrNpeStop (
    IxNpeDlNpeId npeId)
{
    UINT32    npeBaseAddress;
    IX_STATUS status = IX_SUCCESS;
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrNpeStop\n");
    
    /* get base memory address of NPE from npeId */
    npeBaseAddress = ixNpeDlNpeMgrBaseAddressGet (npeId);

    /* stop NPE execution by issuing command through EXCTL register on NPE */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_STOP);

    /* verify that NPE Stop was successful */
    if (! ixNpeDlNpeMgrBitsSetCheck (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCTL,
				     IX_NPEDL_EXCTL_STATUS_STOP))
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrNpeStop: "
			       "failed to stop NPE execution\n");
	status = IX_FAIL;
    }

    ixNpeDlNpeMgrStats.npeStops++;
    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrNpeStop : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrBitsSetCheck
 */
PRIVATE BOOL
ixNpeDlNpeMgrBitsSetCheck (
    UINT32 npeBaseAddress,
    UINT32 regOffset,
    UINT32 expectedBitsSet)
{
    UINT32 regVal;
    IX_NPEDL_REG_READ (npeBaseAddress, regOffset, &regVal);

    return expectedBitsSet == (expectedBitsSet & regVal);
}


/*
 * Function definition: ixNpeDlNpeMgrStatsShow
 */
void
ixNpeDlNpeMgrStatsShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\nixNpeDlNpeMgrStatsShow:\n"
               "\tInstruction Blocks loaded: %u\n"
               "\tData Blocks loaded: %u\n"
               "\tState Information Blocks loaded: %u\n"
               "\tCritical NPE errors: %u\n"
               "\tCritical Microcode errors: %u\n",
               ixNpeDlNpeMgrStats.instructionBlocksLoaded,
               ixNpeDlNpeMgrStats.dataBlocksLoaded,
               ixNpeDlNpeMgrStats.stateInfoBlocksLoaded,
               ixNpeDlNpeMgrStats.criticalNpeErrors,
               ixNpeDlNpeMgrStats.criticalMicrocodeErrors,
               0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\tSuccessful NPE Starts: %u\n"
               "\tSuccessful NPE Stops: %u\n"
               "\tSuccessful NPE Resets: %u\n\n",
               ixNpeDlNpeMgrStats.npeStarts,
               ixNpeDlNpeMgrStats.npeStops,
               ixNpeDlNpeMgrStats.npeResets,
               0,0,0);

    ixNpeDlNpeMgrUtilsStatsShow ();
}


/*
 * Function definition: ixNpeDlNpeMgrStatsReset
 */
void
ixNpeDlNpeMgrStatsReset (void)
{
    ixNpeDlNpeMgrStats.instructionBlocksLoaded = 0;
    ixNpeDlNpeMgrStats.dataBlocksLoaded = 0;
    ixNpeDlNpeMgrStats.stateInfoBlocksLoaded = 0;
    ixNpeDlNpeMgrStats.criticalNpeErrors = 0;
    ixNpeDlNpeMgrStats.criticalMicrocodeErrors = 0;
    ixNpeDlNpeMgrStats.npeStarts = 0;
    ixNpeDlNpeMgrStats.npeStops = 0;
    ixNpeDlNpeMgrStats.npeResets = 0;

    ixNpeDlNpeMgrUtilsStatsReset ();
}
