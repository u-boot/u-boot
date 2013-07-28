/**
 * @file IxEthDB_p.h
 *
 * @brief Private MAC learning API
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

#ifndef IxEthDB_p_H
#define IxEthDB_p_H

#include <IxTypes.h>
#include <IxOsal.h>
#include <IxEthDB.h>
#include <IxNpeMh.h>
#include <IxEthDBPortDefs.h>

#include "IxEthDBMessages_p.h"
#include "IxEthDBLog_p.h"

#if (CPU==SIMSPARCSOLARIS)

/* when running unit tests intLock() won't protect the event queue so we lock it manually */
#define TEST_FIXTURE_LOCK_EVENT_QUEUE   { ixOsalMutexLock(&eventQueueLock, IX_OSAL_WAIT_FOREVER); }
#define TEST_FIXTURE_UNLOCK_EVENT_QUEUE { ixOsalMutexUnlock(&eventQueueLock); }

#else

#define TEST_FIXTURE_LOCK_EVENT_QUEUE   /* nothing */
#define TEST_FIXTURE_UNLOCK_EVENT_QUEUE /* nothing */

#endif /* #if(CPU==SIMSPARCSOLARIS) */

#ifndef IX_UNIT_TEST

#define TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER /* nothing */
#define TEST_FIXTURE_MARK_OVERFLOW_EVENT              /* nothing */

#else

extern int dbAccessCounter;
extern int overflowEvent;

#define TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER { dbAccessCounter++; }
#define TEST_FIXTURE_MARK_OVERFLOW_EVENT              { overflowEvent = 1; }

#endif

/* code readability markers */
#define __mempool__      /* memory pool marker */
#define __lock__         /* hash write locking marker */
#define __smartpointer__ /* smart pointer marker - warning: use only clone() when duplicating! */
#define __alignment__    /* marker for data used only as alignment zones */

/* constants */
#define IX_ETH_DB_NPE_TIMEOUT (100) /* NPE response timeout, in ms */

/**
 * number of hash table buckets
 * it should be at least 8x the predicted number of entries for performance
 * each bucket needs 8 bytes
 */
#define NUM_BUCKETS (8192)

/**
 * number of hash table buckets to preload when incrementing bucket iterator
 * = two cache lines
 */
#define IX_ETHDB_CACHE_LINE_AHEAD (2)

#define IX_ETHDB_BUCKETPTR_AHEAD  ((IX_ETHDB_CACHE_LINE_AHEAD * IX_OSAL_CACHE_LINE_SIZE)/sizeof(void *))

#define IX_ETHDB_BUCKET_INDEX_MASK   (((IX_OSAL_CACHE_LINE_SIZE)/sizeof(void *)) - 1)

/* locks */
#define MAX_LOCKS (20) /**< maximum number of locks used simultaneously, do not tamper with */

/* learning tree constants */
#define INITIAL_ELT_SIZE    (8)                              /**< initial byte size of tree (empty unused root size) */
#define MAX_ELT_SIZE        (512)                            /**< maximum number of entries (includes unused root) */
#define MAX_GW_SIZE         (32)                             /**< maximum number of gateway entries (including unused root) */
#define MAX_FW_SIZE         (32)                             /**< maximum number of firewall entries (including unused root) */
#define ELT_ENTRY_SIZE      (8)                              /**< entry size, in bytes */
#define ELT_ROOT_OFFSET     (ELT_ENTRY_SIZE)                 /**< tree root offset, in bytes - node preceeding root is unused */
#define FULL_ELT_BYTE_SIZE  (MAX_ELT_SIZE * ELT_ENTRY_SIZE)  /**< full size of tree, in bytes, including unused root */
#define FULL_GW_BYTE_SIZE   (MAX_GW_SIZE * ELT_ENTRY_SIZE)   /**< full size of gateway list, in bytes, including unused root */
#define FULL_FW_BYTE_SIZE   (MAX_FW_SIZE * ELT_ENTRY_SIZE)   /**< full size of firewall table, in bytes, including unused root */

