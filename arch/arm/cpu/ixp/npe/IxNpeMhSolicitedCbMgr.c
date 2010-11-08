/**
 * @file IxNpeMhSolicitedCbMgr.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the
 * Solicited Callback Manager module.
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
#ifndef IXNPEMHCONFIG_P_H
#	define IXNPEMHSOLICITEDCBMGR_C
#else
#	error "Error, IxNpeMhConfig_p.h should not be included before this definition."
#endif

/*
 * Put the system defined include files required.
 */


/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"

#include "IxNpeMhMacros_p.h"
#include "IxNpeMhSolicitedCbMgr_p.h"
#include "IxNpeMhConfig_p.h"
/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @struct IxNpeMhSolicitedCallbackListEntry
 *
 * @brief This structure is used to store the information associated with
 * an entry in the callback list.  This consists of the ID of the send
 * message (which indicates the ID of the corresponding response message)
 * and the callback function pointer itself.
 *
 */

typedef struct IxNpeMhSolicitedCallbackListEntry
{
    /** message ID */
    IxNpeMhMessageId messageId;

    /** callback function pointer */
    IxNpeMhCallback callback;

    /** pointer to next entry in the list */
    struct IxNpeMhSolicitedCallbackListEntry *next;
} IxNpeMhSolicitedCallbackListEntry;

/**
 * @struct IxNpeMhSolicitedCallbackList
 *
 * @brief This structure is used to maintain the list of response
 * callbacks.  The number of entries in this list will be variable, and
 * they will be stored in a linked list fashion for ease of addition and
 * removal.  The entries themselves are statically allocated, and are
 * organised into a "free" list and a "callback" list.  Adding an entry
 * means taking an entry from the "free" list and adding it to the
 * "callback" list.  Removing an entry means removing it from the
 * "callback" list and returning it to the "free" list.
 */

typedef struct
{
    /** pointer to the head of the free list */
    IxNpeMhSolicitedCallbackListEntry *freeHead;

    /** pointer to the head of the callback list */
    IxNpeMhSolicitedCallbackListEntry *callbackHead;

    /** pointer to the tail of the callback list */
    IxNpeMhSolicitedCallbackListEntry *callbackTail;

    /** array of entries - the first entry is used as a dummy entry to */
    /* avoid the scenario of having an empty list, hence '+ 1' */
    IxNpeMhSolicitedCallbackListEntry entries[IX_NPEMH_MAX_CALLBACKS + 1];
} IxNpeMhSolicitedCallbackList;

/**
 * @struct IxNpeMhSolicitedCbMgrStats
 *
 * @brief This structure is used to maintain statistics for the Solicited
 * Callback Manager module.
 */

typedef struct
{
    UINT32 saves;     /**< callback list saves */
    UINT32 retrieves; /**< callback list retrieves */
} IxNpeMhSolicitedCbMgrStats;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

PRIVATE IxNpeMhSolicitedCallbackList
ixNpeMhSolicitedCbMgrCallbackLists[IX_NPEMH_NUM_NPES];

PRIVATE IxNpeMhSolicitedCbMgrStats
ixNpeMhSolicitedCbMgrStats[IX_NPEMH_NUM_NPES];

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */

/*
 * Function definition: ixNpeMhSolicitedCbMgrInitialize
 */

