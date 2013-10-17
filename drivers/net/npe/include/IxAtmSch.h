/**
 * @file    IxAtmSch.h
 *
 * @date    23-NOV-2001
 *
 * @brief   Header file for the IXP400 ATM Traffic Shaper
 *
 * This component demonstrates an ATM Traffic Shaper implementation. It
 * will perform shaping on upto 12 ports and total of 44 VCs accross all ports,
 * 32 are intended for AAL0/5 and 12 for OAM (1 per port).
 * The supported traffic types are;1 rt-VBR VC where PCR = SCR.
 * (Effectively CBR) and Up-to 44 VBR VCs.
 *
 * This component models the ATM ports and VCs and is capable of producing
 * a schedule of ATM cells per port which can be supplied to IxAtmdAcc
 * for execution on the data path.
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
 * @sa IxAtmm.h
 *
 */

/**
 * @defgroup IxAtmSch IXP400 ATM Transmit Scheduler (IxAtmSch) API
 *
 * @brief IXP400 ATM scheduler component Public API
 *
 * @{
 */

#ifndef IXATMSCH_H
#define IXATMSCH_H

#include "IxOsalTypes.h"
#include "IxAtmTypes.h"

/*
 * #defines and macros used in this file.
 */

/* Return codes */

/** 
 * @ingroup IxAtmSch
 *
 * @def IX_ATMSCH_RET_NOT_ADMITTED
 * @brief Indicates that CAC function has rejected VC registration due
 *         to insufficient line capacity.
*/
#define IX_ATMSCH_RET_NOT_ADMITTED 2

/**  
 * @ingroup IxAtmSch
 *
 * @def IX_ATMSCH_RET_QUEUE_FULL
 *  @brief Indicates that the VC queue is full, no more demand can be
 *         queued at this time.
 */
#define IX_ATMSCH_RET_QUEUE_FULL 3

/**  
 * @ingroup IxAtmSch
 *
 *  @def IX_ATMSCH_RET_QUEUE_EMPTY
 *  @brief Indicates that all VC queues on this port are empty and
 *         therefore there are no cells to be scheduled at this time.
 */
#define IX_ATMSCH_RET_QUEUE_EMPTY 4

/*
 * Function declarations
 */

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchInit(void)
 *
 *  @brief This function is used to initialize the ixAtmSch component. It
 *         should be called before any other IxAtmSch API function.
 *
 * @param None
 *
 * @return
 * - <b>IX_SUCCESS :</b> indicates that
 *          -# The ATM scheduler component has been successfully initialized.
 *          -# The scheduler is ready to accept Port modelling requests.
 * - <b>IX_FAIL :</b> Some internal error has prevented the scheduler component
 *          from initialising.
 */
PUBLIC IX_STATUS
ixAtmSchInit(void);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchPortModelInitialize( IxAtmLogicalPort port,
                                       unsigned int portRate,
                                       unsigned int minCellsToSchedule)
 *
 * @brief This function shall be called first to initialize an ATM port before
 *         any other ixAtmSch API calls may be made for that port.
 *
 * @param port @ref IxAtmLogicalPort [in] - The specific port to initialize.  Valid
 *          values range from 0 to IX_UTOPIA_MAX_PORTS - 1, representing a 
 *          maximum of IX_UTOPIA_MAX_PORTS possible ports.
 *
 * @param portRate unsigned int [in] - Value indicating the upstream capacity
 *          of the indicated port.  The value should be supplied in
 *          units of ATM (53 bytes) cells per second.
 *          A port rate of 800Kbits/s is the equivalent 
 *          of 1886 cells per second
 *
 * @param minCellsToSchedule unsigned int [in] - This parameter specifies the minimum
 *          number of cells which the scheduler will put in a schedule
 *          table for this port. This value sets the worst case CDVT for VCs
 *          on this port i.e. CDVT = 1*minCellsToSchedule/portRate.
 * @return
 *    - <b>IX_SUCCESS :</b> indicates that
 *          -# The ATM scheduler has been successfully initialized.
 *          -# The requested port model has been established.
 *          -# The scheduler is ready to accept VC modelling requests
 *            on the ATM port.
 *    - <b>IX_FAIL :</b> indicates the requested port could not be
 * initialized.  */
PUBLIC IX_STATUS
ixAtmSchPortModelInitialize( IxAtmLogicalPort port,
                                       unsigned int portRate,
                                       unsigned int minCellsToSchedule);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchPortRateModify( IxAtmLogicalPort port,
                        unsigned int portRate)
 *
 *  @brief This function is called to modify the portRate on a
 *         previously initialized port, typically in the event that
 *         the line condition of the port changes.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port which is to be
 *          modified.
 *
 * @param portRate unsigned int [in] - Value indicating the new upstream
 *          capacity for this port in cells/second.
 *          A port rate of 800Kbits/s is the equivalent 
 *          of 1886 cells per second
 *
 * @return
 * - <b>IX_SUCCESS :</b> The port rate has been successfully modified.<br>
 * - <b>IX_FAIL :</b> The port rate could not be modified, either
 *      because the input data was invalid, or the new port rate is
 *      insufficient to support established ATM VC contracts on this
 *      port.
 *
 * @warning The IxAtmSch component will validate the supplied port
 *          rate is sufficient to support all established VC
 *          contracts on the port.  If the new port rate is
 *          insufficient to support all established contracts then
 *          the request to modify the port rate will be rejected.
 *          In this event, the user is expected to remove
 *          established contracts using the ixAtmSchVcModelRemove
 *          interface and then retry this interface.
 *
 * @sa ixAtmSchVcModelRemove() */
PUBLIC IX_STATUS
ixAtmSchPortRateModify( IxAtmLogicalPort port,
                        unsigned int portRate);


/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchVcModelSetup( IxAtmLogicalPort port,
                      IxAtmTrafficDescriptor *trafficDesc,
                      IxAtmSchedulerVcId *vcId)
 *
 *  @brief A client calls this interface to set up an upstream
 *         (transmitting) virtual connection model (VC) on the
 *         specified ATM port.  This function also provides the
 *         virtual * connection admission control (CAC) service to the
 *         client.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the upstream
 *          VC is to be established.
 *
 * @param *trafficDesc @ref IxAtmTrafficDescriptor [in] - Pointer to a structure
 *          describing the requested traffic contract of the VC to be
 *          established.  This structure contains the typical ATM
 *          traffic descriptor values (e.g. PCR, SCR, MBS, CDVT, etc.)
 *          defined by the ATM standard.
 *
 * @param *vcId @ref IxAtmSchedulerVcId [out] - This value will be filled with the
 *              port-unique identifier for this virtual connection.  A
 *              valid identification is a non-negative number.
 *
 * @return
 * - <b>IX_SUCCESS :</b> The VC has been successfully established on
 *      this port.  The client may begin to submit demand on this VC.
 * - <b>IX_ATMSCH_RET_NOT_ADMITTED :</b> The VC cannot be established
 *      on this port because there is insufficient upstream capacity
 *      available to support the requested traffic contract descriptor
 * - <b>IX_FAIL :</b>Input data are invalid.  VC has not been
 *      established.
 */
PUBLIC IX_STATUS
ixAtmSchVcModelSetup( IxAtmLogicalPort port,
                      IxAtmTrafficDescriptor *trafficDesc,
                      IxAtmSchedulerVcId *vcId);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchVcConnIdSet( IxAtmLogicalPort port,
                     IxAtmSchedulerVcId vcId,
                     IxAtmConnId vcUserConnId)
 *
 *  @brief A client calls this interface to set the vcUserConnId for a VC on
 *         the specified ATM port. This vcUserConnId will default to
 *         IX_ATM_IDLE_CELLS_CONNID if this function is not called for a VC.
 *         Hence if the client does not call this function for a VC then only idle
 *         cells will be scheduled for this VC.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the upstream
 *        VC is has been established.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - This is the unique identifier for this virtual
 *        connection. A valid identification is a non-negative number and is
 *        all ports.
 *
 * @param vcUserConnId @ref IxAtmConnId [in] - The connId is used to refer to a VC in schedule
 *        table entries. It is treated as the Id by which the scheduler client
 *        knows the VC. It is used in any communicatations from the Scheduler
 *        to the scheduler user e.g. schedule table entries.
 *
 * @return
 * - <b>IX_SUCCESS :</b> The id has successfully been set.
 * - <b>IX_FAIL :</b>Input data are invalid. connId id is not established.
 */
