/**
 * @file IxEthAccCommon.c
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief This file contains the implementation common support routines for the component
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

/*
 * Component header files
 */

#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxNpeMh.h"
#include "IxEthDBPortDefs.h"
#include "IxFeatureCtrl.h"
#include "IxEthAcc_p.h"
#include "IxEthAccQueueAssign_p.h"

#include "IxEthAccDataPlane_p.h"
#include "IxEthAccMii_p.h"

/**
 * @addtogroup IxEthAccPri
 *@{
 */

extern IxEthAccInfo   ixEthAccDataInfo;

/**
 *
 * @brief Maximum number of RX queues set to be the maximum number
 * of traffic calsses.
 *
 */
#define IX_ETHACC_MAX_RX_QUEUES \
      (IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY \
      - IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY \
      + 1)

/**
 *
 * @brief Maximum number of 128 entry RX queues
 *
 */
#define IX_ETHACC_MAX_LARGE_RX_QUEUES 4

/**
 *
 * @brief Data structure template for Default RX Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrRxDefaultTemplate =
  {
    IX_ETH_ACC_RX_FRAME_ETH_Q,	     /**< Queue ID */
    "Eth Rx Q",
    ixEthRxFrameQMCallback,          /**< Functional callback */
    (IxQMgrCallbackId) 0,	     /**< Callback tag	      */
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    true,			     /**< Enable Q notification at startup */
    IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE,/**< Q Condition to drive callback   */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL1,	     /**< Q High water mark - needed by NPE */
  };

/**
 *
 * @brief Data structure template for Small RX Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrRxSmallTemplate =
  {
    IX_ETH_ACC_RX_FRAME_ETH_Q,	     /**< Queue ID */
    "Eth Rx Q",
    ixEthRxFrameQMCallback,          /**< Functional callback */
    (IxQMgrCallbackId) 0,	     /**< Callback tag	      */
    IX_QMGR_Q_SIZE64,		     /**< Allocate Smaller Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    true,			     /**< Enable Q notification at startup */
    IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE,/**< Q Condition to drive callback   */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL1,	     /**< Q High water mark - needed by NPE */
  };


/**
 *
 * @brief Data structure used to register & initialize the Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrStaticInfo[]=
{
  {
    IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q,
    "Eth Rx Fr Q 1",
    ixEthRxFreeQMCallback,
    (IxQMgrCallbackId) IX_ETH_PORT_1,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    false,			     /**< Disable Q notification at startup */
    IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE, /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /***< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,	     /**< Q High water mark */
  },

  {
    IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q,
    "Eth Rx Fr Q 2",
    ixEthRxFreeQMCallback,
    (IxQMgrCallbackId) IX_ETH_PORT_2,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    false,			     /**< Disable Q notification at startup */
    IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE,  /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,	     /**< Q High water mark */
  },
#ifdef __ixp46X
  {
    IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q,
    "Eth Rx Fr Q 3",
    ixEthRxFreeQMCallback,
    (IxQMgrCallbackId) IX_ETH_PORT_3,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    false,			     /**< Disable Q notification at startup */
    IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q_SOURCE,  /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,	     /**< Q High water mark */
  },
#endif
  {
     IX_ETH_ACC_TX_FRAME_ENET0_Q,
    "Eth Tx Q 1",
     ixEthTxFrameQMCallback,
     (IxQMgrCallbackId) IX_ETH_PORT_1,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    false,			     /**< Disable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE,	 /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,	     /**< Q High water mark */
  },

  {
     IX_ETH_ACC_TX_FRAME_ENET1_Q,
    "Eth Tx Q 2",
     ixEthTxFrameQMCallback,
     (IxQMgrCallbackId) IX_ETH_PORT_2,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    false,			     /**< Disable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE,	     /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,	     /**< Q High water mark */
  },
#ifdef __ixp46X
  {
     IX_ETH_ACC_TX_FRAME_ENET2_Q,
    "Eth Tx Q 3",
     ixEthTxFrameQMCallback,
     (IxQMgrCallbackId) IX_ETH_PORT_3,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    false,			     /** Disable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_ENET2_Q_SOURCE,	     /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL64,	      /** Q High water mark - needed used  */
  },
