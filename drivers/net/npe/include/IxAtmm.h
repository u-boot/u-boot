/**
 * @file    IxAtmm.h
 *
 * @date    3-DEC-2001
 *
 * @brief   Header file for the IXP400 ATM Manager component (IxAtmm)
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


/**
 * @defgroup IxAtmm IXP400 ATM Manager (IxAtmm) API
 *
 * @brief IXP400 ATM Manager component Public API
 *
 * @{
 */

#ifndef IXATMM_H
#define IXATMM_H

/*
 * Put the user defined include files required
 */
#include "IxAtmSch.h"
#include "IxOsalTypes.h"

/*
 * #defines and macros used in this file.
 */

/** 
 * @def IX_ATMM_RET_ALREADY_INITIALIZED
 * 
 * @brief Component has already been initialized 
 */
#define IX_ATMM_RET_ALREADY_INITIALIZED 2

/** 
 * @def IX_ATMM_RET_INVALID_PORT
 * 
 * @brief Specified port does not exist or is out of range */
#define IX_ATMM_RET_INVALID_PORT 3

/** 
 * @def IX_ATMM_RET_INVALID_VC_DESCRIPTOR
 * 
 * @brief The VC description does not adhere to ATM standards */
#define IX_ATMM_RET_INVALID_VC_DESCRIPTOR 4

/** 
 * @def IX_ATMM_RET_VC_CONFLICT
 * 
 * @brief The VPI/VCI values supplied are either reserved, or they
 *         conflict with a previously registered VC on this port */
#define IX_ATMM_RET_VC_CONFLICT 5

/** 
 * @def IX_ATMM_RET_PORT_CAPACITY_IS_FULL
 * 
 * @brief The virtual connection cannot be established on the port
 *         because the remaining port capacity is not sufficient to
 *         support it */
#define IX_ATMM_RET_PORT_CAPACITY_IS_FULL 6

/** 
 * @def IX_ATMM_RET_NO_SUCH_VC
 * 
 * @brief No registered VC, as described by the supplied VCI/VPI or
 *         VC identifier values, exists on this port */
#define IX_ATMM_RET_NO_SUCH_VC 7

/** 
 * @def IX_ATMM_RET_INVALID_VC_ID
 * 
 * @brief The specified VC identifier is out of range. */
#define IX_ATMM_RET_INVALID_VC_ID 8

/** 
 * @def IX_ATMM_RET_INVALID_PARAM_PTR
 * 
 * @brief A pointer parameter was NULL. */
#define IX_ATMM_RET_INVALID_PARAM_PTR 9

/** 
 * @def IX_ATMM_UTOPIA_SPHY_ADDR  
 * 
 * @brief The phy address when in SPHY mode */
#define IX_ATMM_UTOPIA_SPHY_ADDR 31

/**
 * @def IX_ATMM_THREAD_PRI_HIGH
 *
 * @brief The value of high priority thread */
#define IX_ATMM_THREAD_PRI_HIGH 90

/*
 * Typedefs whose scope is limited to this file.
 */

/** @brief Definition for use in the @ref IxAtmmVc structure.
 *         Indicates the direction of a VC */
typedef enum
{
    IX_ATMM_VC_DIRECTION_TX=0, /**< Atmm Vc direction transmit*/
    IX_ATMM_VC_DIRECTION_RX, /**< Atmm Vc direction receive*/
    IX_ATMM_VC_DIRECTION_INVALID /**< Atmm Vc direction invalid*/
} IxAtmmVcDirection;

/** @brief Definition for use with @ref IxAtmmVcChangeCallback
 *         callback.  Indicates that the event type represented by the
 *         callback for this VC. */
typedef enum
{
    IX_ATMM_VC_CHANGE_EVENT_REGISTER=0, /**< Atmm Vc event register*/
    IX_ATMM_VC_CHANGE_EVENT_DEREGISTER, /**< Atmm Vc event de-register*/
    IX_ATMM_VC_CHANGE_EVENT_INVALID /**< Atmm Vc event invalid*/
} IxAtmmVcChangeEvent;