PUBLIC IX_STATUS
ixAtmSchVcConnIdSet( IxAtmLogicalPort port,
                     IxAtmSchedulerVcId vcId,
                     IxAtmConnId vcUserConnId);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchVcModelRemove( IxAtmLogicalPort port,
                       IxAtmSchedulerVcId vcId)
 *
 *  @brief Interface called by the client to remove a previously
 *         established VC on a particular port.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the VC to be
 *          removed is established.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - Identifies the VC to be removed.  This is the
 *          value returned by the @ref ixAtmSchVcModelSetup call which
 *          established the relevant VC.
 *
 * @return
 * - <b>IX_SUCCESS :</b> The VC has been successfully removed from
 *      this port. It is no longer modelled on this port.
 * - <b>IX_FAIL :</b>Input data are invalid. The VC is still being modeled
 *      by the traffic shaper.
 *
 * @sa ixAtmSchVcModelSetup() 
 */
PUBLIC IX_STATUS
ixAtmSchVcModelRemove( IxAtmLogicalPort port,
                       IxAtmSchedulerVcId vcId);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchVcQueueUpdate( IxAtmLogicalPort port,
                       IxAtmSchedulerVcId vcId,
                       unsigned int numberOfCells)
 *
 *  @brief The client calls this function to notify IxAtmSch that the
 *         user of a VC has submitted cells for transmission.
 *
 *  This information is stored, aggregated from a number of calls to
 *  ixAtmSchVcQueueUpdate and eventually used in the call to
 *  ixAtmSchTableUpdate.
 *
 *  Normally IxAtmSch will update the VC queue by adding the number of
 *  cells to the current queue length.  However, if IxAtmSch
 *  determines that the user has over-submitted for the VC and
 *  exceeded its transmission quota the queue request can be rejected.
 *  The user should resubmit the request later when the queue has been
 *  depleted.
 *
 *  This implementation of ixAtmSchVcQueueUpdate uses no operating
 *  system or external facilities, either directly or indirectly.
 *  This allows clients to call this function form within an interrupt handler.
 *
 *  This interface is structurally compatible with the
 *  IxAtmdAccSchQueueUpdate callback type definition required for
 *  IXP400 ATM scheduler interoperability.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the VC to be
 *          updated is established.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - Identifies the VC to be updated.  This is the
 *          value returned by the @ref ixAtmSchVcModelSetup call which
 *          established the relevant VC.
 *
 * @param numberOfCells unsigned int [in] - Indicates how many ATM cells should
 *          be added to the queue for this VC.
 *
 * @return
 *  - <b>IX_SUCCESS :</b> The VC queue has been successfully updated.
 *  - <b>IX_ATMSCH_RET_QUEUE_FULL :</b> The VC queue has reached a
 *       preset limit.  This indicates the client has over-submitted
 *       and exceeded its transmission quota.  The request is
 *       rejected.  The VC queue is not updated.  The VC user is
 *       advised to resubmit the request later.
 *  - <b>IX_FAIL :</b> The input are invalid.  No VC queue is updated.
 *
 * @warning IxAtmSch assumes that the calling software ensures that
 *          calls to ixAtmSchVcQueueUpdate, ixAtmSchVcQueueClear and
 *          ixAtmSchTableUpdate are both self and mutually exclusive
 *          for the same port.
 *
 * @sa ixAtmSchVcQueueUpdate(), ixAtmSchVcQueueClear(), ixAtmSchTableUpdate().  */
PUBLIC IX_STATUS
ixAtmSchVcQueueUpdate( IxAtmLogicalPort port,
                       IxAtmSchedulerVcId vcId,
                       unsigned int numberOfCells);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchVcQueueClear( IxAtmLogicalPort port,
                      IxAtmSchedulerVcId vcId)
 *
 *  @brief The client calls this function to remove all currently
 *         queued cells from a registered VC.  The pending cell count
 *         for the specified VC is reset to zero.
 *
 *  This interface is structurally compatible with the
 *  IxAtmdAccSchQueueClear callback type definition required for
 *  IXP400 ATM scheduler interoperability.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port on which the VC to be
 *          cleared is established.
 *
 * @param vcId @ref IxAtmSchedulerVcId [in] - Identifies the VC to be cleared.  This is the
 *          value returned by the @ref ixAtmSchVcModelSetup call which
 *          established the relevant VC.
 *
 * @return
 *  - <b>IX_SUCCESS :</b> The VC queue has been successfully cleared.
 *  - <b>IX_FAIL :</b> The input are invalid.  No VC queue is modified.
 *
 * @warning IxAtmSch assumes that the calling software ensures that
 *          calls to ixAtmSchVcQueueUpdate, ixAtmSchVcQueueClear and
 *          ixAtmSchTableUpdate are both self and mutually exclusive
 *          for the same port.
 *
 * @sa ixAtmSchVcQueueUpdate(), ixAtmSchVcQueueClear(), ixAtmSchTableUpdate().  */