#endif
  {
     IX_ETH_ACC_TX_FRAME_DONE_ETH_Q,
    "Eth Tx Done Q",
     ixEthTxFrameDoneQMCallback,
     (IxQMgrCallbackId) 0,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    true,			     /**< Enable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_DONE_ETH_Q_SOURCE, /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL2,	     /**< Q High water mark - needed by NPE */
  },

  {  /* Null Termination entry
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  }

};

/**
 *
 * @brief Data structure used to register & initialize the Queues
 *
 * The structure will be filled at run time depending on the NPE
 * image already loaded and the QoS configured in ethDB.
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrRxQueuesInfo[IX_ETHACC_MAX_RX_QUEUES+1]=
{
  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
      (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* PlaceHolder for rx queues
      * depending on the QoS configured
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  },

  {  /* Null Termination entry
      */
     (IxQMgrQId)0,
     (char *) NULL,
     (IxQMgrCallback) NULL,
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  }

};

/* forward declarations */
IX_ETH_ACC_PRIVATE IxEthAccStatus
ixEthAccQMgrQueueSetup(IxEthAccQregInfo *qInfoDes);

/**
 * @fn ixEthAccQMgrQueueSetup(void)
 *
 * @brief Setup one queue and its event, and register the callback required
 * by this component to the QMgr
 *
 * @internal
 */
