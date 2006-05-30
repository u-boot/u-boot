/**
 * @file IxEthDBDBCore.c
 *
 * @brief Database support functions
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

#include "IxEthDB_p.h"

/* list of database hashtables */
IX_ETH_DB_PUBLIC HashTable dbHashtable;
IX_ETH_DB_PUBLIC MatchFunction matchFunctions[IX_ETH_DB_MAX_KEY_INDEX + 1];
IX_ETH_DB_PUBLIC BOOL ixEthDBPortUpdateRequired[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1];
IX_ETH_DB_PUBLIC UINT32 ixEthDBKeyType[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1];

/* private initialization flag */
IX_ETH_DB_PRIVATE BOOL ethDBInitializationComplete = FALSE;

/**
 * @brief initializes EthDB
 *
 * This function must be called to initialize the component.
 *
 * It does the following things:
 * - checks the port definition structure
 * - scans the capabilities of the NPE images and sets the
 *   capabilities of the ports accordingly
 * - initializes the memory pools internally used in EthDB
 *   for storing database records and handling data
 * - registers automatic update handlers for add and remove
 *   operations
 * - registers hashing match functions, depending on key sets
 * - initializes the main database hashtable
 * - allocates contiguous memory zones to be used for NPE
 *   updates
 * - registers the serialize methods used to convert data
 *   into NPE-readable format
 * - starts the event processor
 *
 * Note that this function is documented in the public
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS or an appropriate error if the
 * component failed to initialize correctly
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBInit(void)
{
    IxEthDBStatus result;

    if (ethDBInitializationComplete)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

    /* trap an invalid port definition structure */
    IX_ETH_DB_PORTS_ASSERTION;

    /* memory management */
    ixEthDBInitMemoryPools();

    /* register hashing search methods */
    ixEthDBMatchMethodsRegister(matchFunctions);

    /* register type-based automatic port updates */
    ixEthDBUpdateTypeRegister(ixEthDBPortUpdateRequired);

    /* register record to key type mappings */
    ixEthDBKeyTypeRegister(ixEthDBKeyType);

    /* hash table */
    ixEthDBInitHash(&dbHashtable, NUM_BUCKETS, ixEthDBEntryXORHash, matchFunctions, (FreeFunction) ixEthDBFreeMacDescriptor);

    /* NPE update zones */
    ixEthDBNPEUpdateAreasInit();

    /* register record serialization methods */
    ixEthDBRecordSerializeMethodsRegister();

    /* start the event processor */
    result = ixEthDBEventProcessorInit();

    /* scan NPE features */
    if (result == IX_ETH_DB_SUCCESS)
    {
        ixEthDBFeatureCapabilityScan();
    }

    ethDBInitializationComplete = TRUE;

    return result;
}