/* maximum size of the VLAN table:
 * 4096 bits (one per VLAN)
 * 8 bits in one byte
 * interleaved VLAN membership and VLAN TTI (*2) */
#define FULL_VLAN_BYTE_SIZE (4096 / 8 * 2)

/* upper 9 bits used as set index, lower 3 bits as byte index */
#define VLAN_SET_OFFSET(vlanID) ((vlanID) >> 3)
#define VLAN_SET_MASK(vlanID)   (0x7 - ((vlanID) & 0x7))

/* Update zone definitions */
#define NPE_TREE_MEM_SIZE (4096) /* ((511 entries + 1 unused root) * 8 bytes/entry) */

/* check the above value, we rely on 4k */
#if NPE_TREE_MEM_SIZE != 4096
    #error NPE_TREE_MEM_SIZE is not defined to 4096 bytes!
#endif

/* Size Filtering limits (Jumbo frame filtering) */
#define IX_ETHDB_MAX_FRAME_SIZE     65535 /* other ports than NPE ports */
#define IX_ETHDB_MIN_FRAME_SIZE         1 /* other ports than NPE ports */
#define IX_ETHDB_MAX_NPE_FRAME_SIZE 16320 /* NPE ports firmware limit */
#define IX_ETHDB_MIN_NPE_FRAME_SIZE     1 /* NPE ports firmware limit */
#define IX_ETHDB_DEFAULT_FRAME_SIZE  1522

/* memory management pool sizes  */

/*
 * Note:
 *
 * NODE_POOL_SIZE controls the maximum number of elements in the database at any one time.
 * It should be large enough to cover all the search trees of all the ports simultaneously.
 *
 * MAC_POOL_SIZE should be higher than NODE_POOL_SIZE by at least the total number of MAC addresses
 * possible to be held at any time in all the ports.
 *
 * TREE_POOL_SIZE should follow the same guideline as for MAC_POOL_SIZE.
 *
 * The database structure described here (2000/4000/4000) is enough for two NPEs holding at most 511
 * entries each plus one PCI NIC holding at most 900 entries.
 */

#define NODE_POOL_SIZE (2000) /**< number of HashNode objects - also master number of elements in the database; each entry has 16 bytes */
#define MAC_POOL_SIZE  (4000) /**< number of MacDescriptor objects; each entry has 28 bytes */
#define TREE_POOL_SIZE (4000) /**< number of MacTreeNode objects; each entry has 16 bytes */

/* retry policies */
#define BUSY_RETRY_ENABLED (true)  /**< if set to true the API will retry automatically calls returning BUSY */
#define FOREVER_RETRY      (true)  /**< if set to true the API will retry forever BUSY calls */
#define MAX_RETRIES        (400)   /**< upper retry limit - used only when FOREVER_RETRY is false */
#define BUSY_RETRY_YIELD   (5)     /**< ticks to yield for every failed retry */

/* event management */
#define EVENT_QUEUE_SIZE       (500) /**< size of the sink collecting events from the Message Handler FIFO */
#define EVENT_PROCESSING_LIMIT (100)  /**< batch processing control size (how many events are extracted from the queue at once) */

/* MAC descriptors */
#define STATIC_ENTRY  (true)
#define DYNAMIC_ENTRY (false)

/* age reset on next maintenance - incrementing by 1 will reset to 0 */
#define AGE_RESET (0xFFFFFFFF)

/* dependency maps */
#define EMPTY_DEPENDENCY_MAP (0)

/* trees */
#define RIGHT (1)
#define LEFT  (-1)

/* macros */
#define IX_ETH_DB_CHECK_PORT_EXISTS(portID) \
{ \
    if ((portID) >= IX_ETH_DB_NUMBER_OF_PORTS) \
    { \
        return IX_ETH_DB_INVALID_PORT; \
    } \
}

#define IX_ETH_DB_CHECK_PORT_INITIALIZED(portID) \
{ \
    if ((portID) >= IX_ETH_DB_NUMBER_OF_PORTS) \
    { \
        return IX_ETH_DB_INVALID_PORT; \
    } \
    else \
    { \
        if (!ixEthDBPortInfo[portID].initialized) \
        { \
            return IX_ETH_DB_PORT_UNINITIALIZED; \
        } \
    } \
}

