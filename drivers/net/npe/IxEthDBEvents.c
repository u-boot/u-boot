/**
 * @file IxEthDBEvents.c
 *
 * @brief Implementation of the event processor component
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

#include <IxNpeMh.h>
#include <IxFeatureCtrl.h>

#include "IxEthDB_p.h"

/* forward prototype declarations */
IX_ETH_DB_PUBLIC void ixEthDBEventProcessorLoop(void *); 
IX_ETH_DB_PUBLIC void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PRIVATE void ixEthDBProcessEvent(PortEvent *local_event, IxEthDBPortMap triggerPorts);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBTriggerPortUpdate(UINT32 eventType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, BOOL staticEntry);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStartLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStopLearningFunction(void);

/* data */
IX_ETH_DB_PRIVATE IxOsalSemaphore eventQueueSemaphore;
IX_ETH_DB_PRIVATE PortEventQueue eventQueue;
IX_ETH_DB_PRIVATE IxOsalMutex eventQueueLock;
IX_ETH_DB_PRIVATE IxOsalMutex portUpdateLock;

IX_ETH_DB_PRIVATE BOOL ixEthDBLearningShutdown      = false;
IX_ETH_DB_PRIVATE BOOL ixEthDBEventProcessorRunning = false;

/* imported data */
extern HashTable dbHashtable;

