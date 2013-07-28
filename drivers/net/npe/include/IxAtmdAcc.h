
/**
 * @file    IxAtmdAcc.h
 *
 * @date    07-Nov-2001
 *
 * @brief IxAtmdAcc Public API
 *
 * This file contains the public API of IxAtmdAcc, related to the
 * data functions of the component
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
 */

/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */

/**
 *
 * @defgroup IxAtmdAccAPI IXP400 ATM Driver Access (IxAtmdAcc) API
 *
 * @brief The public API for the IXP400 Atm Driver Data component
 *
 * IxAtmdAcc is the low level interface by which AAL0/AAL5 and
 * OAM data gets transmitted to,and received from the Utopia bus.
 *
 * For AAL0/AAL5 services transmit and receive connections may
 * be established independantly for unique combinations of
 * port,VPI,and VCI.
 *
 * Two AAL0 services supporting 48 or 52 byte cell data are provided.
 * Submitted AAL0 PDUs must be a multiple of the cell data size (48/52).
 * AAL0_52 is a raw cell service the client must format
 * the PDU with an ATM cell header (excluding HEC) at the start of
 * each cell, note that AtmdAcc does not validate the cell headers in
 * a submitted PDU.
 *
 * OAM cells cannot be received over the AAL0 service but instead
 * are received over a dedicated OAM service.
 *
 * For the OAM service an "OAM Tx channel" may be enabled for a port
 * by establishing a single dedicated OAM Tx connection on that port.
 * A single "OAM Rx channel" for all ports may be  enabled by
 * establishing a dedicated OAM Rx connection.
 *
 * The OAM service allows buffers containing 52 byte OAM F4/F5 cells
 * to be transmitted and received over the dedicated OAM channels.
 * HEC is appended/removed, and CRC-10 performed by the NPE. The OAM
 * service offered by AtmdAcc is a raw cell transport service.
 * It is assumed that ITU I.610 procedures that make use of this
 * service are implemented above AtmdAcc.
 *
 * Note that the dedicated OAM connections are established on
 * reserved VPI,VCI, and (in the case of Rx) port values defined below.
 * These values are used purely to descriminate the dedicated OAM channels
 * and do not identify a particular OAM F4/F5 flow. F4/F5 flows may be
 * realised for particluar VPI/VCIs by manipulating the VPI,VCI
 * fields of the ATM cell headers of cells in the buffers passed
 * to AtmdAcc. Note that AtmdAcc does not validate the cell headers
 * in a submitted OAM PDU.
 *
 *
 *
 * This part is related to the User datapath processing
 *
 * @{
 */

#ifndef IXATMDACC_H
#define IXATMDACC_H

#include "IxAtmTypes.h"

/* ------------------------------------------------------
   AtmdAcc Data Types definition
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_WARNING
 *
 * @brief Warning return code
 *
 * This constant is used to tell IxAtmDAcc user about a special case.
 *
 */
#define IX_ATMDACC_WARNING 2

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_BUSY
 *
 * @brief Busy return code
 *
 * This constant is used to tell IxAtmDAcc user that the request
 * is correct, but cannot be processed because the IxAtmAcc resources
 * are already used. The user has to retry its request later
 *
 */
#define IX_ATMDACC_BUSY 3

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_RESOURCES_STILL_ALLOCATED
 *
 * @brief Disconnect return code
 *
 * This constant is used to tell IxAtmDAcc user that the disconnect
 * functions are not complete because the resources used by the driver
 * are not yet released. The user has to retry the disconnect call
 * later.
 *
 */
#define IX_ATMDACC_RESOURCES_STILL_ALLOCATED 4

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_DEFAULT_REPLENISH_COUNT
 *
 * @brief Default resources usage for RxVcFree replenish mechanism
 *
 * This constant is used to tell IxAtmDAcc to allocate and use
 * the minimum of resources for rx free replenish.
 *
 * @sa ixAtmdAccRxVcConnect
 */
#define IX_ATMDACC_DEFAULT_REPLENISH_COUNT 0


/**
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_OAM_TX_VPI
 *
 * @brief The reserved value used for the dedicated OAM
 * Tx connection. This "well known" value is used by atmdAcc and
 * its clients to dsicriminate the OAM channel, and should be chosen so
 * that it does not coencide with the VPI value used in an AAL0/AAL5 connection.
 * Any attempt to connect a service type other than OAM on this VPI will fail.
 *
 *
 */
#define IX_ATMDACC_OAM_TX_VPI 0

/**
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_OAM_TX_VCI
 *
 * @brief The reserved value used for the dedicated OAM
 * Tx connection. This "well known" value is used by atmdAcc and
 * its clients to dsicriminate the OAM channel, and should be chosen so
 * that it does not coencide with the VCI value used in an AAL0/AAL5 connection.
 * Any attempt to connect a service type other than OAM on this VCI will fail.
 */
#define IX_ATMDACC_OAM_TX_VCI 0


 /**
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_OAM_RX_PORT
 *
 * @brief The reserved dummy PORT used for all dedicated OAM
 * Rx connections. Note that this is not a real port but must
 * have a value that lies within the valid range of port values.
 */
#define IX_ATMDACC_OAM_RX_PORT IX_UTOPIA_PORT_0

 /**
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_OAM_RX_VPI
 *
 * @brief The reserved value value used for the dedicated OAM
 * Rx connection. This value should be chosen so that it does not
 * coencide with the VPI value used in an AAL0/AAL5 connection.
 * Any attempt to connect a service type other than OAM on this VPI will fail.
 */
