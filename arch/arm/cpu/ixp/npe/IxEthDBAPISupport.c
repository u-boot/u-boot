/**
 * @file IxEthDBAPISupport.c
 *
 * @brief Public API support functions
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

#include <IxEthDB.h>
#include <IxNpeMh.h>
#include <IxFeatureCtrl.h>

#include "IxEthDB_p.h"
#include "IxEthDBMessages_p.h"
#include "IxEthDB_p.h"
#include "IxEthDBLog_p.h"

#ifdef IX_UNIT_TEST

int dbAccessCounter = 0;
int overflowEvent   = 0;

#endif

/*
 * External declaration
 */
extern HashTable dbHashtable;

/*
 * Internal declaration
 */
IX_ETH_DB_PUBLIC
PortInfo ixEthDBPortInfo[IX_ETH_DB_NUMBER_OF_PORTS];

IX_ETH_DB_PRIVATE
struct
{
    BOOL saved;
    IxEthDBPriorityTable priorityTable;
    IxEthDBVlanSet vlanMembership;
    IxEthDBVlanSet transmitTaggingInfo;
    IxEthDBFrameFilter frameFilter;
    IxEthDBTaggingAction taggingAction;
    IxEthDBFirewallMode firewallMode;
    BOOL stpBlocked;
    BOOL srcAddressFilterEnabled;
    UINT32 maxRxFrameSize;
    UINT32 maxTxFrameSize;
} ixEthDBPortState[IX_ETH_DB_NUMBER_OF_PORTS];

#define IX_ETH_DB_DEFAULT_FRAME_SIZE (1518)

/**
 * @brief initializes a port
 *
 * @param portID ID of the port to be initialized
 *
 * Note that redundant initializations are silently
 * dealt with and do not constitute an error
 *
 * This function is fully documented in the main
 * header file, IxEthDB.h
 */
IX_ETH_DB_PUBLIC
void ixEthDBPortInit(IxEthDBPortId portID)
{
    PortInfo *portInfo;

    if (portID >= IX_ETH_DB_NUMBER_OF_PORTS)
    {
        return;
    }

    portInfo = &ixEthDBPortInfo[portID];

    if (ixEthDBSingleEthNpeCheck(portID) != IX_ETH_DB_SUCCESS)
    {
        WARNING_LOG("EthDB: Unavailable Eth %d: Cannot initialize EthDB Port.\n", (UINT32) portID);

        return;
    }

    if (portInfo->initialized)
    {
        /* redundant */
        return;
    }

    /* initialize core fields */
    portInfo->portID = portID;
    SET_DEPENDENCY_MAP(portInfo->dependencyPortMap, portID);

    /* default values */
    portInfo->agingEnabled       = FALSE;
    portInfo->enabled            = FALSE;
    portInfo->macAddressUploaded = FALSE;
    portInfo->maxRxFrameSize     = IX_ETHDB_DEFAULT_FRAME_SIZE;
    portInfo->maxTxFrameSize     = IX_ETHDB_DEFAULT_FRAME_SIZE;

    /* default update control values */
    portInfo->updateMethod.searchTree              = NULL;
    portInfo->updateMethod.searchTreePendingWrite  = FALSE;
    portInfo->updateMethod.treeInitialized         = FALSE;
    portInfo->updateMethod.updateEnabled           = FALSE;
    portInfo->updateMethod.userControlled          = FALSE;

    /* default WiFi parameters */
    memset(portInfo->bbsid, 0, sizeof (portInfo->bbsid));
    portInfo->frameControlDurationID = 0;

    /* Ethernet NPE-specific initializations */
    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        /* update handler */
        portInfo->updateMethod.updateHandler = ixEthDBNPEUpdateHandler;
    }

    /* initialize state save */
    ixEthDBPortState[portID].saved = FALSE;

    portInfo->initialized = TRUE;
}

