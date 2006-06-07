/**
 * @file IxEthAccMac.c
 *
 * @author Intel Corporation
 * @date
 *
 * @brief  MAC control functions
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

#include "IxOsal.h"
#include "IxNpeMh.h"
#ifdef CONFIG_IXP425_COMPONENT_ETHDB
#include "IxEthDB.h"
#endif
#include "IxEthDBPortDefs.h"
#include "IxEthNpe.h"
#include "IxEthAcc.h"
#include "IxEthAccDataPlane_p.h"
#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"

/* Maximum number of retries during ixEthAccPortDisable, which
 * is approximately 10 seconds
*/
#define IX_ETH_ACC_MAX_RETRY 500

/* Maximum number of retries during ixEthAccPortDisable when expecting
 * timeout
 */
#define IX_ETH_ACC_MAX_RETRY_TIMEOUT 5

#define IX_ETH_ACC_VALIDATE_PORT_ID(portId) \
    do                                                           \
    {                                                            \
        if(!IX_ETH_ACC_IS_PORT_VALID(portId))   \
        {                                                        \
	    return IX_ETH_ACC_INVALID_PORT;                      \
        }                                                        \
    } while(0)

PUBLIC IxEthAccMacState ixEthAccMacState[IX_ETH_ACC_NUMBER_OF_PORTS];

PRIVATE UINT32 ixEthAccMacBase[IX_ETH_ACC_NUMBER_OF_PORTS];

/*Forward function declarations*/
PRIVATE void
ixEthAccPortDisableRx (IxEthAccPortId portId,
		       IX_OSAL_MBUF * mBufPtr,
		       BOOL useMultiBufferCallback);

PRIVATE void
ixEthAccPortDisableRxAndReplenish (IxEthAccPortId portId,
				   IX_OSAL_MBUF * mBufPtr,
				   BOOL useMultiBufferCallback);

PRIVATE void
ixEthAccPortDisableTxDone (UINT32 cbTag,
			   IX_OSAL_MBUF *mbuf);

PRIVATE void
ixEthAccPortDisableTxDoneAndSubmit (UINT32 cbTag,
				    IX_OSAL_MBUF *mbuf);

PRIVATE void
ixEthAccPortDisableRxCallback (UINT32 cbTag,
			       IX_OSAL_MBUF * mBufPtr,
			       UINT32 learnedPortId);

PRIVATE void
ixEthAccPortDisableMultiBufferRxCallback (UINT32 cbTag,
					  IX_OSAL_MBUF **mBufPtr);

PRIVATE IxEthAccStatus
ixEthAccPortDisableTryTransmit(UINT32 portId);

PRIVATE IxEthAccStatus
ixEthAccPortDisableTryReplenish(UINT32 portId);

PRIVATE IxEthAccStatus
ixEthAccPortMulticastMacAddressGet (IxEthAccPortId portId,
				    IxEthAccMacAddr *macAddr);

PRIVATE IxEthAccStatus
ixEthAccPortMulticastMacFilterGet (IxEthAccPortId portId,
				   IxEthAccMacAddr *macAddr);

PRIVATE void
ixEthAccMacNpeStatsMessageCallback (IxNpeMhNpeId npeId,
				    IxNpeMhMessage msg);

PRIVATE void
ixEthAccMacNpeStatsResetMessageCallback (IxNpeMhNpeId npeId,
					 IxNpeMhMessage msg);

PRIVATE void
ixEthAccNpeLoopbackMessageCallback (IxNpeMhNpeId npeId,
				    IxNpeMhMessage msg);

PRIVATE void
ixEthAccMulticastAddressSet(IxEthAccPortId portId);

PRIVATE BOOL
ixEthAccMacEqual(IxEthAccMacAddr *macAddr1,
		 IxEthAccMacAddr *macAddr2);

PRIVATE void
ixEthAccMacPrint(IxEthAccMacAddr *m);

PRIVATE void
ixEthAccMacStateUpdate(IxEthAccPortId portId);

IxEthAccStatus
ixEthAccMacMemInit(void)
{
    ixEthAccMacBase[IX_ETH_PORT_1] =
	(UINT32) IX_OSAL_MEM_MAP(IX_ETH_ACC_MAC_0_BASE,
				 IX_OSAL_IXP400_ETHA_MAP_SIZE);
    ixEthAccMacBase[IX_ETH_PORT_2] =
	(UINT32) IX_OSAL_MEM_MAP(IX_ETH_ACC_MAC_1_BASE,
				 IX_OSAL_IXP400_ETHB_MAP_SIZE);
#ifdef __ixp46X
    ixEthAccMacBase[IX_ETH_PORT_3] =
	(UINT32) IX_OSAL_MEM_MAP(IX_ETH_ACC_MAC_2_BASE,
				 IX_OSAL_IXP400_ETH_NPEA_MAP_SIZE);
    if (ixEthAccMacBase[IX_ETH_PORT_3] == 0)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_FATAL,
		  IX_OSAL_LOG_DEV_STDOUT,
		  "EthAcc: Could not map MAC I/O memory\n",
		  0, 0, 0, 0, 0 ,0);

	return IX_ETH_ACC_FAIL;
    }
#endif

    if (ixEthAccMacBase[IX_ETH_PORT_1] == 0
	|| ixEthAccMacBase[IX_ETH_PORT_2] == 0)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_FATAL,
		  IX_OSAL_LOG_DEV_STDOUT,
		  "EthAcc: Could not map MAC I/O memory\n",
		  0, 0, 0, 0, 0 ,0);

	return IX_ETH_ACC_FAIL;
    }

    return IX_ETH_ACC_SUCCESS;
}

void
ixEthAccMacUnload(void)
{
    IX_OSAL_MEM_UNMAP(ixEthAccMacBase[IX_ETH_PORT_1]);
    IX_OSAL_MEM_UNMAP(ixEthAccMacBase[IX_ETH_PORT_2]);
#ifdef __ixp46X
    IX_OSAL_MEM_UNMAP(ixEthAccMacBase[IX_ETH_PORT_3]);
    ixEthAccMacBase[IX_ETH_PORT_3] = 0;
#endif
    ixEthAccMacBase[IX_ETH_PORT_2] = 0;
    ixEthAccMacBase[IX_ETH_PORT_1] = 0;
}

IxEthAccStatus
ixEthAccPortEnablePriv(IxEthAccPortId portId)
{
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot enable port.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
        printf("EthAcc: (Mac) cannot enable port %d, port not initialized\n", portId);
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if (ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn == NULL)
    {
        /* TxDone callback not registered */
        printf("EthAcc: (Mac) cannot enable port %d, TxDone callback not registered\n", portId);
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if ((ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn == NULL)
	&& (ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackFn == NULL))
    {
        /* Receive callback not registered */
        printf("EthAcc: (Mac) cannot enable port %d, Rx callback not registered\n", portId);
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if(!ixEthAccMacState[portId].initDone)
    {
        printf("EthAcc: (Mac) cannot enable port %d, MAC address not set\n", portId);
	return (IX_ETH_ACC_MAC_UNINITIALIZED);
    }

    /* if the state is being set to what it is already at, do nothing*/
    if (ixEthAccMacState[portId].enabled)
    {
        return IX_ETH_ACC_SUCCESS;
    }

#ifdef CONFIG_IXP425_COMPONENT_ETHDB
    /* enable ethernet database for this port */
    if (ixEthDBPortEnable(portId) != IX_ETH_DB_SUCCESS)
    {
        printf("EthAcc: (Mac) cannot enable port %d, EthDB failure\n", portId);
        return IX_ETH_ACC_FAIL;
    }
#endif

    /* set the MAC core registers */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL2,
	      IX_ETH_ACC_TX_CNTRL2_RETRIES_MASK);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RANDOM_SEED,
	      IX_ETH_ACC_RANDOM_SEED_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_THRESH_P_EMPTY,
	      IX_ETH_ACC_MAC_THRESH_P_EMPTY_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_THRESH_P_FULL,
	      IX_ETH_ACC_MAC_THRESH_P_FULL_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_DEFER,
	      IX_ETH_ACC_MAC_TX_DEFER_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_1,
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_1_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_2,
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_2_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_SLOT_TIME,
	      IX_ETH_ACC_MAC_SLOT_TIME_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_BUF_SIZE_TX,
	      IX_ETH_ACC_MAC_BUF_SIZE_TX_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      IX_ETH_ACC_TX_CNTRL1_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      IX_ETH_ACC_RX_CNTRL1_DEFAULT);

    /* set the global state */
    ixEthAccMacState[portId].portDisableState = ACTIVE;
    ixEthAccMacState[portId].enabled = TRUE;

    /* rewrite the setup (including mac filtering) depending
     * on current options
     */
    ixEthAccMacStateUpdate(portId);

    return IX_ETH_ACC_SUCCESS;
}

