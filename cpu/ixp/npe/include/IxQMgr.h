/**
 * @file    IxQMgr.h
 *
 * @date    30-Oct-2001
 *
 * @brief This file contains the public API of IxQMgr component.
 *
 * Some functions contained in this module are inline to achieve better
 * data-path performance. For this to work, the function definitions are
 * contained in this header file. The "normal" use of inline functions
 * is to use the inline functions in the module in which they are
 * defined. In this case these inline functions are used in external
 * modules and therefore the use of "inline extern". What this means
 * is as follows: if a function foo is declared as "inline extern" this
 * definition is only used for inlining, in no case is the function
 * compiled on its own. If the compiler cannot inline the function it
 * becomes an external reference. Therefore in IxQMgrQAccess.c all
 * inline functions are defined without the "inline extern" specifier
 * and so define the external references. In all other source files
 * including this header file, these funtions are defined as "inline
 * extern".
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

/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */
/**
 * @defgroup IxQMgrAPI IXP400 Queue Manager (IxQMgr) API
 *
 * @brief The public API for the IXP400 QMgr component.
 *
 * IxQMgr is a low level interface to the AHB Queue Manager
 *
 * @{
 */

#ifndef IXQMGR_H
#define IXQMGR_H

/*
 * User defined include files
 */

#include "IxOsal.h"

/* 
 * Define QMgr's IoMem macros, in DC mode if in LE 
 * regular if in BE. (Note: For Linux LSP gold release
 * may need to adjust mode.
 */
#if defined (__BIG_ENDIAN)

#define IX_QMGR_INLINE_READ_LONG IX_OSAL_READ_LONG_BE
#define IX_QMGR_INLINE_WRITE_LONG IX_OSAL_WRITE_LONG_BE

#else

#define IX_QMGR_INLINE_READ_LONG IX_OSAL_READ_LONG_LE_DC
#define IX_QMGR_INLINE_WRITE_LONG IX_OSAL_WRITE_LONG_LE_DC

#endif

/*
 * #defines and macros
 */

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_INLINE
*
* @brief Inline definition, for inlining of Queue Access functions on API
*
* Please read the header information in this file for more details on the
* use of function inlining in this component.
*
*/

#ifndef __wince

#ifdef IXQMGRQACCESS_C
/* If IXQMGRQACCESS_C is set then the IxQmgrQAccess.c is including this file
   and must instantiate a concrete definition for each inlineable API function
   whether or not that function actually gets inlined. */
#    ifdef NO_INLINE_APIS
#        undef NO_INLINE_APIS
#    endif
#    define IX_QMGR_INLINE  /* Empty Define */
#else
#    ifndef NO_INLINE_APIS
#       define IX_QMGR_INLINE IX_OSAL_INLINE_EXTERN
#    else
#       define IX_QMGR_INLINE /* Empty Define */
#    endif
#endif

#else /* ndef __wince */

#    ifndef NO_INLINE_APIS
#       define NO_INLINE_APIS
#    endif
#    define IX_QMGR_INLINE

#endif


/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_NUM_QUEUES
*
* @brief Number of queues supported by the AQM.
*
* This constant is used to indicate the number of AQM queues
*
*/
#define IX_QMGR_MAX_NUM_QUEUES  64

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MIN_QID
*
* @brief Minimum queue identifier.
*
* This constant is used to indicate the smallest queue identifier
*
*/
#define IX_QMGR_MIN_QID IX_QMGR_QUEUE_0

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_QID
*
* @brief Maximum queue identifier.
*
* This constant is used to indicate the largest queue identifier
*
*/
#define IX_QMGR_MAX_QID IX_QMGR_QUEUE_63

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MIN_QUEUPP_QID
*
* @brief Minimum queue identifier for reduced functionality queues.
*
* This constant is used to indicate Minimum queue identifier for reduced
* functionality queues
*
*/
#define IX_QMGR_MIN_QUEUPP_QID 32

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_QNAME_LEN
*
* @brief Maximum queue name length.
*
* This constant is used to indicate the maximum null terminated string length
* (excluding '\0') for a queue name
*
*/
#define IX_QMGR_MAX_QNAME_LEN 16

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_WARNING
 *
 * @brief Warning return code.
 *
 * Execution complete, but there is a special case to handle
 *
 */
#define IX_QMGR_WARNING 2

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_PARAMETER_ERROR
 *
 * @brief Parameter error return code (NULL pointer etc..).
 *
 * parameter error out of range/invalid
 *
 */
#define IX_QMGR_PARAMETER_ERROR 3

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_ENTRY_SIZE
 *
 * @brief Invalid entry size return code.
 *
 * Invalid queue entry size for a queue read/write
 *
 */
#define IX_QMGR_INVALID_Q_ENTRY_SIZE 4

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_ID
 *
 * @brief Invalid queue identifier return code.
 *
 * Invalid queue id, not in range 0-63
 *
 */
#define IX_QMGR_INVALID_Q_ID 5

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_CB_ID
 *
 * @brief Invalid callback identifier return code.
 *
 * Invalid callback id
 */
#define IX_QMGR_INVALID_CB_ID 6

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_CB_ALREADY_SET
 *
 * @brief Callback set error return code.
 *
 * The specified callback has already been for this queue
 *
 */
#define IX_QMGR_CB_ALREADY_SET 7

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_NO_AVAILABLE_SRAM
 *
 * @brief Sram consumed return code.
 *
 * All AQM Sram is consumed by queue configuration
 *
 */
#define IX_QMGR_NO_AVAILABLE_SRAM 8

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_INT_SOURCE_ID
 *
 * @brief Invalid queue interrupt source identifier return code.
 *
 * Invalid queue interrupt source given for notification enable
 */
#define IX_QMGR_INVALID_INT_SOURCE_ID 9

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_QSIZE
 *
 * @brief Invalid queue size error code.
 *
 * Invalid queue size not one of 16,32, 64, 128
 *
 *
 */
#define IX_QMGR_INVALID_QSIZE 10

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_WM
 *
 * @brief Invalid queue watermark return code.
 *
 * Invalid queue watermark given for watermark set
 */
#define IX_QMGR_INVALID_Q_WM 11

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_NOT_CONFIGURED
 *
 * @brief Queue not configured return code.
 *
 * Returned to the client when a function has been called on an unconfigured
 * queue
 *
 */
#define IX_QMGR_Q_NOT_CONFIGURED 12

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_ALREADY_CONFIGURED
 *
 * @brief Queue already configured return code.
 *
 * Returned to client to indicate that a queue has already been configured
 */
#define IX_QMGR_Q_ALREADY_CONFIGURED 13

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_UNDERFLOW
 *
 * @brief Underflow return code.
 *
 * Underflow on a queue read has occurred
 *
 */
#define IX_QMGR_Q_UNDERFLOW 14

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_OVERFLOW
 *
 * @brief Overflow return code.
 *
 * Overflow on a queue write has occurred
 *
 */
#define IX_QMGR_Q_OVERFLOW 15

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_INVALID_PRIORITY
 *
 * @brief Invalid priority return code.
 *
 * Invalid priority, not one of 0,1,2
 */
#define IX_QMGR_Q_INVALID_PRIORITY 16

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS
 *
 * @brief  Entry index out of bounds return code.
 *
 * Entry index is greater than number of entries in queue.
 */
#define IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS 17

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def ixQMgrDispatcherLoopRun
 *
 * @brief  Map old function name ixQMgrDispatcherLoopRun () 
 *         to @ref ixQMgrDispatcherLoopRunA0 ().
 *
 */
#define ixQMgrDispatcherLoopRun ixQMgrDispatcherLoopRunA0


/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_QUEUE
 *
 * @brief  Definition of AQM queue numbers
 *
 */
