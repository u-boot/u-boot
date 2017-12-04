/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LOG_H
#define __LOG_H

#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

#ifdef CONFIG_SPL_BUILD
#define _SPL_BUILD	1
#else
#define _SPL_BUILD	0
#endif

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)			\
	do {						\
		if (cond)				\
			printf(pr_fmt(fmt), ##args);	\
	} while (0)

/* Show a message if DEBUG is defined in a file */
#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)

/* Show a message if not in SPL */
#define warn_non_spl(fmt, args...)			\
	debug_cond(!_SPL_BUILD, fmt, ##args)

/*
 * An assertion is run-time check done in debug mode only. If DEBUG is not
 * defined then it is skipped. If DEBUG is defined and the assertion fails,
 * then it calls panic*( which may or may not reset/halt U-Boot (see
 * CONFIG_PANIC_HANG), It is hoped that all failing assertions are found
 * before release, and after release it is hoped that they don't matter. But
 * in any case these failing assertions cannot be fixed with a reset (which
 * may just do the same assertion again).
 */
void __assert_fail(const char *assertion, const char *file, unsigned int line,
		   const char *function);
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

#endif