/* single NPE check */
#define IX_ETH_DB_CHECK_SINGLE_NPE(portID) \
    if (ixEthDBSingleEthNpeCheck(portID) != IX_ETH_DB_SUCCESS) \
    { \
        WARNING_LOG("EthDB: port ID %d is unavailable\n",(UINT32) portID); \
        \
        return IX_ETH_DB_INVALID_PORT; \
    }

/* feature check */
#define IX_ETH_DB_CHECK_FEATURE(portID, feature) \
    if ((ixEthDBPortInfo[portID].featureStatus & feature) == 0) \
    { \
        return IX_ETH_DB_FEATURE_UNAVAILABLE; \
    }

/* busy retrying */
#define BUSY_RETRY(functionCall) \
    { \
        UINT32 retries = 0; \
        IxEthDBStatus br_result; \
        \
        while ((br_result = functionCall) == IX_ETH_DB_BUSY \
            && BUSY_RETRY_ENABLED && (FOREVER_RETRY || ++retries < MAX_RETRIES)) { ixOsalSleep(BUSY_RETRY_YIELD); }; \
        \
        if ((!FOREVER_RETRY && retries == MAX_RETRIES) || (br_result == IX_ETH_DB_FAIL)) \
        {\
            ERROR_LOG("Ethernet Learning Database Error: BUSY_RETRY failed at %s:%d\n", __FILE__, __LINE__); \
        }\
    }

#define BUSY_RETRY_WITH_RESULT(functionCall, brwr_result) \
    { \
        UINT32 retries = 0; \
        \
        while ((brwr_result = functionCall) == IX_ETH_DB_BUSY \
            && BUSY_RETRY_ENABLED && (FOREVER_RETRY || ++retries < MAX_RETRIES)) { ixOsalSleep(BUSY_RETRY_YIELD); }; \
        \
        if ((!FOREVER_RETRY && retries == MAX_RETRIES) || (brwr_result == IX_ETH_DB_FAIL)) \
        {\
            ERROR_LOG("Ethernet Learning Database Error: BUSY_RETRY_WITH_RESULT failed at %s:%d\n", __FILE__, __LINE__); \
        }\
    }

/* iterators */
#define IS_ITERATOR_VALID(iteratorPtr) ((iteratorPtr)->node != NULL)

/* dependency port maps */

/* Warning: if port indexing starts from 1 replace (portID) with (portID - 1) in DEPENDENCY_MAP (and make sure IX_ETH_DB_NUMBER_OF_PORTS is big enough) */

/* gives an empty dependency map */
#define SET_EMPTY_DEPENDENCY_MAP(map)      { int i = 0; for (; i < 32 ; i++) map[i] = 0; }

#define IS_EMPTY_DEPENDENCY_MAP(result, map)       { int i = 0 ; result = true; for (; i < 32 ; i++) if (map[i] != 0) { result = false; break; }}

/**
 * gives a map consisting only of 'portID'
 */
#define SET_DEPENDENCY_MAP(map, portID)    {SET_EMPTY_DEPENDENCY_MAP(map); map[portID >> 3] = 1 << (portID & 0x7);}

/**
 * gives a map resulting from joining map1 and map2
 */
#define JOIN_MAPS(map, map1, map2)         { int i = 0; for (; i < 32 ; i++) map[i] = map1[i] | map2[i]; }

/**
 * gives the map resulting from joining portID and map
 */
#define JOIN_PORT_TO_MAP(map, portID)      { map[portID >> 3] |= 1 << (portID & 0x7); }

/**
 * gives the map resulting from excluding portID from map
 */
#define EXCLUDE_PORT_FROM_MAP(map, portID) { map[portID >> 3] &= ~(1 << (portID & 0x7); }

/**
 * returns true if map1 is a subset of map2 and false otherwise
 */
#define IS_MAP_SUBSET(result, map1, map2)  { int i = 0; result = true; for (; i < 32 ; i++) if ((map1[i] | map2[i]) != map2[i]) result = false; }