/** @brief Definitions for use with @ref ixAtmmUTOPIAInit interface to
 *         indicate that UTOPIA loopback should be enabled or disabled
 *         on initialisation. */
typedef enum
{
    IX_ATMM_UTOPIA_LOOPBACK_DISABLED=0, /**< Atmm Utopia loopback mode disabled*/
    IX_ATMM_UTOPIA_LOOPBACK_ENABLED, /**< Atmm Utopia loopback mode enabled*/
    IX_ATMM_UTOPIA_LOOPBACK_INVALID /**< Atmm Utopia loopback mode invalid*/
} IxAtmmUtopiaLoopbackMode;

/** @brief This structure describes the required attributes of a
 *         virtual connection.
*/
typedef struct {
    unsigned vpi;  /**< VPI value of this virtual connection */
    unsigned vci;  /**< VCI value of this virtual connection. */
    IxAtmmVcDirection direction; /**< VC direction */

    /** Traffic descriptor of this virtual connection.  This structure
     *  is defined by the @ref IxAtmSch component.  */
    IxAtmTrafficDescriptor trafficDesc;
} IxAtmmVc;


/** @brief Definitions for use with @ref ixAtmmUtopiaInit interface to
 *         indicate that UTOPIA multi-phy/single-phy mode is used.
 */
typedef enum
{
    IX_ATMM_MPHY_MODE = 0, /**< Atmm phy mode mphy*/
    IX_ATMM_SPHY_MODE, /**< Atmm phy mode sphy*/
    IX_ATMM_PHY_MODE_INVALID /**< Atmm phy mode invalid*/
} IxAtmmPhyMode;


/** @brief Structure contains port-specific information required to
 *         initialize IxAtmm, and specifically, the IXP400 UTOPIA
 *         Level-2 device. */
typedef struct {
    unsigned reserved_1:11;     /**< [31:21] Should be zero */
    unsigned UtopiaTxPhyAddr:5; /**< [20:16] Address of the
     *   transmit (Tx) PHY for this
     *   port on the 5-bit UTOPIA
     *   Level-2 address bus */
    unsigned reserved_2:11;     /**< [15:5] Should be zero */
    unsigned UtopiaRxPhyAddr:5; /**< [4:0] Address of the receive
     *   (Rx) PHY for this port on the
     *   5-bit UTOPIA  Level-2
     *   address bus */
} IxAtmmPortCfg;

/** @brief Callback type used with @ref ixAtmmVcChangeCallbackRegister interface
 *         Defines a callback type  which will be used to notify registered
 *         users of registration/deregistration events on a particular port
 *
 * @param eventType @ref IxAtmmVcChangeEvent [in] - Event indicating
 *                        whether the VC supplied has been added or
 *                        removed
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the port on which the event has
 *                        occurred
 *
 * @param vcChanged @ref IxAtmmVc* [in] - Pointer to a structure which gives
 *                              details of the VC which has been added
 *                              or removed on the port
 */
typedef void (*IxAtmmVcChangeCallback) (IxAtmmVcChangeEvent eventType,
					IxAtmLogicalPort port,
					const IxAtmmVc* vcChanged);

/*
 * Variable declarations global to this file only. Externs are followed by
 * static variables.
 */

/*
 * Extern function prototypes
 */

/*
 * Function declarations
 */


/** 
 * @ingroup IxAtmm
 *
 * @fn ixAtmmInit (void)
 *
 * @brief Interface to initialize the IxAtmm software component.  Can
 *         be called once only.
 *
 *  Must be called before any other IxAtmm API is called.
 *
 * @param "none"
 *
 *  @return @li  IX_SUCCESS : IxAtmm has been successfully initialized.
 *      Calls to other IxAtmm interfaces may now be performed.
 *  @return @li  IX_FAIL : IxAtmm has already been initialized.
 */
PUBLIC IX_STATUS
ixAtmmInit (void);

