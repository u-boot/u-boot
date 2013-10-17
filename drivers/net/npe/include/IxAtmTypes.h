/**
 * @file IxAtmTypes.h
 *
 * @date 24-MAR-2002
 *
 * @brief This file contains Atm types common to a number of Atm components.
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

/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */
/**
 * @defgroup IxAtmTypes IXP400 ATM Types (IxAtmTypes)
 *
 * @brief The common set of types used in many Atm components
 *
 * @{ */

#ifndef IXATMTYPES_H
#define IXATMTYPES_H

#include "IxNpeA.h"

/**
 * @enum IxAtmLogicalPort
 *
 * @brief Logical Port Definitions  :
 *
 * Only 1 port is available in SPHY configuration
 * 12 ports are enabled in MPHY configuration
 *
 */
typedef enum
{
    IX_UTOPIA_PORT_0 = 0,  /**< Port 0 */
#ifdef IX_NPE_MPHYMULTIPORT
    IX_UTOPIA_PORT_1,      /**< Port 1 */
    IX_UTOPIA_PORT_2,      /**< Port 2 */
    IX_UTOPIA_PORT_3,      /**< Port 3 */
    IX_UTOPIA_PORT_4,      /**< Port 4 */
    IX_UTOPIA_PORT_5,      /**< Port 5 */
    IX_UTOPIA_PORT_6,      /**< Port 6 */
    IX_UTOPIA_PORT_7,      /**< Port 7 */
    IX_UTOPIA_PORT_8,      /**< Port 8 */
    IX_UTOPIA_PORT_9,      /**< Port 9 */
    IX_UTOPIA_PORT_10,     /**< Port 10 */
    IX_UTOPIA_PORT_11,     /**< Port 11 */
#endif /* IX_NPE_MPHY */
    IX_UTOPIA_MAX_PORTS    /**< Not a port - just a definition for the
                           * maximum possible ports
                           */
} IxAtmLogicalPort;

/**
 * @def IX_ATM_CELL_PAYLOAD_SIZE
 * @brief Size of a ATM cell payload
 */
#define IX_ATM_CELL_PAYLOAD_SIZE             (48)

/**
 * @def IX_ATM_CELL_SIZE
 * @brief Size of a ATM cell, including header
 */
#define IX_ATM_CELL_SIZE                     (53)

/**
 * @def IX_ATM_CELL_SIZE_NO_HEC
 * @brief Size of a ATM cell, excluding HEC byte
 */
#define IX_ATM_CELL_SIZE_NO_HEC              (IX_ATM_CELL_SIZE - 1)

/**
 * @def IX_ATM_OAM_CELL_SIZE_NO_HEC
 * @brief Size of a OAM cell, excluding HEC byte
 */
#define IX_ATM_OAM_CELL_SIZE_NO_HEC          IX_ATM_CELL_SIZE_NO_HEC

/**
 * @def IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE
 * @brief Size of a AAL0 48 Cell payload
 */
#define IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE     IX_ATM_CELL_PAYLOAD_SIZE

/**
 * @def IX_ATM_AAL5_CELL_PAYLOAD_SIZE
 * @brief Size of a AAL5 Cell payload
 */
#define IX_ATM_AAL5_CELL_PAYLOAD_SIZE        IX_ATM_CELL_PAYLOAD_SIZE

/**
 * @def IX_ATM_AAL0_52_CELL_SIZE_NO_HEC
 * @brief Size of a AAL0 52 Cell, excluding HEC byte
 */
#define IX_ATM_AAL0_52_CELL_SIZE_NO_HEC      IX_ATM_CELL_SIZE_NO_HEC


/**
 * @def IX_ATM_MAX_VPI
 * @brief Maximum value of an ATM VPI
 */
#define IX_ATM_MAX_VPI 255

/**
 * @def IX_ATM_MAX_VCI
 * @brief Maximum value of an ATM VCI
 */
#define IX_ATM_MAX_VCI 65535

 /**
 * @def IX_ATM_MAX_NUM_AAL_VCS
 * @brief Maximum number of active AAL5/AAL0 VCs in the system
 */
#define IX_ATM_MAX_NUM_AAL_VCS 32

/**
 * @def IX_ATM_MAX_NUM_VC
 * @brief Maximum number of active AAL5/AAL0 VCs in the system
 * The use of this macro is depreciated, it is retained for
 * backward compatiblity. For current software release
 * and beyond the define IX_ATM_MAX_NUM_AAL_VC should be used.
 */
#define IX_ATM_MAX_NUM_VC IX_ATM_MAX_NUM_AAL_VCS



