/**
 * @file IxNpeMhUnsolicitedCbMgr_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Unsolicited Callback
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
 * @defgroup IxNpeMhUnsolicitedCbMgr_p IxNpeMhUnsolicitedCbMgr_p
 *
 * @brief The private API for the Unsolicited Callback Manager module.
 * 
 * @{
 */

#ifndef IXNPEMHUNSOLICITEDCBMGR_P_H
#define IXNPEMHUNSOLICITEDCBMGR_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhUnsolicitedCbMgrInitialize (void)
 *
 * @brief This function initializes the Unsolicited Callback Manager
 * module, setting up a callback data structure for each NPE.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrInitialize (void);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrCallbackSave (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId unsolicitedMessageId,
           IxNpeMhCallback unsolicitedCallback)
 *
 * @brief This function saves a callback in the specified NPE's callback
 * table.  If a callback already exists for the specified ID then it will
 * be overwritten.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE in whose callback
 * table the callback will be saved.
 * @param IxNpeMhMessageId unsolicitedMessageId (in) - The ID of the
 * messages that this callback is for.
 * @param IxNpeMhCallback unsolicitedCallback (in) - The callback function
 * pointer to save.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrCallbackSave (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback unsolicitedCallback);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId unsolicitedMessageId,
           IxNpeMhCallback *unsolicitedCallback)
 *
 * @brief This function retrieves the callback for the specified ID from
 * the specified NPE's callback table.  If no callback is registered for
 * the specified ID and NPE then a callback value of NULL will be returned.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE from whose callback
 * table the callback will be retrieved.
 * @param IxNpeMhMessageId unsolicitedMessageId (in) - The ID of the
 * messages that the callback is for.
 * @param IxNpeMhCallback unsolicitedCallback (out) - The callback function
 * pointer retrieved.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback *unsolicitedCallback);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Unsolicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Unsolicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHUNSOLICITEDCBMGR_P_H */

/**
 * @} defgroup IxNpeMhUnsolicitedCbMgr_p
 */
