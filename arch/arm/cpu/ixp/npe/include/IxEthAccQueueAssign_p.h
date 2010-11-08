/**
 * @file IxEthAccQueueAssign_p.h
 *
 * @author Intel Corporation
 * @date 06-Mar-2002
 *
 * @brief   Mapping from QMgr Q's to internal assignment
 *
 * Design Notes:
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
 * @addtogroup IxEthAccPri
 *@{
 */

/*
 * Os/System dependancies.
 */
#include "IxOsal.h"

/*
 * Intermodule dependancies
 */
#include "IxQMgr.h"
#include "IxQueueAssignments.h"

/* Check range of Q's assigned to this component. */
#if IX_ETH_ACC_RX_FRAME_ETH_Q >= (IX_QMGR_MIN_QUEUPP_QID ) |    	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_TX_FRAME_ENET0_Q >=   (IX_QMGR_MIN_QUEUPP_QID) |  		\
 IX_ETH_ACC_TX_FRAME_ENET1_Q >=   (IX_QMGR_MIN_QUEUPP_QID) | 		\
 IX_ETH_ACC_TX_FRAME_DONE_ETH_Q  >=  (IX_QMGR_MIN_QUEUPP_QID)  
#error "Not all Ethernet Access Queues are betweem 1-31, requires full functionalty Q's unless otherwise validated "
#endif

/**
*
* @typedef  IxEthAccQregInfo
*
* @brief 
*
*/
typedef struct 
{
   IxQMgrQId qId;
   char *qName;
   IxQMgrCallback qCallback;
   IxQMgrCallbackId callbackTag;
   IxQMgrQSizeInWords qSize;
   IxQMgrQEntrySizeInWords qWords; 
   BOOL           qNotificationEnableAtStartup;
   IxQMgrSourceId qConditionSource; 
   IxQMgrWMLevel  AlmostEmptyThreshold;
   IxQMgrWMLevel  AlmostFullThreshold;

} IxEthAccQregInfo;

/*
 * Prototypes for all QM callbacks.
 */

/* 
 * Rx Callbacks 
 */
IX_ETH_ACC_PUBLIC
void  ixEthRxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthRxMultiBufferQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthRxFreeQMCallback(IxQMgrQId, IxQMgrCallbackId);

/* 
 * Tx Callback.
 */
IX_ETH_ACC_PUBLIC
void  ixEthTxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthTxFrameDoneQMCallback(IxQMgrQId, IxQMgrCallbackId );


#define IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY (IX_QMGR_Q_PRIORITY_0) /* Highest priority */

/*
 * Queue watermarks
 */
#define IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )
#define IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )   
#define IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   ) 
#define IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_ENET2_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_DONE_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )
