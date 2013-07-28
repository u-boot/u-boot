/**
 * @file IxNpeMh.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the public API for the
 * IXP425 NPE Message Handler component.
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
 * Put the system defined include files required.
 */

/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxNpeMhMacros_p.h"

#include "IxNpeMh.h"

#include "IxNpeMhConfig_p.h"
#include "IxNpeMhReceive_p.h"
#include "IxNpeMhSend_p.h"
#include "IxNpeMhSolicitedCbMgr_p.h"
#include "IxNpeMhUnsolicitedCbMgr_p.h"

/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

PRIVATE BOOL ixNpeMhInitialized = false;

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */

/*
 * Function definition: ixNpeMhInitialize
 */

PUBLIC IX_STATUS ixNpeMhInitialize (
    IxNpeMhNpeInterrupts npeInterrupts)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhInitialize\n");

    /* check the npeInterrupts parameter */
    if ((npeInterrupts != IX_NPEMH_NPEINTERRUPTS_NO) &&
        (npeInterrupts != IX_NPEMH_NPEINTERRUPTS_YES))
    {
        IX_NPEMH_ERROR_REPORT ("Illegal npeInterrupts parameter value\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* initialize the Receive module */
    ixNpeMhReceiveInitialize ();

    /* initialize the Solicited Callback Manager module */
    ixNpeMhSolicitedCbMgrInitialize ();

    /* initialize the Unsolicited Callback Manager module */
    ixNpeMhUnsolicitedCbMgrInitialize ();

    /* initialize the Configuration module
     *
     * NOTE: This module was originally configured before the
     * others, but the sequence was changed so that interrupts
     * would only be enabled after the handler functions were
     * set up.  The above modules need to be initialised to
     * handle the NPE interrupts.  See SCR #2231.
     */
    ixNpeMhConfigInitialize (npeInterrupts);

    ixNpeMhInitialized = true;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhInitialize\n");

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhUnload
 */

PUBLIC IX_STATUS ixNpeMhUnload (void)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhUnload\n");

    if (!ixNpeMhInitialized)
    {
	return IX_FAIL;
    }

    /* Uninitialize the Configuration module */
    ixNpeMhConfigUninit ();

    ixNpeMhInitialized = false;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhUnload\n");

    return IX_SUCCESS;
}


/*
 * Function definition: ixNpeMhUnsolicitedCallbackRegister
 */

PUBLIC IX_STATUS ixNpeMhUnsolicitedCallbackRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId messageId,
    IxNpeMhCallback unsolicitedCallback)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhUnsolicitedCallbackRegister\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* check the messageId parameter */
    if ((messageId < IX_NPEMH_MIN_MESSAGE_ID)
        || (messageId > IX_NPEMH_MAX_MESSAGE_ID))
    {
        IX_NPEMH_ERROR_REPORT ("Message ID is out of range\n");
        return IX_FAIL;
    }

    /* the unsolicitedCallback parameter is allowed to be NULL */

    /* parameters are ok ... */

    /* get the lock to prevent other clients from entering */
    ixNpeMhConfigLockGet (npeId);

    /* save the unsolicited callback for the message ID */
    ixNpeMhUnsolicitedCbMgrCallbackSave (
        npeId, messageId, unsolicitedCallback);

    /* release the lock to allow other clients back in */
    ixNpeMhConfigLockRelease (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhUnsolicitedCallbackRegister\n");

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhUnsolicitedCallbackForRangeRegister
 */

PUBLIC IX_STATUS ixNpeMhUnsolicitedCallbackForRangeRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId minMessageId,
    IxNpeMhMessageId maxMessageId,
    IxNpeMhCallback unsolicitedCallback)
{
    IxNpeMhMessageId messageId;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhUnsolicitedCallbackForRangeRegister\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* check the minMessageId parameter */
    if ((minMessageId < IX_NPEMH_MIN_MESSAGE_ID)
        || (minMessageId > IX_NPEMH_MAX_MESSAGE_ID))
    {
        IX_NPEMH_ERROR_REPORT ("Min message ID is out of range\n");
        return IX_FAIL;
    }

    /* check the maxMessageId parameter */
    if ((maxMessageId < IX_NPEMH_MIN_MESSAGE_ID)
        || (maxMessageId > IX_NPEMH_MAX_MESSAGE_ID))
    {
        IX_NPEMH_ERROR_REPORT ("Max message ID is out of range\n");
        return IX_FAIL;
    }

    /* check the semantics of the message range parameters */
    if (minMessageId > maxMessageId)
    {
        IX_NPEMH_ERROR_REPORT ("Min message ID greater than max message "
                               "ID\n");
        return IX_FAIL;
    }

    /* the unsolicitedCallback parameter is allowed to be NULL */

    /* parameters are ok ... */

    /* get the lock to prevent other clients from entering */
    ixNpeMhConfigLockGet (npeId);

    /* for each message ID in the range ... */
    for (messageId = minMessageId; messageId <= maxMessageId; messageId++)
    {
        /* save the unsolicited callback for the message ID */
        ixNpeMhUnsolicitedCbMgrCallbackSave (
            npeId, messageId, unsolicitedCallback);
    }

    /* release the lock to allow other clients back in */
    ixNpeMhConfigLockRelease (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhUnsolicitedCallbackForRangeRegister\n");

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhMessageSend
 */

PUBLIC IX_STATUS ixNpeMhMessageSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    UINT32 maxSendRetries)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhMessageSend\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* get the lock to prevent other clients from entering */
    ixNpeMhConfigLockGet (npeId);

    /* send the message */
    status = ixNpeMhSendMessageSend (npeId, message, maxSendRetries);
    if (status != IX_SUCCESS)
    {
        IX_NPEMH_ERROR_REPORT ("Failed to send message\n");
    }

    /* release the lock to allow other clients back in */
    ixNpeMhConfigLockRelease (npeId);

    IX_NPEMH_TRACE1 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhMessageSend"
                     " : status = %d\n", status);

    return status;
}