#define IX_ATMDACC_OAM_RX_VPI 0

/**
 * @ingroup IxAtmdAccAPI
 *
 * @def IX_ATMDACC_OAM_RX_VCI
 *
 * @brief The reserved value value used for the dedicated OAM
 * Rx connection. This value should be chosen so that it does not
 * coencide with the VCI value used in an AAL0/AAL5 connection.
 * Any attempt to connect a service type other than OAM on this VCI will fail.
 */
#define IX_ATMDACC_OAM_RX_VCI 0


/**
 * @enum IxAtmdAccPduStatus
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief IxAtmdAcc Pdu status :
 *
 * IxAtmdAccPduStatus is used during a RX operation to indicate
 * the status of the received PDU
 *
 */

typedef enum
{
    IX_ATMDACC_AAL0_VALID = 0,    /**< aal0 pdu */
    IX_ATMDACC_OAM_VALID,         /**< OAM pdu */
    IX_ATMDACC_AAL2_VALID,        /**< aal2 pdu @b reserved for future use */
    IX_ATMDACC_AAL5_VALID,        /**< aal5 pdu complete and trailer is valid */
    IX_ATMDACC_AAL5_PARTIAL,      /**< aal5 pdu not complete, trailer is missing */
    IX_ATMDACC_AAL5_CRC_ERROR,    /**< aal5 pdu not complete, crc error/length error */
    IX_ATMDACC_MBUF_RETURN        /**< empty buffer returned to the user */
} IxAtmdAccPduStatus;


/**
 *
 * @enum IxAtmdAccAalType
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief IxAtmdAcc AAL Service Type :
 *
 * IxAtmdAccAalType defines the type of traffic to run on this VC
 *
 */
typedef enum
{
    IX_ATMDACC_AAL5,                /**< ITU-T AAL5 */
    IX_ATMDACC_AAL2,                /**< ITU-T AAL2 @b reserved for future use */
    IX_ATMDACC_AAL0_48,             /**< AAL0 48 byte payloads (cell header is added by NPE)*/
    IX_ATMDACC_AAL0_52,             /**< AAL0 52 byte cell data (HEC is added by NPE) */
    IX_ATMDACC_OAM,                 /**< OAM cell transport service (HEC is added by NPE)*/
    IX_ATMDACC_MAX_SERVICE_TYPE     /**< not a service, used for parameter validation */
} IxAtmdAccAalType;

/**
 *
 * @enum IxAtmdAccClpStatus
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief IxAtmdAcc CLP indication
 *
 * IxAtmdAccClpStatus defines the CLP status of the current PDU
 *
 */
typedef enum
{
    IX_ATMDACC_CLP_NOT_SET = 0,     /**< CLP indication is not set */
    IX_ATMDACC_CLP_SET = 1     /**< CLP indication is set */
} IxAtmdAccClpStatus;

/**
 * @typedef IxAtmdAccUserId
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief User-supplied Id
 *
 * IxAtmdAccUserId is passed through callbacks and allows the
 * IxAtmdAcc user to identify the source of a call back. The range of
 * this user-owned Id is [0...2^32-1)].
 *
 * The user provides this own Ids on a per-channel basis as a parameter
 * in a call to @a ixAtmdAccRxVcConnect() or @a ixAtmdAccRxVcConnect()
 *
 * @sa ixAtmdAccRxVcConnect
 * @sa ixAtmdAccTxVcConnect
 *
 */
typedef unsigned int IxAtmdAccUserId;

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to RX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief  Rx callback prototype
 *
 * IxAtmdAccRxVcRxCallback is the prototype of the Rx callback user
 * function called once per PDU to pass a receive Pdu to a user on a
 * partilcular connection. The callback is likely to push the mbufs
 * to a protocol layer, and recycle the mbufs for a further use.
 *
 * @note -This function is called ONLY in the context of
 *        the @a ixAtmdAccRxDispatch() function
 *
 * @sa ixAtmdAccRxDispatch
 * @sa ixAtmdAccRxVcConnect
 *
 * @param port @ref IxAtmLogicalPort [in] - the port on which this PDU was received
 *        a logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param userId @ref IxAtmdAccUserId [in] - user Id provided in the call
 *        to @a ixAtmdAccRxVcConnect()
 * @param status @ref IxAtmdAccPduStatus [in] - an indication about the PDU validity.
 *        In the case of AAL0 the only possibile value is
 *        AAL0_VALID, in this case the client may optionally determine
 *        that an rx timeout occured by checking if the mbuf is
 *        compleletly or only partially filled, the later case
 *        indicating a timeout.
 *        In the case of OAM the only possible value is OAM valid.
 *        The status is set to @a IX_ATMDACC_MBUF_RETURN when
 *        the mbuf is released during a disconnect process.
 * @param clp @ref IxAtmdAccClpStatus [in] - clp indication for this PDU.
 *        For AAL5/AAL0_48 this information
 *        is set if the clp bit of any rx cell is set
 *        For AAL0-52/OAM the client may inspect the CLP in individual
 *        cell headers in the PDU, and this parameter is set to 0.
 * @param *mbufPtr @ref IX_OSAL_MBUF [in] - depending on the servive type a pointer to
 *        an mbuf (AAL5/AAL0/OAM) or mbuf chain (AAL5 only),
 *        that comprises the complete PDU data.
 *
 *        This parameter is guaranteed not to be a null pointer.
 *
 */
