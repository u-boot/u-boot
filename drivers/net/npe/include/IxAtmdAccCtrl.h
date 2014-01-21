/**
 * @file    IxAtmdAccCtrl.h
 *
 * @date    20-Mar-2002
 *
 * @brief IxAtmdAcc Public API
 *
 * This file contains the public API of IxAtmdAcc, related to the
 * control functions of the component.
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
 * @defgroup IxAtmdAccCtrlAPI IXP400 ATM Driver Access (IxAtmdAcc) Control API
 *
 * @brief The public API for the IXP400 Atm Driver Control component
 *
 * IxAtmdAcc is the low level interface by which AAL PDU get transmitted
 * to,and received from the Utopia bus
 *
 * This part is related to the Control configuration
 *
 * @{
 */

#ifndef IXATMDACCCTRL_H
#define IXATMDACCCTRL_H

#include "IxAtmdAcc.h"

/* ------------------------------------------------------
   AtmdAccCtrl Data Types definition
   ------------------------------------------------------ */

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @def IX_ATMDACC_PORT_DISABLE_IN_PROGRESS
*
* @brief Port enable return code
*
* This constant is used to tell IxAtmDAcc user that the port disable
* functions are not complete. The user can call ixAtmdAccPortDisableComplete()
* to find out when the disable has finished. The port enable can then proceed.
*
*/
#define IX_ATMDACC_PORT_DISABLE_IN_PROGRESS 5

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @def IX_ATMDACC_ALLPDUS
*
* @brief All PDUs
*
* This constant is used to tell IxAtmDAcc to process all PDUs from
* the RX queue or the TX Done
*
* @sa IxAtmdAccRxDispatcher
* @sa IxAtmdAccTxDoneDispatcher
*
*/
#define IX_ATMDACC_ALLPDUS 0xffffffff

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to RX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @brief Callback prototype for notification of available PDUs for
 * an Rx Q.
 *
 * This a protoype for a function which is called when there is at
 * least one Pdu available for processing on a particular Rx Q.
 *
 * This function should call @a ixAtmdAccRxDispatch() with
 * the aprropriate number of parameters to read and process the Rx Q.
 *
 * @sa ixAtmdAccRxDispatch
 * @sa ixAtmdAccRxVcConnect
 * @sa ixAtmdAccRxDispatcherRegister
 *
 * @param rxQueueId @ref IxAtmRxQueueId [in] indicates which RX queue to has Pdus to process.
 * @param numberOfPdusToProcess unsigned int [in] indicates the minimum number of
 *        PDUs available to process all PDUs from the queue.
 * @param reservedPtr unsigned int* [out] pointer to a int location which can
 *        be written to, but does not retain written values. This is
 *        provided to make this prototype compatible
 *        with @a ixAtmdAccRxDispatch()
 *
 * @return @li int - ignored.
 *
 */
typedef IX_STATUS (*IxAtmdAccRxDispatcher) (IxAtmRxQueueId rxQueueId,
                         unsigned int numberOfPdusToProcess,
                         unsigned int *reservedPtr);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to TX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @brief Callback prototype for transmitted mbuf when threshold level is
 *        crossed.
 *
 * IxAtmdAccTxDoneDispatcher is the prototype of the user function
 * which get called when pdus are completely transmitted. This function
 * is likely to call the @a ixAtmdAccTxDoneDispatch() function.
 *
 * This function is called when the number of available pdus for
 * reception is crossing the threshold level as defined
 * in @a ixAtmdAccTxDoneDispatcherRegister()
 *
 * This function is called inside an Qmgr dispatch context. No system
 * resource or interrupt-unsafe feature should be used inside this
 * callback.
 *
 * Transmitted buffers recycling implementation is a sytem-wide mechanism
 * and needs to be set before any traffic is started. If this threshold
 * mechanism is not used, the user is responsible for polling the
 * transmitted buffers with @a ixAtmdAccTxDoneDispatch()
 * and @a ixAtmdAccTxDoneLevelQuery() functions.
 *
 * @sa ixAtmdAccTxDoneDispatcherRegister
 * @sa ixAtmdAccTxDoneDispatch
 * @sa ixAtmdAccTxDoneLevelQuery
 *
 * @param numberOfPdusToProcess unsigned int [in] - The current number of pdus currently
 *        available for recycling
 * @param *reservedPtr unsigned int [out] - pointer to a int location which can be
 *        written to but does not retain written values. This is provided
 *        to make this prototype compatible
 *        with @a ixAtmdAccTxDoneDispatch()
 *
 * @return @li IX_SUCCESS This is provided to make
 *    this prototype compatible with @a ixAtmdAccTxDoneDispatch()
 * @return @li IX_FAIL invalid parameters or some unspecified internal
 *    error occured. This is provided to make
 *    this prototype compatible with @a ixAtmdAccTxDoneDispatch()
 *
 */
