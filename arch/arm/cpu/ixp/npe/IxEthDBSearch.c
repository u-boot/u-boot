/**
 * @file IxEthDBSearch.c
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

extern HashTable dbHashtable;

/**
 * @brief matches two database records based on their MAC addresses
 *
 * @param untypedReference record to match against
 * @param untypedEntry record to match
 *
 * @return TRUE if the match is successful or FALSE otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
BOOL ixEthDBAddressRecordMatch(void *untypedReference, void *untypedEntry)
{
    MacDescriptor *entry     = (MacDescriptor *) untypedEntry;
    MacDescriptor *reference = (MacDescriptor *) untypedReference;
    
    /* check accepted record types */
    if ((entry->type & reference->type) == 0) return FALSE;
       
    return (ixEthDBAddressCompare((UINT8 *) entry->macAddress, (UINT8 *) reference->macAddress) == 0);
}

/**
 * @brief matches two database records based on their MAC addresses
 * and VLAN IDs
 *
 * @param untypedReference record to match against
 * @param untypedEntry record to match
 *
 * @return TRUE if the match is successful or FALSE otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
BOOL ixEthDBVlanRecordMatch(void *untypedReference, void *untypedEntry)
{
    MacDescriptor *entry     = (MacDescriptor *) untypedEntry;
    MacDescriptor *reference = (MacDescriptor *) untypedReference;
    
    /* check accepted record types */
    if ((entry->type & reference->type) == 0) return FALSE;
    
    return (IX_ETH_DB_GET_VLAN_ID(entry->recordData.filteringVlanData.ieee802_1qTag) ==
        IX_ETH_DB_GET_VLAN_ID(reference->recordData.filteringVlanData.ieee802_1qTag)) &&
        (ixEthDBAddressCompare(entry->macAddress, reference->macAddress) == 0);
}

/**
 * @brief matches two database records based on their MAC addresses
 * and port IDs
 *
 * @param untypedReference record to match against
 * @param untypedEntry record to match
 *
 * @return TRUE if the match is successful or FALSE otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
BOOL ixEthDBPortRecordMatch(void *untypedReference, void *untypedEntry)
{
    MacDescriptor *entry     = (MacDescriptor *) untypedEntry;
    MacDescriptor *reference = (MacDescriptor *) untypedReference;
    
    /* check accepted record types */
    if ((entry->type & reference->type) == 0) return FALSE;
    
    return (entry->portID == reference->portID) &&
        (ixEthDBAddressCompare(entry->macAddress, reference->macAddress) == 0);
}

/**
 * @brief dummy matching function, registered for safety
 *
 * @param reference record to match against (unused)
 * @param entry record to match (unused)
 *
 * This function is registered in the matching functions
 * array on invalid types. Calling it will display an 
 * error message, indicating an error in the component logic.
 *
 * @return FALSE
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
BOOL ixEthDBNullMatch(void *reference, void *entry)
{
    /* display an error message */

    ixOsalLog(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT, "DB: (Search) The NullMatch function was called, wrong key type?\n", 0, 0, 0, 0, 0, 0);


    return FALSE;
}

/**
 * @brief registers hash matching methods
 *
 * @param matchFunctions table of match functions to be populated
 *
 * This function registers the available record matching functions
 * by indexing them on record types into the given function array.
 * 
 * Note that it is compulsory to call this in ixEthDBInit(), 
 * otherwise hashtable searching and removal will not work
 *
 * @return number of registered functions
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBMatchMethodsRegister(MatchFunction *matchFunctions)
{
    UINT32 i;
    
    /* safety first */
    for ( i = 0 ; i < IX_ETH_DB_MAX_KEY_INDEX + 1 ; i++)
    {
        matchFunctions[i] = ixEthDBNullMatch;
    }
    
    /* register MAC search method */
    matchFunctions[IX_ETH_DB_MAC_KEY] = ixEthDBAddressRecordMatch;
    
    /* register MAC/PortID search method */
    matchFunctions[IX_ETH_DB_MAC_PORT_KEY] = ixEthDBPortRecordMatch;
    
    /* register MAC/VLAN ID search method */
    matchFunctions[IX_ETH_DB_MAC_VLAN_KEY] = ixEthDBVlanRecordMatch;
    
    return 3; /* three methods */
}

