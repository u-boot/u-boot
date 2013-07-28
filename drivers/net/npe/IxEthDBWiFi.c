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

/* forward prototypes */
IX_ETH_DB_PUBLIC
MacTreeNode *ixEthDBGatewaySelect(MacTreeNode *stations);

/**
 * @brief sets the BBSID value for the WiFi header conversion feature
 *
 * @param portID ID of the port
 * @param bbsid pointer to the 6-byte BBSID value
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiBBSIDSet(IxEthDBPortId portID, IxEthDBMacAddr *bbsid)
{
    IxNpeMhMessage message;
    IX_STATUS result;
    
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);
 
    IX_ETH_DB_CHECK_REFERENCE(bbsid);
    
    memcpy(ixEthDBPortInfo[portID].bbsid, bbsid, sizeof (IxEthDBMacAddr));

    FILL_SETBBSID_MSG(message, portID, bbsid);

    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);
    
    return result;
}

/**
 * @brief updates the Frame Control and Duration/ID WiFi header
 * conversion parameters in an NPE
 *
 * @param portID ID of the port
 *
 * This function will send a message to the NPE updating the 
 * frame conversion parameters for 802.3 => 802.11 header conversion.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or IX_ETH_DB_FAIL otherwise
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBWiFiFrameControlDurationIDUpdate(IxEthDBPortId portID)
{
    IxNpeMhMessage message;
    IX_STATUS result;

    FILL_SETFRAMECONTROLDURATIONID(message, portID, ixEthDBPortInfo[portID].frameControlDurationID);
    
    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);
    
    return result;
}

/**
 * @brief sets the Duration/ID WiFi frame header conversion parameter
 *
 * @param portID ID of the port
 * @param durationID 16-bit value containing the new Duration/ID parameter
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiDurationIDSet(IxEthDBPortId portID, UINT16 durationID)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    ixEthDBPortInfo[portID].frameControlDurationID = (ixEthDBPortInfo[portID].frameControlDurationID & 0xFFFF0000) | durationID;
    
    return ixEthDBWiFiFrameControlDurationIDUpdate(portID);
}

/**
 * @brief sets the Frame Control WiFi frame header conversion parameter
 *
 * @param portID ID of the port
 * @param durationID 16-bit value containing the new Frame Control parameter
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiFrameControlSet(IxEthDBPortId portID, UINT16 frameControl)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    ixEthDBPortInfo[portID].frameControlDurationID = (ixEthDBPortInfo[portID].frameControlDurationID & 0xFFFF) | (frameControl << 16); 
    
    return ixEthDBWiFiFrameControlDurationIDUpdate(portID);
}

/**
 * @brief removes a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to remove
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    MacDescriptor recordTemplate;
    
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);
    
    memcpy(recordTemplate.macAddress, macAddr, sizeof (IxEthDBMacAddr));
    
    recordTemplate.type   = IX_ETH_DB_WIFI_RECORD;
    recordTemplate.portID = portID;
    
    return ixEthDBRemove(&recordTemplate, NULL);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 * @param gatewayMacAddr address of the gateway (or
 * NULL if this is a station record)
 *
 * This function adds a record of type AP_TO_AP (gateway is not NULL)
 * or AP_TO_STA (gateway is NULL) in the main database as a 
 * WiFi header conversion record.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBWiFiEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBMacAddr *gatewayMacAddr)
{
    MacDescriptor recordTemplate;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    memcpy(recordTemplate.macAddress, macAddr, sizeof (IxEthDBMacAddr));
    
    recordTemplate.type   = IX_ETH_DB_WIFI_RECORD;
    recordTemplate.portID = portID;
    
    if (gatewayMacAddr != NULL)
    {
        memcpy(recordTemplate.recordData.wifiData.gwMacAddress, gatewayMacAddr, sizeof (IxEthDBMacAddr));
        
        recordTemplate.recordData.wifiData.type = IX_ETH_DB_WIFI_AP_TO_AP;
    }
    else
    {
        memset(recordTemplate.recordData.wifiData.gwMacAddress, 0, sizeof (IxEthDBMacAddr));

        recordTemplate.recordData.wifiData.type = IX_ETH_DB_WIFI_AP_TO_STA;
    }
    
    return ixEthDBAdd(&recordTemplate, NULL);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 * @param gatewayMacAddr address of the gateway 
 *
 * This function adds a record of type AP_TO_AP
 * in the main database as a WiFi header conversion record.
 *
 * This is simply a wrapper over @ref ixEthDBWiFiEntryAdd().
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiAccessPointEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBMacAddr *gatewayMacAddr)
{
    IX_ETH_DB_CHECK_REFERENCE(gatewayMacAddr);

    return ixEthDBWiFiEntryAdd(portID, macAddr, gatewayMacAddr);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 *
 * This function adds a record of type AP_TO_STA
 * in the main database as a WiFi header conversion record.
 *
 * This is simply a wrapper over @ref ixEthDBWiFiEntryAdd().
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiStationEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    return ixEthDBWiFiEntryAdd(portID, macAddr, NULL);
}

/**
 * @brief selects a set of gateways from a tree of 
 * WiFi header conversion records
 *
 * @param stations binary tree containing pointers to WiFi header
 * conversion records
 *
 * This function browses through the input binary tree, identifies
 * records of type AP_TO_AP, clones these records and appends them
 * to a vine (a single right-branch binary tree) which is returned
 * as result. A maximum of MAX_GW_SIZE entries containing gateways
 * will be cloned from the original tree.
 *
 * @return vine (linear binary tree) containing record
 * clones of AP_TO_AP type, which have a gateway field
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacTreeNode *ixEthDBGatewaySelect(MacTreeNode *stations)
{
    MacTreeNodeStack *stack;
    MacTreeNode *gateways, *insertionPlace;
    UINT32 gwIndex = 1; /* skip the empty root */
    
    if (stations == NULL)
    {
        return NULL;
    }

    stack = ixOsalCacheDmaMalloc(sizeof (MacTreeNodeStack));

    if (stack == NULL)
    {
        ERROR_LOG("DB: (WiFi) failed to allocate the node stack for gateway tree linearization, out of memory?\n");
        return NULL;
    }
    
    /* initialize root node */
    gateways = insertionPlace = NULL;
        
    /* start browsing the station tree */
    NODE_STACK_INIT(stack);
    
    /* initialize stack by pushing the tree root at offset 0 */
    NODE_STACK_PUSH(stack, stations, 0);
    
    while (NODE_STACK_NONEMPTY(stack))
    {
        MacTreeNode *node;
        UINT32 offset;
       
        NODE_STACK_POP(stack, node, offset);

        /* we can store maximum 31 (32 total, 1 empty root) entries in the gateway tree */
        if (offset > (MAX_GW_SIZE - 1)) break;
        
        /* check if this record has a gateway address */
        if (node->descriptor != NULL && node->descriptor->recordData.wifiData.type == IX_ETH_DB_WIFI_AP_TO_AP)
        {
            /* found a record, create an insertion place */
            if (insertionPlace != NULL)
            {
                insertionPlace->right = ixEthDBAllocMacTreeNode();
                insertionPlace        = insertionPlace->right;
            }
            else
            {
                gateways       = ixEthDBAllocMacTreeNode();
                insertionPlace = gateways;
            }

            if (insertionPlace == NULL)
            {
                /* no nodes left, bail out with what we have */
                ixOsalCacheDmaFree(stack);
                return gateways;
            }
            
            /* clone the original record for the gateway tree */
            insertionPlace->descriptor = ixEthDBCloneMacDescriptor(node->descriptor);
            
            /* insert and update the offset in the original record */
            node->descriptor->recordData.wifiData.gwAddressIndex = gwIndex++;
        }
        
        /* browse the tree */
        if (node->left != NULL)
        {
            NODE_STACK_PUSH(stack, node->left, LEFT_CHILD_OFFSET(offset));
        }

        if (node->right != NULL)
        {
            NODE_STACK_PUSH(stack, node->right, RIGHT_CHILD_OFFSET(offset));
        }
    }
    
    ixOsalCacheDmaFree(stack);
    return gateways;    
}

