/**
 * @file IxNpeDlNpeMgrUtils.c
 *
 * @author Intel Corporation
 * @date 18 February 2002
 *
 * @brief This file contains the implementation of the private API for the 
 *        IXP425 NPE Downloader NpeMgr Utils module
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
 * Put the system defined include files required.
 */
#define IX_NPE_DL_MAX_NUM_OF_RETRIES 1000000 /**< Maximum number of
                                              * retries before
                                              * timeout
					                          */  

/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxNpeDl.h"
#include "IxNpeDlNpeMgrUtils_p.h"
#include "IxNpeDlNpeMgrEcRegisters_p.h"
#include "IxNpeDlMacros_p.h"

/*
 * #defines and macros used in this file.
 */

/* used to bit-mask a number of bytes */
#define IX_NPEDL_MASK_LOWER_BYTE_OF_WORD  0x000000FF
#define IX_NPEDL_MASK_LOWER_SHORT_OF_WORD 0x0000FFFF
#define IX_NPEDL_MASK_FULL_WORD           0xFFFFFFFF

#define IX_NPEDL_BYTES_PER_WORD           4
#define IX_NPEDL_BYTES_PER_SHORT          2

#define IX_NPEDL_REG_SIZE_BYTE            8
#define IX_NPEDL_REG_SIZE_SHORT           16
#define IX_NPEDL_REG_SIZE_WORD            32

/*
 * Introduce extra read cycles after issuing read command to NPE
 * so that we read the register after the NPE has updated it
 * This is to overcome race condition between XScale and NPE
 */
#define IX_NPEDL_DELAY_READ_CYCLES        2
/*
 * To mask top three MSBs of 32bit word to download into NPE IMEM
 */
#define IX_NPEDL_MASK_UNUSED_IMEM_BITS    0x1FFFFFFF;


/*
 * typedefs
 */
typedef struct
{
    UINT32 regAddress;
    UINT32 regSize;
} IxNpeDlCtxtRegAccessInfo;

/* module statistics counters */
typedef struct
{
    UINT32 insMemWrites;
    UINT32 insMemWriteFails;
    UINT32 dataMemWrites;
    UINT32 dataMemWriteFails;
    UINT32 ecsRegWrites;
    UINT32 ecsRegReads;
    UINT32 dbgInstructionExecs;
    UINT32 contextRegWrites;
    UINT32 physicalRegWrites;
    UINT32 nextPcWrites;
} IxNpeDlNpeMgrUtilsStats;


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* 
 * contains useful address and function pointers to read/write Context Regs, 
 * eliminating some switch or if-else statements in places
 */
static IxNpeDlCtxtRegAccessInfo ixNpeDlCtxtRegAccInfo[IX_NPEDL_CTXT_REG_MAX] =
{
    {
	IX_NPEDL_CTXT_REG_ADDR_STEVT,
	IX_NPEDL_REG_SIZE_BYTE
    },
    {
	IX_NPEDL_CTXT_REG_ADDR_STARTPC,
	IX_NPEDL_REG_SIZE_SHORT
    },
    {
	IX_NPEDL_CTXT_REG_ADDR_REGMAP,
	IX_NPEDL_REG_SIZE_SHORT
    },
    {
	IX_NPEDL_CTXT_REG_ADDR_CINDEX,
	IX_NPEDL_REG_SIZE_BYTE
    }
};

static UINT32 ixNpeDlSavedExecCount = 0;
static UINT32 ixNpeDlSavedEcsDbgCtxtReg2 = 0;

static IxNpeDlNpeMgrUtilsStats ixNpeDlNpeMgrUtilsStats;


/*
 * static function prototypes.
 */
PRIVATE __inline__ void
ixNpeDlNpeMgrWriteCommandIssue (UINT32 npeBaseAddress, UINT32 cmd,
				UINT32 addr, UINT32 data);

PRIVATE __inline__ UINT32
ixNpeDlNpeMgrReadCommandIssue (UINT32 npeBaseAddress, UINT32 cmd, UINT32 addr);

PRIVATE IX_STATUS
ixNpeDlNpeMgrLogicalRegRead (UINT32 npeBaseAddress, UINT32 regAddr,
			     UINT32 regSize, UINT32 ctxtNum, UINT32 *regVal);

PRIVATE IX_STATUS
ixNpeDlNpeMgrLogicalRegWrite (UINT32 npeBaseAddress, UINT32 regAddr,
			      UINT32 regVal, UINT32 regSize,
			      UINT32 ctxtNum, BOOL verify);

