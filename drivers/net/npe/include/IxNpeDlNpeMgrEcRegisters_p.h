/**
 * @file IxNpeDlNpeMgrEcRegisters_p.h
 *
 * @author Intel Corporation
 * @date 14 December 2001

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


#ifndef IXNPEDLNPEMGRECREGISTERS_P_H
#define IXNPEDLNPEMGRECREGISTERS_P_H

#include "IxOsal.h"

/*
 * Base Memory Addresses for accessing NPE registers
 */

#define IX_NPEDL_NPE_BASE (IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE)

#define IX_NPEDL_NPEA_OFFSET (0x6000) /**< NPE-A register base offset */
#define IX_NPEDL_NPEB_OFFSET (0x7000) /**< NPE-B register base offset */
#define IX_NPEDL_NPEC_OFFSET (0x8000) /**< NPE-C register base offset */

/**
 * @def IX_NPEDL_NPEBASEADDRESS_NPEA
 * @brief Base Memory Address of NPE-A Configuration Bus registers
 */
#define IX_NPEDL_NPEBASEADDRESS_NPEA (IX_NPEDL_NPE_BASE + IX_NPEDL_NPEA_OFFSET)

/**
 * @def IX_NPEDL_NPEBASEADDRESS_NPEB
 * @brief Base Memory Address of NPE-B Configuration Bus registers
 */
#define IX_NPEDL_NPEBASEADDRESS_NPEB (IX_NPEDL_NPE_BASE + IX_NPEDL_NPEB_OFFSET)

/**
 * @def IX_NPEDL_NPEBASEADDRESS_NPEC
 * @brief Base Memory Address of NPE-C Configuration Bus registers
 */
#define IX_NPEDL_NPEBASEADDRESS_NPEC (IX_NPEDL_NPE_BASE + IX_NPEDL_NPEC_OFFSET)


/*
 * Instruction Memory Size (in words) for each NPE 
 */

/**
 * @def IX_NPEDL_INS_MEMSIZE_WORDS_NPEA
 * @brief Size (in words) of NPE-A Instruction Memory
 */
#define IX_NPEDL_INS_MEMSIZE_WORDS_NPEA     4096

/**
 * @def IX_NPEDL_INS_MEMSIZE_WORDS_NPEB
 * @brief Size (in words) of NPE-B Instruction Memory
 */
#define IX_NPEDL_INS_MEMSIZE_WORDS_NPEB     2048

/**
 * @def IX_NPEDL_INS_MEMSIZE_WORDS_NPEC
 * @brief Size (in words) of NPE-B Instruction Memory
 */
#define IX_NPEDL_INS_MEMSIZE_WORDS_NPEC     2048


/*
 * Data Memory Size (in words) for each NPE 
 */

/**
 * @def IX_NPEDL_DATA_MEMSIZE_WORDS_NPEA
 * @brief Size (in words) of NPE-A Data Memory
 */
#define IX_NPEDL_DATA_MEMSIZE_WORDS_NPEA    2048

/**
 * @def IX_NPEDL_DATA_MEMSIZE_WORDS_NPEB
 * @brief Size (in words) of NPE-B Data Memory
 */
#define IX_NPEDL_DATA_MEMSIZE_WORDS_NPEB    2048

/**
 * @def IX_NPEDL_DATA_MEMSIZE_WORDS_NPEC
 * @brief Size (in words) of NPE-C Data Memory
 */
#define IX_NPEDL_DATA_MEMSIZE_WORDS_NPEC    2048


/*
 * Configuration Bus Register offsets (in bytes) from NPE Base Address
 */