/**
 * @brief enables a port
 *
 * @param portID ID of the port to enable
 *
 * This function is fully documented in the main
 * header file, IxEthDB.h
 *
 * @return IX_ETH_DB_SUCCESS if enabling was
 * accomplished, or a meaningful error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID)
{
    IxEthDBPortMap triggerPorts;
    PortInfo *portInfo;

    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    portInfo = &ixEthDBPortInfo[portID];

    if (portInfo->enabled)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

    SET_DEPENDENCY_MAP(triggerPorts, portID);

    /* mark as enabled */
    portInfo->enabled = TRUE;

    /* Operation stops here when Ethernet Learning is not enabled */
    if(IX_FEATURE_CTRL_SWCONFIG_DISABLED ==
       ixFeatureCtrlSwConfigurationCheck(IX_FEATURECTRL_ETH_LEARNING))
    {
        return IX_ETH_DB_SUCCESS;
    }

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE && !portInfo->macAddressUploaded)
    {
        IX_ETH_DB_SUPPORT_TRACE("DB: (Support) MAC address not set on port %d, enable failed\n", portID);

        /* must use UnicastAddressSet() before enabling an NPE port */
        return IX_ETH_DB_MAC_UNINITIALIZED;
    }

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Attempting to enable the NPE callback for port %d...\n", portID);

        if (!portInfo->updateMethod.userControlled
                && ((portInfo->featureCapability & IX_ETH_DB_FILTERING) != 0))
        {
            portInfo->updateMethod.updateEnabled = TRUE;
        }

        /* if this is first time initialization then we already have
           write access to the tree and can AccessRelease directly */
        if (!portInfo->updateMethod.treeInitialized)
        {
            IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Initializing tree for port %d\n", portID);

            /* create an initial tree and release access into it */
            ixEthDBUpdatePortLearningTrees(triggerPorts);

            /* mark tree as being initialized */
            portInfo->updateMethod.treeInitialized = TRUE;
        }
    }

    if (ixEthDBPortState[portID].saved)
    {
        /* previous configuration data stored, restore state */
        if ((portInfo->featureCapability & IX_ETH_DB_FIREWALL) != 0)
        {
            ixEthDBFirewallModeSet(portID, ixEthDBPortState[portID].firewallMode);
            ixEthDBFirewallInvalidAddressFilterEnable(portID, ixEthDBPortState[portID].srcAddressFilterEnabled);
        }

#if 0 /* test-only */
        if ((portInfo->featureCapability & IX_ETH_DB_VLAN_QOS) != 0)
        {
            ixEthDBAcceptableFrameTypeSet(portID, ixEthDBPortState[portID].frameFilter);
            ixEthDBIngressVlanTaggingEnabledSet(portID, ixEthDBPortState[portID].taggingAction);

            ixEthDBEgressVlanTaggingEnabledSet(portID, ixEthDBPortState[portID].transmitTaggingInfo);
            ixEthDBPortVlanMembershipSet(portID, ixEthDBPortState[portID].vlanMembership);

            ixEthDBPriorityMappingTableSet(portID, ixEthDBPortState[portID].priorityTable);
        }
#endif

        if ((portInfo->featureCapability & IX_ETH_DB_SPANNING_TREE_PROTOCOL) != 0)
        {
            ixEthDBSpanningTreeBlockingStateSet(portID, ixEthDBPortState[portID].stpBlocked);
        }

        ixEthDBFilteringPortMaximumRxFrameSizeSet(portID, ixEthDBPortState[portID].maxRxFrameSize);
        ixEthDBFilteringPortMaximumTxFrameSizeSet(portID, ixEthDBPortState[portID].maxTxFrameSize);

        /* discard previous save */
        ixEthDBPortState[portID].saved = FALSE;
    }

    IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Enabling succeeded for port %d\n", portID);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief disables a port
 *
 * @param portID ID of the port to disable
 *
 * This function is fully documented in the
 * main header file, IxEthDB.h
 *
 * @return IX_ETH_DB_SUCCESS if disabling was
 * successful or an appropriate error message
 * otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID)
{
    HashIterator iterator;
    IxEthDBPortMap triggerPorts; /* ports who will have deleted records and therefore will need updating */
    BOOL result;
    PortInfo *portInfo;
    IxEthDBFeature learningEnabled;
#if 0 /* test-only */
    IxEthDBPriorityTable classZeroTable;
#endif

    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    portInfo = &ixEthDBPortInfo[portID];

    if (!portInfo->enabled)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        /* save filtering state */
        ixEthDBPortState[portID].firewallMode            = portInfo->firewallMode;
        ixEthDBPortState[portID].frameFilter             = portInfo->frameFilter;
        ixEthDBPortState[portID].taggingAction           = portInfo->taggingAction;
        ixEthDBPortState[portID].stpBlocked              = portInfo->stpBlocked;
        ixEthDBPortState[portID].srcAddressFilterEnabled = portInfo->srcAddressFilterEnabled;
        ixEthDBPortState[portID].maxRxFrameSize          = portInfo->maxRxFrameSize;
        ixEthDBPortState[portID].maxTxFrameSize          = portInfo->maxTxFrameSize;

        memcpy(ixEthDBPortState[portID].vlanMembership, portInfo->vlanMembership, sizeof (IxEthDBVlanSet));
        memcpy(ixEthDBPortState[portID].transmitTaggingInfo, portInfo->transmitTaggingInfo, sizeof (IxEthDBVlanSet));
        memcpy(ixEthDBPortState[portID].priorityTable, portInfo->priorityTable, sizeof (IxEthDBPriorityTable));

        ixEthDBPortState[portID].saved = TRUE;

        /* now turn off all EthDB filtering features on the port */

#if 0 /* test-only */
        /* VLAN & QoS */
        if ((portInfo->featureCapability & IX_ETH_DB_VLAN_QOS) != 0)
        {
            ixEthDBPortVlanMembershipRangeAdd((IxEthDBPortId) portID, 0, IX_ETH_DB_802_1Q_MAX_VLAN_ID);
            ixEthDBEgressVlanRangeTaggingEnabledSet((IxEthDBPortId) portID, 0, IX_ETH_DB_802_1Q_MAX_VLAN_ID, FALSE);
            ixEthDBAcceptableFrameTypeSet((IxEthDBPortId) portID, IX_ETH_DB_ACCEPT_ALL_FRAMES);
            ixEthDBIngressVlanTaggingEnabledSet((IxEthDBPortId) portID, IX_ETH_DB_PASS_THROUGH);

            memset(classZeroTable, 0, sizeof (classZeroTable));
            ixEthDBPriorityMappingTableSet((IxEthDBPortId) portID, classZeroTable);
        }
#endif

        /* STP */
        if ((portInfo->featureCapability & IX_ETH_DB_SPANNING_TREE_PROTOCOL) != 0)
        {
            ixEthDBSpanningTreeBlockingStateSet((IxEthDBPortId) portID, FALSE);
        }

        /* Firewall */
        if ((portInfo->featureCapability & IX_ETH_DB_FIREWALL) != 0)
        {
            ixEthDBFirewallModeSet((IxEthDBPortId) portID, IX_ETH_DB_FIREWALL_BLACK_LIST);
            ixEthDBFirewallTableDownload((IxEthDBPortId) portID);
            ixEthDBFirewallInvalidAddressFilterEnable((IxEthDBPortId) portID, FALSE);
        }

        /* Frame size filter */
        ixEthDBFilteringPortMaximumFrameSizeSet((IxEthDBPortId) portID, IX_ETH_DB_DEFAULT_FRAME_SIZE);

        /* WiFi */
        if ((portInfo->featureCapability & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0)
        {
            ixEthDBWiFiConversionTableDownload((IxEthDBPortId) portID);
        }

        /* save and disable the learning feature bit */
        learningEnabled          = portInfo->featureStatus & IX_ETH_DB_LEARNING;
        portInfo->featureStatus &= ~IX_ETH_DB_LEARNING;
    }
    else
    {
        /* save the learning feature bit */
        learningEnabled          = portInfo->featureStatus & IX_ETH_DB_LEARNING;
    }

    SET_EMPTY_DEPENDENCY_MAP(triggerPorts);

    ixEthDBUpdateLock();

    /* wipe out current entries for this port */
    BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

    while (IS_ITERATOR_VALID(&iterator))
    {
        MacDescriptor *descriptor =  (MacDescriptor *) iterator.node->data;

        /* check if the port match. If so, remove the entry  */
        if (descriptor->portID == portID
                && (descriptor->type == IX_ETH_DB_FILTERING_RECORD || descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD)
                && !descriptor->recordData.filteringData.staticEntry)
        {
            /* delete entry */
            BUSY_RETRY(ixEthDBRemoveEntryAtHashIterator(&dbHashtable, &iterator));

            /* add port to the set of update trigger ports */
            JOIN_PORT_TO_MAP(triggerPorts, portID);
        }
        else
        {
            /* move to the next record */
            BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
        }
    }

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        if (portInfo->updateMethod.searchTree != NULL)
        {
            ixEthDBFreeMacTreeNode(portInfo->updateMethod.searchTree);
            portInfo->updateMethod.searchTree = NULL;
        }

        ixEthDBNPEUpdateHandler(portID, IX_ETH_DB_FILTERING_RECORD);
    }

    /* mark as disabled */
    portInfo->enabled = FALSE;

    /* disable updates unless the user has specifically altered the default behavior */
    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
        if (!portInfo->updateMethod.userControlled)
        {
            portInfo->updateMethod.updateEnabled = FALSE;
        }

        /* make sure we re-initialize the NPE learning tree when the port is re-enabled */
        portInfo->updateMethod.treeInitialized = FALSE;
    }

    ixEthDBUpdateUnlock();

    /* restore learning feature bit */
    portInfo->featureStatus |= learningEnabled;

    /* if we've removed any records or lost any events make sure to force an update */
    IS_EMPTY_DEPENDENCY_MAP(result, triggerPorts);

    if (!result)
    {
        ixEthDBUpdatePortLearningTrees(triggerPorts);
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief sends the updated maximum Tx/Rx frame lengths to the NPE
 *
 * @param portID ID of the port to update
 *
 * @return IX_ETH_DB_SUCCESS if the update completed
 * successfully or an appropriate error message otherwise
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBPortFrameLengthsUpdate(IxEthDBPortId portID)
{
    IxNpeMhMessage message;
    PortInfo *portInfo = &ixEthDBPortInfo[portID];
    IX_STATUS result;

    FILL_SETMAXFRAMELENGTHS_MSG(message, portID, portInfo->maxRxFrameSize, portInfo->maxTxFrameSize);

    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);

    return result;
}