void ixNpeMhSolicitedCbMgrInitialize (void)
{
    IxNpeMhNpeId npeId;
    UINT32 localIndex;
    IxNpeMhSolicitedCallbackList *list = NULL;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhSolicitedCbMgrInitialize\n");

    /* for each NPE ... */
    for (npeId = 0; npeId < IX_NPEMH_NUM_NPES; npeId++)
    {
        /* initialise a pointer to the list for convenience */
        list = &ixNpeMhSolicitedCbMgrCallbackLists[npeId];

        /* for each entry in the list, after the dummy entry ... */
        for (localIndex = 1; localIndex <= IX_NPEMH_MAX_CALLBACKS; localIndex++)
        {
            /* initialise the entry */
            list->entries[localIndex].messageId = 0x00;
            list->entries[localIndex].callback = NULL;

            /* if this entry is before the last entry */
            if (localIndex < IX_NPEMH_MAX_CALLBACKS)
            {
                /* chain this entry to the following entry */
                list->entries[localIndex].next = &(list->entries[localIndex + 1]);
            }
            else /* this entry is the last entry */
            {
                /* the last entry isn't chained to anything */
                list->entries[localIndex].next = NULL;
            }
        }

        /* set the free list pointer to point to the first real entry */
        /* (all real entries begin chained together on the free list) */
        list->freeHead = &(list->entries[1]);

        /* set the callback list pointers to point to the dummy entry */
        /* (the callback list is initially empty) */
        list->callbackHead = &(list->entries[0]);
        list->callbackTail = &(list->entries[0]);
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhSolicitedCbMgrInitialize\n");
}

/*
 * Function definition: ixNpeMhSolicitedCbMgrCallbackSave
 */

IX_STATUS ixNpeMhSolicitedCbMgrCallbackSave (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback solicitedCallback)
{
    IxNpeMhSolicitedCallbackList *list = NULL;
    IxNpeMhSolicitedCallbackListEntry *callbackEntry = NULL;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhSolicitedCbMgrCallbackSave\n");

    /* initialise a pointer to the list for convenience */
    list = &ixNpeMhSolicitedCbMgrCallbackLists[npeId];

    /* check to see if there are any entries in the free list */
    if (list->freeHead == NULL)
    {
        IX_NPEMH_ERROR_REPORT ("Solicited callback list is full\n");
        return IX_FAIL;
    }

    /* there is an entry in the free list we can use */

    /* update statistical info */
    ixNpeMhSolicitedCbMgrStats[npeId].saves++;

    /* remove a callback entry from the start of the free list */
    callbackEntry = list->freeHead;
    list->freeHead = callbackEntry->next;

    /* fill in the callback entry with the new data */
    callbackEntry->messageId = solicitedMessageId;
    callbackEntry->callback = solicitedCallback;

    /* the new callback entry will be added to the tail of the callback */
    /* list, so it isn't chained to anything */
    callbackEntry->next = NULL;

    /* chain new callback entry to the last entry of the callback list */
    list->callbackTail->next = callbackEntry;
    list->callbackTail = callbackEntry;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhSolicitedCbMgrCallbackSave\n");

    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhSolicitedCbMgrCallbackRetrieve
 */

void ixNpeMhSolicitedCbMgrCallbackRetrieve (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback *solicitedCallback)
{
    IxNpeMhSolicitedCallbackList *list = NULL;
    IxNpeMhSolicitedCallbackListEntry *callbackEntry = NULL;
    IxNpeMhSolicitedCallbackListEntry *previousEntry = NULL;

    /* initialise a pointer to the list for convenience */
    list = &ixNpeMhSolicitedCbMgrCallbackLists[npeId];

    /* initialise the callback entry to the first entry of the callback */
    /* list - we must skip over the dummy entry, which is the previous */
    callbackEntry = list->callbackHead->next;
    previousEntry = list->callbackHead;

    /* traverse the callback list looking for an entry with a matching */
    /* message ID.  note we also save the previous entry's pointer to */
    /* allow us to unchain the matching entry from the callback list */
    while ((callbackEntry != NULL) &&
           (callbackEntry->messageId != solicitedMessageId))
    {
        previousEntry = callbackEntry;
        callbackEntry = callbackEntry->next;
    }

    /* if we didn't find a matching callback entry */
    if (callbackEntry == NULL)
    {
        /* return a NULL callback in the outgoing parameter */
        *solicitedCallback = NULL;
    }
    else /* we found a matching callback entry */
    {
        /* update statistical info */
        ixNpeMhSolicitedCbMgrStats[npeId].retrieves++;

        /* return the callback in the outgoing parameter */
        *solicitedCallback = callbackEntry->callback;

        /* unchain callback entry by chaining previous entry to next */
        previousEntry->next = callbackEntry->next;

        /* if the callback entry is at the tail of the list */
        if (list->callbackTail == callbackEntry)
        {
            /* update the tail of the callback list */
            list->callbackTail = previousEntry;
        }

        /* re-initialise the callback entry */
        callbackEntry->messageId = 0x00;
        callbackEntry->callback = NULL;

        /* add the callback entry to the start of the free list */
        callbackEntry->next = list->freeHead;
        list->freeHead = callbackEntry;
    }
}

/*
 * Function definition: ixNpeMhSolicitedCbMgrShow
 */

void ixNpeMhSolicitedCbMgrShow (
    IxNpeMhNpeId npeId)
{
    /* show the solicited callback list save counter */
    IX_NPEMH_SHOW ("Solicited callback list saves",
                   ixNpeMhSolicitedCbMgrStats[npeId].saves);

    /* show the solicited callback list retrieve counter */
    IX_NPEMH_SHOW ("Solicited callback list retrieves",
                   ixNpeMhSolicitedCbMgrStats[npeId].retrieves);
}

/*
 * Function definition: ixNpeMhSolicitedCbMgrShowReset
 */

void ixNpeMhSolicitedCbMgrShowReset (
    IxNpeMhNpeId npeId)
{
    /* reset the solicited callback list save counter */
    ixNpeMhSolicitedCbMgrStats[npeId].saves = 0;

    /* reset the solicited callback list retrieve counter */
    ixNpeMhSolicitedCbMgrStats[npeId].retrieves = 0;
}