/**
 * returns true is portID is part of map and false otherwise
 */
#define IS_PORT_INCLUDED(portID, map)      ((map[portID >> 3] & (1 << (portID & 0x7))) != 0)

/**
 * returns the difference between map1 and map2 (ports included in map1 and not included in map2)
 */
#define DIFF_MAPS(map, map1, map2)         { int i = 0; for (; i < 32 ; i++) map[i] = map1[i] ^ (map1[i] & map2[i]); }

/**
 * returns true if the maps collide (have at least one port in common) and false otherwise
 */
#define MAPS_COLLIDE(result, map1, map2)   { int i = 0; result = false; for (; i < 32 ; i++) if ((map1[i] & map2[i]) != 0) result = true; }

/* size (number of ports) of a dependency map */
#define GET_MAP_SIZE(map, size)            { int i = 0, b = 0; size = 0; for (; i < 32 ; i++) { char y = map[i]; for (; b < 8 && (y >>= 1); b++) size += (y & 1); }}

/* copy map2 into map1 */
#define COPY_DEPENDENCY_MAP(map1, map2)    { memcpy (map1, map2, sizeof (map1)); }

/* definition of a port map size/port number which cannot be reached (we support at most 32 ports) */
#define MAX_PORT_SIZE   (0xFF)
#define MAX_PORT_NUMBER (0xFF)

#define IX_ETH_DB_CHECK_REFERENCE(ptr)   { if ((ptr) == NULL) { return IX_ETH_DB_INVALID_ARG; } }
#define IX_ETH_DB_CHECK_MAP(portID, map) { if (!IS_PORT_INCLUDED(portID, map)) { return IX_ETH_DB_INVALID_ARG; } }

/* event queue macros */
#define EVENT_QUEUE_WRAP(offset)            ((offset) >= EVENT_QUEUE_SIZE ? (offset) - EVENT_QUEUE_SIZE : (offset))

#define CAN_ENQUEUE(eventQueuePtr)          ((eventQueuePtr)->length < EVENT_QUEUE_SIZE)

#define QUEUE_HEAD(eventQueuePtr)           (&(eventQueuePtr)->queue[EVENT_QUEUE_WRAP((eventQueuePtr)->base + (eventQueuePtr)->length)])

#define QUEUE_TAIL(eventQueuePtr)           (&(eventQueuePtr)->queue[(eventQueuePtr)->base])

#define PUSH_UPDATE_QUEUE(eventQueuePtr)    { (eventQueuePtr)->length++; }

#define SHIFT_UPDATE_QUEUE(eventQueuePtr) \
        { \
            (eventQueuePtr)->base = EVENT_QUEUE_WRAP((eventQueuePtr)->base + 1); \
            (eventQueuePtr)->length--; \
        }

#define RESET_QUEUE(eventQueuePtr) \
    { \
        (eventQueuePtr)->base   = 0; \
        (eventQueuePtr)->length = 0; \
    }

/* node stack macros - used to browse a tree without using a recursive function */
#define NODE_STACK_INIT(stack)               { (stack)->nodeCount = 0; }
#define NODE_STACK_PUSH(stack, node, offset) { (stack)->nodes[(stack)->nodeCount] = (node); (stack)->offsets[(stack)->nodeCount++] = (offset); }
#define NODE_STACK_POP(stack, node, offset)  { (node) = (stack)->nodes[--(stack)->nodeCount]; offset = (stack)->offsets[(stack)->nodeCount]; }
#define NODE_STACK_NONEMPTY(stack)           ((stack)->nodeCount != 0)

#ifndef IX_NDEBUG
#define IX_ETH_DB_NPE_MSG_HISTORY_DEPTH (100)
#define LOG_NPE_MSG(msg) \
    do { \
        UINT32 npeMsgHistoryIndex = (npeMsgHistoryLen++) % IX_ETH_DB_NPE_MSG_HISTORY_DEPTH; \
        npeMsgHistory[npeMsgHistoryIndex][0] = msg.data[0]; \
        npeMsgHistory[npeMsgHistoryIndex][1] = msg.data[1]; \
    } while (0);