/*
 * PortDisable local variables. They contain the intermediate steps
 * while the port is being disabled and the buffers being drained out
 * of the NPE.
 */
typedef void (*IxEthAccPortDisableRx)(IxEthAccPortId portId,
				      IX_OSAL_MBUF * mBufPtr,
				      BOOL useMultiBufferCallback);
static IxEthAccPortRxCallback
ixEthAccPortDisableFn[IX_ETH_ACC_NUMBER_OF_PORTS];
static IxEthAccPortMultiBufferRxCallback
ixEthAccPortDisableMultiBufferFn[IX_ETH_ACC_NUMBER_OF_PORTS];
static IxEthAccPortDisableRx
ixEthAccPortDisableRxTable[IX_ETH_ACC_NUMBER_OF_PORTS];
static UINT32
ixEthAccPortDisableCbTag[IX_ETH_ACC_NUMBER_OF_PORTS];
static UINT32
ixEthAccPortDisableMultiBufferCbTag[IX_ETH_ACC_NUMBER_OF_PORTS];

static IxEthAccPortTxDoneCallback
ixEthAccPortDisableTxDoneFn[IX_ETH_ACC_NUMBER_OF_PORTS];
static UINT32
ixEthAccPortDisableTxDoneCbTag[IX_ETH_ACC_NUMBER_OF_PORTS];

static UINT32
ixEthAccPortDisableUserBufferCount[IX_ETH_ACC_NUMBER_OF_PORTS];

/*
 * PortDisable private callbacks functions. They handle the user
 * traffic, and the special buffers (one for tx, one for rx) used
 * in portDisable.
 */
PRIVATE void
ixEthAccPortDisableTxDone(UINT32 cbTag,
			  IX_OSAL_MBUF *mbuf)
{
    IxEthAccPortId portId = (IxEthAccPortId)cbTag;
    volatile IxEthAccPortDisableState *txState = &ixEthAccMacState[portId].txState;

    /* check for the special mbuf used in portDisable */
    if (mbuf == ixEthAccMacState[portId].portDisableTxMbufPtr)
    {
        *txState = TRANSMIT_DONE;
    }
    else
    {
	/* increment the count of user traffic during portDisable */
	ixEthAccPortDisableUserBufferCount[portId]++;

       /* call client TxDone function */
        ixEthAccPortDisableTxDoneFn[portId](ixEthAccPortDisableTxDoneCbTag[portId], mbuf);
    }
}

PRIVATE IxEthAccStatus
ixEthAccPortDisableTryTransmit(UINT32 portId)
{
    int key;
    IxEthAccStatus status = IX_ETH_ACC_SUCCESS;
    volatile IxEthAccPortDisableState *txState = &ixEthAccMacState[portId].txState;
    /* transmit the special buffer again if it is transmitted
     * and update the txState
     * This section is protected because the portDisable context
     * run an identical code, so the system keeps transmitting at the
     * maximum rate.
     */
    key = ixOsalIrqLock();
    if (*txState == TRANSMIT_DONE)
    {
	IX_OSAL_MBUF *mbufTxPtr = ixEthAccMacState[portId].portDisableTxMbufPtr;
	*txState = TRANSMIT;
	status = ixEthAccPortTxFrameSubmit(portId,
					   mbufTxPtr,
					   IX_ETH_ACC_TX_DEFAULT_PRIORITY);
    }
    ixOsalIrqUnlock(key);

    return status;
}

PRIVATE void
ixEthAccPortDisableTxDoneAndSubmit(UINT32 cbTag,
				   IX_OSAL_MBUF *mbuf)
{
    IxEthAccPortId portId = (IxEthAccPortId)cbTag;

    /* call the callback which forwards the traffic to the client */
    ixEthAccPortDisableTxDone(cbTag, mbuf);

    /* try to transmit the buffer used in portDisable
     * if seen in TxDone
     */
    ixEthAccPortDisableTryTransmit(portId);
}

PRIVATE void
ixEthAccPortDisableRx (IxEthAccPortId portId,
		       IX_OSAL_MBUF * mBufPtr,
		       BOOL useMultiBufferCallback)
{
    volatile IxEthAccPortDisableState *rxState = &ixEthAccMacState[portId].rxState;
    IX_OSAL_MBUF *mNextPtr;

    while (mBufPtr)
    {
	mNextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBufPtr);
	IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBufPtr) = NULL;

	/* check for the special mbuf used in portDisable */
	if (mBufPtr == ixEthAccMacState[portId].portDisableRxMbufPtr)
	{
            *rxState = RECEIVE;
	}
	else
	{
	    /* increment the count of user traffic during portDisable */
	    ixEthAccPortDisableUserBufferCount[portId]++;

	    /* reset the received payload length during portDisable */
	    IX_OSAL_MBUF_MLEN(mBufPtr)    = 0;
	    IX_OSAL_MBUF_PKT_LEN(mBufPtr) = 0;

	    if (useMultiBufferCallback)
	    {
		/* call the user callback with one unchained
		 * buffer, without payload. A small array is built
		 * to be used as a parameter (the user callback expects
		 * to receive an array ended by a NULL pointer.
		 */
		IX_OSAL_MBUF *mBufPtrArray[2];

		mBufPtrArray[0] = mBufPtr;
		mBufPtrArray[1] = NULL;
		ixEthAccPortDisableMultiBufferFn[portId](
			 ixEthAccPortDisableMultiBufferCbTag[portId],
			 mBufPtrArray);
	    }
	    else
	    {
		/* call the user callback with a unchained
		 * buffer, without payload and the destination port is
		 * unknown.
		 */
		ixEthAccPortDisableFn[portId](
		      ixEthAccPortDisableCbTag[portId],
		      mBufPtr,
		      IX_ETH_DB_UNKNOWN_PORT /* port not found */);
	    }
        }

        mBufPtr = mNextPtr;
    }
}

PRIVATE IxEthAccStatus
ixEthAccPortDisableTryReplenish(UINT32 portId)
{
    int key;
    IxEthAccStatus status = IX_ETH_ACC_SUCCESS;
    volatile IxEthAccPortDisableState *rxState = &ixEthAccMacState[portId].rxState;
    /* replenish with the special buffer again if it is received
     * and update the rxState
     * This section is protected because the portDisable context
     * run an identical code, so the system keeps replenishing at the
     * maximum rate.
     */
    key = ixOsalIrqLock();
    if (*rxState == RECEIVE)
    {
	IX_OSAL_MBUF *mbufRxPtr = ixEthAccMacState[portId].portDisableRxMbufPtr;
	*rxState = REPLENISH;
	IX_OSAL_MBUF_MLEN(mbufRxPtr) = IX_ETHACC_RX_MBUF_MIN_SIZE;
	status = ixEthAccPortRxFreeReplenish(portId, mbufRxPtr);
    }
    ixOsalIrqUnlock(key);

    return status;
}

PRIVATE void
ixEthAccPortDisableRxAndReplenish (IxEthAccPortId portId,
				   IX_OSAL_MBUF * mBufPtr,
				   BOOL useMultiBufferCallback)
{
    /* call the callback which forwards the traffic to the client */
    ixEthAccPortDisableRx(portId, mBufPtr, useMultiBufferCallback);

    /* try to replenish with the buffer used in portDisable
     * if seen in Rx
     */
    ixEthAccPortDisableTryReplenish(portId);
}

PRIVATE void
ixEthAccPortDisableRxCallback (UINT32 cbTag,
			       IX_OSAL_MBUF * mBufPtr,
			       UINT32 learnedPortId)
{
    IxEthAccPortId portId = (IxEthAccPortId)cbTag;

    /* call the portDisable receive callback */
   (ixEthAccPortDisableRxTable[portId])(portId, mBufPtr, FALSE);
}

PRIVATE void
ixEthAccPortDisableMultiBufferRxCallback (UINT32 cbTag,
					  IX_OSAL_MBUF **mBufPtr)
{
    IxEthAccPortId portId = (IxEthAccPortId)cbTag;

    while (*mBufPtr)
    {
	/* call the portDisable receive callback with one buffer at a time */
	(ixEthAccPortDisableRxTable[portId])(portId, *mBufPtr++, TRUE);
    }
}