/**
 * @def IX_NPEDL_REG_OFFSET_EXAD
 * @brief Offset (in bytes) of EXAD (Execution Address) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_EXAD             0x00000000		

/**
 * @def IX_NPEDL_REG_OFFSET_EXDATA
 * @brief Offset (in bytes) of EXDATA (Execution Data) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_EXDATA           0x00000004

/**
 * @def IX_NPEDL_REG_OFFSET_EXCTL
 * @brief Offset (in bytes) of EXCTL (Execution Control) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_EXCTL            0x00000008		

/**
 * @def IX_NPEDL_REG_OFFSET_EXCT
 * @brief Offset (in bytes) of EXCT (Execution Count) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_EXCT 	     0x0000000C		

/**
 * @def IX_NPEDL_REG_OFFSET_AP0
 * @brief Offset (in bytes) of AP0 (Action Point 0) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_AP0	             0x00000010		

/**
 * @def IX_NPEDL_REG_OFFSET_AP1
 * @brief Offset (in bytes) of AP1 (Action Point 1) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_AP1	             0x00000014		

/**
 * @def IX_NPEDL_REG_OFFSET_AP2
 * @brief Offset (in bytes) of AP2 (Action Point 2) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_AP2	             0x00000018		

/**
 * @def IX_NPEDL_REG_OFFSET_AP3
 * @brief Offset (in bytes) of AP3 (Action Point 3) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_AP3	             0x0000001C		

/**
 * @def IX_NPEDL_REG_OFFSET_WFIFO
 * @brief Offset (in bytes) of WFIFO (Watchpoint FIFO) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_WFIFO            0x00000020

/**
 * @def IX_NPEDL_REG_OFFSET_WC
 * @brief Offset (in bytes) of WC (Watch Count) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_WC	             0x00000024 		

/**
 * @def IX_NPEDL_REG_OFFSET_PROFCT
 * @brief Offset (in bytes) of PROFCT (Profile Count) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_PROFCT           0x00000028		

/**
 * @def IX_NPEDL_REG_OFFSET_STAT
 * @brief Offset (in bytes) of STAT (Messaging Status) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_STAT	     0x0000002C		

/**
 * @def IX_NPEDL_REG_OFFSET_CTL
 * @brief Offset (in bytes) of CTL (Messaging Control) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_CTL	             0x00000030		

/**
 * @def IX_NPEDL_REG_OFFSET_MBST
 * @brief Offset (in bytes) of MBST (Mailbox Status) register from NPE Base
 *        Address
 */
#define IX_NPEDL_REG_OFFSET_MBST	     0x00000034		

/**
 * @def IX_NPEDL_REG_OFFSET_FIFO
 * @brief Offset (in bytes) of FIFO (messaging in/out FIFO) register from NPE
 *        Base Address
 */
#define IX_NPEDL_REG_OFFSET_FIFO	     0x00000038		


/*
 * Non-zero reset values for the Configuration Bus registers
 */

/**
 * @def IX_NPEDL_REG_RESET_FIFO
 * @brief Reset value for Mailbox (MBST) register
 *        NOTE that if used, it should be complemented with an NPE intruction
 *        to clear the Mailbox at the NPE side as well
 */
#define IX_NPEDL_REG_RESET_MBST              0x0000F0F0


/*
 * Bit-masks used to read/write particular bits in Configuration Bus registers
 */

/**
 * @def IX_NPEDL_MASK_WFIFO_VALID
 * @brief Masks the VALID bit in the WFIFO register
 */
#define IX_NPEDL_MASK_WFIFO_VALID            0x80000000

/**
 * @def IX_NPEDL_MASK_STAT_OFNE
 * @brief Masks the OFNE bit in the STAT register
 */
#define IX_NPEDL_MASK_STAT_OFNE              0x00010000

/**
 * @def IX_NPEDL_MASK_STAT_IFNE
 * @brief Masks the IFNE bit in the STAT register
 */
#define IX_NPEDL_MASK_STAT_IFNE              0x00080000


/*
 * EXCTL (Execution Control) Register commands 
*/

/**
 * @def IX_NPEDL_EXCTL_CMD_NPE_STEP
 * @brief EXCTL Command to Step execution of an NPE Instruction
 */

#define IX_NPEDL_EXCTL_CMD_NPE_STEP          0x01

/**
 * @def IX_NPEDL_EXCTL_CMD_NPE_START
 * @brief EXCTL Command to Start NPE execution
 */
#define IX_NPEDL_EXCTL_CMD_NPE_START         0x02

