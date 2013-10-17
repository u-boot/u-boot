/**
 * @file IxOsalOsMsgQ.c (eCos)
 *
 * @brief OS-specific Message Queue implementation.
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

/*******************************
 * Public functions
 *******************************/
PUBLIC IX_STATUS
ixOsalMessageQueueCreate (IxOsalMessageQueue * queue,
    UINT32 msgCount, UINT32 msgLen)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueDelete (IxOsalMessageQueue * queue)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueSend (IxOsalMessageQueue * queue, UINT8 * message)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueReceive (IxOsalMessageQueue * queue, UINT8 * message)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

