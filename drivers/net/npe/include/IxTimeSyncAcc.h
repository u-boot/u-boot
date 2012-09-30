/**
 * @file IxTimeSyncAcc.h
 *
 * @author Intel Corporation
 * @date 07 May 2004
 *
 * @brief  Header file for IXP400 Access Layer to IEEE 1588(TM) Precision
 * Clock Synchronisation Protocol Hardware Assist
 *
 * @version 1
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
 * @defgroup IxTimeSyncAcc IXP400 Time Sync Access Component API 
 *
 * @brief Public API for IxTimeSyncAcc
 *
 * @{
 */
#ifndef IXTIMESYNCACC_H
#define IXTIMESYNCACC_H

#ifdef  __ixp46X

#include "IxOsal.h"

/**
 * Section for enum
 */

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @enum IxTimeSyncAccStatus
 *
 * @brief The status as returned from the API
 */
typedef enum /**< IxTimeSyncAccStatus */
{
    IX_TIMESYNCACC_SUCCESS = IX_SUCCESS,    /**< Requested operation successful */
    IX_TIMESYNCACC_INVALIDPARAM,            /**< An invalid parameter was passed */
    IX_TIMESYNCACC_NOTIMESTAMP,             /**< While polling no time stamp available */
    IX_TIMESYNCACC_INTERRUPTMODEINUSE,      /**< Polling not allowed while operating in interrupt mode */
    IX_TIMESYNCACC_FAILED                   /**< Internal error occurred */
}IxTimeSyncAccStatus;

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @enum IxTimeSyncAccAuxMode
 *
 * @brief Master or Slave Auxiliary Time Stamp (Snap Shot)
 */
typedef enum /**< IxTimeSyncAccAuxMode */
{
    IX_TIMESYNCACC_AUXMODE_MASTER,          /**< Auxiliary Master Mode */
    IX_TIMESYNCACC_AUXMODE_SLAVE,           /**< Auxiliary Slave Mode */
    IX_TIMESYNCACC_AUXMODE_INVALID          /**< Invalid Auxiliary Mode */
}IxTimeSyncAccAuxMode;

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @enum IxTimeSyncAcc1588PTPPort
 *
 * @brief IEEE 1588 PTP Communication Port(Channel)
 */
typedef enum /**< IxTimeSyncAcc1588PTPPort */
{
    IX_TIMESYNCACC_NPE_A_1588PTP_PORT,      /**< PTP Communication Port on NPE-A */
    IX_TIMESYNCACC_NPE_B_1588PTP_PORT,      /**< PTP Communication Port on NPE-B */
    IX_TIMESYNCACC_NPE_C_1588PTP_PORT,      /**< PTP Communication Port on NPE-C */
    IX_TIMESYNCACC_NPE_1588PORT_INVALID     /**< Invalid PTP Communication Port */
} IxTimeSyncAcc1588PTPPort;

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @enum IxTimeSyncAcc1588PTPPortMode
 *
 * @brief Master or Slave mode for IEEE 1588 PTP Communication Port
 */
typedef enum  /**< IxTimeSyncAcc1588PTPPortMode */
{
    IX_TIMESYNCACC_1588PTP_PORT_MASTER,       /**< PTP Communication Port in Master Mode */
    IX_TIMESYNCACC_1588PTP_PORT_SLAVE,        /**< PTP Communication Port in Slave Mode */
    IX_TIMESYNCACC_1588PTP_PORT_ANYMODE,      /**< PTP Communication Port in ANY Mode
                                                  allows time stamping of all messages
                                                  including non-1588 PTP */
    IX_TIMESYNCACC_1588PTP_PORT_MODE_INVALID  /**< Invalid PTP Port Mode */
}IxTimeSyncAcc1588PTPPortMode;

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @enum IxTimeSyncAcc1588PTPMsgType
 *
 * @brief 1588 PTP Messages types that can be detected on communication port
 *
 * Note that client code can determine this based on master/slave mode in which
 * it is already operating in and this information is made available for the sake
 * of convenience only.
 */
typedef enum  /**< IxTimeSyncAcc1588PTPMsgType */
{
    IX_TIMESYNCACC_1588PTP_MSGTYPE_SYNC,     /**< PTP Sync message sent by Master or received by Slave */
    IX_TIMESYNCACC_1588PTP_MSGTYPE_DELAYREQ, /**< PTP Delay_Req message sent by Slave or received by Master */
    IX_TIMESYNCACC_1588PTP_MSGTYPE_UNKNOWN   /**< Other PTP and non-PTP message sent or received by both
                                                Master and/or Slave */
} IxTimeSyncAcc1588PTPMsgType;

/**
 * Section for struct
 */

/**
 * @ingroup IxTimeSyncAcc
 *
 * @struct IxTimeSyncAccTimeValue
 *
 * @brief Struct to hold 64 bit SystemTime and TimeStamp values
 */
typedef struct  /**< IxTimeSyncAccTimeValue */
{
    UINT32 timeValueLowWord;               /**< Lower 32 bits of the time value */
    UINT32 timeValueHighWord;              /**< Upper 32 bits of the time value */  
} IxTimeSyncAccTimeValue;

/**
 * @ingroup IxTimeSyncAcc
 *
 * @struct IxTimeSyncAccUuid
 *
 * @brief Struct to hold 48 bit UUID values captured in Sync or Delay_Req messages
 */
typedef struct  /**< IxTimeSyncAccUuid */
{
    UINT32 uuidValueLowWord;               /**<The lower 32 bits of the UUID */
    UINT16 uuidValueHighHalfword;          /**<The upper 16 bits of the UUID */  
} IxTimeSyncAccUuid;

/**
 * @ingroup IxTimeSyncAcc
 *
 * @struct IxTimeSyncAccPtpMsgData
 *
 * @brief Struct for data from the PTP message returned when TimeStamp available
 */
typedef struct  /**< IxTimeSyncAccPtpMsgData */
{
    IxTimeSyncAcc1588PTPMsgType ptpMsgType; /**< PTP Messages type */
    IxTimeSyncAccTimeValue ptpTimeStamp;    /**< 64 bit TimeStamp value from PTP Message */
    IxTimeSyncAccUuid ptpUuid;              /**< 48 bit UUID value from the PTP Message */
    UINT16 ptpSequenceNumber;               /**< 16 bit Sequence Number from PTP Message */
} IxTimeSyncAccPtpMsgData;

/**
 * @ingroup IxTimeSyncAcc
 *
 * @struct IxTimeSyncAccStats
 *
 * @brief Statistics for the PTP messages
 */
typedef struct  /**< IxTimeSyncAccStats */
{
    UINT32 rxMsgs; /**< Count of timestamps for received PTP Messages */
    UINT32 txMsgs; /**< Count of timestamps for transmitted PTP Messages */
} IxTimeSyncAccStats;

/**
 * @ingroup IxTimeSyncAcc
 *
 * @typedef IxTimeSyncAccTargetTimeCallback
 *
 * @brief Callback for use by target time stamp interrupt
 */
typedef void (*IxTimeSyncAccTargetTimeCallback)(IxTimeSyncAccTimeValue targetTime);

/**
 * @ingroup IxTimeSyncAcc
 *
 * @typedef IxTimeSyncAccAuxTimeCallback
 *
 * @brief Callback for use by auxiliary time interrupts
 */
typedef void (*IxTimeSyncAccAuxTimeCallback)(IxTimeSyncAccAuxMode auxMode,
             IxTimeSyncAccTimeValue auxTime);

/*
 * Section for prototypes interface functions
 */

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccPTPPortConfigSet(
                               IxTimeSyncAcc1588PTPPort ptpPort,
                               IxTimeSyncAcc1588PTPPortMode ptpPortMode)
 *
 * @brief Configures the IEEE 1588 message detect on particular PTP port.
 *
 * @param ptpPort [in] - PTP port to config
 * @param ptpPortMode [in]- Port to operate in Master or Slave mode
 *
 * This API will enable the time stamping on a particular PTP port.
 *          
 * @li Re-entrant   : No
 * @li ISR Callable : No
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPPortConfigSet(IxTimeSyncAcc1588PTPPort ptpPort,
       IxTimeSyncAcc1588PTPPortMode ptpPortMode);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccPTPPortConfigGet(
                               IxTimeSyncAcc1588PTPPort ptpPort,
                               IxTimeSyncAcc1588PTPPortMode *ptpPortMode)
 *
 * @brief Retrieves IEEE 1588 PTP operation mode on particular PTP port.
 *
 * @param ptpPort [in] - PTP port
 * @param ptpPortMode [in]- Mode of operation of PTP port (Master or Slave)
 *
 * This API will identify the time stamping capability of a PTP port by means
 * of obtaining its mode of operation.
 *          
 * @li Re-entrant   : No
 * @li ISR Callable : No
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPPortConfigGet(IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAcc1588PTPPortMode *ptpPortMode);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccPTPRxPoll(
                               IxTimeSyncAcc1588PTPPort ptpPort,
                               IxTimeSyncAccPtpMsgData  *ptpMsgData)
 *
 * @brief Polls the IEEE 1588 message/time stamp detect status on a particular 
 * PTP Port on the Receive side.
 *
 * @param ptpPort [in] - PTP port to poll
 * @param ptpMsgData [out] - Current TimeStamp and other Data
 *
 * This API will poll for the availability of a time stamp on the received Sync 
 * (Slave) or Delay_Req (Master) messages.
 * The client application will provide the buffer.
 *              
 * @li Re-entrant   : No
 * @li ISR Callable : No
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_NOTIMESTAMP - No time stamp available
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPRxPoll(IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAccPtpMsgData  *ptpMsgData);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccPTPTxPoll(
                               IxTimeSyncAcc1588PTPPort ptpPort,
                               IxTimeSyncAccPtpMsgData  *ptpMsgData)
 *
 *
 * @brief Polls the IEEE 1588 message/time stamp detect status on a particular 
 * PTP Port on the Transmit side.
 *
 * @param ptpPort [in] - PTP port to poll
 * @param ptpMsgData [out] - Current TimeStamp and other Data
 *                        
 * This API will poll for the availability of a time stamp on the transmitted
 * Sync (Master) or Delay_Req (Slave) messages.
 * The client application will provide the buffer.
 *              
 * @li Re-entrant   : No
 * @li ISR Callable : No
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_NOTIMESTAMP - No time stamp available
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPTxPoll(IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAccPtpMsgData  *ptpMsgData);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccSystemTimeSet(
                               IxTimeSyncAccTimeValue systemTime)
 *
 * @brief Sets the System Time in the IEEE 1588 hardware assist block
 *
 * @param systemTime [in] - Value to set System Time
 *                        
 * This API will set the SystemTime to given value.
 *              
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccSystemTimeSet(IxTimeSyncAccTimeValue systemTime);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccSystemTimeGet(
                               IxTimeSyncAccTimeValue *systemTime)
 *
 * @brief Gets the System Time from the IEEE 1588 hardware assist block
 *
 * @param systemTime [out] - Copy the current System Time into the client 
 * application provided buffer
 *
 * This API will get the SystemTime from IEEE1588 block and return to client
 *      
 * @li Re-entrant   : no
 * @li ISR Callable : no
 * 
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccSystemTimeGet(IxTimeSyncAccTimeValue *systemTime);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTickRateSet(
                               UINT32 tickRate)
 *
 * @brief Sets the Tick Rate (Frequency Scaling Value) in the IEEE 1588
 * hardware assist block
 *
 * @param tickRate [in] - Value to set Tick Rate
 *                        
 * This API will set the Tick Rate (Frequency Scaling Value) in the IEEE
 * 1588 block to the given value. The Accumulator register (not client 
 * visible) is incremented by this TickRate value every clock cycle. When 
 * the Accumulator overflows, the SystemTime is incremented by one. This
 * TickValue can therefore be used to adjust the system timer.
 *           
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTickRateSet(UINT32 tickRate);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTickRateGet(
                               UINT32 *tickRate)
 *
 * @brief Gets the Tick Rate (Frequency Scaling Value) from the IEEE 1588
 * hardware assist block
 *
 * @param tickRate [out] - Current Tick Rate value in the IEEE 1588 block
 *
 * This API will get the TickRate on IEE15588 block. Refer to @ref 
 * ixTimeSyncAccTickRateSet for notes on usage of this value.
 *         
 * @li   Reentrant    : yes
 * @li   ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTickRateGet(UINT32 *tickRate);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTargetTimeInterruptEnable(
                               IxTimeSyncAccTargetTimeCallback targetTimeCallback)
 *
 * @brief Enables the interrupt to verify the condition where the System Time 
 * greater or equal to the Target Time in the IEEE 1588 hardware assist block. 
 * If the condition is true an interrupt will be sent to XScale.
 *
 * @param targetTimeCallback [in] - Callback to be invoked when interrupt fires
 *
 * This API will enable the Target Time reached/hit condition interrupt.
 *
 * NOTE: The client application needs to ensure that the APIs 
 * @ref ixTimeSyncAccTargetTimeInterruptEnable, @ref ixTimeSyncAccTargetTimeSet and
 * @ref ixTimeSyncAccTargetTimeInterruptDisable are accessed in mutual exclusive 
 * manner with respect to each other.
 *
 * @li Re-entrant   : no
 * @li ISR Callable : yes
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Null parameter passed for callback
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeInterruptEnable(IxTimeSyncAccTargetTimeCallback targetTimeCallback);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTargetTimeInterruptDisable(
                               void)
 *
 * @brief Disables the interrupt for the condition explained in the function
 * description of @ref ixTimeSyncAccTargetTimeInterruptEnable.
 *
 * This API will disable the Target Time interrupt.
 *
 * NOTE: The client application needs to ensure that the APIs 
 * @ref ixTimeSyncAccTargetTimeInterruptEnable, @ref ixTimeSyncAccTargetTimeSet and
 * @ref ixTimeSyncAccTargetTimeInterruptDisable are accessed in mutual exclusive 
 * manner with respect to each other.
 *              
 * @li Re-entrant   : no
 * @li ISR Callable : yes
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeInterruptDisable(void);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTargetTimePoll(
                               BOOL *ttmPollFlag,
                               IxTimeSyncAccTimeValue *targetTime)
 *
 * @brief Poll to verify the condition where the System Time greater or equal to
 * the Target Time in the IEEE 1588 hardware assist block. If the condition is
 * true an event flag is set in the hardware.
 *
 * @param ttmPollFlag [out] - TRUE  if the target time reached/hit condition event set
 *                            FALSE if the target time reached/hit condition event is 
                                    not set
 * @param targetTime [out] - Capture current targetTime into client provided buffer
 *
 * Poll the target time reached/hit condition status. Return true and the current
 * target time value, if the condition is true else return false.
 *
 * NOTE: The client application will need to clear the event flag that will be set
 * as long as the condition that the System Time greater or equal to the Target Time is
 * valid, in one of the following ways:
 *     1) Invoke the API to change the target time
 *     2) Change the system timer value 
 *              
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Null parameter passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 *         @li IX_TIMESYNCACC_INTERRUPTMODEINUSE - Interrupt mode in use
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimePoll(BOOL *ttmPollFlag,
    IxTimeSyncAccTimeValue *targetTime);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTargetTimeSet(
                               IxTimeSyncAccTimeValue targetTime)
 *
 * @brief Sets the Target Time in the IEEE 1588 hardware assist block
 *
 * @param targetTime [in] - Value to set Target Time
 *                        
 * This API will set the Target Time to a given value.
 *
 * NOTE: The client application needs to ensure that the APIs 
 * @ref ixTimeSyncAccTargetTimeInterruptEnable, @ref ixTimeSyncAccTargetTimeSet and
 * @ref ixTimeSyncAccTargetTimeInterruptDisable are accessed in mutual exclusive 
 * manner with respect to each other.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : yes
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeSet(IxTimeSyncAccTimeValue targetTime);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccTargetTimeGet(
                               IxTimeSyncAccTimeValue *targetTime)
 *
 * @brief Gets the Target Time in the IEEE 1588 hardware assist block
 *
 * @param targetTime [out] - Copy current time to client provided buffer
 *                                                 
 * This API will get the Target Time from IEEE 1588 block and return to the 
 * client application
 *            
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Null parameter passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeGet(IxTimeSyncAccTimeValue *targetTime);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccAuxTimeInterruptEnable(
                               IxTimeSyncAccAuxMode auxMode, 
                               IxTimeSyncAccAuxTimeCallback auxTimeCallback)
 *
 * @brief Enables the interrupt notification for the given mode of Auxiliary Time 
 * Stamp in the IEEE 1588 hardware assist block
 * 
 * @param auxMode [in] - Auxiliary time stamp register (slave or master) to use
 * @param auxTimeCallback [in] - Callback to be invoked when interrupt fires 
 *                                                              
 * This API will enable the Auxiliary Master/Slave Time stamp Interrupt.
 *
 * <pre>
 * NOTE: 1) An individual callback is to be registered for each Slave and Master 
 * Auxiliary Time Stamp registers. Thus to register for both Master and Slave time
 * stamp interrupts either the same callback or two separate callbacks the API has
 * to be invoked twice.
 *       2) On the IXDP465 Development Platform, the Auxiliary Timestamp signal for 
 * slave mode is tied to GPIO 8 pin. This signal is software routed by default to 
 * PCI for backwards compatibility with the IXDP425 Development Platform. This
 * routing must be disabled for the auxiliary slave time stamp register to work 
 * properly. The following commands may be used to accomplish this. However, refer
 * to the IXDP465 Development Platform Users Guide or the BSP/LSP documentation for
 * more specific information.
 *
 * For Linux (at the Redboot prompt i.e., before loading zImage):
 *     mfill -b 0x54100000 -1 -l 1 -p 8
 *     mfill -b 0x54100001 -1 -l 1 -p 0x7f
 * For vxWorks, at the prompt:
 *     intDisable(25)
 *     ixdp400FpgaIODetach(8)
 * </pre>
 *              
 * @li Re-entrant   : no
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Null parameter passed for callback or
                                            invalid auxiliary snapshot mode
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimeInterruptEnable(IxTimeSyncAccAuxMode auxMode,
    IxTimeSyncAccAuxTimeCallback auxTimeCallback);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccAuxTimeInterruptDisable(
                               IxTimeSyncAccAuxMode auxMode)
 *
 * @brief Disables the interrupt for the indicated mode of Auxiliary Time Stamp
 * in the IEEE 1588 hardware assist block
 *
 * @param auxMode [in] - Auxiliary time stamp mode (slave or master) using which
 * the interrupt will be disabled.
 *                        
 * This API will disable the Auxiliary Time Stamp Interrupt (Master or Slave)
 *              
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Invalid parameters passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimeInterruptDisable(IxTimeSyncAccAuxMode auxMode);