/**  
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmUtopiaInit (unsigned numPorts,
		  IxAtmmPhyMode phyMode,
		  IxAtmmPortCfg portCfgs[],
		  IxAtmmUtopiaLoopbackMode loopbackMode)
 *
 * @brief Interface to initialize the UTOPIA Level-2 ATM coprocessor
 *         for the specified number of physical ports.  The function
 *         must be called before the ixAtmmPortInitialize interface
 *         can operate successfully.
 *
 * @param numPorts unsigned [in] - Indicates the total number of logical
 *          ports that are active on the device.  Up to 12 ports are
 *          supported.
 *
 * @param phyMode @ref IxAtmmPhyMode [in] - Put the Utopia coprocessor in SPHY
 *        or MPHY mode.
 *
 * @param portCfgs[] @ref IxAtmmPortCfg [in] - Pointer to an array of elements
 *          detailing the UTOPIA specific port characteristics.  The
 *          length of the array must be equal to the number of ports
 *          activated.  ATM ports are referred to by the relevant
 *          offset in this array in all subsequent IxAtmm interface
 *          calls.
 *
 * @param loopbackMode @ref IxAtmmUtopiaLoopbackMode [in] - Value must be one of
 *          @ref IX_ATMM_UTOPIA_LOOPBACK_ENABLED or @ref
 *          IX_ATMM_UTOPIA_LOOPBACK_DISABLED indicating whether
 *          loopback should be enabled on the device.  Loopback can
 *          only be supported on a single PHY, therefore the numPorts
 *          parameter must be 1 if loopback is enabled.
 *
 * @return @li IX_SUCCESS : Indicates that the  UTOPIA device has been
 *      successfully initialized for the supplied ports.
 * @return @li IX_ATMM_RET_ALREADY_INITIALIZED : The UTOPIA device has
 *      already been initialized.
 * @return @li IX_FAIL : The supplied parameters are invalid or have been
 *     rejected by the UTOPIA-NPE device.
 *
 * @warning
 * This interface may only be called once.
 * Port identifiers are assumed to range from 0 to (numPorts - 1) in all 
 * instances.
 * In all subsequent calls to interfaces supplied by IxAtmm, the specified
 * port value is expected to represent the offset in the portCfgs array
 * specified in this interface.  i.e. The first port in this array will
 * subsequently be represented as port 0, the second port as port 1,
 * and so on.*/
PUBLIC IX_STATUS
ixAtmmUtopiaInit (unsigned numPorts,
		  IxAtmmPhyMode phyMode,
		  IxAtmmPortCfg portCfgs[],
		  IxAtmmUtopiaLoopbackMode loopbackMode);


/**   
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmPortInitialize (IxAtmLogicalPort port,
		      unsigned txPortRate,
		      unsigned rxPortRate)
 *
 * @brief The interface is called following @ref ixAtmmUtopiaInit ()
 *         and before calls to any other IxAtmm interface.  It serves
 *         to activate the registered ATM port with IxAtmm.
 *
 *  The transmit and receive port rates are specified in bits per
 *  second.  This translates to ATM cells per second according to the
 *  following formula: CellsPerSecond = portRate / (53*8)  The
 *  IXP400 device supports only 53 byte cells. The client shall make
 *  sure that the off-chip physical layer device has already been
 *  initialized.
 *
 *  IxAtmm will configure IxAtmdAcc and IxAtmSch to enable scheduling
 *  on the port.
 *
 *  This interface must be called once for each active port in the
 *  system.  The first time the interface is invoked, it will configure
 *  the mechanism by which the handling of transmit, transmit-done and
 *  receive are driven with the IxAtmdAcc component.
 *
 *  This function is reentrant.
 *
 *  @note The minimum tx rate that will be accepted is 424 bit/s which equates
 *        to 1 cell (53 bytes) per second.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies the port which is to be
 *          initialized.
 *
 * @param txPortRate unsigned [in] - Value specifies the
 *          transmit port rate for this port in
 *          bits/second.  This value is used by the ATM Scheduler
 *          component is evaluating VC access requests for the port.
 *
 * @param rxPortRate unsigned [in] - Value specifies the
 *          receive port rate for this port in bits/second.
 *
 * @return @li IX_SUCCESS : The specificed ATM port has been successfully
 *       initialized. IxAtmm is ready to accept VC registrations on
 *       this port.
 *
 * @return @li IX_ATMM_RET_ALREADY_INITIALIZED : ixAtmmPortInitialize has
 *       already been called successfully on this port.  The current
 *       call is rejected.
 *
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid.  The request is rejected.
 *
 * @return @li IX_FAIL : IxAtmm could not initialize the port because the
 * inputs are not understood.
 *
 * @sa ixAtmmPortEnable, ixAtmmPortDisable
 *
 */
