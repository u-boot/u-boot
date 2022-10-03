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

DECLARE_GLOBAL_DATA_PTR;

void hw_watchdog_reset(void);

struct list_head *cyclic_get_list(void)
{
	return &gd->cyclic->cyclic_list;
}

struct cyclic_info *cyclic_register(cyclic_func_t func, uint64_t delay_us,
				    const char *name, void *ctx)
{
	struct cyclic_info *cyclic;

	if (!gd->cyclic->cyclic_ready) {
		pr_debug("Cyclic IF not ready yet\n");
		return NULL;
	}

	cyclic = calloc(1, sizeof(struct cyclic_info));
	if (!cyclic) {
		pr_debug("Memory allocation error\n");
		return NULL;
	}

	/* Store values in struct */
	cyclic->func = func;
	cyclic->ctx = ctx;
	cyclic->name = strdup(name);
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = timer_get_us();
	list_add_tail(&cyclic->list, &gd->cyclic->cyclic_list);

	return cyclic;
}

int cyclic_unregister(struct cyclic_info *cyclic)
{
	list_del(&cyclic->list);
	free(cyclic);

	return 0;
}

void cyclic_run(void)
{
	struct cyclic_info *cyclic, *tmp;
	uint64_t now, cpu_time;

	/* Prevent recursion */
	if (gd->cyclic->cyclic_running)
		return;

	gd->cyclic->cyclic_running = true;
	list_for_each_entry_safe(cyclic, tmp, &gd->cyclic->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		now = timer_get_us();
		if (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic->ctx);
			cyclic->run_cnt++;
			cpu_time = timer_get_us() - now;
			cyclic->cpu_time_us += cpu_time;

			/* Check if cpu-time exceeds max allowed time */
			if (cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max, disabling\n",
				       cyclic->name, cpu_time,
				       CONFIG_CYCLIC_MAX_CPU_TIME_US);

				/* Unregister this cyclic function */
				cyclic_unregister(cyclic);
			}
		}
	}
	gd->cyclic->cyclic_running = false;
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
	if (gd && gd->cyclic && gd->cyclic->cyclic_ready)
		cyclic_run();
}

int cyclic_uninit(void)
{
	struct cyclic_info *cyclic, *tmp;

	list_for_each_entry_safe(cyclic, tmp, &gd->cyclic->cyclic_list, list)
		cyclic_unregister(cyclic);
	gd->cyclic->cyclic_ready = false;

	return 0;
}

int cyclic_init(void)
{
	int size = sizeof(struct cyclic_drv);

	gd->cyclic = (struct cyclic_drv *)malloc(size);
	if (!gd->cyclic)
		return -ENOMEM;

	memset(gd->cyclic, '\0', size);
	INIT_LIST_HEAD(&gd->cyclic->cyclic_list);
	gd->cyclic->cyclic_ready = true;

	return 0;
}