/*
 * Function definition: ixNpeDlNpeMgrWriteCommandIssue
 */
PRIVATE __inline__ void
ixNpeDlNpeMgrWriteCommandIssue (
    UINT32 npeBaseAddress,
    UINT32 cmd,
    UINT32 addr,
    UINT32 data)
{
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXDATA, data);
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXAD, addr);
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCTL, cmd);
}


/*
 * Function definition: ixNpeDlNpeMgrReadCommandIssue
 */
PRIVATE __inline__ UINT32
ixNpeDlNpeMgrReadCommandIssue (
    UINT32 npeBaseAddress,
    UINT32 cmd,
    UINT32 addr)
{
    UINT32 data = 0;
    int i;

    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXAD, addr);
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCTL, cmd);
    for (i = 0; i <= IX_NPEDL_DELAY_READ_CYCLES; i++)
    {
	IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXDATA, &data);
    }

    return data;
}

/*
 * Function definition: ixNpeDlNpeMgrInsMemWrite
 */
IX_STATUS
ixNpeDlNpeMgrInsMemWrite (
    UINT32 npeBaseAddress,
    UINT32 insMemAddress,
    UINT32 insMemData,
    BOOL verify)
{
    UINT32 insMemDataRtn;

    ixNpeDlNpeMgrWriteCommandIssue (npeBaseAddress,
				    IX_NPEDL_EXCTL_CMD_WR_INS_MEM,
				    insMemAddress, insMemData);
    if (verify)
    {
        /* write invalid data to this reg, so we can see if we're reading 
	   the EXDATA register too early */
	IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXDATA,
			    ~insMemData);

        /*Disabled since top 3 MSB are not used for Azusa hardware Refer WR:IXA00053900*/
        insMemData&=IX_NPEDL_MASK_UNUSED_IMEM_BITS;

        insMemDataRtn=ixNpeDlNpeMgrReadCommandIssue (npeBaseAddress,
                                           IX_NPEDL_EXCTL_CMD_RD_INS_MEM,
                                           insMemAddress);

        insMemDataRtn&=IX_NPEDL_MASK_UNUSED_IMEM_BITS;

	if (insMemData != insMemDataRtn)
	{
	    ixNpeDlNpeMgrUtilsStats.insMemWriteFails++;
	    return IX_FAIL;
	}
    }

    ixNpeDlNpeMgrUtilsStats.insMemWrites++;
    return IX_SUCCESS;
}


/*
 * Function definition: ixNpeDlNpeMgrDataMemWrite
 */
IX_STATUS
ixNpeDlNpeMgrDataMemWrite (
    UINT32 npeBaseAddress,
    UINT32 dataMemAddress,
    UINT32 dataMemData,
    BOOL verify)
{
    ixNpeDlNpeMgrWriteCommandIssue (npeBaseAddress,
				    IX_NPEDL_EXCTL_CMD_WR_DATA_MEM,
				    dataMemAddress, dataMemData);
    if (verify)
    {
        /* write invalid data to this reg, so we can see if we're reading 
	   the EXDATA register too early */
	IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXDATA, ~dataMemData);

	if (dataMemData !=
	    ixNpeDlNpeMgrReadCommandIssue (npeBaseAddress,
					   IX_NPEDL_EXCTL_CMD_RD_DATA_MEM,
					   dataMemAddress))
	{
	    ixNpeDlNpeMgrUtilsStats.dataMemWriteFails++;
	    return IX_FAIL;
	}
    }

    ixNpeDlNpeMgrUtilsStats.dataMemWrites++;
    return IX_SUCCESS;
}


/*
 * Function definition: ixNpeDlNpeMgrExecAccRegWrite
 */
void
ixNpeDlNpeMgrExecAccRegWrite (
    UINT32 npeBaseAddress,
    UINT32 regAddress,
    UINT32 regData)
{
    ixNpeDlNpeMgrWriteCommandIssue (npeBaseAddress,
				    IX_NPEDL_EXCTL_CMD_WR_ECS_REG,
				    regAddress, regData);
    ixNpeDlNpeMgrUtilsStats.ecsRegWrites++;
}


/*
 * Function definition: ixNpeDlNpeMgrExecAccRegRead
 */