IxEthAccStatus
ixEthAccPortDisablePriv(IxEthAccPortId portId)
{
    IxEthAccStatus status = IX_ETH_ACC_SUCCESS;
    int key;
    int retry, retryTimeout;
    volatile IxEthAccPortDisableState *state = &ixEthAccMacState[portId].portDisableState;
    volatile IxEthAccPortDisableState *rxState = &ixEthAccMacState[portId].rxState;
    volatile IxEthAccPortDisableState *txState = &ixEthAccMacState[portId].txState;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot disable port.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* if the state is being set to what it is already at, do nothing */
    if (!ixEthAccMacState[portId].enabled)
    {
        return IX_ETH_ACC_SUCCESS;
    }

    *state = DISABLED;

    /* disable MAC receive first */
    ixEthAccPortRxDisablePriv(portId);

#ifdef CONFIG_IXP425_COMPONENT_ETHDB
    /* disable ethernet database for this port - It is done now to avoid
     * issuing ELT maintenance after requesting 'port disable' in an NPE
     */
    if (ixEthDBPortDisable(portId) != IX_ETH_DB_SUCCESS)
    {
	status = IX_ETH_ACC_FAIL;
        IX_ETH_ACC_FATAL_LOG("ixEthAccPortDisable: failed to disable EthDB for this port\n", 0, 0, 0, 0, 0, 0);
    }
#endif

    /* enter the critical section */
    key = ixOsalIrqLock();

    /* swap the Rx and TxDone callbacks */
    ixEthAccPortDisableFn[portId]            = ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn;
    ixEthAccPortDisableMultiBufferFn[portId] = ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackFn;
    ixEthAccPortDisableCbTag[portId]         = ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag;
    ixEthAccPortDisableMultiBufferCbTag[portId] = ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackTag;
    ixEthAccPortDisableTxDoneFn[portId]      = ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn;
    ixEthAccPortDisableTxDoneCbTag[portId]   = ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag;
    ixEthAccPortDisableRxTable[portId]       =  ixEthAccPortDisableRx;

    /* register temporary callbacks */
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn            = ixEthAccPortDisableRxCallback;
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag           = portId;

    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackFn = ixEthAccPortDisableMultiBufferRxCallback;
    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackTag = portId;

    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn  = ixEthAccPortDisableTxDone;
    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag           = portId;

    /* initialise the Rx state and Tx states */
    *txState = TRANSMIT_DONE;
    *rxState = RECEIVE;

    /* exit the critical section */
    ixOsalIrqUnlock(key);

    /* enable a NPE loopback */
    if (ixEthAccNpeLoopbackEnablePriv(portId) != IX_ETH_ACC_SUCCESS)
    {
	status = IX_ETH_ACC_FAIL;
    }

    if (status == IX_ETH_ACC_SUCCESS)
    {
	retry = 0;

	/* Step 1 : Drain Tx traffic and TxDone queues :
	 *
	 * Transmit and replenish at least once with the
	 * special buffers until both of them are seen
	 * in the callback hook
	 *
	 * (the receive callback keeps replenishing, so once we see
	 * the special Tx buffer, we can be sure that Tx drain is complete)
	 */
	ixEthAccPortDisableRxTable[portId]
	    =  ixEthAccPortDisableRxAndReplenish;
	ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn
	    = ixEthAccPortDisableTxDone;

	do
	{
	    /* keep replenishing */
	    status = ixEthAccPortDisableTryReplenish(portId);
	    if (status == IX_ETH_ACC_SUCCESS)
	    {
		/* keep transmitting */
		status = ixEthAccPortDisableTryTransmit(portId);
	    }
	    if (status == IX_ETH_ACC_SUCCESS)
	    {
		/* wait for some traffic being processed */
		ixOsalSleep(IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS);
	    }
	}
	while ((status == IX_ETH_ACC_SUCCESS)
	       && (retry++ < IX_ETH_ACC_MAX_RETRY)
	       && (*txState == TRANSMIT));

	/* Step 2 : Drain Rx traffic, RxFree and Rx queues :
	 *
	 * Transmit and replenish at least once with the
	 * special buffers until both of them are seen
	 * in the callback hook
	 * (the transmit callback keeps transmitting, and when we see
	 * the special Rx buffer, we can be sure that rxFree drain
	 * is complete)
	 *
	 * The nested loop helps to retry if the user was keeping
	 * replenishing or transmitting during portDisable.
	 *
	 * The 2 nested loops ensure more retries if user traffic is
	 * seen during portDisable : the user should not replenish
	 * or transmit while portDisable is running. However, because of
	 * the queueing possibilities in ethAcc dataplane, it is possible
	 * that a lot of traffic is left in the queues (e.g. when
	 * transmitting over a low speed link) and therefore, more
	 * retries are allowed to help flushing the buffers out.
	 */
	ixEthAccPortDisableRxTable[portId]
	    =  ixEthAccPortDisableRx;
	ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn
	    = ixEthAccPortDisableTxDoneAndSubmit;

	do
	{
	    do
	    {
		ixEthAccPortDisableUserBufferCount[portId] = 0;

		/* keep replenishing */
		status = ixEthAccPortDisableTryReplenish(portId);
		if (status == IX_ETH_ACC_SUCCESS)
		{
		    /* keep transmitting */
		    status = ixEthAccPortDisableTryTransmit(portId);
		}
		if (status == IX_ETH_ACC_SUCCESS)
		{
		    /* wait for some traffic being processed */
		    ixOsalSleep(IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS);
		}
	    }
	    while ((status == IX_ETH_ACC_SUCCESS)
		   && (retry++ < IX_ETH_ACC_MAX_RETRY)
		   && ((ixEthAccPortDisableUserBufferCount[portId] != 0)
		       || (*rxState == REPLENISH)));

	    /* After the first iteration, change the receive callbacks,
	     * to process only 1 buffer at a time
	     */
	    ixEthAccPortDisableRxTable[portId]
		= ixEthAccPortDisableRx;
	    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn
		= ixEthAccPortDisableTxDone;

	    /* repeat the whole process while user traffic is seen in TxDone
	     *
	     * The conditions to stop the loop are
	     * - Xscale has both Rx and Tx special buffers
	     *   (txState = transmit, rxState = receive)
	     * - any error in txSubmit or rxReplenish
	     * - no user traffic seen
	     * - an excessive amount of retries
	     */
	}
	while ((status == IX_ETH_ACC_SUCCESS)
	       && (retry < IX_ETH_ACC_MAX_RETRY)
	       && (*txState == TRANSMIT));

	/* check the loop exit conditions. The NPE should not hold
	 * the special buffers.
	 */
	if ((*rxState == REPLENISH) || (*txState == TRANSMIT))
	{
	    status = IX_ETH_ACC_FAIL;
	}

	if (status == IX_ETH_ACC_SUCCESS)
	{
	    /* Step 3 : Replenish without transmitting until a timeout
	     * occurs, in order to drain the internal NPE fifos
	     *
	     * we can expect a few frames srill held
	     * in the NPE.
	     *
	     * The 2 nested loops take care about the NPE dropping traffic
	     * (including loopback traffic) when the Rx queue is full.
	     *
	     * The timeout value is very conservative
	     * since the loopback used keeps replenishhing.
	     *
	     */
	    do
	    {
		ixEthAccPortDisableRxTable[portId] = ixEthAccPortDisableRxAndReplenish;
		ixEthAccPortDisableUserBufferCount[portId] = 0;
		retryTimeout = 0;
		do
		{
		    /* keep replenishing */
		    status = ixEthAccPortDisableTryReplenish(portId);
		    if (status == IX_ETH_ACC_SUCCESS)
		    {
			/* wait for some traffic being processed */
			ixOsalSleep(IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS);
		    }
		}
		while ((status == IX_ETH_ACC_SUCCESS)
		       && (retryTimeout++ < IX_ETH_ACC_MAX_RETRY_TIMEOUT));

		/* Step 4 : Transmit once. Stop replenish
		 *
		 * After the Rx timeout, we are sure that the NPE does not
		 * hold any frame in its internal NPE fifos.
		 *
		 * At this point, the NPE still holds the last rxFree buffer.
		 * By transmitting a single frame, this should unblock the
		 * last rxFree buffer. This code just transmit once and
		 * wait for both frames seen in TxDone and in rxFree.
		 *
		 */
		ixEthAccPortDisableRxTable[portId] =  ixEthAccPortDisableRx;
		status = ixEthAccPortDisableTryTransmit(portId);

		/* the NPE should immediatelyt release
		 * the last Rx buffer and the last transmitted buffer
		 * unless the last Tx frame was dropped (rx queue full)
		 */
		if (status == IX_ETH_ACC_SUCCESS)
		{
		    retryTimeout = 0;
		    do
		    {
			ixOsalSleep(IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS);
		    }
		    while ((*rxState == REPLENISH)
			   && (retryTimeout++ < IX_ETH_ACC_MAX_RETRY_TIMEOUT));
		}

		/* the NPE may have dropped the traffic because of Rx
		 * queue being full. This code ensures that the last
		 * Tx and Rx frames are both received.
		 */
	    }
	    while ((status == IX_ETH_ACC_SUCCESS)
		   && (retry++ < IX_ETH_ACC_MAX_RETRY)
		   && ((*txState == TRANSMIT)
		       || (*rxState == REPLENISH)
		       || (ixEthAccPortDisableUserBufferCount[portId] != 0)));

	    /* Step 5 : check the final states : the NPE has
	     * no buffer left, nor in Tx , nor in Rx directions.
	     */
	    if ((*rxState == REPLENISH) || (*txState == TRANSMIT))
	    {
		status = IX_ETH_ACC_FAIL;
	    }
	}

        /* now all the buffers are drained, disable NPE loopback
	 * This is done regardless of the logic to drain the queues and
	 * the internal buffers held by the NPE.
	 */
	if (ixEthAccNpeLoopbackDisablePriv(portId) != IX_ETH_ACC_SUCCESS)
	{
	    status = IX_ETH_ACC_FAIL;
	}
    }

    /* disable MAC Tx and Rx services */
    ixEthAccMacState[portId].enabled = FALSE;
    ixEthAccMacStateUpdate(portId);

    /* restore the Rx and TxDone callbacks (within a critical section) */
    key = ixOsalIrqLock();

    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn            = ixEthAccPortDisableFn[portId];
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag           = ixEthAccPortDisableCbTag[portId];
    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackFn = ixEthAccPortDisableMultiBufferFn[portId];
    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackTag = ixEthAccPortDisableMultiBufferCbTag[portId];
    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn  = ixEthAccPortDisableTxDoneFn[portId];
    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag           = ixEthAccPortDisableTxDoneCbTag[portId];

    ixOsalIrqUnlock(key);

    /* the MAC core rx/tx disable may left the MAC hardware in an
     * unpredictable state. A hw reset is executed before resetting
     * all the MAC parameters to a known value.
     */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_RESET);

    ixOsalSleep(IX_ETH_ACC_MAC_RESET_DELAY);

    /* rewrite all parameters to their current value */
    ixEthAccMacStateUpdate(portId);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_MDC_EN);

    return status;
}

