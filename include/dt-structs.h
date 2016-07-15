/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DT_STTUCTS
#define __DT_STTUCTS

/* These structures may only be used in SPL */
#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct phandle_2_cell {
	const void *node;
	int id;
};
#include <generated/dt-structs.h>
#endif

#endif