UINT32
ixNpeDlNpeMgrExecAccRegRead (
    UINT32 npeBaseAddress,
    UINT32 regAddress)
{
    ixNpeDlNpeMgrUtilsStats.ecsRegReads++;
    return ixNpeDlNpeMgrReadCommandIssue (npeBaseAddress,
					  IX_NPEDL_EXCTL_CMD_RD_ECS_REG,
					  regAddress);
}


/*
 * Function definition: ixNpeDlNpeMgrCommandIssue
 */
void
ixNpeDlNpeMgrCommandIssue (
    UINT32 npeBaseAddress,
    UINT32 command)     
{
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrCommandIssue\n");

    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCTL, command);

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrCommandIssue\n");
}


/*
 * Function definition: ixNpeDlNpeMgrDebugInstructionPreExec
 */
void
ixNpeDlNpeMgrDebugInstructionPreExec(
    UINT32 npeBaseAddress)
{
    /* turn off the halt bit by clearing Execution Count register. */
    /* save reg contents 1st and restore later */
    IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCT,
		       &ixNpeDlSavedExecCount);
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCT, 0);

    /* ensure that IF and IE are on (temporarily), so that we don't end up
     * stepping forever */
    ixNpeDlSavedEcsDbgCtxtReg2 = ixNpeDlNpeMgrExecAccRegRead (npeBaseAddress,
				                   IX_NPEDL_ECS_DBG_CTXT_REG_2);

    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_2,
				  (ixNpeDlSavedEcsDbgCtxtReg2 |
				   IX_NPEDL_MASK_ECS_DBG_REG_2_IF |
				   IX_NPEDL_MASK_ECS_DBG_REG_2_IE));
}


/*
 * Function definition: ixNpeDlNpeMgrDebugInstructionExec
 */
IX_STATUS
ixNpeDlNpeMgrDebugInstructionExec(
    UINT32 npeBaseAddress,
    UINT32 npeInstruction,
    UINT32 ctxtNum,
    UINT32 ldur)
{
    UINT32 ecsDbgRegVal;
    UINT32 oldWatchcount, newWatchcount;
    UINT32 retriesCount = 0;
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrDebugInstructionExec\n");

    /* set the Active bit, and the LDUR, in the debug level */
    ecsDbgRegVal = IX_NPEDL_MASK_ECS_REG_0_ACTIVE |
	(ldur << IX_NPEDL_OFFSET_ECS_REG_0_LDUR);

    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_0,
				  ecsDbgRegVal);

    /*
     * set CCTXT at ECS DEBUG L3 to specify in which context to execute the
     * instruction, and set SELCTXT at ECS DEBUG Level to specify which context
     * store to access.
     * Debug ECS Level Reg 1 has form  0x000n000n, where n = context number
     */
    ecsDbgRegVal = (ctxtNum << IX_NPEDL_OFFSET_ECS_REG_1_CCTXT) |
	(ctxtNum << IX_NPEDL_OFFSET_ECS_REG_1_SELCTXT);

    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_1,
				  ecsDbgRegVal);

    /* clear the pipeline */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);

    /* load NPE instruction into the instruction register */
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_INSTRUCT_REG,
				  npeInstruction);

    /* we need this value later to wait for completion of NPE execution step */
    IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_WC, &oldWatchcount);

    /* issue a Step One command via the Execution Control register */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_STEP);

	/* Watch Count register increments when NPE completes an instruction */
	IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_WC,
        &newWatchcount);
        
    /*
     * force the XScale to wait until the NPE has finished execution step
     * NOTE that this delay will be very small, just long enough to allow a
     * single NPE instruction to complete execution; if instruction execution
     * is not completed before timeout retries, exit the while loop
     */
    while ((IX_NPE_DL_MAX_NUM_OF_RETRIES > retriesCount) 
        && (newWatchcount == oldWatchcount))
    {
	    /* Watch Count register increments when NPE completes an instruction */
	    IX_NPEDL_REG_READ (npeBaseAddress, IX_NPEDL_REG_OFFSET_WC,
		    &newWatchcount);
			   
        retriesCount++;
    }    

    if (IX_NPE_DL_MAX_NUM_OF_RETRIES > retriesCount)
    {
        ixNpeDlNpeMgrUtilsStats.dbgInstructionExecs++;
    }
    else
    {
        /* Return timeout status as the instruction has not been executed
         * after maximum retries */
        status = IX_NPEDL_CRITICAL_NPE_ERR;
    }
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrDebugInstructionExec\n");
		     
    return status;
}    


/*
 * Function definition: ixNpeDlNpeMgrDebugInstructionPostExec
 */
