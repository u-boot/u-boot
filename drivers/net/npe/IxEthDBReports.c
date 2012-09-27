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
IX_ETH_DB_PRIVATE void ixEthDBPortInfoShow(IxEthDBPortId portID, IxEthDBRecordType recordFilter);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBHeaderShow(IxEthDBRecordType recordFilter);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBDependencyPortMapShow(IxEthDBPortId portID, IxEthDBPortMap map);

/**
 * @brief displays a port dependency map
 *
 * @param portID ID of the port
 * @param map port map to display
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully
 */ 
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBDependencyPortMapShow(IxEthDBPortId portID, IxEthDBPortMap map)
{
    UINT32 portIndex;
    BOOL mapSelf = TRUE, mapNone = TRUE, firstPort = TRUE;
    
    /* dependency port maps */
    printf("Dependency port map: ");
    
    /* browse the port map */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (IS_PORT_INCLUDED(portIndex, map))
        {
            mapNone   = FALSE;
            
            if (portIndex != portID)
            {
                mapSelf = FALSE;
            }
            
            printf("%s%d", firstPort ? "{" : ", ", portIndex);
            
            firstPort = FALSE;
        }
    }
    
    if (mapNone)
    {
        mapSelf = FALSE;
    }
    
    printf("%s (%s)\n", firstPort ? "" : "}", mapSelf ? "self" : mapNone ? "none" : "group");
    
    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief displays all the filtering records belonging to a port
 *
 * @param portID ID of the port to display
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @warning deprecated, use @ref ixEthDBFilteringDatabaseShowRecords() 
 * instead. Calling this function is equivalent to calling
 * ixEthDBFilteringDatabaseShowRecords(portID, IX_ETH_DB_FILTERING_RECORD)
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDatabaseShow(IxEthDBPortId portID)
{
    IxEthDBStatus local_result;
    HashIterator iterator;
    PortInfo *portInfo;
    UINT32 recordCount = 0;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    portInfo = &ixEthDBPortInfo[portID];

    /* display table header */
    printf("Ethernet database records for port ID [%d]\n", portID);
    
    ixEthDBDependencyPortMapShow(portID, portInfo->dependencyPortMap);
    
    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        printf("NPE updates are %s\n\n", portInfo->updateMethod.updateEnabled ? "enabled" : "disabled");
    }
    else
    {
        printf("updates disabled (not an NPE)\n\n");
    }

    printf("    MAC address    |   Age  | Type \n");
    printf("___________________________________\n");

    /* browse database */
    BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

    while (IS_ITERATOR_VALID(&iterator))
    {
      MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

      if (descriptor->portID == portID && descriptor->type == IX_ETH_DB_FILTERING_RECORD)
      {
          recordCount++;

          /* display entry */
          printf(" %02X:%02X:%02X:%02X:%02X:%02X | %5d  | %s\n",
              descriptor->macAddress[0],
              descriptor->macAddress[1],
              descriptor->macAddress[2],
              descriptor->macAddress[3],
              descriptor->macAddress[4],
              descriptor->macAddress[5],
              descriptor->recordData.filteringData.age,
              descriptor->recordData.filteringData.staticEntry ? "static" : "dynamic");
      }

      /* move to the next record */
      BUSY_RETRY_WITH_RESULT(ixEthDBIncrementHashIterator(&dbHashtable, &iterator), local_result);

      /* debug */
      if (local_result == IX_ETH_DB_BUSY)
      {
          return IX_ETH_DB_FAIL;
      }
    }

    /* display number of records */
    printf("\nFound %d records\n", recordCount);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief displays all the filtering records belonging to all the ports
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @warning deprecated, use @ref ixEthDBFilteringDatabaseShowRecords() 
 * instead. Calling this function is equivalent to calling
 * ixEthDBFilteringDatabaseShowRecords(IX_ETH_DB_ALL_PORTS, IX_ETH_DB_FILTERING_RECORD)
 */