#else
#define LOG_NPE_MSG() /* nothing */
#endif

/* ----------- Data -------------- */

/* typedefs */

typedef UINT32 (*HashFunction)(void *entity);
typedef BOOL (*MatchFunction)(void *reference, void *entry);
typedef void (*FreeFunction)(void *entry);

/**
 * basic component of a hash table
 */
typedef struct HashNode_t
{
    void *data;                                 /**< specific data */
    struct HashNode_t *next;                    /**< used for bucket chaining */

    __mempool__ struct HashNode_t *nextFree;    /**< memory pool management */

    __lock__ IxOsalFastMutex lock;              /**< node lock */
} HashNode;

/**
 * @brief hash table iterator definition
 *
 * an iterator is an object which can be used
 * to browse a hash table
 */
typedef struct
{
    UINT32 bucketIndex;     /**< index of the currently iterated bucket */
    HashNode *previousNode; /**< reference to the previously iterated node within the current bucket */
    HashNode *node;         /**< reference to the currently iterated node */
} HashIterator;

/**
 * definition of a MAC descriptor (a database record)
 */

typedef enum
{
    IX_ETH_DB_WIFI_AP_TO_STA = 0x0,
    IX_ETH_DB_WIFI_AP_TO_AP  = 0x1
} IxEthDBWiFiRecordType;

typedef union
{
    struct
    {
        UINT32 age;
        BOOL staticEntry; /**< true if this address is static (doesn't age) */
    } filteringData;

    struct
    {
        UINT32 age;
        BOOL staticEntry;
        UINT32 ieee802_1qTag;
      } filteringVlanData;

    struct
    {
        IxEthDBWiFiRecordType type;  /**< AP_TO_AP (0x1) or AP_TO_STA (0x0) */
        UINT32 gwAddressIndex; /**< used only when linearizing the entries for NPE usage */
        UINT8 gwMacAddress[IX_IEEE803_MAC_ADDRESS_SIZE];

        __alignment__ UINT8 reserved2[2];
    } wifiData;
} IxEthDBRecordData;

typedef struct MacDescriptor_t
{
    UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE];

    __alignment__ UINT8 reserved1[2];

    UINT32 portID;
    IxEthDBRecordType type;
    IxEthDBRecordData recordData;

    /* used for internal operations, such as NPE linearization */
    void *internal;

    /* custom user data */
    void *user;

    __mempool__ struct MacDescriptor_t *nextFree;   /**< memory pool management */
    __smartpointer__ UINT32 refCount;               /**< smart pointer reference counter */
} MacDescriptor;

/**
 * hash table definition
 */
typedef struct
{
    HashNode *hashBuckets[NUM_BUCKETS];
    UINT32 numBuckets;

    __lock__ IxOsalFastMutex bucketLocks[NUM_BUCKETS];

    HashFunction entryHashFunction;
    MatchFunction *matchFunctions;
    FreeFunction freeFunction;
} HashTable;

typedef enum
{
    IX_ETH_DB_MAC_KEY       = 1,
    IX_ETH_DB_MAC_PORT_KEY  = 2,
    IX_ETH_DB_MAC_VLAN_KEY  = 3,
    IX_ETH_DB_MAX_KEY_INDEX = 3
} IxEthDBSearchKeyType;

typedef struct MacTreeNode_t
{
    __smartpointer__  MacDescriptor *descriptor;
    struct MacTreeNode_t *left, *right;

    __mempool__ struct MacTreeNode_t *nextFree;
} MacTreeNode;

typedef IxEthDBStatus (*IxEthDBPortUpdateHandler)(IxEthDBPortId portID, IxEthDBRecordType type);

typedef void (*IxEthDBNoteWriteFn)(void *address, MacTreeNode *node);