/**
 * @brief initializes the event processor
 *
 * Initializes the event processor queue and processing thread.
 * Called from ixEthDBInit() DB-subcomponent master init function.
 *
 * @warning do not call directly
 *
 * @retval IX_ETH_DB_SUCCESS initialization was successful
 * @retval IX_ETH_DB_FAIL initialization failed (OSAL or mutex init failure)
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBEventProcessorInit(void)
{
    if (ixOsalMutexInit(&portUpdateLock) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    if (ixOsalMutexInit(&eventQueueLock) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    if (IX_FEATURE_CTRL_SWCONFIG_ENABLED ==
        ixFeatureCtrlSwConfigurationCheck (IX_FEATURECTRL_ETH_LEARNING))
    {

        /* start processor loop thread */
        if (ixEthDBStartLearningFunction() != IX_ETH_DB_SUCCESS)
        {
            return IX_ETH_DB_FAIL;
        }
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief initializes the event queue and the event processor
 *
 * This function is called by the component initialization
 * function, ixEthDBInit().
 *
 * @warning do not call directly
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or IX_ETH_DB_FAIL otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBStartLearningFunction(void)
{
    IxOsalThread eventProcessorThread;
    IxOsalThreadAttr threadAttr;

    threadAttr.name      = "EthDB event thread";
    threadAttr.stackSize = 32 * 1024; /* 32kbytes */
    threadAttr.priority  = 128;

    /* reset event queue */
    ixOsalMutexLock(&eventQueueLock, IX_OSAL_WAIT_FOREVER);

    RESET_QUEUE(&eventQueue);

    ixOsalMutexUnlock(&eventQueueLock);

    /* init event queue semaphore */
    if (ixOsalSemaphoreInit(&eventQueueSemaphore, 0) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    ixEthDBLearningShutdown = false;

    /* create processor loop thread */
    if (ixOsalThreadCreate(&eventProcessorThread, &threadAttr, ixEthDBEventProcessorLoop, NULL) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    /* start event processor */
    ixOsalThreadStart(&eventProcessorThread);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief stops the event processor
 *
 * Stops the event processor and frees the event queue semaphore
 * Called by the component de-initialization function, ixEthDBUnload()
 *
 * @warning do not call directly
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed 
 * successfully or IX_ETH_DB_FAIL otherwise;
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBStopLearningFunction(void)
{
    ixEthDBLearningShutdown = true;

    /* wake up event processing loop to actually process the shutdown event */
    ixOsalSemaphorePost(&eventQueueSemaphore);

    if (ixOsalSemaphoreDestroy(&eventQueueSemaphore) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief default NPE event processing callback
 *
 * @param npeID ID of the NPE that generated the event
 * @param msg NPE message (encapsulated event)
 *
 * Creates an event object on the Ethernet event processor queue
 * and signals the new event by incrementing the event queue semaphore.
 * Events are processed by @ref ixEthDBEventProcessorLoop() which runs
 * at user level.
 *
 * @see ixEthDBEventProcessorLoop()
 *
 * @warning do not call directly
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg)
{
    PortEvent *local_event;

    IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) new event received by processor callback from port %d, id 0x%X\n", IX_ETH_DB_NPE_TO_PORT_ID(npeID), NPE_MSG_ID(msg), 0, 0, 0, 0);

    if (CAN_ENQUEUE(&eventQueue))
    {
        TEST_FIXTURE_LOCK_EVENT_QUEUE;

        local_event = QUEUE_HEAD(&eventQueue);

        /* create event structure on queue */
        local_event->eventType = NPE_MSG_ID(msg);
        local_event->portID    = IX_ETH_DB_NPE_TO_PORT_ID(npeID);
        
        /* update queue */
        PUSH_UPDATE_QUEUE(&eventQueue);

        TEST_FIXTURE_UNLOCK_EVENT_QUEUE;

        IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) Waking up main processor loop...\n", 0, 0, 0, 0, 0, 0);

        /* increment event queue semaphore */
        ixOsalSemaphorePost(&eventQueueSemaphore);
    }
    else
    {
        IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) Warning: could not enqueue event (overflow)\n", 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief Ethernet event processor loop
 *
 * Extracts at most EVENT_PROCESSING_LIMIT batches of events and
 * sends them for processing to @ref ixEthDBProcessEvent().
 * Triggers port updates which normally follow learning events.
 *
 * @warning do not call directly, executes in separate thread
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBEventProcessorLoop(void *unused1)
{
    IxEthDBPortMap triggerPorts;
    IxEthDBPortId portIndex;

    ixEthDBEventProcessorRunning = true;

    IX_ETH_DB_EVENTS_TRACE("DB: (Events) Event processor loop was started\n");

    while (!ixEthDBLearningShutdown)
    {
        BOOL keepProcessing    = true;
        UINT32 processedEvents = 0;

        IX_ETH_DB_EVENTS_VERBOSE_TRACE("DB: (Events) Waiting for new learning event...\n");

        ixOsalSemaphoreWait(&eventQueueSemaphore, IX_OSAL_WAIT_FOREVER);

        IX_ETH_DB_EVENTS_VERBOSE_TRACE("DB: (Events) Received new event\n");

        if (!ixEthDBLearningShutdown)
        {
            /* port update handling */
            SET_EMPTY_DEPENDENCY_MAP(triggerPorts);

            while (keepProcessing)
            {
                PortEvent local_event;
                UINT32 intLockKey;

                /* lock queue */
                ixOsalMutexLock(&eventQueueLock, IX_OSAL_WAIT_FOREVER);

                /* lock NPE interrupts */
                intLockKey = ixOsalIrqLock();

                /* extract event */
                local_event = *(QUEUE_TAIL(&eventQueue));

                SHIFT_UPDATE_QUEUE(&eventQueue);

                ixOsalIrqUnlock(intLockKey);

                ixOsalMutexUnlock(&eventQueueLock);

                IX_ETH_DB_EVENTS_TRACE("DB: (Events) Processing event with ID 0x%X\n", local_event.eventType);

                ixEthDBProcessEvent(&local_event, triggerPorts);

                processedEvents++;

                if (processedEvents > EVENT_PROCESSING_LIMIT /* maximum burst reached? */
                    || ixOsalSemaphoreTryWait(&eventQueueSemaphore) != IX_SUCCESS) /* or empty queue? */
                {
                    keepProcessing = false;
                }
            }

            ixEthDBUpdatePortLearningTrees(triggerPorts);
        }
    }

    /* turn off automatic updates */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        ixEthDBPortInfo[portIndex].updateMethod.updateEnabled = false;
    }

    ixEthDBEventProcessorRunning = false;
}

/**
 * @brief event processor routine
 *
 * @param event event to be processed
 * @param triggerPorts port map accumulating ports to be updated
 *
 * Processes learning events by synchronizing the database with
 * newly learnt data. Called only by @ref ixEthDBEventProcessorLoop().
 *
 * @warning do not call directly
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBProcessEvent(PortEvent *local_event, IxEthDBPortMap triggerPorts)
{
    MacDescriptor recordTemplate;

    switch (local_event->eventType)
    {
        case IX_ETH_DB_ADD_FILTERING_RECORD:
            /* add record */
            memset(&recordTemplate, 0, sizeof (recordTemplate));
            memcpy(recordTemplate.macAddress, local_event->macAddr.macAddress, sizeof (IxEthDBMacAddr));
            
            recordTemplate.type   = IX_ETH_DB_FILTERING_RECORD;
            recordTemplate.portID = local_event->portID;
            recordTemplate.recordData.filteringData.staticEntry = local_event->staticEntry;
            
            ixEthDBAdd(&recordTemplate, triggerPorts);

            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Added record on port %d\n", local_event->portID);

            break;

        case IX_ETH_DB_REMOVE_FILTERING_RECORD:
            /* remove record */
            memset(&recordTemplate, 0, sizeof (recordTemplate));
            memcpy(recordTemplate.macAddress, local_event->macAddr.macAddress, sizeof (IxEthDBMacAddr));
            
            recordTemplate.type = IX_ETH_DB_FILTERING_RECORD | IX_ETH_DB_FILTERING_VLAN_RECORD;
            
            ixEthDBRemove(&recordTemplate, triggerPorts);
            
            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Removed record on port %d\n", local_event->portID);

            break;

        default:
            /* can't handle/not interested in this event type */
            ERROR_LOG("DB: (Events) Event processor received an unknown event type (0x%X)\n", local_event->eventType);

            return;
    }
}

