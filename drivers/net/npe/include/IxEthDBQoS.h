/**
 * @file IxEthDBQoS.h
 *
 * @brief Public definitions for QoS traffic classes
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

/**
 * @defgroup IxEthDBPortDefs IXP400 Ethernet QoS definitions
 *
 * @brief IXP00 Public definitions for QoS traffic classes
 *
 * @{
 */

#ifndef IxEthDBQoS_H
#define IxEthDBQoS_H

/**
 * @def IX_ETH_DB_QUEUE_UNAVAILABLE
 * @brief alias to indicate a queue (traffic class) is not available
 */
#define IX_ETH_DB_QUEUE_UNAVAILABLE  (0)

#ifndef IX_IEEE802_1Q_QOS_PRIORITY_COUNT
/**
 * @def IX_IEEE802_1Q_QOS_PRIORITY_COUNT
 * @brief number of QoS priorities, according to IEEE 802.1Q
 */
#define IX_IEEE802_1Q_QOS_PRIORITY_COUNT (8)
#endif

/**
 * @brief array containing all the supported traffic class configurations
 */
static const
UINT8 ixEthDBQueueAssignments[][IX_IEEE802_1Q_QOS_PRIORITY_COUNT] = 
{
    { 4, 5, 6, 7, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE },
    { 15, 16, 17, 18, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE },
    { 11, 23, 26, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE, IX_ETH_DB_QUEUE_UNAVAILABLE,  IX_ETH_DB_QUEUE_UNAVAILABLE },
    { 4, 5, 6, 7, 8, 9, 10, 11 }
    /* add here all other cases of queue configuration structures and update ixEthDBTrafficClassDefinitions to use them */
};

/**
 * @brief value used to index the NPE A functionality ID in the traffic class definition table
 */
#define IX_ETH_DB_NPE_A_FUNCTIONALITY_ID_INDEX    (0)

/**
 * @brief value used to index the traffic class count in the traffic class definition table
 */
#define IX_ETH_DB_TRAFFIC_CLASS_COUNT_INDEX       (1)

/**
 * @brief value used to index the queue assignment index in the traffic class definition table
 */
#define IX_ETH_DB_QUEUE_ASSIGNMENT_INDEX          (2)

/**
 * @brief traffic class definitions
 *
 * This array contains the default traffic class definition configuration,
 * as well as any special cases dictated by the functionality ID of NPE A.
 *
 * The default case should not be removed (otherwise the Ethernet
 * components will assert a fatal failure on initialization).
 */
static const
UINT8 ixEthDBTrafficClassDefinitions[][3] = 
{
    /* NPE A functionality ID | traffic class count | queue assignment index (points to the queue enumeration in ixEthDBQueueAssignments) */
    {            0x00,                      4,                    0 }, /* default case - DO NOT REMOVE */
    {            0x04,                      4,                    1 }, /* NPE A image ID 0.4.0.0 */
    {            0x09,                      3,                    2 }, /* NPE A image ID 0.9.0.0 */
    {            0x80,                      8,                    3 }, /* NPE A image ID 10.80.02.0 */
    {            0x81,                      8,                    3 }, /* NPE A image ID 10.81.02.0 */
    {            0x82,                      8,                    3 }  /* NPE A image ID 10.82.02.0 */
};

/**
 * @brief IEEE 802.1Q recommended QoS Priority => traffic class maps
 *
 * @verbatim
                    Number of available traffic classes
                    1 2 3 4 5 6 7 8
    QoS Priority
        0           0 0 0 1 1 1 1 2
        1           0 0 0 0 0 0 0 0
        2           0 0 0 0 0 0 0 1
        3           0 0 0 1 1 2 2 3
        4           0 1 1 2 2 3 3 4
        5           0 1 1 2 3 4 4 5
        6           0 1 2 3 4 5 5 6
        7           0 1 2 3 4 5 6 7

    @endverbatim
 */
static const
UINT8 ixEthIEEE802_1QUserPriorityToTrafficClassMapping[IX_IEEE802_1Q_QOS_PRIORITY_COUNT][IX_IEEE802_1Q_QOS_PRIORITY_COUNT] = 
 {
     { 0, 0, 0, 0, 0, 0, 0, 0 }, /* 1 traffic class available */
     { 0, 0, 0, 0, 1, 1, 1, 1 }, /* 2 traffic classes available */
     { 0, 0, 0, 0, 1, 1, 2, 2 }, /* 3 traffic classes available */
     { 1, 0, 0, 1, 2, 2, 3, 3 }, /* 4 traffic classes available */
     { 1, 0, 0, 1, 2, 3, 4, 4 }, /* 5 traffic classes available */
     { 1, 0, 0, 2, 3, 4, 5, 5 }, /* 6 traffic classes available */
     { 1, 0, 0, 2, 3, 4, 5, 6 }, /* 7 traffic classes available */
     { 2, 0, 1, 3, 4, 5, 6, 7 }  /* 8 traffic classes available */
 };

#endif /* IxEthDBQoS_H */

/**
 *@}
 */
