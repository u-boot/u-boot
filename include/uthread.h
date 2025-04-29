/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Linaro Limited
 */

#include <linux/list.h>
#include <linux/types.h>
#include <setjmp.h>

#ifndef _UTHREAD_H_
#define _UTHREAD_H_

/**
 * DOC: Overview
 *
 * The uthread framework is a basic task scheduler that allows to run functions
 * "in parallel" on a single CPU core. The scheduling is cooperative, not
 * preemptive -- meaning that context switches from one task to another task is
 * voluntary, via a call to uthread_schedule(). This characteristic makes thread
 * synchronization much easier, because a thread cannot be interrupted in the
 * middle of a critical section (reading from or writing to shared state, for
 * instance).
 *
 * CONFIG_UTHREAD in lib/Kconfig enables the uthread framework. When disabled,
 * the uthread_create()  and uthread_schedule() functions may still be used so
 * that code differences between uthreads enabled and disabled can be reduced to
 * a minimum.
 */

/**
 * struct uthread - a thread object
 *
 * @fn: thread entry point
 * @arg: argument passed to the entry point when the thread is started
 * @ctx: context to resume execution of this thread (via longjmp())
 * @stack: initial stack pointer for the thread
 * @done: true once @fn has returned, false otherwise
 * @grp_id: user-supplied identifier for this thread and possibly others. A
 * thread can belong to zero or one group (not more), and a group may contain
 * any number of threads.
 * @list: link in the global scheduler list
 */
struct uthread {
	void (*fn)(void *arg);
	void *arg;
	jmp_buf ctx;
	void *stack;
	bool done;
	unsigned int grp_id;
	struct list_head list;
};

/**
 * enum uthread_mutex_state - internal state of a struct uthread_mutex
 *
 * @UTHREAD_MUTEX_UNLOCKED: mutex has no owner
 * @UTHREAD_MUTEX_LOCKED: mutex has one owner
 */
enum uthread_mutex_state {
	UTHREAD_MUTEX_UNLOCKED = 0,
	UTHREAD_MUTEX_LOCKED = 1
};

/**
 * struct uthread_mutex - a mutex object
 *
 * @state: the internal state of the mutex
 */
struct uthread_mutex {
	enum uthread_mutex_state state;
};

#define UTHREAD_MUTEX_INITIALIZER { .state = UTHREAD_MUTEX_UNLOCKED }

#ifdef CONFIG_UTHREAD

/**
 * uthread_create() - Create a uthread object and make it ready for execution
 *
 * Threads are automatically deleted when they return from their entry point.
 *
 * @uthr: a pointer to a user-allocated uthread structure to store information
 * about the new thread, or NULL to let the framework allocate and manage its
 * own structure.
 * @fn: the thread's entry point
 * @arg: argument passed to the thread's entry point
 * @stack_sz: stack size for the new thread (in bytes). The stack is allocated
 * on the heap.
 * @grp_id: an optional thread group ID that the new thread should belong to
 * (zero for no group)
 */
int uthread_create(struct uthread *uthr, void (*fn)(void *), void *arg,
		   size_t stack_sz, unsigned int grp_id);
/**
 * uthread_schedule() - yield the CPU to the next runnable thread
 *
 * This function is called either by the main thread or any secondary thread
 * (that is, any thread created via uthread_create()) to switch execution to
 * the next runnable thread.
 *
 * Return: true if a thread was scheduled, false if no runnable thread was found
 */
bool uthread_schedule(void);
/**
 * uthread_grp_new_id() - return a new ID for a thread group
 *
 * Return: the new thread group ID
 */
unsigned int uthread_grp_new_id(void);
/**
 * uthread_grp_done() - test if all threads in a group are done
 *
 * @grp_id: the ID of the thread group that should be considered
 * Return: false if the group contains at least one runnable thread (i.e., one
 * thread which entry point has not returned yet), true otherwise
 */
bool uthread_grp_done(unsigned int grp_id);

/**
 * uthread_mutex_lock() - lock a mutex
 *
 * If the cwmutexlock is available (i.e., not owned by any other thread), then
 * it is locked for use by the current thread. Otherwise the current thread
 * blocks: it enters a wait loop by scheduling other threads until the mutex
 * becomes unlocked.
 *
 * @mutex: pointer to the mutex to lock
 * Return: 0 on success, in which case the lock is owned by the calling thread.
 * != 0 otherwise (the lock is not owned by the calling thread).
 */
int uthread_mutex_lock(struct uthread_mutex *mutex);

/**
 * uthread_mutex_trylock() - lock a mutex if not currently locked
 *
 * Similar to uthread_mutex_lock() except return immediately if the mutex is
 * locked already.
 *
 * @mutex: pointer to the mutex to lock
 * Return: 0 on success, in which case the lock is owned by the calling thread.
 * EBUSY if the mutex is already locked by another thread. Any other non-zero
 * value on error.
 */
int uthread_mutex_trylock(struct uthread_mutex *mutex);

/**
 * uthread_mutex_unlock() - unlock a mutex
 *
 * The mutex is assumed to be owned by the calling thread on entry. On exit, it
 * is unlocked.
 *
 * @mutex: pointer to the mutex to unlock
 * Return: 0 on success, != 0 on error
 */
int uthread_mutex_unlock(struct uthread_mutex *mutex);

#else

static inline int uthread_create(struct uthread *uthr, void (*fn)(void *),
				 void *arg, size_t stack_sz,
				 unsigned int grp_id)
{
	fn(arg);
	return 0;
}

static inline bool uthread_schedule(void)
{
	return false;
}

static inline unsigned int uthread_grp_new_id(void)
{
	return 0;
}

static inline bool uthread_grp_done(unsigned int grp_id)
{
	return true;
}

/* These are macros for convenience on the caller side */
#define uthread_mutex_lock(_mutex) ({ 0; })
#define uthread_mutex_trylock(_mutex) ({ 0 })
#define uthread_mutex_unlock(_mutex) ({ 0; })

#endif /* CONFIG_UTHREAD */
#endif /* _UTHREAD_H_ */