#define IX_QMGR_QUEUE_0 (0)      /**< Queue Number 0 */
#define IX_QMGR_QUEUE_1 (1)      /**< Queue Number 1 */
#define IX_QMGR_QUEUE_2 (2)      /**< Queue Number 2 */
#define IX_QMGR_QUEUE_3 (3)      /**< Queue Number 3 */
#define IX_QMGR_QUEUE_4 (4)      /**< Queue Number 4 */
#define IX_QMGR_QUEUE_5 (5)      /**< Queue Number 5 */
#define IX_QMGR_QUEUE_6 (6)      /**< Queue Number 6 */
#define IX_QMGR_QUEUE_7 (7)      /**< Queue Number 7 */
#define IX_QMGR_QUEUE_8 (8)      /**< Queue Number 8 */
#define IX_QMGR_QUEUE_9 (9)      /**< Queue Number 9 */
#define IX_QMGR_QUEUE_10 (10)    /**< Queue Number 10 */
#define IX_QMGR_QUEUE_11 (11)    /**< Queue Number 11 */
#define IX_QMGR_QUEUE_12 (12)    /**< Queue Number 12 */
#define IX_QMGR_QUEUE_13 (13)    /**< Queue Number 13 */
#define IX_QMGR_QUEUE_14 (14)    /**< Queue Number 14 */
#define IX_QMGR_QUEUE_15 (15)    /**< Queue Number 15 */
#define IX_QMGR_QUEUE_16 (16)    /**< Queue Number 16 */
#define IX_QMGR_QUEUE_17 (17)    /**< Queue Number 17 */
#define IX_QMGR_QUEUE_18 (18)    /**< Queue Number 18 */
#define IX_QMGR_QUEUE_19 (19)    /**< Queue Number 19 */
#define IX_QMGR_QUEUE_20 (20)    /**< Queue Number 20 */
#define IX_QMGR_QUEUE_21 (21)    /**< Queue Number 21 */
#define IX_QMGR_QUEUE_22 (22)    /**< Queue Number 22 */
#define IX_QMGR_QUEUE_23 (23)    /**< Queue Number 23 */
#define IX_QMGR_QUEUE_24 (24)    /**< Queue Number 24 */
#define IX_QMGR_QUEUE_25 (25)    /**< Queue Number 25 */
#define IX_QMGR_QUEUE_26 (26)    /**< Queue Number 26 */
#define IX_QMGR_QUEUE_27 (27)    /**< Queue Number 27 */
#define IX_QMGR_QUEUE_28 (28)    /**< Queue Number 28 */
#define IX_QMGR_QUEUE_29 (29)    /**< Queue Number 29 */
#define IX_QMGR_QUEUE_30 (30)    /**< Queue Number 30 */
#define IX_QMGR_QUEUE_31 (31)    /**< Queue Number 31 */
#define IX_QMGR_QUEUE_32 (32)    /**< Queue Number 32 */
#define IX_QMGR_QUEUE_33 (33)    /**< Queue Number 33 */
#define IX_QMGR_QUEUE_34 (34)    /**< Queue Number 34 */
#define IX_QMGR_QUEUE_35 (35)    /**< Queue Number 35 */
#define IX_QMGR_QUEUE_36 (36)    /**< Queue Number 36 */
#define IX_QMGR_QUEUE_37 (37)    /**< Queue Number 37 */
#define IX_QMGR_QUEUE_38 (38)    /**< Queue Number 38 */
#define IX_QMGR_QUEUE_39 (39)    /**< Queue Number 39 */
#define IX_QMGR_QUEUE_40 (40)    /**< Queue Number 40 */
#define IX_QMGR_QUEUE_41 (41)    /**< Queue Number 41 */
#define IX_QMGR_QUEUE_42 (42)    /**< Queue Number 42 */
#define IX_QMGR_QUEUE_43 (43)    /**< Queue Number 43 */
#define IX_QMGR_QUEUE_44 (44)    /**< Queue Number 44 */
#define IX_QMGR_QUEUE_45 (45)    /**< Queue Number 45 */
#define IX_QMGR_QUEUE_46 (46)    /**< Queue Number 46 */
#define IX_QMGR_QUEUE_47 (47)    /**< Queue Number 47 */
#define IX_QMGR_QUEUE_48 (48)    /**< Queue Number 48 */
#define IX_QMGR_QUEUE_49 (49)    /**< Queue Number 49 */
#define IX_QMGR_QUEUE_50 (50)    /**< Queue Number 50 */
#define IX_QMGR_QUEUE_51 (51)    /**< Queue Number 51 */
#define IX_QMGR_QUEUE_52 (52)    /**< Queue Number 52 */
#define IX_QMGR_QUEUE_53 (53)    /**< Queue Number 53 */
#define IX_QMGR_QUEUE_54 (54)    /**< Queue Number 54 */
#define IX_QMGR_QUEUE_55 (55)    /**< Queue Number 55 */
#define IX_QMGR_QUEUE_56 (56)    /**< Queue Number 56 */
#define IX_QMGR_QUEUE_57 (57)    /**< Queue Number 57 */
#define IX_QMGR_QUEUE_58 (58)    /**< Queue Number 58 */
#define IX_QMGR_QUEUE_59 (59)    /**< Queue Number 59 */
#define IX_QMGR_QUEUE_60 (60)    /**< Queue Number 60 */
#define IX_QMGR_QUEUE_61 (61)    /**< Queue Number 61 */
#define IX_QMGR_QUEUE_62 (62)    /**< Queue Number 62 */
#define IX_QMGR_QUEUE_63 (63)    /**< Queue Number 63 */
#define IX_QMGR_QUEUE_INVALID (64)     /**< AQM Queue Number Delimiter */


/*
 * Typedefs
 */

/**
 * @typedef IxQMgrQId
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Used in the API to identify the AQM queues.
 *
 */
typedef int IxQMgrQId;

/**
 * @typedef IxQMgrQStatus
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue status.
 *
 * A queues status is defined by its relative fullness or relative emptiness.
 * Each of the queues 0-31 have Nearly Empty, Nearly Full, Empty, Full,
 * Underflow and Overflow status flags. Queues 32-63 have just Nearly Empty and
 * Full status flags.
 * The flags bit positions are outlined below:
 *        
 *        OF - bit-5<br> 
 *        UF - bit-4<br> 
 *         F - bit-3<br> 
 *        NF - bit-2<br> 
 *        NE - bit-1<br>
 *         E - bit-0<br> 
 *
 */
typedef UINT32 IxQMgrQStatus;

/**
 * @enum IxQMgrQStatusMask
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue status mask.
 *
 * Masks for extracting the individual status flags from the IxQMgrStatus
 * word.
 *
 */
typedef enum
{
    IX_QMGR_Q_STATUS_E_BIT_MASK  = 0x1,
    IX_QMGR_Q_STATUS_NE_BIT_MASK = 0x2,
    IX_QMGR_Q_STATUS_NF_BIT_MASK = 0x4,
    IX_QMGR_Q_STATUS_F_BIT_MASK  = 0x8,
    IX_QMGR_Q_STATUS_UF_BIT_MASK = 0x10,
    IX_QMGR_Q_STATUS_OF_BIT_MASK = 0x20
} IxQMgrQStatusMask;

/**
 * @enum IxQMgrSourceId
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue interrupt source select.
 *
 * This enum defines the different source conditions on a queue that result in
 * an interupt being fired by the AQM. Interrupt source is configurable for
 * queues 0-31 only. The interrupt source for queues 32-63 is hardwired to the
 * NE(Nearly Empty) status flag.
 *
 */
typedef enum
{
    IX_QMGR_Q_SOURCE_ID_E = 0,  /**< Queue Empty due to last read             */
    IX_QMGR_Q_SOURCE_ID_NE,     /**< Queue Nearly Empty due to last read      */
    IX_QMGR_Q_SOURCE_ID_NF,     /**< Queue Nearly Full due to last write      */
    IX_QMGR_Q_SOURCE_ID_F,      /**< Queue Full due to last write             */
    IX_QMGR_Q_SOURCE_ID_NOT_E,  /**< Queue Not Empty due to last write        */
    IX_QMGR_Q_SOURCE_ID_NOT_NE, /**< Queue Not Nearly Empty due to last write */
    IX_QMGR_Q_SOURCE_ID_NOT_NF, /**< Queue Not Nearly Full due to last read   */
    IX_QMGR_Q_SOURCE_ID_NOT_F   /**< Queue Not Full due to last read          */
} IxQMgrSourceId;

