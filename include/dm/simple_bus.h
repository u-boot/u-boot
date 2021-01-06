/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 */

#ifndef __DM_SIMPLE_BUS_H
#define __DM_SIMPLE_BUS_H

struct simple_bus_plat {
	u32 base;
	u32 size;
	u32 target;
};

#endif