IxEthAccStatus
ixEthAccPortEnabledQueryPriv(IxEthAccPortId portId, BOOL *enabled)
{
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot enable port.\n",(INT32)portId,0,0,0,0,0);

        /* Since Eth NPE is not available, port must be disabled */
        *enabled = FALSE ;
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
        /* Since Eth NPE is not available, port must be disabled */
        *enabled = FALSE ;
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    *enabled = ixEthAccMacState[portId].enabled;

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortMacResetPriv(IxEthAccPortId portId)
{
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot reset Ethernet coprocessor.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_RESET);

    ixOsalSleep(IX_ETH_ACC_MAC_RESET_DELAY);

    /* rewrite all parameters to their current value */
    ixEthAccMacStateUpdate(portId);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_MDC_EN);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortLoopbackEnable(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot enable loopback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* read register */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    /* update register */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_LOOP_EN);

    return IX_ETH_ACC_SUCCESS;
}

PRIVATE void
ixEthAccNpeLoopbackMessageCallback (IxNpeMhNpeId npeId,
				    IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

#ifndef NDEBUG
    /* Prudent to at least check the port is within range */
    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
    {
	IX_ETH_ACC_FATAL_LOG("IXETHACC:ixEthAccPortDisableMessageCallback: Illegal port: %u\n",
            (UINT32) portId, 0, 0, 0, 0, 0);

	return;
    }
#endif

    /* unlock message reception mutex */
    ixOsalMutexUnlock(&ixEthAccMacState[portId].npeLoopbackMessageLock);
}

IxEthAccStatus
ixEthAccNpeLoopbackEnablePriv(IxEthAccPortId portId)
{
    IX_STATUS npeMhStatus;
    IxNpeMhMessage message;
    IxEthAccStatus status = IX_ETH_ACC_SUCCESS;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot enable NPE loopback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* enable NPE loopback (lsb of the message contains the value 1) */
    message.data[0] = (IX_ETHNPE_SETLOOPBACK_MODE << IX_ETH_ACC_MAC_MSGID_SHL)
	| 0x01;
    message.data[1] = 0;

    npeMhStatus = ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_TO_NPE_ID(portId),
		message,
		IX_ETHNPE_SETLOOPBACK_MODE_ACK,
		ixEthAccNpeLoopbackMessageCallback,
		IX_NPEMH_SEND_RETRIES_DEFAULT);

    if (npeMhStatus != IX_SUCCESS)
    {
        status = IX_ETH_ACC_FAIL;
    }
    else
    {
	/* wait for NPE loopbackEnable response */
        if (ixOsalMutexLock(&ixEthAccMacState[portId]. npeLoopbackMessageLock,
			    IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS)
	    != IX_SUCCESS)
        {
            status = IX_ETH_ACC_FAIL;
        }
    }

    return status;
}

IxEthAccStatus
ixEthAccPortTxEnablePriv(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot enable TX.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* read register */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    /* update register */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval | IX_ETH_ACC_TX_CNTRL1_TX_EN);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortRxEnablePriv(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot enable RX.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* read register */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    /* update register */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_RX_EN);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortLoopbackDisable(IxEthAccPortId portId)
{
    UINT32 regval;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot disable loopback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*disable MAC loopabck */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      (regval & ~IX_ETH_ACC_RX_CNTRL1_LOOP_EN));

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccNpeLoopbackDisablePriv(IxEthAccPortId portId)
{
    IX_STATUS npeMhStatus;
    IxNpeMhMessage message;
    IxEthAccStatus status = IX_ETH_ACC_SUCCESS;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot enable NPE loopback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* disable NPE loopback (lsb of the message contains the value 0) */
    message.data[0] = (IX_ETHNPE_SETLOOPBACK_MODE << IX_ETH_ACC_MAC_MSGID_SHL);
    message.data[1] = 0;

    npeMhStatus = ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_TO_NPE_ID(portId),
		message,
		IX_ETHNPE_SETLOOPBACK_MODE_ACK,
		ixEthAccNpeLoopbackMessageCallback,
		IX_NPEMH_SEND_RETRIES_DEFAULT);

    if (npeMhStatus != IX_SUCCESS)
    {
        status = IX_ETH_ACC_FAIL;
    }
    else
    {
	/* wait for NPE loopbackEnable response */
        if (ixOsalMutexLock(&ixEthAccMacState[portId].npeLoopbackMessageLock,
			    IX_ETH_ACC_PORT_DISABLE_DELAY_MSECS)
	    != IX_SUCCESS)
        {
            status = IX_ETH_ACC_FAIL;
        }
    }

    return status;
}