/**
 * @brief asynchronously adds a filtering record
 * by posting an ADD_FILTERING_RECORD event to the event queue
 *
 * @param macAddr MAC address of the new record
 * @param portID port ID of the new record
 * @param staticEntry true if record is static, false if dynamic
 *
 * @return IX_ETH_DB_SUCCESS if the event creation was
 * successfull or IX_ETH_DB_BUSY if the event queue is full
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBTriggerAddPortUpdate(IxEthDBMacAddr *macAddr, IxEthDBPortId portID, BOOL staticEntry)
{
    MacDescriptor reference;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    /* fill search fields */
    memcpy(reference.macAddress, macAddr, sizeof (IxEthDBMacAddr));
    reference.portID = portID;
    
    /* set acceptable record types */
    reference.type = IX_ETH_DB_ALL_FILTERING_RECORDS;

    if (ixEthDBPeekHashEntry(&dbHashtable, IX_ETH_DB_MAC_PORT_KEY, &reference) == IX_ETH_DB_SUCCESS)
    {
        /* already have an identical record */
        return IX_ETH_DB_SUCCESS;
    }
    else
    {
        return ixEthDBTriggerPortUpdate(IX_ETH_DB_ADD_FILTERING_RECORD, macAddr, portID, staticEntry);
    }
}

/**
 * @brief asynchronously removes a filtering record
 * by posting a REMOVE_FILTERING_RECORD event to the event queue
 *
 * @param macAddr MAC address of the record to remove
 * @param portID port ID of the record to remove
 *
 * @return IX_ETH_DB_SUCCESS if the event creation was
 * successfull or IX_ETH_DB_BUSY if the event queue is full
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBTriggerRemovePortUpdate(IxEthDBMacAddr *macAddr, IxEthDBPortId portID)
{
    if (ixEthDBPeek(macAddr, IX_ETH_DB_ALL_FILTERING_RECORDS) != IX_ETH_DB_NO_SUCH_ADDR)
    {
        return ixEthDBTriggerPortUpdate(IX_ETH_DB_REMOVE_FILTERING_RECORD, macAddr, portID, false);
    }
    else
    {
        return IX_ETH_DB_NO_SUCH_ADDR;
    }
}

/**
 * @brief adds an ADD or REMOVE event to the main event queue
 *
 * @param eventType event type - IX_ETH_DB_ADD_FILTERING_RECORD 
 * to add and IX_ETH_DB_REMOVE_FILTERING_RECORD to remove a
 * record.
 *
 * @return IX_ETH_DB_SUCCESS if the event was successfully
 * sent or IX_ETH_DB_BUSY if the event queue is full
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBTriggerPortUpdate(UINT32 eventType, IxEthDBMacAddr *macAddr, IxEthDBPortId portID, BOOL staticEntry)
{
    UINT32 intLockKey;

    /* lock interrupts to protect queue */
    intLockKey = ixOsalIrqLock();

    if (CAN_ENQUEUE(&eventQueue))
    {
        PortEvent *queueEvent = QUEUE_HEAD(&eventQueue);

        /* update fields on the queue */
        memcpy(queueEvent->macAddr.macAddress, macAddr->macAddress, sizeof (IxEthDBMacAddr));
        
        queueEvent->eventType     = eventType;
        queueEvent->portID        = portID;
        queueEvent->staticEntry   = staticEntry;

        PUSH_UPDATE_QUEUE(&eventQueue);

        /* imcrement event queue semaphore */
        ixOsalSemaphorePost(&eventQueueSemaphore);
        
        /* unlock interrupts */
        ixOsalIrqUnlock(intLockKey);

        return IX_ETH_DB_SUCCESS;
    }
    else /* event queue full */
    {
        /* unlock interrupts */
        ixOsalIrqUnlock(intLockKey);

        return IX_ETH_DB_BUSY;
    }
}

/**
 * @brief Locks learning tree updates and port disable
 *
 *
 * This function locks portUpdateLock single mutex. It is primarily used
 * to avoid executing 'port disable' during ELT maintenance.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBUpdateLock(void)
{
    ixOsalMutexLock(&portUpdateLock, IX_OSAL_WAIT_FOREVER);
}

/**
 * @brief Unlocks learning tree updates and port disable
 *
 *
 * This function unlocks a portUpdateLock mutex. It is primarily used
 * to avoid executing 'port disable' during ELT maintenance.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBUpdateUnlock(void)
{
    ixOsalMutexUnlock(&portUpdateLock);
}

