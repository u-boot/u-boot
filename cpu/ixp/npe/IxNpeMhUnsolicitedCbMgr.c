/**
 * @file IxNpeMhUnsolicitedCbMgr.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the private API for
 * the Unsolicited Callback Manager module.
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


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"

#include "IxNpeMhMacros_p.h"

#include "IxNpeMhUnsolicitedCbMgr_p.h"


/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @struct IxNpeMhUnsolicitedCallbackTable
 *
 * @brief This structure is used to maintain the list of registered
 * callbacks.  One entry exists for each message ID, and a NULL entry will
 * signify that no callback has been registered for that ID.
 */

typedef struct
{
    /** array of entries */
    IxNpeMhCallback entries[IX_NPEMH_MAX_MESSAGE_ID + 1];
} IxNpeMhUnsolicitedCallbackTable;

/**
 * @struct IxNpeMhUnsolicitedCbMgrStats
 *
 * @brief This structure is used to maintain statistics for the Unsolicited
 * Callback Manager module.
 */

typedef struct
{
    UINT32 saves;      /**< callback table saves */
    UINT32 overwrites; /**< callback table overwrites */
} IxNpeMhUnsolicitedCbMgrStats;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

PRIVATE IxNpeMhUnsolicitedCallbackTable
ixNpeMhUnsolicitedCallbackTables[IX_NPEMH_NUM_NPES];

PRIVATE IxNpeMhUnsolicitedCbMgrStats
ixNpeMhUnsolicitedCbMgrStats[IX_NPEMH_NUM_NPES];

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */

/*
 * Function definition: ixNpeMhUnsolicitedCbMgrInitialize
 */

void ixNpeMhUnsolicitedCbMgrInitialize (void)
{
    IxNpeMhNpeId npeId = 0;
    IxNpeMhUnsolicitedCallbackTable *table = NULL;
    IxNpeMhMessageId messageId = 0;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhUnsolicitedCbMgrInitialize\n");

    /* for each NPE ... */
    for (npeId = 0; npeId < IX_NPEMH_NUM_NPES; npeId++)
    {
        /* initialise a pointer to the table for convenience */
        table = &ixNpeMhUnsolicitedCallbackTables[npeId];

        /* for each message ID ... */
        for (messageId = IX_NPEMH_MIN_MESSAGE_ID;
             messageId <= IX_NPEMH_MAX_MESSAGE_ID; messageId++)
        {
            /* initialise the callback for this message ID to NULL */
            table->entries[messageId] = NULL;
        }
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhUnsolicitedCbMgrInitialize\n");
}

/*
 * Function definition: ixNpeMhUnsolicitedCbMgrCallbackSave
 */

void ixNpeMhUnsolicitedCbMgrCallbackSave (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback unsolicitedCallback)
{
    IxNpeMhUnsolicitedCallbackTable *table = NULL;

    /* initialise a pointer to the table for convenience */
    table = &ixNpeMhUnsolicitedCallbackTables[npeId];

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhUnsolicitedCbMgrCallbackSave\n");

    /* update statistical info */
    ixNpeMhUnsolicitedCbMgrStats[npeId].saves++;

    /* check if there is a callback already registered for this NPE and */
    /* message ID */
    if (table->entries[unsolicitedMessageId] != NULL)
    {
	/* if we are overwriting an existing callback */
	if (unsolicitedCallback != NULL)
	{
	    IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "Unsolicited callback "
			     "overwriting existing callback for NPE ID %d "
			     "message ID 0x%02X\n", npeId, unsolicitedMessageId);
	}
	else /* if we are clearing an existing callback */
	{
	    IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NULL unsolicited callback "
			     "clearing existing callback for NPE ID %d "
			     "message ID 0x%02X\n", npeId, unsolicitedMessageId);
	}

        /* update statistical info */
        ixNpeMhUnsolicitedCbMgrStats[npeId].overwrites++;
    }

    /* save the callback into the table */
    table->entries[unsolicitedMessageId] = unsolicitedCallback;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhUnsolicitedCbMgrCallbackSave\n");
}

/*
 * Function definition: ixNpeMhUnsolicitedCbMgrCallbackRetrieve
 */

void ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback *unsolicitedCallback)
{
    IxNpeMhUnsolicitedCallbackTable *table = NULL;

    /* initialise a pointer to the table for convenience */
    table = &ixNpeMhUnsolicitedCallbackTables[npeId];

    /* retrieve the callback from the table */
    *unsolicitedCallback = table->entries[unsolicitedMessageId];
}

/*
 * Function definition: ixNpeMhUnsolicitedCbMgrShow
 */

void ixNpeMhUnsolicitedCbMgrShow (
    IxNpeMhNpeId npeId)
{
    /* show the unsolicited callback table save counter */
    IX_NPEMH_SHOW ("Unsolicited callback table saves",
                   ixNpeMhUnsolicitedCbMgrStats[npeId].saves);

    /* show the unsolicited callback table overwrite counter */
    IX_NPEMH_SHOW ("Unsolicited callback table overwrites",
                   ixNpeMhUnsolicitedCbMgrStats[npeId].overwrites);
}

/*
 * Function definition: ixNpeMhUnsolicitedCbMgrShowReset
 */

void ixNpeMhUnsolicitedCbMgrShowReset (
    IxNpeMhNpeId npeId)
{
    /* reset the unsolicited callback table save counter */
    ixNpeMhUnsolicitedCbMgrStats[npeId].saves = 0;

    /* reset the unsolicited callback table overwrite counter */
    ixNpeMhUnsolicitedCbMgrStats[npeId].overwrites = 0;
}
