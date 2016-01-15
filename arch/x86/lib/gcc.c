/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 coresystems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifdef __GNUC__

/*
 * GCC's libgcc handling is quite broken. While the libgcc functions
 * are always regparm(0) the code that calls them uses whatever the
 * compiler call specifies. Therefore we need a wrapper around those
 * functions. See gcc bug PR41055 for more information.
 */
#define WRAP_LIBGCC_CALL(type, name) \
	type __normal_##name(type a, type b) __attribute__((regparm(0))); \
	type __wrap_##name(type a, type b); \
	type __attribute__((no_instrument_function)) \
		__wrap_##name(type a, type b) \
		 { return __normal_##name(a, b); }

WRAP_LIBGCC_CALL(long long, __divdi3)
WRAP_LIBGCC_CALL(unsigned long long, __udivdi3)
WRAP_LIBGCC_CALL(long long, __moddi3)
WRAP_LIBGCC_CALL(unsigned long long, __umoddi3)

#endif