/**
 * @def IX_ATM_MAX_NUM_OAM_TX_VCS
 * @brief Maximum number of active OAM Tx VCs in the system, 
 *        1 OAM VC per port
 */
#define IX_ATM_MAX_NUM_OAM_TX_VCS IX_UTOPIA_MAX_PORTS

/**
 * @def IX_ATM_MAX_NUM_OAM_RX_VCS
 * @brief Maximum number of active OAM Rx VCs in the system, 
 *        1 OAM VC shared accross all ports
 */
#define IX_ATM_MAX_NUM_OAM_RX_VCS 1

/**
 * @def IX_ATM_MAX_NUM_AAL_OAM_TX_VCS
 * @brief Maximum number of active AAL5/AAL0/OAM Tx VCs in the system
 */
#define IX_ATM_MAX_NUM_AAL_OAM_TX_VCS (IX_ATM_MAX_NUM_AAL_VCS + IX_ATM_MAX_NUM_OAM_TX_VCS)

/**
 * @def IX_ATM_MAX_NUM_AAL_OAM_RX_VCS
 * @brief Maximum number of active AAL5/AAL0/OAM Rx VCs in the system
 */
#define IX_ATM_MAX_NUM_AAL_OAM_RX_VCS (IX_ATM_MAX_NUM_AAL_VCS + IX_ATM_MAX_NUM_OAM_RX_VCS)

/**
 *  @def IX_ATM_IDLE_CELLS_CONNID
 *  @brief VC Id used to indicate idle cells in the returned schedule table.
 */
#define IX_ATM_IDLE_CELLS_CONNID 0


/**
 *  @def IX_ATM_CELL_HEADER_VCI_GET
 *  @brief get the VCI field from a cell header
 */
#define IX_ATM_CELL_HEADER_VCI_GET(cellHeader) \
    (((cellHeader) >> 4) & IX_OAM_VCI_BITS_MASK);

/**
 *  @def IX_ATM_CELL_HEADER_VPI_GET
 *  @brief get the VPI field from a cell header
 */
#define IX_ATM_CELL_HEADER_VPI_GET(cellHeader) \
    (((cellHeader) >> 20) & IX_OAM_VPI_BITS_MASK);

/**
 *  @def IX_ATM_CELL_HEADER_PTI_GET
 *  @brief get the PTI field from a cell header
 */
#define IX_ATM_CELL_HEADER_PTI_GET(cellHeader) \
    ((cellHeader) >> 1) & IX_OAM_PTI_BITS_MASK;

/**
 * @typedef IxAtmCellHeader
 *
 * @brief ATM Cell Header, does not contain 4 byte HEC, added by NPE-A 
 */
typedef unsigned int IxAtmCellHeader;


/**
 * @enum IxAtmServiceCategory
 *
 * @brief Enumerated type representing available ATM service categories.
 *   For more informatoin on these categories, see "Traffic Management
 *   Specification" v4.1, published by the ATM Forum -
 *   http://www.atmforum.com
 */
typedef enum
{
    IX_ATM_CBR,    /**< Constant Bit Rate */
    IX_ATM_RTVBR,  /**< Real Time Variable Bit Rate */
    IX_ATM_VBR,    /**< Variable Bit Rate */
    IX_ATM_UBR,    /**< Unspecified Bit Rate */
    IX_ATM_ABR     /**< Available Bit Rate (not supported) */

} IxAtmServiceCategory;

/**
 *
 * @enum IxAtmRxQueueId
 *
 * @brief Rx Queue Type for RX traffic
 *
 * IxAtmRxQueueId defines the queues involved for receiving data.
 *
 * There are two queues to facilitate prioritisation handling
 * and processing the 2 queues with different algorithms and
 * constraints
 *
 * e.g. : one queue can carry voice (or time-critical traffic), the
 * other queue can carry non-voice traffic
 *
 */
typedef enum
{
    IX_ATM_RX_A = 0,      /**< RX queue A */
    IX_ATM_RX_B,          /**< RX queue B */
    IX_ATM_MAX_RX_STREAMS /**< Maximum number of RX streams */
} IxAtmRxQueueId;

/**
 * @brief Structure describing an ATM traffic contract for a Virtual
 *         Connection (VC).
 *
 * Structure is used to specify the requested traffic contract for a
 * VC to the IxAtmSch component using the @ref ixAtmSchVcModelSetup
 * interface.
 *
 * These parameters are defined by the ATM forum working group
 * (http://www.atmforum.com).
 *
 * @note Typical values for a voice channel 64 Kbit/s
 * - atmService @a IX_ATM_RTVBR
 * - pcr   400  (include IP overhead, and AAL5 trailer)
 * - cdvt  5000000 (5 ms)
 * - scr = pcr
 *
 * @note Typical values for a data channel 800 Kbit/s
 * - atmService @a IX_ATM_UBR
 * - pcr   1962  (include IP overhead, and AAL5 trailer)
 * - cdvt  5000000 (5 ms)
 *
 */
