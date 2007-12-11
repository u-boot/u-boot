
/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2000 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* This program is free software; you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, located in the file LICENSE.                 */
/*                                                                            */
/* Queue functions.                                                           */
/*    void          QQ_InitQueue(PQQ_CONTAINER pQueue)                        */
/*    char          QQ_Full(PQQ_CONTAINER pQueue)                             */
/*    char          QQ_Empty(PQQ_CONTAINER pQueue)                            */
/*    unsigned int QQ_GetSize(PQQ_CONTAINER pQueue)                          */
/*    unsigned int QQ_GetEntryCnt(PQQ_CONTAINER pQueue)                      */
/*    char          QQ_PushHead(PQQ_CONTAINER pQueue, PQQ_ENTRY pEntry)       */
/*    char          QQ_PushTail(PQQ_CONTAINER pQueue, PQQ_ENTRY pEntry)       */
/*    PQQ_ENTRY     QQ_PopHead(PQQ_CONTAINER pQueue)                          */
/*    PQQ_ENTRY     QQ_PopTail(PQQ_CONTAINER pQueue)                          */
/*    PQQ_ENTRY     QQ_GetHead(PQQ_CONTAINER pQueue, unsigned int Idx)       */
/*    PQQ_ENTRY     QQ_GetTail(PQQ_CONTAINER pQueue, unsigned int Idx)       */
/*                                                                            */
/*                                                                            */
/* History:                                                                   */
/*    02/25/00 Hav Khauv        Initial version.                              */
/******************************************************************************/

#ifndef BCM_QUEUE_H
#define BCM_QUEUE_H
#ifndef EMBEDDED
#define EMBEDDED 1
#endif

/******************************************************************************/
/* Queue definitions. */
/******************************************************************************/

/* Entry for queueing. */
typedef void *PQQ_ENTRY;

/* Linux Atomic Ops support */
typedef struct { int counter; } atomic_t;


/*
 * This combination of `inline' and `extern' has almost the effect of a
 * macro.  The way to use it is to put a function definition in a header
 * file with these keywords, and put another copy of the definition
 * (lacking `inline' and `extern') in a library file.  The definition in
 * the header file will cause most calls to the function to be inlined.
 * If any uses of the function remain, they will refer to the single copy
 * in the library.
 */
extern __inline void
atomic_set(atomic_t* entry, int val)
{
    entry->counter = val;
}
extern __inline int
atomic_read(atomic_t* entry)
{
    return entry->counter;
}
extern __inline void
atomic_inc(atomic_t* entry)
{
    if(entry)
	entry->counter++;
}

extern __inline void
atomic_dec(atomic_t* entry)
{
    if(entry)
	entry->counter--;
}

extern __inline void
atomic_sub(int a, atomic_t* entry)
{
    if(entry)
	entry->counter -= a;
}
extern __inline void
atomic_add(int a, atomic_t* entry)
{
    if(entry)
	entry->counter += a;
}


/* Queue header -- base type. */
typedef struct {
    unsigned int Head;
    unsigned int Tail;
    unsigned int Size;
    atomic_t EntryCnt;
    PQQ_ENTRY Array[1];
} QQ_CONTAINER, *PQQ_CONTAINER;


/* Declare queue type macro. */
#define DECLARE_QUEUE_TYPE(_QUEUE_TYPE, _QUEUE_SIZE)            \
								\
    typedef struct {                                            \
	QQ_CONTAINER Container;                                 \
	PQQ_ENTRY EntryBuffer[_QUEUE_SIZE];                     \
    } _QUEUE_TYPE, *P##_QUEUE_TYPE


/******************************************************************************/
/* Compilation switches. */
/******************************************************************************/

#if DBG
#undef QQ_NO_OVERFLOW_CHECK
#undef QQ_NO_UNDERFLOW_CHECK
#endif /* DBG */

#ifdef QQ_USE_MACROS
/* notdone */
#else

#ifdef QQ_NO_INLINE
#define __inline
#endif /* QQ_NO_INLINE */

/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline void
QQ_InitQueue(
PQQ_CONTAINER pQueue,
unsigned int QueueSize) {
    pQueue->Head = 0;
    pQueue->Tail = 0;
    pQueue->Size = QueueSize+1;
    atomic_set(&pQueue->EntryCnt, 0);
} /* QQ_InitQueue */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline char
QQ_Full(
PQQ_CONTAINER pQueue) {
    unsigned int NewHead;

    NewHead = (pQueue->Head + 1) % pQueue->Size;

    return(NewHead == pQueue->Tail);
} /* QQ_Full */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline char
QQ_Empty(
PQQ_CONTAINER pQueue) {
    return(pQueue->Head == pQueue->Tail);
} /* QQ_Empty */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline unsigned int
QQ_GetSize(
PQQ_CONTAINER pQueue) {
    return pQueue->Size;
} /* QQ_GetSize */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline unsigned int
QQ_GetEntryCnt(
PQQ_CONTAINER pQueue) {
    return atomic_read(&pQueue->EntryCnt);
} /* QQ_GetEntryCnt */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
extern __inline char
QQ_PushHead(
PQQ_CONTAINER pQueue,
PQQ_ENTRY pEntry) {
    unsigned int Head;

    Head = (pQueue->Head + 1) % pQueue->Size;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Head == pQueue->Tail) {
	return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[pQueue->Head] = pEntry;
    wmb();
    pQueue->Head = Head;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushHead */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
extern __inline char
QQ_PushTail(
PQQ_CONTAINER pQueue,
PQQ_ENTRY pEntry) {
    unsigned int Tail;

    Tail = pQueue->Tail;
    if(Tail == 0) {
	Tail = pQueue->Size;
    } /* if */
    Tail--;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Tail == pQueue->Head) {
	return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[Tail] = pEntry;
    wmb();
    pQueue->Tail = Tail;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushTail */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline PQQ_ENTRY
QQ_PopHead(
PQQ_CONTAINER pQueue) {
    unsigned int Head;
    PQQ_ENTRY Entry;

    Head = pQueue->Head;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Head == pQueue->Tail) {
	return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    if(Head == 0) {
	Head = pQueue->Size;
    } /* if */
    Head--;

    Entry = pQueue->Array[Head];
#ifdef EMBEDDED
    membar();
#else
    mb();
#endif
    pQueue->Head = Head;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopHead */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline PQQ_ENTRY
QQ_PopTail(
PQQ_CONTAINER pQueue) {
    unsigned int Tail;
    PQQ_ENTRY Entry;

    Tail = pQueue->Tail;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Tail == pQueue->Head) {
	return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    Entry = pQueue->Array[Tail];
#ifdef EMBEDDED
    membar();
#else
    mb();
#endif
    pQueue->Tail = (Tail + 1) % pQueue->Size;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopTail */


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline PQQ_ENTRY
QQ_GetHead(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
	return (PQQ_ENTRY) 0;
    }

    if(pQueue->Head > Idx)
    {
	Idx = pQueue->Head - Idx;
    }
    else
    {
	Idx = pQueue->Size - (Idx - pQueue->Head);
    }
    Idx--;

    return pQueue->Array[Idx];
}


/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
extern __inline PQQ_ENTRY
QQ_GetTail(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
	return (PQQ_ENTRY) 0;
    }

    Idx += pQueue->Tail;
    if(Idx >= pQueue->Size)
    {
	Idx = Idx - pQueue->Size;
    }

    return pQueue->Array[Idx];
}

#endif /* QQ_USE_MACROS */


#endif /* QUEUE_H */
