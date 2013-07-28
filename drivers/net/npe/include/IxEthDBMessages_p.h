/**
 * @file IxEthDBMessages_p.h
 *
 * @brief Definitions of NPE messages
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

#ifndef IxEthDBMessages_p_H
#define IxEthDBMessages_p_H

#include <IxEthNpe.h>
#include <IxOsCacheMMU.h>
#include "IxEthDB_p.h"

/* events watched by the Eth event processor */
#define IX_ETH_DB_MIN_EVENT_ID        (IX_ETHNPE_EDB_GETMACADDRESSDATABASE)
#define IX_ETH_DB_MAX_EVENT_ID        (IX_ETHNPE_PC_SETAPMACTABLE)

/* macros to fill and extract data from NPE messages - place any endian conversions here */
#define RESET_ELT_MESSAGE(message) { memset((void *) &(message), 0, sizeof((message))); }
#define NPE_MSG_ID(msg) ((msg).data[0] >> 24)

#define FILL_SETPORTVLANTABLEENTRY_MSG(message, portID, setOffset, vlanMembershipSet, ttiSet) \
    do { \
        message.data[0] = (IX_ETHNPE_VLAN_SETPORTVLANTABLEENTRY << 24) | (portID << 16) | (setOffset * 2); \
        message.data[1] = (vlanMembershipSet << 8) | ttiSet; \
    } while (0);

#define FILL_SETPORTVLANTABLERANGE_MSG(message, portID, offset, length, zone) \
    do { \
        message.data[0] = IX_ETHNPE_VLAN_SETPORTVLANTABLERANGE << 24 | portID << 16 | offset << 9 | length << 1; \
        message.data[1] = (UINT32) zone; \
    } while (0);

#define FILL_SETDEFAULTRXVID_MSG(message, portID, tpid, vlanTag) \
    do { \
        message.data[0] = (IX_ETHNPE_VLAN_SETDEFAULTRXVID << 24) \
            | (portID << 16); \
        \
        message.data[1] = (tpid << 16) | vlanTag; \
    } while (0);

#define FILL_SETRXTAGMODE_MSG(message, portID, filterMode, tagMode) \
    do { \
        message.data[0] = IX_ETHNPE_VLAN_SETRXTAGMODE << 24 \
            | portID << 16 \
            | filterMode << 2 \
            | tagMode; \
        \
        message.data[1] = 0; \
    } while (0);

#define FILL_SETRXQOSENTRY(message, portID, classIndex, trafficClass, aqmQueue) \
    do { \
        message.data[0] = IX_ETHNPE_VLAN_SETRXQOSENTRY << 24 \
            | portID << 16 \
            | classIndex; \
        \
        message.data[1] = trafficClass << 24 \
            | 0x1 << 23 \
            | aqmQueue << 16 \
            | aqmQueue << 4; \
    } while (0);

#define FILL_SETPORTIDEXTRACTIONMODE(message, portID, enable) \
    do { \
        message.data[0] = IX_ETHNPE_VLAN_SETPORTIDEXTRACTIONMODE << 24 \
            | portID << 16 \
            | (enable ? 0x1 : 0x0); \
        \
        message.data[1] = 0; \
    } while (0);


#define FILL_SETBLOCKINGSTATE_MSG(message, portID, blocked) \
    do { \
        message.data[0] = IX_ETHNPE_STP_SETBLOCKINGSTATE << 24 \
            | portID << 16 \
            | (blocked ? 0x1 : 0x0); \
        \
        message.data[1] = 0; \
    } while (0);

#define FILL_SETBBSID_MSG(message, portID, bbsid) \
    do { \
        message.data[0] = IX_ETHNPE_PC_SETBBSID << 24 \
            | portID << 16 \
            | bbsid->macAddress[0] << 8 \
            | bbsid->macAddress[1]; \
        \
        message.data[1] = bbsid->macAddress[2] << 24 \
            | bbsid->macAddress[3] << 16 \
            | bbsid->macAddress[4] << 8 \
            | bbsid->macAddress[5]; \
    } while (0);

#define FILL_SETFRAMECONTROLDURATIONID(message, portID, frameControlDurationID) \
    do { \
        message.data[0] = (IX_ETHNPE_PC_SETFRAMECONTROLDURATIONID << 24) | (portID << 16); \
        message.data[1] = frameControlDurationID; \
    } while (0);

#define FILL_SETAPMACTABLE_MSG(message, zone) \
    do { \
        message.data[0] = IX_ETHNPE_PC_SETAPMACTABLE << 24 \
            | 0 << 8      /* always use index 0 */ \
            | 64;         /* 32 entries, 8 bytes each, 4 bytes in a word */ \
        message.data[1] = (UINT32) zone; \
    } while (0);

#define FILL_SETFIREWALLMODE_MSG(message, portID, epDelta, mode, address) \
    do { \
        message.data[0] = IX_ETHNPE_FW_SETFIREWALLMODE << 24 \
            | portID << 16 \
            | (epDelta & 0xFF) << 8 \
            | mode; \
        \
        message.data[1] = (UINT32) address; \
    } while (0);

#define FILL_SETMACADDRESSDATABASE_MSG(message, portID, epDelta, blockCount, address) \
    do { \
        message.data[0] = IX_ETHNPE_EDB_SETMACADDRESSSDATABASE << 24 \
                | (epDelta & 0xFF) << 8 \
                | (blockCount & 0xFF); \
        \
        message.data[1] = (UINT32) address; \
    } while (0);