typedef void (*IxAtmdAccRxVcRxCallback) (IxAtmLogicalPort port,
                       IxAtmdAccUserId userId,
                       IxAtmdAccPduStatus status,
                       IxAtmdAccClpStatus clp,
                       IX_OSAL_MBUF * mbufPtr);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief Callback prototype for free buffer level is low.
 *
 * IxAtmdAccRxVcFreeLowCallback is the prototype of the user function
 * which get called on a per-VC basis, when more  mbufs are needed to
 * continue the ATM data reception. This function is likely to supply
 * more available mbufs by one or many calls to the replenish function
 * @a ixAtmdAccRxVcFreeReplenish()
 *
 * This function is called when the number of available buffers for
 * reception is going under the threshold level as defined
 * in @a ixAtmdAccRxVcFreeLowCallbackRegister()
 *
 * This function is called inside an Qmgr dispatch context. No system
 * resource or interrupt-unsafe feature should be used inside this
 * callback.
 *
 * @sa ixAtmdAccRxVcFreeLowCallbackRegister
 * @sa IxAtmdAccRxVcFreeLowCallback
 * @sa ixAtmdAccRxVcFreeReplenish
 * @sa ixAtmdAccRxVcFreeEntriesQuery
 * @sa ixAtmdAccRxVcConnect
 *
 * @param userId @ref IxAtmdAccUserId [in] - user Id provided in the call
 * to @a ixAtmdAccRxVcConnect()
 *
 * @return None
 *
 */
typedef void (*IxAtmdAccRxVcFreeLowCallback) (IxAtmdAccUserId userId);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to TX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @brief  Buffer callback prototype.
 *
 * This function is called to relinguish ownership of a transmitted
 * buffer chain to the user.
 *
 * @note -In the case of a chained mbuf the AmtdAcc component can
 * chain many user buffers together and pass ownership to the user in
 * one function call.
 *
 * @param userId @ref IxAtmdAccUserId [in] - user If provided at registration of this
 *        callback.
 * @param mbufPtr @ref IX_OSAL_MBUF [in] - a pointer to mbufs or chain of mbufs and is
 *        guaranteed not to be a null pointer.
 *
 */
typedef void (*IxAtmdAccTxVcBufferReturnCallback) (IxAtmdAccUserId userId,
                        IX_OSAL_MBUF * mbufPtr);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to Initialisation
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccInit (void)
 *
 * @brief Initialise the IxAtmdAcc Component
 *
 * This function initialise the IxAtmdAcc component. This function shall
 * be called before any other function of the API. Its role is to
 * initialise all internal resources of the IxAtmdAcc component.
 *
 * The ixQmgr component needs to be initialized prior the use of
 * @a ixAtmdAccInit()
 *
 * @param none
 *
 * Failing to initilialize the IxAtmdAcc API before any use of it will
 * result in a failed status.
 * If the specified component is not present, a success status will still be 
 * returned, however, a warning indicating the NPE to download to is not
 * present will be issued.
 *
 * @return @li IX_SUCCESS initialisation is complete (in case of component not
 *             being present, a warning is clearly indicated)
 * @return @li IX_FAIL unable to process this request either
 *             because this IxAtmdAcc is already initialised
 *             or some unspecified error has occrred.
 */
PUBLIC IX_STATUS ixAtmdAccInit (void);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccShow (void)
 *
 * @brief Show IxAtmdAcc configuration on a per port basis
 *
 * @param none
 *
 * @return none
 *
 * @note - Display use printf() and are redirected to stdout
 */
PUBLIC void
ixAtmdAccShow (void);

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccStatsShow (void)
 *
 * @brief Show all IxAtmdAcc stats
 *
 * @param none
 *
 * @return none
 *
 * @note - Stats display use printf() and are redirected to stdout
 */
PUBLIC void
ixAtmdAccStatsShow (void);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccStatsReset (void)
 *
 * @brief Reset all IxAtmdAcc stats
 *
 * @param none
 *
 * @return none
 *
 */
