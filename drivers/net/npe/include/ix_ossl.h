/**
 * ============================================================================
 * = COPYRIGHT
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
 * = PRODUCT
 *      Intel(r) IXP425 Software Release
 *
 * = LIBRARY
 *      OSSL - Operating System Services  Library
 *
 * = MODULE
 *      OSSL  Abstraction layer header file
 *
 * = FILENAME
 *       ix_ossl.h (Replaced by OSAL)
 *
 * = DESCRIPTION
 *   This file contains the prototypes of OS-independent wrapper
 *   functions which allow the programmer not to be tied to a specific
 *   operating system. The OSSL functions can be divided into three classes:
 *
 *   1) synchronization-related wrapper functions around thread system calls
 *   2) thread-related wrapper functions around thread calls
 *   3) transactor/workbench osapi calls -- defined in osApi.h
 *
 *   Both 1 and 2 classes of functions provide Thread Management, Thread 
 *   Synchronization, Mutual Exclusion and  Timer primitives. Namely, 
 *   creation and deletion functions as well as the standard "wait" and
 *   "exit". Additionally, a couple of utility functions which enable to 
 *   pause the execution of a thread are also provided.
 *
 *   The 3rd class provides a  slew of other OSAPI functions to handle
 *   Transactor/WorkBench OS calls.
 *
 *
 *   OSSL Thread APIs:
 *             The OSSL thread functions that allow for thread creation,
 *             get thread id, thread deletion and set thread priroity.
 *
 *             ix_ossl_thread_create
 *             ix_ossl_thread_get_id
 *             ix_ossl_thread_exit
 *             ix_ossl_thread_kill
 *             ix_ossl_thread_set_priority
 *             ix_ossl_thread__delay
 * 
 *   OSSL Semaphore APIs:
 *             The OSSL semaphore functions that allow for initialization,
 *             posting, waiting and deletion of semaphores.
 *
 *             ix_ossl_sem_init
 *             ix_ossl_sem_fini
 *             ix_ossl_sem_take
 *             ix_ossl_sem_give
 *             ix_ossl_sem_flush
 *
 *   OSSL Mutex APIs:
 *             The OSSL wrapper functions that allow for initialization,
 *             posting, waiting and deletion of mutexes.
 *
 *             ix_ossl_mutex_init
 *             ix_ossl_mutex_fini
 *             ix_ossl_mutex_lock
 *             ix_ossl_mutex_unlock
 *
 *   OSSL Timer APIs:
 *     	       The timer APIs provide sleep and get time functions.
 *
 *             ix_ossl_sleep
 *             ix_ossl_sleep_tick
 *             ix_ossl_time_get
 * 
 *   OSAPIs for Transactor/WorkBench:
 *             These OSAPI functions are used for transator OS calls. 
 *             They are defined in osApi.h. 
 *
 *             Sem_Init 			
 *             Sem_Destroy 		
 *             Sem_Wait 			
 *             Sem_Wait			
 *             Thread_Create		
 *             Thread_Cancel 		
 *             Thread_SetPriority 		
 *             delayMs 			
 *             delayTick			
 *             
 *
 *
 **********************************************************************
 *      
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = ACKNOWLEDGEMENTS
 *      
 *
 * = CREATION TIME
 *      1/8/2002 1:53:42 PM
 *
 * = CHANGE HISTORY
 *   02/22/2002 : Renamed osapi.h os_api.h
 *                Moved OS header file includes from OSSL.h to os_api.h
 *                Moved OS specific datatypes to os_api.h 
 *                Modified data types, macros and functions as per
 *                'C' coding guidelines.
 *
 *
 * ============================================================================
 */

#ifndef _IX_OSSL_H
#ifndef __doxygen_hide
#define _IX_OSSL_H
#endif /* __doxygen_hide */

#include "IxOsalBackward.h"
 
#endif /* _IX_OSSL_H */

/**
 * @} defgroup IxOSSL
 */