IxEthAccStatus
ixEthAccPortTxDisablePriv(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot disable TX.\n", (INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* read register */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    /* update register */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      (regval & ~IX_ETH_ACC_TX_CNTRL1_TX_EN));

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortRxDisablePriv(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot disable RX.\n", (INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* read register */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    /* update register */
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      (regval & ~IX_ETH_ACC_RX_CNTRL1_RX_EN));

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortPromiscuousModeClearPriv(IxEthAccPortId portId)
{
    UINT32 regval;

    /* Turn off promiscuous mode */
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot clear promiscuous mode.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*set bit 5 of Rx control 1 - enable address filtering*/
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_ADDR_FLTR_EN);

    ixEthAccMacState[portId].promiscuous = FALSE;

    ixEthAccMulticastAddressSet(portId);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortPromiscuousModeSetPriv(IxEthAccPortId portId)
{
    UINT32 regval;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot set promiscuous mode.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*
     * Set bit 5 of Rx control 1 - We enable address filtering even in
     * promiscuous mode because we want the MAC to set the appropriate
     * bits in m_flags which doesn't happen if we turn off filtering.
     */
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_ADDR_FLTR_EN);

    ixEthAccMacState[portId].promiscuous = TRUE;

    ixEthAccMulticastAddressSet(portId);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortUnicastMacAddressSetPriv (IxEthAccPortId portId,
				  IxEthAccMacAddr *macAddr)
{
    UINT32 i;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot set Unicast Mac Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }


    if (macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }

    if ( macAddr->macAddress[0] & IX_ETH_ACC_ETH_MAC_BCAST_MCAST_BIT )
    {
	/* This is a multicast/broadcast address cant set it ! */
	return IX_ETH_ACC_FAIL;
    }

    if ( macAddr->macAddress[0] == 0 &&
	 macAddr->macAddress[1] == 0 &&
	 macAddr->macAddress[2] == 0 &&
	 macAddr->macAddress[3] == 0 &&
	 macAddr->macAddress[4] == 0 &&
	 macAddr->macAddress[5] == 0  )
    {
	/* This is an invalid mac address cant set it ! */
	return IX_ETH_ACC_FAIL;
    }

#ifdef CONFIG_IXP425_COMPONENT_ETHDB
    /* update the MAC address in the ethernet database */
    if (ixEthDBPortAddressSet(portId, (IxEthDBMacAddr *) macAddr) != IX_ETH_DB_SUCCESS)
    {
        return IX_ETH_ACC_FAIL;
    }
#endif

    /*Set the Unicast MAC to the specified value*/
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_UNI_ADDR_1 + i*sizeof(UINT32),
		  macAddr->macAddress[i]);
    }
    ixEthAccMacState[portId].initDone = TRUE;

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortUnicastMacAddressGetPriv (IxEthAccPortId portId,
				  IxEthAccMacAddr *macAddr)
{
    /*Return the current value of the Unicast MAC from h/w
      for the specified port*/
    UINT32 i;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot get Unicast Mac Address.\n",(INT32)portId,0,0,0,0,0);
        /* Since Eth Npe is unavailable, return invalid MAC Address = 00:00:00:00:00:00 */
        for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
        {
	    macAddr->macAddress[i] = 0;
        }
        return IX_ETH_ACC_SUCCESS ;
    }

    if(!ixEthAccMacState[portId].initDone)
    {
	return (IX_ETH_ACC_MAC_UNINITIALIZED);
    }

    if (macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }


    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_UNI_ADDR_1 + i*sizeof(UINT32),
		 macAddr->macAddress[i]);
    }
    return IX_ETH_ACC_SUCCESS;
}

PRIVATE IxEthAccStatus
ixEthAccPortMulticastMacAddressGet (IxEthAccPortId portId,
				    IxEthAccMacAddr *macAddr)
{
    /*Return the current value of the Multicast MAC from h/w
      for the specified port*/
    UINT32 i;

    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {

	REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_ADDR_1 + i*sizeof(UINT32),
		 macAddr->macAddress[i]);
    }

    return IX_ETH_ACC_SUCCESS;
}

PRIVATE IxEthAccStatus
ixEthAccPortMulticastMacFilterGet (IxEthAccPortId portId,
				   IxEthAccMacAddr *macAddr)
{
    /*Return the current value of the Multicast MAC from h/w
      for the specified port*/
    UINT32 i;

    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {

	REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_ADDR_MASK_1 + i*sizeof(UINT32),
		 macAddr->macAddress[i]);
    }
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortMulticastAddressJoinPriv (IxEthAccPortId portId,
				  IxEthAccMacAddr *macAddr)
{
    UINT32 i;
    IxEthAccMacAddr broadcastAddr = {{0xff,0xff,0xff,0xff,0xff,0xff}};

    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot join Multicast Mac Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*Check that the mac address is valid*/
    if(macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }

    /* Check that this is a multicast address */
    if (!(macAddr->macAddress[0] & IX_ETH_ACC_ETH_MAC_BCAST_MCAST_BIT))
    {
	return IX_ETH_ACC_FAIL;
    }

    /* We don't add the Broadcast address */
    if(ixEthAccMacEqual(&broadcastAddr, macAddr))
    {
	return IX_ETH_ACC_FAIL;
    }

    for (i = 0;
	 i<ixEthAccMacState[portId].mcastAddrIndex;
	 i++)
    {
	/*Check if the current entry already match an existing matches*/
	if(ixEthAccMacEqual(&ixEthAccMacState[portId].mcastAddrsTable[i], macAddr))
	{
	    /* Address found in the list and already configured,
	     * return a success status
	     */
	    return IX_ETH_ACC_SUCCESS;
	}
    }

    /* check for availability at the end of the current table */
    if(ixEthAccMacState[portId].mcastAddrIndex >= IX_ETH_ACC_MAX_MULTICAST_ADDRESSES)
    {
	return IX_ETH_ACC_FAIL;
    }

    /*First add the address to the multicast table for the
      specified port*/
    i=ixEthAccMacState[portId].mcastAddrIndex;

    memcpy(&ixEthAccMacState[portId].mcastAddrsTable[i],
	   &macAddr->macAddress,
	   IX_IEEE803_MAC_ADDRESS_SIZE);

    /*Increment the index into the table, this must be done here
     as MulticastAddressSet below needs to know about the latest
     entry.
    */
    ixEthAccMacState[portId].mcastAddrIndex++;

    /*Then calculate the new value to be written to the address and
      address mask registers*/
    ixEthAccMulticastAddressSet(portId);

    return IX_ETH_ACC_SUCCESS;
}


IxEthAccStatus
ixEthAccPortMulticastAddressJoinAllPriv (IxEthAccPortId portId)
{
    IxEthAccMacAddr mcastMacAddr = {{0x1,0x0,0x0,0x0,0x0,0x0}};

    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot join all Multicast Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* remove all entries from the database and
    *  insert a multicast entry
    */
    memcpy(&ixEthAccMacState[portId].mcastAddrsTable[0],
	   &mcastMacAddr.macAddress,
	   IX_IEEE803_MAC_ADDRESS_SIZE);

    ixEthAccMacState[portId].mcastAddrIndex = 1;
    ixEthAccMacState[portId].joinAll = TRUE;

    ixEthAccMulticastAddressSet(portId);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortMulticastAddressLeavePriv (IxEthAccPortId portId,
				   IxEthAccMacAddr *macAddr)
{
    UINT32 i;
    IxEthAccMacAddr mcastMacAddr = {{0x1,0x0,0x0,0x0,0x0,0x0}};

    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot leave Multicast Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*Check that the mac address is valid*/
    if(macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }
    /* Remove this mac address from the mask for the specified port
     * we copy down all entries above the blanked entry, and
     * decrement the index
     */
    i=0;

    while(i<ixEthAccMacState[portId].mcastAddrIndex)
    {
	/*Check if the current entry matches*/
	if(ixEthAccMacEqual(&ixEthAccMacState[portId].mcastAddrsTable[i],
			    macAddr))
	{
	    if(ixEthAccMacEqual(macAddr, &mcastMacAddr))
	    {
		ixEthAccMacState[portId].joinAll = FALSE;
	    }
	    /*Decrement the index into the multicast address table
	      for the current port*/
	    ixEthAccMacState[portId].mcastAddrIndex--;

	    /*Copy down all entries above the current entry*/
	    while(i<ixEthAccMacState[portId].mcastAddrIndex)
	    {
		memcpy(&ixEthAccMacState[portId].mcastAddrsTable[i],
		       &ixEthAccMacState[portId].mcastAddrsTable[i+1],
		       IX_IEEE803_MAC_ADDRESS_SIZE);
                i++;
	    }
	    /*recalculate the mask and write it to the MAC*/
	    ixEthAccMulticastAddressSet(portId);

	    return IX_ETH_ACC_SUCCESS;
	}
	/* search the next entry */
	i++;
    }
    /* no matching entry found */
    return IX_ETH_ACC_NO_SUCH_ADDR;
}

IxEthAccStatus
ixEthAccPortMulticastAddressLeaveAllPriv (IxEthAccPortId portId)
{
    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot leave all Multicast Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    ixEthAccMacState[portId].mcastAddrIndex = 0;
    ixEthAccMacState[portId].joinAll = FALSE;

    ixEthAccMulticastAddressSet(portId);

    return IX_ETH_ACC_SUCCESS;
}


IxEthAccStatus
ixEthAccPortUnicastAddressShowPriv (IxEthAccPortId portId)
{
    IxEthAccMacAddr macAddr;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot show Unicast Address.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /*Get the MAC (UINICAST) address from hardware*/
    if(ixEthAccPortUnicastMacAddressGetPriv(portId, &macAddr) != IX_ETH_ACC_SUCCESS)
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: MAC address uninitialised port %u\n",
			       (INT32)portId,0,0,0,0,0);
	return IX_ETH_ACC_MAC_UNINITIALIZED;
    }

    /*print it out*/
    ixEthAccMacPrint(&macAddr);
    printf("\n");
    return IX_ETH_ACC_SUCCESS;
}



