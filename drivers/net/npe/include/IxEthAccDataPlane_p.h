/**
 * @file IxEthAccDataPlane_p.h
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief  Internal Header file for IXP425 Ethernet Access component.
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



#ifndef IxEthAccDataPlane_p_H
#define IxEthAccDataPlane_p_H

#include <IxOsal.h>
#include <IxQMgr.h>

/**
 * @addtogroup IxEthAccPri
 *@{
 */

/* typedefs global to this file*/

typedef struct
{
    IX_OSAL_MBUF *pHead;
    IX_OSAL_MBUF *pTail;
}IxEthAccDataPlaneQList;


/**
 * @struct  IxEthAccDataPlaneStats
 * @brief   Statistics data structure associated with the data plane
 *
 */
typedef struct
{
    UINT32 addToSwQ;
    UINT32 removeFromSwQ;
    UINT32 unchainedTxMBufs;
    UINT32 chainedTxMBufs;
    UINT32 unchainedTxDoneMBufs;
    UINT32 chainedTxDoneMBufs;
    UINT32 unchainedRxMBufs;
    UINT32 chainedRxMBufs;
    UINT32 unchainedRxFreeMBufs;
    UINT32 chainedRxFreeMBufs;
    UINT32 rxCallbackCounter;
    UINT32 rxCallbackBurstRead;
    UINT32 txDoneCallbackCounter;
    UINT32 unexpectedError;
} IxEthAccDataPlaneStats;

/**
 * @fn ixEthAccMbufFromSwQ
 * @brief  used during disable steps to convert mbufs from 
 *  swq format, ready to be pushed into hw queues for NPE, 
 *  back into XScale format 
 */
IX_OSAL_MBUF *ixEthAccMbufFromSwQ(IX_OSAL_MBUF *mbuf);

/**
 * @fn ixEthAccDataPlaneShow
 * @brief  Show function (for data plane statistics
 */
void ixEthAccDataPlaneShow(void);

/*
 * lock dataplane when atomic operation is required
 */
#define IX_ETH_ACC_DATA_PLANE_LOCK(arg) arg = ixOsalIrqLock();
#define IX_ETH_ACC_DATA_PLANE_UNLOCK(arg) ixOsalIrqUnlock(arg);

/*
 * Use MBUF fields
 */
#define IX_ETHACC_NE_SHARED(mBufPtr) \
 ((IxEthAccNe *)&((mBufPtr)->ix_ne))

#if 1

#define IX_ETHACC_NE_NEXT(mBufPtr) (mBufPtr)->ix_ne.reserved[0]

/* tm - wrong!! len and pkt_len are in the second word - #define IX_ETHACC_NE_LEN(mBufPtr) (mBufPtr)->ix_ne.reserved[3] */
#define IX_ETHACC_NE_LEN(mBufPtr) (mBufPtr)->ix_ne.reserved[1]

#define IX_ETHACC_NE_DATA(mBufPtr)(mBufPtr)->ix_ne.reserved[2]

#else

#define IX_ETHACC_NE_NEXT(mBufPtr) \
  IX_ETHACC_NE_SHARED(mBufPtr)->ixReserved_next

#define IX_ETHACC_NE_LEN(mBufPtr) \
  IX_ETHACC_NE_SHARED(mBufPtr)->ixReserved_lengths

#define IX_ETHACC_NE_DATA(mBufPtr) \
  IX_ETHACC_NE_SHARED(mBufPtr)->ixReserved_data
#endif

/*
 * Use MBUF  next pointer field to chain data.
 */
#define IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER(mbuf) (mbuf)->ix_ctrl.ix_chain



#define IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(mbuf_list) ((mbuf_list.pHead) == NULL)
    

#define IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(mbuf_list,mbuf_to_add) 		\
  do {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.addToSwQ);                           \
    if ( (mbuf_list.pHead) != NULL ) 						\
    {										\
      (IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add))) = (mbuf_list.pHead);\
      (mbuf_list.pHead) = (mbuf_to_add);					\
    } 										\
    else {									\
      (mbuf_list.pTail) = (mbuf_list.pHead) = (mbuf_to_add);			\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  } while(0)


#define IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(mbuf_list,mbuf_to_add)   	\
  do {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.addToSwQ);                           \
    if ( (mbuf_list.pHead) == NULL ) 						\
    {										\
      (mbuf_list.pHead) = mbuf_to_add;						\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    else {									\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_list.pTail)) = (mbuf_to_add);	\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    (mbuf_list.pTail) = mbuf_to_add;						\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  } while (0)


#define IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(mbuf_list,mbuf_to_rem)   	\
  do {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    if ( (mbuf_list.pHead) != NULL ) 						\
    {										\
      IX_ETH_ACC_STATS_INC(ixEthAccDataStats.removeFromSwQ);                    \
      (mbuf_to_rem) = (mbuf_list.pHead) ;					\
      (mbuf_list.pHead) = (IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_rem)));\
    } 										\
    else {									\
      (mbuf_to_rem) = NULL;							\
    } 										\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  } while (0)


/**
 * @brief message handler QManager entries for NPE id => port ID conversion (NPE_B => 0, NPE_C => 1)
 */
#define IX_ETH_ACC_PORT_TO_NPE_ID(port) \
   ixEthAccPortData[(port)].npeId

#define IX_ETH_ACC_NPE_TO_PORT_ID(npe) ((npe == 0 ? 2 : (npe == 1 ? 0 : ( npe == 2 ? 1 : -1 ))))

#define IX_ETH_ACC_PORT_TO_TX_Q_ID(port)  \
   ixEthAccPortData[(port)].ixEthAccTxData.txQueue

#define IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(port) \
   ixEthAccPortData[(port)].ixEthAccRxData.rxFreeQueue

#define IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(port)    (port == IX_ETH_PORT_1 ? IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE : (port == IX_ETH_PORT_2 ? IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE : IX_ETH_ACC_TX_FRAME_ENET2_Q_SOURCE))

#define IX_ETH_ACC_PORT_TO_RX_FREE_Q_SOURCE(port) (port == IX_ETH_PORT_1 ? IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE : (port == IX_ETH_PORT_2 ? IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE : IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q_SOURCE ))

/* Flush the mbufs chain and all data pointed to by the mbuf */

#ifndef NDEBUG
#define IX_ETH_ACC_STATS_INC(x) (x++)
#else
#define IX_ETH_ACC_STATS_INC(x)
#endif

#define IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT 128

void ixEthRxFrameQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId);
void ixEthRxMultiBufferQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId);
void ixEthTxFrameDoneQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId);

#endif /* IxEthAccDataPlane_p_H */


/**
 *@}
 */

