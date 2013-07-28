/** @file IxEthDB.h
 *
 * @brief this file contains the public API of @ref IxEthDB component
 *
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
 *
 */
 
#ifndef IxEthDB_H
#define IxEthDB_H

#include <IxOsBuffMgt.h>
#include <IxTypes.h>

/**
 * @defgroup IxEthDB IXP400 Ethernet Database (IxEthDB) API
 *
 * @brief ethDB is a library that does provides a MAC address database learning/filtering capability
 *
 *@{
 */

#define INLINE __inline__

#define IX_ETH_DB_PRIVATE PRIVATE /* imported from IxTypes.h */

#define IX_ETH_DB_PUBLIC PUBLIC

/**
 * @brief port ID => message handler NPE id conversion (0 => NPE_B, 1 => NPE_C)
 */
#define IX_ETH_DB_PORT_ID_TO_NPE(id) (id == 0 ? 1 : (id == 1 ? 2 : (id == 2 ? 0 : -1)))

/**
 * @def IX_ETH_DB_NPE_TO_PORT_ID(npe)
 * @brief message handler NPE id => port ID conversion (NPE_B => 0, NPE_C => 1)
 */
#define IX_ETH_DB_NPE_TO_PORT_ID(npe) (npe == 0 ? 2 : (npe == 1 ? 0 : (npe == 2 ? 1 : -1)))

/* temporary define - won't work for Azusa */
#define IX_ETH_DB_PORT_ID_TO_NPE_LOGICAL_ID(id) (IX_ETH_DB_PORT_ID_TO_NPE(id) << 4)
#define IX_ETH_DB_NPE_LOGICAL_ID_TO_PORT_ID(id) (IX_ETH_DB_NPE_TO_PORT_ID(id >> 4))

/**
 * @def IX_IEEE803_MAC_ADDRESS_SIZE
 * @brief The size of the MAC address
 */
#define IX_IEEE803_MAC_ADDRESS_SIZE (6)

/**
 * @def IX_IEEE802_1Q_QOS_PRIORITY_COUNT
 * @brief Number of QoS priorities defined by IEEE802.1Q
 */
#define IX_IEEE802_1Q_QOS_PRIORITY_COUNT (8)

/**
 * @enum IxEthDBStatus
 * @brief Ethernet Database API return values
 */
typedef enum /* IxEthDBStatus */
{
  IX_ETH_DB_SUCCESS = IX_SUCCESS,   /**< Success */
  IX_ETH_DB_FAIL = IX_FAIL,         /**< Failure */
  IX_ETH_DB_INVALID_PORT,           /**< Invalid port */
  IX_ETH_DB_PORT_UNINITIALIZED,     /**< Port not initialized */
  IX_ETH_DB_MAC_UNINITIALIZED,      /**< MAC not initialized */
  IX_ETH_DB_INVALID_ARG,            /**< Invalid argument */
  IX_ETH_DB_NO_SUCH_ADDR,           /**< Address not found for search or delete operations */
  IX_ETH_DB_NOMEM,                  /**< Learning database memory full */
  IX_ETH_DB_BUSY,                   /**< Learning database cannot complete operation, access temporarily blocked */
  IX_ETH_DB_END,                    /**< Database browser passed the end of the record set */
  IX_ETH_DB_INVALID_VLAN,           /**< Invalid VLAN ID (valid range is 0..4094, 0 signifies no VLAN membership, used for priority tagged frames) */
  IX_ETH_DB_INVALID_PRIORITY,       /**< Invalid QoS priority/traffic class (valid range for QoS priority is 0..7, valid range for traffic class depends on run-time configuration) */
  IX_ETH_DB_NO_PERMISSION,          /**< No permission for attempted operation */
  IX_ETH_DB_FEATURE_UNAVAILABLE,    /**< Feature not available (or not enabled) */
  IX_ETH_DB_INVALID_KEY,            /**< Invalid search key */
  IX_ETH_DB_INVALID_RECORD_TYPE     /**< Invalid record type */
} IxEthDBStatus;
    
/** @brief VLAN ID type, valid range is 0..4094, 0 signifying no VLAN membership */
typedef UINT32 IxEthDBVlanId;

/** @brief 802.1Q VLAN tag, contains 3 bits user priority, 1 bit CFI, 12 bits VLAN ID */
typedef UINT32 IxEthDBVlanTag;

/** @brief QoS priority/traffic class type, valid range is 0..7, 0 being the lowest */
typedef UINT32 IxEthDBPriority;

/** @brief Priority mapping table; 0..7 QoS priorities used to index, table contains traffic classes */
typedef UINT8 IxEthDBPriorityTable[8];

/** @brief A 4096 bit array used to map the complete VLAN ID range */
typedef UINT8 IxEthDBVlanSet[512];

#define IX_ETH_DB_802_1Q_VLAN_MASK (0xFFF)
#define IX_ETH_DB_802_1Q_QOS_MASK  (0x7)

#define IX_ETH_DB_802_1Q_MAX_VLAN_ID (0xFFE)

/**
 * @def IX_ETH_DB_SET_VLAN_ID
 * @brief returns the given 802.1Q tag with the VLAN ID field substituted with the given VLAN ID
 *
 * This macro is used to change the VLAN ID in a 802.1Q tag.
 *
 * Example: 
 * 
 *  tag = IX_ETH_DB_SET_VLAN_ID(tag, 32)
 *
 * inserts the VLAN ID "32" in the given tag.
 */
#define IX_ETH_DB_SET_VLAN_ID(vlanTag, vlanID) (((vlanTag) & 0xF000) | ((vlanID) & IX_ETH_DB_802_1Q_VLAN_MASK))

/**
* @def IX_ETH_DB_GET_VLAN_ID
* @brief returns the VLAN ID from the given 802.1Q tag
*/
#define IX_ETH_DB_GET_VLAN_ID(vlanTag) ((vlanTag) & IX_ETH_DB_802_1Q_VLAN_MASK)

#define IX_ETH_DB_GET_QOS_PRIORITY(vlanTag) (((vlanTag) >> 13) & IX_ETH_DB_802_1Q_QOS_MASK)

#define IX_ETH_DB_SET_QOS_PRIORITY(vlanTag, priority) (((vlanTag) & 0x1FFF) | (((priority) & IX_ETH_DB_802_1Q_QOS_MASK) << 13))

#define IX_ETH_DB_CHECK_VLAN_TAG(vlanTag) { if(((vlanTag & 0xFFFF0000) != 0) || (IX_ETH_DB_GET_VLAN_ID(vlanTag) > 4094)) return IX_ETH_DB_INVALID_VLAN; }

#define IX_ETH_DB_CHECK_VLAN_ID(vlanId) { if (vlanId > IX_ETH_DB_802_1Q_MAX_VLAN_ID) return IX_ETH_DB_INVALID_VLAN; }

#define IX_IEEE802_1Q_VLAN_TPID (0x8100)
    
typedef enum
{
  IX_ETH_DB_UNTAGGED_FRAMES        = 0x1, /**< Accepts untagged frames */
  IX_ETH_DB_VLAN_TAGGED_FRAMES     = 0x2, /**< Accepts tagged frames */
  IX_ETH_DB_PRIORITY_TAGGED_FRAMES = 0x4, /**< Accepts tagged frames with VLAN ID set to 0 (no VLAN membership) */
  IX_ETH_DB_ACCEPT_ALL_FRAMES      = 
      IX_ETH_DB_UNTAGGED_FRAMES | IX_ETH_DB_VLAN_TAGGED_FRAMES /**< Accepts all the frames */
} IxEthDBFrameFilter;

typedef enum
{
  IX_ETH_DB_PASS_THROUGH = 0x1, /**< Leave frame as-is */
  IX_ETH_DB_ADD_TAG      = 0x2, /**< Add default port VLAN tag */
  IX_ETH_DB_REMOVE_TAG   = 0x3  /**< Remove VLAN tag from frame */
} IxEthDBTaggingAction;

typedef enum
{
  IX_ETH_DB_FIREWALL_WHITE_LIST = 0x1,  /**< Firewall operates in white-list mode (MAC address based admission) */
  IX_ETH_DB_FIREWALL_BLACK_LIST = 0x2   /**< Firewall operates in black-list mode (MAC address based blocking) */
} IxEthDBFirewallMode;
  
typedef enum
{
  IX_ETH_DB_FILTERING_RECORD        = 0x01, /**< <table><caption> Filtering record </caption> 
                                             *      <tr><td> MAC address <td> static/dynamic type <td> age 
                                             *   </table> 
                                             */
  IX_ETH_DB_FILTERING_VLAN_RECORD   = 0x02, /**< <table><caption> VLAN-enabled filtering record </caption>
                                             *      <tr><td> MAC address <td> static/dynamic type <td> age <td> 802.1Q tag 
                                             *   </table> 
                                             */
  IX_ETH_DB_WIFI_RECORD             = 0x04, /**< <table><caption> WiFi header conversion record </caption>
                                             *      <tr><td> MAC address <td> optional gateway MAC address <td> 
                                             *   </table>
                                             */
  IX_ETH_DB_FIREWALL_RECORD         = 0x08, /**< <table><caption> Firewall record </caption>
                                             *      <tr><td> MAC address 
                                             *   </table>
                                             */
  IX_ETH_DB_GATEWAY_RECORD          = 0x10, /**< <i>For internal use only</i> */
  IX_ETH_DB_MAX_RECORD_TYPE_INDEX   = 0x10, /**< <i>For internal use only</i> */
  IX_ETH_DB_NO_RECORD_TYPE          = 0,    /**< None of the registered record types */
  IX_ETH_DB_ALL_FILTERING_RECORDS   = IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD, /**< All the filtering records */
  IX_ETH_DB_ALL_RECORD_TYPES        = IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD |
      IX_ETH_DB_WIFI_RECORD | IX_ETH_DB_FIREWALL_RECORD /**< All the record types registered within EthDB */    
} IxEthDBRecordType;
  
