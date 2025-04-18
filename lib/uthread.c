// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Ahmad Fatoum, Pengutronix
 * Copyright (C) 2025 Linaro Limited
 *
 * An implementation of cooperative multi-tasking inspired from barebox threads
 * https://github.com/barebox/barebox/blob/master/common/bthread.c
 */

#include <compiler.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdint.h>
#include <uthread.h>

static struct uthread main_thread = {
	.list = LIST_HEAD_INIT(main_thread.list),
};

static struct uthread *current = &main_thread;

/**
 * uthread_trampoline() - Call the current thread's entry point then resume the
 * main thread.
 *
 * This is a helper function which is used as the @func argument to the
 * initjmp() function, and ultimately invoked via setjmp(). It does not return
 * but instead longjmp()'s back to the main thread.
 */
static void __noreturn uthread_trampoline(void)
{
	struct uthread *curr = current;

	curr->fn(curr->arg);
	curr->done = true;
	current = &main_thread;
	longjmp(current->ctx, 1);
	/* Not reached */
	while (true)
		;
}

/**
 * uthread_free() - Free memory used by a uthread object.
 */
static void uthread_free(struct uthread *uthread)
{
	if (!uthread)
		return;
	free(uthread->stack);
	free(uthread);
}

int uthread_create(struct uthread *uthr, void (*fn)(void *), void *arg,
		   size_t stack_sz, unsigned int grp_id)
{
	bool user_allocated = false;

	if (!stack_sz)
		stack_sz = CONFIG_UTHREAD_STACK_SIZE;

	if (uthr) {
		user_allocated = true;
	} else {
		uthr = calloc(1, sizeof(*uthr));
		if (!uthr)
			return -1;
	}

	uthr->stack = memalign(16, stack_sz);
	if (!uthr->stack)
		goto err;

	uthr->fn = fn;
	uthr->arg = arg;
	uthr->grp_id = grp_id;

	list_add_tail(&uthr->list, &current->list);

	initjmp(uthr->ctx, uthread_trampoline, uthr->stack, stack_sz);

	return 0;
err:
	if (!user_allocated)
		free(uthr);
	return -1;
}

/**
 * uthread_resume() - switch execution to a given thread
 *
 * @uthread: the thread object that should be resumed
 */
static void uthread_resume(struct uthread *uthread)
{
	if (!setjmp(current->ctx)) {
		current = uthread;
		longjmp(uthread->ctx, 1);
	}
}

bool uthread_schedule(void)
{
	struct uthread *next;
	struct uthread *tmp;

	list_for_each_entry_safe(next, tmp, &current->list, list) {
		if (!next->done) {
			uthread_resume(next);
			return true;
		}
		/* Found a 'done' thread, free its resources */
		list_del(&next->list);
		uthread_free(next);
	}
	return false;
}

unsigned int uthread_grp_new_id(void)
{
	static unsigned int id;

	return ++id;
}

bool uthread_grp_done(unsigned int grp_id)
{
	struct uthread *next;

	list_for_each_entry(next, &main_thread.list, list) {
		if (next->grp_id == grp_id && !next->done)
			return false;
	}

	return true;
}

int uthread_mutex_lock(struct uthread_mutex *mutex)
{
	while (mutex->state == UTHREAD_MUTEX_LOCKED)
		uthread_schedule();

	mutex->state = UTHREAD_MUTEX_LOCKED;
	return 0;
}

int uthread_mutex_trylock(struct uthread_mutex *mutex)
{
	if (mutex->state == UTHREAD_MUTEX_UNLOCKED) {
		mutex->state = UTHREAD_MUTEX_LOCKED;
		return 0;
	}

	return -EBUSY;
}

int uthread_mutex_unlock(struct uthread_mutex *mutex)
{
	mutex->state = UTHREAD_MUTEX_UNLOCKED;

	return 0;
}
