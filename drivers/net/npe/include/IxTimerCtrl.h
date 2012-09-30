/** 
 * @file IxTimerCtrl.h
 * @brief 
 *    This is the header file for the Timer Control component.
 *
 *    The timer callback control component provides a mechanism by which different 
 *    client components can start a timer and have a supplied callback function
 *    invoked when the timer expires.
 *    The callbacks are all dispatched from one thread inside this component. 
 *    Any component that needs to be called periodically should use this facility 
 *    rather than create its own task with a sleep loop. 
 *
 * @par
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
 * @defgroup IxTimerCtrl IXP400 Timer Control (IxTimerCtrl) API
 *
 * @brief The public API for the IXP400 Timer Control Component.
 *
 * @{
 */

#ifndef IxTimerCtrl_H
#define IxTimerCtrl_H


#include "IxTypes.h"
/* #include "Ossl.h" */

/*
 * #defines and macros used in this file.
 */

/**
 * @ingroup IxTimerCtrl
 *
 * @def IX_TIMERCTRL_NO_FREE_TIMERS
 *
 * @brief Timer schedule return code.
 *
 * Indicates that the request to start a timer failed because
 * all available timer resources are used. 
 */
#define IX_TIMERCTRL_NO_FREE_TIMERS 2


/**
 * @ingroup IxTimerCtrl
 *
 * @def IX_TIMERCTRL_PARAM_ERROR
 *
 * @brief Timer schedule return code.
 *
 * Indicates that the request to start a timer failed because
 * the client has supplied invalid parameters.
 */
#define IX_TIMERCTRL_PARAM_ERROR 3

 
/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @ingroup IxTimerCtrl
 *
 * @brief A typedef for a pointer to a timer callback function. 
 * @para void * - This parameter is supplied by the client when the
 * timer is started and passed back to the client in the callback.
 * @note in general timer callback functions should not block or 
 * take longer than 100ms. This constraint is required to ensure that
 * higher priority callbacks are not held up. 
 * All callbacks are called from the same thread. 
 * This thread is a shared resource. 
 * The parameter passed is provided when the timer is scheduled.
 */
typedef void (*IxTimerCtrlTimerCallback)(void *userParam);


/**
 * @ingroup IxTimerCtrl
 *
 * @brief List used to identify the users of timers.
 * @note The order in this list indicates priority.  Components appearing 
 * higher in the list will be given priority over components lower in the
 * list.  When adding components, please insert at an appropriate position
 * for priority ( i.e values should be less than IxTimerCtrlMaxPurpose ) .
 */
typedef enum 
{
    IxTimerCtrlAdslPurpose,
   /* Insert new purposes above this line only
    */
   IxTimerCtrlMaxPurpose
}
IxTimerCtrlPurpose;


/*
 * Function definition
 */

/**
 * @ingroup IxTimerCtrl
 *
 * @fn ixTimerCtrlSchedule(IxTimerCtrlTimerCallback func, 
                       void *userParam, 
                       IxTimerCtrlPurpose purpose,
	               UINT32 relativeTime,
                       unsigned *timerId )
 * 
 * @brief Schedules a callback function to be called after a period of "time".
 * The callback function should not block or run for more than 100ms.
 * This function 
 *
 * @param func @ref IxTimerCtrlTimerCallback [in] - the callback function to be called.
 * @param userParam void [in] - a parameter to send to the callback function, can be NULL.
 * @param purpose @ref IxTimerCtrlPurpose [in] - the purpose of the callback, internally this component will 
 * decide the priority of callbacks with different purpose.
 * @param relativeTime UINT32 [in] - time relative to now in milliseconds after which the callback 
 * will be called. The time must be greater than the duration of one OS tick.
 * @param *timerId unsigned [out] -  An id for the callback scheduled. 
 * This id can be used to cancel the callback.
 * @return 
 * @li IX_SUCCESS - The timer was started successfully.
 * @li IX_TIMERCTRL_NO_FREE_TIMERS - The timer was not started because the maximum number
 * of running timers has been exceeded.
 * @li IX_TIMERCTRL_PARAM_ERROR - The timer was not started because the client has supplied 
 * a NULL callback func, or the requested timeout is less than one OS tick.
 * @note  This function is re-entrant. The function accesses a list of running timers
 * and may suspend the calling thread if this list is being accesed by another thread.
 */
