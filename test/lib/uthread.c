// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Linaro Limited
 *
 * Unit test for uthread
 */

#include <stdbool.h>
#include <test/lib.h>
#include <test/ut.h>
#include <uthread.h>

static int count;

/* A thread entry point */
static void worker(void *arg)
{
	int loops = (int)(unsigned long)arg;
	int i;

	for (i = 0; i < loops; i++) {
		count++;
		uthread_schedule();
	}
}

/*
 * uthread() - testing the uthread API
 *
 * This function creates two threads with the same entry point. The first one
 * receives 5 as an argument, the second one receives 10. The number indicates
 * the number of time the worker thread should loop on uthread_schedule()
 * before returning. The workers increment a global counter each time they loop.
 * As a result the main thread knows how many times it should call
 * uthread_schedule() to let the two threads proceed, and it also knows which
 * value the counter should have at any moment.
 */
static int uthread(struct unit_test_state *uts)
{
	int i;
	int id1, id2;

	count = 0;
	id1 = uthread_grp_new_id();
	ut_assert(id1 != 0);
	id2 = uthread_grp_new_id();
	ut_assert(id2 != 0);
	ut_assert(id1 != id2);
	ut_assertok(uthread_create(NULL, worker, (void *)5, 0, id1));
	ut_assertok(uthread_create(NULL, worker, (void *)10, 0, 0));
	/*
	 * The first call is expected to schedule the first worker, which will
	 * schedule the second one, which will schedule back to the main thread
	 * (here). Therefore count should be 2.
	 */
	ut_assert(uthread_schedule());
	ut_asserteq(2, count);
	ut_assert(!uthread_grp_done(id1));
	/* Four more calls should bring the count to 10 */
	for (i = 0; i < 4; i++) {
		ut_assert(!uthread_grp_done(id1));
		ut_assert(uthread_schedule());
	}
	ut_asserteq(10, count);
	/* This one allows the first worker to exit */
	ut_assert(uthread_schedule());
	/* At this point there should be no runnable thread in group 'id1' */
	ut_assert(uthread_grp_done(id1));
	/* Five more calls for the second worker to finish incrementing  */
	for (i = 0; i < 5; i++)
		ut_assert(uthread_schedule());
	ut_asserteq(15, count);
	/* Plus one call to let the second worker return from its entry point */
	ut_assert(uthread_schedule());
	/* Now both tasks should be done, schedule should return false */
	ut_assert(!uthread_schedule());

	return 0;
}
LIB_TEST(uthread, 0);