/**
 * @ingroup IxTimeSyncAcc
 * 
 * @fn IxTimeSyncAccStatus ixTimeSyncAccAuxTimePoll(
                               IxTimeSyncAccAuxMode auxMode,
                               BOOL *auxPollFlag,
                               IxTimeSyncAccTimeValue *auxTime)
 *
 * @brief Poll for the Auxiliary Time Stamp captured for the mode indicated 
 * (Master or Slave)
 *
 * @param auxMode [in] - Auxiliary Snapshot Register (Slave or Master) to be checked
 * @param auxPollFlag [out] - TRUE if the time stamp captured in auxiliary 
                                   snapshot register
 *                            FALSE if the time stamp not captured in 
                                   auxiliary snapshot register
 * @param auxTime [out] - Copy the current Auxiliary Snapshot Register value into the
 * client provided buffer
 *
 * Polls for the Time stamp in the appropriate Auxiliary Snapshot Registers based 
 * on the mode specified. Return true and the contents of the Auxiliary snapshot,
 * if it is available else return false.
 * 
 * Please refer to the note #2 of the API @ref ixTimeSyncAccAuxTimeInterruptEnable
 * for more information for Auxiliary Slave mode.
 *
 * @li Re-entrant   : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - Null parameter passed for auxPollFlag,
                   callback or invalid auxiliary snapshot mode
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 *         @li IX_TIMESYNCACC_INTERRUPTMODEINUSE - Interrupt mode in use
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimePoll(IxTimeSyncAccAuxMode auxMode,
    BOOL *auxPollFlag,
    IxTimeSyncAccTimeValue *auxTime);

/**
 * @ingroup IxTimeSyncAcc
 *
 * @fn IxTimeSyncAccStatus ixTimeSyncAccReset(void)
 *
 * @brief Resets the IEEE 1588 hardware assist block
 *
 * Sets the reset bit in the IEEE1588 silicon which fully resets the silicon block
 *            
 * @li Reentrant    : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED  - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccReset(void);

/**
 * @ingroup IxTimeSyncAcc
 *
 * @fn IxTimeSyncAccStatus ixTimeSyncAccStatsGet(IxTimeSyncAccStats
                               *timeSyncStats)
 *
 * @brief Returns the IxTimeSyncAcc Statistics in the client supplied buffer
 *
 * @param timeSyncStats [out] - TimeSync statistics counter values
 *
 * This API will return the statistics of the received or transmitted messages.
 * 
 * NOTE: 1) These counters are updated only when the client polls for the time 
 * stamps or interrupt are enabled. This is because the IxTimeSyncAcc module 
 * does not either transmit or receive messages and does only run the code 
 * when explicit requests received by client application.
 *
 *       2) These statistics reflect the number of valid PTP messages exchanged
 * in Master and Slave modes but includes all the messages (including valid 
 * non-PTP messages) while operating in the Any mode.
 *              
 * @li Reentrant    : no
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_INVALIDPARAM - NULL parameter passed
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccStatsGet(IxTimeSyncAccStats *timeSyncStats);

/**
 * @ingroup IxTimeSyncAcc
 *
 * @fn void ixTimeSyncAccStatsReset(void)
 *
 * @brief Reset Time Sync statistics
 *
 * This API will reset the statistics counters of the TimeSync access layer.
 *             
 * @li Reentrant    : yes
 * @li ISR Callable:  no
 *
 * @return @li None
 */
PUBLIC void
ixTimeSyncAccStatsReset(void);

/**
 * @ingroup IxTimeSyncAcc
 *
 * @fn IxTimeSyncAccStatus ixTimeSyncAccShow(void)
 *
 * @brief Displays the Time Sync current status
 *
 * This API will display status on the current configuration of the IEEE
 * 1588 hardware assist block, contents of the various time stamp registers,
 * outstanding interrupts and/or events.
 *
 * Note that this is intended for debug only, and in contrast to the other
 * functions, it does not clear the any of the status bits associated with
 * active timestamps and so is passive in its nature.
 *            
 * @li Reentrant    : yes
 * @li ISR Callable : no
 *
 * @return @li IX_TIMESYNCACC_SUCCESS - Operation is successful
 *         @li IX_TIMESYNCACC_FAILED - Internal error occurred
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccShow(void);

#endif /* __ixp46X */
#endif /* IXTIMESYNCACC_H */

/**
 * @} defgroup IxTimeSyncAcc
 */