/**
 * @enum IxQMgrQEntrySizeInWords
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr queue entry sizes.
 *
 * The entry size of a queue specifies the size of a queues entry in words.
 *
 */
typedef enum
{
    IX_QMGR_Q_ENTRY_SIZE1 = 1,   /**< 1 word entry       */
    IX_QMGR_Q_ENTRY_SIZE2 = 2,   /**< 2 word entry       */
    IX_QMGR_Q_ENTRY_SIZE4 = 4    /**< 4 word entry       */
} IxQMgrQEntrySizeInWords;

/**
 * @enum IxQMgrQSizeInWords
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr queue sizes.
 *
 * These values define the allowed queue sizes for AQM queue. The sizes are
 * specified in words.
 *
 */
typedef enum
{
    IX_QMGR_Q_SIZE16 = 16,   /**< 16 word buffer     */
    IX_QMGR_Q_SIZE32 = 32,   /**< 32 word buffer     */
    IX_QMGR_Q_SIZE64 = 64,   /**< 64 word buffer     */
    IX_QMGR_Q_SIZE128 = 128, /**< 128 word buffer    */
    IX_QMGR_Q_SIZE_INVALID = 129  /**< Insure that this is greater than largest
				    * queue size supported by the hardware
				    */
} IxQMgrQSizeInWords;

/**
 * @enum IxQMgrWMLevel
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr watermark levels.
 *
 * These values define the valid watermark levels(in ENTRIES) for queues. Each
 * queue 0-63 have configurable Nearly full and Nearly empty watermarks. For
 * queues 32-63 the Nearly full watermark has NO EFFECT.
 * If the Nearly full watermark is set to IX_QMGR_Q_WM_LEVEL16 this means that
 * the nearly full flag will be set by the hardware when there are >= 16 empty
 * entries in the specified queue.
 * If the Nearly empty watermark is set to IX_QMGR_Q_WM_LEVEL16 this means that
 * the Nearly empty flag will be set by the hardware when there are <= 16 full
 * entries in the specified queue.
 */
typedef enum
{
    IX_QMGR_Q_WM_LEVEL0 = 0,    /**< 0 entry watermark  */
    IX_QMGR_Q_WM_LEVEL1 = 1,    /**< 1 entry watermark  */
    IX_QMGR_Q_WM_LEVEL2 = 2,    /**< 2 entry watermark  */
    IX_QMGR_Q_WM_LEVEL4 = 4,    /**< 4 entry watermark  */
    IX_QMGR_Q_WM_LEVEL8 = 8,    /**< 8 entry watermark  */
    IX_QMGR_Q_WM_LEVEL16 = 16,  /**< 16 entry watermark */
    IX_QMGR_Q_WM_LEVEL32 = 32,  /**< 32 entry watermark */
    IX_QMGR_Q_WM_LEVEL64 = 64   /**< 64 entry watermark */
} IxQMgrWMLevel;

/**
 * @ingroup IxQMgrAPI
 * 
 * @enum IxQMgrDispatchGroup
 *
 * @brief QMgr dispatch group select identifiers.
 *
 * This enum defines the groups over which the  dispatcher will process when
 * called. One of the enum values must be used as a input to
 * @a ixQMgrDispatcherLoopRunA0, @a ixQMgrDispatcherLoopRunB0 
 * or @a ixQMgrDispatcherLoopRunB0LLP.
 *
 */
typedef enum
{
    IX_QMGR_QUELOW_GROUP = 0, /**< Queues 0-31  */
    IX_QMGR_QUEUPP_GROUP      /**< Queues 32-63 */
} IxQMgrDispatchGroup;

/**
 * @ingroup IxQMgrAPI
 * 
 * @enum IxQMgrPriority
 *
 * @brief Dispatcher priority levels.
 *
 * This enum defines the different queue dispatch priority levels.
 * The lowest priority number (0) is the highest priority level.
 *
 */
typedef enum
{
  IX_QMGR_Q_PRIORITY_0 = 0,  /**< Priority level 0 */
  IX_QMGR_Q_PRIORITY_1,      /**< Priority level 1 */
  IX_QMGR_Q_PRIORITY_2,      /**< Priority level 2 */
  IX_QMGR_Q_PRIORITY_INVALID /**< Invalid Priority level */
} IxQMgrPriority;

/**
 * @ingroup IxQMgrAPI
 *
 * @enum IxQMgrType
 *
 * @brief Callback types as used with livelock prevention
 *
 * This enum defines the different callback types. 
 * These types are only used when Livelock prevention is enabled.
 * The default is IX_QMGR_TYPE_REALTIME_OTHER. 
 *
 */

typedef enum
{
  IX_QMGR_TYPE_REALTIME_OTHER = 0, /**< Real time callbacks-always allowed run*/
  IX_QMGR_TYPE_REALTIME_PERIODIC,  /**< Periodic callbacks-always allowed run */
  IX_QMGR_TYPE_REALTIME_SPORADIC   /**< Sporadic callbacks-only run if no
                                        periodic callbacks are in progress    */
} IxQMgrType;


/**
 * @ingroup IxQMgrAPI
 * 
 * @typedef IxQMgrCallbackId
 *
 * @brief Uniquely identifies a callback function.
 *
 * A unique callback identifier associated with each callback
 * registered by clients.
 *
 */
typedef unsigned IxQMgrCallbackId;

/**
 * @typedef IxQMgrCallback
 *
 * @brief QMgr notification callback type.
 *
 * This defines the interface to all client callback functions.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param cbId @ref IxQMgrCallbackId [in] - the callback identifier
 */
typedef void (*IxQMgrCallback)(IxQMgrQId qId,
                               IxQMgrCallbackId cbId);

/**
 * @ingroup IxQMgrAPI
 *
 * @typedef IxQMgrDispatcherFuncPtr
 *
 * @brief QMgr Dispatcher Loop function pointer.
 *
 * This defines the interface for QMgr Dispather functions.
 *
 * @param group @ref IxQMgrDispatchGroup [in] - the group of the 
 *                  queue of which the dispatcher will run   
 */
typedef void (*IxQMgrDispatcherFuncPtr)(IxQMgrDispatchGroup group);

/*
 * Function Prototypes
 */

/* ------------------------------------------------------------
   Initialisation related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrInit (void)
 *
 * @brief Initialise the QMgr.
 *
 * This function must be called before and other QMgr function. It
 * sets up internal data structures.
 *
 * @return @li IX_SUCCESS, the IxQMgr successfully initialised
 * @return @li IX_FAIL, failed to initialize the Qmgr
 *
 */