/**
 * @brief sets the port maximum Rx frame size
 *
 * @param portID ID of the port to set the frame size on
 * @param maximumRxFrameSize maximum Rx frame size
 *
 * This function updates the internal data structures and
 * calls ixEthDBPortFrameLengthsUpdate() for NPE update.
 *
 * This function is fully documented in the main header
 * file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation was
 * successfull or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortMaximumRxFrameSizeSet(IxEthDBPortId portID, UINT32 maximumRxFrameSize)
{
    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

	if (!ixEthDBPortInfo[portID].initialized)
	{
		return IX_ETH_DB_PORT_UNINITIALIZED;
	}

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
	if ((maximumRxFrameSize < IX_ETHDB_MIN_NPE_FRAME_SIZE) ||
	    (maximumRxFrameSize > IX_ETHDB_MAX_NPE_FRAME_SIZE))
	{
	    return IX_ETH_DB_INVALID_ARG;
	}
    }
    else
    {
        return IX_ETH_DB_NO_PERMISSION;
    }

    /* update internal structure */
    ixEthDBPortInfo[portID].maxRxFrameSize = maximumRxFrameSize;

    /* update the maximum frame size in the NPE */
    return ixEthDBPortFrameLengthsUpdate(portID);
}

/**
 * @brief sets the port maximum Tx frame size
 *
 * @param portID ID of the port to set the frame size on
 * @param maximumTxFrameSize maximum Tx frame size
 *
 * This function updates the internal data structures and
 * calls ixEthDBPortFrameLengthsUpdate() for NPE update.
 *
 * This function is fully documented in the main header
 * file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation was
 * successfull or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortMaximumTxFrameSizeSet(IxEthDBPortId portID, UINT32 maximumTxFrameSize)
{
    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

	if (!ixEthDBPortInfo[portID].initialized)
	{
		return IX_ETH_DB_PORT_UNINITIALIZED;
	}

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
	if ((maximumTxFrameSize < IX_ETHDB_MIN_NPE_FRAME_SIZE) ||
	    (maximumTxFrameSize > IX_ETHDB_MAX_NPE_FRAME_SIZE))
	{
	    return IX_ETH_DB_INVALID_ARG;
	}
    }
    else
    {
        return IX_ETH_DB_NO_PERMISSION;
    }

    /* update internal structure */
    ixEthDBPortInfo[portID].maxTxFrameSize = maximumTxFrameSize;

    /* update the maximum frame size in the NPE */
    return ixEthDBPortFrameLengthsUpdate(portID);
}