void
ixNpeDlNpeMgrDebugInstructionPostExec(
    UINT32 npeBaseAddress)
{
    /* clear active bit in debug level */
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_0,
				  0);

    /* clear the pipeline */
    ixNpeDlNpeMgrCommandIssue (npeBaseAddress, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);
    
    /* restore Execution Count register contents. */
    IX_NPEDL_REG_WRITE (npeBaseAddress, IX_NPEDL_REG_OFFSET_EXCT,
			ixNpeDlSavedExecCount);

    /* restore IF and IE bits to original values */
    ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress, IX_NPEDL_ECS_DBG_CTXT_REG_2,
				  ixNpeDlSavedEcsDbgCtxtReg2);
}


/*
 * Function definition: ixNpeDlNpeMgrLogicalRegRead
 */
PRIVATE IX_STATUS
ixNpeDlNpeMgrLogicalRegRead (
    UINT32 npeBaseAddress, 
    UINT32 regAddr,
    UINT32 regSize,
    UINT32 ctxtNum,
    UINT32 *regVal)
{
    IX_STATUS status = IX_SUCCESS;
    UINT32 npeInstruction = 0;
    UINT32 mask = 0;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrLogicalRegRead\n");

    switch (regSize)
    {
    case IX_NPEDL_REG_SIZE_BYTE:
      npeInstruction = IX_NPEDL_INSTR_RD_REG_BYTE;
      mask = IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;  break;
    case IX_NPEDL_REG_SIZE_SHORT:
      npeInstruction = IX_NPEDL_INSTR_RD_REG_SHORT;
      mask = IX_NPEDL_MASK_LOWER_SHORT_OF_WORD;  break;
    case IX_NPEDL_REG_SIZE_WORD:
      npeInstruction = IX_NPEDL_INSTR_RD_REG_WORD;
      mask = IX_NPEDL_MASK_FULL_WORD;  break;
    }

    /* make regAddr be the SRC and DEST operands (e.g. movX d0, d0) */
    npeInstruction |= (regAddr << IX_NPEDL_OFFSET_INSTR_SRC) |
	(regAddr << IX_NPEDL_OFFSET_INSTR_DEST);

    /* step execution of NPE intruction using Debug Executing Context stack */
    status = ixNpeDlNpeMgrDebugInstructionExec (npeBaseAddress, npeInstruction,
				       ctxtNum, IX_NPEDL_RD_INSTR_LDUR);

    if (IX_SUCCESS != status)
    {
        return status;
    }
    
    /* read value of register from Execution Data register */
    IX_NPEDL_REG_READ (npeBaseAddress,	IX_NPEDL_REG_OFFSET_EXDATA, regVal);

   /* align value from left to right */
    *regVal = (*regVal >> (IX_NPEDL_REG_SIZE_WORD - regSize)) & mask;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrLogicalRegRead\n");
    
    return IX_SUCCESS;
}


/*
 * Function definition: ixNpeDlNpeMgrLogicalRegWrite
 */