/**
 * @def IX_NPEDL_EXCTL_CMD_NPE_STOP
 * @brief EXCTL Command to Stop NPE execution
 */
#define IX_NPEDL_EXCTL_CMD_NPE_STOP          0x03

/**
 * @def IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE
 * @brief EXCTL Command to Clear NPE instruction pipeline
 */
#define IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE      0x04

/**
 * @def IX_NPEDL_EXCTL_CMD_RD_INS_MEM
 * @brief EXCTL Command to read NPE instruction memory at address in EXAD
 *        register and return value in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_RD_INS_MEM        0x10

/**
 * @def IX_NPEDL_EXCTL_CMD_WR_INS_MEM
 * @brief EXCTL Command to write NPE instruction memory at address in EXAD
 *        register with data in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_WR_INS_MEM        0x11

/**
 * @def IX_NPEDL_EXCTL_CMD_RD_DATA_MEM
 * @brief EXCTL Command to read NPE data memory at address in EXAD
 *        register and return value in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_RD_DATA_MEM       0x12

/**
 * @def IX_NPEDL_EXCTL_CMD_WR_DATA_MEM
 * @brief EXCTL Command to write NPE data memory at address in EXAD
 *        register with data in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_WR_DATA_MEM       0x13

/**
 * @def IX_NPEDL_EXCTL_CMD_RD_ECS_REG
 * @brief EXCTL Command to read Execution Access register at address in EXAD
 *        register and return value in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_RD_ECS_REG        0x14

/**
 * @def IX_NPEDL_EXCTL_CMD_WR_ECS_REG
 * @brief EXCTL Command to write Execution Access register at address in EXAD
 *        register with data in EXDATA register
 */
#define IX_NPEDL_EXCTL_CMD_WR_ECS_REG        0x15

/**
 * @def IX_NPEDL_EXCTL_CMD_CLR_PROFILE_CNT
 * @brief EXCTL Command to clear Profile Count register
 */
#define IX_NPEDL_EXCTL_CMD_CLR_PROFILE_CNT   0x0C


/*
 * EXCTL (Execution Control) Register status bit masks
 */

/**
 * @def IX_NPEDL_EXCTL_STATUS_RUN
 * @brief Masks the RUN status bit in the EXCTL register
 */
#define IX_NPEDL_EXCTL_STATUS_RUN            0x80000000

/**
 * @def IX_NPEDL_EXCTL_STATUS_STOP
 * @brief Masks the STOP status bit in the EXCTL register
 */
#define IX_NPEDL_EXCTL_STATUS_STOP           0x40000000

/**
 * @def IX_NPEDL_EXCTL_STATUS_CLEAR
 * @brief Masks the CLEAR status bit in the EXCTL register
 */
#define IX_NPEDL_EXCTL_STATUS_CLEAR          0x20000000

/**
 * @def IX_NPEDL_EXCTL_STATUS_ECS_K
 * @brief Masks the K (pipeline Klean) status bit in the EXCTL register
 */
#define IX_NPEDL_EXCTL_STATUS_ECS_K          0x00800000


/*
 * Executing Context Stack (ECS) level registers 
 */

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_0
 * @brief Execution Access register address for register 0 at Backgound 
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_0           0x00

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_1
 * @brief Execution Access register address for register 1 at Backgound 
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_1           0x01

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_2
 * @brief Execution Access register address for register 2 at Backgound 
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_2           0x02

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_0
 * @brief Execution Access register address for register 0 at Priority 1
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_0        0x04

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_1
 * @brief Execution Access register address for register 1 at Priority 1
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_1        0x05

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_2
 * @brief Execution Access register address for register 2 at Priority 1
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_2        0x06

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_0
 * @brief Execution Access register address for register 0 at Priority 2
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_0        0x08

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_1
 * @brief Execution Access register address for register 1 at Priority 2
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_1        0x09

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_2
 * @brief Execution Access register address for register 2 at Priority 2
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_2        0x0A

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_0
 * @brief Execution Access register address for register 0 at Debug
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_0          0x0C

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_1
 * @brief Execution Access register address for register 1 at Debug 
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_1          0x0D

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_2
 * @brief Execution Access register address for register 2 at Debug 
 *        Executing Context Stack level
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_2          0x0E