/**
 * @brief sets the port maximum Tx and Rx frame sizes
 *
 * @param portID ID of the port to set the frame size on
 * @param maximumFrameSize maximum Tx and Rx frame sizes
 *
 * This function updates the internal data structures and
 * calls ixEthDBPortFrameLengthsUpdate() for NPE update.
 *
 * Note that both the maximum Tx and Rx frame size are set
 * to the same value. This function is kept for compatibility
 * reasons.
 *
 * This function is fully documented in the main header
 * file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation was
 * successfull or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortMaximumFrameSizeSet(IxEthDBPortId portID, UINT32 maximumFrameSize)
{
    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    if (!ixEthDBPortInfo[portID].initialized)
    {
        return IX_ETH_DB_PORT_UNINITIALIZED;
    }

    if (ixEthDBPortDefinitions[portID].type == IX_ETH_NPE)
    {
	if ((maximumFrameSize < IX_ETHDB_MIN_NPE_FRAME_SIZE) ||
	    (maximumFrameSize > IX_ETHDB_MAX_NPE_FRAME_SIZE))
	{
	    return IX_ETH_DB_INVALID_ARG;
	}
    }
    else
    {
        return IX_ETH_DB_NO_PERMISSION;
    }

    /* update internal structure */
    ixEthDBPortInfo[portID].maxRxFrameSize = maximumFrameSize;
    ixEthDBPortInfo[portID].maxTxFrameSize = maximumFrameSize;

    /* update the maximum frame size in the NPE */
    return ixEthDBPortFrameLengthsUpdate(portID);
}