typedef struct
{
    BOOL updateEnabled;                         /**< true if updates are enabled for port */
    BOOL userControlled;                        /**< true if the user has manually used ixEthDBPortUpdateEnableSet */
    BOOL treeInitialized;                       /**< true if the NPE has received an initial tree */
    IxEthDBPortUpdateHandler updateHandler;     /**< port update handler routine */
    void *npeUpdateZone;                        /**< port update memory zone */
    void *npeGwUpdateZone;                      /**< port update memory zone for gateways */
    void *vlanUpdateZone;                       /**< port update memory zone for VLAN tables */
    MacTreeNode *searchTree;                    /**< internal search tree, in MacTreeNode representation */
    BOOL searchTreePendingWrite;                /**< true if searchTree holds a tree pending write to the port */
} PortUpdateMethod;

typedef struct
{
    IxEthDBPortId portID;                   /**< port ID */
    BOOL enabled;                           /**< true if the port is enabled */
    BOOL agingEnabled;                      /**< true if aging on this port is enabled */
    BOOL initialized;
    IxEthDBPortMap dependencyPortMap;       /**< dependency port map for this port */
    PortUpdateMethod updateMethod;          /**< update method structure */
    BOOL macAddressUploaded;                /**< true if the MAC address was uploaded into the port */
    UINT32 maxRxFrameSize;                  /**< maximum Rx frame size for this port */
    UINT32 maxTxFrameSize;                  /**< maximum Rx frame size for this port */

    UINT8 bbsid[6];
    __alignment__ UINT8 reserved[2];
    UINT32 frameControlDurationID;          /**< Frame Control - Duration/ID WiFi control */

    IxEthDBVlanTag vlanTag;                  /**< default VLAN tag for port */
    IxEthDBPriorityTable priorityTable;     /**< QoS <=> internal priority mapping */
    IxEthDBVlanSet vlanMembership;
    IxEthDBVlanSet transmitTaggingInfo;
    IxEthDBFrameFilter frameFilter;
    IxEthDBTaggingAction taggingAction;

    UINT32 npeFrameFilter;
    UINT32 npeTaggingAction;

    IxEthDBFirewallMode firewallMode;
    BOOL srcAddressFilterEnabled;

    BOOL stpBlocked;

    IxEthDBFeature featureCapability;
    IxEthDBFeature featureStatus;

    UINT32 ixEthDBTrafficClassAQMAssignments[IX_IEEE802_1Q_QOS_PRIORITY_COUNT];

    UINT32 ixEthDBTrafficClassCount;

    UINT32 ixEthDBTrafficClassAvailable;



    __lock__ IxOsalMutex npeAckLock;
} PortInfo;

/* list of port information structures indexed on port Ids */
extern IX_ETH_DB_PUBLIC PortInfo ixEthDBPortInfo[IX_ETH_DB_NUMBER_OF_PORTS];

typedef enum
{
    IX_ETH_DB_ADD_FILTERING_RECORD    = 0xFF0001,
    IX_ETH_DB_REMOVE_FILTERING_RECORD = 0xFF0002
} PortEventType;

typedef struct
{
    UINT32 eventType;
    IxEthDBPortId portID;
    IxEthDBMacAddr macAddr;
    BOOL staticEntry;
} PortEvent;

typedef struct
{
    PortEvent queue[EVENT_QUEUE_SIZE];
    UINT32 base;
    UINT32 length;
} PortEventQueue;

typedef struct
{
    IxEthDBPortId portID; /**< originating port */
    MacDescriptor *macDescriptors[MAX_ELT_SIZE]; /**< addresses to be synced into db */
    UINT32 addressCount; /**< number of addresses */
} TreeSyncInfo;

typedef struct
{
    MacTreeNode *nodes[MAX_ELT_SIZE];
    UINT32 offsets[MAX_ELT_SIZE];
    UINT32 nodeCount;
} MacTreeNodeStack;

/* Prototypes */

/* ----------- Memory management -------------- */

IX_ETH_DB_PUBLIC void ixEthDBInitMemoryPools(void);

IX_ETH_DB_PUBLIC HashNode* ixEthDBAllocHashNode(void);
IX_ETH_DB_PUBLIC void ixEthDBFreeHashNode(HashNode *);

