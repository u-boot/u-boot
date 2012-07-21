#ifndef IxOsalOsTypes_H
#define IxOsalOsTypes_H

#include <asm/types.h>

typedef s64 INT64;
typedef u64 UINT64;
typedef s32 INT32;
typedef u32 UINT32;
typedef s16 INT16;
typedef u16 UINT16;
typedef s8 INT8;
typedef u8 UINT8;

typedef u32 ULONG;
typedef u16 USHORT;
typedef u8 UCHAR;
typedef u32 BOOL;

#if 0 /* FIXME */

/* Default stack limit is 10 KB */
#define IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE  (10240)

/* Maximum stack limit is 32 MB */
#define IX_OSAL_OS_THREAD_MAX_STACK_SIZE      (33554432)  /* 32 MBytes */

/* Default thread priority */
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (90)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (255)

#endif /* FIXME */

#define IX_OSAL_OS_WAIT_FOREVER (-1L)
#define IX_OSAL_OS_WAIT_NONE	0


/* Thread handle is eventually an int type */
typedef int IxOsalOsThread;

/* Semaphore handle FIXME */
typedef int IxOsalOsSemaphore;

/* Mutex handle */
typedef int IxOsalOsMutex;

/*
 * Fast mutex handle - fast mutex operations are implemented in
 * native assembler code using atomic test-and-set instructions
 */
typedef int IxOsalOsFastMutex;

typedef struct
{
} IxOsalOsMessageQueue;


#endif /* IxOsalOsTypes_H */
