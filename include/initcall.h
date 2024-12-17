/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __INITCALL_H
#define __INITCALL_H

#include <asm/types.h>
#include <event.h>

_Static_assert(EVT_COUNT < 256, "Can only support 256 event types with 8 bits");

#define INITCALL(_call) \
	do { \
		if (!ret) { \
			debug("%s(): calling %s()\n", __func__, #_call); \
			ret = _call(); \
		} \
	} while (0)

#define INITCALL_EVT(_evt) \
	do { \
		if (!ret) { \
			debug("%s(): event %d/%s\n", __func__, _evt, \
			      event_type_name(_evt)) ; \
			ret = event_notify_null(_evt); \
		} \
	} while (0)

#if defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
#define WATCHDOG_INIT() INITCALL(init_func_watchdog_init)
#define WATCHDOG_RESET() INITCALL(init_func_watchdog_reset)
#else
#define WATCHDOG_INIT()
#define WATCHDOG_RESET()
#endif

#endif