/*
 * Function definition: ixNpeMhMessageWithResponseSend
 */

PUBLIC IX_STATUS ixNpeMhMessageWithResponseSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback solicitedCallback,
    UINT32 maxSendRetries)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhCallback unsolicitedCallback = NULL;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhMessageWithResponseSend\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* the solicitecCallback parameter is allowed to be NULL.  this */
    /* signifies the client is not interested in the response message */

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* check the solicitedMessageId parameter */
    if ((solicitedMessageId < IX_NPEMH_MIN_MESSAGE_ID)
        || (solicitedMessageId > IX_NPEMH_MAX_MESSAGE_ID))
    {
        IX_NPEMH_ERROR_REPORT ("Solicited message ID is out of range\n");
        return IX_FAIL;
    }

    /* check the solicitedMessageId parameter.  if an unsolicited */
    /* callback has been registered for the specified message ID then */
    /* report an error and return failure */
    ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
        npeId, solicitedMessageId, &unsolicitedCallback);
    if (unsolicitedCallback != NULL)
    {
        IX_NPEMH_ERROR_REPORT ("Solicited message ID conflicts with "
                               "unsolicited message ID\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* get the lock to prevent other clients from entering */
    ixNpeMhConfigLockGet (npeId);

    /* send the message */
    status = ixNpeMhSendMessageWithResponseSend (
        npeId, message, solicitedMessageId, solicitedCallback,
        maxSendRetries);
    if (status != IX_SUCCESS)
    {
        IX_NPEMH_ERROR_REPORT ("Failed to send message\n");
    }

    /* release the lock to allow other clients back in */
    ixNpeMhConfigLockRelease (npeId);

    IX_NPEMH_TRACE1 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhMessageWithResponseSend"
                     " : status = %d\n", status);

    return status;
}

/*
 * Function definition: ixNpeMhMessagesReceive
 */

PUBLIC IX_STATUS ixNpeMhMessagesReceive (
    IxNpeMhNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhMessagesReceive\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* get the lock to prevent other clients from entering */
    ixNpeMhConfigLockGet (npeId);

    /* receive messages from the NPE */
    status = ixNpeMhReceiveMessagesReceive (npeId);

    if (status != IX_SUCCESS)
    {
        IX_NPEMH_ERROR_REPORT ("Failed to receive message\n");
    }

    /* release the lock to allow other clients back in */
    ixNpeMhConfigLockRelease (npeId);

    IX_NPEMH_TRACE1 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhMessagesReceive"
                     " : status = %d\n", status);

    return status;
}

/*
 * Function definition: ixNpeMhShow
 */

PUBLIC IX_STATUS ixNpeMhShow (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhShow\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* note we don't get the lock here as printing the statistics */
    /* to a console may take some time and we don't want to impact */
    /* system performance.  this means that the statistics displayed */
    /* may be in a state of flux and make not represent a consistent */
    /* snapshot. */

    /* display a header */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
	       "Current state of NPE ID %d:\n\n", npeId, 0, 0, 0, 0, 0);

    /* show the current state of each module */

    /* show the current state of the Configuration module */
    ixNpeMhConfigShow (npeId);

    /* show the current state of the Receive module */
    ixNpeMhReceiveShow (npeId);

    /* show the current state of the Send module */
    ixNpeMhSendShow (npeId);

    /* show the current state of the Solicited Callback Manager module */
    ixNpeMhSolicitedCbMgrShow (npeId);

    /* show the current state of the Unsolicited Callback Manager module */
    ixNpeMhUnsolicitedCbMgrShow (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhShow\n");

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhShowReset
 */

PUBLIC IX_STATUS ixNpeMhShowReset (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhShowReset\n");

    /* check that we are initialized */
    if (!ixNpeMhInitialized)
    {
        IX_NPEMH_ERROR_REPORT ("IxNpeMh component is not initialized\n");
        return IX_FAIL;
    }

    /* check the npeId parameter */
    if (!ixNpeMhConfigNpeIdIsValid (npeId))
    {
        IX_NPEMH_ERROR_REPORT ("NPE ID invalid\n");
        return IX_FAIL;
    }

    /* parameters are ok ... */

    /* note we don't get the lock here as resetting the statistics */
    /* shouldn't impact system performance. */

    /* reset the current state of each module */

    /* reset the current state of the Configuration module */
    ixNpeMhConfigShowReset (npeId);

    /* reset the current state of the Receive module */
    ixNpeMhReceiveShowReset (npeId);

    /* reset the current state of the Send module */
    ixNpeMhSendShowReset (npeId);

    /* reset the current state of the Solicited Callback Manager module */
    ixNpeMhSolicitedCbMgrShowReset (npeId);

    /* reset the current state of the Unsolicited Callback Manager module */
    ixNpeMhUnsolicitedCbMgrShowReset (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhShowReset\n");

    return IX_SUCCESS;
}