/**
 * @brief search a record in the Ethernet datbase
 *
 * @param macAddress MAC address to perform the search on
 * @param typeFilter type of records to consider for matching
 *
 * @warning if searching is successful an implicit write lock
 * to the search result is granted, therefore unlock the 
 * entry using @ref ixEthDBReleaseHashNode() as soon as possible.
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @return the search result, or NULL if a record with the given
 * MAC address was not found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
HashNode* ixEthDBSearch(IxEthDBMacAddr *macAddress, IxEthDBRecordType typeFilter)
{
    HashNode *searchResult = NULL;
    MacDescriptor reference;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    if (macAddress == NULL)
    {
        return NULL;
    }

    /* fill search fields */
    memcpy(reference.macAddress, macAddress, sizeof (IxEthDBMacAddr));
    
    /* set acceptable record types */
    reference.type = typeFilter;
    
    BUSY_RETRY(ixEthDBSearchHashEntry(&dbHashtable, IX_ETH_DB_MAC_KEY, &reference, &searchResult));

    return searchResult;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPeek(IxEthDBMacAddr *macAddress, IxEthDBRecordType typeFilter)
{
    MacDescriptor reference;
    IxEthDBStatus result;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    if (macAddress == NULL)
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    /* fill search fields */
    memcpy(reference.macAddress, macAddress, sizeof (IxEthDBMacAddr));
    
    /* set acceptable record types */
    reference.type = typeFilter;
    
    result = ixEthDBPeekHashEntry(&dbHashtable, IX_ETH_DB_MAC_KEY, &reference);

    return result;
}

/**
 * @brief search a record in the Ethernet datbase
 *
 * @param macAddress MAC address to perform the search on
 * @param portID port ID to perform the search on
 * @param typeFilter type of records to consider for matching
 *
 * @warning if searching is successful an implicit write lock
 * to the search result is granted, therefore unlock the 
 * entry using @ref ixEthDBReleaseHashNode() as soon as possible.
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @return the search result, or NULL if a record with the given
 * MAC address/port ID combination was not found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
HashNode* ixEthDBPortSearch(IxEthDBMacAddr *macAddress, IxEthDBPortId portID, IxEthDBRecordType typeFilter)
{
    HashNode *searchResult = NULL;
    MacDescriptor reference;
    
    if (macAddress == NULL)
    {
        return NULL;
    }
    
    /* fill search fields */
    memcpy(reference.macAddress, macAddress, sizeof (IxEthDBMacAddr));
    reference.portID = portID;
    
    /* set acceptable record types */
    reference.type = typeFilter;

    BUSY_RETRY(ixEthDBSearchHashEntry(&dbHashtable, IX_ETH_DB_MAC_PORT_KEY, &reference, &searchResult));

    return searchResult;
}

/**
 * @brief search a record in the Ethernet datbase
 *
 * @param macAddress MAC address to perform the search on
 * @param vlanID VLAN ID to perform the search on
 * @param typeFilter type of records to consider for matching
 *
 * @warning if searching is successful an implicit write lock
 * to the search result is granted, therefore unlock the 
 * entry using @ref ixEthDBReleaseHashNode() as soon as possible.
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @return the search result, or NULL if a record with the given
 * MAC address/VLAN ID combination was not found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
HashNode* ixEthDBVlanSearch(IxEthDBMacAddr *macAddress, IxEthDBVlanId vlanID, IxEthDBRecordType typeFilter)
{
    HashNode *searchResult = NULL;
    MacDescriptor reference;
    
    if (macAddress == NULL)
    {
        return NULL;
    }
    
    /* fill search fields */
    memcpy(reference.macAddress, macAddress, sizeof (IxEthDBMacAddr));
    reference.recordData.filteringVlanData.ieee802_1qTag = 
            IX_ETH_DB_SET_VLAN_ID(reference.recordData.filteringVlanData.ieee802_1qTag, vlanID);
    
    /* set acceptable record types */
    reference.type = typeFilter;

    BUSY_RETRY(ixEthDBSearchHashEntry(&dbHashtable, IX_ETH_DB_MAC_VLAN_KEY, &reference, &searchResult));

    return searchResult;
}