PUBLIC void
ixAtmdAccStatsReset (void);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to RX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccRxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmRxQueueId rxQueueId,
                      IxAtmdAccUserId userCallbackId,
                      IxAtmdAccRxVcRxCallback rxCallback,
                      unsigned int minimumReplenishCount,
                      IxAtmConnId * connIdPtr,
                      IxAtmNpeRxVcId * npeVcIdPtr )
 *
 * @brief Connect to a Aal Pdu receive service for a particular
 *        port/vpi/vci, and service type.
 *
 * This function allows a user to connect to an Aal5/Aal0/OAM Pdu receive service
 * for a particular port/vpi/vci. It registers the callback and allocates
 * internal resources and a Connection Id to be used in further API calls
 * related to this VCC.
 *
 * The function will setup VC receive service on the specified rx queue.
 *
 * This function is blocking and makes use internal locks, and hence
 * should not be called from an interrupt context.
 *
 * On return from @a ixAtmdAccRxVcConnect() with a failure status, the
 * connection Id parameter is unspecified. Its value cannot be used.
 * A connId is the reference by which IxAtmdAcc refers to a
 * connected VC. This identifier is the result of a succesful call
 * to a connect function. This identifier is invalid after a
 * sucessful call to a disconnect function.
 *
 * Calling this function for the same combination of Vpi, Vci and more
 * than once without calling @a ixAtmdAccRxVcTryDisconnect() will result in a
 * failure status.
 *
 * If this function returns success the user should supply receive
 * buffers by calling @a ixAtmdAccRxVcFreeReplenish() and then call
 * @a ixAtmdAccRxVcEnable() to begin receiving pdus.
 *
 * There is a choice of two receive Qs on which the VC pdus could be
 * receive. The user must associate the VC with one of these. Essentially
 * having two qs allows more flexible system configuration such as have
 * high prioriy traffic on one q (e.g. voice) and low priority traffic on
 * the other (e.g. data). The high priority Q could be serviced in
 * preference to the low priority Q. One queue may be configured to be
 * serviced as soon as there is traffic, the other queue may be configured
 * to be serviced by a polling mechanism running at idle time.
 *
 * Two AAL0 services supporting 48 or 52 byte cell data are provided.
 * Received AAL0 PDUs will be be a multiple of the cell data size (48/52).
 * AAL0_52 is a raw cell service and includes an ATM cell header
 * (excluding HEC) at the start of each cell.
 *
 * A single "OAM Rx channel" for all ports may be  enabled by
 * establishing a dedicated OAM Rx connection.
 *
 * The OAM service allows buffers containing 52 byte OAM F4/F5 cells
 * to be transmitted and received over the dedicated OAM channels.
 * HEC is appended/removed, and CRC-10 performed by the NPE. The OAM
 * service offered by AtmdAcc is a raw cell transport service.
 * It is assumed that ITU I.610 procedures that make use of this
 * service are implemented above AtmdAcc.
 *
 * Note that the dedicated OAM connections are established on
 * reserved VPI,VCI, and (in the case of Rx) port values.
 * These values are used purely to descriminate the dedicated OAM channels
 * and do not identify a particular OAM F4/F5 flow. F4/F5 flows may be
 * realised for particluar VPI/VCIs by manipulating the VPI,VCI
 * fields of the ATM cell headers of cells in the buffers passed
 * to AtmdAcc.
 *
 * Calling this function prior to enable the port will fail.
 *
 * @sa ixAtmdAccRxDispatch
 * @sa ixAtmdAccRxVcEnable
 * @sa ixAtmdAccRxVcDisable
 * @sa ixAtmdAccRxVcTryDisconnect
 * @sa ixAtmdAccPortEnable
 *
 * @param port @ref IxAtmLogicalPort [in] - VC identification : logical PHY port
 *                  [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param vpi unsigned int [in] - VC identification : ATM Vpi [0..255] or IX_ATMDACC_OAM_VPI
 * @param vci unsigned int [in] - VC identification : ATM Vci [0..65535] or IX_ATMDACC_OAM_VCI
 * @param aalServiceType @ref IxAtmdAccAalType [in] - type of service: AAL5, AAL0_48, AAL0_52, or OAM
 * @param rxQueueId @ref IxAtmRxQueueId [in] - this identifieds which of two Qs the VC
 *     should use.when icoming traffic is processed
 * @param userCallbackId @ref IxAtmdAccUserId [in] - user Id used later as a parameter to
 *     the supplied rxCallback.
 * @param rxCallback [in] @ref IxAtmdAccRxVxRxCallback - function called when mbufs are received.
 *     This parameter cannot be a null pointer.
 * @param bufferFreeCallback [in] - function to be called to return
 *     ownership of buffers to IxAtmdAcc user.
 * @param minimumReplenishCount unsigned int [in] -  For AAL5/AAL0 the number of free mbufs
 *     to be used with this channel. Use a high number when the expected traffic
 *     rate on this channel is high, or when the user's mbufs are small, or when
 *     the RxVcFreeLow Notification has to be invoked less often. When this
 *     value is IX_ATMDACC_DEFAULT_REPLENISH_COUNT, the minimum of
 *     resources  will be used. Depending on traffic rate, pdu
 *     size and mbuf size, rxfree queue size, polling/interrupt rate, this value may
 *     require to be replaced by a different value in the range 1-128
 *     For OAM the rxFree queue size is fixed by atmdAcc and this parameter is ignored.
 * @param connIdPtr @ref IxAtmConnId [out] - pointer to a connection Id
 *     This parameter cannot be a null pointer.
 * @param npeVcIdPtr @ref IxAtmNpeRxVcId [out] - pointer to an npe Vc Id
 *     This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS successful call to IxAtmdAccRxVcConnect
 * @return @li IX_ATMDACC_BUSY cannot process this request :
 *         no VC is available
 * @return @li IX_FAIL
 *             parameter error,
 *             VC already in use,
 *             attempt to connect AAL service on reserved OAM VPI/VCI,
 *             attempt to connect OAM service on VPI/VCI other than the reserved OAM VPI/VCI,
 *             port is not initialised,
 *             or some other error occurs during processing.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmRxQueueId rxQueueId,
                      IxAtmdAccUserId userCallbackId,
                      IxAtmdAccRxVcRxCallback rxCallback,
                      unsigned int minimumReplenishCount,
                      IxAtmConnId * connIdPtr,
                      IxAtmNpeRxVcId * npeVcIdPtr );

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccRxVcFreeReplenish (IxAtmConnId connId,
                        IX_OSAL_MBUF * mbufPtr)
 *
 * @brief Provide free mbufs for data reception on a connection.
 *
 * This function provides mbufs for data reception by the hardware. This
 * function needs to be called by the user on a regular basis to ensure
 * no packet loss. Providing free buffers is a connection-based feature;
 * each connection can have different requirements in terms of buffer size
 * number of buffers, recycling rate. This function could be invoked from
 * within the context of a @a IxAtmdAccRxVcFreeLowCallback() callback
 * for a particular VC
 *
 * Mbufs provided through this function call can be chained. They will be
 * unchained internally. A call to this function with chained mbufs or
 * multiple calls with unchained mbufs are equivalent, but calls with
 * unchained mbufs are more efficients.
 *
 * Mbufs provided to this interface need to be able to hold at least one
 * full cell payload (48/52 bytes, depending on service type).
 * Chained buffers with a size less than the size supported by the hardware
 * will be returned through the rx callback provided during the connect step.
 *
 * Failing to invoke this function prior to enabling the RX traffic
 * can result in packet loss.
 *
 * This function is not reentrant for the same connId.
 *
 * This function does not use system resources and can be
 * invoked from an interrupt context.
 *
 * @note - Over replenish is detected, and extra mbufs are returned through
 *         the rx callback provided during the connect step.
 *
 * @note - Mbuf provided to the replenish function should have a length greater or
 *         equal to 48/52 bytes according to service type.
 *
 * @note - The memory cache of mMbuf payload should be invalidated prior to Mbuf
 *         submission. Flushing the Mbuf headers is handled by IxAtmdAcc.
 *
 * @note - When a chained mbuf is provided, this function process the mbufs
 *         up to the hardware limit and invokes the user-supplied callback
 *         to release extra buffers.
 *
 * @sa ixAtmdAccRxVcFreeLowCallbackRegister
 * @sa IxAtmdAccRxVcFreeLowCallback
 * @sa ixAtmdAccRxVcConnect
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as returned from a succesfull call to
 *        @a IxAtmdAccRxVcConnect()
 * @param mbufPtr @ref IX_OSAL_MBUF [in] - pointer to a mbuf structure to be used for data
 *        reception. The mbuf pointed to by this parameter can be chained
 *        to an other mbuf.
 *
 * @return @li IX_SUCCESS successful call to @a ixAtmdAccRxVcFreeReplenish()
 *          and the mbuf is now ready to use for incoming traffic.
 * @return @li IX_ATMDACC_BUSY cannot process this request because
 *         the max number of outstanding free buffers has been reached
 *         or the internal resources have exhausted for this VC.
 *         The user is responsible for retrying this request later.
 * @return @li IX_FAIL cannot process this request because of parameter
 *         errors or some unspecified internal error has occurred.
 *
 * @note - It is not always guaranteed the replenish step to be as fast as the
 *   hardware is consuming Rx Free mbufs. There is nothing in IxAtmdAcc to
 *   guarantee that replenish reaches the rxFree threshold level. If the
 *   threshold level is not reached, the next rxFree low notification for
 *   this channel will not be triggered.
 *   The preferred ways to replenish can be as follows (depending on
 *   applications and implementations) :
 *   @li Replenish in a rxFree low notification until the function
 *       ixAtmdAccRxVcFreeReplenish() returns IX_ATMDACC_BUSY
 *   @li Query the queue level using @sa ixAtmdAccRxVcFreeEntriesQuery, then
 *     , replenish using @a ixAtmdAccRxVcFreeReplenish(), then query the queue
 *       level again, and replenish if the threshold is still not reached.
 *   @li Trigger replenish from an other event source and use rxFree starvation
 *       to throttle the Rx traffic.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcFreeReplenish (IxAtmConnId connId,
                        IX_OSAL_MBUF * mbufPtr);

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccRxVcFreeLowCallbackRegister (IxAtmConnId connId,
                                    unsigned int numberOfMbufs,
                                    IxAtmdAccRxVcFreeLowCallback callback)
 *
 * @brief Configure the RX Free threshold value and register a callback
 * to handle threshold notifications.
 *
 * The function ixAtmdAccRxVcFreeLowCallbackRegister sets the threshold value for
 * a particular RX VC. When the number of buffers reaches this threshold
 * the callback is invoked.
 *
 * This function should be called once per VC before RX traffic is
 * enabled.This function will fail if the curent level of the free buffers
 * is equal or less than the threshold value.
 *
 * @sa ixAtmdAccRxVcFreeLowCallbackRegister
 * @sa IxAtmdAccRxVcFreeLowCallback
 * @sa ixAtmdAccRxVcFreeReplenish
 * @sa ixAtmdAccRxVcFreeEntriesQuery
 * @sa ixAtmdAccRxVcConnect
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call
 *        to @a IxAtmdAccRxVcConnect()
 * @param numberOfMbufs unsigned int [in] - threshold number of buffers. This number
 *        has to be a power of 2, one of the values 0,1,2,4,8,16,32....
 *        The maximum value cannot be more than half of the rxFree queue
 *        size (which can be retrieved using @a ixAtmdAccRxVcFreeEntriesQuery()
 *        before any use of the @a ixAtmdAccRxVcFreeReplenish() function)
 * @param callback @ref IxAtmdAccRxVcFreeLowCallback [in] - function telling the user that the number of
 *        free buffers has reduced to the threshold value.
 *
 * @return @li IX_SUCCESS Threshold set successfully.
 * @return @li IX_FAIL parameter error or the current number of free buffers
 *              is less than or equal to the threshold supplied or some
 *              unspecified error has occrred.
 *
 * @note - the callback will be called when the threshold level will drop from
 *        exactly (numberOfMbufs + 1) to (numberOfMbufs).
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcFreeLowCallbackRegister (IxAtmConnId connId,
    unsigned int numberOfMbufs,
    IxAtmdAccRxVcFreeLowCallback callback);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccRxVcFreeEntriesQuery (IxAtmConnId connId,
                         unsigned int *numberOfMbufsPtr)
 *
 * @brief Get the number of rx mbufs the system can accept to replenish the
 *       the rx reception mechanism on a particular channel
 *
 * The ixAtmdAccRxVcFreeEntriesQuery function is used to retrieve the current
 * number of available mbuf entries for reception, on a per-VC basis. This
 * function can be used to know the number of mbufs which can be provided
 * using @a ixAtmdAccRxVcFreeReplenish().
 *
 * This function can be used from a timer context, or can be associated
 * with a threshold event, or can be used inside an active polling
 * mechanism which is under user control.
 *
 * This function is reentrant and does not use system resources and can
 * be invoked from an interrupt context.
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call
 *        to @a IxAtmdAccRxVcConnect()
 * @param numberOfMbufsPtr unsigned int [out] - Pointer to the number of available entries.
 *      . This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS the current number of mbufs not yet used for incoming traffic
 * @return @li IX_FAIL invalid parameter
 *
 * @sa ixAtmdAccRxVcFreeReplenish
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcFreeEntriesQuery (IxAtmConnId connId,
                         unsigned int *numberOfMbufsPtr);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccRxVcEnable (IxAtmConnId connId)
 *
 * @brief Start the RX service on a VC.
 *
 * This functions kicks-off the traffic reception for a particular VC.
 * Once invoked, incoming PDUs will be made available by the hardware
 * and are eventually directed to the @a IxAtmdAccRxVcRxCallback() callback
 * registered for the connection.
 *
 * If the traffic is already running, this function returns IX_SUCCESS.
 * This function can be invoked many times.
 *
 * IxAtmdAccRxVcFreeLowCallback event will occur only after
 * @a ixAtmdAccRxVcEnable() function is invoked.
 *
 * Before using this function, the @a ixAtmdAccRxVcFreeReplenish() function
 * has to be used to replenish the RX Free queue. If not, incoming traffic
 * may be discarded.and in the case of interrupt driven reception the
 * @a IxAtmdAccRxVcFreeLowCallback() callback may be invoked as a side effect
 * during a replenish action.
 *
 * This function is not reentrant and should not be used inside an
 * interrupt context.
 *
 * For an VC connection this function can be called after a call to
 * @a ixAtmdAccRxVcDisable() and should not be called after
 * @a ixAtmdAccRxVcTryDisconnect()
 *
 * @sa ixAtmdAccRxVcDisable
 * @sa ixAtmdAccRxVcConnect
 * @sa ixAtmdAccRxVcFreeReplenish
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call
 * to @a IxAtmdAccRxVcConnect()
 *
 * @return @li IX_SUCCESS successful call to ixAtmdAccRxVcEnable
 * @return @li IX_ATMDACC_WARNING the channel is already enabled
 * @return @li IX_FAIL invalid parameters or some unspecified internal
 *         error occured.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcEnable (IxAtmConnId connId);

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccRxVcDisable (IxAtmConnId connId)
 *
 * @brief Stop the RX service on a VC.
 *
 * This functions stops the traffic reception for a particular VC connection.
 *
 * Once invoked, incoming Pdus are discarded by the hardware. Any Pdus
 * pending will be freed to the user
 *
 * Hence once this function returns no more receive callbacks will be
 * called for that VC. However, buffer free callbacks will be invoked
 * until such time as all buffers supplied by the user have been freed
 * back to the user
 *
 * Calling this function doe not invalidate the connId.
 * @a ixAtmdAccRxVcEnable() can be invoked to enable Pdu reception again.
 *
 * If the traffic is already stopped, this function returns IX_SUCCESS.
 *
 * This function is not reentrant and should not be used inside an
 * interrupt context.
 *
 * @sa ixAtmdAccRxVcConnect
 * @sa ixAtmdAccRxVcEnable
 * @sa ixAtmdAccRxVcDisable
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call to @a
 *     IxAtmdAccRxVcConnect()
 *
 * @return @li IX_SUCCESS successful call to @a ixAtmdAccRxVcDisable().
 * @return @li IX_ATMDACC_WARNING the channel is already disabled
 * @return @li IX_FAIL invalid parameters or some unspecified internal error occured
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcDisable (IxAtmConnId connId);

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccRxVcTryDisconnect (IxAtmConnId connId)
 *
 * @brief Disconnect a VC from the RX service.
 *
 * This function deregisters the VC and guarantees that all resources
 * associated with this VC are free. After its execution, the connection
 * Id is not available.
 *
 * This function will fail until such time as all resources allocated to
 * the VC connection have been freed. The user is responsible to delay and
 * call again this function many times until a success status is returned.
 *
 * This function needs internal locks and should not be called from an
 * interrupt context
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call to
 * @a IxAtmdAccRxVcConnect()
 *
 * @return @li IX_SUCCESS successful call to ixAtmdAccRxVcDisable
 * @return @li IX_ATMDACC_RESOURCES_STILL_ALLOCATED not all resources
 *         associated with the connection have been freed.
 * @return @li IX_FAIL cannot process this request because of a parameter
 *         error
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxVcTryDisconnect (IxAtmConnId connId);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to TX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccAPI
 * 
 * @fn ixAtmdAccTxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmdAccUserId userId,
                      IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
                      IxAtmConnId * connIdPtr)
 *
 * @brief Connect to a Aal Pdu transmit service for a particular
 *        port/vpi/vci and service type.
 *
 * This function allows a user to connect to an Aal5/Aal0/OAM Pdu transmit service
 * for a particular port/vpi/vci. It registers the callback and allocates
 * internal resources and a Connection Id to be used in further API calls
 * related to this VC.
 *
 * The function will setup VC transmit service on the specified on the
 * specified port. A connId is the reference by which IxAtmdAcc refers to a
 * connected VC. This identifier is the result of a succesful call
 * to a connect function. This identifier is invalid after a
 * sucessful call to a disconnect function.
 *
 * This function needs internal locks, and hence  should not be called
 * from an interrupt context.
 *
 * On return from @a ixAtmdAccTxVcConnect() with a failure status, the
 * connection Id parameter is unspecified. Its value cannot be used.
 *
 * Calling this function for the same combination of port, Vpi, Vci and
 * more than once without calling @a ixAtmdAccTxVcTryDisconnect() will result
 * in a failure status.
 *
 * Two AAL0 services supporting 48 or 52 byte cell data are provided.
 * Submitted AAL0 PDUs must be a multiple of the cell data size (48/52).
 * AAL0_52 is a raw cell service the client must format
 * the PDU with an ATM cell header (excluding HEC) at the start of
 * each cell, note that AtmdAcc does not validate the cell headers in
 * a submitted PDU.
 *
 * For the OAM service an "OAM Tx channel" may be enabled for a port
 * by establishing a single dedicated OAM Tx connection on that port.
 *
 * The OAM service allows buffers containing 52 byte OAM F4/F5 cells
 * to be transmitted and received over the dedicated OAM channels.
 * HEC is appended/removed, and CRC-10 performed by the NPE. The OAM
 * service offered by AtmdAcc is a raw cell transport service.
 * It is assumed that ITU I.610 procedures that make use of this
 * service are implemented above AtmdAcc.
 *
 * Note that the dedicated OAM connections are established on
 * reserved VPI,VCI, and (in the case of Rx) port values.
 * These values are used purely to descriminate the dedicated OAM channels
 * and do not identify a particular OAM F4/F5 flow. F4/F5 flows may be
 * realised for particluar VPI/VCIs by manipulating the VPI,VCI
 * fields of the ATM cell headers of cells in the buffers passed
 * to AtmdAcc.
 *
 * Calling this function before enabling the port will fail.
 *
 * @sa ixAtmdAccTxVcTryDisconnect
 * @sa ixAtmdAccPortTxScheduledModeEnable
 * @sa ixAtmdAccPortEnable
 *
 * @param port @ref IxAtmLogicalPort [in] - VC identification : logical PHY port
 *                  [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param vpi unsigned int  [in] - VC identification : ATM Vpi [0..255] or IX_ATMDACC_OAM_VPI
 * @param vci unsigned int [in] - VC identification : ATM Vci [0..65535] or IX_ATMDACC_OAM_VCI
 * @param aalServiceType @ref IxAtmdAccAalType [in] - type of service AAL5, AAL0_48, AAL0_52, or OAM
 * @param userId @ref IxAtmdAccUserId [in] - user id to be used later during callbacks related
 *        to this channel
 * @param bufferFreeCallback @ref IxAtmdAccTxVcBufferReturnCallback [in] - function called when mbufs
 *        transmission is complete. This parameter cannot be a null
 *        pointer.
 * @param connIdPtr @ref IxAtmConnId [out] - Pointer to a connection Id.
 *        This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS successful call to @a IxAtmdAccRxVcConnect().
 * @return @li IX_ATMDACC_BUSY cannot process this request
 *         because no VC is available
 * @return @li IX_FAIL
 *             parameter error,
 *             VC already in use,
 *             attempt to connect AAL service on reserved OAM VPI/VCI,
 *             attempt to connect OAM service on VPI/VCI other than the reserved OAM VPI/VCI,
 *             port is not initialised,
 *             or some other error occurs during processing.
 *
 * @note - Unscheduled mode is not supported in ixp425 1.0. Therefore, the
 *       function @a ixAtmdAccPortTxScheduledModeEnable() need to be called
 *       for this port before any establishing a Tx Connection
 */
PUBLIC IX_STATUS ixAtmdAccTxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmdAccUserId userId,
                      IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
                      IxAtmConnId * connIdPtr);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccTxVcPduSubmit (IxAtmConnId connId,
                    IX_OSAL_MBUF * mbufPtr,
                    IxAtmdAccClpStatus clp,
                    unsigned int numberOfCells)
 *
 * @brief Submit a Pdu for transmission on connection.
 *
 * A data user calls this function to submit an mbufs containing a Pdu
 * to be transmitted. The buffer supplied can be chained and the Pdu it
 * contains must be complete.
 *
 * The transmission behavior of this call depends on the operational mode
 * of the port on which the connection is made.
 *
 * In unscheduled mode the mbuf will be submitted to the hardware
 * immediately if sufficent resource is available. Otherwise the function
 * will return failure.
 *
 * In scheduled mode the buffer is queued internally in IxAtmdAcc. The cell
 * demand is made known to the traffic shaping entity. Cells from the
 * buffers are MUXed onto the port some time later as dictated by the
 * traffic shaping entity. The traffic shaping entity does this by sending
 * transmit schedules to IxAtmdAcc via @a ixAtmdAccPortTxProcess() function call.
 *
 * Note that the dedicated OAM channel is scheduled just like any
 * other channel. This means that any OAM traffic relating to an
 * active AAL0/AAL5 connection will be scheduled independantly of the
 * AAL0/AAL5 traffic for that connection.
 *
 * When transmission is complete, the TX Done mechanism will give the
 * owmnership of these buffers back to the customer. The tx done mechanism
 * must be in operation before transmission is attempted.
 *
 * For AAL0/OAM submitted AAL0 PDUs must be a multiple of the cell data
 * size (48/52). AAL0_52 and OAM are raw cell services, and the client
 * must format the PDU with an ATM cell header (excluding HEC) at the
 * start of each cell, note that AtmdAcc does not validate the cell headers in
 * a submitted PDU.
 *
 *
 * @sa IxAtmdAccTxVcBufferReturnCallback
 * @sa ixAtmdAccTxDoneDispatch
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call to
 *        @a ixAtmdAccTxVcConnect()
 * @param mbufPtr @ref IX_OSAL_MBUF [in] - pointer to a chained structure of mbufs to transmit.
 *       This parameter cannot be a null pointer.
 * @param clp @ref IxAtmdAccClpStatus [in] - clp indication for this PDU. All cells of this pdu
 *       will be sent with the clp bit set
 * @param numberOfCells unsigned int [in] - number of cells in the PDU.
 *
 * @return @li IX_SUCCESS successful call to @a ixAtmdAccTxVcPduSubmit()
 *             The pdu pointed by the mbufPtr parameter will be
 *             transmitted
 * @return @li IX_ATMDACC_BUSY unable to process this request because
 *             internal resources are all used. The caller is responsible
 *             for retrying this request later.
 * @return @li IX_FAIL unable to process this request because of error
 *             in the parameters (wrong connId supplied,
 *             or wrong mbuf pointer supplied), the total length of all buffers
 *             in the chain should be a multiple of the cell size
 *             ( 48/52 depending on the service type ),
 *             or unspecified error during processing
 *
 * @note - This function in not re-entrant for the same VC (e.g. : two
 *         thread cannot send PDUs for the same VC). But two threads can
 *         safely call this function with a different connection Id
 *
 * @note - In unscheduled mode, this function is not re-entrant on a per
 *         port basis. The size of pdus is limited to 8Kb.
 *
 * @note - 0-length mbufs should be removed from the chain before submission.
 *         The total length of the pdu (sdu + padding +trailer) has to be
 *         updated in the header of the first mbuf of a chain of mbufs.
 *
 * @note - Aal5 trailer information (UUI, CPI, SDU length) has to be supplied
 *         before submission.
 *
 * @note - The payload memory cache should be flushed, if needed, prior to
 *         transmission. Mbuf headers are flushed by IxAtmdAcc
 *
 * @note - This function does not use system resources and can be used
 *         inside an interrupt context
 */