PUBLIC IX_STATUS
ixQMgrInit (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrUnload (void)
 *
 * @brief Uninitialise the QMgr.
 *
 * This function will perform the tasks required to unload the QMgr component
 * cleanly.  This includes unmapping kernel memory.
 * This should be called before a soft reboot or unloading of a kernel module.
 *
 * @pre It should only be called if @ref ixQMgrInit has already been called.
 *
 * @post No QMgr functions should be called until ixQMgrInit is called again.
 *
 * @return @li IX_SUCCESS, the IxQMgr successfully uninitialised
 * @return @li IX_FAIL, failed to uninitialize the Qmgr
 *
 */
PUBLIC IX_STATUS
ixQMgrUnload (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrShow (void)
 *
 * @brief Describe queue configuration and statistics for active queues.
 *
 * This function shows active queues, their configurations and statistics.
 *
 * @return @li void
 *
 */
PUBLIC void
ixQMgrShow (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrQShow (IxQMgrQId qId)
 *
 * @brief Display aqueue configuration and statistics for a queue.
 *
 * This function shows queue configuration and statistics for a queue.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier.
 *
 * @return @li IX_SUCCESS, success
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 *
 */
PUBLIC IX_STATUS
ixQMgrQShow (IxQMgrQId qId);


/* ------------------------------------------------------------
   Configuration related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQConfig (char *qName,
	       IxQMgrQId qId,
	       IxQMgrQSizeInWords qSizeInWords,
	       IxQMgrQEntrySizeInWords qEntrySizeInWords)
 *
 * @brief Configure an AQM queue.
 *
 * This function is called by a client to setup a queue. The size and entrySize
 * qId and qName(NULL pointer) are checked for valid values. This function must
 * be called for each queue, before any queue accesses are made and after
 * ixQMgrInit() has been called. qName is assumed to be a '\0' terminated array
 * of 16 charachters or less.
 *
 * @param *qName char [in] - is the name provided by the client and is associated
 *                          with a QId by the QMgr.
 * @param qId @ref IxQMgrQId [in]  - the qId of this queue
 * @param qSizeInWords @ref IxQMgrQSize [in] - the size of the queue can be one of 16,32
 *                                       64, 128 words.
 * @param qEntrySizeInWords @ref IxQMgrQEntrySizeInWords [in] - the size of a queue entry
 *                                                        can be one of 1,2,4 words.
 *
 * @return @li IX_SUCCESS, a specified queue has been successfully configured.
 * @return @li IX_FAIL, IxQMgr has not been initialised.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s).
 * @return @li IX_QMGR_INVALID_QSIZE, invalid queue size
 * @return @li IX_QMGR_INVALID_Q_ID, invalid queue id
 * @return @li IX_QMGR_INVALID_Q_ENTRY_SIZE, invalid queue entry size
 * @return @li IX_QMGR_Q_ALREADY_CONFIGURED, queue already configured
 *
 */
PUBLIC IX_STATUS
ixQMgrQConfig (char *qName,
	       IxQMgrQId qId,
	       IxQMgrQSizeInWords qSizeInWords,
	       IxQMgrQEntrySizeInWords qEntrySizeInWords);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQSizeInEntriesGet (IxQMgrQId qId,
			 unsigned *qSizeInEntries)
 *
 * @brief Return the size of a queue in entries.
 *
 * This function returns the the size of the queue in entriese.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param *qSizeInEntries @ref IxQMgrQSize [out] - queue size in entries
 *
 * @return @li IX_SUCCESS, successfully retrieved the number of full entrie
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s).
 *
 */
PUBLIC IX_STATUS
ixQMgrQSizeInEntriesGet (IxQMgrQId qId,
			 unsigned *qSizeInEntries);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrWatermarkSet (IxQMgrQId qId,
		    IxQMgrWMLevel ne,
		    IxQMgrWMLevel nf)
 *
 * @brief Set the Nearly Empty and Nearly Full Watermarks fo a queue.
 *
 * This function is called by a client to set the watermarks NE and NF for the
 * queue specified by qId.
 * The queue must be empty at the time this function is called, it is the clients
 * responsibility to ensure that the queue is empty.
 * This function will read the status of the queue before the watermarks are set
 * and again after the watermarks are set. If the status register has changed,
 * due to a queue access by an NPE for example, a warning is returned.
 * Queues 32-63 only support the NE flag, therefore the value of nf will be ignored
 * for these queues.
 *
 * @param qId @ref IxQMgrQId [in] -  the QId of the queue.
 * @param ne @ref IxQMgrWMLevel [in]  - the NE(Nearly Empty) watermark for this
 *                                 queue. Valid values are 0,1,2,4,8,16,32 and
 *                                 64 entries.
 * @param nf @ref IxQMgrWMLevel [in] - the NF(Nearly Full) watermark for this queue.
 *                                 Valid values are 0,1,2,4,8,16,32 and 64
 *                                 entries.
 *
 * @return @li IX_SUCCESS, watermarks have been set for the queu
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_INVALID_Q_WM, invalid watermark
 * @return @li IX_QMGR_WARNING, the status register may not be constistent
 *
 */
PUBLIC IX_STATUS
ixQMgrWatermarkSet (IxQMgrQId qId,
		    IxQMgrWMLevel ne,
		    IxQMgrWMLevel nf);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrAvailableSramAddressGet (UINT32 *address,
			       unsigned *sizeOfFreeSram)
 *
 * @brief Return the address of available AQM SRAM.
 *
 * This function returns the starting address in AQM SRAM not used by the
 * current queue configuration and should only be called after all queues
 * have been configured.
 * Calling this function before all queues have been configured will will return
 * the currently available SRAM. A call to configure another queue will use some
 * of the available SRAM.
 * The amount of SRAM available is specified in sizeOfFreeSram. The address is the
 * address of the bottom of available SRAM. Available SRAM extends from address
 * from address to address + sizeOfFreeSram.
 *
 * @param **address UINT32 [out] - the address of the available SRAM, NULL if
 *                                none available.
 * @param *sizeOfFreeSram unsigned [out]- the size in words of available SRAM
 *
 * @return @li IX_SUCCESS, there is available SRAM and is pointed to by address
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s)
 * @return @li IX_QMGR_NO_AVAILABLE_SRAM, all AQM SRAM is consumed by the queue
 *             configuration.
 *
 */
PUBLIC IX_STATUS
ixQMgrAvailableSramAddressGet (UINT32 *address,
			       unsigned *sizeOfFreeSram);


/* ------------------------------------------------------------
   Queue access related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry)
 *
 * @brief Read an entry from a queue.
 *
 * This function reads an entire entry from a queue returning it in entry. The
 * queue configuration word is read to determine what entry size this queue is
 * configured for and then the number of words specified by the entry size is
 * read.  entry must be a pointer to a previously allocated array of sufficient
 * size to hold an entry.
 *
 * @note - IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained.
 *
 * @param  qId @ref IxQMgrQId [in]   - the queue identifier.
 * @param  *entry UINT32 [out]  - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue
 *
 */
PUBLIC IX_STATUS
ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry);



/** 
 * @brief Internal structure to facilitate inlining functions in IxQMgr.h
 */
typedef struct
{
    /* fields related to write functions */
    UINT32 qOflowStatBitMask;         /**< overflow status mask */
    UINT32 qWriteCount;               /**< queue write count */

    /* fields related to read and write functions */
    volatile UINT32 *qAccRegAddr;     /**< access register */
    volatile UINT32 *qUOStatRegAddr;  /**< status register */
    volatile UINT32 *qConfigRegAddr;  /**< config register */
    UINT32 qEntrySizeInWords;         /**< queue entry size in words */
    UINT32 qSizeInEntries;            /**< queue size in entries */

    /* fields related to read functions */
    UINT32 qUflowStatBitMask;         /**< underflow status mask */
    UINT32 qReadCount;                /**< queue read count */
} IxQMgrQInlinedReadWriteInfo;


/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrQReadMWordsMinus1 (IxQMgrQId qId,
			        UINT32 *entry)
 *
 * @brief This function reads the remaining of the q entry
 *        for queues configured with many words.
 *        (the first word of the entry is already read 
 *        in the inlined function and the entry pointer already
 *        incremented
 *
 * @param  qId @ref IxQMgrQId [in]  - the queue identifier.
 * @param  *entry UINT32 [out] - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue
 *
 */
PUBLIC IX_STATUS
ixQMgrQReadMWordsMinus1 (IxQMgrQId qId,
			 UINT32 *entry);



/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQRead (IxQMgrQId qId,
	     UINT32 *entry)
 *
 * @brief Fast read of an entry from a queue.
 *
 * This function is a heavily streamlined version of ixQMgrQReadWithChecks(),
 * but performs essentially the same task.  It reads an entire entry from a
 * queue, returning it in entry which must be a pointer to a previously
 * allocated array of sufficient size to hold an entry.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any statistics.
 * Also, it does not check that the queue specified by qId has been configured.
 * or is in range. It simply reads an entry from the queue, and checks for
 * underflow.
 *
 * @note - IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained.
 *
 * @param  qId @ref IxQMgrQId [in] - the queue identifier.
 * @param  *entry UINT32 [out] - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue
 *
 */
#ifdef NO_INLINE_APIS
PUBLIC IX_STATUS
ixQMgrQRead (IxQMgrQId qId,
             UINT32 *entryPtr);
#else 
extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
extern IX_STATUS ixQMgrQReadMWordsMinus1 (IxQMgrQId qId, UINT32 *entryPtr);

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQRead (IxQMgrQId qId,
             UINT32 *entryPtr);
#endif

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQRead (IxQMgrQId qId,
	     UINT32 *entryPtr)
#ifdef NO_INLINE_APIS
    ;
#else
{
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 entry, entrySize;

    /* get a new entry */
    entrySize = infoPtr->qEntrySizeInWords;
    entry = IX_QMGR_INLINE_READ_LONG(infoPtr->qAccRegAddr);

    if (entrySize != IX_QMGR_Q_ENTRY_SIZE1)
    {	
	*entryPtr = entry;
	/* process the remaining part of the entry */
	return ixQMgrQReadMWordsMinus1(qId, entryPtr);
    }

    /* underflow is available for lower queues only */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	/* the counter of queue entries is decremented. In happy 
	 * day scenario there are many entries in the queue
	 * and the counter does not reach zero.
	 */
 	if (infoPtr->qReadCount-- == 0)
	{
	    /* There is maybe no entry in the queue
	     * qReadCount is now negative, but will be corrected before
	     * the function returns.
	     */
	    UINT32 qPtrs; /* queue internal pointers */

	    /* when a queue is empty, the hw guarantees to return 
	     * a null value. If the value is not null, the queue is
	     * not empty.
	     */
	    if (entry == 0)
	    {
		/* get the queue status */
		UINT32 status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr);
	
		/* check the underflow status */
		if (status & infoPtr->qUflowStatBitMask)
		{
		    /* the queue is empty 
		    *  clear the underflow status bit if it was set 
		    */
		    IX_QMGR_INLINE_WRITE_LONG(infoPtr->qUOStatRegAddr,
					 status & ~infoPtr->qUflowStatBitMask);
		    *entryPtr = 0;
		    infoPtr->qReadCount = 0;
		    return IX_QMGR_Q_UNDERFLOW;
		}
	    }
	    /* store the result */
	    *entryPtr = entry;

	    /* No underflow occured : someone is filling the queue
	     * or the queue contains null entries.
	     * The current counter needs to be
	     * updated from the current number of entries in the queue
	     */

	    /* get snapshot of queue pointers */
	    qPtrs = IX_QMGR_INLINE_READ_LONG(infoPtr->qConfigRegAddr);

	    /* Mod subtraction of pointers to get number of words in Q. */
	    qPtrs = (qPtrs - (qPtrs >> 7)) & 0x7f; 
  
	    if (qPtrs == 0)
	    {
		/* no entry in the queue */
		infoPtr->qReadCount = 0;
	    }
	    else
	    {
		/* convert the number of words inside the queue
		 * to a number of entries 
		 */
		infoPtr->qReadCount = qPtrs & (infoPtr->qSizeInEntries - 1);
	    }
	    return IX_SUCCESS;
	}
    }
    *entryPtr = entry;
    return IX_SUCCESS;
}
#endif

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQBurstRead (IxQMgrQId qId,
		  UINT32 numEntries,
		  UINT32 *entries)
 *
 * @brief Read a number of entries from an AQM queue.
 *
 * This function will burst read a number of entries from the specified queue.
 * The entry size of queue is auto-detected. The function will attempt to
 * read as many entries as specified by the numEntries parameter and will
 * return an UNDERFLOW if any one of the individual entry reads fail.
 *
 * @warning
 * IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained, hence there is a potential for
 * silent failure here. This function must be used with caution.
 *
 * @note
 * This function is intended for fast draining of queues, so to make it
 * as efficient as possible, it has the following features:
 * - This function is inlined, to reduce unnecessary function call overhead.
 * - It does not perform any parameter checks, or update any statistics.
 * - It does not check that the queue specified by qId has been configured.
 * - It does not check that the queue has the number of full entries that
 * have been specified to be read. It will read until it finds a NULL entry or
 * until the number of specified entries have been read.  It always checks for
 * underflow after all the reads have been performed.
 * Therefore, the client should ensure before calling this function that there
 * are enough entries in the queue to read.  ixQMgrQNumEntriesGet() will
 * provide the number of full entries in a queue.
 * ixQMgrQRead() or ixQMgrQReadWithChecks(), which only reads
 * a single queue entry per call, should be used instead if the user requires
 * checks for UNDERFLOW after each entry read.
 *
 * @param qId @ref IxQMgrQId [in]   - the queue identifier.
 * @param numEntries unsigned [in] - the number of entries to read. 
 *                     This number should be greater than 0
 * @param *entries UINT32 [out] - the word(s) read.
 *
 * @return @li IX_SUCCESS, entries were successfully read.
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue 
 *
 */
#ifdef NO_INLINE_APIS  
PUBLIC IX_STATUS
ixQMgrQBurstRead (IxQMgrQId qId,
                  UINT32 numEntries,
                  UINT32 *entries);
#else
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstRead (IxQMgrQId qId,
                  UINT32 numEntries,
                  UINT32 *entries);
#endif  /* endif NO_INLINE_APIS */

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstRead (IxQMgrQId qId,
		  UINT32 numEntries,
		  UINT32 *entries)
#ifdef NO_INLINE_APIS
;
#else
{
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 nullCheckEntry;

    if (infoPtr->qEntrySizeInWords == IX_QMGR_Q_ENTRY_SIZE1)
    {
	volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;

	/* the code is optimized to take care of data dependencies:
	 * Durig a read, there are a few cycles needed to get the 
	 * read complete. During these cycles, it is poossible to
	 * do some CPU, e.g. increment pointers and decrement 
	 * counters.
	 */

	/* fetch a queue entry */
	nullCheckEntry = IX_QMGR_INLINE_READ_LONG(infoPtr->qAccRegAddr);

	/* iterate the specified number of queue entries */ 
	while (--numEntries)
	{
	    /* check the result of the previous read */
	    if (nullCheckEntry == 0)
	    {
		/* if we read a NULL entry, stop. We have underflowed */
		break;
	    }
	    else
	    {
		/* write the entry */
		*entries = nullCheckEntry;
		/* fetch next entry */
		nullCheckEntry = IX_QMGR_INLINE_READ_LONG(qAccRegAddr);
		/* increment the write address */
		entries++;
	    }
	}
	/* write the pre-fetched entry */
	*entries = nullCheckEntry;
    }
    else
    {
	IxQMgrQEntrySizeInWords entrySizeInWords = infoPtr->qEntrySizeInWords;
	/* read the specified number of queue entries */
	nullCheckEntry = 0;
	while (numEntries--)
	{
	    UINT32 i;

	    for (i = 0; i < (UINT32)entrySizeInWords; i++)
	    {
		*entries = IX_QMGR_INLINE_READ_LONG(infoPtr->qAccRegAddr + i);
		nullCheckEntry |= *entries++;
	    }

	    /* if we read a NULL entry, stop. We have underflowed */
	    if (nullCheckEntry == 0)
	    {
		break;
	    }
	    nullCheckEntry = 0;
	}
    }

    /* reset the current read count : next access to the read function 
     * will force a underflow status check 
     */
    infoPtr->qReadCount = 0;

    /* Check if underflow occurred on the read */
    if (nullCheckEntry == 0 && qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	/* get the queue status */
	UINT32 status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr);

	if (status & infoPtr->qUflowStatBitMask)
	{
	    /* clear the underflow status bit if it was set */
	    IX_QMGR_INLINE_WRITE_LONG(infoPtr->qUOStatRegAddr,
				 status & ~infoPtr->qUflowStatBitMask);
	    return IX_QMGR_Q_UNDERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
 *
 * @brief Read an entry from a queue without moving the read pointer.
 *
 * This function inspects an entry in a queue. The entry is inspected directly
 * in AQM SRAM and is not read from queue access registers. The entry is NOT removed
 * from the queue and the read/write pointers are unchanged.
 * N.B: The queue should not be accessed when this function is called.
 *
 * @param  qId @ref IxQMgrQId [in]   - the queue identifier.
 * @param  entryIndex unsigned int [in] - index of entry in queue in the range
 *                          [0].......[current number of entries in queue].
 * @param  *entry UINT32 [out] - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully inspected.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId.
 * @return @li IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS, an entry does not exist at
 *             specified index.
 * @return @li IX_FAIL, failed to inpected the queue entry.
 */
PUBLIC IX_STATUS
ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry)
 *
 * @brief Write an entry to an AQM queue.
 *
 * This function will write the entry size number of words pointed to by entry to
 * the queue specified by qId. The queue configuration word is read to
 * determine the entry size of queue and the corresponding number of words is
 * then written to the queue.
 *
 * @note - IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier.
 * @param *entry UINT32 [in] - the word(s) to write.
 *
 * @return @li IX_SUCCESS, value was successfully written.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
PUBLIC IX_STATUS
ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry)
 *
 * @brief Fast write of an entry to a queue.
 *
 * This function is a heavily streamlined version of ixQMgrQWriteWithChecks(),
 * but performs essentially the same task.  It will write the entry size number
 * of words pointed to by entry to the queue specified by qId.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any
 * statistics. Also, it does not check that the queue specified by qId has
 * been configured. It simply writes an entry to the queue, and checks for
 * overflow.
 *
 * @note - IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained.
 *
 * @param  qId @ref IxQMgrQId [in]   - the queue identifier.
 * @param  *entry UINT32 [in] - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
