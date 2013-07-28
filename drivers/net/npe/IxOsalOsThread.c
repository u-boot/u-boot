/**
 * @file IxOsalOsThread.c (eCos)
 *
 * @brief OS-specific thread implementation.
 *
 *
 * @par
 * IXP400 SW Release version 1.5
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

#include "IxOsal.h"

/* Thread attribute is ignored */
PUBLIC IX_STATUS
ixOsalThreadCreate (IxOsalThread * ptrTid,
    IxOsalThreadAttr * threadAttr, IxOsalVoidFnVoidPtr entryPoint, void *arg)
{
    return IX_SUCCESS;
}

/*
 * Start thread after given its thread handle
 */
PUBLIC IX_STATUS
ixOsalThreadStart (IxOsalThread * tId)
{
    /* Thread already started upon creation */
    return IX_SUCCESS;
}

/*
 * In Linux threadKill does not actually destroy the thread,
 * it will stop the signal handling.
 */
PUBLIC IX_STATUS
ixOsalThreadKill (IxOsalThread * tid)
{
    return IX_SUCCESS;
}

PUBLIC void
ixOsalThreadExit (void)
{
}

PUBLIC IX_STATUS
ixOsalThreadPrioritySet (IxOsalOsThread * tid, UINT32 priority)
{
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalThreadSuspend (IxOsalThread * tId)
{
    return IX_SUCCESS;

}

PUBLIC IX_STATUS
ixOsalThreadResume (IxOsalThread * tId)
{
    return IX_SUCCESS;
}