void
ixEthAccPortMulticastAddressShowPriv(IxEthAccPortId portId)
{
    IxEthAccMacAddr macAddr;
    UINT32 i;

    if(!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
        return;
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot show Multicast Address.\n",(INT32)portId,0,0,0,0,0);
        return ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return;
    }

    printf("Multicast MAC: ");
    /*Get the MAC (MULTICAST) address from hardware*/
    ixEthAccPortMulticastMacAddressGet(portId, &macAddr);
    /*print it out*/
    ixEthAccMacPrint(&macAddr);
    /*Get the MAC (MULTICAST) filter from hardware*/
    ixEthAccPortMulticastMacFilterGet(portId, &macAddr);
    /*print it out*/
    printf(" ( ");
    ixEthAccMacPrint(&macAddr);
    printf(" )\n");
    printf("Constituent Addresses:\n");
    for(i=0;i<ixEthAccMacState[portId].mcastAddrIndex;i++)
    {
	ixEthAccMacPrint(&ixEthAccMacState[portId].mcastAddrsTable[i]);
	printf("\n");
    }
    return;
}

/*Set the duplex mode*/
IxEthAccStatus
ixEthAccPortDuplexModeSetPriv (IxEthAccPortId portId,
			   IxEthAccDuplexMode mode)
{
    UINT32 txregval;
    UINT32 rxregval;

    /*This is bit 1 of the transmit control reg, set to 1 for half
      duplex, 0 for full duplex*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot set Duplex Mode.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     txregval);

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     rxregval);

    if (mode ==  IX_ETH_ACC_FULL_DUPLEX)
    {
	/*Clear half duplex bit in TX*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  txregval & ~IX_ETH_ACC_TX_CNTRL1_DUPLEX);

	/*We must set the pause enable in the receive logic when in
	  full duplex mode*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  rxregval | IX_ETH_ACC_RX_CNTRL1_PAUSE_EN);
	ixEthAccMacState[portId].fullDuplex = TRUE;

    }
    else if (mode ==  IX_ETH_ACC_HALF_DUPLEX)
    {
	/*Set half duplex bit in TX*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  txregval | IX_ETH_ACC_TX_CNTRL1_DUPLEX);

	/*We must clear pause enable in the receive logic when in
	  half duplex mode*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  rxregval & ~IX_ETH_ACC_RX_CNTRL1_PAUSE_EN);

	ixEthAccMacState[portId].fullDuplex = FALSE;
    }
    else
    {
	return IX_ETH_ACC_FAIL;
    }


    return IX_ETH_ACC_SUCCESS;

}



IxEthAccStatus
ixEthAccPortDuplexModeGetPriv (IxEthAccPortId portId,
			   IxEthAccDuplexMode *mode)
{
    /*Return the duplex mode for the specified port*/
    UINT32 regval;

    /*This is bit 1 of the transmit control reg, set to 1 for half
      duplex, 0 for full duplex*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot get Duplex Mode.\n",(INT32)portId,0,0,0,0,0);
        /* return hald duplex */
        *mode = IX_ETH_ACC_HALF_DUPLEX ;
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if (mode == NULL)
    {
	return (IX_ETH_ACC_FAIL);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    if( regval & IX_ETH_ACC_TX_CNTRL1_DUPLEX)
    {
	*mode = IX_ETH_ACC_HALF_DUPLEX;
    }
    else
    {
	*mode = IX_ETH_ACC_FULL_DUPLEX;
    }

    return IX_ETH_ACC_SUCCESS;
}



IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingEnablePriv (IxEthAccPortId portId)
{
    UINT32 regval;
    /*Enable FCS computation by the MAC and appending to the
      frame*/

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot enable Tx Frame Append Padding.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval |
	      IX_ETH_ACC_TX_CNTRL1_PAD_EN);

    ixEthAccMacState[portId].txPADAppend = TRUE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingDisablePriv (IxEthAccPortId portId)
{
    UINT32 regval;

    /*disable FCS computation and appending*/
    /*Set bit 4 of Tx control register one to zero*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot disble Tx Frame Append Padding.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval & ~IX_ETH_ACC_TX_CNTRL1_PAD_EN);

    ixEthAccMacState[portId].txPADAppend = FALSE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortTxFrameAppendFCSEnablePriv (IxEthAccPortId portId)
{
    UINT32 regval;

    /*Enable FCS computation by the MAC and appending to the
      frame*/

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot enable Tx Frame Append FCS.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval | IX_ETH_ACC_TX_CNTRL1_FCS_EN);

    ixEthAccMacState[portId].txFCSAppend = TRUE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortTxFrameAppendFCSDisablePriv (IxEthAccPortId portId)
{
    UINT32 regval;

    /*disable FCS computation and appending*/
    /*Set bit 4 of Tx control register one to zero*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot disable Tx Frame Append FCS.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval & ~IX_ETH_ACC_TX_CNTRL1_FCS_EN);

    ixEthAccMacState[portId].txFCSAppend = FALSE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortRxFrameAppendFCSEnablePriv (IxEthAccPortId portId)
{
    /*Set bit 2 of Rx control 1*/
    UINT32 regval;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot enable Rx Frame Append FCS.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_CRC_EN);

    ixEthAccMacState[portId].rxFCSAppend = TRUE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortRxFrameAppendFCSDisablePriv (IxEthAccPortId portId)
{
    UINT32 regval;

    /*Clear bit 2 of Rx control 1*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot disable Rx Frame Append FCS.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval & ~IX_ETH_ACC_RX_CNTRL1_CRC_EN);

    ixEthAccMacState[portId].rxFCSAppend = FALSE;
    return IX_ETH_ACC_SUCCESS;
}



PRIVATE void
ixEthAccMacNpeStatsMessageCallback (IxNpeMhNpeId npeId,
				    IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

#ifndef NDEBUG
    /* Prudent to at least check the port is within range */
    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
    {
	IX_ETH_ACC_FATAL_LOG(
     "IXETHACC:ixEthAccMacNpeStatsMessageCallback: Illegal port: %u\n",
     (UINT32)portId, 0, 0, 0, 0, 0);
	return;
    }
#endif

    /*Unblock Stats Get call*/
    ixOsalMutexUnlock(&ixEthAccMacState[portId].ackMIBStatsLock);

}

PRIVATE void
ixEthAccMibIIStatsEndianConvert (IxEthEthObjStats *retStats)
{
    /* endianness conversion */

    /* Rx stats */
    retStats->dot3StatsAlignmentErrors =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsAlignmentErrors);
    retStats->dot3StatsFCSErrors =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsFCSErrors);
    retStats->dot3StatsInternalMacReceiveErrors =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsInternalMacReceiveErrors);
    retStats->RxOverrunDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxOverrunDiscards);
    retStats->RxLearnedEntryDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxLearnedEntryDiscards);
    retStats->RxLargeFramesDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxLargeFramesDiscards);
    retStats->RxSTPBlockedDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxSTPBlockedDiscards);
    retStats->RxVLANTypeFilterDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxVLANTypeFilterDiscards);
    retStats->RxVLANIdFilterDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxVLANIdFilterDiscards);
    retStats->RxInvalidSourceDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxInvalidSourceDiscards);
    retStats->RxBlackListDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxBlackListDiscards);
    retStats->RxWhiteListDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxWhiteListDiscards);
    retStats->RxUnderflowEntryDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->RxUnderflowEntryDiscards);

    /* Tx stats */
    retStats->dot3StatsSingleCollisionFrames =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsSingleCollisionFrames);
    retStats->dot3StatsMultipleCollisionFrames =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsMultipleCollisionFrames);
    retStats->dot3StatsDeferredTransmissions =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsDeferredTransmissions);
    retStats->dot3StatsLateCollisions =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsLateCollisions);
    retStats->dot3StatsExcessiveCollsions =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsExcessiveCollsions);
    retStats->dot3StatsInternalMacTransmitErrors =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsInternalMacTransmitErrors);
    retStats->dot3StatsCarrierSenseErrors =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->dot3StatsCarrierSenseErrors);
    retStats->TxLargeFrameDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->TxLargeFrameDiscards);
    retStats->TxVLANIdFilterDiscards =
	IX_OSAL_SWAP_BE_SHARED_LONG(retStats->TxVLANIdFilterDiscards);
}

