/**
 * @file IxNpeMhMacros_p.h
 *
 * @author Intel Corporation
 * @date 21 Jan 2002
 *
 * @brief This file contains the macros for the IxNpeMh component.
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
 * @defgroup IxNpeMhMacros_p IxNpeMhMacros_p
 *
 * @brief Macros for the IxNpeMh component.
 * 
 * @{
 */

#ifndef IXNPEMHMACROS_P_H
#define IXNPEMHMACROS_P_H

/* if we are running as a unit test */
#ifdef IX_UNIT_TEST
#undef NDEBUG
#endif /* #ifdef IX_UNIT_TEST */

#include "IxOsal.h"

/*
 * #defines for function return types, etc.
 */

#define IX_NPEMH_SHOW_TEXT_WIDTH (40) /**< text width for stats display */
#define IX_NPEMH_SHOW_STAT_WIDTH (10) /**< stat width for stats display */

/**
 * @def IX_NPEMH_SHOW
 *
 * @brief Macro for displaying a stat preceded by a textual description.
 */

#define IX_NPEMH_SHOW(TEXT, STAT) \
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, \
               "%-40s: %10d\n", (int) TEXT, (int) STAT, 0, 0, 0, 0)

/*
 * Prototypes for interface functions.
 */

/**
 * @typedef IxNpeMhTraceTypes
 *
 * @brief Enumeration defining IxNpeMh trace levels
 */

typedef enum
{
    IX_NPEMH_TRACE_OFF     = IX_OSAL_LOG_LVL_NONE,    /**< no trace */
    IX_NPEMH_WARNING       = IX_OSAL_LOG_LVL_WARNING, /**< warning */
    IX_NPEMH_DEBUG         = IX_OSAL_LOG_LVL_MESSAGE, /**< debug */
    IX_NPEMH_FN_ENTRY_EXIT = IX_OSAL_LOG_LVL_DEBUG3   /**< function entry/exit */
} IxNpeMhTraceTypes;

#ifdef IX_UNIT_TEST
#define IX_NPEMH_TRACE_LEVEL (IX_NPEMH_FN_ENTRY_EXIT) /**< trace level */
#else
#define IX_NPEMH_TRACE_LEVEL (IX_NPEMH_TRACE_OFF) /**< trace level */
#endif

/**
 * @def IX_NPEMH_TRACE0
 *
 * @brief Trace macro taking 0 arguments.
 */

#define IX_NPEMH_TRACE0(LEVEL, STR) \
    IX_NPEMH_TRACE6(LEVEL, STR, 0, 0, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE1
 *
 * @brief Trace macro taking 1 argument.
 */

#define IX_NPEMH_TRACE1(LEVEL, STR, ARG1) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, 0, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE2
 *
 * @brief Trace macro taking 2 arguments.
 */

#define IX_NPEMH_TRACE2(LEVEL, STR, ARG1, ARG2) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE3
 *
 * @brief Trace macro taking 3 arguments.
 */

#define IX_NPEMH_TRACE3(LEVEL, STR, ARG1, ARG2, ARG3) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE4
 *
 * @brief Trace macro taking 4 arguments.
 */

#define IX_NPEMH_TRACE4(LEVEL, STR, ARG1, ARG2, ARG3, ARG4) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, 0, 0)

/**
 * @def IX_NPEMH_TRACE5
 *
 * @brief Trace macro taking 5 arguments.
 */

#define IX_NPEMH_TRACE5(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5, 0)

/**
 * @def IX_NPEMH_TRACE6
 *
 * @brief Trace macro taking 6 arguments.
 */

#define IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
{ \
    if (LEVEL <= IX_NPEMH_TRACE_LEVEL) \
    { \
        (void) ixOsalLog (LEVEL, IX_OSAL_LOG_DEV_STDOUT, (STR), \
                          (int)(ARG1), (int)(ARG2), (int)(ARG3), \
                          (int)(ARG4), (int)(ARG5), (int)(ARG6)); \
    } \
}

/**
 * @def IX_NPEMH_ERROR_REPORT
 *
 * @brief Error reporting facility.
 */

#define IX_NPEMH_ERROR_REPORT(STR) \
{ \
    (void) ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, \
		      (STR), 0, 0, 0, 0, 0, 0); \
}

/* if we are running on XScale, i.e. real environment */
#if CPU==XSCALE

/**
 * @def IX_NPEMH_REGISTER_READ
 *
 * @brief This macro reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ(registerAddress, value) \
{ \
    *value = IX_OSAL_READ_LONG(registerAddress); \
}

/**
 * @def IX_NPEMH_REGISTER_READ_BITS
 *
 * @brief This macro partially reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ_BITS(registerAddress, value, mask) \
{ \
    *value = (IX_OSAL_READ_LONG(registerAddress) & mask); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE
 *
 * @brief This macro writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE(registerAddress, value) \
{ \
    IX_OSAL_WRITE_LONG(registerAddress, value); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE_BITS
 *
 * @brief This macro partially writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE_BITS(registerAddress, value, mask) \
{ \
    UINT32 orig = IX_OSAL_READ_LONG(registerAddress); \
    orig &= (~mask); \
    orig |= (value & mask); \
    IX_OSAL_WRITE_LONG(registerAddress, orig); \
}


/* if we are running as a unit test */
#else /* #if CPU==XSCALE */

#include "IxNpeMhTestRegister.h"

/**
 * @def IX_NPEMH_REGISTER_READ
 *
 * @brief This macro reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ(registerAddress, value) \
{ \
    ixNpeMhTestRegisterRead (registerAddress, value); \
}

/**
 * @def IX_NPEMH_REGISTER_READ_BITS
 *
 * @brief This macro partially reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ_BITS(registerAddress, value, mask) \
{ \
    ixNpeMhTestRegisterReadBits (registerAddress, value, mask); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE
 *
 * @brief This macro writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE(registerAddress, value) \
{ \
    ixNpeMhTestRegisterWrite (registerAddress, value); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE_BITS
 *
 * @brief This macro partially writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE_BITS(registerAddress, value, mask) \
{ \
    ixNpeMhTestRegisterWriteBits (registerAddress, value, mask); \
}

#endif /* #if CPU==XSCALE */

#endif /* IXNPEMHMACROS_P_H */

/**
 * @} defgroup IxNpeMhMacros_p
 */