IX_ETH_DB_PUBLIC __smartpointer__ MacDescriptor* ixEthDBAllocMacDescriptor(void);
IX_ETH_DB_PUBLIC __smartpointer__ MacDescriptor* ixEthDBCloneMacDescriptor(MacDescriptor *macDescriptor);
IX_ETH_DB_PUBLIC __smartpointer__ void ixEthDBFreeMacDescriptor(MacDescriptor *);

IX_ETH_DB_PUBLIC __smartpointer__ MacTreeNode* ixEthDBAllocMacTreeNode(void);
IX_ETH_DB_PUBLIC __smartpointer__ MacTreeNode* ixEthDBCloneMacTreeNode(MacTreeNode *);
IX_ETH_DB_PUBLIC __smartpointer__ void ixEthDBFreeMacTreeNode(MacTreeNode *);

IX_ETH_DB_PUBLIC void ixEthDBPoolFreeMacTreeNode(MacTreeNode *);
IX_ETH_DB_PUBLIC UINT32 ixEthDBSearchTreeUsageGet(MacTreeNode *tree);
IX_ETH_DB_PUBLIC int ixEthDBShowMemoryStatus(void);

/* Hash Table */
IX_ETH_DB_PUBLIC void ixEthDBInitHash(HashTable *hashTable, UINT32 numBuckets, HashFunction entryHashFunction, MatchFunction *matchFunctions, FreeFunction freeFunction);

IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBAddHashEntry(HashTable *hashTable, void *entry);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemoveHashEntry(HashTable *hashTable, int keyType, void *reference);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBSearchHashEntry(HashTable *hashTable, int keyType, void *reference, HashNode **searchResult);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPeekHashEntry(HashTable *hashTable, int keyType, void *reference);
IX_ETH_DB_PUBLIC void ixEthDBReleaseHashNode(HashNode *node);

IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBInitHashIterator(HashTable *hashTable, HashIterator *iterator);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBIncrementHashIterator(HashTable *hashTable, HashIterator *iterator);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemoveEntryAtHashIterator(HashTable *hashTable, HashIterator *iterator);
IX_ETH_DB_PUBLIC void ixEthDBReleaseHashIterator(HashIterator *iterator);

/* API Support */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);
IX_ETH_DB_PUBLIC void ixEthDBMaximumFrameSizeAckCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);

