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
 * SPDX-License-Identifier:	BSD-3-Clause
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