typedef enum
{
  IX_ETH_DB_LEARNING                = 0x01, /**< Learning feature; enables EthDB to learn MAC address (filtering) records, including 802.1Q enabled records */
  IX_ETH_DB_FILTERING               = 0x02, /**< Filtering feature; enables EthDB to communicate with the NPEs for downloading filtering information in the NPEs; depends on the learning feature */
  IX_ETH_DB_VLAN_QOS                = 0x04, /**< VLAN/QoS feature; enables EthDB to configure NPEs to operate in VLAN/QoS aware modes */
  IX_ETH_DB_FIREWALL                = 0x08, /**< Firewall feature; enables EthDB to configure NPEs to operate in firewall mode, using white/black address lists */
  IX_ETH_DB_SPANNING_TREE_PROTOCOL  = 0x10, /**< Spanning tree protocol feature; enables EthDB to configure the NPEs as STP nodes */
  IX_ETH_DB_WIFI_HEADER_CONVERSION  = 0x20  /**< WiFi 802.3 to 802.11 header conversion feature; enables EthDB to handle WiFi conversion data */
} IxEthDBFeature;
  
typedef UINT32 IxEthDBProperty;  /**< Property ID type */

typedef enum
{
  IX_ETH_DB_INTEGER_PROPERTY  = 0x1, /**< 4 byte unsigned integer type */
  IX_ETH_DB_STRING_PROPERTY   = 0x2, /**< NULL-terminated string type of maximum 255 characters (including the terminator) */
  IX_ETH_DB_MAC_ADDR_PROPERTY = 0x3, /**< 6 byte MAC address type */
  IX_ETH_DB_BOOL_PROPERTY     = 0x4  /**< 4 byte boolean type; can contain only true and false values */
} IxEthDBPropertyType;

/* list of supported properties for the IX_ETH_DB_VLAN_QOS feature */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_COUNT_PROPERTY   (0x01)     /**< Property identifying number the supported number of traffic classes */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY (0x10)  /**< Rx queue assigned to traffic class 0 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_1_RX_QUEUE_PROPERTY (0x11)  /**< Rx queue assigned to traffic class 1 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_2_RX_QUEUE_PROPERTY (0x12)  /**< Rx queue assigned to traffic class 2 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_3_RX_QUEUE_PROPERTY (0x13)  /**< Rx queue assigned to traffic class 3 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_4_RX_QUEUE_PROPERTY (0x14)  /**< Rx queue assigned to traffic class 4 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_5_RX_QUEUE_PROPERTY (0x15)  /**< Rx queue assigned to traffic class 5 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_6_RX_QUEUE_PROPERTY (0x16)  /**< Rx queue assigned to traffic class 6 */
#define IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY (0x17)  /**< Rx queue assigned to traffic class 7 */

/* private property used by EthAcc to indicate queue configuration complete */
#define IX_ETH_DB_QOS_QUEUE_CONFIGURATION_COMPLETE (0x18)
      
/**
 *
 * @brief The IEEE 802.3 Ethernet MAC address structure.
 * 
 * The data should be packed with bytes xx:xx:xx:xx:xx:xx 
 *
 * @note The data must be packed in network byte order.
 */
typedef struct  
{
   UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE];
} IxEthDBMacAddr;

/**
 * @ingroup IxEthDB
 *
 * @brief Definition of an IXP400 port.
 */
typedef UINT32 IxEthDBPortId;

/**
 * @ingroup IxEthDB
 *
 * @brief Port dependency map definition
 */
typedef UINT8 IxEthDBPortMap[32];
    
/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBInit(void)
 *
 * @brief Initializes the Ethernet learning/filtering database
 *
 * @note calling this function multiple times does not constitute an error;
 * redundant calls will be ignored, returning IX_ETH_DB_SUCCESS
 *
 * @retval IX_ETH_DB_SUCCESS initialization was successful
 * @retval IX_ETH_DB_FAIL initialization failed (OS error)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBInit(void);
 
/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBUnload(void)
 *
 * @brief Stops and prepares the EthDB component for unloading.
 *
 * @retval IX_ETH_DB_SUCCESS de-initialization was successful
 * @retval IX_ETH_DB_BUSY de-initialization failed, ports must be disabled first
 * @retval IX_ETH_DB_FAIL de-initialization failed (OS error)
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBUnload(void);

/**
 * @ingroup IxEthDB
 *
 * @fn void ixEthDBPortInit(IxEthDBPortId portID)
 *
 * @brief Initializes a port
 *
 * This function is called automatically by the Ethernet Access
 * ixEthAccPortInit() routine for Ethernet NPE ports and should be manually
 * called for any user-defined port (any port that is not one of
 * the two Ethernet NPEs). 
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to be initialized
 *
 * @see IxEthDBPortDefs.h for port definitions
 *
 * @note calling this function multiple times does not constitute an error;
 * redundant calls will be ignored
 */
IX_ETH_DB_PUBLIC 
void ixEthDBPortInit(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID)
 *
 * @brief Enables a port
 *
 * This function is called automatically from the Ethernet Access component
 * ixEthAccPortEnable() routine for Ethernet NPE ports and should be manually
 * called for any user-defined port (any port that is not one of
 * the Ethernet NPEs). 
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to enable processing on
 *
 * @retval IX_ETH_DB_SUCCESS if enabling is successful
 * @retval IX_ETH_DB_FAIL if the enabling was not successful due to
 * a message handler error
 * @retval IX_ETH_DB_MAC_UNINITIALIZED the MAC address of this port was
 * not initialized (only for Ethernet NPEs)
 * @retval IX_ETH_DB_INVALID_PORT if portID is invalid
 *
 * @pre ixEthDBPortAddressSet needs to be called prior to enabling the port events
 * for Ethernet NPEs
 *
 * @see ixEthDBPortAddressSet
 *
 * @see IxEthDBPortDefs.h for port definitions
 *
 * @note calling this function multiple times does not constitute an error;
 * redundant calls will be ignored
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID)
 *
 * @brief Disables processing on a port
 *
 * This function is called automatically from the Ethernet Access component
 * ixEthAccPortDisable() routine for Ethernet NPE ports and should be manually
 * called for any user-defined port (any port that is not one of
 * the Ethernet NPEs).
 *
 * @note Calling ixEthAccPortDisable() will disable the respective Ethernet NPE.
 * After Ethernet NPEs are disabled they are stopped therefore
 * when re-enabled they need to be reset, downloaded with microcode and started.
 * For learning to restart working the user needs to call again 
 * ixEthAccPortUnicastMacAddressSet or ixEthDBUnicastAddressSet
 * with the respective port MAC address.
 * Residual MAC addresses learnt before the port was disabled are deleted as soon
 * as the port is disabled. This only applies to dynamic (learnt) entries, static
 * entries do not dissapear when the port is disabled.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to disable processing on
 *
 * @retval IX_ETH_DB_SUCCESS if disabling is successful
 * @retval IX_ETH_DB_FAIL if the disabling was not successful due to
 * a message handler error
 * @retval IX_ETH_DB_INVALID_PORT if portID is invalid
 *
 * @note calling this function multiple times after the first time completed successfully
 * does not constitute an error; redundant calls will be ignored and return IX_ETH_DB_SUCCESS