PUBLIC IX_STATUS 
ixTimerCtrlSchedule(IxTimerCtrlTimerCallback func, 
                       void *userParam, 
                       IxTimerCtrlPurpose purpose,
	               UINT32 relativeTime,
                       unsigned *timerId );


/**
 * @ingroup IxTimerCtrl
 *
 * @fn ixTimerCtrlScheduleRepeating(IxTimerCtrlTimerCallback func, 
                                void *param, 
                                IxTimerCtrlPurpose purpose,
			        UINT32 interval,
                                unsigned *timerId )
 * 
 * @brief Schedules a callback function to be called after a period of "time".
 * The callback function should not block or run for more than 100ms.
 *
 * @param func @ref IxTimerCtrlTimerCallback [in] - the callback function to be called.
 * @param userParam void [in] - a parameter to send to the callback function, can be NULL.
 * @param purpose @ref IxTimerCtrlPurpose [in] - the purpose of the callback, internally this component will 
 * decide the priority of callbacks with different purpose.
 * @param interval UINT32 [in] - the interval in milliseconds between calls to func. 
 * @param timerId unsigned [out] - An id for the callback scheduled. 
 * This id can be used to cancel the callback.
 * @return 
 * @li IX_SUCCESS - The timer was started successfully.
 * @li IX_TIMERCTRL_NO_FREE_TIMERS - The timer was not started because the maximum number
 * of running timers has been exceeded.
 * @li IX_TIMERCTRL_PARAM_ERROR - The timer was not started because the client has supplied 
 * a NULL callback func, or the requested timeout is less than one OS tick.
 * @note  This function is re-entrant. The function accesses a list of running timers
 * and may suspend the calling thread if this list is being accesed by another thread.
 */
PUBLIC IX_STATUS 
ixTimerCtrlScheduleRepeating(IxTimerCtrlTimerCallback func, 
                                void *param, 
                                IxTimerCtrlPurpose purpose,
			        UINT32 interval,
                                unsigned *timerId );

/**
 * @ingroup IxTimerCtrl
 *
 * @fn ixTimerCtrlCancel (unsigned id)
 *
 * @brief Cancels a scheduled callback.
 *
 * @param id unsigned [in] - the id of the callback to be cancelled.
 * @return
 * @li IX_SUCCESS - The timer was successfully stopped.
 * @li IX_FAIL - The id parameter did not corrrespond to any running timer..
 * @note This function is re-entrant. The function accesses a list of running timers
 * and may suspend the calling thread if this list is being accesed by another thread.
 */
PUBLIC IX_STATUS
ixTimerCtrlCancel (unsigned id);

/**
 * @ingroup IxTimerCtrl
 *
 * @fn ixTimerCtrlInit(void)
 *
 * @brief Initialise the Timer Control Component.
 * @return 
 * @li IX_SUCCESS - The timer control component initialized successfully.
 * @li IX_FAIL - The timer control component initialization failed,
 * or the component was already initialized.
 * @note This must be done before any other API function is called.
 * This function should be called once only and is not re-entrant.
 */
PUBLIC IX_STATUS
ixTimerCtrlInit(void);


/**
 * @ingroup IxTimerCtrl
 *
 * @fn ixTimerCtrlShow( void )
 *
 * @brief Display the status of the Timer Control Component. 
 * @return void
 * @note Displays a list of running timers.
 * This function is not re-entrant. This function does not suspend the calling thread.
 */
PUBLIC void
ixTimerCtrlShow( void );

#endif  /* IXTIMERCTRL_H */