#ifdef NO_INLINE_APIS
PUBLIC IX_STATUS
ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry);
#else
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry);
#endif /* NO_INLINE_APIS */

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry)
#ifdef NO_INLINE_APIS
    ;
#else
{
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 entrySize;

    /* write the entry */
    IX_QMGR_INLINE_WRITE_LONG(infoPtr->qAccRegAddr, *entry);
    entrySize = infoPtr->qEntrySizeInWords;

    if (entrySize != IX_QMGR_Q_ENTRY_SIZE1)
    {	
	/* process the remaining part of the entry */
	volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;
	while (--entrySize)
	{
	    ++entry;
	    IX_QMGR_INLINE_WRITE_LONG(++qAccRegAddr, *entry);
	}
 	entrySize = infoPtr->qEntrySizeInWords;
    }

    /* overflow is available for lower queues only */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {   
	UINT32 qSize = infoPtr->qSizeInEntries;
	/* increment the current number of entries in the queue
	 * and check for overflow 
	 */
	if (infoPtr->qWriteCount++ == qSize)
	{
	    /* the queue may have overflow */
	    UINT32 qPtrs; /* queue internal pointers */
  
	    /* get the queue status */
	    UINT32 status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr);

	    /* read the status twice because the status may 
	     * not be immediately ready after the write operation
	     */
	    if ((status & infoPtr->qOflowStatBitMask) ||
		((status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr))
		 & infoPtr->qOflowStatBitMask))
	    {
		/* the queue is full, clear the overflow status
		 *  bit if it was set 
		 */
		IX_QMGR_INLINE_WRITE_LONG(infoPtr->qUOStatRegAddr,
				     status & ~infoPtr->qOflowStatBitMask);
		infoPtr->qWriteCount = infoPtr->qSizeInEntries;
		return IX_QMGR_Q_OVERFLOW;
	    }
	    /* No overflow occured : someone is draining the queue
	     * and the current counter needs to be
	     * updated from the current number of entries in the queue
	     */

	    /* get q pointer snapshot */
	    qPtrs = IX_QMGR_INLINE_READ_LONG(infoPtr->qConfigRegAddr);

	    /* Mod subtraction of pointers to get number of words in Q. */
	    qPtrs = (qPtrs - (qPtrs >> 7)) & 0x7f; 

	    if (qPtrs == 0)
	    {
		/* the queue may be full at the time of the 
		 * snapshot. Next access will check 
		 * the overflow status again.
		 */
		infoPtr->qWriteCount = qSize;
	    }
	    else 
	    {
		/* convert the number of words to a number of entries */
		if (entrySize == IX_QMGR_Q_ENTRY_SIZE1)
		{
		    infoPtr->qWriteCount = qPtrs & (qSize - 1);
		}
		else
		{
		    infoPtr->qWriteCount = (qPtrs / entrySize) & (qSize - 1);
		}
	    }
	}
    }
    return IX_SUCCESS;
}
#endif

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries)
 *
 * @brief Write a number of entries to an AQM queue.
 *
 * This function will burst write a number of entries to the specified queue.
 * The entry size of queue is auto-detected. The function will attempt to
 * write as many entries as specified by the numEntries parameter and will
 * return an OVERFLOW if any one of the individual entry writes fail.
 *
 * @warning
 * IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained, hence there is a potential for
 * silent failure here. This function must be used with caution.
 *
 * @note
 * This function is intended for fast population of queues, so to make it
 * as efficient as possible, it has the following features:
 * - This function is inlined, to reduce unnecessary function call overhead.
 * - It does not perform any parameter checks, or update any statistics.
 * - It does not check that the queue specified by qId has been configured.
 * - It does not check that the queue has enough free space to hold the entries
 * before writing, and only checks for overflow after all writes have been
 * performed.  Therefore, the client should ensure before calling this function
 * that there is enough free space in the queue to hold the number of entries
 * to be written.  ixQMgrQWrite() or ixQMgrQWriteWithChecks(), which only writes
 * a single queue entry per call, should be used instead if the user requires
 * checks for OVERFLOW after each entry written.
 *
 * @param qId @ref IxQMgrQId [in]   - the queue identifier.
 * @param numEntries unsigned [in] - the number of entries to write.
 * @param *entries UINT32 [in]  - the word(s) to write.
 *
 * @return @li IX_SUCCESS, value was successfully written.
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
#ifdef NO_INLINE_APIS
PUBLIC IX_STATUS
ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries);
#else
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries);
#endif /* NO_INLINE_APIS */

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries)
#ifdef NO_INLINE_APIS
;
#else
{
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 status;

    /* update the current write count */
    infoPtr->qWriteCount += numEntries;

    if (infoPtr->qEntrySizeInWords == IX_QMGR_Q_ENTRY_SIZE1)
    {
	volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;
	while (numEntries--)
	{
	    IX_QMGR_INLINE_WRITE_LONG(qAccRegAddr, *entries);
	    entries++;
	}
    }
    else
    {
	IxQMgrQEntrySizeInWords entrySizeInWords = infoPtr->qEntrySizeInWords;
	UINT32 i;

	/* write each queue entry */
	while (numEntries--)
	{
	    /* write the queueEntrySize number of words for each entry */
	    for (i = 0; i < (UINT32)entrySizeInWords; i++)
	    {
		IX_QMGR_INLINE_WRITE_LONG((infoPtr->qAccRegAddr + i), *entries);
		entries++;
	    }
	}
    }

    /* check if the write count overflows */
    if (infoPtr->qWriteCount > infoPtr->qSizeInEntries)
    {
	/* reset the current write count */
	infoPtr->qWriteCount = infoPtr->qSizeInEntries;
    }

    /* Check if overflow occurred on the write operation */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	/* get the queue status */
	status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr);

	/* read the status twice because the status may 
	 * not be ready at the time of the write
	 */
	if ((status & infoPtr->qOflowStatBitMask) ||
	    ((status = IX_QMGR_INLINE_READ_LONG(infoPtr->qUOStatRegAddr))
	     & infoPtr->qOflowStatBitMask))
	{
	    /* clear the underflow status bit if it was set */
	    IX_QMGR_INLINE_WRITE_LONG(infoPtr->qUOStatRegAddr,
				 status & ~infoPtr->qOflowStatBitMask);
	    return IX_QMGR_Q_OVERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQPoke (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
 *
 * @brief Write an entry to a queue without moving the write pointer.
 *
 * This function modifies an entry in a queue. The entry is modified directly
 * in AQM SRAM and not using the queue access registers. The entry is NOT added to the
 * queue and the read/write pointers are unchanged.
 * N.B: The queue should not be accessed when this function is called.
 *
 * @param qId @ref IxQMgrQId [in]  - the queue identifier.
 * @param  entryIndex unsigned int [in] - index of entry in queue in the range
 *                          [0].......[current number of entries in queue].
 * @param  *entry UINT32 [in] - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully modified.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId.
 * @return @li IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS, an entry does not exist at
 *             specified index.
 * @return @li IX_FAIL, failed to modify the queue entry.
 */
PUBLIC IX_STATUS
ixQMgrQPoke (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntries)
 *
 * @brief Get a snapshot of the number of entries in a queue.
 *
 * This function gets the number of entries in a queue.
 *
 * @param qId @ref IxQMgrQId [in] qId - the queue idenfifier
 * @param *numEntries unsigned [out] - the number of entries in a queue
 *
 * @return @li IX_SUCCESS, got the number of entries for the queue
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_WARNING, could not determine num entries at this time
 *
 */
PUBLIC IX_STATUS
ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntries);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus)
 *
 * @brief Get a queues status.
 *
 * This function reads the specified queues status. A queues status is defined
 * by its status flags. For queues 0-31 these flags are E,NE,NF,F. For
 * queues 32-63 these flags are NE and F.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier.
 * @param &qStatus @ref IxQMgrQStatus [out] - the status of the specified queue.
 *
 * @return @li IX_SUCCESS, queue status was successfully read.
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter.
 *
 */