IxEthAccStatus
ixEthAccMibIIStatsGet (IxEthAccPortId portId,
		       IxEthEthObjStats *retStats )
{
    IxNpeMhMessage message;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
        printf("EthAcc: ixEthAccMibIIStatsGet (Mac) EthAcc service is not initialized\n");
	return (IX_ETH_ACC_FAIL);
    }

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (retStats == NULL)
    {
        printf("EthAcc: ixEthAccMibIIStatsGet (Mac) NULL argument\n");
	return (IX_ETH_ACC_FAIL);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        printf("EthAcc: ixEthAccMibIIStatsGet (Mac) NPE for port %d is not available\n", portId);

        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot get MIB II Stats.\n",(INT32)portId,0,0,0,0,0);

        /* Return all zero stats */
        IX_ETH_ACC_MEMSET(retStats, 0, sizeof(IxEthEthObjStats));

        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
        printf("EthAcc: ixEthAccMibIIStatsGet (Mac) port %d is not initialized\n", portId);
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    IX_OSAL_CACHE_INVALIDATE(retStats, sizeof(IxEthEthObjStats));

    message.data[0] = IX_ETHNPE_GETSTATS << IX_ETH_ACC_MAC_MSGID_SHL;
    message.data[1] = (UINT32) IX_OSAL_MMU_VIRT_TO_PHYS(retStats);

    /* Permit only one task to request MIB statistics Get operation
       at a time */
    ixOsalMutexLock(&ixEthAccMacState[portId].MIBStatsGetAccessLock, IX_OSAL_WAIT_FOREVER);

    if(ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_TO_NPE_ID(portId),
				      message,
				      IX_ETHNPE_GETSTATS,
				      ixEthAccMacNpeStatsMessageCallback,
				      IX_NPEMH_SEND_RETRIES_DEFAULT)
       != IX_SUCCESS)
    {
	ixOsalMutexUnlock(&ixEthAccMacState[portId].MIBStatsGetAccessLock);

        printf("EthAcc: (Mac) StatsGet failed to send NPE message\n");

	return IX_ETH_ACC_FAIL;
    }

    /* Wait for callback invocation indicating response to
       this request - we need this mutex in order to ensure
       that the return from this function is synchronous */
    ixOsalMutexLock(&ixEthAccMacState[portId].ackMIBStatsLock, IX_ETH_ACC_MIB_STATS_DELAY_MSECS);

    /* Permit other tasks to perform MIB statistics Get operation */
    ixOsalMutexUnlock(&ixEthAccMacState[portId].MIBStatsGetAccessLock);

    ixEthAccMibIIStatsEndianConvert (retStats);

    return IX_ETH_ACC_SUCCESS;
}


PRIVATE void
ixEthAccMacNpeStatsResetMessageCallback (IxNpeMhNpeId npeId,
					 IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

#ifndef NDEBUG
    /* Prudent to at least check the port is within range */
    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
    {
	IX_ETH_ACC_FATAL_LOG(
     "IXETHACC:ixEthAccMacNpeStatsResetMessageCallback: Illegal port: %u\n",
     (UINT32)portId, 0, 0, 0, 0, 0);
	return;
    }
#endif

    /*Unblock Stats Get & reset call*/
    ixOsalMutexUnlock(&ixEthAccMacState[portId].ackMIBStatsResetLock);

}



IxEthAccStatus
ixEthAccMibIIStatsGetClear (IxEthAccPortId portId,
			    IxEthEthObjStats *retStats)
{
    IxNpeMhMessage message;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
        printf("EthAcc: ixEthAccMibIIStatsGetClear (Mac) EthAcc service is not initialized\n");
	return (IX_ETH_ACC_FAIL);
    }

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (retStats == NULL)
    {
        printf("EthAcc: ixEthAccMibIIStatsGetClear (Mac) NULL argument\n");
	return (IX_ETH_ACC_FAIL);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        printf("EthAcc: ixEthAccMibIIStatsGetClear (Mac) NPE for port %d is not available\n", portId);

        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot get and clear MIB II Stats.\n", (INT32)portId, 0, 0, 0, 0, 0);

        /* Return all zero stats */
        IX_ETH_ACC_MEMSET(retStats, 0, sizeof(IxEthEthObjStats));

        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
        printf("EthAcc: ixEthAccMibIIStatsGetClear (Mac) port %d is not initialized\n", portId);
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    IX_OSAL_CACHE_INVALIDATE(retStats, sizeof(IxEthEthObjStats));

    message.data[0] = IX_ETHNPE_RESETSTATS << IX_ETH_ACC_MAC_MSGID_SHL;
    message.data[1] = (UINT32) IX_OSAL_MMU_VIRT_TO_PHYS(retStats);

    /* Permit only one task to request MIB statistics Get-Reset operation at a time */
    ixOsalMutexLock(&ixEthAccMacState[portId].MIBStatsGetResetAccessLock, IX_OSAL_WAIT_FOREVER);

    if(ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_TO_NPE_ID(portId),
				      message,
				      IX_ETHNPE_RESETSTATS,
				      ixEthAccMacNpeStatsResetMessageCallback,
				      IX_NPEMH_SEND_RETRIES_DEFAULT)
       != IX_SUCCESS)
    {
	ixOsalMutexUnlock(&ixEthAccMacState[portId].MIBStatsGetResetAccessLock);

        printf("EthAcc: (Mac) ixEthAccMibIIStatsGetClear failed to send NPE message\n");

	return IX_ETH_ACC_FAIL;
    }

    /* Wait for callback invocation indicating response to this request */
    ixOsalMutexLock(&ixEthAccMacState[portId].ackMIBStatsResetLock, IX_ETH_ACC_MIB_STATS_DELAY_MSECS);

    /* permit other tasks to get and reset MIB stats*/
    ixOsalMutexUnlock(&ixEthAccMacState[portId].MIBStatsGetResetAccessLock);

    ixEthAccMibIIStatsEndianConvert(retStats);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccMibIIStatsClear (IxEthAccPortId portId)
{
    static IxEthEthObjStats retStats;
    IxEthAccStatus status;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot clear MIB II Stats.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    /* there is no reset operation without a corresponding Get */
    status = ixEthAccMibIIStatsGetClear(portId, &retStats);

    return status;
}

/* Initialize the ethernet MAC settings */
IxEthAccStatus
ixEthAccMacInit(IxEthAccPortId portId)
{
    IX_OSAL_MBUF_POOL* portDisablePool;
    UINT8 *data;

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot initialize Mac.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if(ixEthAccMacState[portId].macInitialised == FALSE)
    {
	ixEthAccMacState[portId].fullDuplex  = TRUE;
	ixEthAccMacState[portId].rxFCSAppend = TRUE;
	ixEthAccMacState[portId].txFCSAppend = TRUE;
	ixEthAccMacState[portId].txPADAppend = TRUE;
	ixEthAccMacState[portId].enabled     = FALSE;
	ixEthAccMacState[portId].promiscuous = TRUE;
	ixEthAccMacState[portId].joinAll     = FALSE;
	ixEthAccMacState[portId].initDone    = FALSE;
	ixEthAccMacState[portId].macInitialised = TRUE;

        /* initialize MIB stats mutexes */
        ixOsalMutexInit(&ixEthAccMacState[portId].ackMIBStatsLock);
        ixOsalMutexLock(&ixEthAccMacState[portId].ackMIBStatsLock, IX_OSAL_WAIT_FOREVER);

        ixOsalMutexInit(&ixEthAccMacState[portId].ackMIBStatsResetLock);
        ixOsalMutexLock(&ixEthAccMacState[portId].ackMIBStatsResetLock, IX_OSAL_WAIT_FOREVER);

        ixOsalMutexInit(&ixEthAccMacState[portId].MIBStatsGetAccessLock);

        ixOsalMutexInit(&ixEthAccMacState[portId].MIBStatsGetResetAccessLock);

        ixOsalMutexInit(&ixEthAccMacState[portId].npeLoopbackMessageLock);

	ixEthAccMacState[portId].portDisableRxMbufPtr = NULL;
        ixEthAccMacState[portId].portDisableTxMbufPtr = NULL;

	portDisablePool = IX_OSAL_MBUF_POOL_INIT(2,
			  IX_ETHACC_RX_MBUF_MIN_SIZE,
			  "portDisable Pool");

        IX_OSAL_ENSURE(portDisablePool != NULL, "Failed to initialize PortDisable pool");

	ixEthAccMacState[portId].portDisableRxMbufPtr = IX_OSAL_MBUF_POOL_GET(portDisablePool);
        ixEthAccMacState[portId].portDisableTxMbufPtr = IX_OSAL_MBUF_POOL_GET(portDisablePool);

	IX_OSAL_ENSURE(ixEthAccMacState[portId].portDisableRxMbufPtr != NULL,
		  "Pool allocation failed");
        IX_OSAL_ENSURE(ixEthAccMacState[portId].portDisableTxMbufPtr != NULL,
		  "Pool allocation failed");
	/* fill the payload of the Rx mbuf used in portDisable */
        IX_OSAL_MBUF_MLEN(ixEthAccMacState[portId].portDisableRxMbufPtr) = IX_ETHACC_RX_MBUF_MIN_SIZE;

        memset(IX_OSAL_MBUF_MDATA(ixEthAccMacState[portId].portDisableRxMbufPtr),
	       0xAA,
	       IX_ETHACC_RX_MBUF_MIN_SIZE);

	/* fill the payload of the Tx mbuf used in portDisable (64 bytes) */
        IX_OSAL_MBUF_MLEN(ixEthAccMacState[portId].portDisableTxMbufPtr) = 64;
        IX_OSAL_MBUF_PKT_LEN(ixEthAccMacState[portId].portDisableTxMbufPtr) = 64;

        data = (UINT8 *) IX_OSAL_MBUF_MDATA(ixEthAccMacState[portId].portDisableTxMbufPtr);
        memset(data, 0xBB, 64);
        data[0] = 0x00; /* unicast destination MAC address */
        data[6] = 0x00; /* unicast source MAC address */
        data[12] = 0x08; /* typelength : IP frame */
        data[13] = 0x00; /* typelength : IP frame */

	IX_OSAL_CACHE_FLUSH(data, 64);
    }

    IX_OSAL_ASSERT (ixEthAccMacBase[portId] != 0);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_RESET);

    ixOsalSleep(IX_ETH_ACC_MAC_RESET_DELAY);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_MDC_EN);

    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);

    ixEthAccMacStateUpdate(portId);

    return IX_ETH_ACC_SUCCESS;
}

