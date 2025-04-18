/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _U_BOOT_SCHEDULE_H
#define _U_BOOT_SCHEDULE_H

#include <uthread.h>

#if CONFIG_IS_ENABLED(CYCLIC)
/**
 * schedule() - Schedule all potentially waiting tasks
 *
 * Run all pending tasks registered via the cyclic framework, and
 * potentially perform other actions that need to be done
 * periodically.
 */
void schedule(void);

#else

static inline void schedule(void)
{
	uthread_schedule();
}

#endif

#endif
