/**
 * @file IxNpeMhReceive_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Receive module.
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
 * @defgroup IxNpeMhReceive_p IxNpeMhReceive_p
 *
 * @brief The private API for the Receive module.
 * 
 * @{
 */

#ifndef IXNPEMHRECEIVE_P_H
#define IXNPEMHRECEIVE_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhReceiveInitialize (void)
 *
 * @brief This function registers an internal ISR to handle the NPEs'
 * "outFIFO not empty" interrupts and receive messages from the NPEs when
 * they become available.
 *
 * @return No return value.
 */

void ixNpeMhReceiveInitialize (void);

/**
 * @fn IX_STATUS ixNpeMhReceiveMessagesReceive (
           IxNpeMhNpeId npeId)
 *
 * @brief This function reads messages from a particular NPE's outFIFO
 * until the outFIFO is empty, and for each message looks first for an
 * unsolicited callback, then a solicited callback, to pass the message
 * back to the client.  If no callback can be found the message is
 * discarded and an error reported. This function will return TIMEOUT 
 * status if NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to receive
 * messages from.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhReceiveMessagesReceive (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHRECEIVE_P_H */

/**
 * @} defgroup IxNpeMhReceive_p
 */