PRIVATE IX_STATUS
ixNpeDlNpeMgrLogicalRegWrite (
    UINT32 npeBaseAddress, 
    UINT32 regAddr,
    UINT32 regVal,
    UINT32 regSize,
    UINT32 ctxtNum,
    BOOL verify)
{
    UINT32 npeInstruction = 0;
    UINT32 mask = 0;
    IX_STATUS status = IX_SUCCESS;
    UINT32 retRegVal;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrLogicalRegWrite\n");

    if (regSize == IX_NPEDL_REG_SIZE_WORD)
    {
	/* NPE register addressing is left-to-right: e.g. |d0|d1|d2|d3| */
	/* Write upper half-word (short) to |d0|d1| */
	status = ixNpeDlNpeMgrLogicalRegWrite (npeBaseAddress, regAddr,
				      regVal >> IX_NPEDL_REG_SIZE_SHORT,
				      IX_NPEDL_REG_SIZE_SHORT,
				      ctxtNum, verify);
				      
	if (IX_SUCCESS != status)
	{
	    return status;
	}
	
	/* Write lower half-word (short) to |d2|d3| */
	status = ixNpeDlNpeMgrLogicalRegWrite (npeBaseAddress,
				      regAddr + IX_NPEDL_BYTES_PER_SHORT,
                                    regVal & IX_NPEDL_MASK_LOWER_SHORT_OF_WORD,
				      IX_NPEDL_REG_SIZE_SHORT,
				      ctxtNum, verify);
    
    if (IX_SUCCESS != status)
	{
	    return status;
	}
	}
    else
    {
        switch (regSize)
	{ 
	case IX_NPEDL_REG_SIZE_BYTE:
	    npeInstruction = IX_NPEDL_INSTR_WR_REG_BYTE;
	    mask = IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;  break;
	case IX_NPEDL_REG_SIZE_SHORT:
            npeInstruction = IX_NPEDL_INSTR_WR_REG_SHORT;
	    mask = IX_NPEDL_MASK_LOWER_SHORT_OF_WORD;  break;
	}
	/* mask out any redundant bits, so verify will work later */
	regVal &= mask;

	/* fill dest operand field of  instruction with destination reg addr */
	npeInstruction |= (regAddr << IX_NPEDL_OFFSET_INSTR_DEST);

	/* fill src operand field of instruction with least-sig 5 bits of val*/
	npeInstruction |= ((regVal & IX_NPEDL_MASK_IMMED_INSTR_SRC_DATA) <<
			   IX_NPEDL_OFFSET_INSTR_SRC);

	/* fill coprocessor field of instruction with most-sig 11 bits of val*/
	npeInstruction |= ((regVal & IX_NPEDL_MASK_IMMED_INSTR_COPROC_DATA) <<
			   IX_NPEDL_DISPLACE_IMMED_INSTR_COPROC_DATA);

	/* step execution of NPE intruction using Debug ECS */
	status = ixNpeDlNpeMgrDebugInstructionExec(npeBaseAddress, npeInstruction,
					  ctxtNum, IX_NPEDL_WR_INSTR_LDUR);
					  
	if (IX_SUCCESS != status)
	{
	    return status;  
	} 
    }/* condition: if reg to be written is 8-bit or 16-bit (not 32-bit) */

    if (verify)
    {
	status = ixNpeDlNpeMgrLogicalRegRead (npeBaseAddress, regAddr,
						   regSize, ctxtNum, &retRegVal);
						   
        if (IX_SUCCESS == status)
        {
            if (regVal != retRegVal)
            {
                status = IX_FAIL;
            }
        }        
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrLogicalRegWrite : status = %d\n",
		     status);
    
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrPhysicalRegWrite
 */
IX_STATUS
ixNpeDlNpeMgrPhysicalRegWrite (
    UINT32 npeBaseAddress,
    UINT32 regAddr,
    UINT32 regValue,
    BOOL verify)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeMgrPhysicalRegWrite\n");

/*
 * There are 32 physical registers used in an NPE.  These are
 * treated as 16 pairs of 32-bit registers.  To write one of the pair,
 * write the pair number (0-16) to the REGMAP for Context 0.  Then write
 * the value to register  0 or 4 in the regfile, depending on which
 * register of the pair is to be written
 */

    /*
     * set REGMAP for context 0 to (regAddr >> 1) to choose which pair (0-16)
     * of physical registers to write 
     */
    status = ixNpeDlNpeMgrLogicalRegWrite (npeBaseAddress,
					   IX_NPEDL_CTXT_REG_ADDR_REGMAP,
					   (regAddr >>
					  IX_NPEDL_OFFSET_PHYS_REG_ADDR_REGMAP),
					   IX_NPEDL_REG_SIZE_SHORT, 0, verify);
    if (status == IX_SUCCESS)
    {
	/* regAddr = 0 or 4  */
	regAddr = (regAddr & IX_NPEDL_MASK_PHYS_REG_ADDR_LOGICAL_ADDR) *
	    IX_NPEDL_BYTES_PER_WORD;
    
    status = ixNpeDlNpeMgrLogicalRegWrite (npeBaseAddress, regAddr, regValue, 
					   IX_NPEDL_REG_SIZE_WORD, 0, verify);
    }
    
    if (status != IX_SUCCESS)
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrPhysicalRegWrite: "
			       "error writing to physical register\n");
    }

    ixNpeDlNpeMgrUtilsStats.physicalRegWrites++;

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeMgrPhysicalRegWrite : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrCtxtRegWrite
 */