typedef struct
{
    IxAtmServiceCategory atmService; /**< ATM service category */
    unsigned pcr;   /**< Peak Cell Rate - cells per second */
    unsigned cdvt;  /**< Cell Delay Variation Tolerance - in nanoseconds */
    unsigned scr;   /**< Sustained Cell Rate - cells per second */
    unsigned mbs;   /**< Max Burst Size - cells */
    unsigned mcr;   /**< Minimum Cell Rate - cells per second */
    unsigned mfs;   /**< Max Frame Size - cells */
} IxAtmTrafficDescriptor;

/**
 * @typedef IxAtmConnId
 *
 * @brief ATM VC data connection identifier.
 *
 * This is is generated by IxAtmdAcc when a successful connection is
 * made on a VC. The is the ID by which IxAtmdAcc knows an active
 * VC and should be used in IxAtmdAcc API calls to reference a
 * specific VC.
 */
typedef unsigned int IxAtmConnId;

/**
 * @typedef IxAtmSchedulerVcId
 *
 * @brief ATM VC scheduling connection identifier.
 *
 * This id is generated and used by ATM Tx controller, generally
 * the traffic shaper (e.g. IxAtmSch). The IxAtmdAcc component
 * will request one of these Ids whenever a data connection on
 * a Tx VC is requested. This ID will be used in callbacks to
 * the ATM Transmission Ctrl s/w (e.g. IxAtmm) to reference a
 * particular VC.
 */
typedef int IxAtmSchedulerVcId;

/**
 * @typedef IxAtmNpeRxVcId
 *
 * @brief ATM Rx VC identifier used by the ATM Npe.
 *
 * This Id is generated by IxAtmdAcc when a successful data connection
 * is made on a rx VC.
 */
typedef unsigned int IxAtmNpeRxVcId;

/**
 * @brief ATM Schedule Table entry
 *
 * This IxAtmScheduleTableEntry is used by an ATM scheduler to inform
 * IxAtmdAcc about the data to transmit (in term of cells per VC)
 *
 * This structure defines
 * @li the number of cells to be transmitted (numberOfCells)
 * @li the VC connection to be used for transmission (connId).
 *
 * @note - When the connection Id value is IX_ATM_IDLE_CELLS_CONNID, the
 * corresponding number of idle cells will be transmitted to the hardware.
 *
 */
typedef struct
{
    IxAtmConnId connId; /**< connection Id
                 *
                 * Identifier of VC from which cells are to be transmitted.
                 * When this valus is IX_ATM_IDLE_CELLS_CONNID, this indicates
                 * that the system should transmit the specified number
                 * of idle cells. Unknown connIds result in the transmission
                 * idle cells.
                 */
    unsigned int numberOfCells; /**< number of cells to transmit
                 *
                 * The number of contiguous cells to schedule from this VC
                 * at this point. The valid range is from 1 to
                 * @a IX_ATM_SCHEDULETABLE_MAXCELLS_PER_ENTRY. This
                 * number can swap over mbufs and pdus. OverSchduling results
                 * in the transmission of idle cells.
                 */
} IxAtmScheduleTableEntry;

/**
 * @brief This structure defines a schedule table which gives details
 *         on which data (from which VCs) should be transmitted for a
 *         forthcoming period of time for a particular port and the
 *         order in which that data should be transmitted.
 *
 *  The schedule table consists of a series of entries each of which
 *  will schedule one or more cells from a particular registered VC.
 *  The total number of cells scheduled and the total number of
 *  entries in the table are also indicated.
 *
 */
typedef struct
{
    unsigned tableSize;      /**< Number of entries
                              *
                              * Indicates the total number of
                              *   entries in the table.
                              */
    unsigned totalCellSlots; /**< Number of cells
                              *
                              * Indicates the total number of ATM
                              *   cells which are scheduled by all the
                              *   entries in the table.
                              */
    IxAtmScheduleTableEntry *table; /**< Pointer to schedule entries
                                     *
                                     * Pointer to an array
                                     *   containing tableSize entries
                                     */
} IxAtmScheduleTable;

#endif /* IXATMTYPES_H */

/**
 * @} defgroup IxAtmTypes
 */