PUBLIC IX_STATUS
ixAtmSchVcQueueClear( IxAtmLogicalPort port,
                      IxAtmSchedulerVcId vcId);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchTableUpdate( IxAtmLogicalPort port,
                     unsigned int maxCells,
                     IxAtmScheduleTable **rettable)
 *
 *  @brief The client calls this function to request an update of the
 *         schedule table for a particular ATM port.
 *
 *  This is called when the client decides it needs a new sequence of
 *  cells to send (probably because the transmit queue is near to
 *  empty for this ATM port).  The scheduler will use its stored
 *  information on the cells submitted for transmit (i.e. data
 *  supplied via @ref ixAtmSchVcQueueUpdate function) with the traffic
 *  descriptor information of all established VCs on the ATM port to
 *  decide the sequence of cells to be sent and fill the schedule
 *  table for a period of time into the future.
 *
 *  IxAtmSch will guarantee a minimum of minCellsToSchedule if there
 *  is at least one cell ready to send. If there are no cells then
 *  IX_ATMSCH_RET_QUEUE_EMPTY is returned.
 *
 *  This implementation of ixAtmSchTableUpdate uses no operating
 *  system or external facilities, either directly or indirectly.
 *  This allows clients to call this function form within an FIQ
 *  interrupt handler.
 *
 * @param port @ref IxAtmLogicalPort [in] - Specifies the ATM port for which requested
 *          schedule table is to be generated.
 *
 * @param maxCells unsigned [in] - Specifies the maximum number of cells
 *          that must be scheduled in the supplied table during any
 *          call to the interface.
 *
 * @param **table @ref IxAtmScheduleTable [out] - A pointer to an area of
 *              storage is returned which contains the generated
 *              schedule table.  The client should not modify the
 *              contents of this table.
 *
 * @return
 *  - <b>IX_SUCCESS :</b> The schedule table has been published.
 *       Currently there is at least one VC queue that is nonempty.
 *  - <b>IX_ATMSCH_RET_QUEUE_EMPTY :</b> Currently all VC queues on
 *       this port are empty.  The schedule table returned is set to
 *       NULL.  The client is not expected to invoke this function
 *       again until more cells have been submitted on this port
 *       through the @ref ixAtmSchVcQueueUpdate function.
 *  - <b>IX_FAIL :</b> The input are invalid.  No action is taken.
 *
 * @warning IxAtmSch assumes that the calling software ensures that
 *          calls to ixAtmSchVcQueueUpdate, ixAtmSchVcQueueClear and
 *          ixAtmSchTableUpdate are both self and mutually exclusive
 *          for the same port.
 *
 * @warning Subsequent calls to this function for the same port will
 *          overwrite the contents of previously supplied schedule
 *          tables.  The client must be completely finished with the
 *          previously supplied schedule table before calling this
 *          function again for the same port.
 *
 * @sa ixAtmSchVcQueueUpdate(), ixAtmSchVcQueueClear(), ixAtmSchTableUpdate().  */
PUBLIC IX_STATUS
ixAtmSchTableUpdate( IxAtmLogicalPort port,
                     unsigned int maxCells,
                     IxAtmScheduleTable **rettable);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchShow(void)
 *
 *  @brief Utility function which will print statistics on the current
 *         and accumulated state of VCs and traffic in the ATM
 *         scheduler component.  Output is sent to the default output
 *         device.
 *
 * @param none
 * @return none
 */
PUBLIC void
ixAtmSchShow(void);

/**  
 * @ingroup IxAtmSch
 *
 * @fn ixAtmSchStatsClear(void)
 *
 *  @brief Utility function which will reset all counter statistics in
 *         the ATM scheduler to zero.
 *
 * @param none
 * @return none
 */
PUBLIC void
ixAtmSchStatsClear(void);

#endif
/* IXATMSCH_H */

/** @} */