IX_ETH_ACC_PRIVATE IxEthAccStatus
ixEthAccQMgrQueueSetup(IxEthAccQregInfo *qInfoDes)
{
    /*
     * Configure each Q.
     */
    if ( ixQMgrQConfig( qInfoDes->qName,
			qInfoDes->qId,
			qInfoDes->qSize,
			qInfoDes->qWords) != IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    if ( ixQMgrWatermarkSet( qInfoDes->qId,
			     qInfoDes->AlmostEmptyThreshold,
			     qInfoDes->AlmostFullThreshold
			     ) != IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    /*
     * Set dispatcher priority.
     */
    if ( ixQMgrDispatcherPrioritySet( qInfoDes->qId,
				      IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY)
	 != IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    /*
     * Register callbacks for each Q.
     */
    if ( ixQMgrNotificationCallbackSet(qInfoDes->qId,
				       qInfoDes->qCallback,
				       qInfoDes->callbackTag)
	 != IX_SUCCESS )
    {
	return IX_ETH_ACC_FAIL;
    }

    /*
     * Set notification condition for Q
     */
    if (qInfoDes->qNotificationEnableAtStartup == true)
    {
	if (   ixQMgrNotificationEnable(qInfoDes->qId,
					qInfoDes->qConditionSource)
	       != IX_SUCCESS )
	{
	    return IX_ETH_ACC_FAIL;
	}
    }

    return(IX_ETH_ACC_SUCCESS);
}

/**
 * @fn ixEthAccQMgrQueuesConfig(void)
 *
 * @brief Setup all the queues and register all callbacks required
 * by this component to the QMgr
 *
 * The RxFree queues, tx queues, rx queues are configured statically
 *
 * Rx queues configuration is driven by QoS setup.
 * Many Rx queues may be required when QoS is enabled (this depends
 * on IxEthDB setup and the images being downloaded). The configuration
 * of the rxQueues is done in many steps as follows:
 *
 * @li select all Rx queues as configured by ethDB for all ports
 * @li sort the queues by traffic class
 * @li build the priority dependency for all queues
 * @li fill the configuration for all rx queues
 * @li configure all statically configured queues
 * @li configure all dynamically configured queues
 *
 * @param none
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccQMgrQueuesConfig(void)
{
    struct
    {
	int npeCount;
	UINT32 npeId;
	IxQMgrQId qId;
	IxEthDBProperty trafficClass;
    } rxQueues[IX_ETHACC_MAX_RX_QUEUES];

    UINT32 rxQueue = 0;
    UINT32 rxQueueCount = 0;
    IxQMgrQId ixQId =IX_QMGR_MAX_NUM_QUEUES;
    IxEthDBStatus ixEthDBStatus = IX_ETH_DB_SUCCESS;
    IxEthDBPortId ixEthDbPortId = 0;
    IxEthAccPortId ixEthAccPortId = 0;
    UINT32 ixNpeId = 0;
    UINT32 ixHighestNpeId = 0;
    UINT32 sortIterations = 0;
    IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;
    IxEthAccQregInfo *qInfoDes = NULL;
    IxEthDBProperty ixEthDBTrafficClass = IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY;
    IxEthDBPropertyType ixEthDBPropertyType = IX_ETH_DB_INTEGER_PROPERTY;
    UINT32 ixEthDBParameter = 0;
    BOOL completelySorted = false;

    /* Fill the corspondance between ports and queues
     * This defines the mapping from port to queue Ids.
     */

    ixEthAccPortData[IX_ETH_PORT_1].ixEthAccRxData.rxFreeQueue
	= IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q;
    ixEthAccPortData[IX_ETH_PORT_2].ixEthAccRxData.rxFreeQueue
	= IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q;
#ifdef __ixp46X
    ixEthAccPortData[IX_ETH_PORT_3].ixEthAccRxData.rxFreeQueue
	= IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q;
#endif
    ixEthAccPortData[IX_ETH_PORT_1].ixEthAccTxData.txQueue
	= IX_ETH_ACC_TX_FRAME_ENET0_Q;
    ixEthAccPortData[IX_ETH_PORT_2].ixEthAccTxData.txQueue
	= IX_ETH_ACC_TX_FRAME_ENET1_Q;
#ifdef __ixp46X
    ixEthAccPortData[IX_ETH_PORT_3].ixEthAccTxData.txQueue
	= IX_ETH_ACC_TX_FRAME_ENET2_Q;
#endif
    /* Fill the corspondance between ports and NPEs
     * This defines the mapping from port to npeIds.
     */

    ixEthAccPortData[IX_ETH_PORT_1].npeId = IX_NPEMH_NPEID_NPEB;
    ixEthAccPortData[IX_ETH_PORT_2].npeId = IX_NPEMH_NPEID_NPEC;
#ifdef __ixp46X
    ixEthAccPortData[IX_ETH_PORT_3].npeId = IX_NPEMH_NPEID_NPEA;
#endif
    /* set the default rx scheduling discipline */
    ixEthAccDataInfo.schDiscipline = FIFO_NO_PRIORITY;

    /*
     * Queue Selection step:
     *
     * The following code selects all the queues and build
     * a temporary array which contains for each queue
     * - the queue Id,
     * - the highest traffic class (in case of many
     * priorities configured for the same queue on different
     * ports)
     * - the number of different Npes which are
     * configured to write to this queue.
     *
     * The output of this loop is a temporary array of RX queues
     * in any order.
     *
     */
#ifdef CONFIG_IXP425_COMPONENT_ETHDB
    for (ixEthAccPortId = 0;
	 (ixEthAccPortId < IX_ETH_ACC_NUMBER_OF_PORTS)
	     && (ret == IX_ETH_ACC_SUCCESS);
	 ixEthAccPortId++)
    {
	/* map between ethDb and ethAcc port Ids */
	ixEthDbPortId = (IxEthDBPortId)ixEthAccPortId;

	/* map between npeId and ethAcc port Ids */
	ixNpeId = IX_ETH_ACC_PORT_TO_NPE_ID(ixEthAccPortId);

	/* Iterate thru the different priorities */
	for (ixEthDBTrafficClass = IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY;
	     ixEthDBTrafficClass <= IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY;
	     ixEthDBTrafficClass++)
	{
	    ixEthDBStatus = ixEthDBFeaturePropertyGet(
	      ixEthDbPortId,
	      IX_ETH_DB_VLAN_QOS,
	      ixEthDBTrafficClass,
	      &ixEthDBPropertyType,
	      (void *)&ixEthDBParameter);

	    if (ixEthDBStatus == IX_ETH_DB_SUCCESS)
	    {
		/* This port and QoS class are mapped to
		 * a RX queue.
		 */
		if (ixEthDBPropertyType == IX_ETH_DB_INTEGER_PROPERTY)
		{
		    /* remember the highest npe Id supporting ethernet */
		    if (ixNpeId > ixHighestNpeId)
		    {
			ixHighestNpeId = ixNpeId;
		    }

		    /* search the queue in the list of queues
		     * already used by an other port or QoS
		     */
		    for (rxQueue = 0;
			 rxQueue < rxQueueCount;
			 rxQueue++)
		    {
			if (rxQueues[rxQueue].qId == (IxQMgrQId)ixEthDBParameter)
			{
			    /* found an existing setup, update the number of ports
			     * for this queue if the port maps to
			     * a different NPE.
			     */
			    if (rxQueues[rxQueue].npeId != ixNpeId)
			    {
				rxQueues[rxQueue].npeCount++;
				rxQueues[rxQueue].npeId = ixNpeId;
			    }
			    /* get the highest traffic class for this queue */
			    if (rxQueues[rxQueue].trafficClass > ixEthDBTrafficClass)
			    {
				rxQueues[rxQueue].trafficClass = ixEthDBTrafficClass;
			    }
			    break;
			}
		    }
		    if (rxQueue == rxQueueCount)
		    {
			/* new queue not found in the current list,
			 * add a new entry.
			 */
			IX_OSAL_ASSERT(rxQueueCount < IX_ETHACC_MAX_RX_QUEUES);
			rxQueues[rxQueueCount].qId = ixEthDBParameter;
			rxQueues[rxQueueCount].npeCount = 1;
			rxQueues[rxQueueCount].npeId = ixNpeId;
			rxQueues[rxQueueCount].trafficClass = ixEthDBTrafficClass;
			rxQueueCount++;
		    }
		}
		else
		{
		    /* unexpected property type (not Integer) */
		    ret = IX_ETH_ACC_FAIL;

                    IX_ETH_ACC_WARNING_LOG("ixEthAccQMgrQueuesConfig: unexpected property type returned by EthDB\n", 0, 0, 0, 0, 0, 0);

		    /* no point to continue to iterate */
		    break;
		}
	    }
	    else
	    {
		/* No Rx queue configured for this port
		 * and this traffic class. Do nothing.
		 */
	    }
	}

        /* notify EthDB that queue initialization is complete and traffic class allocation is frozen */
        ixEthDBFeaturePropertySet(ixEthDbPortId,
            IX_ETH_DB_VLAN_QOS,
            IX_ETH_DB_QOS_QUEUE_CONFIGURATION_COMPLETE,
            NULL /* ignored */);
    }

#else

    ixNpeId = IX_ETH_ACC_PORT_TO_NPE_ID(ixEthAccPortId);
    rxQueues[0].qId = 4;
    rxQueues[0].npeCount = 1;
    rxQueues[0].npeId = ixNpeId;
    rxQueues[0].trafficClass = IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY;
    rxQueueCount++;

#endif

    /* check there is at least 1 rx queue : there is no point
     * to continue if there is no rx queue configured
     */
    if ((rxQueueCount == 0) || (ret == IX_ETH_ACC_FAIL))
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccQMgrQueuesConfig: no queues configured, bailing out\n", 0, 0, 0, 0, 0, 0);
	return (IX_ETH_ACC_FAIL);
    }

    /* Queue sort step:
     *
     * Re-order the array of queues by decreasing traffic class
     * using a bubble sort. (trafficClass 0 is the lowest
     * priority traffic, trafficClass 7 is the highest priority traffic)
     *
     * Primary sort order is traffic class
     * Secondary sort order is npeId
     *
     * Note that a bubble sort algorithm is not very efficient when
     * the number of queues grows . However, this is not a very bad choice
     * considering the very small number of entries to sort. Also, bubble
     * sort is extremely fast when the list is already sorted.
     *
     * The output of this loop is a sorted array of queues.
     *
     */
    sortIterations = 0;
    do
    {
	sortIterations++;
	completelySorted = true;
	for (rxQueue = 0;
	     rxQueue < rxQueueCount - sortIterations;
	     rxQueue++)
	{
	    /* compare adjacent elements */
	    if ((rxQueues[rxQueue].trafficClass <
		rxQueues[rxQueue+1].trafficClass)
		|| ((rxQueues[rxQueue].trafficClass ==
		     rxQueues[rxQueue+1].trafficClass)
		    &&(rxQueues[rxQueue].npeId <
		       rxQueues[rxQueue+1].npeId)))
	    {
		/* swap adjacent elements */
		int npeCount = rxQueues[rxQueue].npeCount;
		UINT32 npeId = rxQueues[rxQueue].npeId;
		IxQMgrQId qId = rxQueues[rxQueue].qId;
		IxEthDBProperty trafficClass = rxQueues[rxQueue].trafficClass;
		rxQueues[rxQueue].npeCount = rxQueues[rxQueue+1].npeCount;
		rxQueues[rxQueue].npeId = rxQueues[rxQueue+1].npeId;
		rxQueues[rxQueue].qId = rxQueues[rxQueue+1].qId;
		rxQueues[rxQueue].trafficClass = rxQueues[rxQueue+1].trafficClass;
		rxQueues[rxQueue+1].npeCount = npeCount;
		rxQueues[rxQueue+1].npeId = npeId;
		rxQueues[rxQueue+1].qId = qId;
		rxQueues[rxQueue+1].trafficClass = trafficClass;
		completelySorted = false;
	    }
	}
    }
    while (!completelySorted);

    /* Queue traffic class list:
     *
     * Fill an array of rx queues linked by ascending traffic classes.
     *
     * If the queues are configured as follows
     *   qId 6 -> traffic class 0 (lowest)
     *   qId 7 -> traffic class 0
     *   qId 8 -> traffic class 6
     *   qId 12 -> traffic class 7 (highest)
     *
     * Then the output of this loop will be
     *
     * higherPriorityQueue[6] = 8
     * higherPriorityQueue[7] = 8
     * higherPriorityQueue[8] = 12
     * higherPriorityQueue[12] = Invalid queueId
     * higherPriorityQueue[...] = Invalid queueId
     *
     * Note that this queue ordering does not handle all possibilities
     * that could result from different rules associated with different
     * ports, and inconsistencies in the rules. In all cases, the
     * output of this  algorithm is a simple linked list of queues,
     * without closed circuit.

     * This list is implemented as an array with invalid values initialized
     * with an "invalid" queue id which is the maximum number of queues.
     *
     */

    /*
     * Initialise the rx queue list.
     */
    for (rxQueue = 0; rxQueue < IX_QMGR_MAX_NUM_QUEUES; rxQueue++)
    {
	ixEthAccDataInfo.higherPriorityQueue[rxQueue] = IX_QMGR_MAX_NUM_QUEUES;
    }

    /* build the linked list for this NPE.
     */
    for (ixNpeId = 0;
	 ixNpeId <= ixHighestNpeId;
	 ixNpeId++)
    {
	/* iterate thru the sorted list of queues
	 */
	ixQId = IX_QMGR_MAX_NUM_QUEUES;
	for (rxQueue = 0;
	     rxQueue < rxQueueCount;
	     rxQueue++)
	{
	    if (rxQueues[rxQueue].npeId == ixNpeId)
	    {
		ixEthAccDataInfo.higherPriorityQueue[rxQueues[rxQueue].qId] = ixQId;
		/* iterate thru queues with the same traffic class
		 * than the current queue. (queues are ordered by descending
		 * traffic classes and npeIds).
		 */
		while ((rxQueue < rxQueueCount - 1)
		       && (rxQueues[rxQueue].trafficClass
			   == rxQueues[rxQueue+1].trafficClass)
		       && (ixNpeId == rxQueues[rxQueue].npeId))
		{
		    rxQueue++;
		    ixEthAccDataInfo.higherPriorityQueue[rxQueues[rxQueue].qId] = ixQId;
		}
		ixQId = rxQueues[rxQueue].qId;
	    }
	}
    }

    /* point on the first dynamic queue description */
    qInfoDes = ixEthAccQmgrRxQueuesInfo;

    /* update the list of queues with the rx queues */
    for (rxQueue = 0;
	 (rxQueue < rxQueueCount) && (ret == IX_ETH_ACC_SUCCESS);
	 rxQueue++)
    {
	/* Don't utilize more than IX_ETHACC_MAX_LARGE_RX_QUEUES queues
	 * with the full 128 entries.  For the lower priority queues, use
	 * a smaller number of entries.  This ensures queue resources
	 * remain available for other components.
	 */
	if( (rxQueueCount > IX_ETHACC_MAX_LARGE_RX_QUEUES) &&
	    (rxQueue < rxQueueCount - IX_ETHACC_MAX_LARGE_RX_QUEUES) )
	{
	    /* add the small RX Queue setup template to the list of queues */
	    memcpy(qInfoDes, &ixEthAccQmgrRxSmallTemplate, sizeof(*qInfoDes));
	} else {
	    /* add the default RX Queue setup template to the list of queues */
	    memcpy(qInfoDes, &ixEthAccQmgrRxDefaultTemplate, sizeof(*qInfoDes));
	}

	/* setup the RxQueue ID */
	qInfoDes->qId = rxQueues[rxQueue].qId;

	/* setup the RxQueue watermark level
	 *
	 * Each queue can be filled by many NPEs. To avoid the
	 * NPEs to write to a full queue, need to set the
	 * high watermark level for nearly full condition.
	 * (the high watermark level are a power of 2
	 * starting from the top of the queue)
	 *
	 * Number of     watermark
         *   ports        level
         *    1             0
	 *    2             1
	 *    3             2
	 *    4             4
	 *    5             4
	 *    6             8
	 *    n          approx. 2**ceil(log2(n))
	 */
	if (rxQueues[rxQueue].npeCount == 1)
	{
	    qInfoDes->AlmostFullThreshold = IX_QMGR_Q_WM_LEVEL0;
	}
	else if (rxQueues[rxQueue].npeCount == 2)
	{
	    qInfoDes->AlmostFullThreshold = IX_QMGR_Q_WM_LEVEL1;
	}
	else if (rxQueues[rxQueue].npeCount == 3)
	{
	    qInfoDes->AlmostFullThreshold = IX_QMGR_Q_WM_LEVEL2;
	}
	else
	{
	    /* reach the maximum number for CSR 2.0 */
            IX_ETH_ACC_WARNING_LOG("ixEthAccQMgrQueuesConfig: maximum number of NPEs per queue reached, bailing out\n", 0, 0, 0, 0, 0, 0);
	    ret = IX_ETH_ACC_FAIL;
	    break;
	}

	/* move to next queue entry */
	++qInfoDes;
    }

    /* configure the static list (RxFree, Tx and TxDone queues) */
    for (qInfoDes = ixEthAccQmgrStaticInfo;
	 (qInfoDes->qCallback != (IxQMgrCallback) NULL )
	     && (ret == IX_ETH_ACC_SUCCESS);
	 ++qInfoDes)
    {
	ret = ixEthAccQMgrQueueSetup(qInfoDes);
    }

    /* configure the dynamic list (Rx queues) */
    for (qInfoDes = ixEthAccQmgrRxQueuesInfo;
	 (qInfoDes->qCallback != (IxQMgrCallback) NULL )
	     && (ret == IX_ETH_ACC_SUCCESS);
	 ++qInfoDes)
    {
	ret = ixEthAccQMgrQueueSetup(qInfoDes);
    }

    return(ret);
}

/**
 * @fn ixEthAccQMgrRxQEntryGet(UINT32 *rxQueueEntries)
 *
 * @brief Add and return the total number of entries in all Rx queues
 *
 * @param UINT32 rxQueueEntries[in] number of entries in all queues
 *
 * @return void
 *
 * @note Rx queues configuration is driven by Qos Setup. There is a
 * variable number of rx queues which are set at initialisation.
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
void ixEthAccQMgrRxQEntryGet(UINT32 *numRxQueueEntries)
{
    UINT32 rxQueueLevel;
    IxEthAccQregInfo *qInfoDes;;

    *numRxQueueEntries = 0;

    /* iterate thru rx queues */
    for (qInfoDes = ixEthAccQmgrRxQueuesInfo;
	 qInfoDes->qCallback != (IxQMgrCallback)NULL;
	 ++qInfoDes)
    {
	/* retrieve the rx queue level */
	rxQueueLevel = 0;
	ixQMgrQNumEntriesGet(qInfoDes->qId, &rxQueueLevel);
	(*numRxQueueEntries) += rxQueueLevel;
    }
}