/**
 * @def IX_NPEDL_ECS_INSTRUCT_REG
 * @brief Execution Access register address for NPE Instruction Register 
 */
#define IX_NPEDL_ECS_INSTRUCT_REG            0x11


/*
 * Execution Access register reset values
 */

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_0_RESET
 * @brief Reset value for Execution Access Background ECS level register 0
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_0_RESET     0xA0000000

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_1_RESET
 * @brief Reset value for Execution Access Background ECS level register 1
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_1_RESET     0x01000000

/**
 * @def IX_NPEDL_ECS_BG_CTXT_REG_2_RESET
 * @brief Reset value for Execution Access Background ECS level register 2
 */
#define IX_NPEDL_ECS_BG_CTXT_REG_2_RESET     0x00008000

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_0_RESET
 * @brief Reset value for Execution Access Priority 1 ECS level register 0
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_0_RESET  0x20000080

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_1_RESET
 * @brief Reset value for Execution Access Priority 1 ECS level register 1
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_1_RESET  0x01000000

/**
 * @def IX_NPEDL_ECS_PRI_1_CTXT_REG_2_RESET
 * @brief Reset value for Execution Access Priority 1 ECS level register 2
 */
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_2_RESET  0x00008000

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_0_RESET
 * @brief Reset value for Execution Access Priority 2 ECS level register 0
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_0_RESET  0x20000080

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_1_RESET
 * @brief Reset value for Execution Access Priority 2 ECS level register 1
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_1_RESET  0x01000000

/**
 * @def IX_NPEDL_ECS_PRI_2_CTXT_REG_2_RESET
 * @brief Reset value for Execution Access Priority 2 ECS level register 2
 */
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_2_RESET  0x00008000

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_0_RESET
 * @brief Reset value for Execution Access Debug ECS level register 0
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_0_RESET    0x20000000

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_1_RESET
 * @brief Reset value for Execution Access Debug ECS level register 1
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_1_RESET    0x00000000

/**
 * @def IX_NPEDL_ECS_DBG_CTXT_REG_2_RESET
 * @brief Reset value for Execution Access Debug ECS level register 2
 */
#define IX_NPEDL_ECS_DBG_CTXT_REG_2_RESET    0x001E0000

/**
 * @def IX_NPEDL_ECS_INSTRUCT_REG_RESET
 * @brief Reset value for Execution Access NPE Instruction Register
 */
#define IX_NPEDL_ECS_INSTRUCT_REG_RESET      0x1003C00F


/*
 * masks used to read/write particular bits in Execution Access registers
 */

/**
 * @def IX_NPEDL_MASK_ECS_REG_0_ACTIVE
 * @brief Mask the A (Active) bit in Execution Access Register 0 of all ECS
 *        levels
 */
#define IX_NPEDL_MASK_ECS_REG_0_ACTIVE       0x80000000

/**
 * @def IX_NPEDL_MASK_ECS_REG_0_NEXTPC
 * @brief Mask the NextPC bits in Execution Access Register 0 of all ECS
 *        levels (except Debug ECS level)
 */
#define IX_NPEDL_MASK_ECS_REG_0_NEXTPC       0x1FFF0000

/**
 * @def IX_NPEDL_MASK_ECS_REG_0_LDUR
 * @brief Mask the LDUR bits in Execution Access Register 0 of all ECS levels
 */
#define IX_NPEDL_MASK_ECS_REG_0_LDUR         0x00000700

/**
 * @def IX_NPEDL_MASK_ECS_REG_1_CCTXT
 * @brief Mask the NextPC bits in Execution Access Register 1 of all ECS levels
 */
#define IX_NPEDL_MASK_ECS_REG_1_CCTXT        0x000F0000

/**
 * @def IX_NPEDL_MASK_ECS_REG_1_SELCTXT
 * @brief Mask the NextPC bits in Execution Access Register 1 of all ECS levels
 */
