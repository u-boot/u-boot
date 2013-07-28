/**
 * @file IxEthDBLog_p.h
 *
 * @brief definitions of log macros and log configuration
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

#include <IxOsal.h>

#ifdef IX_UNIT_TEST
#define NULL_PRINT_ROUTINE(format, arg...) /* nothing */
#else
#define NULL_PRINT_ROUTINE if(0) printf
#endif

/***************************************************
 *                 Globals                         *
 ***************************************************/
/* safe to permanently leave these on */
#define HAS_ERROR_LOG
#define HAS_ERROR_IRQ_LOG
#define HAS_WARNING_LOG

/***************************************************
 *              Log Configuration                  *
 ***************************************************/

/* debug output can be turned on unless specifically
   declared as a non-debug build */
#ifndef NDEBUG

#undef HAS_EVENTS_TRACE
#undef HAS_EVENTS_VERBOSE_TRACE

#undef HAS_SUPPORT_TRACE
#undef HAS_SUPPORT_VERBOSE_TRACE

#undef HAS_NPE_TRACE
#undef HAS_NPE_VERBOSE_TRACE
#undef HAS_DUMP_NPE_TREE

#undef HAS_UPDATE_TRACE
#undef HAS_UPDATE_VERBOSE_TRACE

#endif /* NDEBUG */


/***************************************************
 *              Log Macros                         *
 ***************************************************/

/************** Globals ******************/

#ifdef HAS_ERROR_LOG

    #define ERROR_LOG printf

#else

    #define ERROR_LOG NULL_PRINT_ROUTINE

#endif

#ifdef HAS_ERROR_IRQ_LOG

    #define ERROR_IRQ_LOG(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, format, arg1, arg2, arg3, arg4, arg5, arg6)

#else

    #define ERROR_IRQ_LOG(format, arg1, arg2, arg3, arg4, arg5, arg6) /* nothing */

#endif

#ifdef HAS_WARNING_LOG

    #define WARNING_LOG printf

#else

    #define WARNING_LOG NULL_PRINT_ROUTINE

#endif

/************** Events *******************/

#ifdef HAS_EVENTS_TRACE

    #define IX_ETH_DB_EVENTS_TRACE     printf
    #define IX_ETH_DB_IRQ_EVENTS_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsalLog(IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_EVENTS_VERBOSE_TRACE

        #define IX_ETH_DB_EVENTS_VERBOSE_TRACE printf
    
    #else

        #define IX_ETH_DB_EVENTS_VERBOSE_TRACE NULL_PRINT_ROUTINE

    #endif /* HAS_EVENTS_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_EVENTS_TRACE         NULL_PRINT_ROUTINE
    #define IX_ETH_DB_EVENTS_VERBOSE_TRACE NULL_PRINT_ROUTINE
    #define IX_ETH_DB_IRQ_EVENTS_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) /* nothing */

#endif /* HAS_EVENTS_TRACE */

/************** Support *******************/

#ifdef HAS_SUPPORT_TRACE

    #define IX_ETH_DB_SUPPORT_TRACE     printf
    #define IX_ETH_DB_SUPPORT_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsalLog(IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_SUPPORT_VERBOSE_TRACE

        #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE NULL_PRINT_ROUTINE

    #endif /* HAS_SUPPORT_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_SUPPORT_TRACE         NULL_PRINT_ROUTINE
    #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE NULL_PRINT_ROUTINE
    #define IX_ETH_DB_SUPPORT_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) /* nothing */

#endif /* HAS_SUPPORT_TRACE */

/************** NPE Adaptor *******************/

#ifdef HAS_NPE_TRACE

    #define IX_ETH_DB_NPE_TRACE     printf
    #define IX_ETH_DB_NPE_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsalLog(IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_NPE_VERBOSE_TRACE

        #define IX_ETH_DB_NPE_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_NPE_VERBOSE_TRACE NULL_PRINT_ROUTINE

    #endif /* HAS_NPE_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_NPE_TRACE         NULL_PRINT_ROUTINE
    #define IX_ETH_DB_NPE_VERBOSE_TRACE NULL_PRINT_ROUTINE
    #define IX_ETH_DB_NPE_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) /* nothing */

#endif /* HAS_NPE_TRACE */

#ifdef HAS_DUMP_NPE_TREE

#define IX_ETH_DB_NPE_DUMP_ELT(eltBaseAddress, eltSize) ixEthELTDumpTree(eltBaseAddress, eltSize)

#else

#define IX_ETH_DB_NPE_DUMP_ELT(eltBaseAddress, eltSize) /* nothing */

#endif /* HAS_DUMP_NPE_TREE */

/************** Port Update *******************/

#ifdef HAS_UPDATE_TRACE

    #define IX_ETH_DB_UPDATE_TRACE printf

    #ifdef HAS_UPDATE_VERBOSE_TRACE

        #define IX_ETH_DB_UPDATE_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_UPDATE_VERBOSE_TRACE NULL_PRINT_ROUTINE

    #endif

#else /* HAS_UPDATE_VERBOSE_TRACE */

    #define IX_ETH_DB_UPDATE_TRACE         NULL_PRINT_ROUTINE
    #define IX_ETH_DB_UPDATE_VERBOSE_TRACE NULL_PRINT_ROUTINE

#endif /* HAS_UPDATE_TRACE */