/**
 * @brief sets the MAC address of an NPE port
 *
 * @param portID port ID to set the MAC address on
 * @param macAddr pointer to the 6-byte MAC address
 *
 * This function is called by the EthAcc
 * ixEthAccUnicastMacAddressSet() and should not be
 * manually invoked unless required by special circumstances.
 *
 * @return IX_ETH_DB_SUCCESS if the operation succeeded
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IxNpeMhMessage message;
    IxOsalMutex *ackPortAddressLock;
    IX_STATUS result;

    /* use this macro instead CHECK_PORT
       as the port doesn't need to be enabled */
    IX_ETH_DB_CHECK_PORT_EXISTS(portID);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    if (!ixEthDBPortInfo[portID].initialized)
    {
        return IX_ETH_DB_PORT_UNINITIALIZED;
    }

    ackPortAddressLock = &ixEthDBPortInfo[portID].npeAckLock;

    /* Operation stops here when Ethernet Learning is not enabled */
    if(IX_FEATURE_CTRL_SWCONFIG_DISABLED ==
       ixFeatureCtrlSwConfigurationCheck(IX_FEATURECTRL_ETH_LEARNING))
    {
        return IX_ETH_DB_SUCCESS;
    }

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    /* exit if the port is not an Ethernet NPE */
    if (ixEthDBPortDefinitions[portID].type != IX_ETH_NPE)
    {
        return IX_ETH_DB_INVALID_PORT;
    }

    /* populate message */
    FILL_SETPORTADDRESS_MSG(message, portID, macAddr->macAddress);

    IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Sending SetPortAddress on port %d...\n", portID);

    /* send a SetPortAddress message */
    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);

    if (result == IX_SUCCESS)
    {
        ixEthDBPortInfo[portID].macAddressUploaded = TRUE;
    }

    return result;
}