IX_STATUS
ixNpeDlNpeMgrCtxtRegWrite (
    UINT32 npeBaseAddress,
    UINT32 ctxtNum,
    IxNpeDlCtxtRegNum ctxtReg,
    UINT32 ctxtRegVal,
    BOOL verify)
{
    UINT32 tempRegVal;
    UINT32 ctxtRegAddr;
    UINT32 ctxtRegSize;
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlNpeMgrCtxtRegWrite\n");

    /*
     * Context 0 has no STARTPC. Instead, this value is used to set
     * NextPC for Background ECS, to set where NPE starts executing code
     */
    if ((ctxtNum == 0) && (ctxtReg == IX_NPEDL_CTXT_REG_STARTPC))
    {
	/* read BG_CTXT_REG_0, update NEXTPC bits, and write back to reg */
	tempRegVal = ixNpeDlNpeMgrExecAccRegRead (npeBaseAddress,
						  IX_NPEDL_ECS_BG_CTXT_REG_0);
	tempRegVal &= ~IX_NPEDL_MASK_ECS_REG_0_NEXTPC;
	tempRegVal |= (ctxtRegVal << IX_NPEDL_OFFSET_ECS_REG_0_NEXTPC) &
	    IX_NPEDL_MASK_ECS_REG_0_NEXTPC;
	
	ixNpeDlNpeMgrExecAccRegWrite (npeBaseAddress,
				      IX_NPEDL_ECS_BG_CTXT_REG_0, tempRegVal);

	ixNpeDlNpeMgrUtilsStats.nextPcWrites++;
    }
    else
    {
	ctxtRegAddr = ixNpeDlCtxtRegAccInfo[ctxtReg].regAddress;
	ctxtRegSize = ixNpeDlCtxtRegAccInfo[ctxtReg].regSize;
	status = ixNpeDlNpeMgrLogicalRegWrite (npeBaseAddress, ctxtRegAddr,
					       ctxtRegVal, ctxtRegSize,
					       ctxtNum, verify);
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeMgrCtxtRegWrite: "
				   "error writing to context store register\n");
	}
	
	ixNpeDlNpeMgrUtilsStats.contextRegWrites++;
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlNpeMgrCtxtRegWrite : status = %d\n",
		     status);

    return status;
}


/*
 * Function definition: ixNpeDlNpeMgrUtilsStatsShow
 */
void
ixNpeDlNpeMgrUtilsStatsShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\nixNpeDlNpeMgrUtilsStatsShow:\n"
               "\tInstruction Memory writes: %u\n"
               "\tInstruction Memory writes failed: %u\n"
               "\tData Memory writes: %u\n"
               "\tData Memory writes failed: %u\n",
               ixNpeDlNpeMgrUtilsStats.insMemWrites,
               ixNpeDlNpeMgrUtilsStats.insMemWriteFails,
               ixNpeDlNpeMgrUtilsStats.dataMemWrites,
               ixNpeDlNpeMgrUtilsStats.dataMemWriteFails,
               0,0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\tExecuting Context Stack Register writes: %u\n"
               "\tExecuting Context Stack Register reads: %u\n"
               "\tPhysical Register writes: %u\n"
               "\tContext Store Register writes: %u\n"
               "\tExecution Backgound Context NextPC writes: %u\n"
               "\tDebug Instructions Executed: %u\n\n",
               ixNpeDlNpeMgrUtilsStats.ecsRegWrites,
               ixNpeDlNpeMgrUtilsStats.ecsRegReads,
               ixNpeDlNpeMgrUtilsStats.physicalRegWrites,
               ixNpeDlNpeMgrUtilsStats.contextRegWrites,
               ixNpeDlNpeMgrUtilsStats.nextPcWrites,
               ixNpeDlNpeMgrUtilsStats.dbgInstructionExecs);
}


/*
 * Function definition: ixNpeDlNpeMgrUtilsStatsReset
 */
void
ixNpeDlNpeMgrUtilsStatsReset (void)
{
    ixNpeDlNpeMgrUtilsStats.insMemWrites = 0;
    ixNpeDlNpeMgrUtilsStats.insMemWriteFails = 0;
    ixNpeDlNpeMgrUtilsStats.dataMemWrites = 0;
    ixNpeDlNpeMgrUtilsStats.dataMemWriteFails = 0;
    ixNpeDlNpeMgrUtilsStats.ecsRegWrites = 0;
    ixNpeDlNpeMgrUtilsStats.ecsRegReads = 0;
    ixNpeDlNpeMgrUtilsStats.physicalRegWrites = 0;
    ixNpeDlNpeMgrUtilsStats.contextRegWrites = 0;
    ixNpeDlNpeMgrUtilsStats.nextPcWrites = 0;
    ixNpeDlNpeMgrUtilsStats.dbgInstructionExecs = 0;
}