PUBLIC IX_STATUS
ixAtmmPortInitialize (IxAtmLogicalPort port,
		      unsigned txPortRate,
		      unsigned rxPortRate);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmPortModify (IxAtmLogicalPort port,
		  unsigned txPortRate,
		  unsigned rxPortRate)
 *
 * @brief A client may call this interface to change the existing
 *         port rate (expressed in bits/second) on an established ATM
 *         port.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies the port which is to be
 *          initialized.
 *
 * @param txPortRate unsigned [in] -  Value specifies the``
 *          transmit port rate for this port in
 *          bits/second.  This value is used by the ATM Scheduler
 *          component is evaluating VC access requests for the port.
 *
 * @param rxPortRate unsigned [in] - Value specifies the
 *          receive port rate for this port in
 *          bits/second.
 *
 * @return @li IX_SUCCESS : The indicated ATM port rates have been
 *      successfully modified.
 *
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid.  The request is rejected.
 *
 * @return @li IX_FAIL : IxAtmm could not update the port because the
 *       inputs are not understood, or the interface was called before
 * the port was initialized.  */
PUBLIC IX_STATUS
ixAtmmPortModify (IxAtmLogicalPort port,
		  unsigned txPortRate,
		  unsigned rxPortRate);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmPortQuery (IxAtmLogicalPort port,
		 unsigned *txPortRate,
		 unsigned *rxPortRate);

 *
 * @brief The client may call this interface to request details on
 *          currently registered transmit and receive rates for an ATM
 *          port.
 *
 * @param port @ref IxAtmLogicalPort [in] - Value identifies the port from which the
 *          rate details are requested.
 *
 * @param *txPortRate unsigned [out] - Pointer to a value
 *          which will be filled with the value of the transmit port
 *          rate specified in bits/second.
 *
 * @param *rxPortRate unsigned [out] - Pointer to a value
 *          which will be filled with the value of the receive port
 *          rate specified in bits/second.
 *
 * @return @li IX_SUCCESS : The information requested on the specified
 *       port has been successfully supplied in the output.
 *
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid.  The request is rejected.
 *
 * @return @li IX_ATMM_RET_INVALID_PARAM_PTR : A pointer parameter was
 *       NULL.
 *
 * @return @li IX_FAIL : IxAtmm could not update the port because the
 *       inputs are not understood, or the interface was called before
 *       the port was initialized.  */
PUBLIC IX_STATUS
ixAtmmPortQuery (IxAtmLogicalPort port,
		 unsigned *txPortRate,
		 unsigned *rxPortRate);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmPortEnable(IxAtmLogicalPort port)
 *
 * @brief The client call this interface to enable transmit for an ATM
 *          port. At initialisation, all the ports are disabled.
 *
 * @param port @ref IxAtmLogicalPort [in] - Value identifies the port
 *
 * @return @li IX_SUCCESS : Transmission over this port is started.
 *
 * @return @li IX_FAIL : The port parameter is not valid, or the
 *       port is already enabled
 *
 * @note - When a port is disabled, Rx and Tx VC Connect requests will fail
 *
 * @note - This function uses system resources and should not be used
 *        inside an interrupt context.
 *
 * @sa ixAtmmPortDisable  */
