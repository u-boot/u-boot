/**
 * @file IxNpeDlMacros_p.h
 *
 * @author Intel Corporation
 * @date 21 January 2002
 *
 * @brief This file contains the macros for the IxNpeDl component.
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
 * @defgroup IxNpeDlMacros_p IxNpeDlMacros_p
 *
 * @brief Macros for the IxNpeDl component.
 * 
 * @{
 */

#ifndef IXNPEDLMACROS_P_H
#define IXNPEDLMACROS_P_H


/*
 * Put the user defined include files required.
 */
#if (CPU != XSCALE)
/* To support IxNpeDl unit tests... */
#include <stdio.h>
#include "test/IxNpeDlTestReg.h"

#else   
#include "IxOsal.h"

#endif


/*
 * Typedefs
 */

/**
 * @typedef IxNpeDlTraceTypes
 * @brief Enumeration defining IxNpeDl trace levels
 */
typedef enum
{
    IX_NPEDL_TRACE_OFF,     /**< no trace */
    IX_NPEDL_DEBUG,         /**< debug */
    IX_NPEDL_FN_ENTRY_EXIT  /**< function entry/exit */
} IxNpeDlTraceTypes;


/*
 * #defines and macros.
 */

/* Implementation of the following macros for use with IxNpeDl unit test code */
#if (CPU != XSCALE)


/**
 * @def IX_NPEDL_TRACE_LEVEL
 *
 * @brief IxNpeDl debug trace level
 */
#define IX_NPEDL_TRACE_LEVEL IX_NPEDL_FN_ENTRY_EXIT

/**
 * @def IX_NPEDL_ERROR_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro simply prints the error string passed.
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_ERROR_REPORT(STR) printf ("IxNpeDl ERROR: %s\n", (STR));

/**
 * @def IX_NPEDL_WARNING_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro simply prints the error string passed.
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_WARNING_REPORT(STR) printf ("IxNpeDl WARNING: %s\n", (STR));

/**
 * @def IX_NPEDL_TRACE0
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, for no arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE0(LEVEL, STR) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf ((STR)); \
        printf ("\n"); \
    } \
}

 /**
 * @def IX_NPEDL_TRACE1
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 1 argument
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported.
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE1(LEVEL, STR, ARG1) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf (STR, ARG1); \
        printf ("\n"); \
    } \
}

/**
 * @def IX_NPEDL_TRACE2
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 2 arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 * @param argType [in] ARG2 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE2(LEVEL, STR, ARG1, ARG2) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf (STR, ARG1, ARG2); \
        printf ("\n"); \
    } \
}


/**
 * @def IX_NPEDL_REG_WRITE
 *
 * @brief Mechanism for writing to a memory-mapped register
 *
 * @param UINT32 [in] base   - Base memory address for this NPE's registers
 * @param UINT32 [in] offset - Offset from base memory address
 * @param UINT32 [in] value  - Value to write to register
 *
 * This macro calls a function from Unit Test code to write a register.  This
 * allows extra flexibility for unit testing of the IxNpeDl component.
 *
 * @return none
 */
#define IX_NPEDL_REG_WRITE(base, offset, value) \
{ \
    ixNpeDlTestRegWrite (base, offset, value); \
}


/**
 * @def IX_NPEDL_REG_READ
 *
 * @brief Mechanism for reading from a memory-mapped register
 *
 * @param UINT32 [in] base     - Base memory address for this NPE's registers
 * @param UINT32 [in] offset   - Offset from base memory address
 * @param UINT32 *[out] value  - Value read from register
 *
 * This macro calls a function from Unit Test code to read a register.  This
 * allows extra flexibility for unit testing of the IxNpeDl component.
 *
 * @return none
 */
#define IX_NPEDL_REG_READ(base, offset, value) \
{ \
    ixNpeDlTestRegRead (base, offset, value); \
}


/* Implementation of the following macros when integrated with IxOsal */
#else  /* #if (CPU != XSCALE) */


/**
 * @def IX_NPEDL_TRACE_LEVEL
 *
 * @brief IxNpeDl debug trace level
 */
#define IX_NPEDL_TRACE_LEVEL IX_NPEDL_DEBUG


/**
 * @def IX_NPEDL_ERROR_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro is used to report IxNpeDl software errors.
 *
 * @return none
 */
#define IX_NPEDL_ERROR_REPORT(STR) \
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, STR, 0, 0, 0, 0, 0, 0);

/**
 * @def IX_NPEDL_WARNING_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software warnings
 *
 * @param char* [in] STR - Warning string to report
 *
 * This macro is used to report IxNpeDl software warnings.
 *
 * @return none
 */
#define IX_NPEDL_WARNING_REPORT(STR) \
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT, STR, 0, 0, 0, 0, 0, 0);


/**
 * @def IX_NPEDL_TRACE0
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, for no arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 *
 * This macro simply prints the trace string passed, if the level is supported.
 *
 * @return none
 */
#define IX_NPEDL_TRACE0(LEVEL, STR) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT, STR, 0, 0, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, STR, 0, 0, 0, 0, 0, 0); \
        } \
    } \
}

/**
 * @def IX_NPEDL_TRACE1
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 1 argument
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 *
 * @return none
 */
#define IX_NPEDL_TRACE1(LEVEL, STR, ARG1) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT, STR, ARG1, 0, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, STR, ARG1, 0, 0, 0, 0, 0); \
        } \
    } \
}

/**
 * @def IX_NPEDL_TRACE2
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 2 arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 * @param argType [in] ARG2 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 *
 * @return none
 */
#define IX_NPEDL_TRACE2(LEVEL, STR, ARG1, ARG2) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT, STR, ARG1, ARG2, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, STR, ARG1, ARG2, 0, 0, 0, 0); \
        } \
    } \
}

/**
 * @def IX_NPEDL_REG_WRITE
 *
 * @brief Mechanism for writing to a memory-mapped register
 *
 * @param UINT32 [in] base   - Base memory address for this NPE's registers
 * @param UINT32 [in] offset - Offset from base memory address
 * @param UINT32 [in] value  - Value to write to register
 *
 * This macro forms the address of the register from base address + offset, and 
 * dereferences that address to write the contents of the register.
 *
 * @return none
 */
#define IX_NPEDL_REG_WRITE(base, offset, value) \
    IX_OSAL_WRITE_LONG(((base) + (offset)), (value))



/**
 * @def IX_NPEDL_REG_READ
 *
 * @brief Mechanism for reading from a memory-mapped register
 *
 * @param UINT32 [in] base    - Base memory address for this NPE's registers
 * @param UINT32 [in] offset  - Offset from base memory address
 * @param UINT32 *[out] value  - Value read from register
 *
 * This macro forms the address of the register from base address + offset, and 
 * dereferences that address to read the register contents.
 *
 * @return none
 */
#define IX_NPEDL_REG_READ(base, offset, value) \
    *(value) = IX_OSAL_READ_LONG(((base) + (offset)))

#endif  /* #if (CPU != XSCALE) */

#endif /* IXNPEDLMACROS_P_H */

/**
 * @} defgroup IxNpeDlMacros_p
 */