*/
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Sets the port MAC address
 *
 * This function is to be called from the Ethernet Access component top-level
 * ixEthDBUnicastAddressSet(). Event processing cannot be enabled for a port
 * until its MAC address has been set.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port whose MAC address is set
 * @param macAddr @ref IxEthDBMacAddr [in] - port MAC address
 *
 * @retval IX_ETH_DB_SUCCESS MAC address was set successfully
 * @retval IX_ETH_DB_FAIL MAC address was not set due to a message handler failure
 * @retval IX_ETH_DB_INVALID_PORT if the port is not an Ethernet NPE
 *
 * @see IxEthDBPortDefs.h for port definitions
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringPortMaximumFrameSizeSet(IxEthDBPortId portID, UINT32 maximumFrameSize)
 *
 * @brief Set the maximum frame size supported on the given port ID
 *
 * This functions set the maximum frame size supported on a specific port ID 
 * 
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to configure
 * @param maximumFrameSize UINT32 [in] - maximum frame size to configure
 *
 * @retval IX_ETH_DB_SUCCESS the port is configured
 * @retval IX_ETH_DB_PORT_UNINITIALIZED the port has not been initialized
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_INVALID_ARG size parameter is out of range
 * @retval IX_ETH_DB_NO_PERMISSION selected port is not an Ethernet NPE
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note
 * This maximum frame size is used to filter the frames based on their
 * destination addresses and the capabilities of the destination port.
 * The mximum value that can be set for a NPE port is 16320.
 * (IX_ETHNPE_ACC_FRAME_LENGTH_MAX)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringPortMaximumFrameSizeSet(IxEthDBPortId portID, UINT32 maximumFrameSize);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Populate the Ethernet learning/filtering database with a static MAC address
 *
 * Populates the Ethernet learning/filtering database with a static MAC address. The entry will not be subject to aging.
 * If there is an entry (static or dynamic) with the corresponding MAC address on any port this entry will take precedence.
 * Any other entry with the same MAC address will be removed.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 * 
 * @param portID @ref IxEthDBPortId [in] - port ID to add the static address to
 * @param macAddr @ref IxEthDBMacAddr [in] - static MAC address to add
 * 
 * @retval IX_ETH_DB_SUCCESS the add was successful
 * @retval IX_ETH_DB_FAIL failed to populate the database entry
 * @retval IX_ETH_DB_BUSY failed due to a temporary busy condition (i.e. lack of CPU cycles), try again later
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Populate the Ethernet learning/filtering database with a dynamic MAC address
 *
 * Populates the Ethernet learning/filtering database with a dynamic MAC address. This entry will be subject to normal 
 * aging function, if aging is enabled on its port.
 * If there is an entry (static or dynamic) with the same MAC address on any port this entry will take precedence.
 * Any other entry with the same MAC address will be removed.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to add the dynamic address to
 * @param macAddr @ref IxEthDBMacAddr [in] - static MAC address to add
 *
 * @retval IX_ETH_DB_SUCCESS the add was successful
 * @retval IX_ETH_DB_FAIL failed to populate the database entry
 * @retval IX_ETH_DB_BUSY failed due to a temporary busy condition (i.e. lack of CPU cycles), try again later
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr)
 *
 * @brief Removes a MAC address entry from the Ethernet learning/filtering database
 *
 * @param macAddr IxEthDBMacAddr [in] - MAC address to remove
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS the removal was successful
 * @retval IX_ETH_DB_NO_SUCH_ADDR failed to remove the address (not in the database)
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_BUSY failed due to a temporary busy condition (i.e. lack of CPU cycles), try again later
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringPortSearch(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Search the Ethernet learning/filtering database for the given MAC address and port ID
 *
 * This functions searches the database for a specific port ID and MAC address. Both the port ID
 * and the MAC address have to match in order for the record to be reported as found.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to search for
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to search for
 *
 * @retval IX_ETH_DB_SUCCESS the record exists in the database
 * @retval IX_ETH_DB_INVALID_ARG invalid macAddr pointer argument
 * @retval IX_ETH_DB_NO_SUCH_ADDR the record was not found in the database
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port ID is not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringPortSearch(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Search the Ethernet learning/filtering database for a MAC address and return the port ID
 *
 * Searches the database for a MAC address. The function returns the portID for the 
 * MAC address record, if found. If no match is found the function returns IX_ETH_DB_NO_SUCH_ADDR. 
 * The portID is only valid if the function finds a match.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID the address belongs to (populated only on a successful search)
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to search for
 *
 * @retval IX_ETH_DB_SUCCESS the record exists in the database
 * @retval IX_ETH_DB_NO_SUCH_ADDR the record was not found in the database
 * @retval IX_ETH_DB_INVALID_ARG invalid macAddr or portID pointer argument(s)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Search the filtering database for a MAC address, return the port ID and reset the record age
 *
 * Searches the database for a MAC address. The function returns the portID for the 
 * MAC address record and resets the entry age to 0, if found. 
 * If no match is found the function returns IX_ETH_DB_NO_SUCH_ADDR. 
 * The portID is only valid if the function finds a match.
 *
 * - Reentrant      - yes
 * - ISR Callable   - no
 *
 * @retval IX_ETH_DB_SUCCESS the MAC address was found
 * @retval IX_ETH_DB_NO_SUCH_ADDR the MAC address was not found
 * @retval IX_ETH_DB_INVALID_ARG invalid macAddr or portID pointer argument(s)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @def IX_ETH_DB_MAINTENANCE_TIME
 *
 * @brief The @ref ixEthDBDatabaseMaintenance must be called by the user at a frequency of 
 * IX_ETH_DB_MAINTENANCE_TIME
 *
 */
#define IX_ETH_DB_MAINTENANCE_TIME (1 * 60) /* 1 Minute */

/**
 * @ingroup IxEthDB
 *
 * @def IX_ETH_DB_LEARNING_ENTRY_AGE_TIME
 *
 * @brief The define specifies the filtering database age entry time. Static entries older than
 * IX_ETH_DB_LEARNING_ENTRY_AGE_TIME +/- IX_ETH_DB_MAINTENANCE_TIME shall be removed.
 *
 */
#define IX_ETH_DB_LEARNING_ENTRY_AGE_TIME (15 * 60 ) /* 15 Mins */

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortAgingDisable(IxEthDBPortId portID)
 *
 * @brief Disable the aging function for a specific port
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to disable aging on
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS aging disabled successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port ID is not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortAgingDisable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortAgingEnable(IxEthDBPortId portID)
 *
 * @brief Enable the aging function for a specific port
 * 
 * Enables the aging of dynamic MAC address entries stored in the learning/filtering database
 * 
 * @note The aging function relies on the @ref ixEthDBDatabaseMaintenance being called with a period of 
 * @ref IX_ETH_DB_MAINTENANCE_TIME seconds.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to enable aging on
 *
 * @retval IX_ETH_DB_SUCCESS aging enabled successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port ID is not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortAgingEnable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn void ixEthDBDatabaseMaintenance(void)
 *
 * @brief Performs a maintenance operation on the Ethernet learning/filtering database
 * 
 * In order to perform a database maintenance this function must be called every
 * @ref IX_ETH_DB_MAINTENANCE_TIME seconds. It should be called regardless of whether learning is
 * enabled or not.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 * 
 * @note this function call will be ignored if the learning feature is disabled
 */
IX_ETH_DB_PUBLIC 
void ixEthDBDatabaseMaintenance(void);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringDatabaseShow(IxEthDBPortId  portID)
 *
 * @brief This function displays the Mac Ethernet MAC address filtering tables.
 *
 * It displays the MAC address, port ID, entry type (dynamic/static),and age for 
 * the given port ID.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to display the MAC address entries
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port ID is not initialized
 * @retval IX_ETH_DB_FAIL record browser failed due to an internal busy or lock condition
 *
 * @note this function is deprecated and kept for compatibility reasons; use @ref ixEthDBFilteringDatabaseShowRecords instead
 * 
 * @see ixEthDBFilteringDatabaseShowRecords
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringDatabaseShow(IxEthDBPortId portID);

/** 
 * @ingroup IxEthDB
 *
 * @fn void ixEthDBFilteringDatabaseShowAll(void)
 *
 * @brief Displays the MAC address recorded in the filtering database for all registered
 * ports (see IxEthDBPortDefs.h), grouped by port ID.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @retval void
 *
 * @note this function is deprecated and kept for compatibility reasons; use @ref ixEthDBFilteringDatabaseShowRecords instead
 * 
 * @see ixEthDBFilteringDatabaseShowRecords
 */
IX_ETH_DB_PUBLIC 
void ixEthDBFilteringDatabaseShowAll(void);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFilteringDatabaseShowRecords(IxEthDBPortId portID, IxEthDBRecordType recordFilter)
 *
 * @brief This function displays per port database records, given a record type filter
 *
 * The supported record type filters are:
 * 
 * - IX_ETH_DB_FILTERING_RECORD - displays the non-VLAN filtering records (MAC address, age, static/dynamic)
 * - IX_ETH_DB_FILTERING_VLAN_RECORD - displays the VLAN filtering records (MAC address, age, static/dynamic, VLAN ID, CFI, QoS class)
 * - IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD - displays the previous two types of records
 * - IX_ETH_DB_WIFI_RECORD - displays the WiFi header conversion records (MAC address, optional gateway MAC address) and WiFi header conversion parameters (BBSID, Duration/ID)
 * - IX_ETH_DB_FIREWALL_RECORD - displays the firewall MAC address table and firewall operating mode (white list/black list)
 * - IX_ETH_DB_ALL_RECORD_TYPES - displays all the record types
 * - IX_ETH_DB_NO_RECORD_TYPE - displays only the port status (no records are displayed)
 * 
 * Additionally, the status of each port will be displayed, containg the following information: type, capabilities, enabled status, 
 * aging enabled status, group membership and maximum frame size.
 *
 * The port ID can either be an actual port or IX_ETH_DB_ALL_PORTS, in which case the requested information
 * will be displayed for all the ports (grouped by port)
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID ID of the port to display information on (use IX_ETH_DB_ALL_PORTS for all the ports)
 * @param recordFilter record type filter
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is invalid
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port ID is not initialized
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFilteringDatabaseShowRecords(IxEthDBPortId portID, IxEthDBRecordType recordFilter);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortDependencyMapSet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap)
 *
 * @brief Sets the dependency port map for a port
 *
 * @param portID ID of the port to set the dependency map to
 * @param dependencyPortMap new dependency map (as bitmap, each bit set indicates a port being included)
 *
 * This function is used to share filtering information between ports.
 * By adding a port into another port's dependency map the target port
 * filtering data will import the filtering data from the port it depends on.
 * Any changes to filtering data for a port - such as adding, updating or removing records -
 * will trigger updates in the filtering information for all the ports depending on
 * on the updated port.
 *
 * For example, if ports 2 and 3 are set in the port 0 dependency map the filtering
 * information for port 0 will also include the filtering information from ports 2 and 3.
 * Adding a record to port 2 will also trigger an update not only on port 2 but also on
 * port 0.
 *
 * The dependency map is a 256 bit array where each bit corresponds to a port corresponding to the
 * bit offset (bit 0 - port 0, bit 1 - port 1 etc). Setting a bit to 1 indicates that the corresponding
 * port is the port map. For example, a dependency port map of 0x14 consists in the ports with IDs 2 and 4.
 * Note that the last bit (offset 255) is reserved and should never be set (it will be automatically
 * cleared by the function).
 *
 * By default, each port has a dependency port map consisting only of itself, i.e. 
 *
 * @verbatim
    IxEthDBPortMap portMap;
    
    // clear all ports from port map
    memset(portMap, 0, sizeof (portMap)); 
    
    // include portID in port map 
    portMap[portID / 8] = 1 << (portID % 8);
   @endverbatim
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @note Setting dependency maps is useful for NPE ports, which benefit from automatic updates
 * of filtering information. Setting dependency maps for user-defined ports is not an error
 * but will have no actual effect.
 * 
 * @note Including a port in its own dependency map is not compulsory, however note that
 * in this case updating the port will not trigger an update on the port itself, which 
 * might not be the intended behavior
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>dependencyPortMap</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Filtering is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDependencyMapSet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortDependencyMapGet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap)
 *
 * @brief Retrieves the dependency port map for a port
 *
 * @param portID ID of the port to set the dependency map to
 * @param dependencyPortMap location where the port dependency map is to be copied
 *
 * This function will copy the port dependency map to a user specified location.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>dependencyPortMap</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Filtering is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDependencyMapGet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanTagSet(IxEthDBPortId portID, IxEthDBVlanTag vlanTag)
 *
 * @brief Sets the default 802.1Q VLAN tag for a given port
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to set the default VLAN tag to
 * @param vlanTag @ref IxEthDBVlanTag [in] - default 802.1Q VLAN tag
 *
 * The tag format has 16 bits and it is defined in the IEEE802.1Q specification.
 * This tag will be used for tagging untagged frames (if enabled) and classifying
 * unexpedited traffic into an internal traffic class (using the user priority field).
 *
 * <table border="1"> <caption> 802.1Q tag format </caption>
 *    <tr> <td>  <b> 3 bits   <td> <b> 1 bit <td> <b> 12 bits </b>
 *    <tr> <td> user priority <td>  CFI  <td>   VID
 * </table>
 *
 * User Priority : Defines user priority, giving eight (2^3) priority levels. IEEE 802.1P defines 
 * the operation for these 3 user priority bits
 * 
 * CFI : Canonical Format Indicator is always set to zero for Ethernet switches. CFI is used for 
 * compatibility reason between Ethernet type network and Token Ring type network. If a frame received 
 * at an Ethernet port has a CFI set to 1, then that frame should not be forwarded as it is to an untagged port. 
 *
 * VID : VLAN ID is the identification of the VLAN, which is basically used by the standard 802.1Q. 
 * It has 12 bits and allow the id entification of 4096 (2^12) VLANs. Of the 4096 possible VIDs, a VID of 0 
 * is used to identify priority frames and value 4095 (FFF) is reserved, so the maximum possible VLAN 
 * configurations are 4,094.
 * 
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_ETH_DB_INVALID_VLAN <i>vlanTag</i> argument does not parse to a valid 802.1Q VLAN tag
 *
 * @note a VLAN ID value of 0 indicates that the port is not part of any VLAN
 * @note the value of the cannonical frame indicator (CFI) field is ignored, the 
 * field being used only in frame tagging operations
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanTagSet(IxEthDBPortId portID, IxEthDBVlanTag vlanTag);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanTagGet(IxEthDBPortId portID, IxEthDBVlanTag *vlanTag)
 *
 * @brief Retrieves the default 802.1Q port VLAN tag for a given port (see also @ref ixEthDBPortVlanTagSet)
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to retrieve the default VLAN tag from
 * @param vlanTag @ref IxEthDBVlanTag [out] - location to write the default port 802.1Q VLAN tag to
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid vlanTag pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanTagGet(IxEthDBPortId portID, IxEthDBVlanTag *vlanTag);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBVlanTagSet(IxEthDBMacAddr *macAddr, IxEthDBVlanTag vlanTag)
 *
 * @brief Sets the 802.1Q VLAN tag for a database record
 *
 * @param macAddr MAC address
 * @param vlanTag 802.1Q VLAN tag
 *
 * This function is used together with @ref ixEthDBVlanTagGet to provide MAC-based VLAN classification support.
 * Please note that the bridging application must contain specific code to make use of this feature (see below).
 * 
 * VLAN tags can be set only in IX_ETH_DB_FILTERING_RECORD or IX_ETH_DB_FILTERING_VLAN_RECORD type records.
 * If to an IX_ETH_DB_FILTERING_RECORD type record is added a VLAN tag the record type is automatically
 * changed to IX_ETH_DB_FILTERING_VLAN_RECORD. Once this has occurred the record type will never
 * revert to a non-VLAN type (unless deleted and re-added).
 *
 * Record types used for different purposes (such as IX_ETH_DB_WIFI_RECORD) will be ignored by
 * this function.
 *
 * After using this function to associate a VLAN ID with a MAC address the VLAN ID can be extracted knowing the
 * MAC address using @ref ixEthDBVlanTagGet. This mechanism can be used to implement MAC-based VLAN classification
 * if a bridging application searches for the VLAN tag when receiving a frame based on the source MAC address 
 * (contained in the <i>ixp_ne_src_mac</i> field of the buffer header).
 * If found in the database, the application can instruct the NPE to tag the frame by writing the VLAN tag
 * in the <i>ixp_ne_vlan_tci</i> field of the buffer header. This way the NPE will inspect the Egress tagging
 * rule associated with the given VLAN ID on the Tx port and tag the frame if Egress tagging on the VLAN is
 * allowed. Additionally, Egress tagging can be forced by setting the <i>ixp_ne_tx_flags.tag_over</i> and 
 * <i>ixp_ne_tx_flags.tag_mode</i> flags in the buffer header.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @note this function will <b>not</b> add a filtering record, it can only be used to update an existing one
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer
 * @retval IX_ETH_DB_NO_SUCH_ADDR a filtering record with the specified MAC address was not found
 * @retval IX_ETH_DB_INVALID_VLAN <i>vlanTag</i> argument does not parse to a valid 802.1Q VLAN tag
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBVlanTagSet(IxEthDBMacAddr *macAddr, IxEthDBVlanTag vlanTag);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBVlanTagGet(IxEthDBMacAddr *macAddr, IxEthDBVlanTag *vlanTag)
 *
 * @brief Retrieves the 802.1Q VLAN tag from a database record given the record MAC address
 *
 * @param macAddr MAC address
 * @param vlanTag location to write the record 802.1Q VLAN tag to
 *
 * @note VLAN tags can be retrieved only from IX_ETH_DB_FILTERING_VLAN_RECORD type records
 * 
 * This function is used together with ixEthDBVlanTagSet to provide MAC-based VLAN classification support.
 * Please note that the bridging application must contain specific code to make use of this feature (see @ref ixEthDBVlanTagSet).
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> or <i>vlanTag</i> pointer
 * @retval IX_ETH_DB_NO_SUCH_ADDR a filtering record with the specified MAC address was not found
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBVlanTagGet(IxEthDBMacAddr *macAddr, IxEthDBVlanTag *vlanTag);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipAdd(IxEthDBPortId portID, IxEthDBVlanId vlanID)
 *
 * @brief Adds a VLAN ID to a port's VLAN membership table
 *
 * Adding a VLAN ID to a port's VLAN membership table will cause frames tagged with the specified 
 * VLAN ID to be accepted by the frame filter, if Ingress VLAN membership filtering is enabled.
 * 
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to add the VLAN ID membership to
 * @param vlanID @ref IxEthDBVlanId [in] - VLAN ID to be added to the port membership table
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN vlanID is not a valid VLAN ID
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note A port's default VLAN ID is always in its own membership table, hence there
 * is no need to explicitly add it using this function (although it is not an error
 * to do so)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipAdd(IxEthDBPortId portID, IxEthDBVlanId vlanID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipRangeAdd(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax)
 *
 * @brief Adds a VLAN ID range to a port's VLAN membership table
 *
 * All the VLAN IDs in the specified range will be added to the port VLAN
 * membership table, including the range start and end VLAN IDs. Tagged frames with
 * VLAN IDs in the specified range will be accepted by the frame filter, if Ingress VLAN
 * membership filtering is enabled.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to add the VLAN membership range into
 * @param vlanIDMin @ref IxEthDBVlanId [in] - start of the VLAN ID range
 * @param vlanIDMax @ref IxEthDBVlanId [in] - end of the VLAN ID range
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN the specified VLAN IDs are invalid or do not constitute a range
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note Is is valid to use the same VLAN ID for both vlanIDMin and vlanIDMax, in which case this
 * function will behave as @ref ixEthDBPortVlanMembershipAdd
 *
 * @note A port's default VLAN ID is always in its own membership table, hence there is no need
 * to explicitly add it using this function (although it is not an error to do so)
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipRangeAdd(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipRemove(IxEthDBPortId portID, IxEthDBVlanId vlanID)
 *
 * @brief Removes a VLAN ID from a port's VLAN membership table
 *
 * Frames tagged with a VLAN ID which is not in a port's VLAN membership table
 * will be discarded by the frame filter, if Ingress membership filtering is enabled.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to remove the VLAN ID membership from
 * @param vlanID @ref IxEthDBVlanId [in] - VLAN ID to be removed from the port membership table
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_VLAN vlanID is not a valid VLAN ID
 * @retval IX_ETH_DB_NO_PERMISSION attempted to remove the default VLAN ID
 * from the port membership table (vlanID was set to the default port VLAN ID)
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note A port's default VLAN ID cannot be removed from the port's membership
 * table; attempting it will return IX_ETH_DB_NO_PERMISSION
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipRemove(IxEthDBPortId portID, IxEthDBVlanId vlanID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipRangeRemove(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax)
 *
 * @brief Removes a VLAN ID range from a port's VLAN membership table
 *
 * All the VLAN IDs in the specified range will be removed from the port VLAN
 * membership table, including the range start and end VLAN IDs. Tagged frames
 * with VLAN IDs in the range will be discarded by the frame filter, if Ingress
 * membership filtering is enabled.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to remove the VLAN membership range from
 * @param vlanIDMin @ref IxEthDBVlanId [in] - start of the VLAN ID range
 * @param vlanIDMax @ref IxEthDBVlanId [in] - end of the VLAN ID range
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN the specified VLAN IDs are invalid or do not constitute a range
 * @retval IX_ETH_DB_NO_PERMISSION attempted to remove the default VLAN ID
 * from the port membership table (both vlanIDMin and vlanIDMax were set to the default port VLAN ID)
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note Is is valid to use the same VLAN ID for both vlanIDMin and vlanIDMax, in which case
 * function will behave as @ref ixEthDBPortVlanMembershipRemove
 *
 * @note If the given range overlaps the default port VLAN ID this function
 * will remove all the VLAN IDs in the range except for the port VLAN ID from its 
 * own membership table. This situation will be silently dealt with (no error message
 * will be returned) as long as the range contains more than one value (i.e. at least 
 * one other value, apart from the default port VLAN ID). If the function is called 
 * with the vlanIDMin and vlanIDMax parameters both set to the port default VLAN ID, the 
 * function will infer that an attempt was specifically made to remove the default port 
 * VLAN ID from the port membership table, in which case the return value will be 
 * IX_ETH_DB_NO_PERMISSION.
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipRangeRemove(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipSet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet)
 *
 * @brief Sets a port's VLAN membership table
 *
 * Sets a port's VLAN membership table from a complete VLAN table containing all the possible
 * 4096 VLAN IDs. The table format is an array containing 4096 bits (512 bytes), where each bit
 * indicates whether the VLAN at that bit index is in the port's membership list (if set) or
 * not (unset).
 *
 * The bit at index 0, indicating VLAN ID 0, indicates no VLAN membership and therefore no
 * other bit must be set if bit 0 is set.
 *
 * The bit at index 4095 is reserved and should never be set (it will be ignored if set).
 *
 * The bit referencing the same VLAN ID as the default port VLAN ID should always be set, as 
 * the membership list must contain at least the default port VLAN ID.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to set the VLAN membership table to
 * @param vlanSet @ref IxEthDBVlanSet [in] - pointer to the VLAN membership table
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>vlanSet</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipSet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPortVlanMembershipGet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet)
 *
 * @brief Retrieves a port's VLAN membership table
 *
 * Retrieves the complete VLAN membership table from a port, containing all the possible
 * 4096 VLAN IDs. The table format is an array containing 4096 bits (512 bytes), where each bit
 * indicates whether the VLAN at that bit index is in the port's membership list (if set) or
 * not (unset).
 *
 * The bit at index 0, indicating VLAN ID 0, indicates no VLAN membership and therefore no
 * other bit will be set if bit 0 is set.
 *
 * The bit at index 4095 is reserved and will not be set (it will be ignored if set).
 *
 * The bit referencing the same VLAN ID as the default port VLAN ID will always be set, as 
 * the membership list must contain at least the default port VLAN ID.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to retrieve the VLAN membership table from
 * @param vlanSet @ref IxEthDBVlanSet [out] - pointer a location where the VLAN membership table will be
 *                written to 
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>vlanSet</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPortVlanMembershipGet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBAcceptableFrameTypeSet(IxEthDBPortId portID, IxEthDBFrameFilter frameFilter)
 *
 * @brief Sets a port's acceptable frame type filter
 *
 * The acceptable frame type is one (or a combination) of the following values:
 *    - IX_ETH_DB_ACCEPT_ALL_FRAMES       - accepts all the frames
 *    - IX_ETH_DB_UNTAGGED_FRAMES         - accepts untagged frames
 *    - IX_ETH_DB_VLAN_TAGGED_FRAMES      - accepts tagged frames
 *    - IX_ETH_DB_PRIORITY_TAGGED_FRAMES  - accepts tagged frames with VLAN ID set to 0 (no VLAN membership)
 *
 * Except for using the exact values given above only the following combinations are valid:
 *    - IX_ETH_DB_UNTAGGED_FRAMES | IX_ETH_DB_VLAN_TAGGED_FRAMES
 *    - IX_ETH_DB_UNTAGGED_FRAMES | IX_ETH_DB_PRIORITY_TAGGED_FRAMES
 *
 * Please note that IX_ETH_DB_UNTAGGED_FRAMES | IX_ETH_DB_VLAN_TAGGED_FRAMES is equivalent
 * to IX_ETH_DB_ACCEPT_ALL_FRAMES.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @note by default the acceptable frame type filter is set to IX_ETH_DB_ACCEPT_ALL_FRAMES
 *
 * @note setting the acceptable frame type to PRIORITY_TAGGED_FRAMES is internally
 * accomplished by changing the frame filter to VLAN_TAGGED_FRAMES and setting the
 * VLAN membership list to include only VLAN ID 0; the membership list will need
 * to be restored manually to an appropriate value if the acceptable frame type
 * filter is changed back to ACCEPT_ALL_FRAMES or VLAN_TAGGED_FRAMES; failure to do so
 * will filter all VLAN traffic bar frames tagged with VLAN ID 0
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to set the acceptable frame type filter to
 * @param frameFilter @ref IxEthDBFrameFilter [in] - acceptable frame type filter
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid frame type filter
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBAcceptableFrameTypeSet(IxEthDBPortId portID, IxEthDBFrameFilter frameFilter);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBAcceptableFrameTypeGet(IxEthDBPortId portID, IxEthDBFrameFilter *frameFilter)
 *
 * @brief Retrieves a port's acceptable frame type filter 
 *
 * For a description of the acceptable frame types see @ref ixEthDBAcceptableFrameTypeSet
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID to retrieve the acceptable frame type filter from
 * @param frameFilter @ref IxEthDBFrameFilter [out] - location to store the acceptable frame type filter
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>frameFilter</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBAcceptableFrameTypeGet(IxEthDBPortId portID, IxEthDBFrameFilter *frameFilter);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPriorityMappingTableSet(IxEthDBPortId portID, IxEthDBPriorityTable priorityTable)
 *
 * @brief Sets a port's priority mapping table
 *
 * The priority mapping table is an 8x2 table mapping a QoS (user) priority into an internal
 * traffic class. There are 8 valid QoS priorities (0..7, 0 being the lowest) which can be
 * mapped into one of the 4 available traffic classes (0..3, 0 being the lowest).
 * If a custom priority mapping table is not specified using this function the following
 * default priority table will be used (as per IEEE 802.1Q and IEEE 802.1D):
 * 
 * <table border="1"> <caption> QoS traffic classes  </caption>
 *    <tr> <td> <b> QoS priority <td> <b> Default traffic class <td> <b> Traffic type </b>
 *    <tr> <td>      0       <td>           1           <td> Best effort, default class for unexpedited traffic
 *    <tr> <td>      1       <td>           0           <td> Background traffic
 *    <tr> <td>      2       <td>           0           <td> Spare bandwidth
 *    <tr> <td>      3       <td>           1           <td> Excellent effort
 *    <tr> <td>      4       <td>           2           <td> Controlled load
 *    <tr> <td>      5       <td>           2           <td> Video traffic
 *    <tr> <td>      6       <td>           3           <td> Voice traffic
 *    <tr> <td>      7       <td>           3           <td> Network control
 * </table>
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - port ID of the port to set the priority mapping table to
 * @param priorityTable @ref IxEthDBPriorityTable [in] - location of the user priority table
 *
 * @note The provided table will be copied into internal data structures in EthDB and 
 * can be deallocated by the called after this function has completed its execution, if
 * so desired
 *
 * @warning The number of available traffic classes differs depending on the NPE images
 * and queue configuration. Check IxEthDBQoS.h for up-to-date information on the availability of
 * traffic classes. Note that specifiying a traffic class in the priority map which exceeds
 * the system availability will produce an IX_ETH_DB_INVALID_PRIORITY return error code and no
 * priority will be remapped.
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>priorityTable</i> pointer
 * @retval IX_ETH_DB_INVALID_PRIORITY at least one priority value exceeds
 * the current number of available traffic classes
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPriorityMappingTableSet(IxEthDBPortId portID, IxEthDBPriorityTable priorityTable);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPriorityMappingTableGet(IxEthDBPortId portID, IxEthDBPriorityTable priorityTable)
 *
 * @brief Retrieves a port's priority mapping table
 *
 * The priority mapping table for the given port will be copied in the location
 * specified by the caller using "priorityTable"
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID ID @ref IxEthDBPortId [in] - of the port to retrieve the priority mapping table from
 * @param priorityTable @ref IxEthDBPriorityTable [out] - pointer to a user specified location where the table will be copied to
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid priorityTable pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPriorityMappingTableGet(IxEthDBPortId portID, IxEthDBPriorityTable priorityTable);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPriorityMappingClassSet(IxEthDBPortId portID, IxEthDBPriority userPriority, IxEthDBPriority trafficClass)
 *
 * @brief Sets one QoS/user priority => traffic class mapping in a port's priority mapping table
 *
 * This function establishes a mapping between a user (QoS) priority and an internal traffic class.
 * The mapping will be saved in the port's priority mapping table. Use this function when not all
 * the QoS priorities need remapping (see also @ref ixEthDBPriorityMappingTableSet)
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to set the mapping to
 * @param userPriority @ref IxEthDBPriority [in] - user (QoS) priority, between 0 and 7 (0 being the lowest)
 * @param trafficClass @ref IxEthDBPriority [in] - internal traffic class, between 0 and 3 (0 being the lowest)
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_PRIORITY <i>userPriority</i> out of range or
 * <i>trafficClass</i> is beyond the number of currently available traffic classes
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPriorityMappingClassSet(IxEthDBPortId portID, IxEthDBPriority userPriority, IxEthDBPriority trafficClass);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBPriorityMappingClassGet(IxEthDBPortId portID, IxEthDBPriority userPriority, IxEthDBPriority *trafficClass)
 *
 * @brief Retrieves one QoS/user priority => traffic class mapping in a port's priority mapping table
 *
 * This function retrieves the internal traffic class associated with a QoS (user) priority from a given
 * port's priority mapping table. Use this function when not all the QoS priority mappings are 
 * required (see also @ref ixEthDBPriorityMappingTableGet)
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to set the mapping to
 * @param userPriority @ref IxEthDBPriority [in] - user (QoS) priority, between 0 and 7 (0 being the lowest)
 * @param trafficClass @ref IxEthDBPriority [out] - location to write the corresponding internal traffic class to
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_PRIORITY invalid userPriority value (out of range)
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>trafficClass</i> pointer argument
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBPriorityMappingClassGet(IxEthDBPortId portID, IxEthDBPriority userPriority, IxEthDBPriority *trafficClass);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBEgressVlanEntryTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanId vlanID, BOOL enabled)
 *
 * @brief Enables or disables Egress VLAN tagging for a port and a given VLAN
 *
 * This function enables or disables Egress VLAN tagging for the given port and VLAN ID.
 * If the VLAN tagging for a certain VLAN ID is enabled then all the frames to be
 * transmitted on the given port tagged with the same VLAN ID will be transmitted in a tagged format.
 * If tagging is not enabled for the given VLAN ID, the VLAN tag from the frames matching
 * this VLAN ID will be removed (the frames will be untagged).
 *
 * VLAN ID 4095 is reserved and should never be used with this function.
 * VLAN ID 0 has the special meaning of "No VLAN membership" and it is used in this
 * context to allow the port to send priority-tagged frames or not.
 *
 * By default, no Egress VLAN tagging is enabled on any port.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to enable or disable the VLAN ID Egress tagging on
 * @param vlanID @ref IxEthDBVlanId [in] - VLAN ID to be matched against outgoing frames
 * @param enabled BOOL [in] - true to enable Egress VLAN tagging on the port and given VLAN, and
 *                false to disable Egress VLAN tagging
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN invalid VLAN ID (out of range)
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBEgressVlanEntryTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanId vlanID, BOOL enabled);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBEgressVlanEntryTaggingEnabledGet(IxEthDBPortId portID, IxEthDBVlanId vlanID, BOOL *enabled)
 *
 * @brief Retrieves the Egress VLAN tagging enabling status for a port and VLAN ID
 *
 * @param portID [in] - ID of the port to extract the Egress VLAN ID tagging status from
 * @param vlanID VLAN [in] - ID whose tagging status is to be extracted
 * @param enabled [in] - user-specifed location where the status is copied to; following
 * the successfull execution of this function the value will be true if Egress VLAN
 * tagging is enabled for the given port and VLAN ID, and false otherwise
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @see ixEthDBEgressVlanEntryTaggingEnabledGet
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN invalid VLAN ID (out of range)
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>enabled</i> argument pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBEgressVlanEntryTaggingEnabledGet(IxEthDBPortId portID, IxEthDBVlanId vlanID, BOOL *enabled);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBEgressVlanRangeTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax, BOOL enabled)
 *
 * @brief Enables or disables Egress VLAN tagging for a port and given VLAN range
 *
 * This function is very similar to @ref ixEthDBEgressVlanEntryTaggingEnabledSet with the
 * difference that it can manipulate the Egress tagging status on multiple VLAN IDs,
 * defined by a contiguous range. Note that both limits in the range are explicitly 
 * included in the execution of this function.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to enable or disable the VLAN ID Egress tagging on
 * @param vlanIDMin @ref IxEthDBVlanId [in] - start of the VLAN range to be matched against outgoing frames
 * @param vlanIDMax @ref IxEthDBVlanId [in] - end of the VLAN range to be matched against outgoing frames
 * @param enabled BOOL [in] - true to enable Egress VLAN tagging on the port and given VLAN range,
 *                and false to disable Egress VLAN tagging
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_VLAN invalid VLAN ID (out of range), or do not constitute a range
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_ETH_DB_NO_PERMISSION attempted to explicitly remove the default port VLAN ID from the tagging table
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @note Specifically removing the default port VLAN ID from the Egress tagging table by setting both vlanIDMin and vlanIDMax
 * to the VLAN ID portion of the PVID is not allowed by this function and will return IX_ETH_DB_NO_PERMISSION.
 * However, this can be circumvented, should the user specifically desire this, by either using a 
 * larger range (vlanIDMin < vlanIDMax) or by using ixEthDBEgressVlanEntryTaggingEnabledSet.
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBEgressVlanRangeTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanId vlanIDMin, IxEthDBVlanId vlanIDMax, BOOL enabled);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBEgressVlanTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet)
 *
 * @brief Sets the complete Egress VLAN tagging table for a port
 *
 * This function is used to set the VLAN tagging/untagging per VLAN ID for a given port
 * covering the entire VLAN ID range (0..4094). The <i>vlanSet</i> parameter is a 4096
 * bit array, each bit indicating the Egress behavior for the corresponding VLAN ID.
 * If a bit is set then outgoing frames with the corresponding VLAN ID will be transmitted
 * with the VLAN tag, otherwise the frame will be transmitted without the VLAN tag.
 *
 * Bit 0 has a special significance, indicating tagging or tag removal for priority-tagged
 * frames.
 *
 * Bit 4095 is reserved and should never be set (it will be ignored if set).
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port whose Egress VLAN tagging behavior is set
 * @param vlanSet @ref IxEthDBVlanSet [in] - 4096 bit array controlling per-VLAN tagging and untagging 
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>vlanSet</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 *
 * @warning This function will automatically add the default port VLAN ID to the Egress tagging table
 * every time it is called. The user should manually call ixEthDBEgressVlanEntryTaggingEnabledSet to
 * prevent tagging on the default port VLAN ID if the default behavior is not intended.
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBEgressVlanTaggingEnabledSet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBEgressVlanTaggingEnabledGet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet)
 *
 * @brief Retrieves the complete Egress VLAN tagging table from a port
 *
 * This function copies the 4096 bit table controlling the Egress VLAN tagging into a user specified
 * area. Each bit in the array indicates whether tagging for the corresponding VLAN (the bit position
 * in the array) is enabled (the bit is set) or not (the bit is unset). 
 *
 * Bit 4095 is reserved and should not be set (it will be ignored if set).
 *
 * @see ixEthDBEgressVlanTaggingEnabledSet
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port whose Egress VLAN tagging behavior is retrieved
 * @param vlanSet @ref IxEthDBVlanSet [out] - user location to copy the Egress tagging table into; should have
 * room to store 4096 bits (512 bytes)
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>vlanSet</i> pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBEgressVlanTaggingEnabledGet(IxEthDBPortId portID, IxEthDBVlanSet vlanSet);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBIngressVlanTaggingEnabledSet(IxEthDBPortId portID, IxEthDBTaggingAction taggingAction)
 *
 * @brief Sets the Ingress VLAN tagging behavior for a port
 *
 * A port's Ingress tagging behavior is controlled by the taggingAction parameter,
 * which can take one of the following values:
 * 
 * - IX_ETH_DB_PASS_THROUGH - leaves the frame unchanged (does not add or remove the VLAN tag)
 * - IX_ETH_DB_ADD_TAG - adds the VLAN tag if not present, using the default port VID
 * - IX_ETH_DB_REMOVE_TAG - removes the VLAN tag if present
 * 
 * @param portID @ref IxEthDBPortId [in] - ID of the port whose Ingress VLAN tagging behavior is set
 * @param taggingAction @ref IxEthDBTaggingAction [in] - tagging behavior for the port
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>taggingAction</i> argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBIngressVlanTaggingEnabledSet(IxEthDBPortId portID, IxEthDBTaggingAction taggingAction);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBIngressVlanTaggingEnabledGet(IxEthDBPortId portID, IxEthDBTaggingAction *taggingAction)
 *
 * @brief Retrieves the Ingress VLAN tagging behavior from a port (see @ref ixEthDBIngressVlanTaggingEnabledSet)
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port whose Ingress VLAN tagging behavior is set
 * @param taggingAction @ref IxEthDBTaggingAction [out] - location where the tagging behavior for the port is written to
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>taggingAction</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBIngressVlanTaggingEnabledGet(IxEthDBPortId portID, IxEthDBTaggingAction *taggingAction);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBVlanPortExtractionEnable(IxEthDBPortId portID, BOOL enable)
 *
 * @brief Enables or disables port ID extraction
 *
 * This feature can be used in the situation when a multi-port device (e.g. a switch) 
 * is connected to an IXP4xx port and the device can provide incoming frame port 
 * identification by tagging the TPID field in the Ethernet frame. Enabling
 * port extraction will instruct the NPE to copy the TPID field from the frame and 
 * place it in the <i>ixp_ne_src_port</i> of the <i>ixp_buf</i> header. In addition,
 * the NPE restores the TPID field to 0.
 *
 * If the frame is not tagged the NPE will fill the <i>ixp_ne_src_port</i> with the 
 * port ID of the MII interface the frame was received from.
 *
 * The TPID field is the least significant byte of the type/length field, which is 
 * normally set to 0x8100 for 802.1Q-tagged frames.
 *
 * This feature is disabled by default.
 *
 * @param portID ID of the port to configure port ID extraction on
 * @param enable true to enable port ID extraction and false to disable it
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE VLAN/QoS feature is not available or not enabled for the port
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBVlanPortExtractionEnable(IxEthDBPortId portID, BOOL enable);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFeatureCapabilityGet(IxEthDBPortId portID, IxEthDBFeature *featureSet)
 *
 * @brief Retrieves the feature capability set for a port
 *
 * This function retrieves the feature capability set for a port or the common capabilities shared between all 
 * the ports, writing the feature capability set in a user specified location.
 *
 * The feature capability set will consist of a set formed by OR-ing one or more of the following values:
 * - IX_ETH_DB_LEARNING - Learning feature; enables EthDB to learn MAC address (filtering) records, including 802.1Q enabled records
 * - IX_ETH_DB_FILTERING - Filtering feature; enables EthDB to communicate with the NPEs for downloading filtering information in the NPEs; depends on the learning feature
 * - IX_ETH_DB_VLAN_QOS - VLAN/QoS feature; enables EthDB to configure NPEs to operate in VLAN/QoS aware modes
 * - IX_ETH_DB_FIREWALL - Firewall feature; enables EthDB to configure NPEs to operate in firewall mode, using white/black address lists
 * - IX_ETH_DB_SPANNING_TREE_PROTOCOL - Spanning tree protocol feature; enables EthDB to configure the NPEs as STP nodes
 * - IX_ETH_DB_WIFI_HEADER_CONVERSION - WiFi 802.3 to 802.11 header conversion feature; enables EthDB to handle WiFi conversion data
 *
 * Note that EthDB provides only the LEARNING feature for non-NPE ports.
 * 
 * @param portID @ref IxEthDBPortId [in] - ID of the port to retrieve the capability set for 
 * (use IX_ETH_DB_ALL_PORTS to retrieve the common capabilities shared between all the ports)
 * @param featureSet @ref IxEthDBFeature [out] - location where the capability set will be written to
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>featureSet</i> pointer
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFeatureCapabilityGet(IxEthDBPortId portID, IxEthDBFeature *featureSet);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFeatureEnable(IxEthDBPortId portID, IxEthDBFeature feature, BOOL enabled)
 *
 * @brief Enables or disables one or more EthDB features
 *
 * Selects one or more features (see @ref ixEthDBFeatureCapabilityGet for a description of the supported
 * features) to be enabled or disabled on the selected port (or all the ports).
 *
 * Note that some features are mutually incompatible:
 * - IX_ETH_DB_FILTERING is incompatible with IX_ETH_DB_WIFI_HEADER_CONVERSION
 *
 * Also note that some features require other features to be enabled: 
 * - IX_ETH_DB_FILTERING requires IX_ETH_DB_LEARNING
 *
 * This function will either enable the entire selected feature set for the selected port (or all the ports),
 * in which case it will return IX_ETH_DB_SUCCESS, or in case of error it will not enable any feature at all
 * and return an appropriate error message.
 *
 * The following features are enabled by default (for ports with the respective capability), 
 * for compatibility reasons with previous versions of CSR:
 * - IX_ETH_DB_LEARNING
 * - IX_ETH_DB_FILTERING
 *
 * All other features are disabled by default and require manual enabling using ixEthDBFeatureEnable.
 *
 * <b>Default settings for VLAN, QoS, Firewall and WiFi header conversion features:</b>
 *
 * <i>VLAN</i>
 *
 * When the VLAN/QoS feature is enabled for a port for the first time the default VLAN behavior 
 * of the port is set to be as <b>permissive</b> (it will accept all the frames) and 
 * <b>non-interferential</b> (it will not change any frames) as possible:
 * - the port VLAN ID (VID) is set to 0
 * - the Ingress acceptable frame filter is set to accept all frames
 * - the VLAN port membership is set to the complete VLAN range (0 - 4094)
 * - the Ingress tagging mode is set to pass-through (will not change frames)
 * - the Egress tagging mode is to send tagged frames in the entire VLAN range (0 - 4094)
 *
 * Note that further disabling and re-enabling the VLAN feature for a given port will not reset the port VLAN behavior
 * to the settings listed above. Any VLAN settings made by the user are kept.
 *
 * <i>QoS</i>
 *
 * The following default priority mapping table will be used (as per IEEE 802.1Q and IEEE 802.1D):
 * 
 * <table border="1"> <caption> QoS traffic classes  </caption>
 *    <tr> <td> <b> QoS priority <td> <b> Default traffic class <td> <b> Traffic type </b>
 *    <tr> <td>      0       <td>           1           <td> Best effort, default class for unexpedited traffic
 *    <tr> <td>      1       <td>           0           <td> Background traffic
 *    <tr> <td>      2       <td>           0           <td> Spare bandwidth
 *    <tr> <td>      3       <td>           1           <td> Excellent effort
 *    <tr> <td>      4       <td>           2           <td> Controlled load
 *    <tr> <td>      5       <td>           2           <td> Video traffic
 *    <tr> <td>      6       <td>           3           <td> Voice traffic
 *    <tr> <td>      7       <td>           3           <td> Network control
 * </table>
 *
 * <i> Firewall </i>
 *  
 * The port firewall is configured by default in <b>black-list mode</b>, and the firewall address table is empty.
 * This means the firewall will not filter any frames until the feature is configured and the firewall table is
 * downloaded to the NPE.
 *
 * <i> Spanning Tree </i>
 *
 * The port is set to <b>STP unblocked mode</b>, therefore it will accept all frames until re-configured.
 *
 * <i> WiFi header conversion </i>
 *
 * The WiFi header conversion database is empty, therefore no actual header conversion will take place until this
 * feature is configured and the conversion table downloaded to the NPE.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port to enable or disable the features on (use IX_ETH_DB_ALL_PORTS for all the ports)
 * @param feature @ref IxEthDBFeature [in] - feature or feature set to enable or disable
 * @param enabled BOOL [in] - true to enable the feature and false to disable it
 * 
 * @note Certain features, from a functional point of view, cannot be disabled as such at NPE level;
 * when such features are set to <i>disabled</i> using the EthDB API they will be configured in such
 * a way to determine a behavior equivalent to the feature being disabled. As well as this, disabled
 * features cannot be configured or accessed via the EthDB API (except for getting their status).
 *
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_NO_PERMISSION attempted to enable mutually exclusive features, 
 * or a feature that depends on another feature which is not present or enabled
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE at least one of the features selected is unavailable
 * @retval IX_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFeatureEnable(IxEthDBPortId portID, IxEthDBFeature feature, BOOL enabled);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFeatureStatusGet(IxEthDBPortId portID, IxEthDBFeature feature, BOOL *present, BOOL *enabled)
 *
 * @brief Retrieves the availability and status of a feature set
 *
 * This function returns the availability and status for a feature set.
 * Note that if more than one feature is selected (e.g. IX_ETH_DB_LEARNING | IX_ETH_DB_FILTERING)
 * the "present" and "enabled" return values will be set to true only if all the features in the
 * feature set are present and enabled (not only some). 
 * 
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param feature @ref IxEthDBFeature [in] - identifier of the feature to retrieve the status for
 * @param present BOOL [out] - location where a boolean flag indicating whether this feature is present will be written to
 * @param enabled BOOL [out] - location where a boolean flag indicating whether this feature is enabled will be written to
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG either <i>present</i> or <i>enabled</i> pointer argument is invalid
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFeatureStatusGet(IxEthDBPortId portID, IxEthDBFeature feature, BOOL *present, BOOL *enabled);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFeaturePropertyGet(IxEthDBPortId portID, IxEthDBFeature feature, IxEthDBProperty property, IxEthDBPropertyType *type, void *value)
 *
 * @brief Retrieves the value of a feature property
 *
 * The EthDB features usually contain feature-specific properties describing or
 * controlling how the feature operates. While essential properties (e.g. the
 * firewall operating mode) have their own API, secondary properties can be 
 * retrieved using this function.
 *
 * Properties can be read-only or read-write. ixEthDBFeaturePropertyGet operates with
 * both types of features.
 *
 * Properties have types associated with them. A descriptor indicating the property
 * type is returned in the <i>type</i> argument for convenience.
 *
 * The currently supported properties and their corresponding features are as follows:
 *
 * <table border="1"> <caption> Properties for IX_ETH_DB_VLAN_QOS </caption>
 *    <tr> <td> <b>        Property identifier                  <td> <b> Property type          <td> <b> Property value                   <td> <b> Read-Only </b>
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_COUNT_PROPERTY      <td> IX_ETH_DB_INTEGER_PROPERTY <td> number of internal traffic classes   <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 0 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_1_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 1 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_2_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 2 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_3_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 3 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_4_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 4 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_5_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 5 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_6_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 6 <td> Yes
 *    <tr> <td> IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY <td> IX_ETH_DB_INTEGER_PROPERTY <td> queue assignment for traffic class 7 <td> Yes
 * </table>
 * 
 * @see ixEthDBFeaturePropertySet
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param feature @ref IxEthDBFeature [in] - EthDB feature for which the property is retrieved
 * @param property @ref IxEthDBProperty [in] - property identifier
 * @param type @ref IxEthDBPropertyType [out] - location where the property type will be stored
 * @param value void [out] - location where the property value will be stored
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG invalid property identifier, <i>type</i> or <i>value</i> pointer arguments
 * @retval IX_ETH_DB_FAIL incorrect property value or unknown error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFeaturePropertyGet(IxEthDBPortId portID, IxEthDBFeature feature, IxEthDBProperty property, IxEthDBPropertyType *type, void *value);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFeaturePropertySet(IxEthDBPortId portID, IxEthDBFeature feature, IxEthDBProperty property, void *value)
 *
 * @brief Sets the value of a feature property
 *
 * Unlike @ref ixEthDBFeaturePropertyGet, this function operates only with read-write properties
 *
 * The currently supported properties and their corresponding features are as follows:
 *
 *   - IX_ETH_DB_QOS_QUEUE_CONFIGURATION_COMPLETE (for IX_ETH_DB_VLAN_QOS): freezes the availability of traffic classes
 *     to the number of traffic classes currently in use
 *
 * Note that this function creates deep copies of the property values; once the function is invoked the client 
 * can free or reuse the memory area containing the original property value.
 *
 * Copy behavior for different property types is defined as follows:
 *
 *   - IX_ETH_DB_INTEGER_PROPERTY   - 4 bytes are copied from the source location
 *   - IX_ETH_DB_STRING_PROPERTY    - the source string will be copied up to the NULL '\0' string terminator, maximum of 255 characters
 *   - IX_ETH_DB_MAC_ADDR_PROPERTY  - 6 bytes are copied from the source location
 *   - IX_ETH_DB_BOOL_PROPERTY      - 4 bytes are copied from the source location; the only allowed values are true (1L) and false (0L)
 *
 * @see ixEthDBFeaturePropertySet
 *
 * @warning IX_ETH_DB_QOS_QUEUE_CONFIGURATION_COMPLETE is provided for EthAcc internal use; 
 * do not attempt to set this property directly
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param feature @ref IxEthDBFeature [in] - EthDB feature for which the property is set
 * @param property @ref IxEthDBProperty [in] - property identifier
 * @param value void [in] - location where the property value is to be copied from
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG invalid property identifier, <i>value</i> pointer, or invalid property value
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFeaturePropertySet(IxEthDBPortId portID, IxEthDBFeature feature, IxEthDBProperty property, void *value);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBDatabaseClear(IxEthDBPortId portID, IxEthDBRecordType recordType)
 *
 * @brief Deletes a set of record types from the Ethernet Database
 *
 * This function deletes all the records of certain types (specified in the recordType filter)
 * associated with a port. Additionally, the IX_ETH_DB_ALL_PORTS value can be used as port ID
 * to indicate that the specified record types should be deleted for all the ports.
 *
 * The record type filter can be an ORed combination of the following types:
 *
 * <caption> Record types </caption>
 *    - IX_ETH_DB_FILTERING_RECORD      <table><caption> Filtering record </caption>
 *                                               <tr><td> MAC address <td> static/dynamic type <td> age </tr>
 *                                             </table> 
 *
 *    - IX_ETH_DB_FILTERING_VLAN_RECORD <table><caption> VLAN-enabled filtering record </caption>
 *                                               <tr><td> MAC address <td> static/dynamic type <td> age <td> 802.1Q tag </tr>
 *                                             </table> 
 *
 *    - IX_ETH_DB_WIFI_RECORD           <table><caption> WiFi header conversion record </caption>
 *                                                <tr><td> MAC address <td> optional gateway MAC address <td> </tr>
 *                                             </table>
 *
 *    - IX_ETH_DB_FIREWALL_RECORD       <table><caption> Firewall record </caption>
 *                                                <tr><td> MAC address </tr>
 *                                             </table>
 *    - IX_ETH_DB_ALL_RECORD_TYPES
 * 
 * Any combination of the above types is valid e.g. 
 *
 *    (IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD | IX_ETH_DB_FIREWALL_RECORD), 
 *
 * although some might be redundant (it is not an error to do so) e.g.
 *
 *    (IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_ALL_RECORD_TYPES)
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port
 * @param recordType @ref IxEthDBRecordType [in] - record type filter
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>recordType</i> filter
 *
 * @note If the record type filter contains any unrecognized value (hence the 
 * IX_ETH_DB_INVALID_ARG error value is returned) no actual records will be deleted.
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBDatabaseClear(IxEthDBPortId portID, IxEthDBRecordType recordType);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiStationEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Adds an "Access Point to Station" record to the database, for 802.3 => 802.11 frame 
 * header conversion
 *
 * Frame header conversion is controlled by the set of MAC addresses
 * added using @ref ixEthDBWiFiStationEntryAdd and @ref ixEthDBWiFiAccessPointEntryAdd.
 * Conversion arguments are added using @ref ixEthDBWiFiFrameControlSet, 
 * @ref ixEthDBWiFiDurationIDSet and @ref ixEthDBWiFiBBSIDSet.
 *
 * Note that adding the same MAC address twice will not return an error
 * (but will not accomplish anything either), while re-adding a record previously added
 * as an "Access Point to Access Point" will migrate the record to the "Access Point
 * to Station" type.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to add
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG macAddr is an invalid pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled 
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_NOMEM maximum number of records reached
 * @retval IX_ETH_DB_BUSY lock condition or transaction in progress, try again later
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiStationEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiAccessPointEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBMacAddr *gatewayMacAddr)
 *
 * @brief Adds an "Access Point to Access Point" record to the database
 *
 * @see ixEthDBWiFiStationEntryAdd
 *
 * Note that adding the same MAC address twice will simply overwrite the previously
 * defined gateway MAC address value in the same record, if the record was previously of the
 * "Access Point to Access Point" type.
 *
 * Re-adding a MAC address as "Access Point to Access Point", which was previously added as 
 * "Access Point to Station" will migrate the record type to "Access Point to Access Point" and
 * record the gateway MAC address.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to add
 * @param gatewayMacAddr @ref IxEthDBMacAddr [in] - MAC address of the gateway Access Point
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG macAddr is an invalid pointer
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled 
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> or <i>gatewayMacAddr</i> pointer argument
 * @retval IX_ETH_DB_NOMEM maximum number of records reached
 * @retval IX_ETH_DB_BUSY lock condition or transaction in progress, try again later
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiAccessPointEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBMacAddr *gatewayMacAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Removes a WiFi station record
 *
 * This function removes both types of WiFi records ("Access Point to Station" and
 * "Access Point to Access Point").
 * 
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to remove
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port is not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_NO_SUCH_ADDR specified address was not found in the database
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled 
 * @retval IX_ETH_DB_BUSY lock condition or transaction in progress, try again later
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiConversionTableDownload(IxEthDBPortId portID)
 *
 * @brief Downloads the MAC address table for 802.3 => 802.11 frame header
 * conversion to the NPE
 *
 * Note that the frame conversion MAC address table must be individually downloaded
 * to each NPE for which the frame header conversion feature is enabled (i.e. it
 * is not possible to specify IX_ETH_DB_ALL_PORTS).
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiConversionTableDownload(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiFrameControlSet(IxEthDBPortId portID, UINT16 frameControl)
 *
 * @brief Sets the GlobalFrameControl field
 *
 * The GlobalFrameControl field is a 2-byte value inserted in the <i>Frame Control</i>
 * field for all 802.3 to 802.11 frame header conversions
 * 
 * @param portID @ref IxEthDBPortId [in] - ID of the port
 * @param frameControl UINT16 [in] - GlobalFrameControl value
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiFrameControlSet(IxEthDBPortId portID, UINT16 frameControl);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiDurationIDSet(IxEthDBPortId portID, UINT16 durationID)
 *
 * @brief Sets the GlobalDurationID field
 *
 * The GlobalDurationID field is a 2-byte value inserted in the <i>Duration/ID</i>
 * field for all 802.3 to 802.11 frame header conversions
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param durationID UINT16 [in] - GlobalDurationID field
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiDurationIDSet(IxEthDBPortId portID, UINT16 durationID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBWiFiBBSIDSet(IxEthDBPortId portID, IxEthDBMacAddr *bbsid)
 *
 * @brief Sets the BBSID field
 *
 * The BBSID field is a 6-byte value which
 * identifies the infrastructure of the service set managed 
 * by the Access Point having the IXP400 as its processor. The value
 * is written in the <i>BBSID</i> field of the 802.11 frame header.
 * The BBSID value is the MAC address of the Access Point.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param bbsid @ref IxEthDBMacAddr [in] - pointer to 6 bytes containing the BSSID
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>bbsid</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE WiFi feature not enabled
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBWiFiBBSIDSet(IxEthDBPortId portID, IxEthDBMacAddr *bbsid);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBSpanningTreeBlockingStateSet(IxEthDBPortId portID, BOOL blocked)
 *
 * @brief Sets the STP blocked/unblocked state for a port
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param blocked BOOL [in] - true to set the port as STP blocked, false to set it as unblocked
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Spanning Tree Protocol feature not enabled
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateSet(IxEthDBPortId portID, BOOL blocked);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBSpanningTreeBlockingStateGet(IxEthDBPortId portID, BOOL *blocked)
 *
 * @brief Retrieves the STP blocked/unblocked state for a port
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param blocked BOOL * [in] - set to true if the port is STP blocked, false otherwise
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>blocked</i> pointer argument
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Spanning Tree Protocol feature not enabled
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateGet(IxEthDBPortId portID, BOOL *blocked);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFirewallModeSet(IxEthDBPortId portID, IxEthDBFirewallMode mode)
 *
 * @brief Sets the firewall mode to use white or black listing
 *
 * When enabled, the NPE MAC address based firewall support operates in two modes:
 *
 * - white-list mode (MAC address based admission) 
 *    - <i>mode</i> set to IX_ETH_DB_FIREWALL_WHITE_LIST
 *    - only packets originating from MAC addresses contained in the firewall address list
 *      are allowed on the Rx path
 * - black-list mode (MAC address based blocking)  
 *    - <i>mode</i> set to IX_ETH_DB_FIREWALL_BLACK_LIST
 *    - packets originating from MAC addresses contained in the firewall address list
 *      are discarded
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param mode @ref IxEthDBFirewallMode [in] - firewall mode (IX_ETH_DB_FIREWALL_WHITE_LIST or IX_ETH_DB_FIREWALL_BLACK_LIST)
 *
 * @note by default the firewall operates in black-list mode with an empty address
 * list, hence it doesn't filter any packets
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Firewall feature not enabled 
 * @retval IX_ETH_DB_INVALID_ARGUMENT <i>mode</i> argument is not a valid firewall configuration mode
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
*/
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallModeSet(IxEthDBPortId portID, IxEthDBFirewallMode mode);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFirewallInvalidAddressFilterEnable(IxEthDBPortId portID, BOOL enable)
 *
 * @brief Enables or disables invalid MAC address filtering
 *
 * According to IEEE802 it is illegal for a source address to be a multicast
 * or broadcast address. If this feature is enabled the NPE inspects the source
 * MAC addresses of incoming frames and discards them if invalid addresses are
 * detected.
 *
 * By default this service is enabled, if the firewall feature is supported by the
 * NPE image.
 *
 * @param portID ID of the port 
 * @param enable true to enable invalid MAC address filtering and false to disable it
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Firewall feature not enabled 
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallInvalidAddressFilterEnable(IxEthDBPortId portID, BOOL enable);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFirewallEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Adds a MAC address to the firewall address list
 *
 * Note that adding the same MAC address twice will not return an error
 * but will not actually accomplish anything.
 *
 * The firewall MAC address list has a limited number of entries; once
 * the maximum number of entries has been reached this function will failed
 * to add more addresses, returning IX_ETH_DB_NOMEM.
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to be added
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_NOMEM maximum number of records reached
 * @retval IX_ETH_DB_BUSY lock condition or transaction in progress, try again later
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Firewall feature not enabled 
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFirewallEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Removes a MAC address from the firewall address list
 *
 * @param portID @ref IxEthDBPortId [in] - ID of the port 
 * @param macAddr @ref IxEthDBMacAddr [in] - MAC address to be removed
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_NO_SUCH_ADDR address not found
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Firewall feature not enabled 
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBFirewallTableDownload(IxEthDBPortId portID)
 *
 * @brief Downloads the MAC firewall table to a port
 *
 * @param portID ID of the port 
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID is not a valid port identifier
 * @retval IX_ETH_DB_PORT_UNINITIALIZED port not initialized
 * @retval IX_ETH_DB_FEATURE_UNAVAILABLE Firewall feature not enabled 
 * @retval IX_ETH_DB_FAIL unknown OS or NPE communication error
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallTableDownload(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBUserFieldSet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, IxEthDBVlanId vlanID, void *field)
 *
 * @brief Adds a user-defined field to a database record
 *
 * This function associates a user-defined field to a database record.
 * The user-defined field is passed as a <i>(void *)</i> parameter, hence it can be used
 * for any purpose (such as identifying a structure). Retrieving the user-defined field from
 * a record is done using @ref ixEthDBUserFieldGet. Note that EthDB never uses the user-defined
 * field for any internal operation and it is not aware of the significance of its contents. The
 * field is only stored as a pointer.
 *
 * The database record is identified using a combination of the given parameters, depending on the record type.
 * All the record types require the record MAC address.
 *
 * - IX_ETH_DB_FILTERING_RECORD requires only the MAC address
 * - IX_ETH_DB_VLAN_FILTERING_RECORD requires the MAC address and the VLAN ID
 * - IX_ETH_DB_WIFI_RECORD requires the MAC address and the portID
 * - IX_ETH_DB_FIREWALL_RECORD requires the MAC address and the portID
 *
 * Please note that if a parameter is not required it is completely ignored (it does not undergo parameter checking).
 * The user-defined field can be cleared using a <b>NULL</b> <i>field</i> parameter.
 *
 * @param recordType @ref IxEthDBRecordType [in] - type of record (can be IX_ETH_DB_FILTERING_RECORD, 
 * IX_ETH_DB_FILTERING_VLAN_RECORD, IX_ETH_DB_WIFI_RECORD or IX_ETH_DB_FIREWALL_RECORD)
 * @param portID @ref IxEthDBPortId [in] - ID of the port (required only for WIFI and FIREWALL records) 
 * @param macAddr @ref IxEthDBMacAddr * [in] - MAC address of the record
 * @param vlanID @ref IxEthDBVlanId [in] - VLAN ID of the record (required only for FILTERING_VLAN records)
 * @param field void * [in] - user defined field
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID was required but it is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> pointer argument
 * @retval IX_ETH_DB_NO_SUCH_ADDR record not found
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBUserFieldSet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, IxEthDBVlanId vlanID, void *field);

/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBUserFieldGet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, IxEthDBVlanId vlanID, void **field)
 *
 * @brief Retrieves a user-defined field from a database record
 *
 * The database record is identified using a combination of the given parameters, depending on the record type.
 * All the record types require the record MAC address.
 *
 * - IX_ETH_DB_FILTERING_RECORD requires only the MAC address
 * - IX_ETH_DB_VLAN_FILTERING_RECORD requires the MAC address and the VLAN ID
 * - IX_ETH_DB_WIFI_RECORD requires the MAC address and the portID
 * - IX_ETH_DB_FIREWALL_RECORD requires the MAC address and the portID
 *
 * Please note that if a parameter is not required it is completely ignored (it does not undergo parameter checking).
 *
 * If no user-defined field was registered with the specified record then <b>NULL</b> will be written
 * at the location specified by <i>field</i>.
 *
 * @param recordType type of record (can be IX_ETH_DB_FILTERING_RECORD, IX_ETH_DB_FILTERING_VLAN_RECORD, IX_ETH_DB_WIFI_RECORD 
 * or IX_ETH_DB_FIREWALL_RECORD)
 * @param portID ID of the port (required only for WIFI and FIREWALL records) 
 * @param macAddr MAC address of the record
 * @param vlanID VLAN ID of the record (required only for FILTERING_VLAN records)
 * @param field location to write the user defined field into
 * 
 * @retval IX_ETH_DB_SUCCESS operation completed successfully
 * @retval IX_ETH_DB_INVALID_PORT portID was required but it is not a valid port identifier
 * @retval IX_ETH_DB_INVALID_ARG invalid <i>macAddr</i> or <i>field</i> pointer arguments
 * @retval IX_ETH_DB_NO_SUCH_ADDR record not found
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBUserFieldGet(IxEthDBRecordType recordType, IxEthDBMacAddr *macAddr, IxEthDBPortId portId, IxEthDBVlanId vlanID, void **field);

/**
 * @}
 */

#endif /* IxEthDB_H */