#define IX_NPEDL_MASK_ECS_REG_1_SELCTXT      0x0000000F

/**
 * @def IX_NPEDL_MASK_ECS_DBG_REG_2_IF
 * @brief Mask the IF bit in Execution Access Register 2 of Debug ECS level
 */
#define IX_NPEDL_MASK_ECS_DBG_REG_2_IF       0x00100000

/**
 * @def IX_NPEDL_MASK_ECS_DBG_REG_2_IE
 * @brief Mask the IE bit in Execution Access Register 2 of Debug ECS level
 */
#define IX_NPEDL_MASK_ECS_DBG_REG_2_IE       0x00080000


/*
 * Bit-Offsets from LSB of particular bit-fields in Execution Access registers
 */

/**
 * @def IX_NPEDL_OFFSET_ECS_REG_0_NEXTPC
 * @brief LSB-offset of NextPC field in Execution Access Register 0 of all ECS
 *        levels (except Debug ECS level)
 */
#define IX_NPEDL_OFFSET_ECS_REG_0_NEXTPC     16 

/**
 * @def IX_NPEDL_OFFSET_ECS_REG_0_LDUR
 * @brief LSB-offset of LDUR field in Execution Access Register 0 of all ECS
 *        levels
 */
#define IX_NPEDL_OFFSET_ECS_REG_0_LDUR        8

/**
 * @def IX_NPEDL_OFFSET_ECS_REG_1_CCTXT
 * @brief LSB-offset of CCTXT field in Execution Access Register 1 of all ECS
 *        levels
 */
#define IX_NPEDL_OFFSET_ECS_REG_1_CCTXT      16

/**
 * @def IX_NPEDL_OFFSET_ECS_REG_1_SELCTXT
 * @brief LSB-offset of SELCTXT field in Execution Access Register 1 of all ECS
 *        levels
 */
#define IX_NPEDL_OFFSET_ECS_REG_1_SELCTXT     0


/*
 * NPE core & co-processor instruction templates to load into NPE Instruction 
 * Register, for read/write of NPE register file registers
 */

/**
 * @def IX_NPEDL_INSTR_RD_REG_BYTE
 * @brief NPE Instruction, used to read an 8-bit NPE internal logical register
 *        and return the value in the EXDATA register (aligned to MSB).
 *        NPE Assembler instruction:  "mov8 d0, d0  &&& DBG_WrExec"
 */
#define IX_NPEDL_INSTR_RD_REG_BYTE    0x0FC00000

/**
 * @def IX_NPEDL_INSTR_RD_REG_SHORT
 * @brief NPE Instruction, used to read a 16-bit NPE internal logical register
 *        and return the value in the EXDATA register (aligned to MSB).
 *        NPE Assembler instruction:  "mov16 d0, d0  &&& DBG_WrExec"
 */
#define IX_NPEDL_INSTR_RD_REG_SHORT   0x0FC08010

/**
 * @def IX_NPEDL_INSTR_RD_REG_WORD
 * @brief NPE Instruction, used to read a 16-bit NPE internal logical register
 *        and return the value in the EXDATA register.
 *        NPE Assembler instruction:  "mov32 d0, d0  &&& DBG_WrExec"
 */
#define IX_NPEDL_INSTR_RD_REG_WORD    0x0FC08210

/**
 * @def IX_NPEDL_INSTR_WR_REG_BYTE
 * @brief NPE Immediate-Mode Instruction, used to write an 8-bit NPE internal
 *        logical register.
 *        NPE Assembler instruction:  "mov8 d0, #0"
 */
#define IX_NPEDL_INSTR_WR_REG_BYTE    0x00004000

/**
 * @def IX_NPEDL_INSTR_WR_REG_SHORT
 * @brief NPE Immediate-Mode Instruction, used to write a 16-bit NPE internal
 *        logical register.
 *        NPE Assembler instruction:  "mov16 d0, #0"
 */
#define IX_NPEDL_INSTR_WR_REG_SHORT   0x0000C000