IX_ETH_DB_PUBLIC
void ixEthDBFilteringDatabaseShowAll()
{
    IxEthDBPortId portIndex;

    printf("\nEthernet learning/filtering database: listing %d ports\n\n", (UINT32) IX_ETH_DB_NUMBER_OF_PORTS);

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        ixEthDBFilteringDatabaseShow(portIndex);

        if (portIndex < IX_ETH_DB_NUMBER_OF_PORTS - 1)
        {
            printf("\n");
        }
    }
}

/**
 * @brief displays one record in a format depending on the record filter
 *
 * @param descriptor pointer to the record
 * @param recordFilter format filter
 *
 * This function will display the fields in a record depending on the
 * selected record filter.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBRecordShow(MacDescriptor *descriptor, IxEthDBRecordType recordFilter)
{
    if (recordFilter == IX_ETH_DB_FILTERING_VLAN_RECORD
        || recordFilter == (IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD))
    {
        /* display VLAN record header - leave this commented code in place, its purpose is to align the print format with the header
        printf("    MAC address    |   Age  |   Type   | VLAN ID | CFI | QoS class \n");
        printf("___________________________________________________________________\n"); */

        if (descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD)
        {
            printf("%02X:%02X:%02X:%02X:%02X:%02X | %3d | %s | %d | %d | %d\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5],
                descriptor->recordData.filteringVlanData.age,
                descriptor->recordData.filteringVlanData.staticEntry ? "static" : "dynamic",
                IX_ETH_DB_GET_VLAN_ID(descriptor->recordData.filteringVlanData.ieee802_1qTag),
                (descriptor->recordData.filteringVlanData.ieee802_1qTag & 0x1000) >> 12,
                IX_ETH_DB_GET_QOS_PRIORITY(descriptor->recordData.filteringVlanData.ieee802_1qTag));
         }
         else if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
         {
            printf("%02X:%02X:%02X:%02X:%02X:%02X | %3d | %s | - | - | -\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5],
                descriptor->recordData.filteringData.age,
                descriptor->recordData.filteringData.staticEntry ? "static" : "dynamic");
         }
    }
    else if (recordFilter == IX_ETH_DB_FILTERING_RECORD)
    {
        /* display filtering record header - leave this commented code in place, its purpose is to align the print format with the header
        printf("    MAC address    |   Age  |   Type   \n");
        printf("_______________________________________\n");  */

        if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
        {
         printf("%02X:%02X:%02X:%02X:%02X:%02X | %3d | %s \n",
             descriptor->macAddress[0],
             descriptor->macAddress[1],
             descriptor->macAddress[2],
             descriptor->macAddress[3],
             descriptor->macAddress[4],
             descriptor->macAddress[5],
             descriptor->recordData.filteringData.age,
             descriptor->recordData.filteringData.staticEntry ? "static" : "dynamic");
        }
    }
    else if (recordFilter == IX_ETH_DB_WIFI_RECORD)
    {
        /* display WiFi record header - leave this commented code in place, its purpose is to align the print format with the header 
        printf("    MAC address    |   GW MAC address  \n");
        printf("_______________________________________\n"); */

        if (descriptor->type == IX_ETH_DB_WIFI_RECORD)
        {
            if (descriptor->recordData.wifiData.type == IX_ETH_DB_WIFI_AP_TO_AP)
            {
                /* gateway address present */
                printf("%02X:%02X:%02X:%02X:%02X:%02X | %02X:%02X:%02X:%02X:%02X:%02X \n",
                    descriptor->macAddress[0],
                    descriptor->macAddress[1],
                    descriptor->macAddress[2],
                    descriptor->macAddress[3],
                    descriptor->macAddress[4],
                    descriptor->macAddress[5],
                    descriptor->recordData.wifiData.gwMacAddress[0],
                    descriptor->recordData.wifiData.gwMacAddress[1],
                    descriptor->recordData.wifiData.gwMacAddress[2],
                    descriptor->recordData.wifiData.gwMacAddress[3],
                    descriptor->recordData.wifiData.gwMacAddress[4],
                    descriptor->recordData.wifiData.gwMacAddress[5]);
            }
            else
            {
                /* no gateway */
                printf("%02X:%02X:%02X:%02X:%02X:%02X | ----no gateway----- \n",
                    descriptor->macAddress[0],
                    descriptor->macAddress[1],
                    descriptor->macAddress[2],
                    descriptor->macAddress[3],
                    descriptor->macAddress[4],
                    descriptor->macAddress[5]);
            }
        }
    }
    else if (recordFilter == IX_ETH_DB_FIREWALL_RECORD)
    {
        /* display Firewall record header - leave this commented code in place, its purpose is to align the print format with the header 
        printf("    MAC address   \n");
        printf("__________________\n"); */

        if (descriptor->type == IX_ETH_DB_FIREWALL_RECORD)
        {
            printf("%02X:%02X:%02X:%02X:%02X:%02X \n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5]);
        }
    }
    else if (recordFilter == IX_ETH_DB_ALL_RECORD_TYPES)
    {
        /* display composite record header - leave this commented code in place, its purpose is to align the print format with the header 
        printf("    MAC address   | Record | Age|  Type   | VLAN |CFI| QoS |  GW MAC address   \n");
        printf("_______________________________________________________________________________\n"); */

        if (descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD)
        {
            printf("%02X:%02X:%02X:%02X:%02X:%02X |  VLAN  | %2d | %s | %4d | %1d |  %1d  | -----------------\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5],
                descriptor->recordData.filteringVlanData.age,
                descriptor->recordData.filteringVlanData.staticEntry ? "static " : "dynamic",
                IX_ETH_DB_GET_VLAN_ID(descriptor->recordData.filteringVlanData.ieee802_1qTag),
                (descriptor->recordData.filteringVlanData.ieee802_1qTag & 0x1000) >> 12,
                IX_ETH_DB_GET_QOS_PRIORITY(descriptor->recordData.filteringVlanData.ieee802_1qTag));
         }
         else if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
         {
            printf("%02X:%02X:%02X:%02X:%02X:%02X | Filter | %2d | %s | ---- | - | --- | -----------------\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5],
                descriptor->recordData.filteringData.age,
                descriptor->recordData.filteringData.staticEntry ? "static " : "dynamic");
         }
        else if (descriptor->type == IX_ETH_DB_WIFI_RECORD)
        {
            if (descriptor->recordData.wifiData.type == IX_ETH_DB_WIFI_AP_TO_AP)
            {
                /* gateway address present */
                printf("%02X:%02X:%02X:%02X:%02X:%02X |  WiFi  | -- | AP=>AP  | ---- | - | --- | %02X:%02X:%02X:%02X:%02X:%02X\n",
                    descriptor->macAddress[0],
                    descriptor->macAddress[1],
                    descriptor->macAddress[2],
                    descriptor->macAddress[3],
                    descriptor->macAddress[4],
                    descriptor->macAddress[5],
                    descriptor->recordData.wifiData.gwMacAddress[0],
                    descriptor->recordData.wifiData.gwMacAddress[1],
                    descriptor->recordData.wifiData.gwMacAddress[2],
                    descriptor->recordData.wifiData.gwMacAddress[3],
                    descriptor->recordData.wifiData.gwMacAddress[4],
                    descriptor->recordData.wifiData.gwMacAddress[5]);
            }
            else
            {
                /* no gateway */
                printf("%02X:%02X:%02X:%02X:%02X:%02X |  WiFi  | -- | AP=>ST  | ---- | - | --- | -- no gateway -- \n",
                    descriptor->macAddress[0],
                    descriptor->macAddress[1],
                    descriptor->macAddress[2],
                    descriptor->macAddress[3],
                    descriptor->macAddress[4],
                    descriptor->macAddress[5]);
            }
        }
        else if (descriptor->type == IX_ETH_DB_FIREWALL_RECORD)
        {
            printf("%02X:%02X:%02X:%02X:%02X:%02X |   FW   | -- | ------- | ---- | - | --- | -----------------\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5]);
        }
    }
    else
    {
        printf("invalid record filter\n");
    }
}

/**
 * @brief displays the status, records and configuration information of a port
 *
 * @param portID ID of the port
 * @param recordFilter record filter to display
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBPortInfoShow(IxEthDBPortId portID, IxEthDBRecordType recordFilter)
{
    PortInfo *portInfo = &ixEthDBPortInfo[portID];
    UINT32 recordCount = 0;
    HashIterator iterator;
    IxEthDBStatus local_result;

    /* display port status */
    printf("== Port ID %d ==\n", portID);

    /* display capabilities */
    printf("- Capabilities: ");

    if ((portInfo->featureCapability & IX_ETH_DB_LEARNING) != 0)
    {
        printf("Learning (%s) ", ((portInfo->featureStatus & IX_ETH_DB_LEARNING) != 0) ? "on" : "off");
    }

    if ((portInfo->featureCapability & IX_ETH_DB_VLAN_QOS) != 0)
    {
        printf("VLAN/QoS (%s) ", ((portInfo->featureStatus & IX_ETH_DB_VLAN_QOS) != 0) ? "on" : "off");
    }

    if ((portInfo->featureCapability & IX_ETH_DB_FIREWALL) != 0)
    {
        printf("Firewall (%s) ", ((portInfo->featureStatus & IX_ETH_DB_FIREWALL) != 0) ? "on" : "off");
    }

    if ((portInfo->featureCapability & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0)
    {
        printf("WiFi (%s) ", ((portInfo->featureStatus & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0) ? "on" : "off");
    }

    if ((portInfo->featureCapability & IX_ETH_DB_SPANNING_TREE_PROTOCOL) != 0)
    {
        printf("STP (%s) ", ((portInfo->featureStatus & IX_ETH_DB_SPANNING_TREE_PROTOCOL) != 0) ? "on" : "off");
    }

    printf("\n");

    /* dependency map */
    ixEthDBDependencyPortMapShow(portID, portInfo->dependencyPortMap);

    /* NPE dynamic updates */
    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE) 
    {
        printf(" - NPE dynamic update is %s\n", portInfo->updateMethod.updateEnabled ? "enabled" : "disabled");
    }
    else
    {
        printf(" - dynamic update disabled (not an NPE)\n");
    }

    if ((portInfo->featureCapability & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0)
    {
        if ((portInfo->featureStatus & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0)
        {
            /* WiFi header conversion */
            if ((portInfo->frameControlDurationID 
                + portInfo->bbsid[0] 
                + portInfo->bbsid[1] 
                + portInfo->bbsid[2] 
                + portInfo->bbsid[3] 
                + portInfo->bbsid[4] 
                + portInfo->bbsid[5]) == 0)
            {
                printf(" - WiFi header conversion not configured\n");
            }
            else
            {  
                printf(" - WiFi header conversion: BBSID [%02X:%02X:%02X:%02X:%02X:%02X], Frame Control 0x%X, Duration/ID 0x%X\n", 
                    portInfo->bbsid[0],
                    portInfo->bbsid[1],
                    portInfo->bbsid[2],
                    portInfo->bbsid[3],
                    portInfo->bbsid[4],
                    portInfo->bbsid[5],
                    portInfo->frameControlDurationID >> 16,
                    portInfo->frameControlDurationID & 0xFFFF);
            }
        }
        else
        {
            printf(" - WiFi header conversion not enabled\n");
        }
    }

    /* Firewall */
    if ((portInfo->featureCapability & IX_ETH_DB_FIREWALL) != 0)
    {
        if ((portInfo->featureStatus & IX_ETH_DB_FIREWALL) != 0)
        {
            printf(" - Firewall is in %s-list mode\n", portInfo->firewallMode == IX_ETH_DB_FIREWALL_BLACK_LIST ? "black" : "white");
            printf(" - Invalid source MAC address filtering is %s\n", portInfo->srcAddressFilterEnabled ? "enabled" : "disabled");
        }
        else
        {
            printf(" - Firewall not enabled\n");
        }
    }
  
    /* browse database if asked to display records */
    if (recordFilter != IX_ETH_DB_NO_RECORD_TYPE)
    {
        printf("\n");
        ixEthDBHeaderShow(recordFilter);

        BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

        while (IS_ITERATOR_VALID(&iterator))
        {
            MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

            if (descriptor->portID == portID && (descriptor->type & recordFilter) != 0)
            {
                recordCount++;

                /* display entry */
                ixEthDBRecordShow(descriptor, recordFilter);
            }

            /* move to the next record */
            BUSY_RETRY_WITH_RESULT(ixEthDBIncrementHashIterator(&dbHashtable, &iterator), local_result);

            /* debug */
            if (local_result == IX_ETH_DB_BUSY)
            {
                printf("EthDB (API): Error, database browser failed (no access), giving up\n");
            }
        }
        
        printf("\nFound %d records\n\n", recordCount);
    }
}

/**
 * @brief displays a record header
 *
 * @param recordFilter record type filter
 *
 * This function displays a record header, depending on
 * the given record type filter. It is useful when used
 * in conjunction with ixEthDBRecordShow which will display
 * record fields formatted for the header, provided the same
 * record filter is used.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or IX_ETH_DB_INVALID_ARG if the recordFilter
 * parameter is invalid or not supported
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBHeaderShow(IxEthDBRecordType recordFilter)
{
  if (recordFilter == IX_ETH_DB_FILTERING_VLAN_RECORD
      || recordFilter == (IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD))
  {
    /* display VLAN record header */
    printf("    MAC address    |   Age  |   Type   | VLAN ID | CFI | QoS class \n");
    printf("___________________________________________________________________\n");
  }
  else if (recordFilter == IX_ETH_DB_FILTERING_RECORD)
  {
    /* display filtering record header */
    printf("    MAC address    |   Age  |   Type   \n");
    printf("_______________________________________\n");
  }
  else if (recordFilter == IX_ETH_DB_WIFI_RECORD)
  {
    /* display WiFi record header */
    printf("    MAC address    |   GW MAC address  \n");
    printf("_______________________________________\n");
  }
  else if (recordFilter == IX_ETH_DB_FIREWALL_RECORD)
  {
    /* display Firewall record header */
    printf("    MAC address   \n");
    printf("__________________\n");
  }
  else if (recordFilter == IX_ETH_DB_ALL_RECORD_TYPES)
  {
    /* display composite record header */
    printf("    MAC address   | Record | Age|  Type   | VLAN |CFI| QoS |  GW MAC address   \n");
    printf("_______________________________________________________________________________\n");
  }
  else
  {
    return IX_ETH_DB_INVALID_ARG;
  }
  
  return IX_ETH_DB_SUCCESS;
}

/**
 * @brief displays database information (records and port information)
 *
 * @param portID ID of the port to display (or IX_ETH_DB_ALL_PORTS for all the ports)
 * @param recordFilter record filter (use IX_ETH_DB_NO_RECORD_TYPE to display only
 * port information)
 * 
 * Note that this function is documented in the main component header
 * file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully or
 * an appropriate error code otherwise
 * 
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDatabaseShowRecords(IxEthDBPortId portID, IxEthDBRecordType recordFilter)
{
  IxEthDBPortId currentPort;
  BOOL showAllPorts = (portID == IX_ETH_DB_ALL_PORTS);
  
  IX_ETH_DB_CHECK_PORT_ALL(portID);

  printf("\nEthernet learning/filtering database: listing %d port(s)\n\n", showAllPorts ? (UINT32) IX_ETH_DB_NUMBER_OF_PORTS : 1);
  
  currentPort = showAllPorts ? 0 : portID;
  
  while (currentPort != IX_ETH_DB_NUMBER_OF_PORTS)
  {
    /* display port info */
    ixEthDBPortInfoShow(currentPort, recordFilter);
    
    /* next port */
    currentPort = showAllPorts ? currentPort + 1 : IX_ETH_DB_NUMBER_OF_PORTS;
  }
  
  return IX_ETH_DB_SUCCESS;
}

