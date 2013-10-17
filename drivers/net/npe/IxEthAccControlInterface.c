/**
 * @file IxEthAccControlInterface.c
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief IX_ETH_ACC_PUBLIC wrappers for control plane functions
 *
 * Design Notes:
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

#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxEthAcc_p.h"

PUBLIC IxOsalMutex ixEthAccControlInterfaceMutex;

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
        printf("EthAcc: (Mac) cannot enable port %d, service not initialized\n", portId);
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    /* check the context is iinitialized */
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortEnabledQueryPriv(portId, enabled);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortPromiscuousModeClearPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortPromiscuousModeSetPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortUnicastMacAddressSet(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastMacAddressSetPriv(portId, macAddr);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortUnicastMacAddressGet(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastMacAddressGetPriv(portId, macAddr);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoin(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressJoinPriv(portId, macAddr);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoinAll(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressJoinAllPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeave(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressLeavePriv(portId, macAddr);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeaveAll(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressLeaveAllPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortUnicastAddressShow(IxEthAccPortId portId)
{
    IxEthAccStatus result;
 
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastAddressShowPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC void 
ixEthAccPortMulticastAddressShow(IxEthAccPortId portId)
{
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return;
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    ixEthAccPortMulticastAddressShowPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeSet(IxEthAccPortId portId, IxEthAccDuplexMode mode)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDuplexModeSetPriv(portId, mode);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeGet(IxEthAccPortId portId, IxEthAccDuplexMode *mode)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDuplexModeGetPriv(portId, mode);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;
     
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendPaddingEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendPaddingDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendFCSEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendFCSDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxFrameAppendFCSEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxFrameAppendFCSDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccTxSchedulingDisciplineSet(IxEthAccPortId portId, IxEthAccSchedulerDiscipline sched)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccTxSchedulingDisciplineSetPriv(portId, sched);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccRxSchedulingDisciplineSet(IxEthAccSchedulerDiscipline sched)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccRxSchedulingDisciplineSetPriv(sched);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccNpeLoopbackEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortTxEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortRxEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccNpeLoopbackDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortTxDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortRxDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortMacReset(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMacResetPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}
