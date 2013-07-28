/**
 * @file IxNpeDlNpeMgr_p.h
 *
 * @author Intel Corporation
 * @date 14 December 2001
 * @brief This file contains the private API for the NpeMgr module.
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


/**
 * @defgroup IxNpeDlNpeMgr_p IxNpeDlNpeMgr_p
 *
 * @brief The private API for the IxNpeDl NpeMgr module
 * 
 * @{
 */

#ifndef IXNPEDLNPEMGR_P_H
#define IXNPEDLNPEMGR_P_H


/*
 * Put the user defined include files required.
 */
#include "IxNpeDl.h"
#include "IxOsalTypes.h"


/*
 * Function Prototypes
 */


/**
 * @fn void ixNpeDlNpeMgrInit (void)
 * 
 * @brief Initialises the NpeMgr module
 *
 * @param none
 * 
 * This function initialises the NpeMgr module.
 * It should be called before any other function in this module is called.
 * It only needs to be called once, but can be called multiple times safely.
 * The code will ASSERT on failure.
 * 
 * @pre
 *     - It must be called before any other function in this module
 *
 * @post
 *     - NPE Configuration Register memory space will be mapped using
 *       IxOsal. This memory will not be unmapped by this module.
 *
 * @return none
 */ 
void
ixNpeDlNpeMgrInit (void);


/**
 * @fn IX_STATUS ixNpeMhNpeMgrUninit (void)
 *
 * @brief This function will uninitialise the IxNpeDlNpeMgr sub-component.
 *
 * This function will uninitialise the IxNpeDlNpeMgr sub-component.
 * It should only be called once, and only if the IxNpeDlNpeMgr sub-component
 * has already been initialised by calling @ref ixNpeDlNpeMgrInit().
 * No other IxNpeDlNpeMgr sub-component API functions should be called
 * until @ref ixNpeDlNpeMgrInit() is called again.
 * If possible, this function should be called before a soft reboot or unloading
 * a kernel module to perform any clean up operations required for IxNpeMh.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */

IX_STATUS ixNpeDlNpeMgrUninit (void);


/**
 * @fn IX_STATUS ixNpeDlNpeMgrImageLoad (IxNpeDlNpeId npeId,
                                           UINT32 *imageCodePtr,
                                           BOOL verify)
 * 
 * @brief Loads a image of microcode onto an NPE
 *
 * @param IxNpeDlNpeId [in] npeId     - Id of target NPE
 * @param UINT32* [in] imageCodePtr - pointer to image code in image to be
 *                                      downloaded
 * @param BOOL [in] verify            - if true, verify each word written to
 *                                      NPE memory.
 * 
 * This function loads a image containing blocks of microcode onto a
 * particular NPE. If the <i>verify</i> option is ON, NpeDl will read back each
 * word written and verify that it was written successfully
 * 
 * @pre
 *     - The NPE should be stopped beforehand
 *
 * @post
 *     - The NPE Instruction Pipeline may be flushed clean
 *
 * @return
 *     - IX_SUCCESS if the download was successful
 *     - IX_FAIL if the download failed
 *     - IX_NPEDL_CRITICAL_NPE_ERR if the download failed due to timeout error 
 *       where NPE is not responding
 */ 
IX_STATUS
ixNpeDlNpeMgrImageLoad (IxNpeDlNpeId npeId, UINT32 *imageCodePtr,
			  BOOL verify);


/**
 * @fn IX_STATUS ixNpeDlNpeMgrNpeReset (IxNpeDlNpeId npeId)
 * 
 * @brief sets a NPE to RESET state
 *
 * @param IxNpeDlNpeId [in] npeId - id of target NPE
 * 
 * This function performs a soft NPE reset by writing reset values to the
 * Configuration Bus Execution Control registers, the Execution Context Stack
 * registers, the Physical Register file, and the Context Store registers for 
 * each context number. It also clears inFIFO, outFIFO and Watchpoint FIFO.
 * It does not reset NPE Co-processors.
 * 
 * @pre
 *     - The NPE should be stopped beforehand
 *
 * @post
 *     - NPE NextProgram Counter (NextPC) will be set to a fixed initial value,
 *       such as 0.  This should be explicitly set by downloading State
 *       Information before starting NPE Execution.
 *     - The NPE Instruction Pipeline will be in a clean state.
 *
 * @return
 *     - IX_SUCCESS if the operation was successful
 *     - IX_FAIL if the operation failed
 *     - IX_NPEDL_CRITICAL_NPE_ERR if the operation failed due to NPE hang
 */ 
IX_STATUS
ixNpeDlNpeMgrNpeReset (IxNpeDlNpeId npeId);


/**
 * @fn IX_STATUS ixNpeDlNpeMgrNpeStart (IxNpeDlNpeId npeId)
 * 
 * @brief Starts NPE Execution
 *
 * @param IxNpeDlNpeId [in] npeId - Id of target NPE
 * 
 * Ensures only background Execution Stack Level is Active, clears instruction
 * pipeline, and starts Execution on a NPE by sending a Start NPE command to
 * the NPE. Checks the execution status of the NPE to verify that it is
 * running.
 * 
 * @pre
 *     - The NPE should be stopped beforehand.
 *     - Note that this function does not set the NPE Next Program Counter 
 *       (NextPC), so it should be set beforehand if required by downloading 
 *       appropriate State Information.
 *
 * @post
 *
 * @return
 *     - IX_SUCCESS if the operation was successful
 *     - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlNpeMgrNpeStart (IxNpeDlNpeId npeId);


/**
 * @fn IX_STATUS ixNpeDlNpeMgrNpeStop (IxNpeDlNpeId npeId)
 * 
 * @brief Halts NPE Execution
 *
 * @param IxNpeDlNpeId [in] npeId - id of target NPE
 * 
 * Stops execution on an NPE by sending a Stop NPE command to the NPE.
 * Checks the execution status of the NPE to verify that it has stopped.
 *
 * @pre
 *
 * @post
 *
 * @return 
 *     - IX_SUCCESS if the operation was successful
 *     - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlNpeMgrNpeStop (IxNpeDlNpeId npeId);


/**
 * @fn void ixNpeDlNpeMgrStatsShow (void)
 *
 * @brief This function will display statistics of the IxNpeDl NpeMgr module
 *
 * @return none
 */
void
ixNpeDlNpeMgrStatsShow (void);


/**
 * @fn void ixNpeDlNpeMgrStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl NpeMgr module
 *
 * @return none
 */
void
ixNpeDlNpeMgrStatsReset (void);


#endif /* IXNPEDLIMAGEMGR_P_H */

/**
 * @} defgroup IxNpeDlNpeMgr_p
 */
