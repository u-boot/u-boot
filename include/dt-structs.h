/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DT_STRUCTS
#define __DT_STRUCTS

/* These structures may only be used in SPL */
#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct phandle_0_arg {
	const void *node;
	int arg[0];
};

struct phandle_1_arg {
	const void *node;
	int arg[1];
};

struct phandle_2_arg {
	const void *node;
	int arg[2];
};
#include <generated/dt-structs-gen.h>
#endif

#endif
