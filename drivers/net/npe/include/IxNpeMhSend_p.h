/**
 * @file IxNpeMhSend_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Send module.
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
 * @defgroup IxNpeMhSend_p IxNpeMhSend_p
 *
 * @brief The private API for the Send module.
 * 
 * @{
 */

#ifndef IXNPEMHSEND_P_H
#define IXNPEMHSEND_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn IX_STATUS ixNpeMhSendMessageSend (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage message,
           UINT32 maxSendRetries)
 *
 * @brief This function writes a message to the specified NPE's inFIFO,
 * and must be used when the message being sent does not solicit a response
 * from the NPE. This function will return TIMEOUT status if NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to send the message
 * to.
 * @param IxNpeMhMessage message (in) - The message to send.
 * @param UINT32 maxSendRetries (in) - Max num. of retries to perform
 * if the NPE's inFIFO is full.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhSendMessageSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    UINT32 maxSendRetries);

/**
 * @fn IX_STATUS ixNpeMhSendMessageWithResponseSend (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage message,
           IxNpeMhMessageId solicitedMessageId,
           IxNpeMhCallback solicitedCallback,
           UINT32 maxSendRetries)
 *
 * @brief This function writes a message to the specified NPE's inFIFO,
 * and must be used when the message being sent solicits a response from
 * the NPE.  The ID of the solicited response must be specified so that it
 * can be recognised, and a callback provided to pass the response back to
 * the client. This function will return TIMEOUT status if NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to send the message
 * to.
 * @param IxNpeMhMessage message (in) - The message to send.
 * @param IxNpeMhMessageId solicitedMessageId (in) - The ID of the
 * solicited response.
 * @param IxNpeMhCallback solicitedCallback (in) - The callback to pass the
 * solicited response back to the client.
 * @param UINT32 maxSendRetries (in) - Max num. of retries to perform
 * if the NPE's inFIFO is full.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhSendMessageWithResponseSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback solicitedCallback,
    UINT32 maxSendRetries);

/**
 * @fn void ixNpeMhSendShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Send module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhSendShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhSendShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Send module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhSendShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHSEND_P_H */

/**
 * @} defgroup IxNpeMhSend_p
 */