/* DB Core functions */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBInit(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBAdd(MacDescriptor *newRecordTemplate, IxEthDBPortMap updateTrigger);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemove(MacDescriptor *templateRecord, IxEthDBPortMap updateTrigger);
IX_ETH_DB_PUBLIC HashNode* ixEthDBSearch(IxEthDBMacAddr *macAddress, IxEthDBRecordType typeFilter);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPeek(IxEthDBMacAddr *macAddress, IxEthDBRecordType typeFilter);

/* Learning support */
IX_ETH_DB_PUBLIC UINT32 ixEthDBAddressCompare(UINT8 *mac1, UINT8 *mac2);
IX_ETH_DB_PUBLIC BOOL ixEthDBAddressMatch(void *reference, void *entry);
IX_ETH_DB_PUBLIC UINT32 ixEthDBEntryXORHash(void *macDescriptor);
IX_ETH_DB_PUBLIC UINT32 ixEthDBKeyXORHash(void *macAddress);

/* Port updates */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBNPEUpdateHandler(IxEthDBPortId portID, IxEthDBRecordType type);
IX_ETH_DB_PUBLIC void ixEthDBUpdatePortLearningTrees(IxEthDBPortMap triggerPorts);
IX_ETH_DB_PUBLIC void ixEthDBNPEAccessRequest(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBUpdateLock(void);
IX_ETH_DB_PUBLIC void ixEthDBUpdateUnlock(void);
IX_ETH_DB_PUBLIC MacTreeNode* ixEthDBQuery(MacTreeNode *searchTree, IxEthDBPortMap query, IxEthDBRecordType recordFilter, UINT32 maximumEntries);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBFirewallUpdate(IxEthDBPortId portID, void *address, UINT32 epDelta);

/* Init/unload */
IX_ETH_DB_PUBLIC void ixEthDBPortSetAckCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBEventProcessorInit(void);
IX_ETH_DB_PUBLIC void ixEthDBPortInit(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBNPEUpdateAreasInit(void);
IX_ETH_DB_PUBLIC UINT32 ixEthDBMatchMethodsRegister(MatchFunction *matchFunctions);
IX_ETH_DB_PUBLIC UINT32 ixEthDBRecordSerializeMethodsRegister(void);
IX_ETH_DB_PUBLIC UINT32 ixEthDBUpdateTypeRegister(BOOL *typeArray);
IX_ETH_DB_PUBLIC void ixEthDBNPEUpdateAreasUnload(void);
IX_ETH_DB_PUBLIC void ixEthDBFeatureCapabilityScan(void);
IX_ETH_DB_PUBLIC UINT32 ixEthDBKeyTypeRegister(UINT32 *keyType);

/* Event processing */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBDefaultEventCallbackEnable(IxEthDBPortId portID, BOOL enable);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBTriggerAddPortUpdate(IxEthDBMacAddr *macAddr, IxEthDBPortId portID, BOOL staticEntry);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBTriggerRemovePortUpdate(IxEthDBMacAddr *macAddr, IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);

/* NPE adaptor */
IX_ETH_DB_PUBLIC void ixEthDBGetMacDatabaseCbk(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PUBLIC void ixEthDBNpeMsgAck(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PUBLIC void ixEthDBNPESyncScan(IxEthDBPortId portID, void *eltBaseAddress, UINT32 eltSize);
IX_ETH_DB_PUBLIC void ixEthDBNPETreeWrite(IxEthDBRecordType type, UINT32 totalSize, void *baseAddress, MacTreeNode *tree, UINT32 *blocks, UINT32 *startIndex);
IX_ETH_DB_PUBLIC void ixEthDBNPEGatewayNodeWrite(void *address, MacTreeNode *node);

/* Other public API functions */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStartLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStopLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortUpdateEnableSet(IxEthDBPortId portID, BOOL enableUpdate);

/* Maximum Tx/Rx public functions */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBFilteringPortMaximumRxFrameSizeSet(IxEthDBPortId portID, UINT32 maximumRxFrameSize);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBFilteringPortMaximumTxFrameSizeSet(IxEthDBPortId portID, UINT32 maximumTxFrameSize);

/* VLAN-related */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortVlanTableSet(IxEthDBPortId portID, IxEthDBVlanSet portVlanTable, IxEthDBVlanSet vlanSet);

/* Record search */
IX_ETH_DB_PUBLIC BOOL ixEthDBAddressRecordMatch(void *untypedReference, void *untypedEntry);
IX_ETH_DB_PUBLIC BOOL ixEthDBVlanRecordMatch(void *untypedReference, void *untypedEntry);
IX_ETH_DB_PUBLIC BOOL ixEthDBPortRecordMatch(void *untypedReference, void *untypedEntry);
IX_ETH_DB_PUBLIC BOOL ixEthDBNullMatch(void *reference, void *entry);
IX_ETH_DB_PUBLIC HashNode* ixEthDBPortSearch(IxEthDBMacAddr *macAddress, IxEthDBPortId portID, IxEthDBRecordType typeFilter);
IX_ETH_DB_PUBLIC HashNode* ixEthDBVlanSearch(IxEthDBMacAddr *macAddress, IxEthDBVlanId vlanID, IxEthDBRecordType typeFilter);

/* Utilities */
IX_ETH_DB_PUBLIC const char* mac2string(const unsigned char *mac);
IX_ETH_DB_PUBLIC void showHashInfo(void);
IX_ETH_DB_PUBLIC int ixEthDBAnalyzeHash(void);
IX_ETH_DB_PUBLIC const char* errorString(IxEthDBStatus error);
IX_ETH_DB_PUBLIC int numHashElements(void);
IX_ETH_DB_PUBLIC void zapHashtable(void);
IX_ETH_DB_PUBLIC BOOL ixEthDBCheckSingleBitValue(UINT32 value);

/* Single Eth NPE Check */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBSingleEthNpeCheck(IxEthDBPortId portId);

#endif /* IxEthDB_p_H */