PUBLIC IX_STATUS
ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus)
 *
 * @brief Fast get of a queue's status.
 *
 * This function is a streamlined version of ixQMgrQStatusGetWithChecks(), but
 * performs essentially the same task.  It reads the specified queue's status.
 * A queues status is defined by its status flags. For queues 0-31 these flags
 * are E,NE,NF,F. For queues 32-63 these flags are NE and F.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any
 * statistics.  Also, it does not check that the queue specified by qId has
 * been configured.  It simply reads the specified queue's status.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier.
 * @param *qStatus @ref IxQMgrQStatus [out] - the status of the specified queue.
 *
 * @return @li void.
 *
 */

#ifdef NO_INLINE_APIS
PUBLIC IX_STATUS
ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus);
#else  
extern UINT32 ixQMgrAqmIfQueLowStatRegAddr[];
extern UINT32 ixQMgrAqmIfQueLowStatBitsOffset[];
extern UINT32 ixQMgrAqmIfQueLowStatBitsMask;
extern UINT32 ixQMgrAqmIfQueUppStat0RegAddr;
extern UINT32 ixQMgrAqmIfQueUppStat1RegAddr;
extern UINT32 ixQMgrAqmIfQueUppStat0BitMask[];
extern UINT32 ixQMgrAqmIfQueUppStat1BitMask[];

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus);
#endif  /* endif NO_INLINE_APIS */

IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus)
#ifdef NO_INLINE_APIS
    ;
#else
{
    /* read the status of a queue in the range 0-31 */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	volatile UINT32 *lowStatRegAddr = (UINT32*)ixQMgrAqmIfQueLowStatRegAddr[qId];

	UINT32 lowStatBitsOffset = ixQMgrAqmIfQueLowStatBitsOffset[qId];
	UINT32 lowStatBitsMask   = ixQMgrAqmIfQueLowStatBitsMask;

	/* read the status register for this queue */
	*qStatus = IX_QMGR_INLINE_READ_LONG(lowStatRegAddr);

	/* mask out the status bits relevant only to this queue */
	*qStatus = (*qStatus >> lowStatBitsOffset) & lowStatBitsMask;

    }
    else /* read status of a queue in the range 32-63 */
    {

	volatile UINT32 *qNearEmptyStatRegAddr = (UINT32*)ixQMgrAqmIfQueUppStat0RegAddr;
	volatile UINT32 *qFullStatRegAddr      = (UINT32*)ixQMgrAqmIfQueUppStat1RegAddr;
	int maskIndex = qId - IX_QMGR_MIN_QUEUPP_QID;
	UINT32 qNearEmptyStatBitMask = ixQMgrAqmIfQueUppStat0BitMask[maskIndex];
	UINT32 qFullStatBitMask      = ixQMgrAqmIfQueUppStat1BitMask[maskIndex];

	/* Reset the status bits */
	*qStatus = 0;

	/* Check if the queue is nearly empty */
	if (IX_QMGR_INLINE_READ_LONG(qNearEmptyStatRegAddr) & qNearEmptyStatBitMask)
	{
	    *qStatus |= IX_QMGR_Q_STATUS_NE_BIT_MASK;
	}

	/* Check if the queue is full */
	if (IX_QMGR_INLINE_READ_LONG(qFullStatRegAddr) & qFullStatBitMask)
	{
	    *qStatus |= IX_QMGR_Q_STATUS_F_BIT_MASK;
	}
    }
    return IX_SUCCESS;
}
#endif

/* ------------------------------------------------------------
   Queue dispatch related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority)
 *
 * @brief Set the dispatch priority of a queue.
 *
 * This function is called to set the dispatch priority of queue. The effect of
 * this function is to add a priority change request to a queue. This queue is
 * serviced by @a ixQMgrDispatcherLoopRunA0, @a ixQMgrDispatcherLoopRunB0 or 
 * @a ixQMgrDispatcherLoopRunB0LLP.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param priority @ref IxQMgrPriority [in] - the new queue dispatch priority
 *
 * @return @li IX_SUCCESS, priority change request is queued
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_Q_INVALID_PRIORITY, specified priority is invalid
 *
 */
PUBLIC IX_STATUS
ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority);
/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationEnable (IxQMgrQId qId,
			  IxQMgrSourceId sourceId)
 *
 * @brief Enable notification on a queue for a specified queue source flag.
 *
 * This function is called by a client of the QMgr to enable notifications on a
 * specified condition.
 * If the condition for the notification is set after the client has called this
 * function but before the function has enabled the interrupt source, then the
 * notification will not occur.
 * For queues 32-63 the notification source is fixed to the NE(Nearly Empty) flag
 * and cannot be changed so the sourceId parameter is ignored for these queues.
 * The status register is read before the notofication is enabled and is read again
 * after the notification has been enabled, if they differ then the warning status
 * is returned.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param sourceId @ref IxQMgrSourceId [in] - the interrupt src condition identifier
 *
 * @return @li IX_SUCCESS, the interrupt has been enabled for the specified source
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_INVALID_INT_SOURCE_ID, interrupt source invalid for this queue
 * @return @li IX_QMGR_WARNING, the status register may not be constistent
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationEnable (IxQMgrQId qId,
			  IxQMgrSourceId sourceId);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationDisable (IxQMgrQId qId)
 *
 * @brief Disable notifications on a queue.
 *
 * This function is called to disable notifications on a specified queue.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 *
 * @return @li IX_SUCCESS, the interrupt has been disabled
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationDisable (IxQMgrQId qId);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherLoopRunA0 (IxQMgrDispatchGroup group)
 *
 * @brief Run the callback dispatcher.
 *
 * This function runs the dispatcher for a group of queues.
 * Callbacks are made for interrupts that have occurred on queues within
 * the group that have registered callbacks. The order in which queues are
 * serviced depends on the queue priorities set by the client.
 * This function may be called from interrupt or task context.
 * For optimisations that were introduced in IXP42X B0 and supported IXP46X
 * the @a ixQMgrDispatcherLoopRunB0, or @a ixQMgrDispatcherLoopRunB0LLP 
 * should be used.
 *
 * This function is not re-entrant.
 *
 * @param group @ref IxQMgrDispatchGroup [in] - the group of queues over which the
 *                                        dispatcher will run
 *
 * @return @li void
 *
 * @note This function may be called from interrupt or task context.
 * However, for optimal performance the choice of context depends also on the
 * operating system used.
 *
 */