#define FILL_GETMACADDRESSDATABASE(message, npeId, zone) \
    do { \
        message.data[0] = IX_ETHNPE_EDB_GETMACADDRESSDATABASE << 24; \
        message.data[1] = (UINT32) zone; \
    } while (0);

#define FILL_SETMAXFRAMELENGTHS_MSG(message, portID, maxRxFrameSize, maxTxFrameSize) \
    do { \
        message.data[0] = IX_ETHNPE_SETMAXFRAMELENGTHS << 24 \
            | portID << 16 \
            | ((maxRxFrameSize + 63) / 64) << 8  /* max Rx 64-byte blocks */ \
            | (maxTxFrameSize + 63) / 64; /* max Tx 64-byte blocks */ \
        \
        message.data[1] = maxRxFrameSize << 16 | maxTxFrameSize; \
    } while (0);

#define FILL_SETPORTADDRESS_MSG(message, portID, macAddress) \
    do { \
        message.data[0] = IX_ETHNPE_EDB_SETPORTADDRESS << 24 \
            | portID << 16 \
            | macAddress[0] << 8 \
            | macAddress[1]; \
        \
        message.data[1] = macAddress[2] << 24 \
            | macAddress[3] << 16 \
            | macAddress[4] << 8 \
            | macAddress[5]; \
    } while (0);

/* access to a MAC node in the NPE tree */
#define NPE_NODE_BYTE(eltNodeAddr, offset)      (((UINT8 *) (eltNodeAddr))[offset])

/* browsing of the implicit linear binary tree structure of the NPE tree */
#define LEFT_CHILD_OFFSET(offset)   ((offset) << 1)
#define RIGHT_CHILD_OFFSET(offset)  (((offset) << 1) + 1)

#define IX_EDB_FLAGS_ACTIVE         (0x2)
#define IX_EDB_FLAGS_VALID          (0x1)
#define IX_EDB_FLAGS_RESERVED       (0xfc)
#define IX_EDB_FLAGS_INACTIVE_VALID (0x1)

#define IX_EDB_NPE_NODE_ELT_PORT_ID_OFFSET (6)
#define IX_EDB_NPE_NODE_ELT_FLAGS_OFFSET   (7)
#define IX_EDB_NPE_NODE_WIFI_INDEX_OFFSET  (6)
#define IX_EDB_NPE_NODE_WIFI_FLAGS_OFFSET  (7)
#define IX_EDB_NPE_NODE_FW_FLAGS_OFFSET    (1)
#define IX_EDB_NPE_NODE_FW_RESERVED_OFFSET (6)
#define IX_EDB_NPE_NODE_FW_ADDR_OFFSET     (2)

#define IX_EDB_NPE_NODE_VALID(address)     ((NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_ELT_FLAGS_OFFSET) & IX_EDB_FLAGS_VALID) != 0)
#define IX_EDB_NPE_NODE_ACTIVE(address)    ((NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_ELT_FLAGS_OFFSET) & IX_EDB_FLAGS_ACTIVE) != 0)
#define IX_EDB_NPE_NODE_PORT_ID(address)   (NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_ELT_PORT_ID_OFFSET))

/* macros to send messages to the NPEs */
#define IX_ETHDB_ASYNC_SEND_NPE_MSG(npeId, msg, result) \
    do { \
        result = ixNpeMhMessageSend(npeId, msg, IX_NPEMH_SEND_RETRIES_DEFAULT); \
        \
        if (result != IX_SUCCESS) \
        { \
            ERROR_LOG("DB: Failed to send NPE message\n"); \
        } \
    } while (0);

#define IX_ETHDB_SYNC_SEND_NPE_MSG(npeId, msg, result) \
    do { \
        result = ixNpeMhMessageWithResponseSend(npeId, msg, msg.data[0] >> 24, ixEthDBNpeMsgAck, IX_NPEMH_SEND_RETRIES_DEFAULT); \
        \
        if (result == IX_SUCCESS) \
        { \
            result = ixOsalMutexLock(&ixEthDBPortInfo[IX_ETH_DB_NPE_TO_PORT_ID(npeId)].npeAckLock, IX_ETH_DB_NPE_TIMEOUT); \
            \
            if (result != IX_SUCCESS) \
            { \
                ERROR_LOG("DB: NPE failed to respond within %dms\n", IX_ETH_DB_NPE_TIMEOUT); \
            } \
        } \
        else \
        { \
            ERROR_LOG("DB: Failed to send NPE message\n"); \
        } \
    } while (0);

#ifndef IX_NDEBUG
#define IX_ETH_DB_NPE_MSG_HISTORY_DEPTH (100)
extern IX_ETH_DB_PUBLIC UINT32 npeMsgHistory[IX_ETH_DB_NPE_MSG_HISTORY_DEPTH][2];
extern IX_ETH_DB_PUBLIC UINT32 npeMsgHistoryLen;
#endif

#define IX_ETHDB_SEND_NPE_MSG(npeId, msg, result) { LOG_NPE_MSG(msg); IX_ETHDB_SYNC_SEND_NPE_MSG(npeId, msg, result); }

#endif /* IxEthDBMessages_p_H */
