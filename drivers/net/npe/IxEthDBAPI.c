/**
 * @file IxEthDBAPI.c
 *
 * @brief Implementation of the public API
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

#include "IxEthDB_p.h"
#include "IxFeatureCtrl.h"

extern HashTable dbHashtable;
extern IxEthDBPortMap overflowUpdatePortList;
extern BOOL ixEthDBPortUpdateRequired[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1];

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_LEARNING);

    return ixEthDBTriggerAddPortUpdate(macAddr, portID, true);
}
    
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
        
    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_LEARNING);

    return ixEthDBTriggerAddPortUpdate(macAddr, portID, false);
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;

    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr, IX_ETH_DB_ALL_FILTERING_RECORDS);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }
    
    ixEthDBReleaseHashNode(searchResult);
    
    /* build a remove event and place it on the event queue */
    return ixEthDBTriggerRemovePortUpdate(macAddr, ((MacDescriptor *) searchResult->data)->portID);
}
       
IX_ETH_DB_PUBLIC
void ixEthDBDatabaseMaintenance()
{
    HashIterator iterator;
    UINT32 portIndex;
    BOOL agingRequired = false;

    /* ports who will have deleted records and therefore will need updating */
    IxEthDBPortMap triggerPorts;

    if (IX_FEATURE_CTRL_SWCONFIG_ENABLED !=
        ixFeatureCtrlSwConfigurationCheck (IX_FEATURECTRL_ETH_LEARNING))
    {
        return;
    }

    SET_EMPTY_DEPENDENCY_MAP(triggerPorts);

    /* check if there's at least a port that needs aging */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (ixEthDBPortInfo[portIndex].agingEnabled && ixEthDBPortInfo[portIndex].enabled)
        {
            agingRequired = true;
        }
    }

    if (agingRequired)
    {
        /* ask each NPE port to write back the database for aging inspection */
        for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
        {
            if (ixEthDBPortDefinitions[portIndex].type == IX_ETH_NPE
                && ixEthDBPortInfo[portIndex].agingEnabled
                && ixEthDBPortInfo[portIndex].enabled)
            {
                IxNpeMhMessage message;
                IX_STATUS result;
                
                /* send EDB_GetMACAddressDatabase message */
                FILL_GETMACADDRESSDATABASE(message, 
                    0 /* unused */, 
                    IX_OSAL_MMU_VIRT_TO_PHYS(ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone));

                IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portIndex), message, result);

                if (result == IX_SUCCESS)
                {
                    /* analyze NPE copy */
                    ixEthDBNPESyncScan(portIndex, ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone, FULL_ELT_BYTE_SIZE);

                    IX_ETH_DB_SUPPORT_TRACE("DB: (API) Finished scanning NPE tree on port %d\n", portIndex);
                }
                else
                {
                    ixEthDBPortInfo[portIndex].agingEnabled                = false;
                    ixEthDBPortInfo[portIndex].updateMethod.updateEnabled  = false;
                    ixEthDBPortInfo[portIndex].updateMethod.userControlled = true;

                    ixOsalLog(IX_OSAL_LOG_LVL_FATAL,
                        IX_OSAL_LOG_DEV_STDOUT, 
                        "EthDB: (Maintenance) warning, disabling aging and updates for port %d (assumed dead)\n",
                        portIndex, 0, 0, 0, 0, 0);

                    ixEthDBDatabaseClear(portIndex, IX_ETH_DB_ALL_RECORD_TYPES);
                }
            }
        }

        /* browse database and age entries */
        BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

        while (IS_ITERATOR_VALID(&iterator))
        {
            MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;
            UINT32 *age               = NULL;
            BOOL staticEntry          = true;

            if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
            {
                age         = &descriptor->recordData.filteringData.age;
                staticEntry = descriptor->recordData.filteringData.staticEntry;
            }
            else if (descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD)
            {
                age         = &descriptor->recordData.filteringVlanData.age;
                staticEntry = descriptor->recordData.filteringVlanData.staticEntry;
            }
            else
            {
                staticEntry = true;
            }

            if (ixEthDBPortInfo[descriptor->portID].agingEnabled && (staticEntry == false))
            {
                /* manually increment the age if the port has no such capability */
                if ((ixEthDBPortDefinitions[descriptor->portID].capabilities & IX_ETH_ENTRY_AGING) == 0)
                {
                    *age += (IX_ETH_DB_MAINTENANCE_TIME / 60);
                }

                /* age entry if it exceeded the maximum time to live */
                if (*age >= (IX_ETH_DB_LEARNING_ENTRY_AGE_TIME / 60))
                {
                    /* add port to the set of update trigger ports */
                    JOIN_PORT_TO_MAP(triggerPorts, descriptor->portID);

                    /* delete entry */
                    BUSY_RETRY(ixEthDBRemoveEntryAtHashIterator(&dbHashtable, &iterator));
                }
                else
                {
                    /* move to the next record */
                    BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
                }
            }
            else
            {
                /* move to the next record */
                BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
            }
        }

        /* update ports which lost records */
        ixEthDBUpdatePortLearningTrees(triggerPorts);
    }
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBDatabaseClear(IxEthDBPortId portID, IxEthDBRecordType recordType)
{
    IxEthDBPortMap triggerPorts;
    HashIterator iterator;

    if (portID >= IX_ETH_DB_NUMBER_OF_PORTS && portID != IX_ETH_DB_ALL_PORTS)
    {
        return IX_ETH_DB_INVALID_PORT;
    }

    /* check if the user passes some extra bits */
    if ((recordType | IX_ETH_DB_ALL_RECORD_TYPES) != IX_ETH_DB_ALL_RECORD_TYPES)
    {
        return IX_ETH_DB_INVALID_ARG;
    }

    SET_EMPTY_DEPENDENCY_MAP(triggerPorts);
    
    /* browse database and age entries */
    BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

    while (IS_ITERATOR_VALID(&iterator))
    {
        MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

        if (((descriptor->portID == portID) || (portID == IX_ETH_DB_ALL_PORTS))
            && ((descriptor->type & recordType) != 0))
        {
            /* add to trigger if automatic updates are required */
            if (ixEthDBPortUpdateRequired[descriptor->type])
            {
                /* add port to the set of update trigger ports */
                JOIN_PORT_TO_MAP(triggerPorts, descriptor->portID);
            }

            /* delete entry */
            BUSY_RETRY(ixEthDBRemoveEntryAtHashIterator(&dbHashtable, &iterator));
        }
        else
        {
            /* move to the next record */
            BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
        }
    }

    /* update ports which lost records */
    ixEthDBUpdatePortLearningTrees(triggerPorts);
    
    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortSearch(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;
    IxEthDBStatus result = IX_ETH_DB_NO_SUCH_ADDR;

    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_LEARNING);

    searchResult = ixEthDBSearch(macAddr, IX_ETH_DB_ALL_FILTERING_RECORDS);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    if (((MacDescriptor *) (searchResult->data))->portID == portID)
    {
        result = IX_ETH_DB_SUCCESS; /* address and port match */
    }

    ixEthDBReleaseHashNode(searchResult);

    return result;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;

    IX_ETH_DB_CHECK_REFERENCE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr, IX_ETH_DB_ALL_FILTERING_RECORDS);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* return the port ID */
    *portID = ((MacDescriptor *) searchResult->data)->portID;

    ixEthDBReleaseHashNode(searchResult);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAgingDisable(IxEthDBPortId portID)
{
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_LEARNING);

    ixEthDBPortInfo[portID].agingEnabled = false;

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAgingEnable(IxEthDBPortId portID)
{
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_LEARNING);

    ixEthDBPortInfo[portID].agingEnabled = true;

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;
    MacDescriptor *descriptor;

    IX_ETH_DB_CHECK_REFERENCE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr, IX_ETH_DB_ALL_FILTERING_RECORDS);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }
    
    descriptor = (MacDescriptor *) searchResult->data;

    /* return the port ID */
    *portID = descriptor->portID;

    /* reset entry age */
    if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
    {
        descriptor->recordData.filteringData.age = 0;
    }
    else
    {
        descriptor->recordData.filteringVlanData.age = 0;
    }

    ixEthDBReleaseHashNode(searchResult);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDependencyMapSet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(dependencyPortMap);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FILTERING);

    /* force bit at offset 255 to 0 (reserved) */
    dependencyPortMap[31] &= 0xFE;

    COPY_DEPENDENCY_MAP(ixEthDBPortInfo[portID].dependencyPortMap, dependencyPortMap);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDependencyMapGet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(dependencyPortMap);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FILTERING);

    COPY_DEPENDENCY_MAP(dependencyPortMap, ixEthDBPortInfo[portID].dependencyPortMap);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortUpdateEnableSet(IxEthDBPortId portID, BOOL enableUpdate)
{
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);    

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FILTERING);

    ixEthDBPortInfo[portID].updateMethod.updateEnabled  = enableUpdate;
    ixEthDBPortInfo[portID].updateMethod.userControlled = true;

    return IX_ETH_DB_SUCCESS;
}