PUBLIC IX_STATUS
ixAtmmPortEnable(IxAtmLogicalPort port);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmPortDisable(IxAtmLogicalPort port)
 *
 * @brief The client call this interface to disable transmit for an ATM
 *          port. At initialisation, all the ports are disabled.
 *
 * @param port @ref IxAtmLogicalPort [in] - Value identifies the port
 *
 * @return @li IX_SUCCESS : Transmission over this port is stopped.
 *
 * @return @li IX_FAIL : The port parameter is not valid, or the
 *       port is already disabled
 *
 * @note - When a port is disabled, Rx and Tx VC Connect requests will fail
 *
 * @note - This function call does not stop RX traffic. It is supposed
 *        that this function is invoked when a serious problem
 *        is detected (e.g. physical layer broken). Then, the RX traffic
 *        is not passing.
 *
 * @note - This function is blocking until the hw acknowledge that the
 *        transmission is stopped.
 *
 * @note - This function uses system resources and should not be used
 *        inside an interrupt context.
 *
 * @sa ixAtmmPortEnable  */
PUBLIC IX_STATUS
ixAtmmPortDisable(IxAtmLogicalPort port);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcRegister (IxAtmLogicalPort port,
		  IxAtmmVc *vcToAdd,
		  IxAtmSchedulerVcId *vcId)
 *
 * @brief This interface is used to register an ATM Virtual
 *         Connection on the specified ATM port.
 *
 *  Each call to this interface registers a unidirectional virtual
 *  connection with the parameters specified.  If a bi-directional VC
 *  is needed, the function should be called twice (once for each
 *  direction, Tx & Rx) where the VPI and VCI and port parameters in
 *  each call are identical.
 *
 *  With the addition of each new VC to a port, a series of
 *  callback functions are invoked by the IxAtmm component to notify
 *  possible external components of the change.  The callback functions
 *  are registered using the @ref ixAtmmVcChangeCallbackRegister interface.
 *
 *  The IxAtmSch component is notified of the registration of transmit
 *  VCs.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies port on which the specified VC is
 *          to be registered.
 *
 * @param *vcToAdd @ref IxAtmmVc [in] -  Pointer to an @ref IxAtmmVc structure
 *          containing a description of the VC to be registered. The
 *          client shall fill the vpi, vci and direction and relevant
 *          trafficDesc members of this structure before calling this
 *          function.
 *
 * @param *vcId @ref IxAtmSchedulerVcId [out] - Pointer to an integer value which is filled
 *              with the per-port unique identifier value for this VC.
 *              This identifier will be required when a request is
 *              made to deregister or change this VC.  VC identifiers
 *              for transmit VCs will have a value between 0-43,
 *              i.e. 32 data Tx VCs + 12 OAM Tx Port VCs.
 *              Receive VCs will have a value between 44-66,
 *              i.e. 32 data Rx VCs + 1 OAM Rx VC.
 *
 * @return @li IX_SUCCESS : The VC has been successfully registered on
 *       this port. The VC is ready for a client to configure IxAtmdAcc
 *       for receive and transmit operations on the VC.
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid or has not been initialized.  The request
 *       is rejected.
 * @return @li IX_ATMM_RET_INVALID_VC_DESCRIPTOR : The descriptor
 *       pointed to by vcToAdd is invalid.  The registration request
 *       is rejected.
 * @return @li IX_ATMM_RET_VC_CONFLICT : The VC requested conflicts with
 *      reserved VPI and/or VCI values or with another VC already activated
 *      on this port.
 * @return @li IX_ATMM_RET_PORT_CAPACITY_IS_FULL : The VC cannot be
 *       registered in the port becuase the port capacity is
 *       insufficient to support the requested ATM traffic contract.
 *       The registration request is rejected.
 * @return @li IX_ATMM_RET_INVALID_PARAM_PTR : A pointer parameter was
 *       NULL.
 *
 * @warning IxAtmm has no capability of signaling or negotiating a virtual
 *          connection. Negotiation of the admission of the VC to the network
 *          is beyond the scope of this function.  This is assumed to be
 *          performed by the calling client, if appropriate,
 *          before or after this function is called.
 */