/**
 * @def IX_NPEDL_INSTR_RD_FIFO
 * @brief NPE Immediate-Mode Instruction, used to write a 16-bit NPE internal
 *        logical register.
 *        NPE Assembler instruction:  "cprd32 d0    &&& DBG_RdInFIFO"
 */
#define IX_NPEDL_INSTR_RD_FIFO        0x0F888220    

/**
 * @def IX_NPEDL_INSTR_RESET_MBOX
 * @brief NPE Instruction, used to reset Mailbox (MBST) register
 *        NPE Assembler instruction:  "mov32 d0, d0  &&& DBG_ClearM"
 */
#define IX_NPEDL_INSTR_RESET_MBOX     0x0FAC8210


/*
 * Bit-offsets from LSB, of particular bit-fields in an NPE instruction
 */

/**
 * @def IX_NPEDL_OFFSET_INSTR_SRC
 * @brief LSB-offset to SRC (source operand) field of an NPE Instruction
 */
#define IX_NPEDL_OFFSET_INSTR_SRC              4

/**
 * @def IX_NPEDL_OFFSET_INSTR_DEST
 * @brief LSB-offset to DEST (destination operand) field of an NPE Instruction
 */
#define IX_NPEDL_OFFSET_INSTR_DEST             9

/**
 * @def IX_NPEDL_OFFSET_INSTR_COPROC
 * @brief LSB-offset to COPROC (coprocessor instruction) field of an NPE
 *        Instruction
 */
#define IX_NPEDL_OFFSET_INSTR_COPROC          18


/*
 * masks used to read/write particular bits of an NPE Instruction
 */

/**
 * @def IX_NPEDL_MASK_IMMED_INSTR_SRC_DATA
 * @brief Mask the bits of 16-bit data value (least-sig 5 bits) to be used in
 *        SRC field of immediate-mode NPE instruction
 */
#define IX_NPEDL_MASK_IMMED_INSTR_SRC_DATA         0x1F 

/**
 * @def IX_NPEDL_MASK_IMMED_INSTR_COPROC_DATA
 * @brief Mask the bits of 16-bit data value (most-sig 11 bits) to be used in
 *        COPROC field of immediate-mode NPE instruction
 */
#define IX_NPEDL_MASK_IMMED_INSTR_COPROC_DATA      0xFFE0

/**
 * @def IX_NPEDL_OFFSET_IMMED_INSTR_COPROC_DATA
 * @brief LSB offset of the bit-field of 16-bit data value (most-sig 11 bits)
 *        to be used in COPROC field of immediate-mode NPE instruction
 */
#define IX_NPEDL_OFFSET_IMMED_INSTR_COPROC_DATA    5

/**
 * @def IX_NPEDL_DISPLACE_IMMED_INSTR_COPROC_DATA
 * @brief Number of left-shifts required to align most-sig 11 bits of 16-bit
 *        data value into COPROC field of immediate-mode NPE instruction
 */
#define IX_NPEDL_DISPLACE_IMMED_INSTR_COPROC_DATA \
     (IX_NPEDL_OFFSET_INSTR_COPROC - IX_NPEDL_OFFSET_IMMED_INSTR_COPROC_DATA)

/**
 * @def IX_NPEDL_WR_INSTR_LDUR
 * @brief LDUR value used with immediate-mode NPE Instructions by the NpeDl
 *        for writing to NPE internal logical registers
 */
#define IX_NPEDL_WR_INSTR_LDUR                     1

/**
 * @def IX_NPEDL_RD_INSTR_LDUR
 * @brief LDUR value used with NON-immediate-mode NPE Instructions by the NpeDl
 *        for reading from NPE internal logical registers
 */
#define IX_NPEDL_RD_INSTR_LDUR                     0


/**
 * @enum IxNpeDlCtxtRegNum
 * @brief Numeric values to identify the NPE internal Context Store registers
 */
typedef enum
{
    IX_NPEDL_CTXT_REG_STEVT = 0,  /**< identifies STEVT   */
    IX_NPEDL_CTXT_REG_STARTPC,    /**< identifies STARTPC */
    IX_NPEDL_CTXT_REG_REGMAP,     /**< identifies REGMAP  */
    IX_NPEDL_CTXT_REG_CINDEX,     /**< identifies CINDEX  */
    IX_NPEDL_CTXT_REG_MAX         /**< Total number of Context Store registers */
} IxNpeDlCtxtRegNum;


