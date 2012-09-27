/**
 * @file IxNpeMhSolicitedCbMgr_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Solicited Callback
 * Manager module.
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
 * @defgroup IxNpeMhSolicitedCbMgr_p IxNpeMhSolicitedCbMgr_p
 *
 * @brief The private API for the Solicited Callback Manager module.
 * 
 * @{
 */

#ifndef IXNPEMHSOLICITEDCBMGR_P_H
#define IXNPEMHSOLICITEDCBMGR_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/** Maximum number of solicited callbacks that can be stored in the list */
#define IX_NPEMH_MAX_CALLBACKS (16)

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhSolicitedCbMgrInitialize (void)
 *
 * @brief This function initializes the Solicited Callback Manager module,
 * setting up a callback data structure for each NPE.
 *
 * @return No return value.
 */

void ixNpeMhSolicitedCbMgrInitialize (void);

/**
 * @fn IX_STATUS ixNpeMhSolicitedCbMgrCallbackSave (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId solicitedMessageId,
           IxNpeMhCallback solicitedCallback)
 *
 * @brief This function saves a callback in the specified NPE's callback
 * list.  If the callback list is full the function will fail.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE in whose callback
 * list the callback will be saved.
 * @param IxNpeMhMessageId solicitedMessageId (in) - The ID of the message
 * that this callback is for.
 * @param IxNpeMhCallback solicitedCallback (in) - The callback function
 * pointer to save.
 *
 * @return The function returns a status indicating success or failure.
 */

IX_STATUS ixNpeMhSolicitedCbMgrCallbackSave (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback solicitedCallback);

/**
 * @fn void ixNpeMhSolicitedCbMgrCallbackRetrieve (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId solicitedMessageId,
           IxNpeMhCallback *solicitedCallback)
 *
 * @brief This function retrieves the first ID-matching callback from the
 * specified NPE's callback list.  If no matching callback can be found the
 * function will fail.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE from whose callback
 * list the callback will be retrieved.
 * @param IxNpeMhMessageId solicitedMessageId (in) - The ID of the message
 * that the callback is for.
 * @param IxNpeMhCallback solicitedCallback (out) - The callback function
 * pointer retrieved.
 *
 * @return No return value.
 */

void ixNpeMhSolicitedCbMgrCallbackRetrieve (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback *solicitedCallback);

/**
 * @fn void ixNpeMhSolicitedCbMgrShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Solicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhSolicitedCbMgrShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhSolicitedCbMgrShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Solicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhSolicitedCbMgrShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHSOLICITEDCBMGR_P_H */

/**
 * @} defgroup IxNpeMhSolicitedCbMgr_p
 */
