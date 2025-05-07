// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#include <cyclic.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <asm/global_data.h>
#include <u-boot/schedule.h>
#include <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

void hw_watchdog_reset(void);

struct hlist_head *cyclic_get_list(void)
{
	/* Silence "discards 'volatile' qualifier" warning. */
	return (struct hlist_head *)&gd->cyclic_list;
}

static bool cyclic_is_registered(const struct cyclic_info *cyclic)
{
	const struct cyclic_info *c;

	hlist_for_each_entry(c, cyclic_get_list(), list) {
		if (c == cyclic)
			return true;
	}

	return false;
}

void cyclic_register(struct cyclic_info *cyclic, cyclic_func_t func,
		     uint64_t delay_us, const char *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, sizeof(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, cyclic_get_list());
}

void cyclic_unregister(struct cyclic_info *cyclic)
{
	if (!cyclic_is_registered(cyclic))
		return;

	hlist_del(&cyclic->list);
}

static void cyclic_run(void)
{
	struct cyclic_info *cyclic;
	struct hlist_node *tmp;
	uint64_t now, cpu_time;

	/* Prevent recursion */
	if (gd->flags & GD_FLG_CYCLIC_RUNNING)
		return;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	hlist_for_each_entry_safe(cyclic, tmp, cyclic_get_list(), list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		now = get_timer_us(0);
		if (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			cyclic->run_cnt++;
			cpu_time = get_timer_us(0) - now;
			cyclic->cpu_time_us += cpu_time;

			/* Check if cpu-time exceeds max allowed time */
			if ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_time,
				       CONFIG_CYCLIC_MAX_CPU_TIME_US);

				/*
				 * Don't disable this function, just warn once
				 * about this exceeding CPU time usage
				 */
				cyclic->already_warned = true;
			}
		}
	}
	gd->flags &= ~GD_FLG_CYCLIC_RUNNING;
}

void schedule(void)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	if (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	if (gd)
		cyclic_run();

	uthread_schedule();
}

int cyclic_unregister_all(void)
{
	struct cyclic_info *cyclic;
	struct hlist_node *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, cyclic_get_list(), list)
		cyclic_unregister(cyclic);

	return 0;
}