PUBLIC void
ixQMgrDispatcherLoopRunA0 (IxQMgrDispatchGroup group);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherLoopRunB0 (IxQMgrDispatchGroup group)
 *
 * @brief Run the callback dispatcher.
 *
 * The enhanced version of @a ixQMgrDispatcherLoopRunA0 that is optimised for
 * features introduced in IXP42X B0 silicon and supported on IXP46X. 
 * This is the default dispatcher for IXP42X B0 and IXP46X silicon. 
 * The function runs the dispatcher for a group of queues.
 * Callbacks are made for interrupts that have occurred on queues within
 * the group that have registered callbacks. The order in which queues are
 * serviced depends on the queue priorities set by the client.
 * This  function may be called from interrupt or task context.
 *
 * This function is not re-entrant.
 *
 * @param group @ref IxQMgrDispatchGroup [in] - the group of queues over which the
 *                                        dispatcher will run
 *
 * @return @li void
 *
 *
 * @note This function may be called from interrupt or task context.
 * However, for optimal performance the choice of context depends also on the
 * operating system used.
 *
 */
PUBLIC void
ixQMgrDispatcherLoopRunB0 (IxQMgrDispatchGroup group);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherLoopRunB0LLP (IxQMgrDispatchGroup group)
 *
 * @brief Run the callback dispatcher.
 *
 * This is a version of the optimised dispatcher for IXP42X B0 and IXP46X, 
 * @a ixQMgrDispatcherLoopRunB0, with added support for livelock prevention. 
 * This dispatcher will only be used for the IXP42X B0 or IXP46X silicon if 
 * feature control indicates that IX_FEATURECTRL_ORIGB0_DISPATCHER is set to   
 * IX_FEATURE_CTRL_SWCONFIG_DISABLED. Otherwise the @a ixQMgrDispatcherLoopRunB0 
 * dispatcher will be used (Default). 
 *
 * When this dispatcher notifies for a queue that is type
 * IX_QMGR_TYPE_REALTIME_PERIODIC, notifications for queues that are set
 * as type IX_QMGR_REALTIME_SPORADIC are not processed and disabled.
 * This helps prevent any tasks resulting from the notification of the 
 * IX_QMGR_TYPE_REALTIME_PERIODIC type queue to being subject to livelock.
 * The function runs the dispatcher for a group of queues.
 * Callbacks are made for interrupts that have occurred on queues within
 * the group that have registered callbacks. The order in which queues are
 * serviced depends on their type along with the  queue priorities set by the 
 * client. This function may be called from interrupt or task context.
 *
 * This function is not re-entrant.
 *
 * @param group @ref IxQMgrDispatchGroup [in] - the group of queues over which 
 *                                        the dispatcher will run
 *
 * @return @li void
 *
 * @note This function may be called from interrupt or task context.
 * However, for optimal performance the choice of context depends also on the
 * operating system used.
 *
 */
PUBLIC void
ixQMgrDispatcherLoopRunB0LLP (IxQMgrDispatchGroup group);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId)
 *
 * @brief Set the notification callback for a queue.
 *
 * This function sets the callback for the specified queue. This callback will
 * be called by the dispatcher, and may be called in the context of a interrupt
 * If callback has a value of NULL the previously registered callback, if one
 * exists will be unregistered.
 *
 * @param qId @ref IxQMgrQId [in] - the queue idenfifier
 * @param callback @ref IxQMgrCallback  [in] - the callback registered for this queue
 * @param callbackId @ref IxQMgrCallbackId [in] - the callback identifier
 *
 * @return @li IX_SUCCESS, the callback for the specified queue has been set
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrDispatcherLoopGet (IxQMgrDispatcherFuncPtr *qDispatcherFuncPtr)
 *
 * @brief Get QMgr DispatcherLoopRun for respective silicon device
 *
 * This function gets a function pointer to ixQMgrDispatcherLoopRunA0() for IXP42X A0
 * Silicon. If the IXP42X B0 or 46X Silicon, the default is the ixQMgrDispatcherLoopRunB0()
 * function, however if live lock prevention is enabled a function pointer to
 * ixQMgrDispatcherLoopRunB0LLP() is given.
 *
 * @param *qDispatchFuncPtr @ref IxQMgrDispatcherFuncPtr [out]  - 
 *              the function pointer of QMgr Dispatcher
 *
 */
PUBLIC void
ixQMgrDispatcherLoopGet (IxQMgrDispatcherFuncPtr *qDispatcherFuncPtr);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrStickyInterruptRegEnable(void)
 *
 * @brief Enable AQM's sticky interrupt register behaviour only available
 *        on B0 Silicon. 
 * 
 * When AQM's sticky interrupt register is enabled, interrupt register bit will
 * only be cleared when a '1' is written to interrupt register bit and the
 * interrupting condition is satisfied, i.e.queue condition does not exist.
 * 
 * @note This function must be called before any queue is enabled.
 *       Calling this function after queue is enabled will cause
 *       undefined results. 
 *
 * @return none
 *
 */
PUBLIC void
ixQMgrStickyInterruptRegEnable(void);


/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrCallbackTypeSet(IxQMgrQId qId,
                             IxQMgrType type)
 *
 * @brief Set the Callback Type of a queue.
 *
 * This function is only used for live lock prevention.
 * This function allows the callback type of a queue to be set. The default for
 * all queues is IX_QMGR_TYPE_REALTIME_OTHER. Setting the type to
 * IX_QMGR_TYPE_REALTIME_SPORADIC means that this queue will have it's 
 * notifications disabled while there is a task associated with a 
 * queue of type IX_QMGR_TYPE_REALTIME_PERIODIC running. As live lock
 * prevention operates on lower queues, this function should
 * be called for lower queues only.
 * This function is not re-entrant.  
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param type @ref IxQMgrType [in] - the type of callback
 *
 * @return @li IX_SUCCESS, successfully set callback type for the queue entry
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s).
 *
 */
PUBLIC IX_STATUS
ixQMgrCallbackTypeSet(IxQMgrQId qId,
                      IxQMgrType type);

/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrCallbackTypeGet(IxQMgrQId qId,
                             IxQMgrType *type)
 *
 * @brief Get the Callback Type of a queue.
 *
 * This function allows the callback type of a queue to be got. As live lock
 * prevention operates on lower queues, this function should
 * be called for lower queues only.
 * This function is re-entrant.
 *
 * @param qId @ref IxQMgrQId [in] - the queue identifier
 * @param *type @ref IxQMgrType [out] - the type of callback
 *
 * @return @li IX_SUCCESS, successfully set callback type for the queue entry
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s)
 *
 */
PUBLIC IX_STATUS
ixQMgrCallbackTypeGet(IxQMgrQId qId,
                      IxQMgrType *type);

/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrPeriodicDone(void)
 *
 * @brief Indicate that the Periodic task is completed for LLP
 *
 * This function is used as part of live lock prevention. 
 * A periodic task is a task that results from a queue that 
 * is set as type IX_QMGR_TYPE_REALTIME_PERIODIC. This function 
 * should be called to indicate to the dispatcher that the
 * the periodic task is completed. This ensures that the notifications
 * for queues set as type sporadic queues are re-enabled.
 * This function is re-entrant.
 *
 */
PUBLIC void
ixQMgrPeriodicDone(void);


/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrLLPShow(int resetStats)
 *
 * @brief Print out the live lock prevention statistics when in debug mode.
 *
 * This function prints out statistics related to the livelock. These
 * statistics are only collected in debug mode.
 * This function is not re-entrant.
 *
 * @param resetStats @ref int [in] - if set the the stats are reset.
 *
 */
PUBLIC void
ixQMgrLLPShow(int resetStats);


#endif /* IXQMGR_H */

/**
 * @} defgroup IxQMgrAPI
 */