/* PRIVATE Functions*/

PRIVATE void
ixEthAccMacStateUpdate(IxEthAccPortId portId)
{
    UINT32 regval;

    if ( ixEthAccMacState[portId].enabled == FALSE )
    {
	/*  Just disable both the transmitter and reciver in the MAC.  */
        REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_RX_CNTRL1,
		 regval);
 	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  regval & ~IX_ETH_ACC_RX_CNTRL1_RX_EN);

        REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_TX_CNTRL1,
		 regval);
 	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  regval & ~IX_ETH_ACC_TX_CNTRL1_TX_EN);
    }

    if(ixEthAccMacState[portId].fullDuplex)
    {
	ixEthAccPortDuplexModeSetPriv (portId, IX_ETH_ACC_FULL_DUPLEX);
    }
    else
    {
	ixEthAccPortDuplexModeSetPriv (portId, IX_ETH_ACC_HALF_DUPLEX);
    }

    if(ixEthAccMacState[portId].rxFCSAppend)
    {
	ixEthAccPortRxFrameAppendFCSEnablePriv (portId);
    }
    else
    {
	ixEthAccPortRxFrameAppendFCSDisablePriv (portId);
    }

    if(ixEthAccMacState[portId].txFCSAppend)
    {
	ixEthAccPortTxFrameAppendFCSEnablePriv (portId);
    }
    else
    {
	ixEthAccPortTxFrameAppendFCSDisablePriv (portId);
    }

    if(ixEthAccMacState[portId].txPADAppend)
    {
	ixEthAccPortTxFrameAppendPaddingEnablePriv (portId);
    }
    else
    {
	ixEthAccPortTxFrameAppendPaddingDisablePriv (portId);
    }

    if(ixEthAccMacState[portId].promiscuous)
    {
	ixEthAccPortPromiscuousModeSetPriv(portId);
    }
    else
    {
	ixEthAccPortPromiscuousModeClearPriv(portId);
    }

    if ( ixEthAccMacState[portId].enabled == TRUE )
    {
        /*   Enable both the transmitter and reciver in the MAC.  */
        REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_RX_CNTRL1,
		 regval);
        REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  regval | IX_ETH_ACC_RX_CNTRL1_RX_EN);

        REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_TX_CNTRL1,
		 regval);
 	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  regval | IX_ETH_ACC_TX_CNTRL1_TX_EN);
    }
}


PRIVATE BOOL
ixEthAccMacEqual(IxEthAccMacAddr *macAddr1,
		 IxEthAccMacAddr *macAddr2)
{
    UINT32 i;
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE; i++)
    {
	if(macAddr1->macAddress[i] != macAddr2->macAddress[i])
	{
	    return FALSE;
	}
    }
    return TRUE;
}

PRIVATE void
ixEthAccMacPrint(IxEthAccMacAddr *m)
{
    printf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
	   m->macAddress[0], m->macAddress[1],
	   m->macAddress[2], m->macAddress[3],
	   m->macAddress[4], m->macAddress[5]);
}

/* Set the multicast address and address mask registers
 *
 * A bit in the address mask register must be set if
 * all multicast addresses always have that bit set, or if
 * all multicast addresses always have that bit cleared.
 *
 * A bit in the address register must be set if all multicast
 * addresses have that bit set, otherwise, it should be cleared
 */

PRIVATE void
ixEthAccMulticastAddressSet(IxEthAccPortId portId)
{
    UINT32 i;
    UINT32 j;
    IxEthAccMacAddr addressMask;
    IxEthAccMacAddr address;
    IxEthAccMacAddr alwaysClearBits;
    IxEthAccMacAddr alwaysSetBits;

    /* calculate alwaysClearBits and alwaysSetBits:
     * alwaysClearBits is calculated by ORing all
     * multicast addresses, those bits that are always
     * clear are clear in the result
     *
     * alwaysSetBits is calculated by ANDing all
     * multicast addresses, those bits that are always set
     * are set in the result
     */

    if (ixEthAccMacState[portId].promiscuous == TRUE)
    {
	/* Promiscuous Mode is set, and filtering
	 * allow all packets, and enable the mcast and
	 * bcast detection.
	 */
	memset(&addressMask.macAddress,
	       0,
	       IX_IEEE803_MAC_ADDRESS_SIZE);
	memset(&address.macAddress,
	       0,
	       IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    else
    {
	if(ixEthAccMacState[portId].joinAll == TRUE)
	{
	    /* Join all is set. The mask and address are
	     * the multicast settings.
	     */
	    IxEthAccMacAddr macAddr = {{0x1,0x0,0x0,0x0,0x0,0x0}};

	    memcpy(addressMask.macAddress,
		   macAddr.macAddress,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	    memcpy(address.macAddress,
		   macAddr.macAddress,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	}
	else if(ixEthAccMacState[portId].mcastAddrIndex == 0)
	{
	    /* No entry in the filtering database,
	     * Promiscuous Mode is cleared, Broadcast filtering
	     * is configured.
	     */
	    memset(addressMask.macAddress,
		   IX_ETH_ACC_MAC_ALL_BITS_SET,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	    memset(address.macAddress,
		   IX_ETH_ACC_MAC_ALL_BITS_SET,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	}
	else
	{
	    /* build a mask and an address which mix all entreis
	     * from the list of multicast addresses
	     */
	    memset(alwaysClearBits.macAddress,
		   0,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	    memset(alwaysSetBits.macAddress,
		   IX_ETH_ACC_MAC_ALL_BITS_SET,
		   IX_IEEE803_MAC_ADDRESS_SIZE);

	    for(i=0;i<ixEthAccMacState[portId].mcastAddrIndex;i++)
	    {
		for(j=0;j<IX_IEEE803_MAC_ADDRESS_SIZE;j++)
		{
		    alwaysClearBits.macAddress[j] |=
			ixEthAccMacState[portId].mcastAddrsTable[i].macAddress[j];
		    alwaysSetBits.macAddress[j] &=
			ixEthAccMacState[portId].mcastAddrsTable[i].macAddress[j];
		}
	    }

	    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
	    {
		addressMask.macAddress[i] = alwaysSetBits.macAddress[i]
		    | ~alwaysClearBits.macAddress[i];
		address.macAddress[i] = alwaysSetBits.macAddress[i];
	    }
	}
    }

    /*write the new addr filtering to h/w*/
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_ADDR_MASK_1+i*sizeof(UINT32),
		  addressMask.macAddress[i]);
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_ADDR_1+i*sizeof(UINT32),
		  address.macAddress[i]);
    }
}