/**
 * @brief prepares EthDB for unloading
 *
 * This function must be called before removing the
 * EthDB component from memory (e.g. doing rmmod in
 * Linux) if the component is to be re-initialized again
 * without rebooting the platform.
 *
 * All the EthDB ports must be disabled before this
 * function is to be called. Failure to disable all
 * the ports will return the IX_ETH_DB_BUSY error.
 *
 * This function will destroy mutexes, deallocate
 * memory and stop the event processor.
 *
 * Note that this function is fully documented in the
 * main component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if de-initialization
 * completed successfully or an appropriate error
 * message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBUnload(void)
{
    IxEthDBPortId portIndex;

    if (!ethDBInitializationComplete)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

    /* check if any ports are enabled */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (ixEthDBPortInfo[portIndex].enabled)
        {
            return IX_ETH_DB_BUSY;
        }
    }

    /* free port resources */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (ixEthDBPortDefinitions[portIndex].type == IX_ETH_NPE)
        {
            ixOsalMutexDestroy(&ixEthDBPortInfo[portIndex].npeAckLock);
        }

        ixEthDBPortInfo[portIndex].initialized = FALSE;
    }

    /* shutdown event processor */
    ixEthDBStopLearningFunction();

    /* deallocate NPE update zones */
    ixEthDBNPEUpdateAreasUnload();

    ethDBInitializationComplete = FALSE;

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief adds a new entry to the Ethernet database
 *
 * @param newRecordTemplate address of the record template to use
 * @param updateTrigger port map containing the update triggers
 * resulting from this update operation
 *
 * Creates a new database entry, populates it with the data
 * copied from the given template and adds the record to the
 * database hash table.
 * It also checks whether the new record type is registered to trigger
 * automatic updates; if it is, the update trigger will contain the
 * port on which the record insertion was performed, as well as the
 * old port in case the addition was a record migration (from one port
 * to the other). The caller can use the updateTrigger to trigger
 * automatic updates on the ports changed as a result of this addition.
 *
 * @retval IX_ETH_DB_SUCCESS addition successful
 * @retval IX_ETH_DB_NOMEM insertion failed, no memory left in the mac descriptor memory pool
 * @retval IX_ETH_DB_BUSY database busy, cannot insert due to locking
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBAdd(MacDescriptor *newRecordTemplate, IxEthDBPortMap updateTrigger)
{
    IxEthDBStatus result;
    MacDescriptor *newDescriptor;
    IxEthDBPortId originalPortID;
    HashNode *node = NULL;

    BUSY_RETRY(ixEthDBSearchHashEntry(&dbHashtable, ixEthDBKeyType[newRecordTemplate->type], newRecordTemplate, &node));

    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    if (node == NULL)
    {
        /* not found, create a new one */
        newDescriptor = ixEthDBAllocMacDescriptor();

        if (newDescriptor == NULL)
        {
            return IX_ETH_DB_NOMEM; /* no memory */
        }

        /* old port does not exist, avoid unnecessary updates */
        originalPortID = newRecordTemplate->portID;
    }
    else
    {
        /* a node with the same key exists, will update node */
        newDescriptor = (MacDescriptor *) node->data;

        /* save original port id */
        originalPortID = newDescriptor->portID;
    }

    /* copy/update fields into new record */
    memcpy(newDescriptor->macAddress, newRecordTemplate->macAddress, sizeof (IxEthDBMacAddr));
    memcpy(&newDescriptor->recordData, &newRecordTemplate->recordData, sizeof (IxEthDBRecordData));

    newDescriptor->type   = newRecordTemplate->type;
    newDescriptor->portID = newRecordTemplate->portID;
    newDescriptor->user   = newRecordTemplate->user;

    if (node == NULL)
    {
        /* new record, insert into hashtable */
        BUSY_RETRY_WITH_RESULT(ixEthDBAddHashEntry(&dbHashtable, newDescriptor), result);

        if (result != IX_ETH_DB_SUCCESS)
        {
            ixEthDBFreeMacDescriptor(newDescriptor);

            return result; /* insertion failed */
        }
    }

    if (node != NULL)
    {
        /* release access */
        ixEthDBReleaseHashNode(node);
    }

    /* trigger add/remove update if required by type */
    if (updateTrigger != NULL &&
        ixEthDBPortUpdateRequired[newRecordTemplate->type])
    {
        /* add new port to update list */
        JOIN_PORT_TO_MAP(updateTrigger, newRecordTemplate->portID);

        /* check if record has moved, we'll need to update the old port as well */
        if (originalPortID != newDescriptor->portID)
        {
            JOIN_PORT_TO_MAP(updateTrigger, originalPortID);
        }
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief remove a record from the Ethernet database
 *
 * @param templateRecord template record used to determine
 * what record is to be removed
 * @param updateTrigger port map containing the update triggers
 * resulting from this update operation
 *
 * This function will examine the template record it receives
 * and attempts to delete a record of the same type and containing
 * the same keys as the template record. If deletion is successful
 * and the record type is registered for automatic port updates the
 * port will also be set in the updateTrigger port map, so that the
 * client can perform an update of the port.
 *
 * @retval IX_ETH_DB_SUCCESS removal was successful
 * @retval IX_ETH_DB_NO_SUCH_ADDR the record with the given MAC address was not found
 * @retval IX_ETH_DB_BUSY database busy, cannot remove due to locking
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBRemove(MacDescriptor *templateRecord, IxEthDBPortMap updateTrigger)
{
    IxEthDBStatus result;

    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    BUSY_RETRY_WITH_RESULT(ixEthDBRemoveHashEntry(&dbHashtable, ixEthDBKeyType[templateRecord->type], templateRecord), result);

    if (result != IX_ETH_DB_SUCCESS)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* trigger add/remove update if required by type */
    if (updateTrigger != NULL
        &&ixEthDBPortUpdateRequired[templateRecord->type])
    {
        /* add new port to update list */
        JOIN_PORT_TO_MAP(updateTrigger, templateRecord->portID);
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief register record key types
 *
 * This function registers the appropriate key types,
 * depending on record types.
 *
 * All filtering records use the MAC address as the key.
 * WiFi and Firewall records use a compound key consisting
 * in both the MAC address and the port ID.
 *
 * @return the number of registered record types
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBKeyTypeRegister(UINT32 *keyType)
{
    /* safety */
    memset(keyType, 0, sizeof (keyType));

    /* register all known record types */
    keyType[IX_ETH_DB_FILTERING_RECORD]      = IX_ETH_DB_MAC_KEY;
    keyType[IX_ETH_DB_FILTERING_VLAN_RECORD] = IX_ETH_DB_MAC_KEY;
    keyType[IX_ETH_DB_ALL_FILTERING_RECORDS] = IX_ETH_DB_MAC_KEY;
    keyType[IX_ETH_DB_WIFI_RECORD]           = IX_ETH_DB_MAC_PORT_KEY;
    keyType[IX_ETH_DB_FIREWALL_RECORD]       = IX_ETH_DB_MAC_PORT_KEY;

    return 5;
}

/**
 * @brief Sets a user-defined field into a database record
 *
 * Note that this function is fully documented in the main component
 * header file.
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBUserFieldSet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, IxEthDBVlanId vlanID, void *field)
{
    HashNode *result = NULL;

    if (macAddr == NULL)
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    if (recordType == IX_ETH_DB_FILTERING_RECORD)
    {
        result = ixEthDBSearch(macAddr, recordType);
    }
    else if (recordType == IX_ETH_DB_FILTERING_VLAN_RECORD)
    {
        result = ixEthDBVlanSearch(macAddr, vlanID, recordType);
    }
    else if (recordType == IX_ETH_DB_WIFI_RECORD || recordType == IX_ETH_DB_FIREWALL_RECORD)
    {
        IX_ETH_DB_CHECK_PORT_EXISTS(portID);

        result = ixEthDBPortSearch(macAddr, portID, recordType);
    }
    else
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    if (result == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR;
    }

    ((MacDescriptor *) result->data)->user = field;

    ixEthDBReleaseHashNode(result);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief Retrieves a user-defined field from a database record
 *
 * Note that this function is fully documented in the main component
 * header file.
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBUserFieldGet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, IxEthDBVlanId vlanID, void **field)
{
    HashNode *result = NULL;

    if (macAddr == NULL || field == NULL)
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    if (recordType == IX_ETH_DB_FILTERING_RECORD)
    {
        result = ixEthDBSearch(macAddr, recordType);
    }
    else if (recordType == IX_ETH_DB_FILTERING_VLAN_RECORD)
    {
        result = ixEthDBVlanSearch(macAddr, vlanID, recordType);
    }
    else if (recordType == IX_ETH_DB_WIFI_RECORD || recordType == IX_ETH_DB_FIREWALL_RECORD)
    {
        IX_ETH_DB_CHECK_PORT_EXISTS(portID);

        result = ixEthDBPortSearch(macAddr, portID, recordType);
    }
    else
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    if (result == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR;
    }

    *field = ((MacDescriptor *) result->data)->user;

    ixEthDBReleaseHashNode(result);

    return IX_ETH_DB_SUCCESS;
}