PUBLIC IX_STATUS
ixAtmmVcRegister (IxAtmLogicalPort port,
		  IxAtmmVc *vcToAdd,
		  IxAtmSchedulerVcId *vcId);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcDeregister (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
 *
 * @brief Function called by a client to deregister a VC from the
 *         system.
 *
 *  With the removal of each new VC from a port, a series of
 *  registered callback functions are invoked by the IxAtmm component
 *  to notify possible external components of the change.  The callback
 *  functions are registered using the @ref ixAtmmVcChangeCallbackRegister.
 *
 *  The IxAtmSch component is notified of the removal of transmit VCs.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies port on which the VC to be
 *          removed is currently registered.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - VC identifier value of the VC to
 *          be deregistered.  This value was supplied to the client when
            the VC was originally registered.  This value can also be
	    queried from the IxAtmm component through the @ref ixAtmmVcQuery
 *          interface.
 *
 * @return @li IX_SUCCESS : The specified VC has been successfully
 *       removed from this port.
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid or has not been initialized.  The request
 *       is rejected.
 * @return @li IX_FAIL : There is no registered VC associated with the
 *       supplied identifier registered on this port. */
PUBLIC IX_STATUS
ixAtmmVcDeregister (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcQuery (IxAtmLogicalPort port,
	       unsigned vpi,
	       unsigned vci,
	       IxAtmmVcDirection direction,
	       IxAtmSchedulerVcId *vcId,
	       IxAtmmVc *vcDesc)
 *
 * @brief This interface supplies information about an active VC on a
 *         particular port when supplied with the VPI, VCI and
 *         direction of that VC.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies port on which the VC to be
 *          queried is currently registered.
 *
 * @param vpi unsigned [in] - ATM VPI value of the requested VC.
 *
 * @param vci unsigned [in] - ATM VCI value of the requested VC.
 *
 * @param direction @ref IxAtmmVcDirection [in] - One of @ref
 *          IX_ATMM_VC_DIRECTION_TX or @ref IX_ATMM_VC_DIRECTION_RX
 *          indicating the direction (Tx or Rx) of the requested VC.
 *
 * @param *vcId @ref IxAtmSchedulerVcId [out] - Pointer to an integer value which will be
 *              filled with the VC identifier value for the requested
 *              VC (as returned by @ref ixAtmmVcRegister), if it
 *              exists on this port.
 *
 * @param *vcDesc @ref IxAtmmVc [out] - Pointer to an @ref IxAtmmVc structure
 *              which will be filled with the specific details of the
 *              requested VC, if it exists on this port.
 *
 * @return @li IX_SUCCESS : The specified VC has been found on this port
 *       and the requested details have been returned.
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid or has not been initialized.  The request
 *       is rejected.
 * @return @li IX_ATMM_RET_NO_SUCH_VC : No VC exists on the specified
 *       port which matches the search criteria (VPI, VCI, direction)
 *       given.  No data is returned.
 * @return @li IX_ATMM_RET_INVALID_PARAM_PTR : A pointer parameter was
 *       NULL.
 *
 */
PUBLIC IX_STATUS
ixAtmmVcQuery (IxAtmLogicalPort port,
	       unsigned vpi,
	       unsigned vci,
	       IxAtmmVcDirection direction,
	       IxAtmSchedulerVcId *vcId,
	       IxAtmmVc *vcDesc);


/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcIdQuery (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId, IxAtmmVc *vcDesc)
 *
 * @brief This interface supplies information about an active VC on a
 *         particular port when supplied with a vcId for that VC.
 *
 * @param port @ref IxAtmLogicalPort [in] - Identifies port on which the VC to be
 *          queried is currently registered.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - Value returned by @ref ixAtmmVcRegister which
 *          uniquely identifies the requested VC on this port.
 *
 * @param *vcDesc @ref IxAtmmVc [out] - Pointer to an @ref IxAtmmVc structure
 *              which will be filled with the specific details of the
 *              requested VC, if it exists on this port.
 *
 * @return @li IX_SUCCESS : The specified VC has been found on this port
 *       and the requested details have been returned.
 * @return @li IX_ATMM_RET_INVALID_PORT : The port value indicated in the
 *       input is not valid or has not been initialized.  The request
 *       is rejected.
 * @return @li IX_ATMM_RET_NO_SUCH_VC : No VC exists on the specified
 *       port which matches the supplied identifier.  No data is
 *       returned.
 * @return @li IX_ATMM_RET_INVALID_PARAM_PTR : A pointer parameter was
 *       NULL.
 */
PUBLIC IX_STATUS
ixAtmmVcIdQuery (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId, IxAtmmVc *vcDesc);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcChangeCallbackRegister (IxAtmmVcChangeCallback callback)
 *
 * @brief This interface is invoked to supply a function to IxAtmm
 *         which will be called to notify the client if a new VC is
 *         registered with IxAtmm or an existing VC is removed.
 *
 * The callback, when invoked, will run within the context of the call
 * to @ref ixAtmmVcRegister or @ref ixAtmmVcDeregister which caused
 * the change of state.
 *
 * A maximum of 32 calbacks may be registered in with IxAtmm.
 *
 * @param callback @ref IxAtmmVcChangeCallback [in] - Callback which complies
 *          with the @ref IxAtmmVcChangeCallback definition.  This
 *          function will be invoked by IxAtmm with the appropiate
 *          parameters for the relevant VC when any VC has been
 *          registered or deregistered with IxAtmm.
 *
 * @return @li IX_SUCCESS : The specified callback has been registered
 *      successfully with IxAtmm and will be invoked when appropriate.
 * @return @li IX_FAIL : Either the supplied callback is invalid, or
 *      IxAtmm has already registered 32 and connot accommodate
 *      any further registrations of this type.  The request is
 *      rejected.
 *
 * @warning The client must not call either the @ref
 *          ixAtmmVcRegister or @ref ixAtmmVcDeregister interfaces
 *          from within the supplied callback function.  */
PUBLIC IX_STATUS ixAtmmVcChangeCallbackRegister (IxAtmmVcChangeCallback callback);


/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmVcChangeCallbackDeregister (IxAtmmVcChangeCallback callback)
 *
 * @brief This interface is invoked to deregister a previously supplied
 *         callback function.
 *
 * @param callback @ref IxAtmmVcChangeCallback [in] - Callback which complies
 *          with the @ref IxAtmmVcChangeCallback definition.  This
 *          function will removed from the table of callbacks.
 *
 * @return @li IX_SUCCESS : The specified callback has been deregistered
 *      successfully from IxAtmm.
 * @return @li IX_FAIL : Either the supplied callback is invalid, or
 *      is not currently registered with IxAtmm.
 */
PUBLIC IX_STATUS
ixAtmmVcChangeCallbackDeregister (IxAtmmVcChangeCallback callback);

/**    
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmUtopiaStatusShow (void)
 * 
 *  @brief Display utopia status counters
 *
 * @param "none"
 *
 * @return @li IX_SUCCESS : Show function was successful
 * @return @li IX_FAIL : Internal failure
 */
PUBLIC IX_STATUS
ixAtmmUtopiaStatusShow (void);

/**     
 * @ingroup IxAtmm
 * 
 * @fn ixAtmmUtopiaCfgShow (void)
 *
 * @brief Display utopia information(config registers and status registers)
 *
 * @param "none"
 *
 * @return @li IX_SUCCESS : Show function was successful
 * @return @li IX_FAIL : Internal failure
 */
PUBLIC IX_STATUS
ixAtmmUtopiaCfgShow (void);

#endif
/* IXATMM_H */

/** @} */
