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
 * SPDX-License-Identifier:	BSD-3-Clause
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