/**
 * @brief downloads the WiFi header conversion table to an NPE
 *
 * @param portID ID of the port
 *
 * This function prepares the WiFi header conversion tables and
 * downloads them to the specified NPE port.
 *
 * The header conversion tables consist in the main table of
 * addresses and the secondary table of gateways. AP_TO_AP records
 * from the first table contain index fields into the second table
 * for gateway selection.
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiConversionTableDownload(IxEthDBPortId portID)
{
    IxEthDBPortMap query;
    MacTreeNode *stations = NULL, *gateways = NULL, *gateway = NULL;
    IxNpeMhMessage message;
    PortInfo *portInfo;
    IX_STATUS result;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    portInfo = &ixEthDBPortInfo[portID];
   
    SET_DEPENDENCY_MAP(query, portID);
    
    ixEthDBUpdateLock();

    stations = ixEthDBQuery(NULL, query, IX_ETH_DB_WIFI_RECORD, MAX_ELT_SIZE);
    gateways = ixEthDBGatewaySelect(stations);
    
    /* clean up gw area */
    memset((void *) portInfo->updateMethod.npeGwUpdateZone, FULL_GW_BYTE_SIZE, 0);

    /* write all gateways */
    gateway = gateways;

    while (gateway != NULL)
    {
        ixEthDBNPEGatewayNodeWrite((void *) (((UINT32) portInfo->updateMethod.npeGwUpdateZone) 
            + gateway->descriptor->recordData.wifiData.gwAddressIndex * ELT_ENTRY_SIZE), 
            gateway);

        gateway = gateway->right;
    }

    /* free the gateway tree */
    if (gateways != NULL)
    {
        ixEthDBFreeMacTreeNode(gateways);
    }

    FILL_SETAPMACTABLE_MSG(message, 
        IX_OSAL_MMU_VIRT_TO_PHYS(portInfo->updateMethod.npeGwUpdateZone));

    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);

    if (result == IX_SUCCESS)
    {
        /* update the main tree (the stations tree) */
        portInfo->updateMethod.searchTree = stations;
        
        result = ixEthDBNPEUpdateHandler(portID, IX_ETH_DB_WIFI_RECORD);
    }

    ixEthDBUpdateUnlock();

    return result;
}
