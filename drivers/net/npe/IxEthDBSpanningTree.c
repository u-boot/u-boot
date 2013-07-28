/**
 * @file IxEthDBSpanningTree.c
 *
 * @brief Implementation of the STP API
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

/**
 * @brief sets the STP blocking state of a port
 *
 * @param portID ID of the port
 * @param blocked true to block the port or false to unblock it
 * 
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateSet(IxEthDBPortId portID, BOOL blocked)
{
    IxNpeMhMessage message;
    IX_STATUS result;
    
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
 
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_SPANNING_TREE_PROTOCOL);
    
    ixEthDBPortInfo[portID].stpBlocked = blocked;

    FILL_SETBLOCKINGSTATE_MSG(message, portID, blocked);
    
    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);
    
    return result;
}

/**
 * @brief retrieves the STP blocking state of a port
 *
 * @param portID ID of the port
 * @param blocked address to write the blocked status into
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateGet(IxEthDBPortId portID, BOOL *blocked)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
 
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_SPANNING_TREE_PROTOCOL);
    
    IX_ETH_DB_CHECK_REFERENCE(blocked);
    
    *blocked = ixEthDBPortInfo[portID].stpBlocked;
    
    return IX_ETH_DB_SUCCESS;
}
