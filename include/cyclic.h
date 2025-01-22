/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ifndef __cyclic_h
#define __cyclic_h

#include <linux/list.h>
#include <asm/types.h>
#include <u-boot/schedule.h> // to be removed later

/**
 * struct cyclic_info - Information about cyclic execution function
 *
 * @func: Function to call periodically
 * @name: Name of the cyclic function, e.g. shown in the commands
 * @delay_us: Delay is us after which this function shall get executed
 * @start_time_us: Start time in us, when this function started its execution
 * @cpu_time_us: Total CPU time of this function
 * @run_cnt: Counter of executions occurances
 * @next_call: Next time in us, when the function shall be executed again
 * @list: List node
 * @already_warned: Flag that we've warned about exceeding CPU time usage
 *
 * When !CONFIG_CYCLIC, this struct is empty.
 */
struct cyclic_info {
#if defined(CONFIG_CYCLIC)
	void (*func)(struct cyclic_info *c);
	const char *name;
	uint64_t delay_us;
	uint64_t start_time_us;
	uint64_t cpu_time_us;
	uint64_t run_cnt;
	uint64_t next_call;
	struct hlist_node list;
	bool already_warned;
#endif
};

/** Function type for cyclic functions */
typedef void (*cyclic_func_t)(struct cyclic_info *c);

#if CONFIG_IS_ENABLED(CYCLIC)

/**
 * cyclic_register - Register a new cyclic function
 *
 * @cyclic: Cyclic info structure
 * @func: Function to call periodically
 * @delay_us: Delay is us after which this function shall get executed
 * @name: Cyclic function name/id
 *
 * The function @func will be called with @cyclic as its
 * argument. @cyclic will usually be embedded in some device-specific
 * structure, which the callback can retrieve using container_of().
 */
void cyclic_register(struct cyclic_info *cyclic, cyclic_func_t func,
		     uint64_t delay_us, const char *name);

/**
 * cyclic_unregister - Unregister a cyclic function
 *
 * @cyclic: Pointer to cyclic_struct of the function that shall be removed
 */
void cyclic_unregister(struct cyclic_info *cyclic);

/**
 * cyclic_unregister_all() - Clean up cyclic functions
 *
 * This removes all cyclic functions
 */
int cyclic_unregister_all(void);

/**
 * cyclic_get_list() - Get cyclic list pointer
 *
 * Return the cyclic list pointer
 *
 * @return: pointer to cyclic_list
 */
struct hlist_head *cyclic_get_list(void);

#else

static inline void cyclic_register(struct cyclic_info *cyclic, cyclic_func_t func,
				   uint64_t delay_us, const char *name)
{
}

static inline void cyclic_unregister(struct cyclic_info *cyclic)
{
}

static inline int cyclic_unregister_all(void)
{
	return 0;
}
#endif /* CYCLIC */

#endif