typedef IX_STATUS (*IxAtmdAccTxDoneDispatcher) (unsigned int numberOfPdusToProcess,
                                                unsigned int *reservedPtr);

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @brief Notification that the threshold number of scheduled cells
* remains in a port's transmit Q.
*
* The is the prototype for of the user notification function which
* gets called on a per-port basis, when the number of remaining
* scheduled cells to be transmitted decreases to the threshold level.
* The number of cells passed as a parameter can be used for scheduling
* purposes as the maximum number of cells that can be passed in a
* schedule table to the @a ixAtmdAccPortTxProcess() function.
*
* @sa ixAtmdAccPortTxCallbackRegister
* @sa ixAtmdAccPortTxProcess
* @sa ixAtmdAccPortTxFreeEntriesQuery
*
* @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
* @param numberOfAvailableCells unsigned int [in] - number of available
*        cell entries.for the port
*
* @note - This functions shall not use system resources when used
*         inside an interrupt context.
*
*/
typedef void (*IxAtmdAccPortTxLowCallback) (IxAtmLogicalPort port,
                           unsigned int numberOfAvailableCells);

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @brief  Prototype to submit cells for transmission
*
* IxAtmdAccTxVcDemandUpdateCallback is the prototype of the callback
* function used by AtmD to notify an ATM Scheduler that the user of
* a VC has submitted cells for transmission.
*
* @sa IxAtmdAccTxVcDemandUpdateCallback
* @sa IxAtmdAccTxVcDemandClearCallback
* @sa IxAtmdAccTxSchVcIdGetCallback
* @sa ixAtmdAccPortTxScheduledModeEnable
*
* @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the VC to be updated
*        is established
* @param vcId int [in] - Identifies the VC to be updated. This is the value
*        returned by the @a IxAtmdAccTxSchVcIdGetCallback() call .
* @param numberOfCells unsigned int [in] - Indicates how many ATM cells should be added
*        to the queue for this VC.
*
* @return @li IX_SUCCESS the function is registering the cell demand for
*        this VC.
* @return @li IX_FAIL the function cannot register cell for this VC : the
*         scheduler maybe overloaded or misconfigured
*
*/
typedef IX_STATUS (*IxAtmdAccTxVcDemandUpdateCallback) (IxAtmLogicalPort port,
                        int vcId,
                        unsigned int numberOfCells);

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @brief  prototype to  remove all currently queued cells from a
* registered VC
*
* IxAtmdAccTxVcDemandClearCallback is the prototype of the function
* to remove all currently queued cells from a registered VC. The
* pending cell count for the specified VC is reset to zero. After the
* use of this callback, the scheduler shall not schedule more cells
* for this VC.
*
* This callback function is called during a VC disconnection
* @a ixAtmdAccTxVcTryDisconnect()
*
* @sa IxAtmdAccTxVcDemandUpdateCallback
* @sa IxAtmdAccTxVcDemandClearCallback
* @sa IxAtmdAccTxSchVcIdGetCallback
* @sa ixAtmdAccPortTxScheduledModeEnable
* @sa ixAtmdAccTxVcTryDisconnect
*
* @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the VC to be cleared
*        is established
* @param vcId int [in] - Identifies the VC to be cleared. This is the value
*        returned by the @a IxAtmdAccTxSchVcIdGetCallback() call .
*
* @return none
*
*/
typedef void (*IxAtmdAccTxVcDemandClearCallback) (IxAtmLogicalPort port,
                             int vcId);

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @brief  prototype to get a scheduler vc id
*
* IxAtmdAccTxSchVcIdGetCallback is the prototype of the function to get
* a scheduler vcId
*
* @sa IxAtmdAccTxVcDemandUpdateCallback
* @sa IxAtmdAccTxVcDemandClearCallback
* @sa IxAtmdAccTxSchVcIdGetCallback
* @sa ixAtmdAccPortTxScheduledModeEnable
*
* @param port @ref IxAtmLogicalPort [in] - Specifies the ATM logical port on which the VC is
*        established
* @param vpi unsigned int [in] - For AAL0/AAL5 specifies the ATM vpi on which the 
*                 VC is established.
*                 For OAM specifies the dedicated "OAM Tx channel" VPI.
* @param vci unsigned int [in] - For AAL0/AAL5 specifies the ATM vci on which the 
*                 VC is established.
*                 For OAM specifies the dedicated "OAM Tx channel" VCI.
* @param connId @ref IxAtmConnId [in] - specifies the IxAtmdAcc connection Id already
*        associated with this VC
* @param vcId int* [out] - pointer to a vcId
*
* @return @li IX_SUCCESS the function is returning a Scheduler vcId for this
*         VC
* @return @li IX_FAIL the function cannot process scheduling for this VC.
*                 the contents of vcId is unspecified
*
*/
typedef IX_STATUS (*IxAtmdAccTxSchVcIdGetCallback) (IxAtmLogicalPort port,
                               unsigned int vpi,
                               unsigned int vci,
                               IxAtmConnId connId,
                               int *vcId);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to RX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccRxDispatcherRegister (
                          IxAtmRxQueueId queueId,
                          IxAtmdAccRxDispatcher callback)
 *
 * @brief Register a notification callback to be invoked when there is
 * at least one entry on a particular Rx queue.
 *
 * This function registers a callback to be invoked when there is at
 * least one entry in a particular queue. The registered callback is
 * called every time when the hardware adds one or more pdus to the
 * specified Rx queue.
 *
 * This function cannot be used when a Rx Vc using this queue is
 * already existing.
 *
 * @note -The callback function can be the API function
 *       @a ixAtmdAccRxDispatch() : every time the threhold level
 *       of the queue is reached, the ixAtmdAccRxDispatch() is
 *       invoked to remove all entries from the queue.
 *
 * @sa ixAtmdAccRxDispatch
 * @sa IxAtmdAccRxDispatcher
 *
 * @param queueId @ref IxAtmRxQueueId [in] RX queue identification
 * @param callback @ref IxAtmdAccRxDispatcher [in] function triggering the delivery of incoming
 *        traffic. This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS Successful call to @a ixAtmdAccRxDispatcherRegister()
 * @return @li IX_FAIL error in the parameters, or there is an
 *             already active RX VC for this queue or some unspecified
 *             internal error occurred.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxDispatcherRegister (
                          IxAtmRxQueueId queueId,
                          IxAtmdAccRxDispatcher callback);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccRxDispatch (IxAtmRxQueueId rxQueueId,
    unsigned int numberOfPdusToProcess,
    unsigned int *numberOfPdusProcessedPtr)
 *
 *
 * @brief Control function which executes Rx processing for a particular
 * Rx stream.
 *
 * The @a IxAtmdAccRxDispatch() function is used to process received Pdus
 * available from one of the two incoming RX streams. When this function
 * is invoked, the incoming traffic (up to the number of PDUs passed as
 * a parameter) will be transferred to the IxAtmdAcc users through the
 * callback @a IxAtmdAccRxVcRxCallback(), as registered during the
 * @a ixAtmdAccRxVcConnect() call.
 *
 * The user receive callbacks will be executed in the context of this
 * function.
 *
 * Failing to use this function on a regular basis when there is traffic
 * will block incoming traffic and can result in Pdus being dropped by
 * the hardware.
 *
 * This should be used to control when received pdus are handed off from
 * the hardware to Aal users from a particluar stream. The function can
 * be used from a timer context, or can be registered as a callback in
 * response to an rx stream threshold event, or can be used inside an
 * active polling mechanism which is under user control.
 *
 * @note - The signature of this function is directly compatible with the
 * callback prototype which can be register with @a ixAtmdAccRxDispatcherRegister().
 *
 * @sa ixAtmdAccRxDispatcherRegister
 * @sa IxAtmdAccRxVcRxCallback
 * @sa ixAtmdAccRxVcFreeEntriesQuery
 *
 * @param rxQueueId @ref IxAtmRxQueueId [in] - indicates which RX queue to process.
 * @param numberOfPdusToProcess unsigned int [in] - indicates the maxiumum number of PDU to
 *     remove from the RX queue. A value of IX_ATMDACC_ALLPDUS indicates
 *     to process all PDUs from the queue. This includes at least the PDUs
 *     in the queue when the fuction is invoked. Because of real-time
 *     constraints, there is no guarantee thatthe queue will be empty
 *     when the function exits. If this parameter is greater than the
 *     number of entries of the queues, the function will succeed
 *     and the parameter numberOfPdusProcessedPtr will reflect the exact
 *     number of PDUs processed.
 * @param *numberOfPdusProcessedPtr unsigned int [out] - indicates the actual number of PDU
 *     processed during this call. This parameter cannot be a null
 *     pointer.
 *
 * @return @li IX_SUCCESS the number of PDUs as indicated in
 *     numberOfPdusProcessedPtr are removed from the RX queue and the VC callback
 *     are called.
 * @return @li IX_FAIL invalid parameters or some unspecified internal
 *     error occured.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxDispatch (IxAtmRxQueueId rxQueueId,
    unsigned int numberOfPdusToProcess,
    unsigned int *numberOfPdusProcessedPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 * 
 * @fn ixAtmdAccRxLevelQuery (IxAtmRxQueueId rxQueueId,
                     unsigned int *numberOfPdusPtr)
 *
 * @brief Query the number of entries in a particular RX queue.
 *
 * This function is used to retrieve the number of pdus received by
 * the hardware and ready for distribution to users.
 *
 * @param rxQueueId @ref IxAtmRxQueueId [in] - indicates which of two RX queues to query.
 * @param numberOfPdusPtr unsigned int* [out] - Pointer to store the number of available
 *        PDUs in the RX queue. This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS the value in numberOfPdusPtr specifies the
 *         number of incoming pdus waiting in this queue
 * @return @li IX_FAIL an error occurs during processing.
 *         The value in numberOfPdusPtr is unspecified.
 *
 * @note - This function is reentrant, doesn't use system resources
 *         and can be used from an interrupt context.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxLevelQuery (IxAtmRxQueueId rxQueueId,
                     unsigned int *numberOfPdusPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccRxQueueSizeQuery (IxAtmRxQueueId rxQueueId,
                     unsigned int *numberOfPdusPtr)
 *
 * @brief Query the size of a particular RX queue.
 *
 * This function is used to retrieve the number of pdus the system is
 * able to queue when reception is complete.
 *
 * @param rxQueueId @ref IxAtmRxQueueId [in] - indicates which of two RX queues to query.
 * @param numberOfPdusPtr unsigned int* [out] - Pointer to store the number of pdus
 *         the system is able to queue in the RX queue. This parameter
 *         cannot be a null pointer.
 *
 * @return @li IX_SUCCESS the value in numberOfPdusPtr specifies the
 *         number of pdus the system is able to queue.
 * @return @li IX_FAIL an error occurs during processing.
 *         The value in numberOfPdusPtr is unspecified.
 *
 * @note - This function is reentrant, doesn't use system resources
 *         and can be used from an interrupt context.
 *
 */
PUBLIC IX_STATUS ixAtmdAccRxQueueSizeQuery (IxAtmRxQueueId rxQueueId,
                     unsigned int *numberOfPdusPtr);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to TX traffic
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccPortTxFreeEntriesQuery (IxAtmLogicalPort port,
                         unsigned int *numberOfCellsPtr)
 *
 * @brief Get the number of available cells the system can accept for
 *       transmission.
 *
 * The function is used to retrieve the number of cells that can be
 * queued for transmission to the hardware.
 *
 * This number is based on the worst schedule table where one cell
 * is stored in one schedule table entry, depending on the pdus size
 * and mbuf size and fragmentation.
 *
 * This function doesn't use system resources and can be used from a
 * timer context, or can be associated with a threshold event, or can
 * be used inside an active polling mechanism
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param numberOfCellsPtr unsigned int* [out] - number of available cells.
 *                   This parameter cannot be a null pointer.
 *
 * @sa ixAtmdAccPortTxProcess
 *
 * @return @li IX_SUCCESS numberOfCellsPtr contains the number of cells that can be scheduled
 *         for this port.
 * @return @li IX_FAIL error in the parameters, or some processing error
 *         occured.
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortTxFreeEntriesQuery (IxAtmLogicalPort port,
                         unsigned int *numberOfCellsPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 * 
 * @fn ixAtmdAccPortTxCallbackRegister (IxAtmLogicalPort port,
                       unsigned int numberOfCells,
                       IxAtmdAccPortTxLowCallback callback)
 *
 * @brief Configure the Tx port threshold value and register a callback to handle
 * threshold notifications.
 *
 * This function sets the threshold in cells
 *
 * @sa ixAtmdAccPortTxCallbackRegister
 * @sa ixAtmdAccPortTxProcess
 * @sa ixAtmdAccPortTxFreeEntriesQuery
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param numberOfCells unsigned int [in] - threshold value which triggers the callback
 *        invocation, This number has to be one of the
 *        values 0,1,2,4,8,16,32 ....
 *        The maximum value cannot be more than half of the txVc queue
 *        size (which can be retrieved using @a ixAtmdAccPortTxFreeEntriesQuery()
 *        before any Tx traffic is sent for this port)
 * @param callback @ref IxAtmdAccPortTxLowCallback [in] - callback function to invoke when the threshold
 *                 level is reached.
 *                 This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS Successful call to @a ixAtmdAccPortTxCallbackRegister()
 * @return @li IX_FAIL error in the parameters, Tx channel already set for this port
 *             threshold level is not correct or within the range regarding the
 *             queue size:or unspecified error during processing:
 *
 * @note - This callback function get called when the threshold level drops from
 *         (numberOfCells+1) cells to (numberOfCells) cells
 *
 * @note - This function should be called during system initialisation,
 *         outside an interrupt context
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortTxCallbackRegister (IxAtmLogicalPort port,
                       unsigned int numberOfCells,
                       IxAtmdAccPortTxLowCallback callback);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccPortTxScheduledModeEnable (IxAtmLogicalPort port,
    IxAtmdAccTxVcDemandUpdateCallback vcDemandUpdateCallback,
    IxAtmdAccTxVcDemandClearCallback vcDemandClearCallback,
    IxAtmdAccTxSchVcIdGetCallback vcIdGetCallback)
 *
 * @brief Put the port into Scheduled Mode
 *
 * This function puts the specified port into scheduled mode of
 * transmission which means an external s/w entity controls the
 * transmission of cells on this port. This faciltates traffic shaping on
 * the port.
 *
 * Any buffers submitted on a VC for this port will be queued in IxAtmdAcc.
 * The transmission of these buffers to and by the hardware will be driven
 * by a transmit schedule submitted regulary in calls to
 * @a ixAtmdAccPortTxProcess() by traffic shaping entity.
 *
 * The transmit schedule is expected to be dynamic in nature based on
 * the demand in cells for each VC on the port. Hence the callback
 * parameters provided to this function allow IxAtmdAcc to inform the
 * shaping entity of demand changes for each VC on the port.
 *
 * By default a port is in Unscheduled Mode so if this function is not
 * called, transmission of data is done without sheduling rules, on a
 * first-come, first-out basis.
 *
 * Once a port is put in scheduled mode it cannot be reverted to
 * un-scheduled mode. Note that unscheduled mode is not supported
 * in ixp425 1.0
 *
 * @note - This function should be called before any VCs have be
 * connected on a port. Otherwise this function call will return failure.
 *
 * @note - This function uses internal locks and should not be called from
 * an interrupt context
 *
 * @sa IxAtmdAccTxVcDemandUpdateCallback
 * @sa IxAtmdAccTxVcDemandClearCallback
 * @sa IxAtmdAccTxSchVcIdGetCallback
 * @sa ixAtmdAccPortTxProcess
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param vcDemandUpdateCallback @ref IxAtmdAccTxVcDemandUpdateCallback [in] - callback function used to update
 *     the number of outstanding cells for transmission. This parameter
 *     cannot be a null pointer.
 * @param vcDemandClearCallback @ref IxAtmdAccTxVcDemandClearCallback [in] - callback function used to remove all
 *     clear the number of outstanding cells for a VC. This parameter
 *     cannot be a null pointer.
 * @param vcIdGetCallback @ref IxAtmdAccTxSchVcIdGetCallback [in] - callback function used to exchange vc
 *     Identifiers between IxAtmdAcc and the entity supplying the
 *     transmit schedule. This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS scheduler registration is complete and the port
 *         is now in scheduled mode.
 * @return @li IX_FAIL failed (wrong parameters, or traffic is already
 *         enabled on this port, possibly without ATM shaping)
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortTxScheduledModeEnable (IxAtmLogicalPort port,
    IxAtmdAccTxVcDemandUpdateCallback vcDemandUpdateCallback,
    IxAtmdAccTxVcDemandClearCallback vcDemandClearCallback,
    IxAtmdAccTxSchVcIdGetCallback vcIdGetCallback);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccPortTxProcess (IxAtmLogicalPort port,
    IxAtmScheduleTable* scheduleTablePtr)
 *
 * @brief Transmit queue cells to the H/W based on the supplied schedule
 *        table.
 *
 * This function @a ixAtmdAccPortTxProcess() process the schedule
 * table provided as a parameter to the function. As a result cells are
 * sent to the underlaying hardware for transmission.
 *
 * The schedule table is executed in its entirety or not at all. So the
 * onus is on the caller not to submit a table containing more cells than
 * can be transmitted at that point. The maximum numbers that can be
 * transmitted is guaranteed to be the number of cells as returned by the
 * function @a ixAtmdAccPortTxFreeEntriesQuery().
 *
 * When the scheduler is invoked on a threshold level, IxAtmdAcc gives the
 * minimum number of cells (to ensure the callback will fire again later)
 * and the maximum number of cells that @a ixAtmdAccPortTxProcess()
 * will be able to process (assuming the ATM scheduler is able
 * to produce the worst-case schedule table, i.e. one entry per cell).
 *
 * When invoked ouside a threshold level, the overall number of cells of
 * the schedule table should be less than the number of cells returned
 * by the @a ixAtmdAccPortTxFreeEntriesQuery() function.
 *
 * After invoking the @a ixAtmdAccPortTxProcess() function, it is the
 * user choice to query again the queue level with the function
 * @a ixAtmdAccPortTxFreeEntriesQuery() and, depending on a new cell
 * number, submit an other schedule table.
 *
 * IxAtmdAcc will check that the number of cells in the schedule table
 * is compatible with the current transmit level. If the
 *
 * Obsolete or invalid connection Id will be silently discarded.
 *
 * This function is not reentrant for the same port.
 *
 * This functions doesn't use system resources and can be used inside an
 * interrupt context.
 *
 * This function is used as a response to the hardware requesting more
 * cells to transmit.
 *
 * @sa ixAtmdAccPortTxScheduledModeEnable
 * @sa ixAtmdAccPortTxFreeEntriesQuery
 * @sa ixAtmdAccPortTxCallbackRegister
 * @sa ixAtmdAccPortEnable
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 * @param scheduleTablePtr @ref IxAtmScheduleTable* [in] - pointer to a scheduler update table. The
 *     content of this table is not modified by this function. This
 *     parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS the schedule table process is complete
 *             and cells are transmitted to the hardware
 * @return @li IX_ATMDACC_WARNING : Traffic will be dropped: the schedule table exceed
 *     the hardware capacity  If this error is ignored, further traffic
 *     and schedule will work correctly.
 *     Overscheduling does not occur when the schedule table does
 *     not contain more entries that the number of free entries returned
 *     by @a ixAtmdAccPortTxFreeEntriesQuery().
 *     However, Disconnect attempts just after this error will fail permanently
 *     with the error code @a IX_ATMDACC_RESOURCES_STILL_ALLOCATED, and it is
 *     necessary to disable the port to make @a ixAtmdAccTxVcTryDisconnect()
 *     successful.
 * @return @li IX_FAIL a wrong parameter is supplied, or the format of
 *     the schedule table is invalid, or the port is not Enabled, or
 *     an internal severe error occured. No cells is transmitted to the hardware
 *
 * @note - If the failure is linked to an overschedule of data cells
 *     the result is an inconsistency in the output traffic (one or many
 *     cells may be missing and the traffic contract is not respected).
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortTxProcess (IxAtmLogicalPort port,
    IxAtmScheduleTable* scheduleTablePtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccTxDoneDispatch (unsigned int numberOfPdusToProcess,
                unsigned int *numberOfPdusProcessedPtr)
 *
 * @brief Process a number of pending transmit done pdus from the hardware.
 *
 * As a by-product of Atm transmit operation buffers which transmission
 * is complete need to be recycled to users. This function is invoked
 * to service the oustanding list of transmitted buffers and pass them
 * to VC users.
 *
 * Users are handed back pdus by invoking the free callback registered
 * during the @a ixAtmdAccTxVcConnect() call.
 *
 * There is a single Tx done stream servicing all active Atm Tx ports
 * which can contain a maximum of 64 entries. If this stream fills port
 * transmission will stop so this function must be call sufficently
 * frequently to ensure no disruption to the transmit operation.
 *
 * This function can be used from a timer context, or can be associated
 * with a TxDone level threshold event (see @a ixAtmdAccTxDoneDispatcherRegister() ),
 * or can be used inside an active polling mechanism under user control.
 *
 * For ease of use the signature of this function is compatible with the
 * TxDone threshold event callback prototype.
 *
 * This functions can be used inside an interrupt context.
 *
 * @sa ixAtmdAccTxDoneDispatcherRegister
 * @sa IxAtmdAccTxVcBufferReturnCallback
 * @sa ixAtmdAccTxDoneLevelQuery
 *
 * @param numberOfPdusToProcess unsigned int [in] - maxiumum number of pdus to remove
 *     from the TX Done queue
 * @param *numberOfPdusProcessedPtr unsigned int [out] - number of pdus removed from
 *     the TX Done queue. This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS the number of pdus as indicated in
 *     numberOfPdusToProcess are removed from the TX Done hardware
 *     and passed to the user through the Tx Done callback registered
 *     during a call to @a ixAtmdAccTxVcConnect()
 * @return @li IX_FAIL invalid parameters or numberOfPdusProcessedPtr is
 *     a null pointer or some unspecified internal error occured.
 *
 */
PUBLIC IX_STATUS
ixAtmdAccTxDoneDispatch (unsigned int numberOfPdusToProcess,
                unsigned int *numberOfPdusProcessedPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 * 
 * @fn ixAtmdAccTxDoneLevelQuery (unsigned int *numberOfPdusPtr)
 *
 * @brief Query the current number of transmit pdus ready for
 *        recycling.
 *
 * This function is used to get the number of transmitted pdus which
 * the hardware is ready to hand back to user.
 *
 * This function can be used from a timer context, or can be associated
 * with a threshold event, on can be used inside an active polling
 * mechanism
 *
 * @sa ixAtmdAccTxDoneDispatch
 *
 * @param *numberOfPdusPtr unsigned int [out] - Pointer to the number of pdus transmitted
 *        at the time of this function call, and ready for recycling
 *        This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS numberOfPdusPtr contains the number of pdus
 *        ready for recycling at the time of this function call
 *
 * @return @li IX_FAIL wrong parameter (null pointer as parameter).or
 *         unspecified rocessing error occurs..The value in numberOfPdusPtr
 *         is unspecified.
 *
 */
PUBLIC IX_STATUS
ixAtmdAccTxDoneLevelQuery (unsigned int *numberOfPdusPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccTxDoneQueueSizeQuery (unsigned int *numberOfPdusPtr)
 *
 * @brief Query the TxDone queue size.
 *
 * This function is used to get the number of pdus which
 * the hardware is able to store after transmission is complete
 *
 * The returned value can be used to set a threshold and enable
 * a callback to be notified when the number of pdus is going over
 * the threshold.
 *
 * @sa ixAtmdAccTxDoneDispatcherRegister
 *
 * @param *numberOfPdusPtr unsigned int [out] - Pointer to the number of pdus the system
 *        is able to queue after transmission
 *
 * @return @li IX_SUCCESS numberOfPdusPtr contains the the number of
 *        pdus the system is able to queue after transmission
 * @return @li IX_FAIL wrong parameter (null pointer as parameter).or
 *         unspecified rocessing error occurs..The value in numberOfPdusPtr
 *         is unspecified.
 *
 * @note - This function is reentrant, doesn't use system resources
 *         and can be used from an interrupt context.
 */
PUBLIC IX_STATUS
ixAtmdAccTxDoneQueueSizeQuery (unsigned int *numberOfPdusPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 * 
 * @fn ixAtmdAccTxDoneDispatcherRegister (unsigned int numberOfPdus,
   IxAtmdAccTxDoneDispatcher notificationCallback)
 *
 * @brief Configure the Tx Done stream threshold value and register a
 * callback to handle threshold notifications.
 *
 * This function sets the threshold level in term of number of pdus at
 * which the supplied notification function should be called.
 *
 * The higher the threshold value is, the less events will be necessary
 * to process transmitted buffers.
 *
 * Transmitted buffers recycling implementation is a sytem-wide mechanism
 * and needs to be set prior any traffic is started. If this threshold
 * mechanism is not used, the user is responsible for polling the
 * transmitted buffers thanks to @a ixAtmdAccTxDoneDispatch() and
 * @a ixAtmdAccTxDoneLevelQuery() functions.
 *
 * This function should be called during system initialisation outside
 * an interrupt context
 *
 * @sa ixAtmdAccTxDoneDispatcherRegister
 * @sa ixAtmdAccTxDoneDispatch
 * @sa ixAtmdAccTxDoneLevelQuery
 *
 * @param numberOfPdus unsigned int [in] - The number of TxDone pdus which triggers the
 *        callback invocation This number has to be a power of 2, one of the
 *        values 0,1,2,4,8,16,32 ...
 *        The maximum value cannot be more than half of the txDone queue
 *        size (which can be retrieved using @a ixAtmdAccTxDoneQueueSizeQuery())
 * @param notificationCallback @ref IxAtmdAccTxDoneDispatcher [in] - The function to invoke. (This
 *        parameter can be @a ixAtmdAccTxDoneDispatch()).This
 *        parameter ust not be a null pointer.
 *
 * @return @li IX_SUCCESS Successful call to ixAtmdAccTxDoneDispatcherRegister
 * @return @li IX_FAIL error in the parameters:
 *
 * @note - The notificationCallback will be called exactly when the threshold level
 *         will increase from (numberOfPdus) to (numberOfPdus+1)
 *
 * @note - If there is no Tx traffic, there is no guarantee that TxDone Pdus will
 *       be released to the user (when txDone level is permanently under the threshold
 *       level. One of the preffered way to return resources to the user is to use
 *       a mix of txDone notifications, used together with a slow
 *       rate timer and an exclusion mechanism protecting from re-entrancy
 *
 * @note - The TxDone threshold will only hand back buffers when the threshold level is
 *      crossed. Setting this threshold to a great number reduce the interrupt rate
 *      and the cpu load, but also increase the number of outstanding mbufs and has
 *      a system wide impact when these mbufs are needed by other components.
 *
 */
PUBLIC IX_STATUS ixAtmdAccTxDoneDispatcherRegister (unsigned int numberOfPdus,
   IxAtmdAccTxDoneDispatcher notificationCallback);

/* ------------------------------------------------------
   Part of the IxAtmdAcc interface related to Utopia config
   ------------------------------------------------------ */

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @defgroup IxAtmdAccUtopiaCtrlAPI IXP400 ATM Driver Access (IxAtmdAcc) Utopia Control API
 *
 * @brief The public API for the IXP400 Atm Driver Control component
 *
 * IxAtmdAcc is the low level interface by which AAL PDU get
 * transmitted to,and received from the Utopia bus
 *
 * This part is related to the UTOPIA configuration.
 *
 * @{
 */

/**
 *
 * @brief Utopia configuration
 *
 * This structure is used to set the Utopia parameters
 * @li contains the values of Utopia registers, to be set during initialisation
 * @li contains debug commands for NPE, to be used during development steps
 *
 * @note - the exact description of all parameters is done in the Utopia reference
 *   documents.
 *
 */
typedef struct
{
    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxConfig_
    * @brief Utopia Tx Config Register
    */
    struct UtTxConfig_
    {

    unsigned int reserved_1:1;  /**< [31] These bits are always 0.*/
    unsigned int txInterface:1;     /**< [30] Utopia Transmit Interface. The following encoding
                                   * is used to set the Utopia Transmit interface as ATM master
                                   * or PHY slave:
                                   * @li 1 - PHY
                                   * @li 0 - ATM
                                   */
    unsigned int txMode:1;      /**< [29] Utopia Transmit Mode. The following encoding is used
    *  to set the Utopia Transmit mode to SPHY or MPHY:
    *  @li 1 - SPHY
    *  @li 0 - MPHY
    */
    unsigned int txOctet:1;  /**< [28] Utopia Transmit cell transfer protocol. Used to set
    * the Utopia cell transfer protocol to Octet-level handshaking.
    * Note this is only applicable in SPHY mode.
    * @li 1 - Octet-handshaking enabled
    * @li 0 - Cell-handshaking enabled
    */
    unsigned int txParity:1;    /**< [27] Utopia Transmit parity enabled when set. TxEvenParity
    * defines the parity format odd/even.
    * @li 1 - Enable Parity generation.
    * @li 0 - ut_op_prty held low.
    */
    unsigned int txEvenParity:1; /**< [26] Utopia Transmit Parity Mode
    * @li 1 - Even Parity Generated.
    * @li 0 - Odd Parity Generated.
    */
    unsigned int txHEC:1; /**< [25] Header Error Check Insertion Mode. Specifies if the transmit
    * cell header check byte is calculated and inserted when set.
    * @li 1 - Generate HEC.
    * @li 0 - Disable HEC generation.
    */
    unsigned int txCOSET:1;    /**< [24] If enabled the HEC is Exclusive-OR'ed with the value 0x55 before
  * being presented on the Utopia bus.
  * @li 1 - Enable HEC ExOR with value 0x55
  * @li 0 - Use generated HEC value.
  */

    unsigned int reserved_2:1;    /**< [23] These bits are always 0
    */
    unsigned int txCellSize:7;    /**< [22:16] Transmit expected cell size. Configures the cell size
    * for the transmit module: Values between 52-64 are valid.
    */
    unsigned int reserved_3:3;  /**< [15:13] These bits are always 0 */
    unsigned int txAddrRange:5;       /**< [12:8] When configured as an ATM master in MPHY mode this
    * register specifies the upper limit of the PHY polling logical
    * range. The number of active PHYs are TxAddrRange + 1.
    */
    unsigned int reserved_4:3;      /**< [7:5] These bits are always 0 */
    unsigned int txPHYAddr:5;     /**< [4:0] When configured as a slave in an MPHY system this register
    * specifies the physical address of the PHY.
    */
    }

    utTxConfig;       /**< Tx config Utopia register */

   /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
   * @struct UtTxStatsConfig_
   * @brief Utopia Tx stats Register
    */
    struct UtTxStatsConfig_
    {

    unsigned int vpi:12;  /**< [31:20] ATM VPI [11:0] OR GFC [3:0] and VPI [7:0]
    @li Note: if VCStatsTxGFC is set to 0 the GFC field is ignored in test. */

    unsigned int vci:16;  /**< [19:4] ATM VCI [15:0] or PHY Address[4] */

    unsigned int pti:3;  /**< [3:1] ATM PTI [2:0] or PHY Address[3:1]
  @li Note: if VCStatsTxPTI is set to 0 the PTI field is ignored in test.
  @li Note: if VCStatsTxEnb is set to 0 only the transmit PHY port
  address as defined by this register is used for ATM statistics [4:0]. */

    unsigned int clp:1;  /**< [0] ATM CLP or PHY Address [0]
  @li Note: if VCStatsTxCLP is set to 0 the CLP field is ignored in test.
  @li Note: if VCStatsTxEnb is set to 0 only the transmit PHY port
  address as defined by this register is used for ATM statistics [4:0]. */
    }

    utTxStatsConfig;       /**< Tx stats config Utopia register */

       /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
       * @struct UtTxDefineIdle_
       * @brief Utopia Tx idle cells Register
    */
    struct UtTxDefineIdle_
    {

    unsigned int vpi:12;  /**< [31:20] ATM VPI [11:0] OR GFC [3:0] and VPI [7:0]
    @li Note: if VCIdleTxGFC is set to 0 the GFC field is ignored in test. */

    unsigned int vci:16;  /**< [19:4] ATM VCI [15:0] */

    unsigned int pti:3;  /**< [3:1] ATM PTI PTI [2:0]
  @li Note: if VCIdleTxPTI is set to 0 the PTI field is ignored in test.*/

    unsigned int clp:1;  /**< [0] ATM CLP [0]
  @li Note: if VCIdleTxCLP is set to 0 the CLP field is ignored in test.*/
    }

    utTxDefineIdle;      /**< Tx idle cell config Utopia register */

      /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
      * @struct UtTxEnableFields_
      * @brief Utopia Tx ienable fields Register
    */
    struct UtTxEnableFields_
    {

    unsigned int defineTxIdleGFC:1;    /**< [31] This register is used to include or exclude the GFC
    field of the ATM header when testing for Idle cells.
    @li 1 - GFC field is valid.
    @li 0 - GFC field ignored.*/

    unsigned int defineTxIdlePTI:1;    /**< [30] This register is used to include or exclude the PTI
    field of the ATM header when testing for Idle cells.
    @li 1 - PTI field is valid
    @li    0 - PTI field ignored.*/

    unsigned int defineTxIdleCLP:1;    /**< [29] This register is used to include or
    exclude the CLP field of the ATM header when testing for Idle cells.
    @li 1 - CLP field is valid.
    @li 0 - CLP field ignored. */

    unsigned int phyStatsTxEnb:1;    /**< [28] This register is used to enable or disable ATM
  statistics gathering based on the specified PHY address as defined
  in TxStatsConfig register.
  @li 1 - Enable statistics for specified transmit PHY address.
    @li 0 - Disable statistics for specified transmit PHY address. */

    unsigned int vcStatsTxEnb:1;  /**< [27] This register is used to change the ATM
      statistics-gathering mode from the specified logical PHY address
      to a specific VPI/VCI address.
      @li 1 - Enable statistics for specified VPI/VCI address.
      @li 0 - Disable statistics for specified VPI/VCI address */

    unsigned int vcStatsTxGFC:1;  /**< [26] This register is used to include or exclude the GFC
      field of the ATM header when ATM VPI/VCI statistics are enabled.
      GFC is only available at the UNI and uses the first 4-bits of
      the VPI field.
      @li 1 - GFC field is valid
      @li 0 - GFC field ignored.*/

    unsigned int vcStatsTxPTI:1;  /**< [25] This register is used to include or exclude the PTI
      field of the ATM header when ATM VPI/VCI statistics are enabled.
      @li 1 - PTI field is valid
      @li 0 - PTI field ignored.*/

    unsigned int vcStatsTxCLP:1;  /**< [24] This register is used to include or exclude the CLP
      field of the ATM header when ATM VPI/VCI statistics are enabled.
      @li 1 - CLP field is valid
      @li 0 - CLP field ignored. */

    unsigned int reserved_1:3;  /**< [23-21] These bits are always 0 */

    unsigned int txPollStsInt:1;    /**< [20] Enable the assertion of the ucp_tx_poll_sts condition
  where there is a change in polling status.
  @li 1 - ucp_tx_poll_sts asserted whenever there is a change in status
  @li    0 - ucp_tx_poll_sts asserted if ANY transmit PHY is available
  */
    unsigned int txCellOvrInt:1;    /**< [19] Enable TxCellCount overflow CBI Transmit Status condition
      assertion.
      @li 1 - If TxCellCountOvr is set assert the Transmit Status Condition.
      @li 0 - No CBI Transmit Status condition assertion */

    unsigned int txIdleCellOvrInt:1;  /**< [18] Enable TxIdleCellCount overflow Transmit Status Condition
    @li 1 - If TxIdleCellCountOvr is set assert the Transmit Status Condition
      @li 0 - No CBI Transmit Status condition assertion..*/

    unsigned int enbIdleCellCnt:1;    /**< [17] Enable Transmit Idle Cell Count.
    @li 1 - Enable count of Idle cells transmitted.
      @li 0 - No count is maintained. */

    unsigned int enbTxCellCnt:1;    /**< [16] Enable Transmit Valid Cell Count of non-idle/non-error cells
      @li 1 - Enable count of valid cells transmitted- non-idle/non-error
      @li 0 - No count is maintained.*/

    unsigned int reserved_2:16;   /**< [15:0] These bits are always 0 */
    } utTxEnableFields;  /**< Tx enable Utopia register */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxTransTable0_
    * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable0_
    {

    unsigned int phy0:5;  /**< [31-27] Tx Mapping value of logical phy 0 */

    unsigned int phy1:5;  /**< [26-22] Tx Mapping value of logical phy 1 */

    unsigned int phy2:5;  /**< [21-17] Tx Mapping value of logical phy 2 */

    unsigned int reserved_1:1;  /**< [16] These bits are always 0.*/

    unsigned int phy3:5;  /**< [15-11] Tx Mapping value of logical phy 3 */

    unsigned int phy4:5;  /**< [10-6] Tx Mapping value of logical phy 4 */

    unsigned int phy5:5;  /**< [5-1] Tx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utTxTransTable0;  /**< Tx translation table */

  /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
  * @struct UtTxTransTable1_
  * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable1_
    {

    unsigned int phy6:5;  /**< [31-27] Tx Mapping value of logical phy 6 */

    unsigned int phy7:5;  /**< [26-22] Tx Mapping value of logical phy 7 */

    unsigned int phy8:5;  /**< [21-17] Tx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy9:5;  /**< [15-11] Tx Mapping value of logical phy 3 */

    unsigned int phy10:5;   /**< [10-6] Tx Mapping value of logical phy 4 */

    unsigned int phy11:5;   /**< [5-1] Tx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utTxTransTable1;  /**< Tx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxTransTable2_
    * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable2_
    {

    unsigned int phy12:5;   /**< [31-27] Tx Mapping value of logical phy 6 */

    unsigned int phy13:5;   /**< [26-22] Tx Mapping value of logical phy 7 */

    unsigned int phy14:5;   /**< [21-17] Tx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy15:5;   /**< [15-11] Tx Mapping value of logical phy 3 */

    unsigned int phy16:5;   /**< [10-6] Tx Mapping value of logical phy 4 */

    unsigned int phy17:5;   /**< [5-1] Tx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utTxTransTable2;    /**< Tx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxTransTable3_
    * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable3_
    {

    unsigned int phy18:5;   /**< [31-27] Tx Mapping value of logical phy 6 */

    unsigned int phy19:5;   /**< [26-22] Tx Mapping value of logical phy 7 */

    unsigned int phy20:5;   /**< [21-17] Tx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy21:5;   /**< [15-11] Tx Mapping value of logical phy 3 */

    unsigned int phy22:5;   /**< [10-6] Tx Mapping value of logical phy 4 */

    unsigned int phy23:5;   /**< [5-1] Tx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utTxTransTable3;  /**< Tx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxTransTable4_
    * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable4_
    {

    unsigned int phy24:5;   /**< [31-27] Tx Mapping value of logical phy 6 */

    unsigned int phy25:5;   /**< [26-22] Tx Mapping value of logical phy 7 */

    unsigned int phy26:5;   /**< [21-17] Tx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy27:5;   /**< [15-11] Tx Mapping value of logical phy 3 */

    unsigned int phy28:5;   /**< [10-6] Tx Mapping value of logical phy 4 */

    unsigned int phy29:5;   /**< [5-1] Tx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utTxTransTable4;  /**< Tx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxTransTable5_
    * @brief Utopia Tx translation table Register
    */
    struct UtTxTransTable5_
    {

    unsigned int phy30:5;   /**< [31-27] Tx Mapping value of logical phy 6 */

    unsigned int reserved_1:27;     /**< [26-0] These bits are always 0 */

    } utTxTransTable5;  /**< Tx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxConfig_
    * @brief Utopia Rx config Register
    */
    struct UtRxConfig_
    {

    unsigned int rxInterface:1;    /**< [31] Utopia Receive Interface. The following encoding is used
 to set the Utopia Receive interface as ATM master or PHY slave:
 @li 1 - PHY
    @li 0 - ATM */

    unsigned int rxMode:1;      /**< [30] Utopia Receive Mode. The following encoding is used to set
      the Utopia Receive mode to SPHY or MPHY:
      @li 1 - SPHY
    @li 0 - MPHY */

    unsigned int rxOctet:1;  /**< [29] Utopia Receive cell transfer protocol. Used to set the Utopia
  cell transfer protocol to Octet-level handshaking. Note this is only
  applicable in SPHY mode.
  @li 1 - Octet-handshaking enabled
    @li 0 - Cell-handshaking enabled */

    unsigned int rxParity:1;    /**< [28] Utopia Receive Parity Checking enable.
    @li 1 - Parity checking enabled
    @li 0 - Parity checking disabled */

    unsigned int rxEvenParity:1;/**< [27] Utopia Receive Parity Mode
    @li 1 - Check for Even Parity
    @li 0 - Check for Odd Parity.*/

    unsigned int rxHEC:1;    /**< [26] RxHEC    Header Error Check Mode. Enables/disables cell header
    error checking on the received cell header.
    @li 1 - HEC checking enabled
    @li 0 - HEC checking disabled */

    unsigned int rxCOSET:1;  /**< [25] If enabled the HEC is Exclusive-OR'ed with the value 0x55
  before being tested with the received HEC.
  @li 1 - Enable HEC ExOR with value 0x55.
    @li 0 - Use generated HEC value.*/

    unsigned int rxHECpass:1;    /**< [24] Specifies if the incoming cell HEC byte should be transferred
     after optional processing to the NPE2 Coprocessor Bus Interface or
     if it should be discarded.
     @li 1 - HEC maintained 53-byte/UDC cell sent to NPE2.
    @li 0 - HEC discarded 52-byte/UDC cell sent to NPE2 coprocessor.*/

    unsigned int reserved_1:1;    /**< [23] These bits are always 0 */

    unsigned int rxCellSize:7;    /**< [22:16] Receive cell size. Configures the receive cell size.
      Values between 52-64 are valid */

    unsigned int rxHashEnbGFC:1;      /**< [15] Specifies if the VPI field [11:8]/GFC field should be
      included in the Hash data input or if the bits should be padded
      with 1'b0.
      @li 1 - VPI [11:8]/GFC field valid and used in Hash residue calculation.
      @li 0 - VPI [11:8]/GFC field padded with 1'b0 */

    unsigned int rxPreHash:1; /**< [14] Enable Pre-hash value generation. Specifies if the
 incoming cell data should be pre-hashed to allow VPI/VCI header look-up
 in a hash table.
 @li 1 - Pre-hashing enabled
      @li 0 - Pre-hashing disabled */

    unsigned int reserved_2:1;    /**< [13] These bits are always 0 */

    unsigned int rxAddrRange:5;     /**< [12:8] In ATM master, MPHY mode,
     * this register specifies the upper
     * limit of the PHY polling logical range. The number of active PHYs are
     * RxAddrRange + 1.
     */
    unsigned int reserved_3:3;    /**< [7-5] These bits are always 0 .*/
    unsigned int rxPHYAddr:5;     /**< [4:0] When configured as a slave in an MPHY system this register
      * specifies the physical address of the PHY.
      */
    } utRxConfig;  /**< Rx config Utopia register */

      /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
      * @struct UtRxStatsConfig_
      * @brief Utopia Rx stats config Register
    */
    struct UtRxStatsConfig_
    {

    unsigned int vpi:12;  /**< [31:20] ATM VPI    VPI [11:0] OR GFC [3:0] and VPI [7:0]
    @li Note: if VCStatsRxGFC is set to 0 the GFC field is ignored in test. */

    unsigned int vci:16;  /**< [19:4] VCI [15:0] or PHY Address [4] */

    unsigned int pti:3;  /**< [3:1] PTI [2:0] or or PHY Address [3:1]
  @li Note: if VCStatsRxPTI is set to 0 the PTI field is ignored in test.
  @li Note: if VCStatsRxEnb is set to 0 only the PHY port address is used
  for statistics gathering.. */

    unsigned int clp:1;  /**< [0] CLP [0] or PHY Address [0]
  @li Note: if VCStatsRxCLP is set to 0 the CLP field is ignored in test.
  @li Note: if VCStatsRxEnb is set to 0 only the PHY port address is used
  for statistics gathering.. */
    } utRxStatsConfig;  /**< Rx stats config Utopia register */

  /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
  * @struct UtRxDefineIdle_
  * @brief Utopia Rx idle cells config Register
    */
    struct UtRxDefineIdle_
    {

    unsigned int vpi:12;  /**< [31:20] ATM VPI [11:0] OR GFC [3:0] and VPI [7:0]
    @li Note: if VCIdleRxGFC is set to 0 the GFC field is ignored in test. */

    unsigned int vci:16;  /**< [19:4] ATM VCI [15:0] */

    unsigned int pti:3;  /**< [3:1] ATM PTI PTI [2:0]
  @li Note: if VCIdleRxPTI is set to 0 the PTI field is ignored in test.*/

    unsigned int clp:1;  /**< [0] ATM CLP [0]
  @li Note: if VCIdleRxCLP is set to 0 the CLP field is ignored in test.*/
    } utRxDefineIdle;      /**< Rx idle cell config Utopia register */

      /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
      * @struct UtRxEnableFields_
      * @brief Utopia Rx enable Register
    */
    struct UtRxEnableFields_
    {

    unsigned int defineRxIdleGFC:1;/**< [31] This register is used to include or exclude the GFC
 field of the ATM header when testing for Idle cells.
 @li 1 - GFC field is valid.
    @li 0 - GFC field ignored.*/

    unsigned int defineRxIdlePTI:1;/**< [30] This register is used to include or exclude the PTI
 field of the ATM header when testing for Idle cells.
 @li 1 - PTI field is valid.
    @li 0 - PTI field ignored.*/

    unsigned int defineRxIdleCLP:1;/**< [29] This register is used to include or exclude the CLP
 field of the ATM header when testing for Idle cells.
 @li 1 - CLP field is valid.
    @li    0 - CLP field ignored.*/

    unsigned int phyStatsRxEnb:1;/**< [28] This register is used to enable or disable ATM statistics
     gathering based on the specified PHY address as defined in RxStatsConfig
     register.
     @li 1 - Enable statistics for specified receive PHY address.
    @li 0 - Disable statistics for specified receive PHY address.*/

    unsigned int vcStatsRxEnb:1;/**< [27] This register is used to enable or disable ATM statistics
    gathering based on a specific VPI/VCI address.
    @li 1 - Enable statistics for specified VPI/VCI address.
    @li 0 - Disable statistics for specified VPI/VCI address.*/

    unsigned int vcStatsRxGFC:1;/**< [26] This register is used to include or exclude the GFC field
    of the ATM header when ATM VPI/VCI statistics are enabled. GFC is only
    available at the UNI and uses the first 4-bits of the VPI field.
    @li 1 - GFC field is valid.
    @li 0 - GFC field ignored. */

    unsigned int vcStatsRxPTI:1;/**< [25] This register is used to include or exclude the PTI field
    of the ATM header when ATM VPI/VCI statistics are enabled.
    @li 1 - PTI field is valid.
    @li 0 - PTI field ignored.*/

    unsigned int vcStatsRxCLP:1;/**< [24] This register is used to include or exclude the CLP field
    of the ATM header when ATM VPI/VCI statistics are enabled.
    @li 1 - CLP field is valid.
    @li 0 - CLP field ignored. */

    unsigned int discardHecErr:1;/**< [23] Discard cells with an invalid HEC.
     @li 1 - Discard cells with HEC errors
    @li 0 - Cells with HEC errors are passed */

    unsigned int discardParErr:1;/**< [22] Discard cells containing parity errors.
     @li 1 - Discard cells with parity errors
    @li 0 - Cells with parity errors are passed */

    unsigned int discardIdle:1;    /**< [21] Discard Idle Cells based on DefineIdle register values
 @li    1 - Discard IDLE cells
    @li 0 - IDLE cells passed */

    unsigned int enbHecErrCnt:1;/**< [20] Enable Receive HEC Error Count.
    @li 1 - Enable count of received cells containing HEC errors
    @li 0 - No count is maintained. */

    unsigned int enbParErrCnt:1;/**< [19] Enable Parity Error Count
    @li    1 - Enable count of received cells containing Parity errors
    @li 0 - No count is maintained. */

    unsigned int enbIdleCellCnt:1;/**< [18] Enable Receive Idle Cell Count.
      @li 1 - Enable count of Idle cells received.
    @li 0 - No count is maintained.*/

    unsigned int enbSizeErrCnt:1;/**< [17] Enable Receive Size Error Count.
     @li 1 - Enable count of received cells of incorrect size
    @li    0 - No count is maintained. */

    unsigned int enbRxCellCnt:1;/**< [16] Enable Receive Valid Cell Count of non-idle/non-error cells.
    @li    1 - Enable count of valid cells received - non-idle/non-error
    @li 0 - No count is maintained. */

    unsigned int reserved_1:3;    /**< [15:13] These bits are always 0 */

    unsigned int rxCellOvrInt:1;      /**< [12] Enable CBI Utopia Receive Status Condition if the RxCellCount
      register overflows.
      @li 1 - CBI Receive Status asserted.
      @li    0 - No CBI Receive Status asserted.*/

    unsigned int invalidHecOvrInt:1;  /**< [11] Enable CBI Receive Status Condition if the InvalidHecCount
    register overflows.
    @li    1 - CBI Receive Condition asserted.
      @li 0 - No CBI Receive Condition asserted */

    unsigned int invalidParOvrInt:1;  /**< [10] Enable CBI Receive Status Condition if the InvalidParCount
    register overflows
    @li    1 - CBI Receive Condition asserted.
      @li 0 - No CBI Receive Condition asserted */

    unsigned int invalidSizeOvrInt:1;   /**< [9] Enable CBI Receive Status Condition if the InvalidSizeCount
     register overflows.
     @li 1 - CBI Receive Status Condition asserted.
     @li 0 - No CBI Receive Status asserted */

    unsigned int rxIdleOvrInt:1;      /**< [8] Enable CBI Receive Status Condition if the RxIdleCount overflows.
      @li 1 - CBI Receive Condition asserted.
      @li 0 - No CBI Receive Condition asserted */

    unsigned int reserved_2:3;    /**< [7:5] These bits are always 0 */

    unsigned int rxAddrMask:5;    /**< [4:0] This register is used as a mask to allow the user to increase
    the PHY receive address range. The register should be programmed with
    the address-range limit, i.e. if set to 0x3 the address range increases
  to a maximum of 4 addresses. */
    } utRxEnableFields;      /**< Rx enable Utopia register */

      /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
      * @struct UtRxTransTable0_
      * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable0_
    {

    unsigned int phy0:5;  /**< [31-27] Rx Mapping value of logical phy 0 */

    unsigned int phy1:5;  /**< [26-22] Rx Mapping value of logical phy 1 */

    unsigned int phy2:5;  /**< [21-17] Rx Mapping value of logical phy 2 */

    unsigned int reserved_1:1;  /**< [16] These bits are always 0 */

    unsigned int phy3:5;  /**< [15-11] Rx Mapping value of logical phy 3 */

    unsigned int phy4:5;  /**< [10-6] Rx Mapping value of logical phy 4 */

    unsigned int phy5:5;  /**< [5-1] Rx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    }

    utRxTransTable0;  /**< Rx translation table */

  /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
  * @struct UtRxTransTable1_
  * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable1_
    {

    unsigned int phy6:5;  /**< [31-27] Rx Mapping value of logical phy 6 */

    unsigned int phy7:5;  /**< [26-22] Rx Mapping value of logical phy 7 */

    unsigned int phy8:5;  /**< [21-17] Rx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy9:5;  /**< [15-11] Rx Mapping value of logical phy 3 */

    unsigned int phy10:5;   /**< [10-6] Rx Mapping value of logical phy 4 */

    unsigned int phy11:5;   /**< [5-1] Rx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    }

    utRxTransTable1;  /**< Rx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxTransTable2_
    * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable2_
    {

    unsigned int phy12:5;   /**< [31-27] Rx Mapping value of logical phy 6 */

    unsigned int phy13:5;   /**< [26-22] Rx Mapping value of logical phy 7 */

    unsigned int phy14:5;   /**< [21-17] Rx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy15:5;   /**< [15-11] Rx Mapping value of logical phy 3 */

    unsigned int phy16:5;   /**< [10-6] Rx Mapping value of logical phy 4 */

    unsigned int phy17:5;   /**< [5-1] Rx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utRxTransTable2;    /**< Rx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxTransTable3_
    * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable3_
    {

    unsigned int phy18:5;   /**< [31-27] Rx Mapping value of logical phy 6 */

    unsigned int phy19:5;   /**< [26-22] Rx Mapping value of logical phy 7 */

    unsigned int phy20:5;   /**< [21-17] Rx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy21:5;   /**< [15-11] Rx Mapping value of logical phy 3 */

    unsigned int phy22:5;   /**< [10-6] Rx Mapping value of logical phy 4 */

    unsigned int phy23:5;   /**< [5-1] Rx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utRxTransTable3;  /**< Rx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxTransTable4_
    * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable4_
    {

    unsigned int phy24:5;   /**< [31-27] Rx Mapping value of logical phy 6 */

    unsigned int phy25:5;   /**< [26-22] Rx Mapping value of logical phy 7 */

    unsigned int phy26:5;   /**< [21-17] Rx Mapping value of logical phy 8 */

    unsigned int reserved_1:1; /**< [16-0] These bits are always 0 */

    unsigned int phy27:5;   /**< [15-11] Rx Mapping value of logical phy 3 */

    unsigned int phy28:5;   /**< [10-6] Rx Mapping value of logical phy 4 */

    unsigned int phy29:5;   /**< [5-1] Rx Mapping value of logical phy 5 */

    unsigned int reserved_2:1;  /**< [0] These bits are always 0 */
    } utRxTransTable4;  /**< Rx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxTransTable5_
    * @brief Utopia Rx translation table Register
    */
    struct UtRxTransTable5_
    {

    unsigned int phy30:5;   /**< [31-27] Rx Mapping value of logical phy 6 */

    unsigned int reserved_1:27;     /**< [26-0] These bits are always 0 */

    } utRxTransTable5;  /**< Rx translation table */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtSysConfig_
    * @brief NPE setup Register
    */
    struct UtSysConfig_
    {

    unsigned int reserved_1:2;     /**< [31-30] These bits are always 0 */
    unsigned int txEnbFSM:1;    /**< [29] Enables the operation ofthe Utopia Transmit FSM
   * @li 1 - FSM enabled
   * @li 0 - FSM inactive
   */
    unsigned int rxEnbFSM:1;    /**< [28] Enables the operation ofthe Utopia Revieve FSM
   * @li 1 - FSM enabled
   * @li 0 - FSM inactive
   */
    unsigned int disablePins:1;    /**< [27] Disable Utopia interface I/O pins forcing the signals to an
   * inactive state.  Note that this bit is set on reset and must be
   * de-asserted
   * @li 0 - Normal data transfer
   * @li    1 - Utopia interface pins are forced inactive
    */
    unsigned int tstLoop:1;    /**< [26] Test Loop Back Enable.
    * @li Note: For loop back to function RxMode and Tx Mode must both be set
    * to single PHY mode.
    * @li 0 - Loop back
    * @li 1 - Normal operating mode
    */

    unsigned int txReset:1;   /**< [25] Resets the Utopia Coprocessor transmit module to a known state.
    * @li Note: All transmit configuration and status registers will be reset
    * to their reset values.
    * @li 0 - Normal operating mode
    * @li 1 - Reset transmit modules
    */

    unsigned int rxReset:1;   /**< [24] Resets the Utopia Coprocessor receive module to a known state.
    * @li Note: All receive configuration and status registers will be reset
    * to their reset values.
    * @li    0 - Normal operating mode
    * @li 1 - Reset receive modules
    */

    unsigned int reserved_2:24;     /**< [23-0] These bits are always 0 */
    } utSysConfig;  /**< NPE debug config */

}
IxAtmdAccUtopiaConfig;

/**
*
* @brief Utopia status
*
* This structure is used to set/get the Utopia status parameters
* @li contains debug cell counters, to be accessed during a read operation
*
* @note - the exact description of all parameters is done in the Utopia reference
*   documents.
*
*/
typedef struct
{

    unsigned int utTxCellCount;  /**< count of cells transmitted */

    unsigned int utTxIdleCellCount;    /**< count of idle cells transmitted */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtTxCellConditionStatus_
    * @brief Utopia Tx Status Register
    */
    struct UtTxCellConditionStatus_
    {

    unsigned int reserved_1:2;   /**< [31:30] These bits are always 0 */
    unsigned int txFIFO2Underflow:1; /**< [29] This bit is set if 64-byte
                                     * Transmit FIFO2 indicates a FIFO underflow
                                     * error condition.
                                     */
    unsigned int txFIFO1Underflow:1; /**< [28] This bit is set if
                                     * 64-byte Transmit FIFO1 indicates a FIFO
                                     * underflow error condition.
                                     */
    unsigned int txFIFO2Overflow:1;  /**< [27] This bit is set if 64-byte
                                     * Transmit FIFO2 indicates a FIFO overflow
                                     * error condition.
                                     */
    unsigned int txFIFO1Overflow:1; /**< [26] This bit is set if 64-byte
                                    * Transmit FIFO1 indicates a FIFO overflow
                                    * error condition.
                                    */
    unsigned int txIdleCellCountOvr:1; /**< [25] This bit is set if the
                                       * TxIdleCellCount register overflows.
                                       */
    unsigned int txCellCountOvr:1; /**< [24] This bit is set if the
                                   * TxCellCount register overflows
                                   */
    unsigned int reserved_2:24;    /**< [23:0] These bits are always 0 */
    } utTxCellConditionStatus;    /**< Tx cells condition status */

    unsigned int utRxCellCount; /**< count of cell received */
    unsigned int utRxIdleCellCount;    /**< count of idle cell received */
    unsigned int utRxInvalidHECount;     /**< count of invalid cell
                                        * received because of HEC errors
                                        */
    unsigned int utRxInvalidParCount;  /**< count of invalid cell received
                                        * because of parity errors
                                        */
    unsigned int utRxInvalidSizeCount;  /**< count of invalid cell
                                        * received because of cell
                                        * size errors
                                        */

    /**
    * @ingroup IxAtmdAccUtopiaCtrlAPI
    * @struct UtRxCellConditionStatus_
    * @brief Utopia Rx Status Register
    */
    struct UtRxCellConditionStatus_
    {

    unsigned int reserved_1:3;  /**< [31:29] These bits are always 0.*/
    unsigned int rxCellCountOvr:1;      /**< [28] This bit is set if the RxCellCount register overflows. */
    unsigned int invalidHecCountOvr:1;    /**< [27] This bit is set if the InvalidHecCount register overflows.*/
    unsigned int invalidParCountOvr:1;    /**< [26] This bit is set if the InvalidParCount register overflows.*/
    unsigned int invalidSizeCountOvr:1;    /**< [25] This bit is set if the InvalidSizeCount register overflows.*/
    unsigned int rxIdleCountOvr:1;      /**< [24] This bit is set if the RxIdleCount register overflows.*/
    unsigned int reserved_2:4;  /**< [23:20] These bits are always 0 */
    unsigned int rxFIFO2Underflow:1;  /**< [19] This bit is set if 64-byte Receive FIFO2
                                      * indicates a FIFO underflow error condition.
                                      */
    unsigned int rxFIFO1Underflow:1;  /**< [18] This bit is set if 64-byte Receive
                                      * FIFO1 indicates a FIFO underflow error condition
                                    . */
    unsigned int rxFIFO2Overflow:1;    /**< [17] This bit is set if 64-byte Receive FIFO2
                                       * indicates a FIFO overflow error condition.
                                        */
    unsigned int rxFIFO1Overflow:1;    /**< [16] This bit is set if 64-byte Receive FIFO1
                                    * indicates a FIFO overflow error condition.
                                    */
    unsigned int reserved_3:16;      /**< [15:0] These bits are always 0. */
    } utRxCellConditionStatus;    /**< Rx cells condition status */

} IxAtmdAccUtopiaStatus;

/**
 * @} defgroup IxAtmdAccUtopiaCtrlAPI
 */

 /**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccUtopiaConfigSet (const IxAtmdAccUtopiaConfig *
                        ixAtmdAccUtopiaConfigPtr)
 *
 * @brief Send the configuration structure to the Utopia interface
 *
 * This function downloads the @a IxAtmdAccUtopiaConfig structure to
 * the Utopia and has the following effects
 *  @li setup the Utopia interface
 *  @li initialise the NPE
 *  @li reset the Utopia cell counters and status registers to known values
 *
 * This action has to be done once at initialisation. A lock is preventing
 * the concurrent use of @a ixAtmdAccUtopiaStatusGet() and
 * @A ixAtmdAccUtopiaConfigSet()
 *
 * @param *ixAtmdAccNPEConfigPtr @ref IxAtmdAccUtopiaConfig [in] - pointer to a structure to download to
 *  Utopia. This parameter cannot be a null pointer.
 *
 * @return @li IX_SUCCESS successful download
 * @return @li IX_FAIL error in the parameters, or configuration is not
 *         complete or failed
 *
 * @sa ixAtmdAccUtopiaStatusGet
 *
 */
PUBLIC IX_STATUS ixAtmdAccUtopiaConfigSet (const IxAtmdAccUtopiaConfig *
                        ixAtmdAccUtopiaConfigPtr);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccUtopiaStatusGet (IxAtmdAccUtopiaStatus *
                        ixAtmdAccUtopiaStatus)
 *
 * @brief Get the Utopia interface configuration.
 *
 * This function reads the Utopia registers and the Cell counts
 * and fills the @a IxAtmdAccUtopiaStatus structure
 *
 * A lock is preventing the concurrent
 * use of @a ixAtmdAccUtopiaStatusGet() and @A ixAtmdAccUtopiaConfigSet()
 *
 * @param ixAtmdAccUtopiaStatus @ref IxAtmdAccUtopiaStatus [out] - pointer to structure to be updated from internal
 *        hardware counters. This parameter cannot be a NULL pointer.
 *
 * @return @li IX_SUCCESS successful read
 * @return @li IX_FAIL error in the parameters null pointer, or
 *          configuration read is not complete or failed
 *
 * @sa ixAtmdAccUtopiaConfigSet
 *
 */
PUBLIC IX_STATUS ixAtmdAccUtopiaStatusGet (IxAtmdAccUtopiaStatus *
                        ixAtmdAccUtopiaStatus);

/**
 *
 * @ingroup IxAtmdAcc
 *
 * @fn ixAtmdAccPortEnable (IxAtmLogicalPort port)
 *
 * @brief enable a PHY logical port
 *
 * This function enables the transmission over one port. It should be
 * called before accessing any resource from this port and before the
 * establishment of a VC.
 *
 * When a port is enabled, the cell transmission to the Utopia interface
 * is started. If there is no traffic already running, idle cells are
 * sent over the interface.
 *
 * This function can be called multiple times.
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 *
 * @return @li IX_SUCCESS enable is complete
 * @return @li IX_ATMDACC_WARNING port already enabled
 * @return @li IX_FAIL enable failed, wrong parameter, or cannot
 *         initialise this port (the port is maybe already in use,
 *         or there is a hardware issue)
 *
 * @note - This function needs internal locks and should not be
 *         called from an interrupt context
 *
 * @sa ixAtmdAccPortDisable
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortEnable (IxAtmLogicalPort port);

/**
 *
 * @ingroup IxAtmdAccCtrlAPI
 *
 * @fn ixAtmdAccPortDisable (IxAtmLogicalPort port)
 *
 * @brief disable a PHY logical port
 *
 * This function disable the transmission over one port.
 *
 * When a port is disabled, the cell transmission to the Utopia interface
 * is stopped.
 *
 * @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
 *
 * @return @li IX_SUCCESS disable is complete
 * @return @li IX_ATMDACC_WARNING port already disabled
 * @return @li IX_FAIL disable failed, wrong parameter .
 *
 * @note - This function needs internal locks and should not be called
 *         from an interrupt context
 *
 * @note - The response from hardware is done through the txDone mechanism
 *         to ensure the synchrnisation with tx resources. Therefore, the
 *         txDone mechanism needs to be serviced to make a PortDisable complete.
 *
 * @sa ixAtmdAccPortEnable
 * @sa ixAtmdAccPortDisableComplete
 * @sa ixAtmdAccTxDoneDispatch
 *
 */
PUBLIC IX_STATUS ixAtmdAccPortDisable (IxAtmLogicalPort port);

/**
*
* @ingroup IxAtmdAccCtrlAPI
*
* @fn ixAtmdAccPortDisableComplete (IxAtmLogicalPort port)
*
* @brief disable a PHY logical port
*
* This function indicates if the port disable for a port has completed. This
* function will return true if the port has never been enabled.
*
* @param port @ref IxAtmLogicalPort [in] - logical PHY port [@a IX_UTOPIA_PORT_0 .. @a IX_UTOPIA_MAX_PORTS - 1]
*
* @return @li true disable is complete
* @return @li false disable failed, wrong parameter .
*
* @note - This function needs internal locks and should not be called
*         from an interrupt context
*
* @sa ixAtmdAccPortEnable
* @sa ixAtmdAccPortDisable
*
*/
PUBLIC BOOL ixAtmdAccPortDisableComplete (IxAtmLogicalPort port);

#endif /* IXATMDACCCTRL_H */

/**
 * @} defgroup IxAtmdAccCtrlAPI
 */