/*
 * NPE Context Store register logical addresses
 */

/**
 * @def IX_NPEDL_CTXT_REG_ADDR_STEVT
 * @brief Logical address of STEVT NPE internal Context Store register
 */
#define IX_NPEDL_CTXT_REG_ADDR_STEVT      0x0000001B

/**
 * @def IX_NPEDL_CTXT_REG_ADDR_STARTPC
 * @brief Logical address of STARTPC NPE internal Context Store register
 */
#define IX_NPEDL_CTXT_REG_ADDR_STARTPC    0x0000001C

/**
 * @def IX_NPEDL_CTXT_REG_ADDR_REGMAP
 * @brief Logical address of REGMAP NPE internal Context Store register
 */
#define IX_NPEDL_CTXT_REG_ADDR_REGMAP     0x0000001E

/**
 * @def IX_NPEDL_CTXT_REG_ADDR_CINDEX
 * @brief Logical address of CINDEX NPE internal Context Store register
 */
#define IX_NPEDL_CTXT_REG_ADDR_CINDEX     0x0000001F


/*
 * NPE Context Store register reset values
 */

/**
 * @def IX_NPEDL_CTXT_REG_RESET_STEVT
 * @brief Reset value of STEVT NPE internal Context Store register
 *        (STEVT = off, 0x80)
 */
#define IX_NPEDL_CTXT_REG_RESET_STEVT     0x80

/**
 * @def IX_NPEDL_CTXT_REG_RESET_STARTPC
 * @brief Reset value of STARTPC NPE internal Context Store register
 *        (STARTPC = 0x0000)
 */
#define IX_NPEDL_CTXT_REG_RESET_STARTPC   0x0000

/**
 * @def IX_NPEDL_CTXT_REG_RESET_REGMAP
 * @brief Reset value of REGMAP NPE internal Context Store register
 *        (REGMAP = d0->p0, d8->p2, d16->p4)
 */
#define IX_NPEDL_CTXT_REG_RESET_REGMAP    0x0820

/**
 * @def IX_NPEDL_CTXT_REG_RESET_CINDEX
 * @brief Reset value of CINDEX NPE internal Context Store register
 *        (CINDEX = 0)
 */
#define IX_NPEDL_CTXT_REG_RESET_CINDEX    0x00


/*
 * numeric range of context levels available on an NPE
 */

/**
 * @def IX_NPEDL_CTXT_NUM_MIN
 * @brief Lowest NPE Context number in range
 */
#define IX_NPEDL_CTXT_NUM_MIN             0

/**
 * @def IX_NPEDL_CTXT_NUM_MAX
 * @brief Highest NPE Context number in range
 */
#define IX_NPEDL_CTXT_NUM_MAX             15


/*
 * Physical NPE internal registers
 */

/**
 * @def IX_NPEDL_TOTAL_NUM_PHYS_REG
 * @brief Number of Physical registers currently supported
 *        Initial NPE implementations will have a 32-word register file.
 *        Later implementations may have a 64-word register file.
 */
#define IX_NPEDL_TOTAL_NUM_PHYS_REG               32

/**
 * @def IX_NPEDL_OFFSET_PHYS_REG_ADDR_REGMAP
 * @brief LSB-offset of Regmap number in Physical NPE register address, used
 *        for Physical To Logical register address mapping in the NPE
 */
#define IX_NPEDL_OFFSET_PHYS_REG_ADDR_REGMAP      1

/**
 * @def IX_NPEDL_MASK_PHYS_REG_ADDR_LOGICAL_ADDR
 * @brief Mask to extract a logical NPE register address from a physical
 *        register address, used for Physical To Logical address mapping
 */
#define IX_NPEDL_MASK_PHYS_REG_ADDR_LOGICAL_ADDR   0x1

#endif /* IXNPEDLNPEMGRECREGISTERS_P_H */