/**
 * @fn ixEthAccQMgrRxCallbacksRegister(IxQMgrCallback ixQMgrCallback)
 *
 * @brief Change the callback registered to all rx queues.
 *
 * @param IxQMgrCallback ixQMgrCallback[in] QMgr callback to register
 *
 * @return IxEthAccStatus
 *
 * @note The user may decide to use different Rx mechanisms
 * (e.g. receive many frames at the same time , or receive
 *  one frame at a time, depending on the overall application
 *  performances). A different QMgr callback is registered. This
 *  way, there is no excessive pointer checks in the datapath.
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccQMgrRxCallbacksRegister(IxQMgrCallback ixQMgrCallback)
{
    IxEthAccQregInfo *qInfoDes;
    IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;

    /* parameter check */
    if (NULL == ixQMgrCallback)
    {
	ret = IX_ETH_ACC_FAIL;
    }

    /* iterate thru rx queues */
    for (qInfoDes = ixEthAccQmgrRxQueuesInfo;
	 (qInfoDes->qCallback != (IxQMgrCallback) NULL )
	     && (ret == IX_ETH_ACC_SUCCESS);
	 ++qInfoDes)
    {
	/* register the rx callback for all queues */
	if (ixQMgrNotificationCallbackSet(qInfoDes->qId,
					     ixQMgrCallback,
					     qInfoDes->callbackTag
					     ) != IX_SUCCESS)
	{
	    ret = IX_ETH_ACC_FAIL;
	}
    }
    return(ret);
}