PUBLIC IX_STATUS ixAtmdAccTxVcPduSubmit (IxAtmConnId connId,
                    IX_OSAL_MBUF * mbufPtr,
                    IxAtmdAccClpStatus clp,
                    unsigned int numberOfCells);

/**
 *
 * @ingroup IxAtmdAccAPI
 *
 * @fn ixAtmdAccTxVcTryDisconnect (IxAtmConnId connId)
 *
 * @brief Disconnect from a Aal Pdu transmit service for a particular
 *        port/vpi/vci.
 *
 * This function deregisters the VC and guarantees that all resources
 * associated with this VC are free. After its execution, the connection
 * Id is not available.
 *
 * This function will fail until such time as all resources allocated to
 * the VC connection have been freed. The user is responsible to delay
 * and call again this function many times until a success status is
 * returned.
 *
 * After its execution, the connection Id is not available.
 *
 * @param connId @ref IxAtmConnId [in] - connection Id as resulted from a succesfull call to
 *        @a ixAtmdAccTxVcConnect()
 *
 * @return @li IX_SUCCESS successful call to @a ixAtmdAccTxVcTryDisconnect()
 * @return @li IX_ATMDACC_RESOURCES_STILL_ALLOCATED not all resources
 *     associated with the connection have been freed. This condition will
 *     disappear after Tx and TxDone is complete for this channel.
 * @return @li IX_FAIL unable to process this request because of errors
 *                     in the parameters (wrong connId supplied)
 *
 * @note - This function needs internal locks and should not be called
 *         from an interrupt context
 *
 * @note - If the @a IX_ATMDACC_RESOURCES_STILL_ALLOCATED error does not
 *     clear after a while, this may be linked to a previous problem
 *     of cell overscheduling. Diabling the port and retry a disconnect
 *     will free the resources associated with this channel.
 *
 * @sa ixAtmdAccPortTxProcess
 *
 */
PUBLIC IX_STATUS ixAtmdAccTxVcTryDisconnect (IxAtmConnId connId);

#endif /* IXATMDACC_H */

/**
 * @} defgroup IxAtmdAccAPI
 */