/**
 * @fn ixEthAccSingleEthNpeCheck(IxEthAccPortId portId)
 *
 * @brief Check the npe exists for this port
 *
 * @param IxEthAccPortId portId[in] port
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccSingleEthNpeCheck(IxEthAccPortId portId)
{

    /* If not IXP42X A0 stepping, proceed to check for existence of coprocessors */
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
        (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
        || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
      {
            if ((IX_ETH_PORT_1 == portId) &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                 IX_FEATURE_CTRL_COMPONENT_ENABLED))
            {
                return IX_ETH_ACC_SUCCESS;
            }

            if ((IX_ETH_PORT_2 == portId) &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                 IX_FEATURE_CTRL_COMPONENT_ENABLED))
            {
                return IX_ETH_ACC_SUCCESS;
            }

            if ((IX_ETH_PORT_3 == portId) &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) ==
                 IX_FEATURE_CTRL_COMPONENT_ENABLED))
            {
                return IX_ETH_ACC_SUCCESS;
            }

            return IX_ETH_ACC_FAIL;
      }

    return IX_ETH_ACC_SUCCESS;
}

/**
 * @fn ixEthAccStatsShow(void)
 *
 * @brief Displays all EthAcc stats
 *
 * @return void
 *
 */
void ixEthAccStatsShow(IxEthAccPortId portId)
{
    ixEthAccMdioShow();

    printf("\nPort %u\nUnicast MAC : ", portId);
    ixEthAccPortUnicastAddressShow(portId);
    ixEthAccPortMulticastAddressShow(portId);
    printf("\n");

    ixEthAccDataPlaneShow();
}



